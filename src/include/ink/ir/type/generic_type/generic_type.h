#ifndef INK_IR_TYPE_GENERIC_TYPE_GENERIC_TYPE_H
#define INK_IR_TYPE_GENERIC_TYPE_GENERIC_TYPE_H

#include "ink/core/name/name.h"
#include "ink/ir/type/type.h"

#include <cstddef>
#include <vector>

namespace ink::ir
{
  // Shared declaration metadata for templates awaiting instantiation.
  class GenericType : public Type
  {
    public:
      ~GenericType() noexcept override = 0;

      core::Name name() const noexcept;
      const std::vector<const Type *> &genericParameters() const noexcept;
      std::size_t genericParameterCount() const noexcept;

      // Templates have no runtime layout. Zero size and unit alignment are placeholders,
      // not the layout of an instantiated value; consumers must check hasLayout().
      bool hasLayout() const noexcept final;
      std::uint64_t size() const noexcept final;
      std::uint64_t alignment() const noexcept final;
      std::string toString() const override;
      static bool classof(const Value *Value) noexcept;

    protected:
      // Owns the generic parameter pointer list and borrows its types. Null types
      // and empty lists are preserved for later semantic resolution and validation.
      GenericType(ValueKind Kind, const MetaType &Meta, core::Name Name, std::vector<const Type *> GenericParameters) noexcept;

    private:
      core::Name Name = core::NAME_None;
      std::vector<const Type *> GenericParameters;
  };
} // namespace ink::ir

#endif
