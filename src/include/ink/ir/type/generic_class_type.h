#ifndef INK_IR_TYPE_GENERIC_CLASS_TYPE_H
#define INK_IR_TYPE_GENERIC_CLASS_TYPE_H

#include "ink/ir/type/generic_type.h"

namespace ink::ir
{
  class TypeContext;

  // Describes a class template; member declarations and instantiated layouts are supplied later.
  class GenericClassType final : public GenericType
  {
    public:
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      GenericClassType(core::Name Name, std::vector<const Type *> GenericParameters) noexcept;
  };
} // namespace ink::ir

#endif
