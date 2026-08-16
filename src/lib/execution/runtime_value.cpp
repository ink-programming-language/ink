#include "ink/execution/runtime_value.h"

#include "runtime_memory.h"
#include "ink/ir/type_layout.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ink::execution
{
  namespace
  {
    template <typename ValueType>
    void reserveForAppend(std::vector<ValueType> &Values)
    {
      if (Values.size() < Values.capacity())
      {
        return;
      }
      const std::size_t NewCapacity = Values.capacity() == 0 ? 1 : Values.capacity() * 2;
      Values.reserve(NewCapacity);
    }

    bool isIntegerType(ir::TypeKind Kind) noexcept
    {
      return Kind == ir::TypeKind::Bool || Kind == ir::TypeKind::Byte || Kind == ir::TypeKind::I32 || Kind == ir::TypeKind::PointerSize;
    }

    bool isValidFloatingPointBits(ir::TypeKind Kind, std::uint64_t Bits) noexcept
    {
      switch (Kind)
      {
      case ir::TypeKind::F16:
        return Bits <= 0xFFFFULL;
      case ir::TypeKind::F32:
        return Bits <= 0xFFFFFFFFULL;
      case ir::TypeKind::F64:
        return true;
      case ir::TypeKind::Void:
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
      case ir::TypeKind::I32:
      case ir::TypeKind::PointerSize:
      case ir::TypeKind::BytePointer:
      case ir::TypeKind::ConstBytePointer:
      case ir::TypeKind::ByteSlice:
      case ir::TypeKind::ConstByteSlice:
      case ir::TypeKind::Struct:
      case ir::TypeKind::Count:
        return false;
      }
      return false;
    }

    bool isValidIntegerValue(ir::TypeKind Kind, std::uint64_t Value, const core::TargetContext &Target) noexcept
    {
      switch (Kind)
      {
      case ir::TypeKind::Bool:
        return Value <= 1;
      case ir::TypeKind::Byte:
        return Value <= std::numeric_limits<std::uint8_t>::max();
      case ir::TypeKind::I32:
        return Value <= 0x7FFFFFFFULL || Value >= 0xFFFFFFFF80000000ULL;
      case ir::TypeKind::PointerSize:
        return Value <= Target.maximumPointerSizeValue();
      case ir::TypeKind::Void:
      case ir::TypeKind::F16:
      case ir::TypeKind::F32:
      case ir::TypeKind::F64:
      case ir::TypeKind::BytePointer:
      case ir::TypeKind::ConstBytePointer:
      case ir::TypeKind::ByteSlice:
      case ir::TypeKind::ConstByteSlice:
      case ir::TypeKind::Struct:
      case ir::TypeKind::Count:
        return false;
      }
      return false;
    }

    bool hasCompatiblePayload(const RuntimeValue &Value) noexcept
    {
      switch (Value.type().kind())
      {
      case ir::TypeKind::Void:
        return Value.kind() == RuntimeValueKind::Void;
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
      case ir::TypeKind::I32:
      case ir::TypeKind::PointerSize:
        return Value.kind() == RuntimeValueKind::Integer && Value.integer().has_value();
      case ir::TypeKind::F16:
      case ir::TypeKind::F32:
      case ir::TypeKind::F64:
      {
        const std::optional<std::uint64_t> Bits = Value.floatingPointBits();
        return Value.kind() == RuntimeValueKind::FloatingPoint && Bits.has_value() && isValidFloatingPointBits(Value.type().kind(), *Bits);
      }
      case ir::TypeKind::BytePointer:
      case ir::TypeKind::ConstBytePointer:
        return Value.kind() == RuntimeValueKind::Pointer;
      case ir::TypeKind::ByteSlice:
      case ir::TypeKind::ConstByteSlice:
      {
        const std::optional<std::size_t> Length = Value.byteLength();
        return Value.kind() == RuntimeValueKind::ByteSlice && Length.has_value() && Value.memoryAlive() && (*Length == 0 || Value.pointer() != nullptr);
      }
      case ir::TypeKind::Struct:
        return Value.kind() == RuntimeValueKind::Aggregate;
      case ir::TypeKind::Count:
        return false;
      }
      return false;
    }

    class RuntimeVoidValue final : public RuntimeValue
    {
    public:
      explicit RuntimeVoidValue(const ir::Type &ValueType) noexcept : ValueType(&ValueType)
      {
      }
      ~RuntimeVoidValue() = default;

      RuntimeValueKind kind() const noexcept override
      {
        return RuntimeValueKind::Void;
      }

      const ir::Type &type() const noexcept override
      {
        return *ValueType;
      }

    private:
      const ir::Type *ValueType;
    };

    class RuntimeIntegerValue final : public RuntimeValue
    {
    public:
      RuntimeIntegerValue(const ir::Type &ValueType, std::uint64_t Value) noexcept : ValueType(&ValueType), Value(Value)
      {
      }
      ~RuntimeIntegerValue() = default;

      RuntimeValueKind kind() const noexcept override
      {
        return RuntimeValueKind::Integer;
      }

      const ir::Type &type() const noexcept override
      {
        return *ValueType;
      }

      std::optional<std::uint64_t> integer() const noexcept override
      {
        return Value;
      }

    private:
      const ir::Type *ValueType;
      std::uint64_t Value;
    };

    class RuntimeFloatingPointValue final : public RuntimeValue
    {
    public:
      RuntimeFloatingPointValue(const ir::Type &ValueType, std::uint64_t Bits) noexcept : ValueType(&ValueType), Bits(Bits)
      {
      }
      ~RuntimeFloatingPointValue() = default;

      RuntimeValueKind kind() const noexcept override
      {
        return RuntimeValueKind::FloatingPoint;
      }

      const ir::Type &type() const noexcept override
      {
        return *ValueType;
      }

      std::optional<std::uint64_t> floatingPointBits() const noexcept override
      {
        return Bits;
      }

    private:
      const ir::Type *ValueType;
      std::uint64_t Bits;
    };

    struct RuntimeMemoryBacking : std::enable_shared_from_this<RuntimeMemoryBacking>
    {
      RuntimeMemoryBacking(const void *Data, std::size_t Size, bool Writable) noexcept : Data(static_cast<const std::uint8_t *>(Data)), Size(Size), Writable(Writable)
      {
      }

      RuntimeMemoryBacking(std::size_t Size, std::size_t OwnerFrame) : OwnedData(std::make_unique<std::uint8_t[]>(Size)), Data(OwnedData.get()), Size(Size), OwnerFrame(OwnerFrame), Managed(true), Writable(true)
      {
      }

      std::unique_ptr<std::uint8_t[]> OwnedData;
      const std::uint8_t *Data;
      std::size_t Size;
      std::size_t OwnerFrame = 0;
      bool Managed = false;
      bool Writable;
      bool Alive = true;
    };

    class RuntimePointerValue final : public RuntimeValue
    {
    public:
      RuntimePointerValue(const ir::Type &ValueType, const void *RawAddress) noexcept : ValueType(&ValueType), RawAddress(RawAddress)
      {
      }

      RuntimePointerValue(const ir::Type &ValueType, RuntimeMemoryBacking *Backing, std::uint64_t ByteOffset) noexcept : ValueType(&ValueType), Backing(Backing), ByteOffset(ByteOffset)
      {
      }
      ~RuntimePointerValue() = default;

      RuntimeValueKind kind() const noexcept override
      {
        return RuntimeValueKind::Pointer;
      }

      const ir::Type &type() const noexcept override
      {
        return *ValueType;
      }

      const void *pointer() const noexcept override
      {
        return resolvedPointer();
      }

      void *mutablePointer() const noexcept override
      {
        if (ValueType->kind() != ir::TypeKind::BytePointer || (Backing != nullptr && !Backing->Writable))
        {
          return nullptr;
        }
        return const_cast<void *>(resolvedPointer());
      }

      bool memoryAlive() const noexcept override
      {
        return Backing == nullptr || Backing->Alive;
      }

      std::optional<std::size_t> byteLength() const noexcept override
      {
        return Backing == nullptr ? std::nullopt : std::optional<std::size_t>(Backing->Size);
      }

      RuntimeMemoryBacking *backing() const noexcept
      {
        return Backing;
      }

      std::uint64_t byteOffset() const noexcept
      {
        return ByteOffset;
      }

      const void *rawAddress() const noexcept
      {
        return Backing == nullptr ? RawAddress : Backing->Data;
      }

      bool isNull() const noexcept
      {
        return Backing == nullptr ? RawAddress == nullptr : Backing->Data == nullptr && ByteOffset == 0;
      }

    private:
      const void *resolvedPointer() const noexcept
      {
        if (Backing == nullptr)
        {
          return RawAddress;
        }
        if (!Backing->Alive || ByteOffset > Backing->Size)
        {
          return nullptr;
        }
        if (Backing->Data == nullptr || ByteOffset > std::numeric_limits<std::uintptr_t>::max())
        {
          return nullptr;
        }
        const std::uintptr_t BaseAddress = reinterpret_cast<std::uintptr_t>(Backing->Data);
        const std::uintptr_t HostOffset = static_cast<std::uintptr_t>(ByteOffset);
        return HostOffset > std::numeric_limits<std::uintptr_t>::max() - BaseAddress ? nullptr : reinterpret_cast<const void *>(BaseAddress + HostOffset);
      }

      const ir::Type *ValueType;
      const void *RawAddress = nullptr;
      RuntimeMemoryBacking *Backing = nullptr;
      std::uint64_t ByteOffset = 0;
    };

    class RuntimeByteSliceValue final : public RuntimeValue
    {
    public:
      RuntimeByteSliceValue(const ir::Type &ValueType, RuntimeMemoryBacking *Backing) noexcept : ValueType(&ValueType), Backing(Backing)
      {
      }
      ~RuntimeByteSliceValue() = default;

      RuntimeValueKind kind() const noexcept override
      {
        return RuntimeValueKind::ByteSlice;
      }

      const ir::Type &type() const noexcept override
      {
        return *ValueType;
      }

      const void *pointer() const noexcept override
      {
        return memoryAlive() ? Backing->Data : nullptr;
      }

      void *mutablePointer() const noexcept override
      {
        return ValueType->kind() == ir::TypeKind::ByteSlice && Backing->Writable && memoryAlive() ? const_cast<std::uint8_t *>(Backing->Data) : nullptr;
      }

      std::optional<std::size_t> byteLength() const noexcept override
      {
        return Backing->Size;
      }

      bool memoryAlive() const noexcept override
      {
        return Backing->Alive;
      }

      const std::uint8_t *data() const noexcept
      {
        return Backing->Data;
      }

      std::uint8_t *mutableData() const noexcept
      {
        return ValueType->kind() == ir::TypeKind::ByteSlice && Backing->Writable ? const_cast<std::uint8_t *>(Backing->Data) : nullptr;
      }

      std::size_t size() const noexcept
      {
        return Backing->Size;
      }

      RuntimeMemoryBacking *backing() const noexcept
      {
        return Backing;
      }

    private:
      const ir::Type *ValueType;
      RuntimeMemoryBacking *Backing;
    };

    class RuntimeAggregateValue final : public RuntimeValue
    {
    public:
      RuntimeAggregateValue(const ir::StructType &ValueType, RuntimeValueRef const *Fields, std::size_t FieldCount) noexcept : ValueType(&ValueType), Fields(Fields), FieldCount(FieldCount)
      {
      }
      ~RuntimeAggregateValue() = default;

      RuntimeValueKind kind() const noexcept override
      {
        return RuntimeValueKind::Aggregate;
      }

      const ir::Type &type() const noexcept override
      {
        return *ValueType;
      }

      std::size_t fieldCount() const noexcept override
      {
        return FieldCount;
      }

      const RuntimeValue *field(std::size_t FieldIndex) const noexcept override
      {
        return FieldIndex < FieldCount ? Fields[FieldIndex] : nullptr;
      }

    private:
      const ir::StructType *ValueType;
      RuntimeValueRef const *Fields;
      std::size_t FieldCount;
    };

    static_assert(std::is_trivially_destructible_v<RuntimeVoidValue>);
    static_assert(std::is_trivially_destructible_v<RuntimeIntegerValue>);
    static_assert(std::is_trivially_destructible_v<RuntimeFloatingPointValue>);
    static_assert(std::is_trivially_destructible_v<RuntimePointerValue>);
    static_assert(std::is_trivially_destructible_v<RuntimeByteSliceValue>);
    static_assert(std::is_trivially_destructible_v<RuntimeAggregateValue>);

  } // namespace

  class RuntimeValueArena::Impl
  {
  public:
    ~Impl()
    {
      for (RuntimeMemoryBacking *Backing : ManagedBackings)
      {
        Backing->Alive = false;
        Backing->OwnedData.reset();
      }
    }

    template <typename ValueType, typename... ArgumentTypes>
    RuntimeValueRef create(ArgumentTypes &&...Arguments)
    {
      static_assert(std::is_base_of_v<RuntimeValue, ValueType>);
      static_assert(std::is_trivially_destructible_v<ValueType>);
      static_assert(std::is_nothrow_constructible_v<ValueType, ArgumentTypes...>);
      void *Storage = allocate(sizeof(ValueType), alignof(ValueType));
      return new (Storage) ValueType(std::forward<ArgumentTypes>(Arguments)...);
    }

    RuntimeValueRef *copyFields(const std::vector<RuntimeValueRef> &Fields)
    {
      if (Fields.empty())
      {
        return nullptr;
      }
      auto *Result = static_cast<RuntimeValueRef *>(allocate(sizeof(RuntimeValueRef) * Fields.size(), alignof(RuntimeValueRef)));
      for (std::size_t FieldIndex = 0; FieldIndex < Fields.size(); ++FieldIndex)
      {
        new (Result + FieldIndex) RuntimeValueRef(Fields[FieldIndex]);
      }
      return Result;
    }

    bool owns(const void *Value) const noexcept
    {
      const std::uintptr_t Address = reinterpret_cast<std::uintptr_t>(Value);
      for (const Block &BlockValue : Blocks)
      {
        const std::uintptr_t Begin = reinterpret_cast<std::uintptr_t>(BlockValue.Data.get());
        if (Address >= Begin && Address < Begin + BlockValue.Used)
        {
          return true;
        }
      }
      return false;
    }

    RuntimeMemoryBacking *createBorrowedBacking(const void *Data, std::size_t Size, bool Writable)
    {
      reserveForAppend(MemoryBackings);
      auto Backing = std::make_shared<RuntimeMemoryBacking>(Data, Size, Writable);
      RuntimeMemoryBacking *Result = Backing.get();
      MemoryBackings.push_back(std::move(Backing));
      RetainedBackings.insert(Result);
      return Result;
    }

    void retainBacking(RuntimeMemoryBacking *Backing)
    {
      if (RetainedBackings.find(Backing) != RetainedBackings.end())
      {
        return;
      }
      std::shared_ptr<RuntimeMemoryBacking> RetainedBacking = Backing->shared_from_this();
      reserveForAppend(MemoryBackings);
      RetainedBackings.insert(Backing);
      MemoryBackings.push_back(std::move(RetainedBacking));
    }

    RuntimeValueRef allocateManagedByteSlice(const ir::Type &ValueType, std::uint64_t Size, std::size_t OwnerFrame, RuntimeMemoryStatus &Status)
    {
      if (Size > MaximumRuntimeByteAllocationSize || Size > std::numeric_limits<std::size_t>::max())
      {
        Status = RuntimeMemoryStatus::AllocationSizeLimitExceeded;
        return nullptr;
      }
      if (ManagedAllocationCount >= MaximumRuntimeByteAllocationCount)
      {
        Status = RuntimeMemoryStatus::AllocationCountLimitExceeded;
        return nullptr;
      }
      if (static_cast<std::uint64_t>(ByteStorageSize) > static_cast<std::uint64_t>(MaximumRuntimeByteStorage) - Size)
      {
        Status = RuntimeMemoryStatus::StorageLimitExceeded;
        return nullptr;
      }

      const std::size_t HostSize = static_cast<std::size_t>(Size);
      std::vector<RuntimeMemoryBacking *> &FrameAllocationList = FrameAllocations[OwnerFrame];
      reserveForAppend(FrameAllocationList);
      reserveForAppend(MemoryBackings);
      reserveForAppend(ManagedBackings);
      auto Backing = std::make_shared<RuntimeMemoryBacking>(HostSize, OwnerFrame);
      RuntimeMemoryBacking *BackingValue = Backing.get();
      RuntimeValueRef Result = create<RuntimeByteSliceValue>(ValueType, BackingValue);
      MemoryBackings.push_back(std::move(Backing));
      RetainedBackings.insert(BackingValue);
      ManagedBackings.push_back(BackingValue);
      FrameAllocationList.push_back(BackingValue);
      ++ManagedAllocationCount;
      ByteStorageSize += HostSize;
      Status = RuntimeMemoryStatus::Ok;
      return Result;
    }

    void endFrameLifetimes(std::size_t OwnerFrame) noexcept
    {
      const auto Found = FrameAllocations.find(OwnerFrame);
      if (Found == FrameAllocations.end())
      {
        return;
      }
      for (RuntimeMemoryBacking *Backing : Found->second)
      {
        Backing->Alive = false;
        Backing->OwnedData.reset();
      }
      FrameAllocations.erase(Found);
    }

    bool ownsManagedBacking(const RuntimeMemoryBacking *Backing) const noexcept
    {
      return std::find(ManagedBackings.begin(), ManagedBackings.end(), Backing) != ManagedBackings.end();
    }

  private:
    struct Block
    {
      explicit Block(std::size_t Capacity) : Data(std::make_unique<std::byte[]>(Capacity)), Capacity(Capacity)
      {
      }

      std::unique_ptr<std::byte[]> Data;
      std::size_t Capacity;
      std::size_t Used = 0;
    };

    void *allocate(std::size_t Size, std::size_t Alignment)
    {
      if (!Blocks.empty())
      {
        Block &Current = Blocks.back();
        void *Address = Current.Data.get() + Current.Used;
        std::size_t Space = Current.Capacity - Current.Used;
        if (std::align(Alignment, Size, Address, Space) != nullptr)
        {
          Current.Used = static_cast<std::size_t>(static_cast<std::byte *>(Address) - Current.Data.get()) + Size;
          return Address;
        }
      }

      constexpr std::size_t InitialBlockSize = 256;
      if (Size > std::numeric_limits<std::size_t>::max() - (Alignment - 1))
      {
        throw std::bad_alloc();
      }
      const std::size_t RequiredCapacity = Size + Alignment - 1;
      std::size_t Capacity = InitialBlockSize;
      if (!Blocks.empty() && Blocks.back().Capacity <= std::numeric_limits<std::size_t>::max() / 2)
      {
        Capacity = Blocks.back().Capacity * 2;
      }
      Capacity = std::max(Capacity, RequiredCapacity);
      Blocks.emplace_back(Capacity);
      Block &Current = Blocks.back();
      void *Address = Current.Data.get();
      std::size_t Space = Current.Capacity;
      if (std::align(Alignment, Size, Address, Space) == nullptr)
      {
        throw std::bad_alloc();
      }
      Current.Used = static_cast<std::size_t>(static_cast<std::byte *>(Address) - Current.Data.get()) + Size;
      return Address;
    }

    std::vector<Block> Blocks;
    std::vector<std::shared_ptr<RuntimeMemoryBacking>> MemoryBackings;
    std::unordered_set<RuntimeMemoryBacking *> RetainedBackings;
    std::vector<RuntimeMemoryBacking *> ManagedBackings;
    std::unordered_map<std::size_t, std::vector<RuntimeMemoryBacking *>> FrameAllocations;
    std::size_t ManagedAllocationCount = 0;
    std::size_t ByteStorageSize = 0;
  };

  std::optional<std::uint64_t> RuntimeValue::integer() const noexcept
  {
    return std::nullopt;
  }

  std::optional<std::uint64_t> RuntimeValue::floatingPointBits() const noexcept
  {
    return std::nullopt;
  }

  const void *RuntimeValue::pointer() const noexcept
  {
    return nullptr;
  }

  void *RuntimeValue::mutablePointer() const noexcept
  {
    return nullptr;
  }

  std::optional<std::size_t> RuntimeValue::byteLength() const noexcept
  {
    return std::nullopt;
  }

  bool RuntimeValue::memoryAlive() const noexcept
  {
    return true;
  }

  std::size_t RuntimeValue::fieldCount() const noexcept
  {
    return 0;
  }

  const RuntimeValue *RuntimeValue::field(std::size_t) const noexcept
  {
    return nullptr;
  }

  RuntimeValueArena::RuntimeValueArena() : RuntimeValueArena(core::TargetContext::native())
  {
  }

  RuntimeValueArena::RuntimeValueArena(core::TargetContext Target) : Target(Target), Implementation(std::make_unique<Impl>())
  {
  }

  RuntimeValueArena::~RuntimeValueArena() = default;

  bool isValidRuntimeIntegerValue(const ir::Type &ValueType, std::uint64_t Value, const core::TargetContext &Target) noexcept
  {
    return isIntegerType(ValueType.kind()) && isValidIntegerValue(ValueType.kind(), Value, Target);
  }

  bool isValidRuntimeFloatingPointValue(const ir::Type &ValueType, std::uint64_t Bits) noexcept
  {
    return ir::isFloatingPointType(ValueType.kind()) && isValidFloatingPointBits(ValueType.kind(), Bits);
  }

  std::optional<bool> runtimePointersEqual(const RuntimeValue &Left, const RuntimeValue &Right) noexcept
  {
    const auto *LeftPointer = dynamic_cast<const RuntimePointerValue *>(&Left);
    const auto *RightPointer = dynamic_cast<const RuntimePointerValue *>(&Right);
    if (LeftPointer == nullptr || RightPointer == nullptr || Left.kind() != RuntimeValueKind::Pointer || Right.kind() != RuntimeValueKind::Pointer)
    {
      return std::nullopt;
    }
    if (LeftPointer->isNull() || RightPointer->isNull())
    {
      return LeftPointer->isNull() && RightPointer->isNull();
    }
    if (LeftPointer->backing() != nullptr || RightPointer->backing() != nullptr)
    {
      return LeftPointer->backing() == RightPointer->backing() && LeftPointer->byteOffset() == RightPointer->byteOffset();
    }
    return LeftPointer->rawAddress() == RightPointer->rawAddress();
  }

  std::optional<std::uint64_t> runtimePointerByteOffset(const RuntimeValue &Value) noexcept
  {
    const auto *Pointer = dynamic_cast<const RuntimePointerValue *>(&Value);
    return Pointer == nullptr || Pointer->backing() == nullptr ? std::nullopt : std::optional<std::uint64_t>(Pointer->byteOffset());
  }

  RuntimeValueRef RuntimeValueArena::voidValue(const ir::Type &ValueType)
  {
    return ValueType.kind() == ir::TypeKind::Void ? Implementation->create<RuntimeVoidValue>(ValueType) : nullptr;
  }

  RuntimeValueRef RuntimeValueArena::integerValue(const ir::Type &ValueType, std::uint64_t Value)
  {
    return isValidRuntimeIntegerValue(ValueType, Value, Target) ? Implementation->create<RuntimeIntegerValue>(ValueType, Value) : nullptr;
  }

  RuntimeValueRef RuntimeValueArena::floatingPointValue(const ir::Type &ValueType, std::uint64_t Bits)
  {
    return isValidRuntimeFloatingPointValue(ValueType, Bits) ? Implementation->create<RuntimeFloatingPointValue>(ValueType, Bits) : nullptr;
  }

  RuntimeValueRef RuntimeValueArena::pointerValue(const ir::Type &ValueType, const void *Value)
  {
    return ValueType.kind() == ir::TypeKind::ConstBytePointer && (Value == nullptr || Target.isNativeAbiCompatible()) ? Implementation->create<RuntimePointerValue>(ValueType, Value) : nullptr;
  }

  RuntimeValueRef RuntimeValueArena::mutablePointerValue(const ir::Type &ValueType, void *Value)
  {
    return ValueType.kind() == ir::TypeKind::BytePointer && (Value == nullptr || Target.isNativeAbiCompatible()) ? Implementation->create<RuntimePointerValue>(ValueType, Value) : nullptr;
  }

  RuntimeValueRef RuntimeValueArena::borrowedPointerValue(const ir::Type &ValueType, const void *Data, std::size_t Size)
  {
    if (ValueType.kind() != ir::TypeKind::ConstBytePointer || (Data == nullptr && Size != 0) || Size > Target.maximumPointerSizeValue())
    {
      return nullptr;
    }
    RuntimeMemoryBacking *Backing = Implementation->createBorrowedBacking(Data, Size, false);
    return Implementation->create<RuntimePointerValue>(ValueType, Backing, 0);
  }

  RuntimeValueRef RuntimeValueArena::byteSliceValue(const ir::Type &ValueType, const void *Data, std::size_t Size)
  {
    if (ValueType.kind() != ir::TypeKind::ConstByteSlice || (Data == nullptr && Size != 0) || Size > Target.maximumPointerSizeValue())
    {
      return nullptr;
    }
    RuntimeMemoryBacking *Backing = Implementation->createBorrowedBacking(Data, Size, false);
    return Implementation->create<RuntimeByteSliceValue>(ValueType, Backing);
  }

  RuntimeValueRef RuntimeValueArena::mutableByteSliceValue(const ir::Type &ValueType, void *Data, std::size_t Size)
  {
    if (ValueType.kind() != ir::TypeKind::ByteSlice || (Data == nullptr && Size != 0) || Size > Target.maximumPointerSizeValue())
    {
      return nullptr;
    }
    RuntimeMemoryBacking *Backing = Implementation->createBorrowedBacking(Data, Size, true);
    return Implementation->create<RuntimeByteSliceValue>(ValueType, Backing);
  }

  RuntimeValueRef RuntimeValueArena::pointerFromByteSlice(const ir::Type &ValueType, const RuntimeValue &Slice)
  {
    if (Slice.kind() != RuntimeValueKind::ByteSlice)
    {
      return nullptr;
    }
    const bool IsMutableSource = Slice.type().kind() == ir::TypeKind::ByteSlice;
    const bool IsConstSource = Slice.type().kind() == ir::TypeKind::ConstByteSlice;
    if ((!IsMutableSource && !IsConstSource) || (ValueType.kind() == ir::TypeKind::BytePointer && !IsMutableSource) || (ValueType.kind() != ir::TypeKind::BytePointer && ValueType.kind() != ir::TypeKind::ConstBytePointer))
    {
      return nullptr;
    }
    if (!Slice.memoryAlive())
    {
      return nullptr;
    }
    const std::optional<std::size_t> Length = Slice.byteLength();
    const void *Data = ValueType.kind() == ir::TypeKind::BytePointer ? Slice.mutablePointer() : Slice.pointer();
    if (!Length.has_value() || *Length > Target.maximumPointerSizeValue() || (Data == nullptr && *Length != 0))
    {
      return nullptr;
    }
    const auto *InternalSlice = dynamic_cast<const RuntimeByteSliceValue *>(&Slice);
    RuntimeMemoryBacking *Backing = InternalSlice == nullptr ? Implementation->createBorrowedBacking(Data, *Length, IsMutableSource) : InternalSlice->backing();
    if (InternalSlice != nullptr)
    {
      Implementation->retainBacking(Backing);
    }
    return Implementation->create<RuntimePointerValue>(ValueType, Backing, 0);
  }

  RuntimeValueRef RuntimeValueArena::allocateByteSlice(const ir::Type &ValueType, std::uint64_t Size, std::size_t OwnerFrame, RuntimeMemoryStatus &Status)
  {
    if (ValueType.kind() != ir::TypeKind::ByteSlice || Size > Target.maximumPointerSizeValue())
    {
      Status = RuntimeMemoryStatus::InvalidValue;
      return nullptr;
    }
    return Implementation->allocateManagedByteSlice(ValueType, Size, OwnerFrame, Status);
  }

  RuntimeValueRef RuntimeValueArena::getElementPointer(const ir::Type &ResultType, const RuntimeValue &Pointer, const ir::Type &ElementType, std::uint64_t Index, RuntimeMemoryStatus &Status)
  {
    return getElementPointer(ResultType, Pointer, ElementType, Index, std::vector<std::uint32_t>{}, Status);
  }

  RuntimeValueRef RuntimeValueArena::getElementPointer(const ir::Type &ResultType, const RuntimeValue &Pointer, const ir::Type &ElementType, std::uint64_t Index, const std::vector<std::uint32_t> &FieldIndices, RuntimeMemoryStatus &Status)
  {
    const auto *PointerValue = dynamic_cast<const RuntimePointerValue *>(&Pointer);
    const bool ValidResultType = ResultType.kind() == ir::TypeKind::BytePointer || ResultType.kind() == ir::TypeKind::ConstBytePointer;
    if (PointerValue == nullptr || Pointer.kind() != RuntimeValueKind::Pointer || !ValidResultType || &ResultType != &Pointer.type())
    {
      Status = RuntimeMemoryStatus::InvalidValue;
      return nullptr;
    }

    std::uint64_t ElementByteOffset = 0;
    Status = detail::computeElementByteOffset(ElementType, Index, FieldIndices, Target, ElementByteOffset);
    if (Status != RuntimeMemoryStatus::Ok)
    {
      return nullptr;
    }
    if (PointerValue->backing() != nullptr)
    {
      if (PointerValue->backing()->Size > Target.maximumPointerSizeValue())
      {
        Status = RuntimeMemoryStatus::InvalidValue;
        return nullptr;
      }
      if (PointerValue->byteOffset() > Target.maximumPointerSizeValue() || ElementByteOffset > Target.maximumPointerSizeValue() - PointerValue->byteOffset())
      {
        Status = RuntimeMemoryStatus::AddressOverflow;
        return nullptr;
      }
      Implementation->retainBacking(PointerValue->backing());
      RuntimeValueRef Result = Implementation->create<RuntimePointerValue>(ResultType, PointerValue->backing(), PointerValue->byteOffset() + ElementByteOffset);
      Status = RuntimeMemoryStatus::Ok;
      return Result;
    }

    const std::uintptr_t BaseAddress = reinterpret_cast<std::uintptr_t>(PointerValue->rawAddress());
    if ((!Target.isNativeAbiCompatible() && (BaseAddress != 0 || ElementByteOffset != 0)) || BaseAddress > Target.maximumPointerSizeValue() || ElementByteOffset > Target.maximumPointerSizeValue() - BaseAddress)
    {
      Status = BaseAddress <= Target.maximumPointerSizeValue() && ElementByteOffset <= Target.maximumPointerSizeValue() - BaseAddress ? RuntimeMemoryStatus::InvalidValue : RuntimeMemoryStatus::AddressOverflow;
      return nullptr;
    }
    RuntimeValueRef Result = Implementation->create<RuntimePointerValue>(ResultType, reinterpret_cast<const void *>(BaseAddress + static_cast<std::uintptr_t>(ElementByteOffset)));
    Status = RuntimeMemoryStatus::Ok;
    return Result;
  }

  RuntimeMemoryStatus RuntimeValueArena::loadValue(const RuntimeValue &Pointer, const ir::Type &ValueType, RuntimeValueRef &Value)
  {
    const auto *PointerValue = dynamic_cast<const RuntimePointerValue *>(&Pointer);
    const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(ValueType, Target);
    if (PointerValue == nullptr || Pointer.kind() != RuntimeValueKind::Pointer || (Pointer.type().kind() != ir::TypeKind::BytePointer && Pointer.type().kind() != ir::TypeKind::ConstBytePointer) || !ir::isMemoryValueType(ValueType) || !Layout.has_value() || Layout->StrideSize == 0)
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    if (PointerValue->backing() == nullptr)
    {
      return RuntimeMemoryStatus::UntrackedPointer;
    }
    if (!PointerValue->memoryAlive())
    {
      return RuntimeMemoryStatus::LifetimeEnded;
    }
    if (PointerValue->backing()->Size > Target.maximumPointerSizeValue() || PointerValue->byteOffset() > Target.maximumPointerSizeValue())
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    const std::uint64_t ByteLength = PointerValue->backing()->Size;
    const std::uint64_t AccessSize = Layout->StrideSize;
    if (PointerValue->byteOffset() > ByteLength || AccessSize > ByteLength - PointerValue->byteOffset())
    {
      return RuntimeMemoryStatus::OutOfBounds;
    }
    const auto *Data = static_cast<const std::uint8_t *>(PointerValue->pointer());
    return Data == nullptr ? RuntimeMemoryStatus::InvalidValue : detail::loadRuntimeMemoryValue(*this, ValueType, Data, Target, Value);
  }

  RuntimeMemoryStatus RuntimeValueArena::storeValue(const RuntimeValue &Pointer, const RuntimeValue &Value)
  {
    const auto *PointerValue = dynamic_cast<const RuntimePointerValue *>(&Pointer);
    const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(Value.type(), Target);
    if (PointerValue == nullptr || Pointer.kind() != RuntimeValueKind::Pointer || Pointer.type().kind() != ir::TypeKind::BytePointer || !ir::isMemoryValueType(Value.type()) || !Layout.has_value() || Layout->StrideSize == 0)
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    if (PointerValue->backing() == nullptr)
    {
      return RuntimeMemoryStatus::UntrackedPointer;
    }
    if (!PointerValue->memoryAlive())
    {
      return RuntimeMemoryStatus::LifetimeEnded;
    }
    if (PointerValue->backing()->Size > Target.maximumPointerSizeValue() || PointerValue->byteOffset() > Target.maximumPointerSizeValue())
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    if (!PointerValue->backing()->Writable)
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    const std::uint64_t ByteLength = PointerValue->backing()->Size;
    const std::uint64_t AccessSize = Layout->StrideSize;
    if (PointerValue->byteOffset() > ByteLength || AccessSize > ByteLength - PointerValue->byteOffset())
    {
      return RuntimeMemoryStatus::OutOfBounds;
    }
    auto *Data = static_cast<std::uint8_t *>(PointerValue->mutablePointer());
    return Data == nullptr ? RuntimeMemoryStatus::InvalidValue : detail::storeRuntimeMemoryValue(Value, Data, Target);
  }

  RuntimeMemoryStatus RuntimeValueArena::endByteSliceLifetime(const RuntimeValue &Slice, std::size_t OwnerFrame) noexcept
  {
    const auto *SliceValue = dynamic_cast<const RuntimeByteSliceValue *>(&Slice);
    if (SliceValue == nullptr || Slice.type().kind() != ir::TypeKind::ByteSlice)
    {
      return RuntimeMemoryStatus::InvalidValue;
    }
    RuntimeMemoryBacking *Backing = SliceValue->backing();
    if (!Implementation->owns(&Slice) || Backing == nullptr || !Implementation->ownsManagedBacking(Backing) || Backing->OwnerFrame != OwnerFrame)
    {
      return RuntimeMemoryStatus::NotOwned;
    }
    if (!Backing->Alive)
    {
      return RuntimeMemoryStatus::LifetimeEnded;
    }
    Backing->Alive = false;
    return RuntimeMemoryStatus::Ok;
  }

  void RuntimeValueArena::endFrameLifetimes(std::size_t OwnerFrame) noexcept
  {
    Implementation->endFrameLifetimes(OwnerFrame);
  }

  RuntimeValueRef RuntimeValueArena::aggregateValue(const ir::StructType &ValueType, std::vector<RuntimeValueRef> Fields)
  {
    if (Fields.size() != ValueType.fieldTypes().size())
    {
      return nullptr;
    }
    for (std::size_t FieldIndex = 0; FieldIndex < Fields.size(); ++FieldIndex)
    {
      if (!owns(Fields[FieldIndex]) || &Fields[FieldIndex]->type() != ValueType.fieldTypes()[FieldIndex] || !hasCompatiblePayload(*Fields[FieldIndex]))
      {
        return nullptr;
      }
    }
    RuntimeValueRef *StoredFields = Implementation->copyFields(Fields);
    return Implementation->create<RuntimeAggregateValue>(ValueType, StoredFields, Fields.size());
  }

  RuntimeValueRef RuntimeValueArena::clone(const RuntimeValue &Value)
  {
    std::unordered_map<const RuntimeValue *, RuntimeValueRef> ClonedValues;
    std::unordered_set<const RuntimeValue *> ActiveValues;
    const auto CloneValue = [this, &ClonedValues, &ActiveValues](const auto &Self, const RuntimeValue &Source) -> RuntimeValueRef
    {
      const auto Existing = ClonedValues.find(&Source);
      if (Existing != ClonedValues.end())
      {
        return Existing->second;
      }
      RuntimeValueRef Result = nullptr;
      switch (Source.kind())
      {
      case RuntimeValueKind::Void:
        Result = voidValue(Source.type());
        break;
      case RuntimeValueKind::Integer:
      {
        const std::optional<std::uint64_t> Integer = Source.integer();
        Result = Integer.has_value() ? integerValue(Source.type(), *Integer) : nullptr;
        break;
      }
      case RuntimeValueKind::FloatingPoint:
      {
        const std::optional<std::uint64_t> Bits = Source.floatingPointBits();
        Result = Bits.has_value() ? floatingPointValue(Source.type(), *Bits) : nullptr;
        break;
      }
      case RuntimeValueKind::Pointer:
      {
        const auto *Pointer = dynamic_cast<const RuntimePointerValue *>(&Source);
        const bool IsMutablePointer = Source.type().kind() == ir::TypeKind::BytePointer;
        const bool IsConstPointer = Source.type().kind() == ir::TypeKind::ConstBytePointer;
        if (!IsMutablePointer && !IsConstPointer)
        {
          break;
        }
        if (Pointer == nullptr)
        {
          Result = IsMutablePointer ? mutablePointerValue(Source.type(), Source.mutablePointer()) : pointerValue(Source.type(), Source.pointer());
          break;
        }
        RuntimeMemoryBacking *Backing = Pointer->backing();
        if (Backing == nullptr)
        {
          Result = IsMutablePointer ? mutablePointerValue(Source.type(), const_cast<void *>(Pointer->rawAddress())) : pointerValue(Source.type(), Pointer->rawAddress());
          break;
        }
        if (!Backing->Alive || Backing->Size > Target.maximumPointerSizeValue() || Pointer->byteOffset() > Target.maximumPointerSizeValue() || (Backing->Data == nullptr && Backing->Size != 0) || (IsMutablePointer && !Backing->Writable))
        {
          break;
        }
        Implementation->retainBacking(Backing);
        Result = Implementation->create<RuntimePointerValue>(Source.type(), Backing, Pointer->byteOffset());
        break;
      }
      case RuntimeValueKind::ByteSlice:
      {
        const bool IsMutableSlice = Source.type().kind() == ir::TypeKind::ByteSlice;
        const bool IsConstSlice = Source.type().kind() == ir::TypeKind::ConstByteSlice;
        const auto *Slice = dynamic_cast<const RuntimeByteSliceValue *>(&Source);
        if (!IsMutableSlice && !IsConstSlice)
        {
          break;
        }
        if (Slice == nullptr)
        {
          const std::optional<std::size_t> Length = Source.byteLength();
          if (!Length.has_value() || !Source.memoryAlive())
          {
            break;
          }
          Result = IsMutableSlice ? mutableByteSliceValue(Source.type(), Source.mutablePointer(), *Length) : byteSliceValue(Source.type(), Source.pointer(), *Length);
          break;
        }
        RuntimeMemoryBacking *Backing = Slice->backing();
        if (!Backing->Alive || Backing->Size > Target.maximumPointerSizeValue() || (Backing->Data == nullptr && Backing->Size != 0) || (IsMutableSlice && !Backing->Writable))
        {
          break;
        }
        Implementation->retainBacking(Backing);
        Result = Implementation->create<RuntimeByteSliceValue>(Source.type(), Backing);
        break;
      }
      case RuntimeValueKind::Aggregate:
      {
        const auto *Struct = dynamic_cast<const ir::StructType *>(&Source.type());
        if (Struct == nullptr || Source.fieldCount() != Struct->fieldTypes().size() || !ActiveValues.insert(&Source).second)
        {
          return nullptr;
        }
        std::vector<RuntimeValueRef> Fields;
        Fields.reserve(Struct->fieldTypes().size());
        for (std::size_t FieldIndex = 0; FieldIndex < Struct->fieldTypes().size(); ++FieldIndex)
        {
          const RuntimeValue *Field = Source.field(FieldIndex);
          if (Field == nullptr || &Field->type() != Struct->fieldTypes()[FieldIndex])
          {
            ActiveValues.erase(&Source);
            return nullptr;
          }
          RuntimeValueRef ClonedField = Self(Self, *Field);
          if (ClonedField == nullptr)
          {
            ActiveValues.erase(&Source);
            return nullptr;
          }
          Fields.push_back(ClonedField);
        }
        ActiveValues.erase(&Source);
        Result = aggregateValue(*Struct, std::move(Fields));
        break;
      }
      }
      if (Result != nullptr)
      {
        ClonedValues.emplace(&Source, Result);
      }
      return Result;
    };
    return CloneValue(CloneValue, Value);
  }

  bool RuntimeValueArena::owns(RuntimeValueRef Value) const noexcept
  {
    return Value != nullptr && Implementation->owns(Value);
  }
} // namespace ink::execution
