#ifndef INK_IR_PARAMETER_H
#define INK_IR_PARAMETER_H

#include "ink/core/name.h"

namespace ink::ir
{
  class Type;

  // A reusable parameter descriptor for function and generic signatures.
  class Parameter final
  {
    public:
      // ParameterType is borrowed and must outlive users of this descriptor.
      // A null type is preserved; consumers requiring resolved types must reject it.
      // NAME_None denotes an unnamed parameter. Variadic describes this parameter only.
      explicit Parameter(const Type *ParameterType, core::Name Name = core::NAME_None, bool Variadic = false) noexcept;

      const Type *type() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      bool isVariadic() const noexcept;

    private:
      const Type *ParameterType = nullptr;
      core::Name Name = core::NAME_None;
      bool Variadic = false;
  };
} // namespace ink::ir

#endif
