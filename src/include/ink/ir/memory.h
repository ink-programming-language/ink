#ifndef INK_IR_MEMORY_H
#define INK_IR_MEMORY_H

#include "ink/ir/instruction.h"

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
    std::unique_ptr<Value> Size;
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
    std::unique_ptr<Value> Pointer;
    // The root index advances by ElementType stride; each following constant i32 index selects one struct field.
    std::unique_ptr<Value> Index;
    std::vector<std::unique_ptr<Value>> FieldIndices;
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
    std::unique_ptr<Value> Pointer;
  };

  class StoreInstruction final : public Instruction
  {
  public:
    StoreInstruction() noexcept
        : Instruction(InstructionKind::Store)
    {
    }

    std::unique_ptr<Value> StoredValue;
    std::unique_ptr<Value> Pointer;
  };

  class LifetimeEndInstruction final : public Instruction
  {
  public:
    LifetimeEndInstruction() noexcept
        : Instruction(InstructionKind::LifetimeEnd)
    {
    }

    std::unique_ptr<Value> Slice;
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
    std::unique_ptr<Value> Slice;
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
    std::unique_ptr<Value> Slice;
  };
} // namespace ink::ir

#endif
