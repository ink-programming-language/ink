#include "ink/ir/builder.h"

#include "ink/ir/model/context.h"
#include "ink/ir/model/operand.h"

#include <algorithm>
#include <utility>

namespace ink::ir
{
  IRBuilder::IRBuilder(IRContext &Context) noexcept
      : Context(&Context),
        ModuleValue(Context)
  {
  }

  IRContext &IRBuilder::context() noexcept
  {
    return *Context;
  }

  const IRContext &IRBuilder::context() const noexcept
  {
    return *Context;
  }

  Module &IRBuilder::module() noexcept
  {
    return ModuleValue;
  }

  const Module &IRBuilder::module() const noexcept
  {
    return ModuleValue;
  }

  void IRBuilder::setModuleName(std::optional<Name> NameValue)
  {
    ModuleValue.Name = std::move(NameValue);
  }

  const StructType &IRBuilder::createStructType(Name NameValue, std::vector<StructField> Fields, StructLayoutConstraints LayoutConstraints)
  {
    const StructType &Result = Context->createStructType(std::move(NameValue), std::move(Fields), std::move(LayoutConstraints));
    ModuleValue.StructTypes.push_back(&Result);
    return Result;
  }

  ByteConstantId IRBuilder::addByteConstant(Name NameValue, std::string Data)
  {
    const ByteConstantId Result{ModuleValue.ByteConstants.size()};
    ModuleValue.ByteConstants.push_back({std::move(NameValue), std::move(Data)});
    return Result;
  }

  GlobalId IRBuilder::addGlobal(GlobalVariable GlobalValue)
  {
    const GlobalId Result{ModuleValue.Globals.size()};
    ModuleValue.Globals.push_back(std::move(GlobalValue));
    return Result;
  }

  FunctionId IRBuilder::addFunction(Function FunctionValue)
  {
    const FunctionId Result{ModuleValue.Functions.size()};
    NextValueIds.push_back(findNextValueId(FunctionValue));
    ModuleValue.Functions.push_back(std::move(FunctionValue));
    return Result;
  }

  std::optional<BlockId> IRBuilder::addBlock(FunctionId Function, BasicBlock Block)
  {
    ink::ir::Function *FunctionValue = function(Function);
    if (FunctionValue == nullptr)
    {
      return std::nullopt;
    }
    const BlockId Result{FunctionValue->Blocks.size()};
    for (const std::unique_ptr<Instruction> &InstructionValue : Block.Instructions)
    {
      if (InstructionValue != nullptr)
      {
        updateNextValueId(Function, *InstructionValue);
      }
    }
    FunctionValue->Blocks.push_back(std::move(Block));
    return Result;
  }

  FunctionId IRBuilder::createFunction(Name NameValue, const Type &ResultType, std::vector<Parameter> Parameters, std::vector<Attribute> Attributes)
  {
    Function FunctionValue(ResultType);
    FunctionValue.Name = std::move(NameValue);
    FunctionValue.Parameters = std::move(Parameters);
    FunctionValue.Attributes = std::move(Attributes);
    return addFunction(std::move(FunctionValue));
  }

  std::optional<BlockId> IRBuilder::createBlock(FunctionId Function, Name NameValue)
  {
    BasicBlock BlockValue;
    BlockValue.Name = std::move(NameValue);
    return addBlock(Function, std::move(BlockValue));
  }

  Function *IRBuilder::function(FunctionId Function) noexcept
  {
    return Function.valid() && Function.value() < ModuleValue.Functions.size() ? &ModuleValue.Functions[Function.value()] : nullptr;
  }

  const Function *IRBuilder::function(FunctionId Function) const noexcept
  {
    return Function.valid() && Function.value() < ModuleValue.Functions.size() ? &ModuleValue.Functions[Function.value()] : nullptr;
  }

  BasicBlock *IRBuilder::block(FunctionId Function, BlockId Block) noexcept
  {
    ink::ir::Function *FunctionValue = function(Function);
    return FunctionValue != nullptr && Block.valid() && Block.value() < FunctionValue->Blocks.size() ? &FunctionValue->Blocks[Block.value()] : nullptr;
  }

  const BasicBlock *IRBuilder::block(FunctionId Function, BlockId Block) const noexcept
  {
    const ink::ir::Function *FunctionValue = function(Function);
    return FunctionValue != nullptr && Block.valid() && Block.value() < FunctionValue->Blocks.size() ? &FunctionValue->Blocks[Block.value()] : nullptr;
  }

  bool IRBuilder::setInitializer(std::optional<FunctionId> Function) noexcept
  {
    if (Function.has_value() && this->function(*Function) == nullptr)
    {
      return false;
    }
    ModuleValue.Initializer = Function;
    return true;
  }

