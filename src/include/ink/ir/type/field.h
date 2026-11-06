#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/enums.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  // Copies an interned name handle; the borrowed type must remain alive whenever the field is accessed.
  class Field
  {
    public:
      Field(core::Name Name, ValueRef FieldType, MemberKind Kind = MemberKind::Instance, SymbolVisibility Visibility = SymbolVisibility::Public) noexcept
          : Name(Name),
            FieldType(&FieldType.get()),
            Kind(Kind),
            Visibility(Visibility)
      {
      }

      core::Name name() const noexcept
      {
        return Name;
      }

      const Value &type() const noexcept
      {
        return *FieldType;
      }

      MemberKind kind() const noexcept
      {
        return Kind;
      }

      bool isStatic() const noexcept
      {
        return Kind == MemberKind::Static;
      }

      SymbolVisibility visibility() const noexcept
      {
        return Visibility;
      }

    private:
      core::Name Name = core::NAME_None;
      const Value *FieldType = nullptr;
      MemberKind Kind = MemberKind::Instance;
      SymbolVisibility Visibility = SymbolVisibility::Public;
  };
} // namespace ink::ir
