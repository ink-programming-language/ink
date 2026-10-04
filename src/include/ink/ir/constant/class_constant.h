#ifndef INK_IR_CONSTANT_CLASS_CONSTANT_H
#define INK_IR_CONSTANT_CLASS_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/class_type.h"

namespace ink::ir
{
  class ClassConstant final : public Constant
  {
    public:
      const ClassType &classType() const noexcept
      {
        return static_cast<const ClassType &>(type());
      }

      std::span<const Constant *const> fields() const noexcept
      {
        return Fields;
      }

      static bool classof(const Value *Object) noexcept
      {
        return Object && Object->kind() == ValueKind::ClassConstant;
      }

    private:
      ClassConstant(const ClassType &ValueType, std::span<const Constant *const> Fields)
          : Constant(ValueKind::ClassConstant, ValueType),
            Fields(Fields.begin(), Fields.end())
      {
      }

      std::vector<const Constant *> Fields;

      friend class ConstantPool;
  };
} // namespace ink::ir

#endif
