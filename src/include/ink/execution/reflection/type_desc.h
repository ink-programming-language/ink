#ifndef INK_EXECUTION_REFLECTION_TYPE_DESC_H
#define INK_EXECUTION_REFLECTION_TYPE_DESC_H

#include "ink/core/visibility.h"
#include "ink/execution/support/runtime_kind.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ink::execution
{
  using RuntimeTypeId = std::uint32_t;
  using FunctionId = std::uint32_t;
  using SignatureId = RuntimeTypeId;
  inline constexpr RuntimeTypeId InvalidRuntimeType = std::numeric_limits<RuntimeTypeId>::max();
  inline constexpr FunctionId InvalidFunction = std::numeric_limits<FunctionId>::max();

  struct RuntimeTypeDomain final
  {
  };

  struct StorageLayout
  {
      std::size_t Size = 0;
      std::size_t Alignment = 1;
  };

  struct TypeDesc;

  // Only metadata uses polymorphism. Instances and VM slots remain plain bytes.
  class TypeDetails
  {
    public:
      virtual ~TypeDetails() = default;
      virtual RuntimeKind kind() const noexcept = 0;
      virtual std::shared_ptr<TypeDetails> clone() const = 0;
  };

  template <typename Derived, RuntimeKind Kind>
  class TypeDetailsBase : public TypeDetails
  {
    public:
      RuntimeKind kind() const noexcept final
      {
        return Kind;
      }

      std::shared_ptr<TypeDetails> clone() const final
      {
        return std::make_shared<Derived>(static_cast<const Derived &>(*this));
      }
  };

  struct IntegerDesc final : TypeDetailsBase<IntegerDesc, RuntimeKind::Integer>
  {
      std::uint32_t BitWidth = 0;
      bool Signed = false;
  };

  struct FloatDesc final : TypeDetailsBase<FloatDesc, RuntimeKind::Float>
  {
      std::uint32_t BitWidth = 0;
  };

  struct PointerDesc final : TypeDetailsBase<PointerDesc, RuntimeKind::Pointer>
  {
      RuntimeTypeId Pointee = InvalidRuntimeType;
      bool Writable = false;
  };

  struct FunctionDesc final : TypeDetailsBase<FunctionDesc, RuntimeKind::Function>
  {
      RuntimeTypeId ReturnType = InvalidRuntimeType;
      std::vector<RuntimeTypeId> Parameters;
  };

  struct ArrayDesc final : TypeDetailsBase<ArrayDesc, RuntimeKind::Array>
  {
      RuntimeTypeId ElementType = InvalidRuntimeType;
      std::uint64_t ElementCount = 0;
      std::shared_ptr<const TypeDesc> ElementLayout;
  };

  struct FieldDesc
  {
      std::string Name;
      RuntimeTypeId Type = InvalidRuntimeType;
      std::size_t Offset = 0;
      core::VisibilityKind Visibility = core::VisibilityKind::Public;
      FunctionId Initializer = InvalidFunction;
      std::shared_ptr<const TypeDesc> Layout;
  };

  struct MethodDesc
  {
      std::string Name;
      RuntimeTypeId Signature = InvalidRuntimeType;
      FunctionId Function = InvalidFunction;
      core::VisibilityKind Visibility = core::VisibilityKind::Public;
      // Instance methods currently take a writable pointer as their first argument.
      bool WritableReceiver = true;
  };

  struct ClassDesc final : TypeDetailsBase<ClassDesc, RuntimeKind::Class>
  {
      std::string NominalIdentity;
      std::vector<FieldDesc> Fields;
      std::vector<MethodDesc> Methods;

      const FieldDesc *findField(std::string_view Name) const noexcept;
      const MethodDesc *findMethod(std::string_view Name, RuntimeTypeId Signature = InvalidRuntimeType) const noexcept;
  };

  // Cheap owned reference to immutable type-specific metadata. Editing a copied
  // description detaches it, so linking cannot mutate another image's types.
  struct TypeDesc : StorageLayout
  {
      std::shared_ptr<const RuntimeTypeDomain> Domain;
      RuntimeTypeId Type = InvalidRuntimeType;
      RuntimeKind Kind = RuntimeKind::Invalid;
      bool Native = false;
      std::string Name;

      void setKind(RuntimeKind Value);
      void setDetails(std::shared_ptr<TypeDetails> Value);
      bool validDetails() const noexcept;
      std::uint32_t bitWidth() const noexcept;
      void setBitWidth(std::uint32_t Value);
      bool isSigned() const noexcept;

      const IntegerDesc &integerDesc() const noexcept
      {
        static const IntegerDesc Empty;
        return Kind == RuntimeKind::Integer && Details ? static_cast<const IntegerDesc &>(*Details) : Empty;
      }

      IntegerDesc &editInteger()
      {
        assert(Kind == RuntimeKind::Integer && Details);
        if (Details.use_count() != 1)
        {
          Details = Details->clone();
        }
        return static_cast<IntegerDesc &>(*Details);
      }

      const FloatDesc &floatDesc() const noexcept
      {
        static const FloatDesc Empty;
        return Kind == RuntimeKind::Float && Details ? static_cast<const FloatDesc &>(*Details) : Empty;
      }

      FloatDesc &editFloat()
      {
        assert(Kind == RuntimeKind::Float && Details);
        if (Details.use_count() != 1)
        {
          Details = Details->clone();
        }
        return static_cast<FloatDesc &>(*Details);
      }

      const PointerDesc &pointerDesc() const noexcept
      {
        static const PointerDesc Empty;
        return Kind == RuntimeKind::Pointer && Details ? static_cast<const PointerDesc &>(*Details) : Empty;
      }

      PointerDesc &editPointer()
      {
        assert(Kind == RuntimeKind::Pointer && Details);
        if (Details.use_count() != 1)
        {
          Details = Details->clone();
        }
        return static_cast<PointerDesc &>(*Details);
      }

      const FunctionDesc &functionDesc() const noexcept
      {
        static const FunctionDesc Empty;
        return Kind == RuntimeKind::Function && Details ? static_cast<const FunctionDesc &>(*Details) : Empty;
      }

      FunctionDesc &editFunction()
      {
        assert(Kind == RuntimeKind::Function && Details);
        if (Details.use_count() != 1)
        {
          Details = Details->clone();
        }
        return static_cast<FunctionDesc &>(*Details);
      }

      const ArrayDesc &arrayDesc() const noexcept
      {
        static const ArrayDesc Empty;
        return Kind == RuntimeKind::Array && Details ? static_cast<const ArrayDesc &>(*Details) : Empty;
      }

      ArrayDesc &editArray()
      {
        assert(Kind == RuntimeKind::Array && Details);
        if (Details.use_count() != 1)
        {
          Details = Details->clone();
        }
        return static_cast<ArrayDesc &>(*Details);
      }

      const ClassDesc &classDesc() const noexcept
      {
        static const ClassDesc Empty;
        return Kind == RuntimeKind::Class && Details ? static_cast<const ClassDesc &>(*Details) : Empty;
      }

      ClassDesc &editClass()
      {
        assert(Kind == RuntimeKind::Class && Details);
        if (Details.use_count() != 1)
        {
          Details = Details->clone();
        }
        return static_cast<ClassDesc &>(*Details);
      }

    private:
      std::shared_ptr<TypeDetails> Details;
      friend class RuntimeTypeTable;
  };
} // namespace ink::execution

#endif
