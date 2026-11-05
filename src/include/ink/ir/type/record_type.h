#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/type/field.h"
#include "ink/ir/type/method.h"

#include <span>
#include <vector>

namespace ink::ir
{
  // Stores declarations from this record only, in insertion order, without language-level validation.
  // Borrowed bases, field types, and method functions must remain alive whenever their declarations are accessed.
  // A common owner can keep recursive record/reference graphs alive throughout use; destruction does not access borrowed types.
  // Appending declarations may invalidate views and references into the corresponding collection.
  class RecordType : public Type
  {
    public:
      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && (Value->getKind() == TypeKind::Struct || Value->getKind() == TypeKind::Object);
      }

      std::span<const RecordType *const> bases() const noexcept
      {
        return Bases;
      }

      std::span<const Field> fields() const noexcept
      {
        return Fields;
      }

      std::span<const Method> methods() const noexcept
      {
        return Methods;
      }

      void addBase(const RecordType &Base)
      {
        Bases.push_back(&Base);
      }

      void addBase(const RecordType &&) = delete;

      void addField(core::Name Name, const Type &FieldType)
      {
        Fields.emplace_back(Name, FieldType);
      }

      void addField(core::Name, const Type &&) = delete;

      void addMethod(const Function &FunctionValue)
      {
        Methods.emplace_back(FunctionValue);
      }

      void addMethod(const Function &&) = delete;

    protected:
      constexpr explicit RecordType(TypeKind Kind) noexcept
          : Type(Kind)
      {
      }

      // Records are destroyed through StructType or ObjectType, never through RecordType.
      ~RecordType() = default;

    private:
      std::vector<const RecordType *> Bases;
      std::vector<Field> Fields;
      std::vector<Method> Methods;
  };
} // namespace ink::ir
