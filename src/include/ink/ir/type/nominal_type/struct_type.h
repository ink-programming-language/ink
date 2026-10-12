#ifndef INK_IR_TYPE_NOMINAL_TYPE_STRUCT_TYPE_H
#define INK_IR_TYPE_NOMINAL_TYPE_STRUCT_TYPE_H

#include "ink/ir/type/nominal_type/nominal_type.h"

namespace ink::ir
{
  class TypeContext;

  // A struct identity with member metadata and a completed, immutable layout.
  class StructType final : public NominalType
  {
    public:
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      StructType(const TypeContext &Context, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept;
  };
} // namespace ink::ir

#endif
