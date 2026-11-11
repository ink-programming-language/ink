#include "function_references.h"

#include "ink/backend/vm/instruction/machine_instruction_visitor.h"
#include "ink/core/name/name_pool.h"

#include <cstdint>
#include <limits>
#include <optional>
#include <tuple>
#include <type_traits>
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
          const auto VisitInstruction = [&](const auto &Typed) -> std::optional<std::expected<void, std::string>>
          {
            using InstructionType = std::remove_cvref_t<decltype(Typed)>;
            if constexpr (OpcodeTraits<InstructionType::Operation>::HasDestination)
            {
              if constexpr (std::is_same_v<std::remove_cvref_t<decltype(Typed.getDestination())>, RegisterOperand>)
              {
                const auto &Destination = Typed.getDestination();
                if (!isWritableAddressRegister(Destination.getRegister()) || Destination.getByteOffset() != 0)
                {
                  return std::expected<void, std::string>(std::unexpected("register destination must be BP or SP with zero byte offset"));
                }
              }
            }
            const auto VisitOperands = [&](const auto &...Operands)
            {
              std::expected<void, std::string> Result;
              static_cast<void>(((Result = visitOperand(Operands, Typed.getOpcode())).has_value() && ...));
              return Result;
            };
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
        std::expected<void, std::string> visitOperand(const RegisterOperand &Value, Opcode) const
        {
          if (!isArithmeticRegister(Value.getRegister()))
          {
            return std::unexpected("instruction contains an invalid explicit register");
          }
          return {};
        }

        std::expected<void, std::string> visitOperand(const AddressOperand &Value, Opcode) const
        {
          if (const auto Register = Value.getRegister())
          {
            if (!isAddressRegister(*Register))
            {
              return std::unexpected("address operand contains an invalid base register");
            }
            return {};
          }
          return VisitReference(*Value.getSymbol(), SymbolReferenceTarget::Any);
        }

        std::expected<void, std::string> visitOperand(const ImmediateOperand &Value, Opcode Operation) const
        {
          if (const auto Bits = Value.bits())
          {
            if (Operation == Opcode::CallNativei && *Bits > std::numeric_limits<std::uint32_t>::max())
            {
              return std::unexpected("callNativei import index exceeds u32 range");
            }
            return {};
          }
          if (Operation == Opcode::Calli || Operation == Opcode::CallNativei)
          {
            if (*Value.getByteOffset() != 0)
            {
              return std::unexpected("direct call symbol target must have a zero byte offset");
            }
            return VisitReference(*Value.getSymbol(), Operation == Opcode::CallNativei ? SymbolReferenceTarget::NativeFunction : SymbolReferenceTarget::VMFunction);
          }
          return VisitReference(*Value.getSymbol(), SymbolReferenceTarget::Any);
        }

        std::expected<void, std::string> visitOperand(const BasicBlockOperand &Value, Opcode) const
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
