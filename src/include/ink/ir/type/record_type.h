#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/enums.h"
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
      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && (static_cast<const Type *>(Value)->getKind() == TypeKind::Struct || static_cast<const Type *>(Value)->getKind() == TypeKind::Object);
      }

      std::span<const Value *const> bases() const noexcept
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

      void addBase(ValueRef Base)
      {
        Bases.push_back(&Base.get());
      }

      void addField(core::Name Name, ValueRef FieldType, MemberKind Kind = MemberKind::Instance, SymbolVisibility Visibility = SymbolVisibility::Public)
      {
        Fields.emplace_back(Name, FieldType, Kind, Visibility);
      }

      void addMethod(const Function &FunctionValue, MemberKind Kind = MemberKind::Instance)
      {
        Methods.emplace_back(FunctionValue, Kind);
      }

      void addMethod(const Function &&, MemberKind = MemberKind::Instance) = delete;

    protected:
      constexpr explicit RecordType(TypeKind Kind) noexcept
          : Type(Kind)
      {
      }

      // Records are destroyed through StructType or ObjectType, never through RecordType.
      ~RecordType() = default;

    private:
      std::vector<const Value *> Bases = {};
      std::vector<Field> Fields = {};
      std::vector<Method> Methods = {};
  };
} // namespace ink::ir
