#ifndef INK_SEMANTIC_MODEL_INSTRUCTION_H
#define INK_SEMANTIC_MODEL_INSTRUCTION_H

#include "ink/semantic/model/value.h"

namespace ink::semantic
{
  // Base for semantic instructions. Concrete instructions provide their value type.
  class Instruction : public Value
  {
    public:
      static bool classof(const Value *ValueObject) noexcept
      {
        if (!ValueObject)
        {
          return false;
        }
        switch (ValueObject->kind())
        {
#define INK_SEMANTIC_INSTRUCTION_VALUE(Name) case ValueKind::Name:
#include "ink/semantic/model/Values.def"
          return true;
        default:
          return false;
        }
      }

    protected:
      explicit Instruction(ValueKind Kind) noexcept
          : Value(Kind)
      {
      }
  };
} // namespace ink::semantic

#endif
