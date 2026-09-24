#ifndef INK_SEMANTIC_MODEL_CALL_INSTRUCTION_H
#define INK_SEMANTIC_MODEL_CALL_INSTRUCTION_H

#include "ink/semantic/model/function.h"
#include "ink/semantic/model/instruction/instruction.h"
#include "ink/semantic/model/type/function_type.h"

#include <span>
#include <vector>

namespace ink::semantic
{
  // One checked call occurrence; targets and arguments are borrowed context-local objects.
  // Arguments have already been mapped and converted to the signature's parameter order.
  class CallInstruction final : public Instruction
  {
    public:
      const Type &type() const noexcept override
      {
        return functionType().returnType();
      }

      const FunctionType &functionType() const noexcept
      {
        return static_cast<const FunctionType &>(Callee.type());
      }

      const Value &callee() const noexcept
      {
        return Callee;
      }

      // Exactly one of directCallee() and indirectCallee() is non-null.
      const Function *directCallee() const noexcept
      {
        return Function::classof(&Callee) ? static_cast<const Function *>(&Callee) : nullptr;
      }

      const Value *indirectCallee() const noexcept
      {
        return directCallee() ? nullptr : &Callee;
      }

      std::span<const Value *const> arguments() const noexcept
      {
        return Arguments;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::CallInstruction;
      }

    private:
      CallInstruction(const Value &Callee, std::span<const Value *const> Arguments)
          : Instruction(ValueKind::CallInstruction),
            Callee(Callee),
            Arguments(Arguments.begin(), Arguments.end())
      {
      }

      const Value &Callee;
      std::vector<const Value *> Arguments;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
