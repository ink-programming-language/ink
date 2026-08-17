#ifndef INK_IR_MEMORY_H
#define INK_IR_MEMORY_H

#include "ink/ir/instruction/instruction.h"

#include <memory>
#include <vector>

namespace ink::ir
{
  class AllocaInstruction final : public Instruction
  {
    public:
      explicit AllocaInstruction(const Type &ResultType) noexcept
          : Instruction(InstructionKind::Alloca),
            ResultType(&ResultType)
      {
      }

      ValueId Result;
      const Type *ResultType;
      ValueHandle Size;
  };

  class GetElementPointerInstruction final : public Instruction
  {
    public:
      GetElementPointerInstruction(const Type &ResultType, const Type &ElementType) noexcept
          : Instruction(InstructionKind::GetElementPointer),
            ResultType(&ResultType),
            ElementType(&ElementType)
      {
      }

      ValueId Result;
      const Type *ResultType;
      const Type *ElementType;
      ValueHandle Pointer;
      // The root index advances by ElementType stride; each following constant i32 index selects one struct field.
      ValueHandle Index;
      std::vector<ValueHandle> FieldIndices;
  };

  class LoadInstruction final : public Instruction
  {
    public:
      explicit LoadInstruction(const Type &ResultType) noexcept
          : Instruction(InstructionKind::Load),
            ResultType(&ResultType)
      {
      }

      ValueId Result;
      const Type *ResultType;
      ValueHandle Pointer;
  };

  class StoreInstruction final : public Instruction
  {
    public:
      StoreInstruction() noexcept
          : Instruction(InstructionKind::Store)
      {
      }

      ValueHandle StoredValue;
      ValueHandle Pointer;
  };

  class LifetimeEndInstruction final : public Instruction
  {
    public:
      LifetimeEndInstruction() noexcept
          : Instruction(InstructionKind::LifetimeEnd)
      {
      }

      ValueHandle Slice;
  };

  class SliceDataInstruction final : public Instruction
  {
    public:
      explicit SliceDataInstruction(const Type &ResultType) noexcept
          : Instruction(InstructionKind::SliceData),
            ResultType(&ResultType)
      {
      }

      ValueId Result;
      const Type *ResultType;
      ValueHandle Slice;
  };

  class SliceLengthInstruction final : public Instruction
  {
    public:
      explicit SliceLengthInstruction(const Type &ResultType) noexcept
          : Instruction(InstructionKind::SliceLength),
            ResultType(&ResultType)
      {
      }

      ValueId Result;
      const Type *ResultType;
      ValueHandle Slice;
  };
} // namespace ink::ir

#endif
