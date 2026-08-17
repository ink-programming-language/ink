#ifndef INK_IR_ID_H
#define INK_IR_ID_H

#include <cstddef>
#include <limits>

namespace ink::ir
{
  constexpr std::size_t InvalidId = std::numeric_limits<std::size_t>::max();

  class ModuleId
  {
  public:
    constexpr ModuleId() noexcept = default;

    explicit constexpr ModuleId(std::size_t Value) noexcept
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

    friend constexpr bool operator==(ModuleId Left, ModuleId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(ModuleId Left, ModuleId Right) noexcept
    {
      return !(Left == Right);
    }

  private:
    std::size_t Value = InvalidId;
  };

  class ByteConstantId
  {
  public:
    constexpr ByteConstantId() noexcept = default;

    explicit constexpr ByteConstantId(std::size_t Value) noexcept
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

    friend constexpr bool operator==(ByteConstantId Left, ByteConstantId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(ByteConstantId Left, ByteConstantId Right) noexcept
    {
      return !(Left == Right);
    }

  private:
    std::size_t Value = InvalidId;
  };

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

  class BlockId
  {
  public:
    constexpr BlockId() noexcept = default;

    explicit constexpr BlockId(std::size_t Value) noexcept
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

    friend constexpr bool operator==(BlockId Left, BlockId Right) noexcept
    {
      return Left.Value == Right.Value;
    }

    friend constexpr bool operator!=(BlockId Left, BlockId Right) noexcept
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

  struct FunctionRef
  {
    constexpr FunctionRef() noexcept = default;

    constexpr FunctionRef(FunctionId Function) noexcept
        : Function(Function)
    {
    }

    constexpr FunctionRef(ModuleId Module, FunctionId Function) noexcept
        : Module(Module),
          Function(Function)
    {
    }

    constexpr FunctionRef &operator=(FunctionId NewFunction) noexcept
    {
      Module = ModuleId{};
      Function = NewFunction;
      return *this;
    }

    constexpr bool valid() const noexcept
    {
      return Function.valid();
    }

    constexpr bool isQualified() const noexcept
    {
      return Module.valid();
    }

    constexpr std::size_t value() const noexcept
    {
      return Function.value();
    }

    friend constexpr bool operator==(FunctionRef Left, FunctionRef Right) noexcept
    {
      return Left.Module == Right.Module && Left.Function == Right.Function;
    }

    friend constexpr bool operator!=(FunctionRef Left, FunctionRef Right) noexcept
    {
      return !(Left == Right);
    }

    ModuleId Module;
    FunctionId Function;
  };

  struct GlobalRef
  {
    constexpr GlobalRef() noexcept = default;

    constexpr GlobalRef(GlobalId Global) noexcept
        : Global(Global)
    {
    }

    constexpr GlobalRef(ModuleId Module, GlobalId Global) noexcept
        : Module(Module),
          Global(Global)
    {
    }

    constexpr bool valid() const noexcept
    {
      return Global.valid();
    }

    constexpr bool isQualified() const noexcept
    {
      return Module.valid();
    }

    friend constexpr bool operator==(GlobalRef Left, GlobalRef Right) noexcept
    {
      return Left.Module == Right.Module && Left.Global == Right.Global;
    }

    friend constexpr bool operator!=(GlobalRef Left, GlobalRef Right) noexcept
    {
      return !(Left == Right);
    }

    ModuleId Module;
    GlobalId Global;
  };
} // namespace ink::ir

#endif
