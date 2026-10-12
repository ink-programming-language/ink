#ifndef INK_IR_TYPE_GENERIC_TYPE_GENERIC_VAR_TYPE_H
#define INK_IR_TYPE_GENERIC_TYPE_GENERIC_VAR_TYPE_H

#include "ink/ir/type/generic_type/generic_type.h"

namespace ink::ir
{
  class TypeContext;

  // Describes a variable declaration template and the type of its instantiated values.
  class GenericVarType final : public GenericType
  {
    public:
      const Type *valueType() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      // Owns the generic parameter list and borrows its types and ValueType.
      // A null ValueType is preserved for declarations awaiting semantic resolution.
      GenericVarType(core::Name Name, std::vector<const Type *> GenericParameters, const Type *ValueType) noexcept;

      const Type *ValueType = nullptr;
  };
} // namespace ink::ir

#endif
