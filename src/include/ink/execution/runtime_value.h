#ifndef INK_EXECUTION_RUNTIME_VALUE_H
#define INK_EXECUTION_RUNTIME_VALUE_H

#include "ink/core/target_context.h"
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
    FloatingPoint,
    Pointer,
    ByteSlice,
    Aggregate,
  };

  enum class RuntimeMemoryStatus : std::uint8_t
  {
    Ok,
    InvalidValue,
    InvalidRepresentation,
    OutOfBounds,
    AddressOverflow,
    UntrackedPointer,
    LifetimeEnded,
    NotOwned,
    AllocationSizeLimitExceeded,
    AllocationCountLimitExceeded,
    StorageLimitExceeded,
  };

  constexpr std::size_t MaximumRuntimeByteAllocationSize = 16U * 1024U * 1024U;
  constexpr std::size_t MaximumRuntimeByteAllocationCount = 65536U;
  constexpr std::size_t MaximumRuntimeByteStorage = 64U * 1024U * 1024U;

  class RuntimeValue
  {
  public:
    virtual RuntimeValueKind kind() const noexcept = 0;
    virtual const ir::Type &type() const noexcept = 0;
    virtual std::optional<std::uint64_t> integer() const noexcept;
    virtual std::optional<std::uint64_t> floatingPointBits() const noexcept;
    virtual const void *pointer() const noexcept;
    virtual void *mutablePointer() const noexcept;
    virtual std::optional<std::size_t> byteLength() const noexcept;
    virtual bool memoryAlive() const noexcept;
    virtual std::size_t fieldCount() const noexcept;
    virtual const RuntimeValue *field(std::size_t FieldIndex) const noexcept;

  protected:
    RuntimeValue() = default;
    ~RuntimeValue() = default;
  };

  using RuntimeValueRef = const RuntimeValue *;

  bool isValidRuntimeIntegerValue(const ir::Type &ValueType, std::uint64_t Value, const core::TargetContext &Target) noexcept;
  bool isValidRuntimeFloatingPointValue(const ir::Type &ValueType, std::uint64_t Bits) noexcept;
  std::optional<bool> runtimePointersEqual(const RuntimeValue &Left, const RuntimeValue &Right) noexcept;
  std::optional<std::uint64_t> runtimePointerByteOffset(const RuntimeValue &Value) noexcept;

  class RuntimeValueArena
  {
  public:
    RuntimeValueArena();
    explicit RuntimeValueArena(core::TargetContext Target);
    ~RuntimeValueArena();
    RuntimeValueArena(const RuntimeValueArena &) = delete;
    RuntimeValueArena &operator=(const RuntimeValueArena &) = delete;
    RuntimeValueArena(RuntimeValueArena &&) = delete;
    RuntimeValueArena &operator=(RuntimeValueArena &&) = delete;

    RuntimeValueRef voidValue(const ir::Type &ValueType);
    RuntimeValueRef integerValue(const ir::Type &ValueType, std::uint64_t Value);
    RuntimeValueRef floatingPointValue(const ir::Type &ValueType, std::uint64_t Bits);
    RuntimeValueRef pointerValue(const ir::Type &ValueType, const void *Value);
    RuntimeValueRef mutablePointerValue(const ir::Type &ValueType, void *Value);
    RuntimeValueRef borrowedPointerValue(const ir::Type &ValueType, const void *Data, std::size_t Size);
    RuntimeValueRef byteSliceValue(const ir::Type &ValueType, const void *Data, std::size_t Size);
    RuntimeValueRef mutableByteSliceValue(const ir::Type &ValueType, void *Data, std::size_t Size);
    RuntimeValueRef pointerFromByteSlice(const ir::Type &ValueType, const RuntimeValue &Slice);
    RuntimeValueRef allocateByteSlice(const ir::Type &ValueType, std::uint64_t Size, std::size_t OwnerFrame, RuntimeMemoryStatus &Status);
    RuntimeValueRef getElementPointer(const ir::Type &ResultType, const RuntimeValue &Pointer, const ir::Type &ElementType, std::uint64_t Index, RuntimeMemoryStatus &Status);
    RuntimeValueRef getElementPointer(const ir::Type &ResultType, const RuntimeValue &Pointer, const ir::Type &ElementType, std::uint64_t Index, const std::vector<std::uint32_t> &FieldIndices, RuntimeMemoryStatus &Status);
    RuntimeMemoryStatus loadValue(const RuntimeValue &Pointer, const ir::Type &ValueType, RuntimeValueRef &Value);
    RuntimeMemoryStatus storeValue(const RuntimeValue &Pointer, const RuntimeValue &Value);
    RuntimeMemoryStatus endByteSliceLifetime(const RuntimeValue &Slice, std::size_t OwnerFrame) noexcept;
    void endFrameLifetimes(std::size_t OwnerFrame) noexcept;
    RuntimeValueRef aggregateValue(const ir::StructType &ValueType, std::vector<RuntimeValueRef> Fields);
    RuntimeValueRef clone(const RuntimeValue &Value);
    bool owns(RuntimeValueRef Value) const noexcept;

  private:
    class Impl;
    core::TargetContext Target;
    std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
