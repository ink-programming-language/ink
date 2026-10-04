#ifndef INK_IR_FIELD_EXTRACT_INSTRUCTION_H
#define INK_IR_FIELD_EXTRACT_INSTRUCTION_H

#include "ink/ir/type/class_type.h"

namespace ink::ir
{
  // Reads a field from an immutable class value; no storage alias is created.
  class FieldExtractInstruction final : public Value
  {
    public:
      const Value &object() const noexcept
      {
        return Object;
      }

      std::size_t fieldIndex() const noexcept
      {
        return FieldIndex;
      }

      static bool classof(const Value *Object) noexcept
      {
        return Object && Object->kind() == ValueKind::FieldExtractInstruction;
      }

    private:
      FieldExtractInstruction(const Value &Object, std::size_t FieldIndex) noexcept
          : Value(Object.context(), ValueKind::FieldExtractInstruction, *static_cast<const ClassType &>(Object.type()).fields()[FieldIndex].FieldType),
            Object(Object),
            FieldIndex(FieldIndex)
      {
      }

      const Value &Object;
      std::size_t FieldIndex;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
