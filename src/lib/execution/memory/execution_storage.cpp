#include "ink/execution/memory/execution_storage.h"

#include <bit>
#include <cstring>
#include <limits>
#include <utility>

namespace ink::execution
{
  static_assert(sizeof(bool) == 1);
  static_assert(sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559);
  static_assert(sizeof(double) == sizeof(std::uint64_t) && std::numeric_limits<double>::is_iec559);

  namespace
  {
    template <typename T>
    T readNativeBits(const void *Data) noexcept
    {
      T Result;
      std::memcpy(&Result, Data, sizeof(Result));
      return Result;
    }
  } // namespace

  ExecutionCell::ExecutionCell(const StorageLayout &Layout, bool Writable, bool Runtime)
      : ExecutionStorage(ExecutionObjectKind::Cell),
        Layout(Layout),
        Writable(Writable),
        Runtime(Runtime)
  {
    if (Layout.Native && !Runtime)
    {
      initializeNative();
    }
  }

  void ExecutionCell::initializeNative() noexcept
  {
    if (Layout.Kind == RuntimeKind::Boolean)
    {
      NativeType = NativeKind::Boolean;
      NativeSize = sizeof(Native.Boolean);
      Native.Boolean = false;
      return;
    }
    if (Layout.Kind == RuntimeKind::Float)
    {
      switch (Layout.BitWidth)
      {
      case 32:
        NativeType = NativeKind::Float32;
        NativeSize = sizeof(Native.Float32);
        Native.Float32 = 0.0F;
        return;
      case 64:
        NativeType = NativeKind::Float64;
        NativeSize = sizeof(Native.Float64);
        Native.Float64 = 0.0;
        return;
      default:
        return;
      }
    }
    if (Layout.Kind != RuntimeKind::Integer)
    {
      return;
    }
    if (Layout.Signed)
    {
      switch (Layout.BitWidth)
      {
      case 8:
        NativeType = NativeKind::Signed8;
        NativeSize = sizeof(Native.Signed8);
        Native.Signed8 = 0;
        return;
      case 16:
        NativeType = NativeKind::Signed16;
        NativeSize = sizeof(Native.Signed16);
        Native.Signed16 = 0;
        return;
      case 32:
        NativeType = NativeKind::Signed32;
        NativeSize = sizeof(Native.Signed32);
        Native.Signed32 = 0;
        return;
      case 64:
        NativeType = NativeKind::Signed64;
        NativeSize = sizeof(Native.Signed64);
        Native.Signed64 = 0;
        return;
      default:
        return;
      }
    }
    switch (Layout.BitWidth)
    {
    case 8:
      NativeType = NativeKind::Unsigned8;
      NativeSize = sizeof(Native.Unsigned8);
      Native.Unsigned8 = 0;
      return;
    case 16:
      NativeType = NativeKind::Unsigned16;
      NativeSize = sizeof(Native.Unsigned16);
      Native.Unsigned16 = 0;
      return;
    case 32:
      NativeType = NativeKind::Unsigned32;
      NativeSize = sizeof(Native.Unsigned32);
      Native.Unsigned32 = 0;
      return;
    case 64:
      NativeType = NativeKind::Unsigned64;
      NativeSize = sizeof(Native.Unsigned64);
      Native.Unsigned64 = 0;
      return;
    default:
      return;
    }
  }

  void *ExecutionCell::data() noexcept
  {
    return const_cast<void *>(std::as_const(*this).data());
  }

  const void *ExecutionCell::data() const noexcept
  {
    // A union and its active scalar member have the same address. Its real
    // native object type is established once by initializeNative().
    return NativeType == NativeKind::None ? nullptr : &Native;
  }

  std::size_t ExecutionCell::size() const noexcept
  {
    return NativeSize;
  }

  RuntimeValueResult ExecutionCell::loadRuntime() const
  {
    if (Runtime)
    {
      return {ExecutionStatus::RuntimeValue};
    }
    if (!Initialized)
    {
      return {ExecutionStatus::Uninitialized};
    }
    if (NativeType == NativeKind::None)
    {
      return {ExecutionStatus::Success, Value};
    }
    std::uint64_t Bits = 0;
    const ExecutionStatus Status = loadBits(Bits);
    return Status == ExecutionStatus::Success ? RuntimeValueResult{Status, RuntimeValue::fromBits(Bits, Layout.Type)} : RuntimeValueResult{Status};
  }

