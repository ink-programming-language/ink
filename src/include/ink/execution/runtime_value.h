#ifndef INK_EXECUTION_RUNTIME_VALUE_H
#define INK_EXECUTION_RUNTIME_VALUE_H

#include "ink/ir/type.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace ink::execution
{
  enum class RuntimeValueKind : std::uint8_t
  {
    Void,
    Integer,
    Pointer,
    Aggregate,
  };

  class RuntimeValue
  {
  public:
    virtual RuntimeValueKind kind() const noexcept = 0;
    virtual const ir::Type &type() const noexcept = 0;
    virtual std::optional<std::uint64_t> integer() const noexcept;
    virtual const void *pointer() const noexcept;
    virtual void *mutablePointer() const noexcept;
    virtual std::size_t fieldCount() const noexcept;
    virtual const RuntimeValue *field(std::size_t FieldIndex) const noexcept;

  protected:
    RuntimeValue() = default;
    ~RuntimeValue() = default;
  };

  using RuntimeValueRef = const RuntimeValue *;

  bool isValidRuntimeIntegerValue(const ir::Type &ValueType, std::uint64_t Value) noexcept;

  class RuntimeValueArena
  {
  public:
    RuntimeValueArena();
    ~RuntimeValueArena();
    RuntimeValueArena(const RuntimeValueArena &) = delete;
    RuntimeValueArena &operator=(const RuntimeValueArena &) = delete;
    RuntimeValueArena(RuntimeValueArena &&) = delete;
    RuntimeValueArena &operator=(RuntimeValueArena &&) = delete;

    RuntimeValueRef voidValue(const ir::Type &ValueType);
    RuntimeValueRef integerValue(const ir::Type &ValueType, std::uint64_t Value);
    RuntimeValueRef pointerValue(const ir::Type &ValueType, const void *Value);
    RuntimeValueRef mutablePointerValue(const ir::Type &ValueType, void *Value);
    RuntimeValueRef aggregateValue(const ir::StructType &ValueType, std::vector<RuntimeValueRef> Fields);
    RuntimeValueRef clone(const RuntimeValue &Value);
    bool owns(RuntimeValueRef Value) const noexcept;

  private:
    class Impl;
    std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
