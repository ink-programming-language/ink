#include "ink/execution/runtime_value.h"

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
    bool isIntegerType(ir::TypeKind Kind) noexcept
    {
      return Kind == ir::TypeKind::Bool || Kind == ir::TypeKind::Byte || Kind == ir::TypeKind::I32 || Kind == ir::TypeKind::PointerSize;
    }

    bool isValidIntegerValue(ir::TypeKind Kind, std::uint64_t Value) noexcept
    {
      switch (Kind)
      {
      case ir::TypeKind::Bool:
        return Value <= 1;
      case ir::TypeKind::Byte:
        return Value <= std::numeric_limits<std::uint8_t>::max();
      case ir::TypeKind::I32:
        return static_cast<std::uint64_t>(static_cast<std::int64_t>(static_cast<std::int32_t>(Value))) == Value;
      case ir::TypeKind::PointerSize:
        return Value <= std::numeric_limits<std::size_t>::max();
      case ir::TypeKind::Void:
      case ir::TypeKind::BytePointer:
      case ir::TypeKind::ConstBytePointer:
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
      case ir::TypeKind::BytePointer:
      case ir::TypeKind::ConstBytePointer:
        return Value.kind() == RuntimeValueKind::Pointer;
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

    class RuntimePointerValue final : public RuntimeValue
    {
    public:
      RuntimePointerValue(const ir::Type &ValueType, const void *Value) noexcept : ValueType(&ValueType), Value(Value)
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
        return Value;
      }

      void *mutablePointer() const noexcept override
      {
        return ValueType->kind() == ir::TypeKind::BytePointer ? const_cast<void *>(Value) : nullptr;
      }

    private:
      const ir::Type *ValueType;
      const void *Value;
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
    static_assert(std::is_trivially_destructible_v<RuntimePointerValue>);
    static_assert(std::is_trivially_destructible_v<RuntimeAggregateValue>);
  } // namespace

  class RuntimeValueArena::Impl
  {
  public:
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
  };

  std::optional<std::uint64_t> RuntimeValue::integer() const noexcept
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

  std::size_t RuntimeValue::fieldCount() const noexcept
  {
    return 0;
  }

  const RuntimeValue *RuntimeValue::field(std::size_t) const noexcept
  {
    return nullptr;
  }

  RuntimeValueArena::RuntimeValueArena() : Implementation(std::make_unique<Impl>())
  {
  }

  RuntimeValueArena::~RuntimeValueArena() = default;

  bool isValidRuntimeIntegerValue(const ir::Type &ValueType, std::uint64_t Value) noexcept
  {
    return isIntegerType(ValueType.kind()) && isValidIntegerValue(ValueType.kind(), Value);
  }

  RuntimeValueRef RuntimeValueArena::voidValue(const ir::Type &ValueType)
  {
    return ValueType.kind() == ir::TypeKind::Void ? Implementation->create<RuntimeVoidValue>(ValueType) : nullptr;
  }

  RuntimeValueRef RuntimeValueArena::integerValue(const ir::Type &ValueType, std::uint64_t Value)
  {
    return isValidRuntimeIntegerValue(ValueType, Value) ? Implementation->create<RuntimeIntegerValue>(ValueType, Value) : nullptr;
  }

  RuntimeValueRef RuntimeValueArena::pointerValue(const ir::Type &ValueType, const void *Value)
  {
    return ValueType.kind() == ir::TypeKind::ConstBytePointer ? Implementation->create<RuntimePointerValue>(ValueType, Value) : nullptr;
  }

  RuntimeValueRef RuntimeValueArena::mutablePointerValue(const ir::Type &ValueType, void *Value)
  {
    return ValueType.kind() == ir::TypeKind::BytePointer ? Implementation->create<RuntimePointerValue>(ValueType, Value) : nullptr;
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
    switch (Value.kind())
    {
    case RuntimeValueKind::Void:
      return voidValue(Value.type());
    case RuntimeValueKind::Integer:
    {
      const std::optional<std::uint64_t> Integer = Value.integer();
      return Integer.has_value() ? integerValue(Value.type(), *Integer) : nullptr;
    }
    case RuntimeValueKind::Pointer:
      return Value.type().kind() == ir::TypeKind::BytePointer ? mutablePointerValue(Value.type(), Value.mutablePointer()) : pointerValue(Value.type(), Value.pointer());
    case RuntimeValueKind::Aggregate:
      break;
    }
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
      case RuntimeValueKind::Pointer:
        Result = Source.type().kind() == ir::TypeKind::BytePointer ? mutablePointerValue(Source.type(), Source.mutablePointer()) : pointerValue(Source.type(), Source.pointer());
        break;
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