  bool IRBuilder::setFinalizer(std::optional<FunctionId> Function) noexcept
  {
    if (Function.has_value() && this->function(*Function) == nullptr)
    {
      return false;
    }
    ModuleValue.Finalizer = Function;
    return true;
  }

  bool IRBuilder::setInsertionPoint(FunctionId Function, BlockId Block) noexcept
  {
    if (this->block(Function, Block) == nullptr)
    {
      return false;
    }
    InsertionFunction = Function;
    InsertionBlock = Block;
    return true;
  }

  void IRBuilder::clearInsertionPoint() noexcept
  {
    InsertionFunction.reset();
    InsertionBlock.reset();
  }

  std::optional<FunctionId> IRBuilder::insertionFunction() const noexcept
  {
    return InsertionFunction;
  }

  std::optional<BlockId> IRBuilder::insertionBlock() const noexcept
  {
    return InsertionBlock;
  }

  std::optional<ValueId> IRBuilder::allocateValueId(FunctionId Function) noexcept
  {
    if (function(Function) == nullptr || Function.value() >= NextValueIds.size())
    {
      return std::nullopt;
    }
    std::size_t &NextValueId = NextValueIds[Function.value()];
    if (NextValueId == InvalidId)
    {
      return std::nullopt;
    }
    return ValueId{NextValueId++};
  }

  std::optional<ValueId> IRBuilder::allocateValueId() noexcept
  {
    return InsertionFunction.has_value() ? allocateValueId(*InsertionFunction) : std::nullopt;
  }

  CallInstruction *IRBuilder::createCall(FunctionId Callee, std::vector<ValueHandle> Arguments)
  {
    const Function *CalleeValue = function(Callee);
    if (CalleeValue == nullptr || CalleeValue->ResultType == nullptr)
    {
      return nullptr;
    }
    for (const ValueHandle &Argument : Arguments)
    {
      if (!Argument)
      {
        return nullptr;
      }
    }
    auto Call = std::make_unique<CallInstruction>(*CalleeValue->ResultType);
    Call->Callee = Callee;
    Call->Arguments = std::move(Arguments);
    return CalleeValue->ResultType->kind() == TypeKind::Void ? insertInstruction(std::move(Call)) : insertValueInstruction(std::move(Call));
  }

  ImportInstruction *IRBuilder::createImport(Name Module)
  {
    return insertInstruction(std::make_unique<ImportInstruction>(std::move(Module)));
  }

  AllocaInstruction *IRBuilder::createAlloca(const Type &ResultType, ValueHandle Size)
  {
    if (!Size)
    {
      return nullptr;
    }
    auto Alloca = std::make_unique<AllocaInstruction>(ResultType);
    Alloca->Size = std::move(Size);
    return insertValueInstruction(std::move(Alloca));
  }

  GetElementPointerInstruction *IRBuilder::createGetElementPointer(const Type &ElementType, ValueHandle Pointer, ValueHandle Index, std::vector<ValueHandle> FieldIndices)
  {
    if (!Pointer || !Index)
    {
      return nullptr;
    }
    for (const ValueHandle &FieldIndex : FieldIndices)
    {
      if (!FieldIndex)
      {
        return nullptr;
      }
    }
    const Type &ResultType = Pointer->type();
    auto GetElementPointer = std::make_unique<GetElementPointerInstruction>(ResultType, ElementType);
    GetElementPointer->Pointer = std::move(Pointer);
    GetElementPointer->Index = std::move(Index);
    GetElementPointer->FieldIndices = std::move(FieldIndices);
    return insertValueInstruction(std::move(GetElementPointer));
  }

  LoadInstruction *IRBuilder::createLoad(const Type &ResultType, ValueHandle Pointer)
  {
    if (!Pointer)
    {
      return nullptr;
    }
    auto Load = std::make_unique<LoadInstruction>(ResultType);
    Load->Pointer = std::move(Pointer);
    return insertValueInstruction(std::move(Load));
  }

  StoreInstruction *IRBuilder::createStore(ValueHandle StoredValue, ValueHandle Pointer)
  {
    if (!StoredValue || !Pointer)
    {
      return nullptr;
    }
    auto Store = std::make_unique<StoreInstruction>();
    Store->StoredValue = std::move(StoredValue);
    Store->Pointer = std::move(Pointer);
    return insertInstruction(std::move(Store));
  }

  LifetimeEndInstruction *IRBuilder::createLifetimeEnd(ValueHandle Slice)
  {
    if (!Slice)
    {
      return nullptr;
    }
    auto LifetimeEnd = std::make_unique<LifetimeEndInstruction>();
    LifetimeEnd->Slice = std::move(Slice);
    return insertInstruction(std::move(LifetimeEnd));
  }

