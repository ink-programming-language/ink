#include "ownership_test_support.h"

#include "ink/semantic/ir_builder.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <vector>

namespace ink::semantic::test
{
  static_assert(!std::is_copy_constructible_v<IRBuilder>);
  static_assert(!std::is_move_constructible_v<IRBuilder>);
  static_assert(!std::is_copy_constructible_v<IRBuilder::InsertPointGuard>);
  static_assert(!std::is_move_constructible_v<IRBuilder::InsertPointGuard>);

  // Every instruction factory attaches in order, retains operand identity, and leaves ownership with Context.
  TEST(SemanticIRBuilderTest, CreatesOrderedInstructionsThatOutliveBuilder)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 1));
    auto MainOwner = Factory.createFunction(Context.namePool().intern("Main"), *Context.typePool().getType<TypeKind::Function>(*Int32));
    Function *Main = MainOwner.get();
    BasicBlock *Entry = Factory.createFunctionBody(*Main);
    const Type *Parameters[] = {Int32};
    auto TargetOwner = Factory.createFunction(Context.namePool().intern("Target"), *Context.typePool().getType<TypeKind::Function>(*Int32, Parameters));
    Function *Target = TargetOwner.get();
    std::vector<Value *> Expected;
    {
      IRBuilder Builder(Context);
      EXPECT_EQ(&Builder.context(), &Context);
      ASSERT_TRUE(Builder.setInsertPoint(*Entry));
      AllocaInstruction *Slot = Builder.createAllocaInstruction(*Int32);
      ASSERT_NE(Slot, nullptr);
      StoreInstruction *Store = Builder.createStoreInstruction(*Slot, *One);
      LoadInstruction *Load = Builder.createLoadInstruction(*Slot);
      ASSERT_NE(Store, nullptr);
      ASSERT_NE(Load, nullptr);
      AddInstruction *Add = Builder.createAddInstruction(*Load, *One);
      ASSERT_NE(Add, nullptr);
      const Value *Arguments[] = {Add};
      CallInstruction *Call = Builder.createCallInstruction(*Target, Arguments);
      ASSERT_NE(Call, nullptr);
      ReturnInstruction *Return = Builder.createReturnInstruction(Call);
      ASSERT_NE(Return, nullptr);
      EXPECT_EQ(&Store->address(), Slot);
      EXPECT_EQ(&Store->storedValue(), One);
      EXPECT_EQ(&Load->address(), Slot);
      EXPECT_EQ(&Add->left(), Load);
      EXPECT_EQ(&Add->right(), One);
      EXPECT_EQ(Call->directCallee(), Target);
      EXPECT_EQ(Call->arguments()[0], Add);
      EXPECT_EQ(Return->returnedValue(), Call);
      EXPECT_EQ(Return->function(), Main);
      Expected = {
          Slot,
          Store,
          Load,
          Add,
          Call,
          Return,
      };
    }
    EXPECT_EQ(borrowedPointers(Entry->values()), Expected);
    for (Value *Instruction : Expected)
    {
      EXPECT_EQ(Instruction->outer(), Entry);
      EXPECT_EQ(&Instruction->context(), &Context);
    }
    EXPECT_EQ(One->outer(), nullptr);
    EXPECT_EQ(Target->outer(), nullptr);
  }

  // Inserting repeatedly before one anchor preserves call order through vector growth and external edits.
  TEST(SemanticIRBuilderTest, AnchorSurvivesGrowthAndOtherInsertions)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    auto BlockOwner = Factory.createBasicBlock();
    BasicBlock *Block = BlockOwner.get();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    AllocaInstruction *Anchor = Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Anchor, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
    const IRBuilder::InsertPoint Saved = Builder.saveInsertPoint();
    std::vector<Value *> Expected;
    for (unsigned Index = 0; Index < 128; ++Index)
    {
      AllocaInstruction *Slot = Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
      ASSERT_NE(Slot, nullptr);
      Expected.push_back(Slot);
    }
    auto ExternalOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *External = ExternalOwner.get();
    ASSERT_TRUE(Factory.insertValue(*Block, std::move(ExternalOwner), Anchor));
    Expected.push_back(External);
    ASSERT_TRUE(Factory.eraseValue(*Block, *Expected.front()));
    Expected.erase(Expected.begin());
    Builder.clearInsertPoint();
    ASSERT_TRUE(Builder.restoreInsertPoint(Saved));
    AllocaInstruction *Last = Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Last, nullptr);
    Expected.push_back(Last);
    Expected.push_back(Anchor);
    EXPECT_EQ(borrowedPointers(Block->values()), Expected);
    EXPECT_EQ(Builder.saveInsertPoint().Before, Anchor);
  }

  // Builders sharing a context keep independent points, and allocations honor a selected non-entry block.
  TEST(SemanticIRBuilderTest, IndependentBuildersFollowExplicitBlockSelection)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    auto MainOwner = Factory.createFunction(Context.namePool().intern("Main"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>()));
    Function *Main = MainOwner.get();
    BasicBlock *Entry = Factory.createBasicBlock(*Main);
    BasicBlock *Tail = Factory.createBasicBlock(*Main);
    IRBuilder First(Context);
    IRBuilder Second(Context);
    ASSERT_TRUE(First.setInsertPoint(*Entry));
    ASSERT_TRUE(Second.setInsertPoint(*Tail));
    AllocaInstruction *EntrySlot = First.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *TailSlot = Second.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(EntrySlot, nullptr);
    ASSERT_NE(TailSlot, nullptr);
    EXPECT_EQ(EntrySlot->outer(), Entry);
    EXPECT_EQ(TailSlot->outer(), Tail);
    ASSERT_TRUE(First.setInsertPoint(*Tail, TailSlot));
    AllocaInstruction *BeforeTail = First.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *AfterTail = Second.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(BeforeTail, nullptr);
    ASSERT_NE(AfterTail, nullptr);
    EXPECT_EQ(borrowedPointers(Entry->values()), std::vector<Value *>({EntrySlot}));
    EXPECT_EQ(borrowedPointers(Tail->values()), std::vector<Value *>({BeforeTail, TailSlot, AfterTail}));
  }

  // Nested guards restore the saved anchor, block end, and initially unset point independently.
  TEST(SemanticIRBuilderTest, NestedGuardsRestoreAnchorsEndsAndUnsetPoints)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    auto FirstOwner = Factory.createBasicBlock();
    BasicBlock *First = FirstOwner.get();
    auto SecondOwner = Factory.createBasicBlock();
    BasicBlock *Second = SecondOwner.get();
    IRBuilder Builder(Context);
    {
      IRBuilder::InsertPointGuard UnsetGuard(Builder);
      ASSERT_TRUE(Builder.setInsertPoint(*First));
      AllocaInstruction *Anchor = Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
      ASSERT_NE(Anchor, nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
      {
        IRBuilder::InsertPointGuard AnchorGuard(Builder);
        ASSERT_TRUE(Builder.setInsertPoint(*Second));
        {
          IRBuilder::InsertPointGuard EndGuard(Builder);
          Builder.clearInsertPoint();
        }
        EXPECT_EQ(Builder.insertBlock(), Second);
        EXPECT_EQ(Builder.saveInsertPoint().Before, nullptr);
        ASSERT_NE(Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>()), nullptr);
      }
      EXPECT_EQ(Builder.insertBlock(), First);
      EXPECT_EQ(Builder.saveInsertPoint().Before, Anchor);
      AllocaInstruction *Before = Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
      ASSERT_NE(Before, nullptr);
      EXPECT_EQ(borrowedPointers(First->values()), std::vector<Value *>({Before, Anchor}));
    }
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    EXPECT_EQ(Builder.saveInsertPoint().Before, nullptr);
    EXPECT_EQ(Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>()), nullptr);
  }

  // All six factories reject an unset or explicitly cleared point even when their operands are valid.
  TEST(SemanticIRBuilderTest, MissingPointRejectsEveryFactory)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 1));
    auto TargetOwner = Factory.createFunction(Context.namePool().intern("Target"), *Context.typePool().getType<TypeKind::Function>(*Int32));
    Function *Target = TargetOwner.get();
    BasicBlock *Block = Factory.createFunctionBody(*Target);
    auto SlotOwner = Factory.createDetachedAllocaInstruction(*Int32);
    AllocaInstruction *Slot = SlotOwner.get();
    IRBuilder Builder(Context);
    for (unsigned Attempt = 0; Attempt < 2; ++Attempt)
    {
      EXPECT_EQ(Builder.createCallInstruction(*Target), nullptr);
      EXPECT_EQ(Builder.createAllocaInstruction(*Int32), nullptr);
      EXPECT_EQ(Builder.createLoadInstruction(*Slot), nullptr);
      EXPECT_EQ(Builder.createStoreInstruction(*Slot, *One), nullptr);
      EXPECT_EQ(Builder.createAddInstruction(*One, *One), nullptr);
      EXPECT_EQ(Builder.createReturnInstruction(One), nullptr);
      EXPECT_TRUE(Block->values().empty());
      ASSERT_TRUE(Builder.setInsertPoint(*Block));
      Builder.clearInsertPoint();
    }
    EXPECT_EQ(Slot->outer(), nullptr);
  }

  // Bad anchors and foreign saved points clear a previously valid destination without changing either block.
  TEST(SemanticIRBuilderTest, InvalidPointSelectionClearsOldDestination)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    SemanticContext Foreign(Compilation);
    IRBuilder ForeignFactory(Foreign);
    auto BlockOwner = Factory.createBasicBlock();
    BasicBlock *Block = BlockOwner.get();
    auto OtherBlockOwner = Factory.createBasicBlock();
    BasicBlock *OtherBlock = OtherBlockOwner.get();
    auto ForeignBlockOwner = ForeignFactory.createBasicBlock();
    BasicBlock *ForeignBlock = ForeignBlockOwner.get();
    auto DetachedOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *Detached = DetachedOwner.get();
    auto AnchorOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *Anchor = AnchorOwner.get();
    ASSERT_TRUE(Factory.appendValue(*OtherBlock, std::move(AnchorOwner)));
    IRBuilder Builder(Context);
    IRBuilder Other(Foreign);
    ASSERT_TRUE(Other.setInsertPoint(*ForeignBlock));
    const IRBuilder::InsertPoint InvalidPoints[] = {
        {Block, Detached},
        {Block, Anchor},
        {nullptr, Anchor},
        Other.saveInsertPoint(),
    };
    for (IRBuilder::InsertPoint Point : InvalidPoints)
    {
      ASSERT_TRUE(Builder.setInsertPoint(*Block));
      EXPECT_FALSE(Builder.restoreInsertPoint(Point));
      EXPECT_EQ(Builder.insertBlock(), nullptr);
      EXPECT_EQ(Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>()), nullptr);
    }
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    EXPECT_FALSE(Builder.setInsertPoint(*Detached));
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    EXPECT_FALSE(Builder.setInsertPoint(*ForeignBlock));
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    EXPECT_FALSE(Builder.setInsertPoint(*Block, Anchor));
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    EXPECT_TRUE(Builder.restoreInsertPoint({}));
    EXPECT_TRUE(Block->values().empty());
    EXPECT_TRUE(ForeignBlock->values().empty());
    EXPECT_EQ(borrowedPointers(OtherBlock->values()), std::vector<Value *>({Anchor}));
  }

  // Removing or moving a selected anchor rejects creation and restoration; a guard clears an invalid saved point.
  TEST(SemanticIRBuilderTest, RemovedAndReparentedAnchorsAreRejected)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    auto FirstOwner = Factory.createBasicBlock();
    BasicBlock *First = FirstOwner.get();
    auto SecondOwner = Factory.createBasicBlock();
    BasicBlock *Second = SecondOwner.get();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*First));
    AllocaInstruction *Anchor = Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Anchor, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
    const IRBuilder::InsertPoint Saved = Builder.saveInsertPoint();
    auto AnchorOwner = Factory.removeValue(*First, *Anchor);
    ASSERT_EQ(AnchorOwner.get(), Anchor);
    EXPECT_EQ(Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Bool>()), nullptr);
    EXPECT_FALSE(Builder.restoreInsertPoint(Saved));
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    ASSERT_TRUE(Factory.appendValue(*Second, std::move(AnchorOwner)));
    EXPECT_FALSE(Builder.restoreInsertPoint(Saved));
    ASSERT_TRUE(Builder.setInsertPoint(*Anchor));
    {
      IRBuilder::InsertPointGuard Guard(Builder);
      ASSERT_TRUE(Builder.setInsertPoint(*First));
      AnchorOwner = Factory.removeValue(*Second, *Anchor);
      ASSERT_EQ(AnchorOwner.get(), Anchor);
    }
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    EXPECT_TRUE(First->values().empty());
    EXPECT_TRUE(Second->values().empty());
  }

  // Operand failures leave the block and insertion point intact so a corrected instruction can be emitted.
  TEST(SemanticIRBuilderTest, InvalidOperandsDoNotInsertOrConsumePoint)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    SemanticContext Foreign(Compilation);
    IRBuilder ForeignFactory(Foreign);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 1));
    const Type *Parameters[] = {Int32};
    auto TargetOwner = Factory.createFunction(Context.namePool().intern("Target"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>(), Parameters));
    Function *Target = TargetOwner.get();
    auto ForeignTargetOwner = ForeignFactory.createFunction(Foreign.namePool().intern("Target"), *Foreign.typePool().getType<TypeKind::Function>(Foreign.typePool().getType<TypeKind::Void>()));
    Function *ForeignTarget = ForeignTargetOwner.get();
    auto BlockOwner = Factory.createBasicBlock();
    BasicBlock *Block = BlockOwner.get();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    AllocaInstruction *Slot = Builder.createAllocaInstruction(*Int32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Slot));
    const Value *WrongArguments[] = {&Context.constantPool().getBoolConstant(true)};
    EXPECT_EQ(Builder.createCallInstruction(*Target, WrongArguments), nullptr);
    EXPECT_EQ(Builder.createCallInstruction(*ForeignTarget), nullptr);
    EXPECT_EQ(Builder.createAllocaInstruction(Context.typePool().getType<TypeKind::Void>()), nullptr);
    EXPECT_EQ(Builder.createAllocaInstruction(Foreign.typePool().getType<TypeKind::Bool>()), nullptr);
    EXPECT_EQ(Builder.createLoadInstruction(*One), nullptr);
    EXPECT_EQ(Builder.createStoreInstruction(*Slot, Context.constantPool().getBoolConstant(true)), nullptr);
    EXPECT_EQ(Builder.createStoreInstruction(*Slot, Foreign.constantPool().getBoolConstant(true)), nullptr);
    EXPECT_EQ(Builder.createAddInstruction(*One, Context.constantPool().getBoolConstant(true)), nullptr);
    EXPECT_EQ(borrowedPointers(Block->values()), std::vector<Value *>({Slot}));
    EXPECT_EQ(Builder.insertBlock(), Block);
    EXPECT_EQ(Builder.saveInsertPoint().Before, Slot);
    const Value *Arguments[] = {One};
    CallInstruction *Call = Builder.createCallInstruction(*Target, Arguments);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(borrowedPointers(Block->values()), std::vector<Value *>({Call, Slot}));
  }

  // Returns match the enclosing function, occur only at the end, and prevent later instructions or duplicate returns.
  TEST(SemanticIRBuilderTest, ReturnChecksSignatureAndTerminalPosition)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    SemanticContext Foreign(Compilation);
    auto MainOwner = Factory.createFunction(Context.namePool().intern("Main"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>()));
    Function *Main = MainOwner.get();
    BasicBlock *Block = Factory.createFunctionBody(*Main);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    EXPECT_EQ(Builder.createReturnInstruction(), nullptr);
    EXPECT_EQ(Builder.createReturnInstruction(&Foreign.constantPool().getBoolConstant(true)), nullptr);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 1));
    EXPECT_EQ(Builder.createReturnInstruction(One), nullptr);
    EXPECT_TRUE(Block->values().empty());
    AllocaInstruction *Slot = Builder.createAllocaInstruction(*Int32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Slot));
    EXPECT_EQ(Builder.createReturnInstruction(&Context.constantPool().getBoolConstant(true)), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    ReturnInstruction *Return = Builder.createReturnInstruction(&Context.constantPool().getBoolConstant(true));
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->function(), Main);
    EXPECT_EQ(Builder.createCallInstruction(*Main), nullptr);
    EXPECT_EQ(Builder.createAllocaInstruction(*Int32), nullptr);
    EXPECT_EQ(Builder.createLoadInstruction(*Slot), nullptr);
    EXPECT_EQ(Builder.createStoreInstruction(*Slot, *One), nullptr);
    EXPECT_EQ(Builder.createAddInstruction(*One, *One), nullptr);
    EXPECT_EQ(Builder.createReturnInstruction(&Context.constantPool().getBoolConstant(false)), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Return));
    EXPECT_EQ(Builder.createReturnInstruction(&Context.constantPool().getBoolConstant(false)), nullptr);
    LoadInstruction *Load = Builder.createLoadInstruction(*Slot);
    ASSERT_NE(Load, nullptr);
    EXPECT_EQ(borrowedPointers(Block->values()), std::vector<Value *>({Slot, Load, Return}));
  }

  // Void returns require a void function's block; standalone and module blocks cannot receive returns.
  TEST(SemanticIRBuilderTest, VoidReturnsRequireFunctionBlocks)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    IRBuilder Factory(Context);
    IRBuilder Builder(Context);
    auto DetachedOwner = Factory.createBasicBlock();
    BasicBlock *Detached = DetachedOwner.get();
    Module *ModuleValue = Factory.createModule(Context.namePool().intern("Module"));
    for (BasicBlock *Block : {Detached, &ModuleValue->entryBlock()})
    {
      ASSERT_TRUE(Builder.setInsertPoint(*Block));
      EXPECT_EQ(Builder.createReturnInstruction(), nullptr);
      EXPECT_TRUE(Block->values().empty());
    }
    auto MainOwner = Factory.createFunction(Context.namePool().intern("Main"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>()));
    Function *Main = MainOwner.get();
    BasicBlock *Entry = Factory.createFunctionBody(*Main);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createReturnInstruction(&Context.constantPool().getBoolConstant(true)), nullptr);
    ReturnInstruction *Return = Builder.createReturnInstruction();
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->function(), Main);
    EXPECT_EQ(Return->returnedValue(), nullptr);
    EXPECT_EQ(borrowedPointers(Entry->values()), std::vector<Value *>({Return}));
  }
} // namespace ink::semantic::test
