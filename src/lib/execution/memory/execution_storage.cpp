#include "ink/execution/memory/execution_storage.h"
#include "ink/execution/memory/execution_memory_manager.h"

#include <bit>
#include <array>
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

    ExecutionStatus validateArrayElement(const TypeDesc &Layout, const RuntimeValue &Value)
    {
      if (!Value.Initialized || Value.Type != Layout.Type)
      {
        return ExecutionStatus::TypeMismatch;
      }
      switch (Layout.Kind)
      {
      case RuntimeKind::Void:
        return !Value.Object && Value.Bits == 0 ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      case RuntimeKind::Class:
        if (Value.kind() != RuntimeKind::Class || Value.fields().size() != Layout.classDesc().Fields.size())
        {
          return ExecutionStatus::TypeMismatch;
        }
        for (std::size_t Index = 0; Index < Value.fields().size(); ++Index)
        {
          const ExecutionStatus Status = validateArrayElement(*Layout.classDesc().Fields[Index].Layout, Value.fields()[Index]);
          if (Status != ExecutionStatus::Success)
          {
            return Status;
          }
        }
        return ExecutionStatus::Success;
      case RuntimeKind::Array:
        if (!Layout.arrayDesc().ElementLayout || Value.kind() != RuntimeKind::Array || Value.array().size() != Layout.arrayDesc().ElementCount)
        {
          return ExecutionStatus::TypeMismatch;
        }
        for (const RuntimeValue &Element : Value.array())
        {
          const ExecutionStatus Status = validateArrayElement(*Layout.arrayDesc().ElementLayout, Element);
          if (Status != ExecutionStatus::Success)
          {
            return Status;
          }
        }
        return ExecutionStatus::Success;
      case RuntimeKind::Boolean:
        return !Value.Object && Value.Bits <= 1 ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      case RuntimeKind::Integer:
        return (Layout.bitWidth() > 64 ? Value.kind() == RuntimeKind::Integer && Value.integer().bitWidth() == Layout.bitWidth() : !Value.Object) ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      case RuntimeKind::Float:
      case RuntimeKind::Function:
        return !Value.Object ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      case RuntimeKind::Pointer:
        return Layout.Size != sizeof(void *) ? ExecutionStatus::UnsupportedOperation : Value.kind() == RuntimeKind::Pointer ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      case RuntimeKind::String:
        return Layout.Size != sizeof(void *) + sizeof(std::size_t) ? ExecutionStatus::UnsupportedOperation : Value.kind() == RuntimeKind::String ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
      default:
        return ExecutionStatus::TypeMismatch;
      }
    }

    const TypeDesc *findElementLayout(const TypeDesc &Layout, std::size_t Offset, RuntimeTypeId Type, bool AllowOnePast) noexcept
    {
      if (Layout.Type == Type && (Offset == 0 || (AllowOnePast && Offset == Layout.Size)))
      {
        return &Layout;
      }
      if (Layout.Kind == RuntimeKind::Class)
      {
        for (std::size_t Index = 0; Index < Layout.classDesc().Fields.size(); ++Index)
        {
          const std::size_t Start = Layout.classDesc().Fields[Index].Offset;
          const TypeDesc &Field = *Layout.classDesc().Fields[Index].Layout;
          if (Offset >= Start && Offset - Start <= Field.Size)
          {
            if (const TypeDesc *Found = findElementLayout(Field, Offset - Start, Type, AllowOnePast))
            {
              return Found;
            }
          }
        }
        return nullptr;
      }
      if (Layout.Kind != RuntimeKind::Array || !Layout.arrayDesc().ElementLayout || Layout.arrayDesc().ElementCount == 0 || Offset > Layout.Size || (Layout.Size != 0 && !AllowOnePast && Offset == Layout.Size))
      {
        return nullptr;
      }
      return findElementLayout(*Layout.arrayDesc().ElementLayout, Layout.arrayDesc().ElementLayout->Size == 0 ? 0 : Offset % Layout.arrayDesc().ElementLayout->Size, Type, AllowOnePast);
    }

    RuntimeValueResult readNativeArray(const TypeDesc &Layout, const std::byte *Data)
    {
      if (Layout.Kind == RuntimeKind::Void)
      {
        return {ExecutionStatus::Success, RuntimeValue::fromBits(0, Layout.Type)};
      }
      if (Layout.Kind == RuntimeKind::Class)
      {
        std::vector<RuntimeValue> Fields;
        for (std::size_t Index = 0; Index < Layout.classDesc().Fields.size(); ++Index)
        {
          RuntimeValueResult Field = readNativeArray(*Layout.classDesc().Fields[Index].Layout, Data + Layout.classDesc().Fields[Index].Offset);
          if (!Field)
          {
            return Field;
          }
          Fields.push_back(std::move(Field.Value));
        }
        return {ExecutionStatus::Success, RuntimeValue::fromClass(std::move(Fields), Layout.Type)};
      }
      if (Layout.Kind == RuntimeKind::Array)
      {
        std::vector<RuntimeValue> Elements;
        Elements.reserve(static_cast<std::size_t>(Layout.arrayDesc().ElementCount));
        for (std::uint64_t Index = 0; Index < Layout.arrayDesc().ElementCount; ++Index)
        {
          RuntimeValueResult Element = readNativeArray(*Layout.arrayDesc().ElementLayout, Data + Index * Layout.arrayDesc().ElementLayout->Size);
          if (!Element)
          {
            return Element;
          }
          Elements.push_back(std::move(Element.Value));
        }
        return {ExecutionStatus::Success, RuntimeValue::fromArray(std::move(Elements), Layout.Type)};
      }
      if (Layout.Kind == RuntimeKind::Pointer)
      {
        if (Layout.Size != sizeof(void *))
        {
          return {ExecutionStatus::UnsupportedOperation};
        }
        return {ExecutionStatus::Success, RuntimeValue::fromPointer(ExecutionPointer::fromNative(readNativeBits<void *>(Data)), Layout.Type)};
      }
      if (Layout.Kind == RuntimeKind::String)
      {
        if (Layout.Size != sizeof(void *) + sizeof(std::size_t))
        {
          return {ExecutionStatus::UnsupportedOperation};
        }
        const char *Bytes = readNativeBits<const char *>(Data);
        const std::size_t Length = readNativeBits<std::size_t>(Data + sizeof(void *));
        return {ExecutionStatus::Success, RuntimeValue::fromString(Length ? std::string_view(Bytes, Length) : std::string_view{}, Layout.Type)};
      }
      const std::size_t Count = Layout.Kind == RuntimeKind::Integer ? (static_cast<std::size_t>(Layout.bitWidth()) + 7) / 8 : Layout.Size;
      if (Count > Layout.Size)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      if (Layout.Kind == RuntimeKind::Integer && Layout.bitWidth() > 64)
      {
        std::vector<std::uint64_t> Words((static_cast<std::size_t>(Layout.bitWidth()) + 63) / 64);
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          const std::size_t Byte = std::endian::native == std::endian::little ? Index : Count - 1 - Index;
          Words[Index / 8] |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(Data[Byte])) << ((Index % 8) * 8);
        }
        return {ExecutionStatus::Success, RuntimeValue::fromInteger(ExecutionInteger(ir::IntegerBits(Layout.bitWidth(), Words)), Layout.Type)};
      }
      if (Count > sizeof(std::uint64_t))
      {
        return {ExecutionStatus::UnsupportedOperation};
      }
      std::uint64_t Bits = 0;
      for (std::size_t Index = 0; Index < Count; ++Index)
      {
        const std::size_t Byte = std::endian::native == std::endian::little ? Index : Count - 1 - Index;
        Bits |= static_cast<std::uint64_t>(std::to_integer<unsigned char>(Data[Byte])) << (Index * 8);
      }
      if (Layout.Kind == RuntimeKind::Integer && Layout.bitWidth() < 64)
      {
        Bits &= (std::uint64_t{1} << Layout.bitWidth()) - 1;
      }
      if (Layout.Kind == RuntimeKind::Boolean && Bits > 1)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      return {ExecutionStatus::Success, RuntimeValue::fromBits(Bits, Layout.Type)};
    }

    void writeNativeArray(const TypeDesc &Layout, std::byte *Data, const RuntimeValue &Value)
    {
      if (Layout.Kind == RuntimeKind::Class)
      {
        for (std::size_t Index = 0; Index < Value.fields().size(); ++Index)
        {
          writeNativeArray(*Layout.classDesc().Fields[Index].Layout, Data + Layout.classDesc().Fields[Index].Offset, Value.fields()[Index]);
        }
        return;
      }
      if (Layout.Kind == RuntimeKind::Array)
      {
        for (std::size_t Index = 0; Index < Value.array().size(); ++Index)
        {
          writeNativeArray(*Layout.arrayDesc().ElementLayout, Data + Index * Layout.arrayDesc().ElementLayout->Size, Value.array()[Index]);
        }
        return;
      }
      if (Layout.Kind == RuntimeKind::Pointer)
      {
        void *Pointer = Value.pointer().address();
        std::memcpy(Data, &Pointer, sizeof(Pointer));
        return;
      }
      if (Layout.Kind == RuntimeKind::String)
      {
        const std::string_view String = Value.string();
        const char *Pointer = String.data();
        const std::size_t Length = String.size();
        std::memcpy(Data, &Pointer, sizeof(Pointer));
        std::memcpy(Data + sizeof(Pointer), &Length, sizeof(Length));
        return;
      }
      const std::size_t Count = Layout.Kind == RuntimeKind::Integer ? (static_cast<std::size_t>(Layout.bitWidth()) + 7) / 8 : Layout.Size;
      std::memset(Data, 0, Layout.Size);
      const bool Wide = Layout.Kind == RuntimeKind::Integer && Layout.bitWidth() > 64;
      const ir::IntegerBits Integer = Wide ? Value.integer().bits() : ir::IntegerBits(64, Value.Bits);
      for (std::size_t Index = 0; Index < Count; ++Index)
      {
        const std::size_t Byte = std::endian::native == std::endian::little ? Index : Count - 1 - Index;
        Data[Byte] = static_cast<std::byte>(Integer.words()[Index / 8] >> ((Index % 8) * 8));
      }
    }
  } // namespace

  ExecutionStatus validateStorageValue(const TypeDesc &Layout, const RuntimeValue &Value)
  {
    return validateArrayElement(Layout, Value);
  }

  RuntimeValueResult readStorage(const TypeDesc &Layout, const void *Address)
  {
    return readNativeArray(Layout, static_cast<const std::byte *>(Address));
  }

  ExecutionStatus writeStorage(const TypeDesc &Layout, void *Address, const RuntimeValue &Value)
  {
    const ExecutionStatus Status = validateArrayElement(Layout, Value);
    if (Status == ExecutionStatus::Success)
    {
      writeNativeArray(Layout, static_cast<std::byte *>(Address), Value);
    }
    return Status;
  }

  ExecutionCell::ExecutionCell(ExecutionMemoryManager &Owner, const TypeDesc &Layout, bool Writable, bool Runtime)
      : ExecutionStorage(ExecutionObjectKind::Cell),
        Layout(Layout),
        Owner(Owner),
        Bytes(allocateRuntimeBytes(Layout.Size, Layout.Alignment)),
        Writable(Writable),
        Runtime(Runtime),
        Initialized(!Runtime && Layout.Kind == RuntimeKind::Array && Layout.Size == 0)
  {
  }

  void *ExecutionCell::data() noexcept
  {
    return Bytes.get();
  }

  const void *ExecutionCell::data() const noexcept
  {
    return Bytes.get();
  }

  std::size_t ExecutionCell::size() const noexcept
  {
    return Layout.Size;
  }

  const TypeDesc *ExecutionCell::elementLayout(std::size_t Offset, RuntimeTypeId Type, bool AllowOnePast) const noexcept
  {
    return findElementLayout(Layout, Offset, Type, AllowOnePast);
  }

  bool ExecutionCell::isInitialized(std::size_t Offset, const TypeDesc &Element) const noexcept
  {
    if (Initialized || Element.Size == 0)
    {
      return true;
    }
    for (const InitializedRange &Range : InitializedRanges)
    {
      if (Offset >= Range.Start && Offset + Element.Size <= Range.End)
      {
        return true;
      }
    }
    if (Element.Kind == RuntimeKind::Array)
    {
      for (std::uint64_t Index = 0; Index < Element.arrayDesc().ElementCount; ++Index)
      {
        if (!isInitialized(Offset + static_cast<std::size_t>(Index) * Element.arrayDesc().ElementLayout->Size, *Element.arrayDesc().ElementLayout))
        {
          return false;
        }
      }
      return true;
    }
    if (Element.Kind == RuntimeKind::Class && !Element.classDesc().Fields.empty())
    {
      for (std::size_t Index = 0; Index < Element.classDesc().Fields.size(); ++Index)
      {
        if (!isInitialized(Offset + Element.classDesc().Fields[Index].Offset, *Element.classDesc().Fields[Index].Layout))
        {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  void ExecutionCell::markInitialized(std::size_t Offset, const TypeDesc &Element)
  {
    if (Initialized)
    {
      return;
    }
    if (Offset == 0 && Element.Domain == Layout.Domain && Element.Type == Layout.Type)
    {
      Initialized = true;
    }
    else
    {
      InitializedRange Added{Offset, Offset + Element.Size};
      auto Current = InitializedRanges.begin();
      while (Current != InitializedRanges.end())
      {
        if (Added.Start <= Current->End && Current->Start <= Added.End)
        {
          Added.Start = std::min(Added.Start, Current->Start);
          Added.End = std::max(Added.End, Current->End);
          Current = InitializedRanges.erase(Current);
        }
        else
        {
          ++Current;
        }
      }
      InitializedRanges.push_back(Added);
      Initialized = isInitialized(0, Layout);
    }
    if (Initialized)
    {
      InitializedRanges.clear();
    }
  }

  ExecutionStatus ExecutionCell::loadBytes(std::size_t Offset, const TypeDesc &Element, void *Destination) const
  {
    if (Runtime)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (Offset > Layout.Size || Element.Size > Layout.Size - Offset)
    {
      return Element.Kind == RuntimeKind::Integer && Element.bitWidth() == 8 ? ExecutionStatus::InvalidPlace : ExecutionStatus::TypeMismatch;
    }
    if (!isInitialized(Offset, Element))
    {
      return ExecutionStatus::Uninitialized;
    }
    std::memmove(Destination, Bytes.get() + Offset, Element.Size);
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionCell::storeBytes(std::size_t Offset, const TypeDesc &Element, const void *Source)
  {
    if (Runtime)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (Offset > Layout.Size || Element.Size > Layout.Size - Offset)
    {
      return Element.Kind == RuntimeKind::Integer && Element.bitWidth() == 8 ? ExecutionStatus::InvalidPlace : ExecutionStatus::TypeMismatch;
    }
    if (!Writable && isInitialized(Offset, Element))
    {
      return ExecutionStatus::ReadOnly;
    }
    std::memmove(Bytes.get() + Offset, Source, Element.Size);
    markInitialized(Offset, Element);
    return ExecutionStatus::Success;
  }

  RuntimeValueResult ExecutionCell::loadRuntime() const
  {
    return loadElement(0, Layout.Type);
  }

  RuntimeValueResult ExecutionCell::loadElement(std::size_t Offset, RuntimeTypeId Type) const
  {
    if (Runtime)
    {
      return {ExecutionStatus::RuntimeValue};
    }
    const TypeDesc *Element = elementLayout(Offset, Type);
    if (!Element)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    if (!isInitialized(Offset, *Element))
    {
      return {ExecutionStatus::Uninitialized};
    }
    return readStorage(*Element, Bytes.get() + Offset);
  }

  ExecutionStatus ExecutionCell::storeRuntime(const RuntimeValue &Value)
  {
    return Value.Type == Layout.Type ? storeElement(0, Value) : ExecutionStatus::TypeMismatch;
  }

  ExecutionStatus ExecutionCell::storeElement(std::size_t Offset, const RuntimeValue &Value)
  {
    if (Runtime)
    {
      return ExecutionStatus::RuntimeValue;
    }
    const TypeDesc *Element = elementLayout(Offset, Value.Type);
    if (!Element)
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (!Writable && isInitialized(Offset, *Element))
    {
      return ExecutionStatus::ReadOnly;
    }
    ExecutionStatus Status = validateArrayElement(*Element, Value);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    // Boundary conversion can exhaust the string budget. Publish the bytes only
    // after every owned string has been retained successfully.
    std::array<std::byte, sizeof(void *) + sizeof(std::size_t)> Inline{};
    RuntimeBytes Temporary{nullptr, RuntimeBytesDeleter{}};
    std::byte *Encoded = Inline.data();
    if (Element->Size > Inline.size())
    {
      Temporary = allocateRuntimeBytes(Element->Size, Element->Alignment);
      Encoded = Temporary.get();
    }
    Status = Owner.writeValueBytes(*Element, Encoded, Value);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    std::memcpy(Bytes.get() + Offset, Encoded, Element->Size);
    markInitialized(Offset, *Element);
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionCell::loadBits(std::uint64_t &Bits) const noexcept
  {
    return loadBits(Bits, Layout.Size);
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
    if (Width != Layout.Size)
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (Width > sizeof(Bits))
    {
      return ExecutionStatus::UnsupportedOperation;
    }
    Bits = RuntimeSlot{Bytes.get(), &Layout, true}.bits();
    return Layout.Kind == RuntimeKind::Boolean && Bits > 1 ? ExecutionStatus::TypeMismatch : ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionCell::storeBits(std::uint64_t Bits) noexcept
  {
    return storeBits(Bits, Layout.Size);
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
    if (Width != Layout.Size || (Layout.Kind == RuntimeKind::Boolean && Bits > 1))
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (Width > sizeof(Bits))
    {
      return ExecutionStatus::UnsupportedOperation;
    }
    RuntimeSlot{Bytes.get(), &Layout, true}.setBits(Bits);
    Initialized = true;
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
