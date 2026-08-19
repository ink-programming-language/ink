#include "lowering_context.h"

#include "ink/ir/analysis/type_layout.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ink::backend::llvm
{
  namespace
  {
    struct BlockSuccessors
    {
        std::size_t Values[2]{};
        std::size_t Count = 0;
    };

    BlockSuccessors blockSuccessors(const ir::Function &FunctionValue, std::size_t BlockIndex)
    {
      BlockSuccessors Result;
      const ir::Instruction &Terminator = *FunctionValue.Blocks[BlockIndex].Instructions.back();
      if (Terminator.kind() == ir::InstructionKind::Branch)
      {
        Result.Values[0] = static_cast<const ir::BranchInstruction &>(Terminator).Target.Block.value();
        Result.Count = 1;
      }
      else if (Terminator.kind() == ir::InstructionKind::ConditionalBranch)
      {
        const ir::ConditionalBranchInstruction &Branch = static_cast<const ir::ConditionalBranchInstruction &>(Terminator);
        Result.Values[0] = Branch.TrueTarget.Block.value();
        Result.Values[1] = Branch.FalseTarget.Block.value();
        Result.Count = 2;
      }
      return Result;
    }

    std::vector<std::size_t> blockLoweringOrder(const ir::Function &FunctionValue)
    {
      struct TraversalEntry
      {
          std::size_t BlockIndex = 0;
          std::size_t NextSuccessor = 0;
      };

      std::vector<bool> Visited(FunctionValue.Blocks.size(), false);
      std::vector<std::size_t> PostOrder;
      std::vector<TraversalEntry> Stack;
      Visited[0] = true;
      Stack.push_back({0, 0});
      while (!Stack.empty())
      {
        TraversalEntry &Entry = Stack.back();
        const BlockSuccessors Successors = blockSuccessors(FunctionValue, Entry.BlockIndex);
        if (Entry.NextSuccessor < Successors.Count)
        {
          const std::size_t Successor = Successors.Values[Entry.NextSuccessor++];
          if (!Visited[Successor])
          {
            Visited[Successor] = true;
            Stack.push_back({Successor, 0});
          }
          continue;
        }
        PostOrder.push_back(Entry.BlockIndex);
        Stack.pop_back();
      }
      std::reverse(PostOrder.begin(), PostOrder.end());
      for (std::size_t BlockIndex = 0; BlockIndex < FunctionValue.Blocks.size(); ++BlockIndex)
      {
        if (!Visited[BlockIndex])
        {
          PostOrder.push_back(BlockIndex);
        }
      }
      return PostOrder;
    }
  } // namespace

  class LoweringContext::FunctionLoweringContext final
  {
    public:
      FunctionLoweringContext(LoweringContext &ModuleContext, const ir::Function &SourceFunction, ::llvm::Function &TargetFunction)
          : ModuleContext(ModuleContext),
            SourceFunction(SourceFunction),
            TargetFunction(TargetFunction),
            Builder(ModuleContext.Context)
      {
      }

      bool lower();

    private:
      struct PendingPhi
      {
          const ir::PhiInstruction *Source = nullptr;
          ::llvm::PHINode *Target = nullptr;
      };

      bool createPhiNodes();
      bool lowerInstruction(const ir::Instruction &InstructionValue);
      ::llvm::Value *lowerValue(const ir::Value &Value);
      bool completePhiNodes();
      ::llvm::Value *lowerMemoryLoad(const ir::Type &TypeValue, ::llvm::Value *Pointer, const std::string &Name);
      bool lowerMemoryStore(const ir::Type &TypeValue, ::llvm::Value *StoredValue, ::llvm::Value *Pointer);
      ::llvm::Value *offsetPointer(::llvm::Value *Pointer, std::size_t ByteOffset, const std::string &Name);
      ::llvm::CmpInst::Predicate lowerComparePredicate(ir::ComparePredicate Predicate, ir::TypeKind OperandType);

      LoweringContext &ModuleContext;
      const ir::Function &SourceFunction;
      ::llvm::Function &TargetFunction;
      ::llvm::IRBuilder<> Builder;
      std::unordered_map<std::size_t, ::llvm::Value *> Values;
      std::unordered_map<const ::llvm::Value *, ::llvm::AllocaInst *> SliceAllocas;
      std::vector<::llvm::BasicBlock *> Blocks;
      std::vector<PendingPhi> PendingPhis;
  };

  bool LoweringContext::declareFunctions()
  {
    Functions.reserve(SourceModule.Functions.size());
    for (const ir::Function &FunctionValue : SourceModule.Functions)
    {
      if (FunctionValue.Kind == ir::FunctionKind::Imported)
      {
        addFailure<core::DiagnosticKind::LLVMImportedFunctionUnsupported>(FunctionValue.Name);
        return false;
      }
      if (FunctionValue.ResultType == nullptr)
      {
        addFailure<core::DiagnosticKind::LLVMFunctionMissingResultType>(FunctionValue.Name);
        return false;
      }
      ::llvm::Type *ResultType = lowerType(*FunctionValue.ResultType);
      if (ResultType == nullptr)
      {
        return false;
      }
      std::vector<::llvm::Type *> ParameterTypes;
      ParameterTypes.reserve(FunctionValue.parameterCount());
      for (std::size_t ParameterIndex = 0; ParameterIndex < FunctionValue.parameterCount(); ++ParameterIndex)
      {
        const ir::Type *ParameterType = FunctionValue.parameterType(ParameterIndex);
        if (ParameterType == nullptr)
        {
          addFailure<core::DiagnosticKind::LLVMFunctionParameterMissingType>(FunctionValue.Name, ParameterIndex);
          return false;
        }
        ::llvm::Type *LoweredParameterType = lowerType(*ParameterType);
        if (LoweredParameterType == nullptr)
        {
          return false;
        }
        ParameterTypes.push_back(LoweredParameterType);
      }
      ::llvm::FunctionType *Signature = ::llvm::FunctionType::get(ResultType, ParameterTypes, false);
      ::llvm::Function *Function = ::llvm::Function::Create(Signature, ::llvm::GlobalValue::ExternalLinkage, FunctionValue.Name.str(), TargetModule.get());
      std::size_t ParameterIndex = 0;
      for (::llvm::Argument &Argument : Function->args())
      {
        Argument.setName(valueName(ir::ValueId{ParameterIndex++}));
      }
      Functions.push_back(Function);
    }
    return true;
  }

  bool LoweringContext::lowerFunctions()
  {
    for (std::size_t FunctionIndex = 0; FunctionIndex < SourceModule.Functions.size(); ++FunctionIndex)
    {
      if (SourceModule.Functions[FunctionIndex].Kind != ir::FunctionKind::Definition)
      {
        continue;
      }
      FunctionLoweringContext FunctionLowering(*this, SourceModule.Functions[FunctionIndex], *Functions[FunctionIndex]);
      if (!FunctionLowering.lower())
      {
        return false;
      }
    }
    return true;
  }

  bool LoweringContext::FunctionLoweringContext::lower()
  {
    std::size_t ParameterIndex = 0;
    for (::llvm::Argument &Argument : TargetFunction.args())
    {
      Values.emplace(ParameterIndex++, &Argument);
    }
    Blocks.reserve(SourceFunction.Blocks.size());
    for (const ir::BasicBlock &Block : SourceFunction.Blocks)
    {
      Blocks.push_back(::llvm::BasicBlock::Create(ModuleContext.Context, Block.Name.str(), &TargetFunction));
    }
    if (!createPhiNodes())
    {
      return false;
    }
    const std::vector<std::size_t> LoweringOrder = blockLoweringOrder(SourceFunction);
    for (const std::size_t BlockIndex : LoweringOrder)
    {
      Builder.SetInsertPoint(Blocks[BlockIndex]);
      for (const std::unique_ptr<ir::Instruction> &InstructionValue : SourceFunction.Blocks[BlockIndex].Instructions)
      {
        if (InstructionValue->kind() != ir::InstructionKind::Phi && !lowerInstruction(*InstructionValue))
        {
          return false;
        }
      }
    }
    return completePhiNodes();
  }

  bool LoweringContext::FunctionLoweringContext::createPhiNodes()
  {
    for (std::size_t BlockIndex = 0; BlockIndex < SourceFunction.Blocks.size(); ++BlockIndex)
    {
      Builder.SetInsertPoint(Blocks[BlockIndex]);
      for (const std::unique_ptr<ir::Instruction> &InstructionValue : SourceFunction.Blocks[BlockIndex].Instructions)
      {
        if (InstructionValue->kind() != ir::InstructionKind::Phi)
        {
          break;
        }
        const ir::PhiInstruction &Phi = static_cast<const ir::PhiInstruction &>(*InstructionValue);
        if (Phi.ResultType == nullptr)
        {
          ModuleContext.addFailure<core::DiagnosticKind::LLVMPhiMissingResultType>(SourceFunction.Name, Phi.Result.value());
          return false;
        }
        ::llvm::Type *ResultType = ModuleContext.lowerType(*Phi.ResultType);
        if (ResultType == nullptr)
        {
          return false;
        }
        ::llvm::PHINode *TargetPhi = Builder.CreatePHI(ResultType, static_cast<unsigned>(Phi.IncomingValues.size()), ModuleContext.valueName(Phi.Result));
        Values.emplace(Phi.Result.value(), TargetPhi);
        PendingPhis.push_back({&Phi, TargetPhi});
      }
    }
    return true;
  }

  ::llvm::Value *LoweringContext::FunctionLoweringContext::lowerValue(const ir::Value &Value)
  {
    switch (Value.kind())
    {
    case ir::ValueKind::IntegerConstant:
    case ir::ValueKind::FloatConstant:
    case ir::ValueKind::StringConstant:
    case ir::ValueKind::NullConstant:
    case ir::ValueKind::ZeroInitializer:
    case ir::ValueKind::AggregateConstant:
      return ModuleContext.lowerConstant(static_cast<const ir::Constant &>(Value));
    case ir::ValueKind::ValueOperand:
    {
      const ir::ValueId Id = static_cast<const ir::ValueOperand &>(Value).id();
      const auto ValueIterator = Values.find(Id.value());
      if (ValueIterator == Values.end())
      {
        ModuleContext.addFailure<core::DiagnosticKind::LLVMUnresolvedSSAValue>(Id.value());
        return nullptr;
      }
      return ValueIterator->second;
    }
    case ir::ValueKind::GlobalAddressOperand:
      return ModuleContext.lowerGlobalAddress(static_cast<const ir::GlobalAddressOperand &>(Value));
    case ir::ValueKind::GlobalVariableAddressOperand:
      return ModuleContext.lowerGlobalVariableAddress(static_cast<const ir::GlobalVariableAddressOperand &>(Value));
    }
    ModuleContext.addFailure<core::DiagnosticKind::LLVMUnknownValueKind>();
    return nullptr;
  }

  bool LoweringContext::FunctionLoweringContext::lowerInstruction(const ir::Instruction &InstructionValue)
  {
    switch (InstructionValue.kind())
    {
    case ir::InstructionKind::Call:
    {
      const ir::CallInstruction &Call = static_cast<const ir::CallInstruction &>(InstructionValue);
      if (!Call.Callee.valid() || Call.Callee.value() >= ModuleContext.Functions.size())
      {
        ModuleContext.addFailure<core::DiagnosticKind::LLVMInvalidCallTarget>(Call.Callee.value());
        return false;
      }
      std::vector<::llvm::Value *> Arguments;
      Arguments.reserve(Call.Arguments.size());
      for (const ir::ValueHandle &Argument : Call.Arguments)
      {
        ::llvm::Value *LoweredArgument = lowerValue(*Argument);
        if (LoweredArgument == nullptr)
        {
          return false;
        }
        Arguments.push_back(LoweredArgument);
      }
      const std::string Name = Call.Result.has_value() ? ModuleContext.valueName(*Call.Result) : std::string{};
      ::llvm::CallInst *LoweredCall = Builder.CreateCall(ModuleContext.Functions[Call.Callee.value()], Arguments, Name);
      if (Call.Result.has_value())
      {
        Values.emplace(Call.Result->value(), LoweredCall);
      }
      return true;
    }
    case ir::InstructionKind::Import:
    {
      const ir::ImportInstruction &Import = static_cast<const ir::ImportInstruction &>(InstructionValue);
      ModuleContext.addFailure<core::DiagnosticKind::LLVMRuntimeModuleImportUnsupported>(Import.Module);
      return false;
    }
    case ir::InstructionKind::Alloca:
    {
      const ir::AllocaInstruction &Alloca = static_cast<const ir::AllocaInstruction &>(InstructionValue);
      ::llvm::Value *Size = lowerValue(*Alloca.Size);
      ::llvm::Type *SliceType = Alloca.ResultType == nullptr ? nullptr : ModuleContext.lowerType(*Alloca.ResultType);
      if (Size == nullptr || SliceType == nullptr)
      {
        return false;
      }
      ::llvm::AllocaInst *Storage = Builder.CreateAlloca(::llvm::Type::getInt8Ty(ModuleContext.Context), Size, ModuleContext.valueName(Alloca.Result) + ".storage");
      Storage->setAlignment(::llvm::Align(1));
      Builder.CreateMemSet(Storage, Builder.getInt8(0), Size, ::llvm::MaybeAlign(1));
      ::llvm::Value *SliceWithData = Builder.CreateInsertValue(::llvm::UndefValue::get(SliceType), Storage, {0}, ModuleContext.valueName(Alloca.Result) + ".data");
      ::llvm::Value *Slice = Builder.CreateInsertValue(SliceWithData, Size, {1}, ModuleContext.valueName(Alloca.Result));
      Values.emplace(Alloca.Result.value(), Slice);
      SliceAllocas.emplace(Slice, Storage);
      return true;
    }
    case ir::InstructionKind::GetElementPointer:
    {
      const ir::GetElementPointerInstruction &GetElementPointer = static_cast<const ir::GetElementPointerInstruction &>(InstructionValue);
      ::llvm::Value *Pointer = lowerValue(*GetElementPointer.Pointer);
      ::llvm::Value *Index = lowerValue(*GetElementPointer.Index);
      ::llvm::Type *ElementType = GetElementPointer.ElementType == nullptr ? nullptr : ModuleContext.lowerType(*GetElementPointer.ElementType);
      if (Pointer == nullptr || Index == nullptr || ElementType == nullptr)
      {
        return false;
      }
      std::vector<::llvm::Value *> Indices{Index};
      const ir::Type *IndexedType = GetElementPointer.ElementType;
      for (const ir::ValueHandle &FieldIndexValue : GetElementPointer.FieldIndices)
      {
        const ir::IntegerConstant &FieldIndex = static_cast<const ir::IntegerConstant &>(*FieldIndexValue);
        const ir::StructType &Struct = static_cast<const ir::StructType &>(*IndexedType);
        const std::size_t LogicalIndex = static_cast<std::size_t>(FieldIndex.unsignedValue());
        const std::vector<unsigned> *PhysicalIndices = ModuleContext.physicalFieldIndices(Struct, LogicalIndex);
        if (PhysicalIndices == nullptr)
        {
          return false;
        }
        for (const unsigned PhysicalIndex : *PhysicalIndices)
        {
          Indices.push_back(::llvm::ConstantInt::get(::llvm::Type::getInt32Ty(ModuleContext.Context), PhysicalIndex));
        }
        IndexedType = Struct.fieldType(LogicalIndex);
      }
      ::llvm::Value *Result = Builder.CreateGEP(ElementType, Pointer, Indices, ModuleContext.valueName(GetElementPointer.Result));
      Values.emplace(GetElementPointer.Result.value(), Result);
      return true;
    }
    case ir::InstructionKind::Load:
    {
      const ir::LoadInstruction &Load = static_cast<const ir::LoadInstruction &>(InstructionValue);
      ::llvm::Value *Pointer = lowerValue(*Load.Pointer);
      if (Pointer == nullptr || Load.ResultType == nullptr)
      {
        return false;
      }
      ::llvm::Value *Result = lowerMemoryLoad(*Load.ResultType, Pointer, ModuleContext.valueName(Load.Result));
      if (Result == nullptr)
      {
        return false;
      }
      Values.emplace(Load.Result.value(), Result);
      return true;
    }
    case ir::InstructionKind::Store:
    {
      const ir::StoreInstruction &Store = static_cast<const ir::StoreInstruction &>(InstructionValue);
      ::llvm::Value *StoredValue = lowerValue(*Store.StoredValue);
      ::llvm::Value *Pointer = lowerValue(*Store.Pointer);
      if (StoredValue == nullptr || Pointer == nullptr)
      {
        return false;
      }
      return lowerMemoryStore(Store.StoredValue->type(), StoredValue, Pointer);
    }
    case ir::InstructionKind::LifetimeEnd:
    {
      const ir::LifetimeEndInstruction &LifetimeEnd = static_cast<const ir::LifetimeEndInstruction &>(InstructionValue);
      ::llvm::Value *Slice = lowerValue(*LifetimeEnd.Slice);
      if (Slice == nullptr)
      {
        return false;
      }
      const auto Storage = SliceAllocas.find(Slice);
      // LLVM lifetime markers accept only direct allocas; non-local slices do not receive an invalid optimization hint.
      if (Storage != SliceAllocas.end())
      {
        Builder.CreateLifetimeEnd(Storage->second);
      }
      return true;
    }
    case ir::InstructionKind::SliceData:
    {
      const ir::SliceDataInstruction &SliceData = static_cast<const ir::SliceDataInstruction &>(InstructionValue);
      ::llvm::Value *Slice = lowerValue(*SliceData.Slice);
      if (Slice == nullptr)
      {
        return false;
      }
      ::llvm::Value *Result = Builder.CreateExtractValue(Slice, {0}, ModuleContext.valueName(SliceData.Result));
      Values.emplace(SliceData.Result.value(), Result);
      return true;
    }
    case ir::InstructionKind::SliceLength:
    {
      const ir::SliceLengthInstruction &SliceLength = static_cast<const ir::SliceLengthInstruction &>(InstructionValue);
      ::llvm::Value *Slice = lowerValue(*SliceLength.Slice);
      if (Slice == nullptr)
      {
        return false;
      }
      ::llvm::Value *Result = Builder.CreateExtractValue(Slice, {1}, ModuleContext.valueName(SliceLength.Result));
      Values.emplace(SliceLength.Result.value(), Result);
      return true;
    }
    case ir::InstructionKind::Phi:
      return true;
    case ir::InstructionKind::Add:
    {
      const ir::AddInstruction &Add = static_cast<const ir::AddInstruction &>(InstructionValue);
      ::llvm::Value *Left = lowerValue(*Add.Left);
      ::llvm::Value *Right = lowerValue(*Add.Right);
      if (Left == nullptr || Right == nullptr)
      {
        return false;
      }
      ::llvm::Value *Result = Builder.CreateAdd(Left, Right, ModuleContext.valueName(Add.Result));
      Values.emplace(Add.Result.value(), Result);
      return true;
    }
    case ir::InstructionKind::Compare:
    {
      const ir::CompareInstruction &Compare = static_cast<const ir::CompareInstruction &>(InstructionValue);
      ::llvm::Value *Left = lowerValue(*Compare.Left);
      ::llvm::Value *Right = lowerValue(*Compare.Right);
      if (Left == nullptr || Right == nullptr)
      {
        return false;
      }
      const ::llvm::CmpInst::Predicate Predicate = lowerComparePredicate(Compare.Predicate, Compare.Left->type().kind());
      if (Predicate == ::llvm::CmpInst::BAD_ICMP_PREDICATE)
      {
        ModuleContext.addFailure<core::DiagnosticKind::LLVMUnsupportedComparison>();
        return false;
      }
      ::llvm::Value *Result = Builder.CreateICmp(Predicate, Left, Right, ModuleContext.valueName(Compare.Result));
      Values.emplace(Compare.Result.value(), Result);
      return true;
    }
    case ir::InstructionKind::InsertValue:
    {
      const ir::InsertValueInstruction &Insert = static_cast<const ir::InsertValueInstruction &>(InstructionValue);
      ::llvm::Value *Aggregate = lowerValue(*Insert.Aggregate);
      ::llvm::Value *Element = lowerValue(*Insert.Element);
      if (Aggregate == nullptr || Element == nullptr)
      {
        return false;
      }
      const ir::StructType &Struct = static_cast<const ir::StructType &>(*Insert.ResultType);
      const std::vector<unsigned> *FieldIndices = ModuleContext.physicalFieldIndices(Struct, Insert.FieldIndex);
      if (FieldIndices == nullptr)
      {
        return false;
      }
      ::llvm::Value *Result = Builder.CreateInsertValue(Aggregate, Element, *FieldIndices, ModuleContext.valueName(Insert.Result));
      Values.emplace(Insert.Result.value(), Result);
      return true;
    }
    case ir::InstructionKind::ExtractValue:
    {
      const ir::ExtractValueInstruction &Extract = static_cast<const ir::ExtractValueInstruction &>(InstructionValue);
      ::llvm::Value *Aggregate = lowerValue(*Extract.Aggregate);
      if (Aggregate == nullptr)
      {
        return false;
      }
      const ir::StructType &Struct = static_cast<const ir::StructType &>(Extract.Aggregate->type());
      const std::vector<unsigned> *FieldIndices = ModuleContext.physicalFieldIndices(Struct, Extract.FieldIndex);
      if (FieldIndices == nullptr)
      {
        return false;
      }
      ::llvm::Value *Result = Builder.CreateExtractValue(Aggregate, *FieldIndices, ModuleContext.valueName(Extract.Result));
      Values.emplace(Extract.Result.value(), Result);
      return true;
    }
    case ir::InstructionKind::Branch:
    {
      const ir::BranchInstruction &Branch = static_cast<const ir::BranchInstruction &>(InstructionValue);
      Builder.CreateBr(Blocks[Branch.Target.Block.value()]);
      return true;
    }
    case ir::InstructionKind::ConditionalBranch:
    {
      const ir::ConditionalBranchInstruction &Branch = static_cast<const ir::ConditionalBranchInstruction &>(InstructionValue);
      ::llvm::Value *Condition = lowerValue(*Branch.Condition);
      if (Condition == nullptr)
      {
        return false;
      }
      Builder.CreateCondBr(Condition, Blocks[Branch.TrueTarget.Block.value()], Blocks[Branch.FalseTarget.Block.value()]);
      return true;
    }
    case ir::InstructionKind::Return:
    {
      const ir::ReturnInstruction &Return = static_cast<const ir::ReturnInstruction &>(InstructionValue);
      if (!Return.ReturnValue)
      {
        Builder.CreateRetVoid();
        return true;
      }
      ::llvm::Value *ReturnValue = lowerValue(*Return.ReturnValue);
      if (ReturnValue == nullptr)
      {
        return false;
      }
      Builder.CreateRet(ReturnValue);
      return true;
    }
    }
    ModuleContext.addFailure<core::DiagnosticKind::LLVMUnknownInstructionKind>();
    return false;
  }

  bool LoweringContext::FunctionLoweringContext::completePhiNodes()
  {
    for (const PendingPhi &Pending : PendingPhis)
    {
      for (const ir::PhiIncoming &Incoming : Pending.Source->IncomingValues)
      {
        ::llvm::Value *Value = lowerValue(*Incoming.Value);
        if (Value == nullptr)
        {
          return false;
        }
        Pending.Target->addIncoming(Value, Blocks[Incoming.Predecessor.value()]);
      }
    }
    return true;
  }

  ::llvm::Value *LoweringContext::FunctionLoweringContext::lowerMemoryLoad(const ir::Type &TypeValue, ::llvm::Value *Pointer, const std::string &Name)
  {
    ::llvm::Type *TargetType = ModuleContext.lowerType(TypeValue);
    if (TargetType == nullptr)
    {
      return nullptr;
    }
    if (TypeValue.kind() != ir::TypeKind::Struct)
    {
      ::llvm::LoadInst *Result = Builder.CreateLoad(TargetType, Pointer, Name);
      Result->setAlignment(::llvm::Align(1));
      return Result;
    }

    const ir::StructType &Struct = static_cast<const ir::StructType &>(TypeValue);
    const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(Struct, ModuleContext.SourceModule.context().compilationContext().targetContext());
    if (!Layout.has_value())
    {
      ModuleContext.addFailure<core::DiagnosticKind::LLVMStructLoadLayoutUnavailable>(Struct.name());
      return nullptr;
    }
    ::llvm::Value *Result = ::llvm::UndefValue::get(TargetType);
    for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldCount(); ++FieldIndex)
    {
      const std::string FieldName = Name + ".field." + std::to_string(FieldIndex);
      ::llvm::Value *FieldPointer = offsetPointer(Pointer, Layout->FieldOffsets[FieldIndex], FieldName + ".address");
      ::llvm::Value *FieldValue = FieldPointer == nullptr ? nullptr : lowerMemoryLoad(*Struct.fieldType(FieldIndex), FieldPointer, FieldName);
      const std::vector<unsigned> *FieldIndices = ModuleContext.physicalFieldIndices(Struct, FieldIndex);
      if (FieldValue == nullptr || FieldIndices == nullptr)
      {
        return nullptr;
      }
      const std::string AggregateName = FieldIndex + 1 == Struct.fieldCount() ? Name : Name + ".aggregate." + std::to_string(FieldIndex);
      Result = Builder.CreateInsertValue(Result, FieldValue, *FieldIndices, AggregateName);
    }
    return Result;
  }

  bool LoweringContext::FunctionLoweringContext::lowerMemoryStore(const ir::Type &TypeValue, ::llvm::Value *StoredValue, ::llvm::Value *Pointer)
  {
    if (TypeValue.kind() != ir::TypeKind::Struct)
    {
      ::llvm::StoreInst *Result = Builder.CreateStore(StoredValue, Pointer);
      Result->setAlignment(::llvm::Align(1));
      return true;
    }

    const ir::StructType &Struct = static_cast<const ir::StructType &>(TypeValue);
    const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(Struct, ModuleContext.SourceModule.context().compilationContext().targetContext());
    if (!Layout.has_value())
    {
      ModuleContext.addFailure<core::DiagnosticKind::LLVMStructStoreLayoutUnavailable>(Struct.name());
      return false;
    }
    for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldCount(); ++FieldIndex)
    {
      const std::vector<unsigned> *FieldIndices = ModuleContext.physicalFieldIndices(Struct, FieldIndex);
      if (FieldIndices == nullptr)
      {
        return false;
      }
      ::llvm::Value *FieldValue = Builder.CreateExtractValue(StoredValue, *FieldIndices);
      ::llvm::Value *FieldPointer = offsetPointer(Pointer, Layout->FieldOffsets[FieldIndex], {});
      if (FieldPointer == nullptr || !lowerMemoryStore(*Struct.fieldType(FieldIndex), FieldValue, FieldPointer))
      {
        return false;
      }
    }
    return true;
  }

  ::llvm::Value *LoweringContext::FunctionLoweringContext::offsetPointer(::llvm::Value *Pointer, std::size_t ByteOffset, const std::string &Name)
  {
    const core::TargetContext &Target = ModuleContext.SourceModule.context().compilationContext().targetContext();
    if (ByteOffset > Target.maximumPointerSizeValue())
    {
      ModuleContext.addFailure<core::DiagnosticKind::LLVMStructFieldOffsetOutOfRange>(ByteOffset, Target.maximumPointerSizeValue());
      return nullptr;
    }
    ::llvm::Type *IndexType = ModuleContext.lowerType(ModuleContext.SourceModule.context().getType(ir::TypeKind::PointerSize));
    if (IndexType == nullptr)
    {
      return nullptr;
    }
    ::llvm::Constant *Offset = ::llvm::ConstantInt::get(static_cast<::llvm::IntegerType *>(IndexType), ByteOffset);
    return Builder.CreateGEP(::llvm::Type::getInt8Ty(ModuleContext.Context), Pointer, Offset, Name);
  }

  bool LoweringContext::lowerLifecycleFunctions()
  {
    if (SourceModule.Initializer.has_value() && !appendLifecycleFunction("llvm.global_ctors", *Functions[SourceModule.Initializer->value()]))
    {
      return false;
    }
    if (SourceModule.Finalizer.has_value() && !appendLifecycleFunction("llvm.global_dtors", *Functions[SourceModule.Finalizer->value()]))
    {
      return false;
    }
    return true;
  }

  bool LoweringContext::appendLifecycleFunction(const char *GlobalName, ::llvm::Function &FunctionValue)
  {
    if (TargetModule->getNamedValue(GlobalName) != nullptr)
    {
      addFailure<core::DiagnosticKind::LLVMReservedLifecycleGlobalName>(GlobalName);
      return false;
    }
    ::llvm::Type *PointerType = ::llvm::PointerType::getUnqual(Context);
    ::llvm::StructType *EntryType = ::llvm::StructType::get(Context, {::llvm::Type::getInt32Ty(Context), PointerType, PointerType}, false);
    ::llvm::Constant *Priority = ::llvm::ConstantInt::get(::llvm::Type::getInt32Ty(Context), 65535);
    ::llvm::Constant *Data = ::llvm::ConstantPointerNull::get(static_cast<::llvm::PointerType *>(PointerType));
    ::llvm::Constant *Entry = ::llvm::ConstantStruct::get(EntryType, {Priority, &FunctionValue, Data});
    ::llvm::ArrayType *ArrayType = ::llvm::ArrayType::get(EntryType, 1);
    ::llvm::Constant *Initializer = ::llvm::ConstantArray::get(ArrayType, {Entry});
    new ::llvm::GlobalVariable(*TargetModule, ArrayType, false, ::llvm::GlobalValue::AppendingLinkage, Initializer, GlobalName);
    return true;
  }

  ::llvm::CmpInst::Predicate LoweringContext::FunctionLoweringContext::lowerComparePredicate(ir::ComparePredicate Predicate, ir::TypeKind OperandType)
  {
    if (Predicate == ir::ComparePredicate::Equal)
    {
      return ::llvm::CmpInst::ICMP_EQ;
    }
    if (Predicate == ir::ComparePredicate::NotEqual)
    {
      return ::llvm::CmpInst::ICMP_NE;
    }
    const bool Signed = OperandType == ir::TypeKind::I32;
    switch (Predicate)
    {
    case ir::ComparePredicate::LessThan:
      return Signed ? ::llvm::CmpInst::ICMP_SLT : ::llvm::CmpInst::ICMP_ULT;
    case ir::ComparePredicate::LessEqual:
      return Signed ? ::llvm::CmpInst::ICMP_SLE : ::llvm::CmpInst::ICMP_ULE;
    case ir::ComparePredicate::GreaterThan:
      return Signed ? ::llvm::CmpInst::ICMP_SGT : ::llvm::CmpInst::ICMP_UGT;
    case ir::ComparePredicate::GreaterEqual:
      return Signed ? ::llvm::CmpInst::ICMP_SGE : ::llvm::CmpInst::ICMP_UGE;
    case ir::ComparePredicate::Equal:
    case ir::ComparePredicate::NotEqual:
    case ir::ComparePredicate::Count:
      return ::llvm::CmpInst::BAD_ICMP_PREDICATE;
    }
    return ::llvm::CmpInst::BAD_ICMP_PREDICATE;
  }
} // namespace ink::backend::llvm
