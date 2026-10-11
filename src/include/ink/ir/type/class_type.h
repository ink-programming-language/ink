#ifndef INK_IR_TYPE_CLASS_TYPE_H
#define INK_IR_TYPE_CLASS_TYPE_H

#include "ink/ir/type/nominal_type.h"

namespace ink::ir
{
  class TypeContext;

  // A class identity with member metadata and a completed, immutable layout.
  class ClassType final : public NominalType
  {
    public:
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      ClassType(const TypeContext &Context, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept;
  };
} // namespace ink::ir

#endif
