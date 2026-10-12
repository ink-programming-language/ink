#include "ink/ir/instruction/getelementptr_instruction.h"

#include "ink/ir/type/structure_type/pointer_type.h"

#include <utility>

namespace ink::ir
{
  GetElementPtrInstruction::GetElementPtrInstruction(const PointerType &ResultType, const Type &SourceElementType, const Value &Pointer, std::vector<const Value *> Indices, bool InBounds) noexcept
      : Instruction(ValueKind::GetElementPtrInstruction, &ResultType),
        SourceElementType(&SourceElementType),
        Pointer(&Pointer),
        Indices(std::move(Indices)),
        InBounds(InBounds)
  {
  }

  const Type *GetElementPtrInstruction::sourceElementType() const noexcept
  {
    return SourceElementType;
  }

  const Value *GetElementPtrInstruction::pointer() const noexcept
  {
    return Pointer;
  }

  const std::vector<const Value *> &GetElementPtrInstruction::indices() const noexcept
  {
    return Indices;
  }

  std::size_t GetElementPtrInstruction::indexCount() const noexcept
  {
    return Indices.size();
  }

  const Value *GetElementPtrInstruction::index(std::size_t Index) const noexcept
  {
    return Index < Indices.size() ? Indices[Index] : nullptr;
  }

  bool GetElementPtrInstruction::isInBounds() const noexcept
  {
    return InBounds;
  }

  bool GetElementPtrInstruction::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::GetElementPtrInstruction;
  }
} // namespace ink::ir
