#include "ink/ir/ir.h"

namespace ink::ir
{
  Value::~Value() = default;
  Instruction::~Instruction() = default;

  const char *typeKindName(TypeKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_TYPE(Name, Spelling) \
  case TypeKind::Name:              \
    return Spelling;
#include "ink/ir/ir.def"
    }
    return "unknown";
  }

  const char *valueKindName(ValueKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_VALUE(Name) \
  case ValueKind::Name:    \
    return #Name;
#include "ink/ir/ir.def"
    }
    return "Unknown";
  }

  const char *instructionKindName(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) \
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
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) \
  case InstructionKind::Name:                          \
    return Mnemonic;
#include "ink/ir/ir.def"
    }
    return "unknown";
  }

  bool isTerminator(InstructionKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) \
  case InstructionKind::Name:                          \
    return Terminator;
#include "ink/ir/ir.def"
    }
    return false;
  }
} // namespace ink::ir
