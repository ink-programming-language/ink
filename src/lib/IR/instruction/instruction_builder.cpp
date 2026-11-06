#include "ink/ir/instruction/instruction_builder.h"

#include "ink/core/support/core_define.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/pointer_type.h"
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

  void InstructionBuilder::checkInsertPoint([[maybe_unused]] InstructionKind Kind) const noexcept
  {
    checkMsgf(InsertBlock != nullptr, "InstructionBuilder requires an insertion point");
    checkMsgf(InsertPosition != InsertBlock->Instructions.end() || !InsertBlock->isTerminated(), "Cannot append instructions after a terminator");
    checkMsgf(!Instruction::isTerminatorKind(Kind) || InsertPosition == InsertBlock->Instructions.end(), "Terminators must be inserted at the end of a block");
#if INK_ENABLE_ASSERTIONS
    if (Kind == PhiInstruction::Kind)
    {
      for (auto Position = InsertBlock->Instructions.begin(); Position != InsertPosition; ++Position)
      {
        checkMsgf(PhiInstruction::classof(Position->get()), "Phi instructions must form a prefix at the start of a block");
      }
    }
    else
    {
      checkMsgf(InsertPosition == InsertBlock->Instructions.end() || !PhiInstruction::classof(InsertPosition->get()), "Non-phi instructions cannot be inserted before phi instructions");
    }
#endif
  }

  template <typename InstructionType, typename... Arguments>
  InstructionType &InstructionBuilder::createInstruction(Arguments &&...Args)
  {
    const auto Destroy = [](Instruction *Value) noexcept
    {
      delete static_cast<InstructionType *>(Value);
    };
    BasicBlock::InstructionOwner Owned(new InstructionType(std::forward<Arguments>(Args)...), Destroy);
    auto *Created = static_cast<InstructionType *>(Owned.get());
    InsertBlock->Instructions.insert(InsertPosition, std::move(Owned));
    Created->Parent = InsertBlock;
    return *Created;
  }

  /* -------------------- Add -------------------- */
  AddInstruction &InstructionBuilder::createAdd(ValueRef LHS, ValueRef RHS)
  {
    checkInsertPoint(AddInstruction::Kind);
    [[maybe_unused]] const Value &OperandType = LHS.get().getType();
    checkMsgf(&OperandType == &RHS.get().getType(), "Add operands must have the same type");
    checkMsgf(IntegerType::classof(&OperandType) || FloatType::classof(&OperandType), "Add requires integer or floating-point operands");
    return createInstruction<AddInstruction>(LHS, RHS);
  }

  /* -------------------- Alloca -------------------- */
  AllocaInstruction &InstructionBuilder::createAlloca(ValueRef AllocatedType)
  {
    checkInsertPoint(AllocaInstruction::Kind);
    checkMsgf(AllocatedType.get().isTypeValue(), "Alloca requires a type value");
    return createInstruction<AllocaInstruction>(Types.getPointerType(AllocatedType));
  }

  /* -------------------- Load -------------------- */
  LoadInstruction &InstructionBuilder::createLoad(ValueRef Source)
  {
    checkInsertPoint(LoadInstruction::Kind);
    const Value &SourceType = Source.get().getType();
    checkMsgf(PointerType::classof(&SourceType), "Load requires a pointer source");
    return createInstruction<LoadInstruction>(static_cast<const PointerType &>(SourceType).pointeeType(), Source);
  }

  /* -------------------- Store -------------------- */
  StoreInstruction &InstructionBuilder::createStore(ValueRef Destination, ValueRef Source)
  {
    checkInsertPoint(StoreInstruction::Kind);
    const Value &DestinationType = Destination.get().getType();
    checkMsgf(PointerType::classof(&DestinationType), "Store requires a pointer destination");
    [[maybe_unused]] const Value &StoredType = static_cast<const PointerType &>(DestinationType).pointeeType();
    checkMsgf(&StoredType == &Source.get().getType(), "Store source type must match the destination pointee type");
    return createInstruction<StoreInstruction>(Types.getVoidType(), Destination, Source);
  }

  /* -------------------- Phi -------------------- */
  PhiInstruction &InstructionBuilder::createPhi(ValueRef ResultType)
  {
    checkInsertPoint(PhiInstruction::Kind);
    checkMsgf(ResultType.get().isTypeValue() && !VoidType::classof(&ResultType.get()), "Phi requires a non-void type value");
    return createInstruction<PhiInstruction>(ResultType);
  }

  /* -------------------- Return -------------------- */
  ReturnInstruction &InstructionBuilder::createReturn()
  {
    checkInsertPoint(ReturnInstruction::Kind);
    if (InsertBlock->parent())
    {
      checkMsgf(VoidType::classof(&InsertBlock->parent()->signature().returnType()), "Void return requires a void function return type");
    }
    return createInstruction<ReturnInstruction>(Types.getVoidType());
  }

  ReturnInstruction &InstructionBuilder::createReturn(ValueRef ReturnValue)
  {
    checkInsertPoint(ReturnInstruction::Kind);
    checkMsgf(!VoidType::classof(&ReturnValue.get().getType()), "Return value must have a non-void type");
    if (InsertBlock->parent())
    {
      checkMsgf(&ReturnValue.get().getType() == &InsertBlock->parent()->signature().returnType(), "Return value type must match the function return type");
    }
    return createInstruction<ReturnInstruction>(Types.getVoidType(), ReturnValue);
  }

  /* -------------------- Branch -------------------- */
  BranchInstruction &InstructionBuilder::createBranch(BasicBlock &Target)
  {
    checkInsertPoint(BranchInstruction::Kind);
    checkMsgf(InsertBlock->parent() != nullptr && Target.parent() == InsertBlock->parent(), "Branch target must belong to the insertion function");
    return createInstruction<BranchInstruction>(Types.getVoidType(), Target);
  }

  CondBranchInstruction &InstructionBuilder::createCondBranch(ValueRef Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget)
  {
    checkInsertPoint(CondBranchInstruction::Kind);
    checkMsgf(InsertBlock->parent() != nullptr && TrueTarget.parent() == InsertBlock->parent() && FalseTarget.parent() == InsertBlock->parent(), "Conditional branch targets must belong to the insertion function");
    checkMsgf(BoolType::classof(&Condition.get().getType()), "Conditional branch requires a boolean condition");
    return createInstruction<CondBranchInstruction>(Types.getVoidType(), Condition, TrueTarget, FalseTarget);
  }
} // namespace ink::ir
