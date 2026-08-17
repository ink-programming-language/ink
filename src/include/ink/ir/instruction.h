#ifndef INK_IR_INSTRUCTION_H
#define INK_IR_INSTRUCTION_H

#include "ink/ir/id.h"
#include "ink/ir/type.h"
#include "ink/ir/value.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace ink::ir
{
  enum class InstructionKind : std::uint8_t
  {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) Name,
#include "ink/ir/ir.def"
  };

  const char *instructionKindName(InstructionKind Kind) noexcept;
  const char *instructionMnemonic(InstructionKind Kind) noexcept;
  bool isTerminator(InstructionKind Kind) noexcept;

#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) class Name##Instruction;
#include "ink/ir/ir.def"

  class Instruction
  {
  public:
    virtual ~Instruction() = 0;

    InstructionKind kind() const noexcept
    {
      return Kind;
    }

  private:
    explicit Instruction(InstructionKind Kind) noexcept
        : Kind(Kind)
    {
    }

    InstructionKind Kind;

#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator) friend class Name##Instruction;
#include "ink/ir/ir.def"
  };

  class CallInstruction final : public Instruction
  {
  public:
    explicit CallInstruction(const Type &ResultType) noexcept
        : Instruction(InstructionKind::Call),
          ResultType(&ResultType)
    {
    }

    std::optional<ValueId> Result;
    const Type *ResultType;
    FunctionRef Callee;
    std::vector<std::unique_ptr<Value>> Arguments;
  };

  class ImportInstruction final : public Instruction
  {
  public:
    ImportInstruction() noexcept
        : Instruction(InstructionKind::Import)
    {
    }

    explicit ImportInstruction(ModuleId Module) noexcept
        : Instruction(InstructionKind::Import),
          Module(Module)
    {
    }

    ModuleId Module;
  };

  class ReturnInstruction final : public Instruction
  {
  public:
    ReturnInstruction() noexcept
        : Instruction(InstructionKind::Return)
    {
    }

    std::unique_ptr<Value> ReturnValue;
  };

  class InsertValueInstruction final : public Instruction
  {
  public:
    explicit InsertValueInstruction(const Type &ResultType) noexcept
        : Instruction(InstructionKind::InsertValue),
          ResultType(&ResultType)
    {
    }

    ValueId Result;
    const Type *ResultType;
    std::unique_ptr<Value> Aggregate;
    std::unique_ptr<Value> Element;
    std::size_t FieldIndex = 0;
  };

  class ExtractValueInstruction final : public Instruction
  {
  public:
    explicit ExtractValueInstruction(const Type &ResultType) noexcept
        : Instruction(InstructionKind::ExtractValue),
          ResultType(&ResultType)
    {
    }

    ValueId Result;
    const Type *ResultType;
    std::unique_ptr<Value> Aggregate;
    std::size_t FieldIndex = 0;
  };
} // namespace ink::ir

#endif
