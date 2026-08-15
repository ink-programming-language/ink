#ifndef INK_IR_IR_H
#define INK_IR_IR_H

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ink::ir
{
  constexpr std::size_t InvalidId = std::numeric_limits<std::size_t>::max();

  class GlobalId
  {
  public:
    constexpr GlobalId() noexcept = default;

    explicit constexpr GlobalId(std::size_t Value) noexcept
        : Value(Value)
    {
    }

    constexpr bool valid() const noexcept
    {
      return Value != InvalidId;
    }

    constexpr std::size_t value() const noexcept
    {
      return Value;
    }

    friend constexpr bool operator==(GlobalId Left, GlobalId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(GlobalId Left, GlobalId Right) noexcept
    {
      return !(Left == Right);
    }

  private:
    std::size_t Value = InvalidId;
  };

  class FunctionId
  {
  public:
    constexpr FunctionId() noexcept = default;

    explicit constexpr FunctionId(std::size_t Value) noexcept
        : Value(Value)
    {
    }

    constexpr bool valid() const noexcept
    {
      return Value != InvalidId;
    }

    constexpr std::size_t value() const noexcept
    {
      return Value;
    }

    friend constexpr bool operator==(FunctionId Left, FunctionId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(FunctionId Left, FunctionId Right) noexcept
    {
      return !(Left == Right);
    }

  private:
    std::size_t Value = InvalidId;
  };

  class ValueId
  {
  public:
    constexpr ValueId() noexcept = default;

    explicit constexpr ValueId(std::size_t Value) noexcept
        : Value(Value)
    {
    }

    constexpr bool valid() const noexcept
    {
      return Value != InvalidId;
    }

    constexpr std::size_t value() const noexcept
    {
      return Value;
    }

    friend constexpr bool operator==(ValueId Left, ValueId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(ValueId Left, ValueId Right) noexcept
    {
      return !(Left == Right);
    }

  private:
    std::size_t Value = InvalidId;
  };

  enum class TypeKind : std::uint8_t
  {
#define INK_IR_TYPE(Name, Spelling) Name,
#include "ink/ir/ir.def"
  };

  const char *typeKindName(TypeKind Kind) noexcept;

  enum class ValueKind : std::uint8_t
  {
#define INK_IR_VALUE(Name) Name,
#include "ink/ir/ir.def"
  };

  const char *valueKindName(ValueKind Kind) noexcept;

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

  class Value
  {
  public:
    virtual ~Value() = 0;

    ValueKind kind() const noexcept
    {
      return Kind;
    }

    TypeKind type() const noexcept
    {
      return Type;
    }

  private:
    Value(ValueKind Kind, TypeKind Type) noexcept
        : Kind(Kind), Type(Type)
    {
    }

    ValueKind Kind;
    TypeKind Type;

#define INK_IR_VALUE(Name) friend class Name;
#include "ink/ir/ir.def"
  };

  class IntegerConstant final : public Value
  {
  public:
    IntegerConstant(TypeKind Type, std::int64_t Integer) noexcept
        : Value(ValueKind::IntegerConstant, Type), Integer(Integer)
    {
    }

    std::int64_t value() const noexcept
    {
      return Integer;
    }

  private:
    std::int64_t Integer;
  };

  class ValueOperand final : public Value
  {
  public:
    ValueOperand(TypeKind Type, ValueId Id) noexcept
        : Value(ValueKind::ValueOperand, Type), Id(Id)
    {
    }

    ValueId id() const noexcept
    {
      return Id;
    }

  private:
    ValueId Id;
  };

  class GlobalAddressOperand final : public Value
  {
  public:
    GlobalAddressOperand(TypeKind Type, GlobalId Global, std::size_t ByteOffset) noexcept
        : Value(ValueKind::GlobalAddressOperand, Type), Global(Global), ByteOffset(ByteOffset)
    {
    }

    GlobalId global() const noexcept
    {
      return Global;
    }

    std::size_t byteOffset() const noexcept
    {
      return ByteOffset;
    }

  private:
    GlobalId Global;
    std::size_t ByteOffset;
  };

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
    CallInstruction() noexcept
        : Instruction(InstructionKind::Call)
    {
    }

    std::optional<ValueId> Result;
    TypeKind ResultType = TypeKind::Void;
    FunctionId Callee;
    std::vector<std::unique_ptr<Value>> Arguments;
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

  struct BasicBlock
  {
    std::string Name;
    std::vector<std::unique_ptr<Instruction>> Instructions;
  };

  enum class FunctionKind : std::uint8_t
  {
    Definition,
    External,
  };

  enum class CallingConvention : std::uint8_t
  {
    Ink,
    C,
  };

  struct Function
  {
    std::string Name;
    FunctionKind Kind = FunctionKind::Definition;
    CallingConvention Convention = CallingConvention::Ink;
    TypeKind ResultType = TypeKind::Void;
    std::vector<TypeKind> ParameterTypes;
    bool HasSideEffects = false;
    std::vector<BasicBlock> Blocks;
  };

  struct ByteConstant
  {
    std::string Name;
    std::string Data;
  };

  struct Module
  {
    std::vector<ByteConstant> ByteConstants;
    std::vector<Function> Functions;
  };
} // namespace ink::ir

#endif
