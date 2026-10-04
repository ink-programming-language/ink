#ifndef INK_IR_CLASS_INSTRUCTION_H
#define INK_IR_CLASS_INSTRUCTION_H

#include "ink/ir/type/class_type.h"

namespace ink::ir
{
  // Operands are already evaluated, in declaration order. The result is a value snapshot.
  class ClassInstruction final : public Value
  {
    public:
      const ClassType &classType() const noexcept
      {
        return static_cast<const ClassType &>(type());
      }

      std::span<const Value *const> fields() const noexcept
      {
        return Fields;
      }

      static bool classof(const Value *Object) noexcept
      {
        return Object && Object->kind() == ValueKind::ClassInstruction;
      }

    private:
      ClassInstruction(const ClassType &ValueType, std::span<const Value *const> Fields)
          : Value(ValueType.context(), ValueKind::ClassInstruction, ValueType),
            Fields(Fields.begin(), Fields.end())
      {
      }

      std::vector<const Value *> Fields;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
