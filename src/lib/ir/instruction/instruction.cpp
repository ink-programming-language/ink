#include "ink/ir/instruction/instruction.h"

#include "ink/ir/instruction/arithmetic.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/instruction/memory.h"

namespace ink::ir
{
  Instruction::~Instruction() = default;

  const char *instructionKindName(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) \
  case InstructionKind::Name:                          \
    return #Name;
#include "ink/ir/ir.def"
    }
    return "Unknown";
  }

  const char *instructionMnemonic(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) \
  case InstructionKind::Name:                          \
    return Mnemonic;
#include "ink/ir/ir.def"
    }
    return "unknown";
  }

  std::optional<InstructionKind> instructionKindFromMnemonic(std::string_view Mnemonic) noexcept
  {
#define INK_IR_INSTRUCTION(Name, Spelling, Terminator, ResultPolicy) \
  if (Mnemonic == Spelling)                                           \
  {                                                                   \
    return InstructionKind::Name;                                     \
  }
#include "ink/ir/ir.def"
    return std::nullopt;
  }

  InstructionResultPolicy instructionResultPolicy(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) \
  case InstructionKind::Name:                                        \
    return InstructionResultPolicy::ResultPolicy;
#include "ink/ir/ir.def"
    }
    return InstructionResultPolicy::Forbidden;
  }

  std::optional<ValueId> instructionResultId(const Instruction &InstructionValue) noexcept
  {
    switch (InstructionValue.kind())
    {
#define INK_IR_INSTRUCTION_RESULT_Forbidden(Name) return std::nullopt;
#define INK_IR_INSTRUCTION_RESULT_Required(Name) return static_cast<const Name##Instruction &>(InstructionValue).Result;
#define INK_IR_INSTRUCTION_RESULT_Optional(Name) return static_cast<const Name##Instruction &>(InstructionValue).Result;
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) \
  case InstructionKind::Name:                                        \
    INK_IR_INSTRUCTION_RESULT_##ResultPolicy(Name)
#include "ink/ir/ir.def"
#undef INK_IR_INSTRUCTION_RESULT_Forbidden
#undef INK_IR_INSTRUCTION_RESULT_Required
#undef INK_IR_INSTRUCTION_RESULT_Optional
    }
    return std::nullopt;
  }

  bool isTerminator(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) \
  case InstructionKind::Name:                          \
    return Terminator;
#include "ink/ir/ir.def"
    }
    return false;
  }
} // namespace ink::ir
