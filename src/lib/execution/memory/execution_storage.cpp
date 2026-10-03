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

    ExecutionStatus validateArrayElement(const StorageLayout &Layout, const RuntimeValue &Value)
    {
      if (!Value.Initialized || Value.Type != Layout.Type)
      {
        return ExecutionStatus::TypeMismatch;
      }
      switch (Layout.Kind)
      {
      case RuntimeKind::Array:
        if (!Layout.ElementLayout || Value.kind() != RuntimeKind::Array || Value.array().size() != Layout.ElementCount)
        {
          return ExecutionStatus::TypeMismatch;
        }
        for (const RuntimeValue &Element : Value.array())
        {
          const ExecutionStatus Status = validateArrayElement(*Layout.ElementLayout, Element);
          if (Status != ExecutionStatus::Success)
          {
            return Status;
          }
        }
        return ExecutionStatus::Success;
      case RuntimeKind::Boolean:
        return !Value.Object && Value.Bits <= 1 ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      case RuntimeKind::Integer:
        return (Layout.BitWidth > 64 ? Value.kind() == RuntimeKind::Integer && Value.integer().bitWidth() == Layout.BitWidth : !Value.Object) ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      case RuntimeKind::Float:
      case RuntimeKind::Function:
        return !Value.Object ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      case RuntimeKind::Pointer:
        return Value.kind() == RuntimeKind::Pointer ? Value.pointer().status() : ExecutionStatus::TypeMismatch;
      case RuntimeKind::String:
        return Value.kind() == RuntimeKind::String ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      default:
        return ExecutionStatus::TypeMismatch;
      }
    }

    const StorageLayout *findElementLayout(const StorageLayout &Layout, std::size_t Offset, RuntimeTypeId Type, bool AllowOnePast) noexcept
    {
      if (Layout.Type == Type && (Offset == 0 || (AllowOnePast && Offset == Layout.Size)))
      {
        return &Layout;
      }
      if (Layout.Kind != RuntimeKind::Array || !Layout.ElementLayout || Layout.ElementCount == 0 || Offset > Layout.Size || (Layout.Size != 0 && !AllowOnePast && Offset == Layout.Size))
      {
        return nullptr;
      }
      return findElementLayout(*Layout.ElementLayout, Layout.ElementLayout->Size == 0 ? 0 : Offset % Layout.ElementLayout->Size, Type, AllowOnePast);
    }

    const RuntimeValue *findElementValue(const StorageLayout &Layout, const RuntimeValue &Value, std::size_t Offset, RuntimeTypeId Type)
    {
      if (Layout.Type == Type && Offset == 0)
      {
        return &Value;
      }
      if (Value.kind() != RuntimeKind::Array || !Layout.ElementLayout)
      {
        return nullptr;
      }
      const std::size_t Index = Layout.ElementLayout->Size == 0 ? 0 : Offset / Layout.ElementLayout->Size;
      return Index < Value.array().size() ? findElementValue(*Layout.ElementLayout, Value.array()[Index], Layout.ElementLayout->Size == 0 ? 0 : Offset % Layout.ElementLayout->Size, Type) : nullptr;
    }

    RuntimeValue replaceElementValue(const StorageLayout &Layout, const RuntimeValue &OldValue, std::size_t Offset, const RuntimeValue &NewValue)
    {
      if (Layout.Type == NewValue.Type && Offset == 0)
      {
        return NewValue;
      }
      std::vector<RuntimeValue> Elements;
      if (OldValue.kind() == RuntimeKind::Array)
      {
        Elements.assign(OldValue.array().begin(), OldValue.array().end());
      }
      else
      {
        Elements.resize(static_cast<std::size_t>(Layout.ElementCount));
        for (RuntimeValue &Element : Elements)
        {
          Element.Type = Layout.ElementType;
        }
      }
      const std::size_t Index = Layout.ElementLayout->Size == 0 ? 0 : Offset / Layout.ElementLayout->Size;
      Elements[Index] = replaceElementValue(*Layout.ElementLayout, Elements[Index], Layout.ElementLayout->Size == 0 ? 0 : Offset % Layout.ElementLayout->Size, NewValue);
      const bool Initialized = std::all_of(Elements.begin(), Elements.end(), [](const RuntimeValue &Element)
      {
        return Element.Initialized;
      });
      RuntimeValue Result = RuntimeValue::fromArray(std::move(Elements), Layout.Type);
      Result.Initialized = Initialized;
      return Result;
    }

    RuntimeValue emptyArrayValue(const StorageLayout &Layout)
    {
      if (Layout.ElementCount == 0)
      {
        return RuntimeValue::fromArray({}, Layout.Type);
      }
      const RuntimeValue Element = emptyArrayValue(*Layout.ElementLayout);
      return RuntimeValue::fromArray(std::vector<RuntimeValue>(static_cast<std::size_t>(Layout.ElementCount), Element), Layout.Type);
    }

    RuntimeValueResult readNativeArray(const StorageLayout &Layout, const std::byte *Data)
    {
      if (Layout.Kind == RuntimeKind::Array)
      {
        std::vector<RuntimeValue> Elements;
        Elements.reserve(static_cast<std::size_t>(Layout.ElementCount));
        for (std::uint64_t Index = 0; Index < Layout.ElementCount; ++Index)
        {
          RuntimeValueResult Element = readNativeArray(*Layout.ElementLayout, Data + Index * Layout.ElementLayout->Size);
          if (!Element)
          {
            return Element;
          }
          Elements.push_back(std::move(Element.Value));
        }
        return {ExecutionStatus::Success, RuntimeValue::fromArray(std::move(Elements), Layout.Type)};
      }
      std::uint64_t Bits = 0;
      switch (Layout.Size)
      {
      case 1:
        Bits = readNativeBits<std::uint8_t>(Data);
        break;
      case 2:
        Bits = readNativeBits<std::uint16_t>(Data);
        break;
      case 4:
        Bits = readNativeBits<std::uint32_t>(Data);
        break;
      case 8:
        Bits = readNativeBits<std::uint64_t>(Data);
        break;
      default:
        return {ExecutionStatus::UnsupportedOperation};
      }
      if (Layout.Kind == RuntimeKind::Boolean && Bits > 1)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      return {ExecutionStatus::Success, RuntimeValue::fromBits(Bits, Layout.Type)};
    }

    void writeNativeArray(const StorageLayout &Layout, std::byte *Data, const RuntimeValue &Value)
    {
      if (Layout.Kind == RuntimeKind::Array)
      {
        for (std::size_t Index = 0; Index < Value.array().size(); ++Index)
        {
          writeNativeArray(*Layout.ElementLayout, Data + Index * Layout.ElementLayout->Size, Value.array()[Index]);
        }
        return;
      }
      switch (Layout.Size)
      {
      case 1:
      {
        const std::uint8_t Bits = static_cast<std::uint8_t>(Value.Bits);
        std::memcpy(Data, &Bits, sizeof(Bits));
        break;
      }
      case 2:
      {
        const std::uint16_t Bits = static_cast<std::uint16_t>(Value.Bits);
        std::memcpy(Data, &Bits, sizeof(Bits));
        break;
      }
      case 4:
      {
        const std::uint32_t Bits = static_cast<std::uint32_t>(Value.Bits);
        std::memcpy(Data, &Bits, sizeof(Bits));
        break;
      }
      case 8:
        std::memcpy(Data, &Value.Bits, sizeof(Value.Bits));
        break;
      }
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
    // Zero-sized aggregates contain no uninitialized scalar leaves. Equal-typed
    // empty subarrays share an address and an equivalent immutable value.
    if (Layout.Kind == RuntimeKind::Array && Layout.Size == 0 && !Runtime)
    {
      Value = emptyArrayValue(Layout);
    }
  }

  void ExecutionCell::initializeNative() noexcept
  {
    if (Layout.Kind == RuntimeKind::Array)
    {
      NativeArray = std::make_unique<std::byte[]>(std::max<std::size_t>(Layout.Size, 1));
      NativeSize = Layout.Size;
      return;
    }
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
    if (NativeArray)
    {
      return NativeArray.get();
    }
    // A union and its active scalar member have the same address. Its real
    // native object type is established once by initializeNative().
    return NativeType == NativeKind::None ? nullptr : &Native;
  }

  std::size_t ExecutionCell::size() const noexcept
  {
    return Layout.Kind == RuntimeKind::Array ? Layout.Size : NativeSize;
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
    if (NativeArray)
    {
      return readNativeArray(Layout, NativeArray.get());
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
    case RuntimeKind::Array:
    {
      const ExecutionStatus Status = validateArrayElement(Layout, NewValue);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      if (NativeArray)
      {
        writeNativeArray(Layout, NativeArray.get(), NewValue);
      }
      Value = NewValue;
      Initialized = true;
      return ExecutionStatus::Success;
    }
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

  const StorageLayout *ExecutionCell::elementLayout(std::size_t Offset, RuntimeTypeId Type, bool AllowOnePast) const noexcept
  {
    return findElementLayout(Layout, Offset, Type, AllowOnePast);
  }

  RuntimeValueResult ExecutionCell::loadElement(std::size_t Offset, RuntimeTypeId Type) const
  {
    if (Runtime)
    {
      return {ExecutionStatus::RuntimeValue};
    }
    const StorageLayout *Element = elementLayout(Offset, Type);
    if (!Element)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    if (Type == Layout.Type && Offset == 0)
    {
      return loadRuntime();
    }
    const RuntimeValue *Stored = findElementValue(Layout, Value, Offset, Type);
    if (!Stored || !Stored->Initialized)
    {
      return {ExecutionStatus::Uninitialized};
    }
    return NativeArray ? readNativeArray(*Element, NativeArray.get() + Offset) : RuntimeValueResult{ExecutionStatus::Success, *Stored};
  }

  ExecutionStatus ExecutionCell::storeElement(std::size_t Offset, const RuntimeValue &NewValue)
  {
    if (Runtime)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (!Writable && Initialized)
    {
      return ExecutionStatus::ReadOnly;
    }
    const StorageLayout *Element = elementLayout(Offset, NewValue.Type);
    if (!Element)
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (Layout.Type == NewValue.Type && Offset == 0)
    {
      return storeRuntime(NewValue);
    }
    const RuntimeValue *Previous = findElementValue(Layout, Value, Offset, NewValue.Type);
    if (!Writable && Previous && Previous->Initialized)
    {
      return ExecutionStatus::ReadOnly;
    }
    const ExecutionStatus Status = validateArrayElement(*Element, NewValue);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    if (NativeArray && Initialized)
    {
      // Native bytes are authoritative; immutable snapshots are reconstructed
      // on load, so a scalar store never copies the complete initialized array.
      writeNativeArray(*Element, NativeArray.get() + Offset, NewValue);
      return ExecutionStatus::Success;
    }
    Value = replaceElementValue(Layout, Value, Offset, NewValue);
    if (NativeArray)
    {
      writeNativeArray(*Element, NativeArray.get() + Offset, NewValue);
    }
    Initialized = Value.Initialized;
    return ExecutionStatus::Success;
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
