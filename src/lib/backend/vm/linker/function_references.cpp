#include "function_references.h"

#include "ink/backend/vm/instruction/machine_instruction_visitor.h"
#include "ink/core/name/name_pool.h"

#include <optional>
#include <tuple>
#include <unordered_set>
#include <utility>

namespace ink::vm::detail
{
  namespace
  {
    class FunctionReferenceWalker
    {
      public:
        explicit FunctionReferenceWalker(const SymbolReferenceVisitor &VisitReference)
            : VisitReference(VisitReference)
        {
        }

        std::expected<void, std::string> visit(const MachineFunction &Function)
        {
          for (const auto &Block : Function.basicBlocks())
          {
            if (Block.parent() != &Function || (Block.name() != core::NAME_None && !core::NamePool::instance().contains(Block.name())))
            {
              return std::unexpected("basic block has an invalid parent or name");
            }
            Blocks.insert(&Block);
          }
          const auto VisitOperands = [&](const auto &...Operands)
          {
            std::expected<void, std::string> Result;
            static_cast<void>(((Result = visitOperand(Operands)).has_value() && ...));
            return Result;
          };
          const auto VisitInstruction = [&](const auto &Typed) -> std::optional<std::expected<void, std::string>>
          {
            return std::apply(VisitOperands, Typed.operands());
          };
          for (const auto &Block : Function.basicBlocks())
          {
            for (const auto &Instruction : Block.instructions())
            {
              if (Instruction.parent() != &Block)
              {
                return std::unexpected("instruction has an invalid parent basic block");
              }
              auto Result = visitInstruction(Instruction, VisitInstruction);
              if (!Result)
              {
                return std::unexpected("instruction has an unknown opcode");
              }
              if (!*Result)
              {
                return std::move(*Result);
              }
            }
          }
          return {};
        }

      private:
        std::expected<void, std::string> visitOperand(const RegisterOperand &Value) const
        {
          if (!isArithmeticRegister(Value.getRegister()))
          {
            return std::unexpected("instruction contains an invalid explicit register");
          }
          return {};
        }

        std::expected<void, std::string> visitOperand(const AddressOperand &Value) const
        {
          if (const auto Register = Value.getRegister())
          {
            if (!isAddressRegister(*Register))
            {
              return std::unexpected("address operand contains an invalid base register");
            }
            return {};
          }
          return VisitReference(*Value.getSymbol());
        }

        std::expected<void, std::string> visitOperand(const ImmediateOperand &Value) const
        {
          if (Value.bits())
          {
            return {};
          }
          return VisitReference(*Value.getSymbol());
        }

        std::expected<void, std::string> visitOperand(const BasicBlockOperand &Value) const
        {
          if (!Blocks.contains(Value.getBlock()))
          {
            return std::unexpected("branch target does not belong to its source function");
          }
          return {};
        }

        const SymbolReferenceVisitor &VisitReference;
        std::unordered_set<const MachineBasicBlock *> Blocks;
    };
  } // namespace

  std::expected<void, std::string> visitFunctionReferences(const MachineFunction &Function, const SymbolReferenceVisitor &VisitReference)
  {
    return FunctionReferenceWalker(VisitReference).visit(Function);
  }
} // namespace ink::vm::detail
