#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/reference_type.h"
#include "ink/ir/type/type_pool.h"

#include <utility>

namespace ink::ir
{
  InstructionBuilder::InstructionBuilder(TypePool &Types) noexcept
      : Types(Types)
  {
  }

  void InstructionBuilder::setInsertPoint(BasicBlock &Block) noexcept
  {
    InsertBlock = &Block;
    InsertPosition = Block.Instructions.end();
  }

  bool InstructionBuilder::setInsertPoint(Instruction &Before) noexcept
  {
    BasicBlock *Block = Before.parent();
    if (!Block)
    {
      return false;
    }
    for (auto Position = Block->Instructions.begin(); Position != Block->Instructions.end(); ++Position)
    {
      if (Position->get() == &Before)
      {
        InsertBlock = Block;
        InsertPosition = Position;
        return true;
      }
    }
    return false;
  }

  void InstructionBuilder::clearInsertPoint() noexcept
  {
    InsertBlock = nullptr;
    InsertPosition = {};
  }

  BasicBlock *InstructionBuilder::getInsertBlock() const noexcept
  {
    return InsertBlock;
  }

  Instruction *InstructionBuilder::getInsertBefore() const noexcept
  {
    if (!InsertBlock || InsertPosition == InsertBlock->Instructions.end())
    {
      return nullptr;
    }
    return InsertPosition->get();
  }

  template <typename InstructionType, typename... Arguments>
  InstructionType *InstructionBuilder::createInstruction(Arguments &&...Args)
  {
    if (!InsertBlock)
    {
      return nullptr;
    }
    const auto Destroy = [](Instruction *Value) noexcept
    {
      delete static_cast<InstructionType *>(Value);
    };
    BasicBlock::InstructionOwner Owned(new InstructionType(std::forward<Arguments>(Args)...), Destroy);
    auto *Created = static_cast<InstructionType *>(Owned.get());
    InsertBlock->Instructions.insert(InsertPosition, std::move(Owned));
    Created->Parent = InsertBlock;
    return Created;
  }

  /* -------------------- Add -------------------- */
  AddInstruction *InstructionBuilder::createAdd(ValueRef LHS, ValueRef RHS)
  {
    return createInstruction<AddInstruction>(LHS, RHS);
  }

  /* -------------------- Alloca -------------------- */
  AllocaInstruction *InstructionBuilder::createAlloca(const Type &AllocatedType)
  {
    if (!InsertBlock)
    {
      return nullptr;
    }
    return createInstruction<AllocaInstruction>(Types.getReferenceType(AllocatedType));
  }

  /* -------------------- Load -------------------- */
  LoadInstruction *InstructionBuilder::createLoad(ValueRef Source)
  {
    if (!InsertBlock)
    {
      return nullptr;
    }
    const Type &SourceType = Source.get().getType();
    if (!ReferenceType::classof(&SourceType))
    {
      return nullptr;
    }
    return createInstruction<LoadInstruction>(static_cast<const ReferenceType &>(SourceType).referencedType(), Source);
  }

  /* -------------------- Store -------------------- */
  StoreInstruction *InstructionBuilder::createStore(ValueRef Destination, ValueRef Source)
  {
    if (!InsertBlock)
    {
      return nullptr;
    }
    const Type &DestinationType = Destination.get().getType();
    if (!ReferenceType::classof(&DestinationType))
    {
      return nullptr;
    }
    const Type &StoredType = static_cast<const ReferenceType &>(DestinationType).referencedType();
    if (&StoredType != &Source.get().getType())
    {
      return nullptr;
    }
    return createInstruction<StoreInstruction>(Types.getVoidType(), Destination, Source);
  }

  /* -------------------- Return -------------------- */
  ReturnInstruction *InstructionBuilder::createReturn()
  {
    return createInstruction<ReturnInstruction>(Types.getVoidType());
  }

  ReturnInstruction *InstructionBuilder::createReturn(ValueRef ReturnValue)
  {
    return createInstruction<ReturnInstruction>(Types.getVoidType(), ReturnValue);
  }
} // namespace ink::ir
