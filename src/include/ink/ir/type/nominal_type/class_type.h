#ifndef INK_IR_TYPE_NOMINAL_TYPE_CLASS_TYPE_H
#define INK_IR_TYPE_NOMINAL_TYPE_CLASS_TYPE_H

#include "ink/ir/type/nominal_type/nominal_type.h"

namespace ink::ir
{
  class Module;

  // A class identity with member metadata and a completed, immutable layout.
  class ClassType final : public NominalType
  {
    public:
      static bool classof(const Value *Value) noexcept;

    private:
      friend class Module;
      friend class TypePool;

      ClassType(const Module &Owner, std::uint64_t Size, std::uint64_t Alignment, core::Name Name) noexcept;
      ClassType(const ClassType &Declaration, bool Mutable) noexcept;
  };
} // namespace ink::ir

#endif
