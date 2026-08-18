#ifndef INK_IR_PARAMETER_H
#define INK_IR_PARAMETER_H

#include "ink/ir/model/name.h"

namespace ink::ir
{
  class Constant;
  class Type;

  class Parameter final
  {
    public:
      explicit Parameter(const Type *ValueType) noexcept;
      Parameter(ink::ir::Name Name, const Type *ValueType, const Constant *DefaultValue = nullptr) noexcept;

      const ink::ir::Name &name() const noexcept;
      const Type *type() const noexcept;
      const Constant *defaultValue() const noexcept;

    private:
      ink::ir::Name Name;
      const Type *ValueType;
      const Constant *DefaultValue;
  };
} // namespace ink::ir

#endif
