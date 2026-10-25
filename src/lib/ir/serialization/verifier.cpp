#include "ink/ir/serialization/serialization.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/type/structure_type/reference_type.h"

#include <algorithm>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    class ModuleVerifier;

    struct InstructionPosition
    {
        std::size_t Block;
        std::size_t Index;
    };

    bool supportedInstruction(const Instruction &Value)
    {
      switch (Value.kind())
      {
      case ValueKind::AllocaInstruction:
        return dynamic_cast<const AllocaInstruction *>(&Value) != nullptr;
      case ValueKind::LoadInstruction:
        return dynamic_cast<const LoadInstruction *>(&Value) != nullptr;
      case ValueKind::StoreInstruction:
        return dynamic_cast<const StoreInstruction *>(&Value) != nullptr;
      case ValueKind::GetElementPtrInstruction:
        return dynamic_cast<const GetElementPtrInstruction *>(&Value) != nullptr;
      case ValueKind::NegInstruction:
        return dynamic_cast<const NegInstruction *>(&Value) != nullptr;
      case ValueKind::ICmpInstruction:
        return dynamic_cast<const ICmpInstruction *>(&Value) != nullptr;
      case ValueKind::CallInstruction:
        return dynamic_cast<const CallInstruction *>(&Value) != nullptr;
      case ValueKind::BranchInstruction:
        return dynamic_cast<const BranchInstruction *>(&Value) != nullptr;
      case ValueKind::ConditionalBranchInstruction:
        return dynamic_cast<const ConditionalBranchInstruction *>(&Value) != nullptr;
      case ValueKind::ReturnInstruction:
        return dynamic_cast<const ReturnInstruction *>(&Value) != nullptr;
      default:
        return false;
      }
    }

    class FunctionVerifier final
    {
      public:
        FunctionVerifier(ModuleVerifier &Owner, const Function &Callable);
        bool verify();

      private:
        bool operand(const Value *Value, const InstructionPosition &Use);
        bool canUseAs(const Value *Value, const Type *Expected, const InstructionPosition &Use);
        const Type *memoryPointee(const Value *Value, const InstructionPosition &Use);
        bool verifyInstruction(const Instruction &Value, const InstructionPosition &Position);
        bool verifyGEP(const GetElementPtrInstruction &Value, const InstructionPosition &Position);
        bool dominates(const InstructionPosition &Definition, const InstructionPosition &Use) const;
        void buildDominators();
        std::size_t intersect(std::size_t Left, std::size_t Right) const;
        bool addSuccessor(std::size_t Source, const BasicBlock *Target);

        ModuleVerifier &Owner;
        const Function &Callable;
        std::unordered_map<const BasicBlock *, std::size_t> Blocks;
        std::unordered_map<const Instruction *, InstructionPosition> Instructions;
        std::vector<std::vector<std::size_t>> Successors;
        std::vector<std::vector<std::size_t>> Predecessors;
        std::vector<std::size_t> Order;
        std::vector<std::size_t> DominatorEnd;
        std::vector<std::size_t> ImmediateDominator;
        std::unordered_set<const AllocaInstruction *> ConstantInitializations;
        static constexpr std::size_t Missing = std::numeric_limits<std::size_t>::max();
    };

    class ModuleVerifier final
    {
      public:
        explicit ModuleVerifier(const Module &ModuleValue)
            : ModuleValue(ModuleValue),
              Context(ModuleValue.irContext())
        {
        }

        bool reject(std::string Message, IRSerializationErrorCode Code = IRSerializationErrorCode::InvalidIR)
        {
          if (Error.empty())
          {
            Error = std::move(Message);
            ErrorCode = Code;
          }
          return false;
        }

        bool validName(core::Name Name, bool Required = false) const
        {
          return Name.valid() ? core::NamePool::instance().contains(Name) : !Required;
        }

        bool validModuleID(const ModuleID &ID, bool Required = false) const
        {
          if (Required && ID.Components.empty())
          {
            return false;
          }
          return std::all_of(ID.Components.begin(), ID.Components.end(), [this](core::Name Name)
                             {
                               return validName(Name, true);
                             });
        }

        bool type(const Type *Value)
        {
          if (CheckedTypes.contains(Value))
          {
            return true;
          }
          std::vector<const Type *> Pending{Value};
          while (!Pending.empty())
          {
            const Type *Current = Pending.back();
            Pending.pop_back();
            if (!Context.containsType(Current))
            {
              return reject("IR references a type outside its context");
            }
            if (!CheckedTypes.insert(Current).second)
            {
              continue;
            }
            switch (Current->kind())
            {
            case ValueKind::MetaType:
            case ValueKind::Variadic:
            case ValueKind::Void:
            case ValueKind::Bool:
            case ValueKind::NullPointer:
            case ValueKind::Integer:
            case ValueKind::Float:
              break;
            case ValueKind::Pointer:
              Pending.push_back(static_cast<const PointerType *>(Current)->pointeeType());
              break;
            case ValueKind::Reference:
              Pending.push_back(static_cast<const ReferenceType *>(Current)->referencedType());
              break;
            case ValueKind::Array:
              Pending.push_back(static_cast<const ArrayType *>(Current)->elementType());
              break;
            case ValueKind::FunctionType:
            {
              const auto &Signature = static_cast<const FunctionType &>(*Current);
              Pending.push_back(Signature.returnType());
              for (const auto &Parameter : Signature.parameters())
              {
                Pending.push_back(Parameter.ParameterType);
              }
              break;
            }
            case ValueKind::Class:
            case ValueKind::Struct:
            case ValueKind::Interface:
              if (!Nominals.contains(static_cast<const NominalType *>(Current)->declarationType()))
              {
                return reject("IR references a nominal type owned by another module");
              }
              break;
            default:
              return reject("IR contains an unsupported type kind", IRSerializationErrorCode::UnsupportedKind);
            }
          }
          return true;
        }

        bool constant(const Constant *Value)
        {
          if (CheckedConstants.contains(Value))
          {
            return true;
          }
          std::vector<const Constant *> Pending{Value};
          while (!Pending.empty())
          {
            const Constant *Current = Pending.back();
            Pending.pop_back();
            if (!Context.constantPool().owns(Current) || !type(Current->type()))
            {
              return reject("IR references a constant outside its context");
            }
            if (!CheckedConstants.insert(Current).second)
            {
              continue;
            }
            if (ArrayConstant::classof(Current))
            {
              for (const Constant *Element : static_cast<const ArrayConstant *>(Current)->elements())
              {
                Pending.push_back(Element);
              }
            }
          }
          return true;
        }

        bool verify()
        {
          if (!validModuleID(ModuleValue.moduleID()))
          {
            return reject("Invalid module identifier");
          }
          for (const auto &Value : ModuleValue.classes())
          {
            Nominals.insert(Value.get());
          }
          for (const auto &Value : ModuleValue.structs())
          {
            Nominals.insert(Value.get());
          }
          for (const auto &Value : ModuleValue.interfaces())
          {
            Nominals.insert(Value.get());
          }
          for (const auto &Value : ModuleValue.globalFunctions())
          {
            Functions.insert(Value.get());
          }
          for (const NominalType *Value : Nominals)
          {
            if (!validName(Value->name()) || !type(Value))
            {
              return reject("Invalid nominal declaration");
            }
            for (const Field &Entry : Value->fields())
            {
              if (!validName(Entry.name(), true) || !type(Entry.type()) || !Entry.valid() || Entry.offset() > Value->size() || Entry.type()->size() > Value->size() - Entry.offset() || (Entry.initializer() && !constant(Entry.initializer())))
              {
                return reject("Invalid nominal field");
              }
            }
            for (const Method &Entry : Value->methods())
            {
              if (!Entry.valid() || !Functions.insert(Entry.function()).second)
              {
                return reject("Invalid nominal method");
              }
            }
          }
          for (const auto &Value : ModuleValue.globalVariables())
          {
            if (!Value || !validName(Value->name()) || !type(Value->type()) || !Value->variableType()->hasLayout() || (Value->initializer() && (!constant(Value->initializer()) || !Value->initializer()->canInitialize(Value->variableType()))))
            {
              return reject("Invalid global variable");
            }
            Globals.insert(Value.get());
          }
          for (const SymbolImport &Entry : ModuleValue.imports())
          {
            if (!validModuleID(Entry.Target.Module, true) || !validName(Entry.Target.Symbol.SymbolName, true) || !type(Entry.SymbolType))
            {
              return reject("Invalid symbol import");
            }
          }
          for (const SymbolExport &Group : ModuleValue.exports())
          {
            if (!validName(Group.Symbol.SymbolName, true) || Group.Entries.empty())
            {
              return reject("Invalid symbol export group");
            }
            for (const SymbolExportEntry &Entry : Group.Entries)
            {
              if (!Entry.Declaration || !ModuleValue.owns(Entry.Declaration) || !type(Entry.SymbolType) || Entry.Declaration->type() != Entry.SymbolType || (Type::classof(Entry.Declaration) && !type(static_cast<const Type *>(Entry.Declaration))))
              {
                return reject("Invalid symbol export declaration");
              }
            }
          }
          for (const Function *Entry : {ModuleValue.initializationFunction(), ModuleValue.deinitializationFunction()})
          {
            if (Entry && (!ModuleValue.owns(Entry) || Entry->parameterCount() != 0 || !VoidType::classof(Entry->functionType()->returnType())))
            {
              return reject("Invalid module lifecycle function");
            }
          }
          for (const Function *Value : Functions)
          {
            if (!FunctionVerifier(*this, *Value).verify())
            {
              return false;
            }
          }
          return true;
        }

        const Module &ModuleValue;
        const IRContext &Context;
        std::unordered_set<const Function *> Functions;
        std::unordered_set<const GlobalVariable *> Globals;
        std::unordered_set<const NominalType *> Nominals;
        std::string Error;
        IRSerializationErrorCode ErrorCode = IRSerializationErrorCode::InvalidIR;

      private:
        std::unordered_set<const Type *> CheckedTypes;
        std::unordered_set<const Constant *> CheckedConstants;
    };

    FunctionVerifier::FunctionVerifier(ModuleVerifier &Owner, const Function &Callable)
        : Owner(Owner),
          Callable(Callable)
    {
    }

    bool FunctionVerifier::addSuccessor(std::size_t Source, const BasicBlock *Target)
    {
      const auto Found = Blocks.find(Target);
      if (Found == Blocks.end())
      {
        return Owner.reject("Branch target does not belong to the current function");
      }
      Successors[Source].push_back(Found->second);
      Predecessors[Found->second].push_back(Source);
      return true;
    }

    std::size_t FunctionVerifier::intersect(std::size_t Left, std::size_t Right) const
    {
      while (Left != Right)
      {
        while (Order[Left] > Order[Right])
        {
          Left = ImmediateDominator[Left];
        }
        while (Order[Right] > Order[Left])
        {
          Right = ImmediateDominator[Right];
        }
      }
      return Left;
    }

    void FunctionVerifier::buildDominators()
    {
      const std::size_t Count = Callable.blockCount();
      Order.assign(Count, Missing);
      ImmediateDominator.assign(Count, Missing);
      std::vector<bool> Seen(Count, false);
      std::vector<std::pair<std::size_t, std::size_t>> Stack{{0, 0}};
      std::vector<std::size_t> Postorder;
      Seen[0] = true;
      while (!Stack.empty())
      {
        const std::size_t Block = Stack.back().first;
        auto &Next = Stack.back().second;
        if (Next < Successors[Block].size())
        {
          const std::size_t Target = Successors[Block][Next++];
          if (!Seen[Target])
          {
            Seen[Target] = true;
            Stack.emplace_back(Target, 0);
          }
        }
        else
        {
          Postorder.push_back(Block);
          Stack.pop_back();
        }
      }
      std::reverse(Postorder.begin(), Postorder.end());
      for (std::size_t Index = 0; Index < Postorder.size(); ++Index)
      {
        Order[Postorder[Index]] = Index;
      }
      ImmediateDominator[0] = 0;
      bool Changed = true;
      while (Changed)
      {
        Changed = false;
        for (std::size_t Index = 1; Index < Postorder.size(); ++Index)
        {
          const std::size_t Block = Postorder[Index];
          std::size_t Dominator = Missing;
          for (std::size_t Predecessor : Predecessors[Block])
          {
            if (ImmediateDominator[Predecessor] != Missing)
            {
              Dominator = Dominator == Missing ? Predecessor : intersect(Dominator, Predecessor);
            }
          }
          if (ImmediateDominator[Block] != Dominator)
          {
            ImmediateDominator[Block] = Dominator;
            Changed = true;
          }
        }
      }
      std::vector<std::vector<std::size_t>> Children(Count);
      for (std::size_t Index = 1; Index < Count; ++Index)
      {
        if (ImmediateDominator[Index] != Missing)
        {
          Children[ImmediateDominator[Index]].push_back(Index);
        }
      }
      DominatorEnd.assign(Count, Missing);
      Stack.emplace_back(0, 0);
      std::size_t NextOrder = 0;
      Order[0] = NextOrder++;
      while (!Stack.empty())
      {
        const std::size_t Block = Stack.back().first;
        auto &Next = Stack.back().second;
        if (Next < Children[Block].size())
        {
          const std::size_t Child = Children[Block][Next++];
          Order[Child] = NextOrder++;
          Stack.emplace_back(Child, 0);
        }
        else
        {
          DominatorEnd[Block] = NextOrder;
          Stack.pop_back();
        }
      }
    }

    bool FunctionVerifier::dominates(const InstructionPosition &Definition, const InstructionPosition &Use) const
    {
      if (Definition.Block == Use.Block)
      {
        return Definition.Index < Use.Index;
      }
      // No entry-to-use path exists for an unreachable block, so cross-block
      // dominance is vacuous there. Local instruction order is still checked.
      if (Order[Use.Block] == Missing)
      {
        return true;
      }
      if (Order[Definition.Block] == Missing)
      {
        return false;
      }
      return Order[Definition.Block] <= Order[Use.Block] && Order[Use.Block] < DominatorEnd[Definition.Block];
    }

    bool FunctionVerifier::operand(const Value *Value, const InstructionPosition &Use)
    {
      if (!Value || !Owner.type(Value->type()) || !Value->type()->hasLayout())
      {
        return Owner.reject("Instruction has an invalid operand type");
      }
      if (Constant::classof(Value))
      {
        return Owner.constant(dynamic_cast<const Constant *>(Value));
      }
      if (Parameter::classof(Value))
      {
        const auto *ParameterValue = dynamic_cast<const Parameter *>(Value);
        return (ParameterValue && ParameterValue->function() == &Callable && !ParameterValue->isVariadic()) || Owner.reject("Operand is a foreign or variadic parameter");
      }
      if (Instruction::classof(Value))
      {
        const auto *InstructionValue = dynamic_cast<const Instruction *>(Value);
        const auto Found = Instructions.find(InstructionValue);
        return (Found != Instructions.end() && !InstructionValue->isTerminator() && !StoreInstruction::classof(Value) && dominates(Found->second, Use)) || Owner.reject("Instruction operand is foreign, does not produce a value, or does not dominate its use");
      }
      return (Function::classof(Value) && Owner.Functions.contains(dynamic_cast<const Function *>(Value))) || (GlobalVariable::classof(Value) && Owner.Globals.contains(dynamic_cast<const GlobalVariable *>(Value))) || Owner.reject("Unsupported or foreign instruction operand");
    }

    bool FunctionVerifier::canUseAs(const Value *Value, const Type *Expected, const InstructionPosition &Use)
    {
      if (!Owner.type(Expected) || !operand(Value, Use))
      {
        return false;
      }
      return (Constant::classof(Value) ? static_cast<const Constant *>(Value)->canInitialize(Expected) : Value->type()->unqualifiedType() == Expected->unqualifiedType()) || Owner.reject("Instruction operand type does not match its expected type");
    }

    const Type *FunctionVerifier::memoryPointee(const Value *Value, const InstructionPosition &Use)
    {
      if (!operand(Value, Use) || !PointerType::classof(Value->type()))
      {
        return nullptr;
      }
      const auto &Pointer = static_cast<const PointerType &>(*Value->type());
      const Type *Pointee = Pointer.pointeeType();
      return !Pointer.isOpaque() && Owner.type(Pointee) && Pointee->hasLayout() ? Pointee : nullptr;
    }

    bool FunctionVerifier::verifyGEP(const GetElementPtrInstruction &Value, const InstructionPosition &Position)
    {
      const Type *Selected = Value.sourceElementType();
      if (!Owner.type(Selected) || !Selected->hasLayout() || VoidType::classof(Selected) || !operand(Value.pointer(), Position) || !PointerType::classof(Value.type()) || !Value.type()->isMutable())
      {
        return Owner.reject("Invalid getelementptr source or result type");
      }
      if (Value.pointer() != Owner.Context.constantPool().get<NullPointerConstant>() && (!PointerType::classof(Value.pointer()->type()) || static_cast<const PointerType *>(Value.pointer()->type())->pointeeType() != Selected))
      {
        return Owner.reject("Getelementptr source does not match its base pointer");
      }
      bool Mutable = Selected->isMutable();
      for (std::size_t Index = 0; Index < Value.indexCount(); ++Index)
      {
        const ir::Value *IndexValue = Value.index(Index);
        if (!operand(IndexValue, Position) || !IntegerType::classof(IndexValue->type()))
        {
          return Owner.reject("Getelementptr index must be an integer value");
        }
        if (Index == 0)
        {
          continue;
        }
        if (ArrayType::classof(Selected))
        {
          Selected = static_cast<const ArrayType *>(Selected)->elementType();
        }
        else if (NominalType::classof(Selected) && IntegerConstant::classof(IndexValue))
        {
          const auto FieldIndex = static_cast<const IntegerConstant *>(IndexValue)->tryGetNonNegativeUInt64();
          const auto &Aggregate = static_cast<const NominalType &>(*Selected);
          if (!FieldIndex || *FieldIndex >= Aggregate.fieldCount())
          {
            return Owner.reject("Getelementptr field index is outside the nominal type");
          }
          Selected = Aggregate.field(static_cast<std::size_t>(*FieldIndex))->type();
        }
        else
        {
          return Owner.reject("Getelementptr index does not select an aggregate element");
        }
        if (!Owner.type(Selected) || !Selected->hasLayout())
        {
          return false;
        }
        Mutable = Mutable && Selected->isMutable();
      }
      const auto *Result = static_cast<const PointerType *>(Value.type());
      return (Result->pointeeType()->unqualifiedType() == Selected->unqualifiedType() && Result->pointeeType()->isMutable() == Mutable) || Owner.reject("Getelementptr result does not preserve the selected type and qualifiers");
    }

    bool FunctionVerifier::verifyInstruction(const Instruction &Value, const InstructionPosition &Position)
    {
      if (!Owner.type(Value.type()) || !Value.type()->hasLayout())
      {
        return Owner.reject("Instruction has an invalid result type");
      }
      const auto *Void = Owner.Context.typePool().get<VoidType>();
      switch (Value.kind())
      {
      case ValueKind::AllocaInstruction:
      {
        const auto &Storage = static_cast<const AllocaInstruction &>(Value);
        return (PointerType::classof(Storage.type()) && Storage.type()->isMutable() && Owner.type(Storage.allocatedType()) && Storage.allocatedType()->hasLayout() && !VoidType::classof(Storage.allocatedType()) && Owner.validName(Storage.name())) || Owner.reject("Invalid allocation type or name");
      }
      case ValueKind::LoadInstruction:
      {
        const auto &Load = static_cast<const LoadInstruction &>(Value);
        const Type *Pointee = memoryPointee(Load.pointer(), Position);
        return (Pointee && Value.type() == Pointee->unqualifiedType()) || Owner.reject("Load result does not match its pointee type");
      }
      case ValueKind::StoreInstruction:
      {
        const auto &Store = static_cast<const StoreInstruction &>(Value);
        const Type *Pointee = memoryPointee(Store.pointer(), Position);
        if (Value.type() != Void || !Pointee || !canUseAs(Store.storedValue(), Pointee, Position))
        {
          return Owner.reject("Invalid store operands or result type");
        }
        if (!Pointee->isMutable())
        {
          if (!AllocaInstruction::classof(Store.pointer()))
          {
            return Owner.reject("Store writes through a const pointer");
          }
          const auto *Storage = static_cast<const AllocaInstruction *>(Store.pointer());
          if (!Storage->isInitialized() || !ConstantInitializations.insert(Storage).second)
          {
            return Owner.reject("Const local initialization is unmarked or repeated");
          }
        }
        return true;
      }
      case ValueKind::GetElementPtrInstruction:
        return verifyGEP(static_cast<const GetElementPtrInstruction &>(Value), Position);
      case ValueKind::NegInstruction:
      {
        const auto &Negation = static_cast<const NegInstruction &>(Value);
        return (operand(Negation.operand(), Position) && (IntegerType::classof(Negation.operand()->type()) || FloatType::classof(Negation.operand()->type())) && Value.type() == Negation.operand()->type()->unqualifiedType()) || Owner.reject("Negation requires a numeric operand and matching result");
      }
      case ValueKind::ICmpInstruction:
      {
        const auto &Comparison = static_cast<const ICmpInstruction &>(Value);
        if (Value.type() != Owner.Context.typePool().get<BoolType>() || static_cast<unsigned>(Comparison.predicate()) > static_cast<unsigned>(ICmpPredicate::SignedLessEqual) || !operand(Comparison.left(), Position) || !operand(Comparison.right(), Position))
        {
          return Owner.reject("Invalid integer comparison");
        }
        const Type *Left = Comparison.left()->type()->unqualifiedType();
        const Type *Right = Comparison.right()->type()->unqualifiedType();
        if (Left == Right && (IntegerType::classof(Left) || PointerType::classof(Left)))
        {
          return true;
        }
        const bool Equality = Comparison.predicate() == ICmpPredicate::Equal || Comparison.predicate() == ICmpPredicate::NotEqual;
        const auto *Null = Owner.Context.constantPool().get<NullPointerConstant>();
        return (Equality && ((PointerType::classof(Left) && Comparison.right() == Null) || (PointerType::classof(Right) && Comparison.left() == Null))) || Owner.reject("Comparison operand types are incompatible");
      }
      case ValueKind::CallInstruction:
      {
        const auto &Call = static_cast<const CallInstruction &>(Value);
        const FunctionType *Signature = Call.functionType();
        if (!Owner.type(Signature) || !operand(Call.callee(), Position) || Call.callee()->type()->unqualifiedType() != Signature->unqualifiedType() || Call.argumentCount() != Signature->parameterCount() || Value.type() != Signature->returnType()->unqualifiedType())
        {
          return Owner.reject("Call signature, callee, arity or result type is invalid");
        }
        for (std::size_t Index = 0; Index < Call.argumentCount(); ++Index)
        {
          const auto &Parameter = Signature->parameters()[Index];
          if (Parameter.Variadic || !Owner.validName(Call.argument(Index)->name()) || !canUseAs(Call.argument(Index)->value(), Parameter.ParameterType, Position))
          {
            return Owner.reject("Invalid or unsupported variadic call argument");
          }
        }
        return true;
      }
      case ValueKind::BranchInstruction:
        return Value.type() == Void || Owner.reject("Branch result must be void");
      case ValueKind::ConditionalBranchInstruction:
        return (Value.type() == Void && canUseAs(static_cast<const ConditionalBranchInstruction &>(Value).condition(), Owner.Context.typePool().get<BoolType>(), Position)) || Owner.reject("Conditional branch requires a bool condition and void result");
      case ValueKind::ReturnInstruction:
      {
        const auto &Return = static_cast<const ReturnInstruction &>(Value);
        const Type *Expected = Callable.functionType()->returnType();
        return (Value.type() == Void && (VoidType::classof(Expected) ? !Return.hasReturnedValue() : canUseAs(Return.returnedValue(), Expected, Position))) || Owner.reject("Return value does not match its function signature");
      }
      default:
        return Owner.reject("Unsupported instruction kind", IRSerializationErrorCode::UnsupportedKind);
      }
    }

    bool FunctionVerifier::verify()
    {
      if (!Owner.validName(Callable.name()) || !Owner.type(Callable.functionType()) || static_cast<unsigned>(Callable.symbolLinkage()) > static_cast<unsigned>(SymbolLinkage::Export) || static_cast<unsigned>(Callable.languageLinkage()) > static_cast<unsigned>(LanguageLinkage::C) || static_cast<unsigned>(Callable.visibility()) > static_cast<unsigned>(FunctionVisibility::Private))
      {
        return Owner.reject("Invalid function declaration");
      }
      for (const auto &Parameter : Callable.parameters())
      {
        if (!Parameter || !Owner.validName(Parameter->name()) || !Owner.type(Parameter->type()))
        {
          return Owner.reject("Invalid function parameter");
        }
      }
      if (Callable.isDeclaration())
      {
        return true;
      }
      Successors.resize(Callable.blockCount());
      Predecessors.resize(Callable.blockCount());
      for (std::size_t Index = 0; Index < Callable.blockCount(); ++Index)
      {
        const BasicBlock *Block = Callable.block(Index);
        if (!Block || Block->function() != &Callable || !Owner.validName(Block->name()) || !Blocks.emplace(Block, Index).second)
        {
          return Owner.reject("Invalid function block");
        }
      }
      for (std::size_t BlockIndex = 0; BlockIndex < Callable.blockCount(); ++BlockIndex)
      {
        const BasicBlock &Block = *Callable.block(BlockIndex);
        if (Block.instructionCount() == 0)
        {
          return Owner.reject("Function block has no terminator");
        }
        for (std::size_t Index = 0; Index < Block.instructionCount(); ++Index)
        {
          const Instruction *Value = Block.instruction(Index);
          if (Value && !supportedInstruction(*Value))
          {
            return Owner.reject("Unsupported instruction class or kind", IRSerializationErrorCode::UnsupportedKind);
          }
          if (!Value || Value->parentBlock() != &Block || !Instructions.emplace(Value, InstructionPosition{BlockIndex, Index}).second || Value->isTerminator() != (Index + 1 == Block.instructionCount()))
          {
            return Owner.reject("Every block must end in exactly one terminator");
          }
        }
        const Instruction *Terminator = Block.instruction(Block.instructionCount() - 1);
        if (BranchInstruction::classof(Terminator))
        {
          if (!addSuccessor(BlockIndex, static_cast<const BranchInstruction *>(Terminator)->target()))
          {
            return false;
          }
        }
        else if (ConditionalBranchInstruction::classof(Terminator))
        {
          const auto &Branch = static_cast<const ConditionalBranchInstruction &>(*Terminator);
          if (!addSuccessor(BlockIndex, Branch.trueTarget()) || !addSuccessor(BlockIndex, Branch.falseTarget()))
          {
            return false;
          }
        }
      }
      buildDominators();
      for (std::size_t BlockIndex = 0; BlockIndex < Callable.blockCount(); ++BlockIndex)
      {
        const BasicBlock &Block = *Callable.block(BlockIndex);
        for (std::size_t Index = 0; Index < Block.instructionCount(); ++Index)
        {
          if (!verifyInstruction(*Block.instruction(Index), {BlockIndex, Index}))
          {
            return false;
          }
        }
      }
      return true;
    }
  } // namespace

  IRSerializationResult<void> IRVerifier::verify(const Module &ModuleValue)
  {
    ModuleVerifier Verifier(ModuleValue);
    if (!Verifier.verify())
    {
      return std::unexpected(IRSerializationError{Verifier.ErrorCode, 0, std::move(Verifier.Error)});
    }
    return {};
  }
} // namespace ink::ir
