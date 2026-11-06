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
  // Owns method functions; borrowed bases, field types, and method signatures must remain alive during use.
  // A common owner can keep recursive record/reference graphs alive throughout use; destruction does not access borrowed types.
  // Appending declarations may invalidate views and references into the corresponding collection; method function addresses remain stable.
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

      std::span<Method> methods() noexcept
      {
        return Methods;
      }

      std::span<const Method> methods() const noexcept
      {
        return Methods;
      }

      void addBase(ValueRef Base)
      {
        Bases.push_back(&Base.get());
      }

      void addField(core::Name Name, ValueRef FieldType, MemberKind KindValue = MemberKind::Instance, SymbolVisibility Visibility = SymbolVisibility::Public)
      {
        Fields.emplace_back(Name, FieldType, KindValue, Visibility);
      }

      Method &addMethod(const FunctionType &Signature, core::Name Name = core::NAME_None, MemberKind KindValue = MemberKind::Instance, SymbolVisibility Visibility = SymbolVisibility::Private, NativeLinkage Linkage = NativeLinkage::None)
      {
        return Methods.emplace_back(Signature, Name, KindValue, Visibility, Linkage);
      }

      Method &addMethod(const FunctionType &&, core::Name = core::NAME_None, MemberKind = MemberKind::Instance, SymbolVisibility = SymbolVisibility::Private, NativeLinkage = NativeLinkage::None) = delete;

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
