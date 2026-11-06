#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/enums.h"
#include "ink/ir/type/pointer_type.h"
#include "ink/ir/value/value.h"

#include <utility>

namespace ink::ir
{
  class Module;

  // Models module-level storage as an address value of PointerType(T).
  // The pointer type and its stored type value must outlive this declaration and its uses.
  class GlobalVariable final : public Value
  {
    public:
      explicit GlobalVariable(const PointerType &ResultType, core::Name Name = core::NAME_None, SymbolVisibility Visibility = SymbolVisibility::Private, NativeLinkage Linkage = NativeLinkage::None) noexcept;
      GlobalVariable(const PointerType &&, core::Name = core::NAME_None, SymbolVisibility = SymbolVisibility::Private, NativeLinkage = NativeLinkage::None) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      const Value &valueType() const noexcept
      {
        return static_cast<const PointerType &>(getType()).pointeeType();
      }

      SymbolVisibility visibility() const noexcept
      {
        return Visibility;
      }

      void setVisibility(SymbolVisibility Value) noexcept
      {
        Visibility = Value;
      }

      NativeLinkage nativeLinkage() const noexcept
      {
        return Linkage;
      }

      void setNativeLinkage(NativeLinkage Value) noexcept
      {
        Linkage = Value;
      }

      auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

    private:
      friend class Module;

      core::Name Name = core::NAME_None;
      SymbolVisibility Visibility = SymbolVisibility::Private;
      NativeLinkage Linkage = NativeLinkage::None;
      Module *Parent = nullptr;
  };
} // namespace ink::ir
