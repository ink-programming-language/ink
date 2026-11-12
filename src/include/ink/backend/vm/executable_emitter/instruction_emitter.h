#pragma once

#include "ink/backend/vm/instruction/register.h"
#include "ink/core/name/name.h"

#include <cstdint>

namespace ink::vm
{
  class ExecutableEmissionContext;
  class FunctionEmissionContext;
  class MachineInstruction;
  class AddressOperand;
  class RegisterOperand;
  class ImmediateOperand;
  class BasicBlockOperand;
  struct DynamicImportSymbol;

  class InstructionEmitter
  {
    public:
      // Both borrowed contexts must outlive this emitter.
      InstructionEmitter(ExecutableEmissionContext &Context, FunctionEmissionContext &FunctionContext) noexcept;

      // Appends the opcode, then dispatches operands in semantic order to padOperand.
      void padInstruction(const MachineInstruction &Instruction);

      // Appends a register identifier and its signed byte offset without dereferencing memory.
      void padOperand(const RegisterOperand &Operand);

      // Appends a numeric immediate or a 64-bit symbol address placeholder with a loading relocation; direct-call targets use opcode-specific encoding.
      void padOperand(const ImmediateOperand &Operand, std::uint32_t Width);

      // Appends a nine-byte address operand; unresolved VM symbols reserve a zero displacement and queue a static relocation.
      void padOperand(const AddressOperand &Operand, std::uint64_t InstructionOffset);

      // Appends a PC-relative basic-block target; unresolved blocks reserve a zero displacement until function finalization.
      void padOperand(const BasicBlockOperand &Operand, std::uint64_t InstructionOffset);

    private:
      void padRegister(RegisterId Register, std::int64_t ByteOffset);
      void padUnresolvedSymbol(RegisterId Register, core::Name Symbol, std::int64_t Addend, std::uint64_t InstructionOffset);
      void padRegisterRelativeSymbol(RegisterId Register, std::uint64_t SymbolOffset, std::int64_t Addend, std::uint64_t InstructionOffset);
      void padNativeSymbol(const DynamicImportSymbol &Import, std::int64_t Addend);

      ExecutableEmissionContext &Context;
      FunctionEmissionContext &FunctionContext;
  };
} // namespace ink::vm
