#ifndef INK_IR_INSTRUCTION_GETELEMENTPTR_INSTRUCTION_H
#define INK_IR_INSTRUCTION_GETELEMENTPTR_INSTRUCTION_H

#include "ink/ir/instruction/instruction.h"

#include <cstddef>
#include <vector>

namespace ink::ir
{
  class PointerType;

  // Computes an element address without loading memory. Owns the index list and borrows its values.
  // SourceElementType describes the base pointee; ResultType describes the selected element pointer.
  // Indices must be non-null integer values; the first indexes the base pointer, subsequent indices traverse aggregates.
  class GetElementPtrInstruction final : public Instruction
  {
    public:
      GetElementPtrInstruction(const PointerType &ResultType, const Type &SourceElementType, const Value &Pointer, std::vector<const Value *> Indices, bool InBounds = false) noexcept;

      const Type *sourceElementType() const noexcept;
      const Value *pointer() const noexcept;
      const std::vector<const Value *> &indices() const noexcept;
      std::size_t indexCount() const noexcept;
      // Returns nullptr outside the index list.
      const Value *index(std::size_t Index) const noexcept;
      bool isInBounds() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      const Type *SourceElementType = nullptr;
      const Value *Pointer = nullptr;
      std::vector<const Value *> Indices;
      bool InBounds = false;
  };
} // namespace ink::ir

#endif
