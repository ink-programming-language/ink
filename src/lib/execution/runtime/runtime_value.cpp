#include "ink/execution/runtime/runtime_value.h"

#include <cassert>
#include <string>
#include <utility>
#include <variant>

namespace ink::execution
{
  struct RuntimePayload
  {
      explicit RuntimePayload(ExecutionInteger Value)
          : Value(std::move(Value))
      {
      }

      explicit RuntimePayload(ExecutionPointer Value)
          : Value(std::move(Value))
      {
      }

      explicit RuntimePayload(std::string_view Value)
          : Value(std::string(Value))
      {
      }

      std::variant<ExecutionInteger, ExecutionPointer, std::string> Value;
  };

  RuntimeValue RuntimeValue::fromBits(std::uint64_t Bits, RuntimeTypeId Type) noexcept
  {
    RuntimeValue Result;
    Result.Type = Type;
    Result.Bits = Bits;
    Result.Initialized = true;
    return Result;
  }

  RuntimeValue RuntimeValue::fromInteger(ExecutionInteger Value, RuntimeTypeId Type)
  {
    if (!Value.valid())
    {
      return {};
    }
    if (Value.bitWidth() <= 64)
    {
      return fromBits(Value.lowWord(), Type);
    }
    RuntimeValue Result = fromBits(0, Type);
    Result.Object = std::make_shared<const RuntimePayload>(std::move(Value));
    return Result;
  }

  RuntimeValue RuntimeValue::fromPointer(ExecutionPointer Value, RuntimeTypeId Type)
  {
    RuntimeValue Result = fromBits(0, Type);
    Result.Object = std::make_shared<const RuntimePayload>(std::move(Value));
    return Result;
  }

  RuntimeValue RuntimeValue::fromString(std::string_view Value, RuntimeTypeId Type)
  {
    RuntimeValue Result = fromBits(0, Type);
    Result.Object = std::make_shared<const RuntimePayload>(Value);
    return Result;
  }

  const ExecutionInteger &RuntimeValue::integer() const noexcept
  {
    assert(kind() == RuntimeKind::Integer && "runtime payload is not a wide integer");
    return *std::get_if<ExecutionInteger>(&Object->Value);
  }

  const ExecutionPointer &RuntimeValue::pointer() const noexcept
  {
    assert(kind() == RuntimeKind::Pointer && "runtime payload is not a pointer");
    return *std::get_if<ExecutionPointer>(&Object->Value);
  }

  std::string_view RuntimeValue::string() const noexcept
  {
    assert(kind() == RuntimeKind::String && "runtime payload is not a string");
    return *std::get_if<std::string>(&Object->Value);
  }

  RuntimeKind RuntimeValue::kind() const noexcept
  {
    if (!Object)
    {
      return RuntimeKind::Invalid;
    }
    switch (Object->Value.index())
    {
    case 0:
      return RuntimeKind::Integer;
    case 1:
      return RuntimeKind::Pointer;
    case 2:
      return RuntimeKind::String;
    default:
      return RuntimeKind::Invalid;
    }
  }
} // namespace ink::execution
