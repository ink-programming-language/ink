#include "ink/execution/bytecode/execution_compiler.h"

#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/array_instruction.h"
#include "ink/ir/instruction/array_element_pointer_instruction.h"
#include "ink/ir/instruction/array_extract_instruction.h"
#include "ink/ir/instruction/class_instruction.h"
#include "ink/ir/instruction/field_extract_instruction.h"
#include "ink/ir/instruction/field_pointer_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/c_string_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/compare_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/logical_and_instruction.h"
#include "ink/ir/instruction/logical_not_instruction.h"
#include "ink/ir/instruction/logical_or_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"

#include <array>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ink::execution
{
  namespace
  {
    ExecutionPredicate lowerPredicate(ir::ComparisonPredicate Predicate)
    {
      switch (Predicate)
      {
      case ir::ComparisonPredicate::Equal:
        return ExecutionPredicate::Equal;
      case ir::ComparisonPredicate::NotEqual:
        return ExecutionPredicate::NotEqual;
      case ir::ComparisonPredicate::Less:
        return ExecutionPredicate::Less;
      case ir::ComparisonPredicate::LessEqual:
        return ExecutionPredicate::LessEqual;
      case ir::ComparisonPredicate::Greater:
        return ExecutionPredicate::Greater;
      case ir::ComparisonPredicate::GreaterEqual:
        return ExecutionPredicate::GreaterEqual;
      }
      return ExecutionPredicate::Equal;
    }

    BytecodeOpcode memoryOpcode(const TypeDesc &Layout, bool Store)
    {
      if (Layout.Kind == RuntimeKind::Integer)
      {
        switch (Layout.bitWidth())
        {
        case 8:
          return Store ? BytecodeOpcode::StoreI8 : BytecodeOpcode::LoadI8;
        case 16:
          return Store ? BytecodeOpcode::StoreI16 : BytecodeOpcode::LoadI16;
        case 32:
          return Store ? BytecodeOpcode::StoreI32 : BytecodeOpcode::LoadI32;
        case 64:
          return Store ? BytecodeOpcode::StoreI64 : BytecodeOpcode::LoadI64;
        default:
          break;
        }
      }
      return Store ? BytecodeOpcode::Store : BytecodeOpcode::Load;
    }

    BytecodeOpcode integerOpcode(const ir::IntegerType &Type, bool Compare)
    {
      if (Compare)
      {
        switch (Type.bitWidth())
        {
        case 8:
          return Type.isSigned() ? BytecodeOpcode::CompareSigned8 : BytecodeOpcode::CompareUnsigned8;
        case 16:
          return Type.isSigned() ? BytecodeOpcode::CompareSigned16 : BytecodeOpcode::CompareUnsigned16;
        case 32:
          return Type.isSigned() ? BytecodeOpcode::CompareSigned32 : BytecodeOpcode::CompareUnsigned32;
        case 64:
          return Type.isSigned() ? BytecodeOpcode::CompareSigned64 : BytecodeOpcode::CompareUnsigned64;
        default:
          return BytecodeOpcode::CompareWide;
        }
      }
      switch (Type.bitWidth())
      {
      case 8:
        return BytecodeOpcode::AddI8;
      case 16:
        return BytecodeOpcode::AddI16;
      case 32:
        return BytecodeOpcode::AddI32;
      case 64:
        return BytecodeOpcode::AddI64;
      default:
        return BytecodeOpcode::AddWide;
      }
    }

    struct BranchFixup
    {
        std::uint32_t Index;
        const ir::BasicBlock *First;
        const ir::BasicBlock *Second = nullptr;
    };

    class FunctionCompiler
    {
      public:
        FunctionCompiler(const ir::Function &Source, SemanticValueBridge &Bridge)
            : Source(Source),
              Bridge(Bridge),
              Result(std::make_unique<ExecutableFunction>())
        {
          Result->Id = Bridge.lowerFunction(Source);
          Result->Signature = Bridge.lowerType(Source.functionType());
          Result->Layouts = Bridge.types();
        }

        ExecutionCompilationResult compile()
        {
          for (const auto &Parameter : Source.parameters())
          {
            if (Parameter->outer() != &Source)
            {
              return {ExecutionStatus::InvalidArguments, nullptr};
            }
            slot(*Parameter);
          }
          for (const auto &Block : Source.blocks())
          {
            if (Block->outer() != &Source || &Block->context() != &Source.context())
            {
              return {ExecutionStatus::InvalidArguments, nullptr};
            }
            for (const auto &Value : Block->values())
            {
              if (Value->outer() != Block.get())
              {
                return {ExecutionStatus::InvalidArguments, nullptr};
              }
              slot(*Value);
              if (ir::AllocaInstruction::classof(Value.get()))
              {
                Locals.insert(Value.get());
              }
            }
          }
          inspectUses(Source, true);
          for (const auto &Block : Source.blocks())
          {
            Blocks.emplace(Block.get(), index(Result->Code.size()));
            for (const auto &Value : Block->values())
            {
              lower(*Value);
              if (Status != ExecutionStatus::Success)
              {
                return {Status, nullptr};
              }
            }
            if (!Block->terminator())
            {
              emit({BytecodeOpcode::Failure, {0, static_cast<std::uint32_t>(ExecutionStatus::MissingBody)}});
            }
          }
          if (Result->Code.empty())
          {
            emit({BytecodeOpcode::Failure, {0, static_cast<std::uint32_t>(ExecutionStatus::MissingBody)}});
          }
          for (const BranchFixup &Fixup : Fixups)
          {
            const auto First = Blocks.find(Fixup.First);
            if (First == Blocks.end())
            {
              return {ExecutionStatus::InvalidArguments, nullptr};
            }
            BytecodeInstruction &InstructionValue = Result->Code[Fixup.Index];
            if (!Fixup.Second)
            {
              InstructionValue.Operands[1] = First->second;
              continue;
            }
            const auto Second = Blocks.find(Fixup.Second);
            if (Second == Blocks.end())
            {
              return {ExecutionStatus::InvalidArguments, nullptr};
            }
            InstructionValue.Operands[2] = First->second;
            InstructionValue.Operands[3] = Second->second;
          }
          if (Status != ExecutionStatus::Success)
          {
            return {Status, nullptr};
          }
          Status = ExecutionCompiler{}.verify(*Result);
          return Status == ExecutionStatus::Success ? ExecutionCompilationResult{Status, std::move(Result)} : ExecutionCompilationResult{Status, nullptr};
        }

      private:
        std::uint32_t index(std::size_t Index)
        {
          if (Index >= InvalidSlot)
          {
            Status = ExecutionStatus::BudgetExceeded;
            return 0;
          }
          return static_cast<std::uint32_t>(Index);
        }

        SlotId slot(const ir::Value &Value)
        {
          if (&Value.context() != &Source.context() || &Value.type().context() != &Source.context())
          {
            Status = ExecutionStatus::ForeignContext;
            return 0;
          }
          const auto Found = Slots.find(&Value);
          if (Found != Slots.end())
          {
            return Found->second;
          }
          const SlotId Slot = index(Result->SlotTypes.size());
          Slots.emplace(&Value, Slot);
          const RuntimeTypeId Type = Bridge.lowerType(Value.type());
          if (Type == InvalidRuntimeType)
          {
            Status = ExecutionStatus::UnsupportedOperation;
          }
          Result->SlotTypes.push_back(Type);
          RuntimeValue Initial;
          Initial.Type = Type;
          if (ir::Constant::classof(&Value) || ir::Function::classof(&Value))
          {
            const RuntimeValueResult Lowered = Bridge.lowerConstant(Value);
            if (!Lowered)
            {
              Status = Lowered.Status;
            }
            else
            {
              Initial = Lowered.Value;
            }
          }
          Result->InitialSlots.push_back(std::move(Initial));
          return Slot;
        }

        void emit(BytecodeInstruction Value)
        {
          index(Result->Code.size());
          Result->Code.push_back(Value);
        }

        void use(const ir::Value &Value, bool Address = false)
        {
          if (!Address)
          {
            Locals.erase(&Value);
          }
        }

        template <typename Binary>
        void inspectBinary(const Binary &Value)
        {
          use(Value.left());
          use(Value.right());
        }

        void inspectUses(const ir::Function &Function, bool LocalFunction)
        {
          std::vector<const ir::Function *> Pending = {&Function};
          while (!Pending.empty())
          {
            const ir::Function *Current = Pending.back();
            Pending.pop_back();
            inspectFunctionUses(*Current, LocalFunction && Current == &Function, Pending);
          }
        }

        void inspectFunctionUses(const ir::Function &Function, bool LocalFunction, std::vector<const ir::Function *> &Pending)
        {
          for (const auto &Block : Function.blocks())
          {
            for (const auto &Owner : Block->values())
            {
              const ir::Value &Value = *Owner;
              switch (Value.kind())
              {
              case ir::ValueKind::AllocaInstruction:
              case ir::ValueKind::CStringInstruction:
              case ir::ValueKind::BranchInstruction:
                break;
              case ir::ValueKind::LoadInstruction:
                use(static_cast<const ir::LoadInstruction &>(Value).address(), LocalFunction);
                break;
              case ir::ValueKind::ClassInstruction:
                for (const ir::Value *Field : static_cast<const ir::ClassInstruction &>(Value).fields())
                {
                  use(*Field);
                }
                break;
              case ir::ValueKind::FieldExtractInstruction:
                use(static_cast<const ir::FieldExtractInstruction &>(Value).object());
                break;
              case ir::ValueKind::FieldPointerInstruction:
                use(static_cast<const ir::FieldPointerInstruction &>(Value).address());
                break;
              case ir::ValueKind::ArrayInstruction:
                for (const ir::Value *Element : static_cast<const ir::ArrayInstruction &>(Value).elements())
                {
                  use(*Element);
                }
                break;
              case ir::ValueKind::ArrayElementPointerInstruction:
              {
                const auto &Element = static_cast<const ir::ArrayElementPointerInstruction &>(Value);
                use(Element.address());
                use(Element.index());
                break;
              }
              case ir::ValueKind::ArrayExtractInstruction:
              {
                const auto &Element = static_cast<const ir::ArrayExtractInstruction &>(Value);
                use(Element.array());
                use(Element.index());
                break;
              }
              case ir::ValueKind::StoreInstruction:
              {
                const auto &Store = static_cast<const ir::StoreInstruction &>(Value);
                use(Store.address(), LocalFunction);
                use(Store.storedValue());
                break;
              }
              case ir::ValueKind::AddInstruction:
                inspectBinary(static_cast<const ir::AddInstruction &>(Value));
                break;
              case ir::ValueKind::CompareInstruction:
                inspectBinary(static_cast<const ir::CompareInstruction &>(Value));
                break;
              case ir::ValueKind::LogicalNotInstruction:
                use(static_cast<const ir::LogicalNotInstruction &>(Value).operand());
                break;
              case ir::ValueKind::LogicalAndInstruction:
                inspectBinary(static_cast<const ir::LogicalAndInstruction &>(Value));
                break;
              case ir::ValueKind::LogicalOrInstruction:
                inspectBinary(static_cast<const ir::LogicalOrInstruction &>(Value));
                break;
              case ir::ValueKind::ConditionalBranchInstruction:
                use(static_cast<const ir::ConditionalBranchInstruction &>(Value).condition());
                break;
              case ir::ValueKind::ReturnInstruction:
                if (const auto *Returned = static_cast<const ir::ReturnInstruction &>(Value).returnedValue())
                {
                  use(*Returned);
                }
                break;
              case ir::ValueKind::CallInstruction:
              {
                const auto &Call = static_cast<const ir::CallInstruction &>(Value);
                use(Call.callee());
                for (const ir::Value *Argument : Call.arguments())
                {
                  use(*Argument);
                }
                break;
              }
              case ir::ValueKind::Function:
                Pending.push_back(&static_cast<const ir::Function &>(Value));
                break;
              default:
                // An unknown instruction might observe any address identity.
                Locals.clear();
                break;
              }
            }
          }
        }

        template <typename Binary>
        void lowerBinary(BytecodeOpcode Code, SlotId Destination, const Binary &Value, std::uint32_t Extra = 0)
        {
          const SlotId First = slot(Value.left());
          const SlotId Second = slot(Value.right());
          emit({Code, {Destination, First, Second, Extra}});
        }

        void lower(const ir::Value &Value)
        {
          const SlotId Destination = slot(Value);
          switch (Value.kind())
          {
          case ir::ValueKind::AllocaInstruction:
          {
            const auto &Alloca = static_cast<const ir::AllocaInstruction &>(Value);
            const RuntimeTypeId Type = Bridge.lowerType(Alloca.allocatedType());
            if (Locals.contains(&Value))
            {
              const auto Local = index(Result->LocalStorageCount);
              ++Result->LocalStorageCount;
              emit({BytecodeOpcode::AllocaLocal, {Destination, Type, Local}});
            }
            else
            {
              emit({BytecodeOpcode::Alloca, {Destination, Type}});
            }
            break;
          }
          case ir::ValueKind::LoadInstruction:
          {
            const auto &Load = static_cast<const ir::LoadInstruction &>(Value);
            const auto *Layout = Result->Layouts->get(Result->SlotTypes[Destination]);
            if (!Layout)
            {
              Status = ExecutionStatus::UnsupportedOperation;
              break;
            }
            emit({Locals.contains(&Load.address()) ? BytecodeOpcode::LoadLocal : memoryOpcode(*Layout, false), {Destination, slot(Load.address())}});
            break;
          }
          case ir::ValueKind::ArrayInstruction:
          {
            const auto &Array = static_cast<const ir::ArrayInstruction &>(Value);
            if (Array.repeated())
            {
              emit({BytecodeOpcode::ArrayRepeat, {Destination, slot(*Array.elements().front())}});
              break;
            }
            const auto Offset = index(Result->ConstantData.size());
            if (Array.elements().size() > (InvalidSlot - Result->ConstantData.size()) / 4)
            {
              Status = ExecutionStatus::BudgetExceeded;
              break;
            }
            for (const ir::Value *Element : Array.elements())
            {
              const SlotId SourceSlot = slot(*Element);
              for (unsigned Byte = 0; Byte < 4; ++Byte)
              {
                Result->ConstantData.push_back(static_cast<char>((SourceSlot >> (Byte * 8)) & 0xff));
              }
            }
            emit({BytecodeOpcode::Array, {Destination, Offset, index(Array.elements().size() * 4)}});
            break;
          }
          case ir::ValueKind::ClassInstruction:
          {
            const auto &Class = static_cast<const ir::ClassInstruction &>(Value);
            const auto Offset = index(Result->ConstantData.size());
            if (Class.fields().size() > (InvalidSlot - Result->ConstantData.size()) / 4)
            {
              Status = ExecutionStatus::BudgetExceeded;
              break;
            }
            for (const ir::Value *Element : Class.fields())
            {
              const SlotId SourceSlot = slot(*Element);
              for (unsigned Byte = 0; Byte < 4; ++Byte)
              {
                Result->ConstantData.push_back(static_cast<char>((SourceSlot >> (Byte * 8)) & 0xff));
              }
            }
            emit({BytecodeOpcode::Class, {Destination, Offset, index(Class.fields().size() * 4)}});
            break;
          }
          case ir::ValueKind::FieldExtractInstruction:
          {
            const auto &Field = static_cast<const ir::FieldExtractInstruction &>(Value);
            emit({BytecodeOpcode::FieldExtract, {Destination, slot(Field.object()), index(Field.fieldIndex())}});
            break;
          }
          case ir::ValueKind::FieldPointerInstruction:
          {
            const auto &Field = static_cast<const ir::FieldPointerInstruction &>(Value);
            emit({BytecodeOpcode::FieldPointer, {Destination, slot(Field.address()), index(Field.fieldIndex())}});
            break;
          }
          case ir::ValueKind::ArrayElementPointerInstruction:
          {
            const auto &Element = static_cast<const ir::ArrayElementPointerInstruction &>(Value);
            emit({BytecodeOpcode::ArrayElementPointer, {Destination, slot(Element.address()), slot(Element.index())}});
            break;
          }
          case ir::ValueKind::ArrayExtractInstruction:
          {
            const auto &Element = static_cast<const ir::ArrayExtractInstruction &>(Value);
            emit({BytecodeOpcode::ArrayExtract, {Destination, slot(Element.array()), slot(Element.index())}});
            break;
          }
          case ir::ValueKind::StoreInstruction:
          {
            const auto &Store = static_cast<const ir::StoreInstruction &>(Value);
            const SlotId Address = slot(Store.address());
            const SlotId Stored = slot(Store.storedValue());
            const auto *Layout = Result->Layouts->get(Result->SlotTypes[Stored]);
            if (!Layout)
            {
              Status = ExecutionStatus::UnsupportedOperation;
              break;
            }
            emit({Locals.contains(&Store.address()) ? BytecodeOpcode::StoreLocal : memoryOpcode(*Layout, true), {Destination, Address, Stored}});
            break;
          }
          case ir::ValueKind::CStringInstruction:
          {
            const auto &CString = static_cast<const ir::CStringInstruction &>(Value);
            if (&CString.source().context() != &Source.context() || !Source.context().constantPool().owns(CString.source()))
            {
              Status = ExecutionStatus::ForeignContext;
              break;
            }
            if (!CString.source().tryGetCString())
            {
              Status = ExecutionStatus::InvalidArguments;
              break;
            }
            const std::string_view Bytes = CString.source().value();
            const auto Offset = index(Result->ConstantData.size());
            const auto Length = index(Bytes.size());
            if (Status != ExecutionStatus::Success || Bytes.size() >= InvalidSlot - Result->ConstantData.size())
            {
              Status = ExecutionStatus::BudgetExceeded;
              break;
            }
            Result->ConstantData.insert(Result->ConstantData.end(), Bytes.begin(), Bytes.end());
            emit({BytecodeOpcode::CString, {Destination, Offset, Length}});
            break;
          }
          case ir::ValueKind::AddInstruction:
            if (!ir::IntegerType::classof(&Value.type()))
            {
              Status = ExecutionStatus::TypeMismatch;
              break;
            }
            lowerBinary(integerOpcode(static_cast<const ir::IntegerType &>(Value.type()), false), Destination, static_cast<const ir::AddInstruction &>(Value));
            break;
          case ir::ValueKind::LogicalNotInstruction:
            emit({BytecodeOpcode::LogicalNot, {Destination, slot(static_cast<const ir::LogicalNotInstruction &>(Value).operand())}});
            break;
          case ir::ValueKind::LogicalAndInstruction:
            lowerBinary(BytecodeOpcode::LogicalAnd, Destination, static_cast<const ir::LogicalAndInstruction &>(Value));
            break;
          case ir::ValueKind::LogicalOrInstruction:
            lowerBinary(BytecodeOpcode::LogicalOr, Destination, static_cast<const ir::LogicalOrInstruction &>(Value));
            break;
          case ir::ValueKind::CompareInstruction:
          {
            const auto &Compare = static_cast<const ir::CompareInstruction &>(Value);
            const ir::Type &Type = Compare.left().type();
            if (Type.typeKind() != ir::TypeKind::Bool && !ir::IntegerType::classof(&Type))
            {
              Status = ExecutionStatus::TypeMismatch;
              break;
            }
            const BytecodeOpcode Code = Type.typeKind() == ir::TypeKind::Bool ? BytecodeOpcode::CompareBool : integerOpcode(static_cast<const ir::IntegerType &>(Type), true);
            lowerBinary(Code, Destination, Compare, static_cast<std::uint32_t>(lowerPredicate(Compare.predicate())));
            break;
          }
          case ir::ValueKind::CallInstruction:
          {
            const auto &Call = static_cast<const ir::CallInstruction &>(Value);
            ExecutionCallSite Site;
            Site.Signature = Bridge.lowerType(Call.functionType());
            Site.Target = Call.directCallee() ? Bridge.lowerFunction(*Call.directCallee()) : InvalidFunction;
            if (Site.Target == InvalidFunction)
            {
              Site.CalleeSlot = slot(Call.callee());
            }
            Site.Arguments.reserve(Call.arguments().size());
            for (const ir::Value *Argument : Call.arguments())
            {
              Site.Arguments.push_back(slot(*Argument));
            }
            const auto SiteIndex = index(Result->Calls.size());
            Result->Calls.push_back(std::move(Site));
            emit({Call.directCallee() ? BytecodeOpcode::CallDirect : BytecodeOpcode::CallIndirect, {Destination, SiteIndex}});
            break;
          }
          case ir::ValueKind::ReturnInstruction:
            if (const auto *Returned = static_cast<const ir::ReturnInstruction &>(Value).returnedValue())
            {
              emit({BytecodeOpcode::Return, {0, slot(*Returned)}});
            }
            else
            {
              emit({BytecodeOpcode::ReturnVoid});
            }
            break;
          case ir::ValueKind::BranchInstruction:
            Fixups.push_back({index(Result->Code.size()), &static_cast<const ir::BranchInstruction &>(Value).target()});
            emit({BytecodeOpcode::Jump});
            break;
          case ir::ValueKind::ConditionalBranchInstruction:
          {
            const auto &Branch = static_cast<const ir::ConditionalBranchInstruction &>(Value);
            Fixups.push_back({index(Result->Code.size()), &Branch.trueTarget(), &Branch.falseTarget()});
            emit({BytecodeOpcode::JumpIf, {0, slot(Branch.condition())}});
            break;
          }
          case ir::ValueKind::Function:
            emit({BytecodeOpcode::Function, {Destination}});
            break;
          default:
            emit({BytecodeOpcode::Failure, {0, static_cast<std::uint32_t>(ExecutionStatus::UnsupportedOperation)}});
            break;
          }
        }

        const ir::Function &Source;
        SemanticValueBridge &Bridge;
        std::unique_ptr<ExecutableFunction> Result;
        ExecutionStatus Status = ExecutionStatus::Success;
        std::unordered_map<const ir::Value *, SlotId> Slots;
        std::unordered_map<const ir::BasicBlock *, std::uint32_t> Blocks;
        std::unordered_set<const ir::Value *> Locals;
        std::vector<BranchFixup> Fixups;
    };

    bool validOperand(BytecodeOperandKind Kind, std::uint32_t Value, const ExecutableFunction &Function)
    {
      switch (Kind)
      {
      case BytecodeOperandKind::FieldIndex:
        return true;
      case BytecodeOperandKind::None:
        return Value == 0;
      case BytecodeOperandKind::WriteSlot:
      case BytecodeOperandKind::ReadSlot:
        return Value < Function.SlotTypes.size();
      case BytecodeOperandKind::Target:
        return Value < Function.Code.size();
      case BytecodeOperandKind::CallSite:
        return Value < Function.Calls.size();
      case BytecodeOperandKind::Layout:
        return Function.Layouts->get(Value) != nullptr;
      case BytecodeOperandKind::Predicate:
        return Value <= static_cast<std::uint32_t>(ExecutionPredicate::GreaterEqual);
      case BytecodeOperandKind::Local:
        return Value < Function.LocalStorageCount;
      case BytecodeOperandKind::DataOffset:
      case BytecodeOperandKind::DataLength:
        return Value <= Function.ConstantData.size();
      case BytecodeOperandKind::Status:
        return Value > static_cast<std::uint32_t>(ExecutionStatus::Success) && Value <= static_cast<std::uint32_t>(ExecutionStatus::IndexOutOfBounds);
      }
      return false;
    }

    bool isKind(const TypeDesc *Layout, RuntimeKind Kind)
    {
      return Layout && Layout->Kind == Kind;
    }

    BytecodeOpcode integerOpcode(const TypeDesc &Layout, bool Compare)
    {
      if (Compare)
      {
        switch (Layout.bitWidth())
        {
        case 8:
          return Layout.isSigned() ? BytecodeOpcode::CompareSigned8 : BytecodeOpcode::CompareUnsigned8;
        case 16:
          return Layout.isSigned() ? BytecodeOpcode::CompareSigned16 : BytecodeOpcode::CompareUnsigned16;
        case 32:
          return Layout.isSigned() ? BytecodeOpcode::CompareSigned32 : BytecodeOpcode::CompareUnsigned32;
        case 64:
          return Layout.isSigned() ? BytecodeOpcode::CompareSigned64 : BytecodeOpcode::CompareUnsigned64;
        default:
          return BytecodeOpcode::CompareWide;
        }
      }
      switch (Layout.bitWidth())
      {
      case 8:
        return BytecodeOpcode::AddI8;
      case 16:
        return BytecodeOpcode::AddI16;
      case 32:
        return BytecodeOpcode::AddI32;
      case 64:
        return BytecodeOpcode::AddI64;
      default:
        return BytecodeOpcode::AddWide;
      }
    }

    bool verifyCall(const ExecutableFunction &Function, const BytecodeInstruction &Value)
    {
      const ExecutionCallSite &Call = Function.Calls[Value.Operands[1]];
      const TypeDesc *Signature = Function.Layouts->get(Call.Signature);
      if (!isKind(Signature, RuntimeKind::Function))
      {
        return false;
      }
      if (Value.Code == BytecodeOpcode::CallDirect)
      {
        // The linker validates the ID against its function table and confirms
        // the descriptor signature before making this target executable.
        if (Call.Target == InvalidFunction || Call.CalleeSlot != InvalidSlot)
        {
          return false;
        }
      }
      else if (Call.Target != InvalidFunction || Call.CalleeSlot >= Function.SlotTypes.size() || Function.SlotTypes[Call.CalleeSlot] != Call.Signature)
      {
        return false;
      }
      if (Signature->functionDesc().ReturnType != Function.SlotTypes[Value.Operands[0]] || Signature->functionDesc().Parameters.size() != Call.Arguments.size())
      {
        return false;
      }
      for (std::size_t Index = 0; Index < Call.Arguments.size(); ++Index)
      {
        if (Call.Arguments[Index] >= Function.SlotTypes.size() || Function.SlotTypes[Call.Arguments[Index]] != Signature->functionDesc().Parameters[Index])
        {
          return false;
        }
      }
      return true;
    }

    bool verifyTypes(const ExecutableFunction &Function, const BytecodeInstruction &Value)
    {
      const auto SlotType = [&Function](SlotId Slot)
      {
        return Function.Layouts->get(Function.SlotTypes[Slot]);
      };
      switch (Value.Code)
      {
      case BytecodeOpcode::Alloca:
      case BytecodeOpcode::AllocaLocal:
      {
        const auto *Layout = SlotType(Value.Operands[0]);
        return isKind(Layout, RuntimeKind::Pointer) && Layout->pointerDesc().Writable && Layout->pointerDesc().Pointee == Value.Operands[1];
      }
      case BytecodeOpcode::Load:
      case BytecodeOpcode::LoadLocal:
      case BytecodeOpcode::LoadI8:
      case BytecodeOpcode::LoadI16:
      case BytecodeOpcode::LoadI32:
      case BytecodeOpcode::LoadI64:
      {
        const auto *Address = SlotType(Value.Operands[1]);
        if (!isKind(Address, RuntimeKind::Pointer) || Address->pointerDesc().Pointee != Function.SlotTypes[Value.Operands[0]])
        {
          return false;
        }
        return Value.Code == BytecodeOpcode::Load || Value.Code == BytecodeOpcode::LoadLocal || Value.Code == memoryOpcode(*SlotType(Value.Operands[0]), false);
      }
      case BytecodeOpcode::Store:
      case BytecodeOpcode::StoreLocal:
      case BytecodeOpcode::StoreI8:
      case BytecodeOpcode::StoreI16:
      case BytecodeOpcode::StoreI32:
      case BytecodeOpcode::StoreI64:
      {
        const auto *Address = SlotType(Value.Operands[1]);
        if (!isKind(SlotType(Value.Operands[0]), RuntimeKind::Void) || !isKind(Address, RuntimeKind::Pointer) || !Address->pointerDesc().Writable || Address->pointerDesc().Pointee != Function.SlotTypes[Value.Operands[2]])
        {
          return false;
        }
        return Value.Code == BytecodeOpcode::Store || Value.Code == BytecodeOpcode::StoreLocal || Value.Code == memoryOpcode(*SlotType(Value.Operands[2]), true);
      }
      case BytecodeOpcode::CString:
      {
        const auto *Pointer = SlotType(Value.Operands[0]);
        if (!isKind(Pointer, RuntimeKind::Pointer) || !Pointer->pointerDesc().Writable)
        {
          return false;
        }
        const auto *Pointee = Function.Layouts->get(Pointer->pointerDesc().Pointee);
        return isKind(Pointee, RuntimeKind::Integer) && Pointee->bitWidth() == 8 && !Pointee->isSigned();
      }
      case BytecodeOpcode::Array:
      case BytecodeOpcode::Class:
      {
        const auto *Array = SlotType(Value.Operands[0]);
        const std::size_t Offset = Value.Operands[1];
        const std::size_t Length = Value.Operands[2];
        const bool Class = Value.Code == BytecodeOpcode::Class;
        if (!isKind(Array, Class ? RuntimeKind::Class : RuntimeKind::Array) || Length % 4 || Offset > Function.ConstantData.size() || Length > Function.ConstantData.size() - Offset || (Class ? Array->classDesc().Fields.size() : Array->arrayDesc().ElementCount) != Length / 4)
        {
          return false;
        }
        for (std::size_t Byte = 0; Byte < Length; Byte += 4)
        {
          const SlotId Source = arraySourceSlot(Function, Offset + Byte);
          if (Source >= Function.SlotTypes.size() || Function.SlotTypes[Source] != (Class ? Array->classDesc().Fields[Byte / 4].Type : Array->arrayDesc().ElementType))
          {
            return false;
          }
        }
        return true;
      }
      case BytecodeOpcode::FieldExtract:
      {
        const auto *Class = SlotType(Value.Operands[1]);
        return isKind(Class, RuntimeKind::Class) && Value.Operands[2] < Class->classDesc().Fields.size() && Class->classDesc().Fields[Value.Operands[2]].Type == Function.SlotTypes[Value.Operands[0]];
      }
      case BytecodeOpcode::FieldPointer:
      {
        const auto *Address = SlotType(Value.Operands[1]);
        const auto *Destination = SlotType(Value.Operands[0]);
        const auto *Class = isKind(Address, RuntimeKind::Pointer) ? Function.Layouts->get(Address->pointerDesc().Pointee) : nullptr;
        return isKind(Class, RuntimeKind::Class) && Value.Operands[2] < Class->classDesc().Fields.size() && isKind(Destination, RuntimeKind::Pointer) && Destination->pointerDesc().Pointee == Class->classDesc().Fields[Value.Operands[2]].Type && Destination->pointerDesc().Writable == Address->pointerDesc().Writable;
      }
      case BytecodeOpcode::ArrayRepeat:
      {
        const auto *Array = SlotType(Value.Operands[0]);
        return isKind(Array, RuntimeKind::Array) && Array->arrayDesc().ElementType == Function.SlotTypes[Value.Operands[1]];
      }
      case BytecodeOpcode::ArrayElementPointer:
      {
        const auto *Address = SlotType(Value.Operands[1]);
        const auto *Destination = SlotType(Value.Operands[0]);
        const auto *Array = isKind(Address, RuntimeKind::Pointer) ? Function.Layouts->get(Address->pointerDesc().Pointee) : nullptr;
        return isKind(Array, RuntimeKind::Array) && isKind(Destination, RuntimeKind::Pointer) && Destination->pointerDesc().Pointee == Array->arrayDesc().ElementType && Destination->pointerDesc().Writable == Address->pointerDesc().Writable && isKind(SlotType(Value.Operands[2]), RuntimeKind::Integer);
      }
      case BytecodeOpcode::ArrayExtract:
      {
        const auto *Array = SlotType(Value.Operands[1]);
        return isKind(Array, RuntimeKind::Array) && Array->arrayDesc().ElementType == Function.SlotTypes[Value.Operands[0]] && isKind(SlotType(Value.Operands[2]), RuntimeKind::Integer);
      }
      case BytecodeOpcode::AddI8:
      case BytecodeOpcode::AddI16:
      case BytecodeOpcode::AddI32:
      case BytecodeOpcode::AddI64:
      case BytecodeOpcode::AddWide:
        return isKind(SlotType(Value.Operands[1]), RuntimeKind::Integer) && Function.SlotTypes[Value.Operands[1]] == Function.SlotTypes[Value.Operands[2]] && Function.SlotTypes[Value.Operands[1]] == Function.SlotTypes[Value.Operands[0]] && integerOpcode(*SlotType(Value.Operands[1]), false) == Value.Code;
      case BytecodeOpcode::LogicalNot:
        return isKind(SlotType(Value.Operands[0]), RuntimeKind::Boolean) && Function.SlotTypes[Value.Operands[0]] == Function.SlotTypes[Value.Operands[1]];
      case BytecodeOpcode::LogicalAnd:
      case BytecodeOpcode::LogicalOr:
        return isKind(SlotType(Value.Operands[0]), RuntimeKind::Boolean) && Function.SlotTypes[Value.Operands[0]] == Function.SlotTypes[Value.Operands[1]] && Function.SlotTypes[Value.Operands[1]] == Function.SlotTypes[Value.Operands[2]];
      case BytecodeOpcode::CompareSigned8:
      case BytecodeOpcode::CompareSigned16:
      case BytecodeOpcode::CompareSigned32:
      case BytecodeOpcode::CompareSigned64:
      case BytecodeOpcode::CompareUnsigned8:
      case BytecodeOpcode::CompareUnsigned16:
      case BytecodeOpcode::CompareUnsigned32:
      case BytecodeOpcode::CompareUnsigned64:
      case BytecodeOpcode::CompareWide:
        return isKind(SlotType(Value.Operands[0]), RuntimeKind::Boolean) && isKind(SlotType(Value.Operands[1]), RuntimeKind::Integer) && Function.SlotTypes[Value.Operands[1]] == Function.SlotTypes[Value.Operands[2]] && integerOpcode(*SlotType(Value.Operands[1]), true) == Value.Code;
      case BytecodeOpcode::CompareBool:
        return isKind(SlotType(Value.Operands[0]), RuntimeKind::Boolean) && Function.SlotTypes[Value.Operands[0]] == Function.SlotTypes[Value.Operands[1]] && Function.SlotTypes[Value.Operands[1]] == Function.SlotTypes[Value.Operands[2]] && Value.Operands[3] <= static_cast<std::uint32_t>(ExecutionPredicate::NotEqual);
      case BytecodeOpcode::CallDirect:
      case BytecodeOpcode::CallIndirect:
        return verifyCall(Function, Value);
      case BytecodeOpcode::Jump:
      case BytecodeOpcode::Failure:
        return true;
      case BytecodeOpcode::JumpIf:
        return isKind(SlotType(Value.Operands[1]), RuntimeKind::Boolean);
      case BytecodeOpcode::Return:
        return Function.SlotTypes[Value.Operands[1]] == Function.Layouts->get(Function.Signature)->functionDesc().ReturnType;
      case BytecodeOpcode::ReturnVoid:
        return isKind(Function.Layouts->get(Function.Layouts->get(Function.Signature)->functionDesc().ReturnType), RuntimeKind::Void);
      case BytecodeOpcode::Function:
        return isKind(SlotType(Value.Operands[0]), RuntimeKind::Function) && Function.InitialSlots[Value.Operands[0]].Initialized;
      case BytecodeOpcode::Count:
        return false;
      }
      return false;
    }

    bool validInitialValue(const RuntimeValue &Value, const TypeDesc &Layout)
    {
      if (!Value.Initialized)
      {
        return !Value.Object && Value.Bits == 0;
      }
      switch (Layout.Kind)
      {
      case RuntimeKind::Void:
        return !Value.Object && Value.Bits == 0;
      case RuntimeKind::Boolean:
        return !Value.Object && Value.Bits <= 1;
      case RuntimeKind::Integer:
        if (Layout.bitWidth() > 64)
        {
          return Value.Object && Value.kind() == RuntimeKind::Integer && Value.integer().bitWidth() == Layout.bitWidth() && Value.integer().valid();
        }
        return Layout.bitWidth() != 0 && !Value.Object && (Layout.bitWidth() == 64 || (Value.Bits >> Layout.bitWidth()) == 0);
      case RuntimeKind::Float:
        return !Value.Object && ((Layout.bitWidth() == 16 || Layout.bitWidth() == 32) ? (Value.Bits >> Layout.bitWidth()) == 0 : Layout.bitWidth() == 64);
      case RuntimeKind::String:
        return Value.Object && Value.kind() == RuntimeKind::String;
      case RuntimeKind::Array:
        if (!Value.Object || Value.kind() != RuntimeKind::Array || !Layout.arrayDesc().ElementLayout || Value.array().size() != Layout.arrayDesc().ElementCount)
        {
          return false;
        }
        for (const RuntimeValue &Element : Value.array())
        {
          if (!Element.Initialized || Element.Type != Layout.arrayDesc().ElementType || !validInitialValue(Element, *Layout.arrayDesc().ElementLayout))
          {
            return false;
          }
        }
        return true;
      case RuntimeKind::Class:
        if (!Value.Object || Value.kind() != RuntimeKind::Class || Value.fields().size() != Layout.classDesc().Fields.size())
        {
          return false;
        }
        for (std::size_t Index = 0; Index < Value.fields().size(); ++Index)
        {
          const RuntimeValue &Field = Value.fields()[Index];
          if (!Field.Initialized || Field.Type != Layout.classDesc().Fields[Index].Type || !validInitialValue(Field, *Layout.classDesc().Fields[Index].Layout))
          {
            return false;
          }
        }
        return true;
      case RuntimeKind::Pointer:
        return Value.Object && Value.kind() == RuntimeKind::Pointer;
      case RuntimeKind::Function:
        return !Value.Object && Value.Bits < InvalidFunction;
      case RuntimeKind::Invalid:
        return false;
      }
      return false;
    }
  } // namespace

  ExecutionCompilationResult ExecutionCompiler::compile(const ir::Function &Function, SemanticValueBridge &Bridge) const
  {
    return FunctionCompiler(Function, Bridge).compile();
  }

  ExecutionStatus ExecutionCompiler::verify(const ExecutableFunction &Function) const noexcept
  {
    if (Function.Id == InvalidFunction || !Function.Layouts || Function.Code.empty() || Function.InitialSlots.size() != Function.SlotTypes.size() || Function.LocalStorageCount > Function.SlotTypes.size())
    {
      return ExecutionStatus::InvalidArguments;
    }
    const TypeDesc *Signature = Function.Layouts->get(Function.Signature);
    if (!isKind(Signature, RuntimeKind::Function) || !Function.Layouts->get(Signature->functionDesc().ReturnType) || Function.SlotTypes.size() < Signature->functionDesc().Parameters.size())
    {
      return ExecutionStatus::InvalidArguments;
    }
    for (std::size_t Index = 0; Index < Function.SlotTypes.size(); ++Index)
    {
      const TypeDesc *Layout = Function.Layouts->get(Function.SlotTypes[Index]);
      if (!Layout)
      {
        return ExecutionStatus::InvalidArguments;
      }
      if (Function.InitialSlots[Index].Type != Function.SlotTypes[Index] || !validInitialValue(Function.InitialSlots[Index], *Layout))
      {
        return ExecutionStatus::TypeMismatch;
      }
      if (Index < Signature->functionDesc().Parameters.size() && (Function.SlotTypes[Index] != Signature->functionDesc().Parameters[Index] || Function.InitialSlots[Index].Initialized))
      {
        return ExecutionStatus::TypeMismatch;
      }
    }
    // Local storage is an internal frame-cell index, never a language pointer.
    // Validate every producer and use before enabling the direct local path.
    std::vector<bool> LocalSlots(Function.SlotTypes.size(), false);
    std::vector<SlotId> LocalOwners(Function.LocalStorageCount, InvalidSlot);
    for (const BytecodeInstruction &Value : Function.Code)
    {
      const BytecodeInstructionMetadata *Metadata = bytecodeInstructionMetadata(Value.Code);
      const auto &Operands = Value.Operands;
      if (!Metadata)
      {
        return ExecutionStatus::UnsupportedOperation;
      }
      for (std::size_t Index = 0; Index < Operands.size(); ++Index)
      {
        if (!validOperand(Metadata->Operands[Index], Operands[Index], Function))
        {
          return ExecutionStatus::InvalidArguments;
        }
      }
      if (Value.Code == BytecodeOpcode::CString)
      {
        const std::size_t Offset = Value.Operands[1];
        const std::size_t Length = Value.Operands[2];
        if (Length > Function.ConstantData.size() - Offset)
        {
          return ExecutionStatus::InvalidArguments;
        }
        for (std::size_t Index = Offset; Index < Offset + Length; ++Index)
        {
          if (Function.ConstantData[Index] == '\0')
          {
            return ExecutionStatus::InvalidArguments;
          }
        }
      }
      if (!verifyTypes(Function, Value))
      {
        return ExecutionStatus::TypeMismatch;
      }
      if (Value.Code == BytecodeOpcode::AllocaLocal)
      {
        if (Function.InitialSlots[Value.Operands[0]].Initialized || Value.Operands[0] < Signature->functionDesc().Parameters.size() || LocalSlots[Value.Operands[0]] || LocalOwners[Value.Operands[2]] != InvalidSlot)
        {
          return ExecutionStatus::InvalidArguments;
        }
        LocalSlots[Value.Operands[0]] = true;
        LocalOwners[Value.Operands[2]] = Value.Operands[0];
      }
    }
    for (SlotId Owner : LocalOwners)
    {
      if (Owner == InvalidSlot)
      {
        return ExecutionStatus::InvalidArguments;
      }
    }
    for (const BytecodeInstruction &Value : Function.Code)
    {
      const BytecodeInstructionMetadata &Metadata = *bytecodeInstructionMetadata(Value.Code);
      if ((Value.Code == BytecodeOpcode::LoadLocal || Value.Code == BytecodeOpcode::StoreLocal) && !LocalSlots[Value.Operands[1]])
      {
        return ExecutionStatus::InvalidArguments;
      }
      const auto &Operands = Value.Operands;
      for (std::size_t Index = 0; Index < Operands.size(); ++Index)
      {
        if (Metadata.Operands[Index] == BytecodeOperandKind::WriteSlot && LocalSlots[Operands[Index]] && Value.Code != BytecodeOpcode::AllocaLocal)
        {
          return ExecutionStatus::InvalidArguments;
        }
        if (Metadata.Operands[Index] == BytecodeOperandKind::ReadSlot && LocalSlots[Operands[Index]] && !(Index == 1 && (Value.Code == BytecodeOpcode::LoadLocal || Value.Code == BytecodeOpcode::StoreLocal)))
        {
          return ExecutionStatus::InvalidArguments;
        }
      }
      if (Value.Code == BytecodeOpcode::CallDirect || Value.Code == BytecodeOpcode::CallIndirect)
      {
        const ExecutionCallSite &Call = Function.Calls[Value.Operands[1]];
        if (Call.CalleeSlot != InvalidSlot && LocalSlots[Call.CalleeSlot])
        {
          return ExecutionStatus::InvalidArguments;
        }
        for (SlotId Argument : Call.Arguments)
        {
          if (LocalSlots[Argument])
          {
            return ExecutionStatus::InvalidArguments;
          }
        }
      }
      if (Value.Code == BytecodeOpcode::Array || Value.Code == BytecodeOpcode::Class)
      {
        for (std::size_t Byte = 0; Byte < Value.Operands[2]; Byte += 4)
        {
          if (LocalSlots[arraySourceSlot(Function, Value.Operands[1] + Byte)])
          {
            return ExecutionStatus::InvalidArguments;
          }
        }
      }
    }
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