  SliceDataInstruction *IRBuilder::createSliceData(ValueHandle Slice)
  {
    if (!Slice)
    {
      return nullptr;
    }
    const Type *ResultType = nullptr;
    if (Slice->type().kind() == TypeKind::ByteSlice)
    {
      ResultType = &context().getType(TypeKind::BytePointer);
    }
    else if (Slice->type().kind() == TypeKind::ConstByteSlice)
    {
      ResultType = &context().getType(TypeKind::ConstBytePointer);
    }
    else
    {
      return nullptr;
    }
    auto SliceData = std::make_unique<SliceDataInstruction>(*ResultType);
    SliceData->Slice = std::move(Slice);
    return insertValueInstruction(std::move(SliceData));
  }

  SliceLengthInstruction *IRBuilder::createSliceLength(ValueHandle Slice)
  {
    if (!Slice || (Slice->type().kind() != TypeKind::ByteSlice && Slice->type().kind() != TypeKind::ConstByteSlice))
    {
      return nullptr;
    }
    auto SliceLength = std::make_unique<SliceLengthInstruction>(context().getType(TypeKind::PointerSize));
    SliceLength->Slice = std::move(Slice);
    return insertValueInstruction(std::move(SliceLength));
  }

  PhiInstruction *IRBuilder::createPhi(const Type &ResultType, std::vector<PhiIncoming> IncomingValues)
  {
    if (!hasValidInsertionPoint())
    {
      return nullptr;
    }
    for (const PhiIncoming &Incoming : IncomingValues)
    {
      if (!Incoming.Value || block(*InsertionFunction, Incoming.Predecessor) == nullptr)
      {
        return nullptr;
      }
    }
    auto Phi = std::make_unique<PhiInstruction>(ResultType);
    Phi->IncomingValues = std::move(IncomingValues);
    return insertValueInstruction(std::move(Phi));
  }

  AddInstruction *IRBuilder::createAdd(ValueHandle Left, ValueHandle Right)
  {
    if (!Left || !Right)
    {
      return nullptr;
    }
    const Type &ResultType = Left->type();
    auto Add = std::make_unique<AddInstruction>(ResultType);
    Add->Left = std::move(Left);
    Add->Right = std::move(Right);
    return insertValueInstruction(std::move(Add));
  }

  CompareInstruction *IRBuilder::createCompare(ComparePredicate Predicate, ValueHandle Left, ValueHandle Right)
  {
    if (!Left || !Right)
    {
      return nullptr;
    }
    auto Compare = std::make_unique<CompareInstruction>(context().getType(TypeKind::Bool));
    Compare->Predicate = Predicate;
    Compare->Left = std::move(Left);
    Compare->Right = std::move(Right);
    return insertValueInstruction(std::move(Compare));
  }

  InsertValueInstruction *IRBuilder::createInsertValue(ValueHandle Aggregate, ValueHandle Element, std::size_t FieldIndex)
  {
    if (!Aggregate || !Element)
    {
      return nullptr;
    }
    const Type &ResultType = Aggregate->type();
    auto Insert = std::make_unique<InsertValueInstruction>(ResultType);
    Insert->Aggregate = std::move(Aggregate);
    Insert->Element = std::move(Element);
    Insert->FieldIndex = FieldIndex;
    return insertValueInstruction(std::move(Insert));
  }

  ExtractValueInstruction *IRBuilder::createExtractValue(ValueHandle Aggregate, std::size_t FieldIndex)
  {
    if (!Aggregate || Aggregate->type().kind() != TypeKind::Struct)
    {
      return nullptr;
    }
    const StructType &AggregateType = static_cast<const StructType &>(Aggregate->type());
    if (FieldIndex >= AggregateType.fieldCount() || AggregateType.fieldType(FieldIndex) == nullptr)
    {
      return nullptr;
    }
    auto Extract = std::make_unique<ExtractValueInstruction>(*AggregateType.fieldType(FieldIndex));
    Extract->Aggregate = std::move(Aggregate);
    Extract->FieldIndex = FieldIndex;
    return insertValueInstruction(std::move(Extract));
  }

  BranchInstruction *IRBuilder::createBranch(BlockId Target)
  {
    if (!hasValidInsertionPoint() || block(*InsertionFunction, Target) == nullptr)
    {
      return nullptr;
    }
    auto Branch = std::make_unique<BranchInstruction>();
    Branch->Target.Block = Target;
    return insertInstruction(std::move(Branch));
  }

  ConditionalBranchInstruction *IRBuilder::createConditionalBranch(ValueHandle Condition, BlockId TrueTarget, BlockId FalseTarget)
  {
    if (!Condition || !hasValidInsertionPoint() || block(*InsertionFunction, TrueTarget) == nullptr || block(*InsertionFunction, FalseTarget) == nullptr)
    {
      return nullptr;
    }
    auto Branch = std::make_unique<ConditionalBranchInstruction>();
    Branch->Condition = std::move(Condition);
    Branch->TrueTarget.Block = TrueTarget;
    Branch->FalseTarget.Block = FalseTarget;
    return insertInstruction(std::move(Branch));
  }