  ExecutionStatus ExecutionCell::storeRuntime(const RuntimeValue &NewValue)
  {
    if (Runtime)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (!Writable && Initialized)
    {
      return ExecutionStatus::ReadOnly;
    }
    if (!NewValue.Initialized || NewValue.Type != Layout.Type)
    {
      return ExecutionStatus::TypeMismatch;
    }
    switch (Layout.Kind)
    {
    case RuntimeKind::Boolean:
      if (NewValue.Object || NewValue.Bits > 1)
      {
        return ExecutionStatus::TypeMismatch;
      }
      break;
    case RuntimeKind::Integer:
      if (Layout.BitWidth > 64 ? NewValue.kind() != RuntimeKind::Integer || NewValue.integer().bitWidth() != Layout.BitWidth : static_cast<bool>(NewValue.Object))
      {
        return ExecutionStatus::TypeMismatch;
      }
      break;
    case RuntimeKind::Float:
    case RuntimeKind::Void:
    case RuntimeKind::Function:
      if (NewValue.Object)
      {
        return ExecutionStatus::TypeMismatch;
      }
      break;
    case RuntimeKind::Pointer:
      if (NewValue.kind() != RuntimeKind::Pointer)
      {
        return ExecutionStatus::TypeMismatch;
      }
      if (const ExecutionStatus Status = NewValue.pointer().status(); Status != ExecutionStatus::Success)
      {
        return Status;
      }
      break;
    case RuntimeKind::String:
      if (NewValue.kind() != RuntimeKind::String)
      {
        return ExecutionStatus::TypeMismatch;
      }
      break;
    case RuntimeKind::Invalid:
      return ExecutionStatus::TypeMismatch;
    }
    if (NativeType != NativeKind::None)
    {
      return storeBits(NewValue.Bits);
    }
    Value = NewValue;
    Initialized = true;
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionCell::loadBits(std::uint64_t &Bits) const noexcept
  {
    return loadBits(Bits, NativeSize);
  }

  ExecutionStatus ExecutionCell::loadBits(std::uint64_t &Bits, std::size_t Width) const noexcept
  {
    if (Runtime)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (!Initialized)
    {
      return ExecutionStatus::Uninitialized;
    }
    if (Width != NativeSize)
    {
      return ExecutionStatus::TypeMismatch;
    }
    switch (Width)
    {
    case 1:
      Bits = readNativeBits<std::uint8_t>(data());
      break;
    case 2:
      Bits = readNativeBits<std::uint16_t>(data());
      break;
    case 4:
      Bits = readNativeBits<std::uint32_t>(data());
      break;
    case 8:
      Bits = readNativeBits<std::uint64_t>(data());
      break;
    default:
      return ExecutionStatus::UnsupportedOperation;
    }
    return NativeType == NativeKind::Boolean && Bits > 1 ? ExecutionStatus::TypeMismatch : ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionCell::storeBits(std::uint64_t Bits, std::size_t Width) noexcept
  {
    if (Runtime)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (!Writable && Initialized)
    {
      return ExecutionStatus::ReadOnly;
    }
    if (Width != NativeSize || (NativeType == NativeKind::Boolean && Bits > 1))
    {
      return ExecutionStatus::TypeMismatch;
    }
    switch (Width)
    {
    case 1:
    {
      const std::uint8_t Narrow = static_cast<std::uint8_t>(Bits);
      std::memcpy(data(), &Narrow, sizeof(Narrow));
      break;
    }
    case 2:
    {
      const std::uint16_t Narrow = static_cast<std::uint16_t>(Bits);
      std::memcpy(data(), &Narrow, sizeof(Narrow));
      break;
    }
    case 4:
    {
      const std::uint32_t Narrow = static_cast<std::uint32_t>(Bits);
      std::memcpy(data(), &Narrow, sizeof(Narrow));
      break;
    }
    case 8:
      std::memcpy(data(), &Bits, sizeof(Bits));
      break;
    default:
      return ExecutionStatus::UnsupportedOperation;
    }
    Initialized = true;
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionCell::storeBits(std::uint64_t Bits) noexcept
  {
    if (Runtime)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (!Writable && Initialized)
    {
      return ExecutionStatus::ReadOnly;
    }
    switch (NativeType)
    {
    case NativeKind::Boolean:
      if (Bits > 1)
      {
        return ExecutionStatus::TypeMismatch;
      }
      Native.Boolean = Bits != 0;
      break;
    case NativeKind::Signed8:
      Native.Signed8 = std::bit_cast<std::int8_t>(static_cast<std::uint8_t>(Bits));
      break;
    case NativeKind::Unsigned8:
      Native.Unsigned8 = static_cast<std::uint8_t>(Bits);
      break;
    case NativeKind::Signed16:
      Native.Signed16 = std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(Bits));
      break;
    case NativeKind::Unsigned16:
      Native.Unsigned16 = static_cast<std::uint16_t>(Bits);
      break;
    case NativeKind::Signed32:
      Native.Signed32 = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(Bits));
      break;
    case NativeKind::Unsigned32:
      Native.Unsigned32 = static_cast<std::uint32_t>(Bits);
      break;
    case NativeKind::Signed64:
      Native.Signed64 = std::bit_cast<std::int64_t>(Bits);
      break;
    case NativeKind::Unsigned64:
      Native.Unsigned64 = Bits;
      break;
    case NativeKind::Float32:
    {
      const std::uint32_t FloatBits = static_cast<std::uint32_t>(Bits);
      std::memcpy(&Native.Float32, &FloatBits, sizeof(FloatBits));
      break;
    }
    case NativeKind::Float64:
    {
      std::memcpy(&Native.Float64, &Bits, sizeof(Bits));
      break;
    }
    case NativeKind::None:
      return ExecutionStatus::UnsupportedOperation;
    }
    Initialized = true;
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
