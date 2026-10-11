#ifndef INK_IR_PARAM_ARGUMENT_H
#define INK_IR_PARAM_ARGUMENT_H

#include "ink/core/name.h"

namespace ink::ir
{
  class Value;

  // A borrowed call-site operand; the referenced value must outlive this argument.
  class Argument final
  {
    public:
      explicit Argument(const Value &ArgumentValue, core::Name Name = core::NAME_None) noexcept;

      const Value *value() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;

    private:
      const Value *ArgumentValue = nullptr;
      core::Name Name = core::NAME_None;
  };
} // namespace ink::ir

#endif
