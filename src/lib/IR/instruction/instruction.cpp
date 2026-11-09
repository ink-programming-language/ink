#include "ink/ir/instruction/instruction.h"

#include "ink/core/support/core_define.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/cond_branch_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/phi_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"

namespace ink::ir
{
  namespace detail
  {
    struct EmptyInstructionOperands
    {
        static std::size_t count(const Instruction &) noexcept
        {
          return 0;
        }

        [[noreturn]] static const Value &get(const Instruction &, std::size_t) noexcept
        {
          panic("Instruction has no operand at this index");
        }

        [[noreturn]] static bool set(Instruction &, std::size_t, ValueRef) noexcept
        {
          panic("Instruction has no operand at this index");
        }
    };

    template <>
    struct InstructionOperands<AddInstruction>
    {
        static std::size_t count(const AddInstruction &) noexcept
        {
          return 2;
        }

        static const Value &get(const AddInstruction &Node, std::size_t Index) noexcept
        {
          return Index == 0 ? Node.getLHS() : Node.getRHS();
        }

        static bool set(AddInstruction &Node, std::size_t Index, ValueRef Replacement) noexcept
        {
          (Index == 0 ? Node.LHS : Node.RHS) = Replacement;
          return true;
        }
    };

    template <>
    struct InstructionOperands<ReturnInstruction>
    {
        static std::size_t count(const ReturnInstruction &Node) noexcept
        {
          return Node.getReturnValue() ? 1 : 0;
        }

        static const Value &get(const ReturnInstruction &Node, std::size_t) noexcept
        {
          return *Node.getReturnValue();
        }

        static bool set(ReturnInstruction &Node, std::size_t, ValueRef Replacement) noexcept
        {
          Node.ReturnValue = &Replacement.get();
          return true;
        }
    };

    template <>
    struct InstructionOperands<AllocaInstruction> : EmptyInstructionOperands
    {
    };

    template <>
    struct InstructionOperands<LoadInstruction>
    {
        static std::size_t count(const LoadInstruction &) noexcept
        {
          return 1;
        }

        static const Value &get(const LoadInstruction &Node, std::size_t) noexcept
        {
          return Node.getSource();
        }

        static bool set(LoadInstruction &Node, std::size_t, ValueRef Replacement) noexcept
        {
          Node.Source = Replacement;
          return true;
        }
    };

    template <>
    struct InstructionOperands<StoreInstruction>
    {
        static std::size_t count(const StoreInstruction &) noexcept
        {
          return 2;
        }

        static const Value &get(const StoreInstruction &Node, std::size_t Index) noexcept
        {
          return Index == 0 ? Node.getDestination() : Node.getSource();
        }

        static bool set(StoreInstruction &Node, std::size_t Index, ValueRef Replacement) noexcept
        {
          (Index == 0 ? Node.Destination : Node.Source) = Replacement;
          return true;
        }
    };

    template <>
    struct InstructionOperands<BranchInstruction> : EmptyInstructionOperands
    {
    };

    template <>
    struct InstructionOperands<CondBranchInstruction>
    {
        static std::size_t count(const CondBranchInstruction &) noexcept
        {
          return 1;
        }

        static const Value &get(const CondBranchInstruction &Node, std::size_t) noexcept
        {
          return Node.getCondition();
        }

        static bool set(CondBranchInstruction &Node, std::size_t, ValueRef Replacement) noexcept
        {
          Node.Condition = Replacement;
          return true;
        }
    };

    template <>
    struct InstructionOperands<CallInstruction>
    {
        static std::size_t count(const CallInstruction &Node) noexcept
        {
          return 1 + Node.argumentCount();
        }

        static const Value &get(const CallInstruction &Node, std::size_t Index) noexcept
        {
          return Index == 0 ? Node.getCallee() : Node.argument(Index - 1);
        }

        static bool set(CallInstruction &Node, std::size_t Index, ValueRef Replacement) noexcept
        {
          if (Index == 0)
          {
            Node.Callee = Replacement;
          }
          else
          {
            Node.Arguments[Index - 1] = Replacement;
          }
          return true;
        }
    };

    template <>
    struct InstructionOperands<PhiInstruction>
    {
        static std::size_t count(const PhiInstruction &Node) noexcept
        {
          return Node.incomingCount();
        }

        static const Value &get(const PhiInstruction &Node, std::size_t Index) noexcept
        {
          return Node.incomingValue(Index);
        }

        static bool set(PhiInstruction &Node, std::size_t Index, ValueRef Replacement) noexcept
        {
          Node.IncomingValues[Index].IncomingValue = &Replacement.get();
          return true;
        }
    };
  } // namespace detail

  std::size_t Instruction::operandCount() const noexcept
  {
    switch (getKind())
    {
#define INSTRUCTION(Kind, Number, Class, Spelling) \
  case InstructionKind::Kind:                      \
    return detail::InstructionOperands<Class>::count(static_cast<const Class &>(*this));
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
    }
    panic("Invalid IR instruction kind");
  }

  const Value &Instruction::operand(std::size_t Index) const noexcept
  {
    checkMsgf(Index < operandCount(), "Instruction operand index is out of range");
    switch (getKind())
    {
#define INSTRUCTION(Kind, Number, Class, Spelling) \
  case InstructionKind::Kind:                      \
    return detail::InstructionOperands<Class>::get(static_cast<const Class &>(*this), Index);
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
    }
    panic("Invalid IR instruction kind");
  }

  bool Instruction::setOperand(std::size_t Index, ValueRef Replacement) noexcept
  {
    if (Index >= operandCount() || &Replacement.get().getType() != &operand(Index).getType())
    {
      return false;
    }
    switch (getKind())
    {
#define INSTRUCTION(Kind, Number, Class, Spelling) \
  case InstructionKind::Kind:                      \
    return detail::InstructionOperands<Class>::set(static_cast<Class &>(*this), Index, Replacement);
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION
    }
    panic("Invalid IR instruction kind");
  }
} // namespace ink::ir
