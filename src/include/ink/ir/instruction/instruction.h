#ifndef INK_IR_INSTRUCTION_H
#define INK_IR_INSTRUCTION_H

#include "ink/ir/model/id.h"
#include "ink/ir/model/name.h"
#include "ink/ir/model/type.h"
#include "ink/ir/model/value.h"
#include "ink/ir/model/value_handle.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::ir
{
  class Instruction;

  enum class InstructionKind : std::uint8_t
  {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) Name,
#include "ink/ir/ir.def"
  };

  enum class InstructionResultPolicy : std::uint8_t
  {
    Forbidden,
    Required,
    Optional,
  };

  const char *instructionKindName(InstructionKind Kind) noexcept;
  const char *instructionMnemonic(InstructionKind Kind) noexcept;
  std::optional<InstructionKind> instructionKindFromMnemonic(std::string_view Mnemonic) noexcept;
  InstructionResultPolicy instructionResultPolicy(InstructionKind Kind) noexcept;
  std::optional<ValueId> instructionResultId(const Instruction &InstructionValue) noexcept;
  bool isTerminator(InstructionKind Kind) noexcept;

#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) class Name##Instruction;
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

#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) friend class Name##Instruction;
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
      FunctionId Callee;
      std::vector<ValueHandle> Arguments;
  };

  class ImportInstruction final : public Instruction
  {
    public:
      ImportInstruction() noexcept
          : Instruction(InstructionKind::Import)
      {
      }

      explicit ImportInstruction(Name Module)
          : Instruction(InstructionKind::Import),
            Module(std::move(Module))
      {
      }

      Name Module;
  };

  class ReturnInstruction final : public Instruction
  {
    public:
      ReturnInstruction() noexcept
          : Instruction(InstructionKind::Return)
      {
      }

      ValueHandle ReturnValue;
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
      ValueHandle Aggregate;
      ValueHandle Element;
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
      ValueHandle Aggregate;
      std::size_t FieldIndex = 0;
  };
} // namespace ink::ir

#endif
