#ifndef INK_IR_INSTRUCTION_CALL_INSTRUCTION_H
#define INK_IR_INSTRUCTION_CALL_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"
#include "ink/ir/param/argument.h"

#include <cstddef>
#include <vector>

namespace ink::ir
{
  class Function;
  class FunctionType;

  // Owns the actual argument records; borrows the signature, callee and argument values.
  // The explicit signature supports indirect calls and must match the resolved callee and arguments.
  class CallInstruction final : public Instruction
  {
    public:
      CallInstruction(const FunctionType &Signature, const Value &Callee, std::vector<Argument> Arguments = {}) noexcept;
      explicit CallInstruction(const Function &Callee, std::vector<Argument> Arguments = {}) noexcept;

      const FunctionType *functionType() const noexcept;
      const Value *callee() const noexcept;
      // Returns nullptr for an indirect call.
      const Function *directCallee() const noexcept;
      const std::vector<Argument> &arguments() const noexcept;
      std::size_t argumentCount() const noexcept;
      // Returns nullptr outside the argument list.
      const Argument *argument(std::size_t Index) const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const FunctionType *Signature = nullptr;
      const Value *Callee = nullptr;
      std::vector<Argument> Arguments;
  };
} // namespace ink::ir

#endif