  ReturnInstruction *IRBuilder::createReturn()
  {
    return insertInstruction(std::make_unique<ReturnInstruction>());
  }

  ReturnInstruction *IRBuilder::createReturn(ValueHandle ReturnValue)
  {
    if (!ReturnValue)
    {
      return nullptr;
    }
    auto Return = std::make_unique<ReturnInstruction>();
    Return->ReturnValue = std::move(ReturnValue);
    return insertInstruction(std::move(Return));
  }

  bool IRBuilder::hasValidInsertionPoint() const noexcept
  {
    return InsertionFunction.has_value() && InsertionBlock.has_value() && block(*InsertionFunction, *InsertionBlock) != nullptr;
  }

  Instruction *IRBuilder::insertInstruction(std::unique_ptr<Instruction> InstructionValue)
  {
    if (InstructionValue == nullptr || !InsertionFunction.has_value() || !InsertionBlock.has_value())
    {
      return nullptr;
    }
    BasicBlock *BlockValue = block(*InsertionFunction, *InsertionBlock);
    if (BlockValue == nullptr)
    {
      return nullptr;
    }
    Instruction *Result = InstructionValue.get();
    updateNextValueId(*InsertionFunction, *InstructionValue);
    BlockValue->Instructions.push_back(std::move(InstructionValue));
    return Result;
  }

  ValueHandle IRBuilder::createValueOperand(const Type &ValueType, ValueId Value) const
  {
    return ValueHandle(std::make_unique<ValueOperand>(ValueType, Value));
  }

  ValueHandle IRBuilder::createGlobalAddress(const Type &ValueType, ByteConstantId Constant, std::size_t ByteOffset) const
  {
    return ValueHandle(std::make_unique<GlobalAddressOperand>(ValueType, Constant, ByteOffset));
  }

  ValueHandle IRBuilder::createGlobalVariableAddress(const Type &ValueType, GlobalId Global) const
  {
    return ValueHandle(std::make_unique<GlobalVariableAddressOperand>(ValueType, Global));
  }

  const FloatConstant &IRBuilder::getFloatConstant(const Type &ValueType, FloatFormat Format, std::uint64_t BitPattern)
  {
    return context().constantPool().getFloatConstant(ValueType, Format, BitPattern);
  }

  const StringConstant &IRBuilder::getStringConstant(const Type &ValueType, std::string Data)
  {
    return context().constantPool().getStringConstant(ValueType, std::move(Data));
  }

  const NullConstant &IRBuilder::getNullConstant(const Type &ValueType)
  {
    return context().constantPool().getNullConstant(ValueType);
  }

  const ZeroInitializer &IRBuilder::getZeroInitializer(const Type &ValueType)
  {
    return context().constantPool().getZeroInitializer(ValueType);
  }

  const AggregateConstant &IRBuilder::getAggregateConstant(const Type &ValueType, const std::vector<std::reference_wrapper<const Constant>> &Elements)
  {
    return context().constantPool().getAggregateConstant(ValueType, Elements);
  }

  Module IRBuilder::takeModule()
  {
    Module Result = std::move(ModuleValue);
    reset();
    return Result;
  }

  void IRBuilder::reset()
  {
    ModuleValue = Module(*Context);
    NextValueIds.clear();
    clearInsertionPoint();
  }

  void IRBuilder::updateNextValueId(FunctionId Function, const Instruction &InstructionValue) noexcept
  {
    if (!Function.valid() || Function.value() >= NextValueIds.size())
    {
      return;
    }
    const std::optional<ValueId> Result = instructionResultId(InstructionValue);
    if (Result.has_value() && Result->valid() && Result->value() != InvalidId)
    {
      NextValueIds[Function.value()] = std::max(NextValueIds[Function.value()], Result->value() + 1);
    }
  }

  std::size_t IRBuilder::findNextValueId(const Function &FunctionValue) const noexcept
  {
    std::size_t Result = FunctionValue.Parameters.size();
    for (const BasicBlock &BlockValue : FunctionValue.Blocks)
    {
      for (const std::unique_ptr<Instruction> &InstructionValue : BlockValue.Instructions)
      {
        if (InstructionValue == nullptr)
        {
          continue;
        }
        const std::optional<ValueId> InstructionResult = instructionResultId(*InstructionValue);
        if (InstructionResult.has_value() && InstructionResult->valid() && InstructionResult->value() != InvalidId)
        {
          Result = std::max(Result, InstructionResult->value() + 1);
        }
      }
    }
    return Result;
  }
} // namespace ink::ir
