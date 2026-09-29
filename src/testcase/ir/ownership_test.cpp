#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <memory>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    class ObservedValue final : public Value
    {
      public:
        ObservedValue(const IRContext &Context, unsigned &Destructions) noexcept
            : Value(Context, static_cast<ValueKind>(255), Context.typePool().getType<TypeKind::Bool>()),
              Destructions(Destructions)
        {
        }

        ~ObservedValue() override
        {
          ++Destructions;
        }

      private:
        unsigned &Destructions;
    };
  } // namespace

  static_assert(std::is_same_v<decltype(std::declval<IRBuilder &>().createBasicBlock()), std::unique_ptr<BasicBlock>>);
  static_assert(std::is_same_v<decltype(std::declval<IRBuilder &>().createDetachedReturnInstruction()), std::unique_ptr<ReturnInstruction>>);
  static_assert(std::is_same_v<decltype(std::declval<IRBuilder &>().removeValue(std::declval<BasicBlock &>(), std::declval<Value &>())), std::unique_ptr<Value>>);
  static_assert(std::is_same_v<decltype(std::declval<Function &>().parameters()), const std::vector<std::unique_ptr<FunctionParameter>> &>);

  // A failed cross-context or invalid-anchor transfer retains the exact owner, and scope exit destroys it once.
  TEST(IROwnershipTest, FailedTransfersPreserveDetachedOwnership)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Foreign(Compilation);
    IRBuilder ForeignFactory(Foreign);
    auto Block = Factory.createBasicBlock();
    auto ForeignBlock = ForeignFactory.createBasicBlock();
    auto DetachedAnchor = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    unsigned Destructions = 0;
    {
      auto Child = std::make_unique<ObservedValue>(Context, Destructions);
      Value *Pointer = Child.get();
      EXPECT_FALSE(Factory.appendValue(*ForeignBlock, std::move(Child)));
      EXPECT_EQ(Child.get(), Pointer);
      EXPECT_FALSE(Factory.insertValue(*Block, std::move(Child), DetachedAnchor.get()));
      EXPECT_EQ(Child.get(), Pointer);
      EXPECT_EQ(Destructions, 0U);
      EXPECT_EQ(Pointer->outer(), nullptr);
      EXPECT_TRUE(Block->values().empty());
      EXPECT_TRUE(ForeignBlock->values().empty());
    }
    EXPECT_EQ(Destructions, 1U);
  }

  // Destroying a root releases its entry, nested modules, functions, function blocks and their owned leaves.
  TEST(IROwnershipTest, RootDestructionReleasesEntireOwnedTree)
  {
    core::CompilationContext Compilation;
    unsigned Destructions = 0;
    {
      IRContext Context(Compilation);
      IRBuilder Factory(Context);
      Module *Root = Factory.createModule(Context.namePool().intern("Root"));
      Module *Nested = Factory.createModule(Context.namePool().intern("Nested"));
      auto NestedOwner = Factory.removeModule(*Nested);
      ASSERT_TRUE(Factory.appendValue(Root->entryBlock(), std::move(NestedOwner)));
      auto FunctionOwner = Factory.createFunction(Context.namePool().intern("Function"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>()));
      Function *FunctionValue = FunctionOwner.get();
      BasicBlock *First = Factory.createBasicBlock(*FunctionValue);
      BasicBlock *Second = Factory.createBasicBlock(*FunctionValue);
      ASSERT_TRUE(Factory.appendValue(*First, std::make_unique<ObservedValue>(Context, Destructions)));
      ASSERT_TRUE(Factory.appendValue(*Second, std::make_unique<ObservedValue>(Context, Destructions)));
      ASSERT_TRUE(Factory.appendValue(Nested->entryBlock(), std::move(FunctionOwner)));
      ASSERT_TRUE(Factory.appendValue(Root->entryBlock(), std::make_unique<ObservedValue>(Context, Destructions)));
      EXPECT_EQ(Destructions, 0U);
      ASSERT_EQ(Context.modules().size(), 1U);
      EXPECT_EQ(Context.modules()[0].get(), Root);
    }
    EXPECT_EQ(Destructions, 3U);
  }

  // A removed subtree outlives its old parent, keeps interior parents, and is destroyed by its new parent.
  TEST(IROwnershipTest, DetachedSubtreeSurvivesParentAndCanBeReparented)
  {
    core::CompilationContext Compilation;
    unsigned Destructions = 0;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    auto First = Factory.createBasicBlock();
    auto Second = Factory.createBasicBlock();
    auto ChildOwner = Factory.createBasicBlock();
    BasicBlock *Child = ChildOwner.get();
    auto LeafOwner = std::make_unique<ObservedValue>(Context, Destructions);
    Value *Leaf = LeafOwner.get();
    ASSERT_TRUE(Factory.appendValue(*Child, std::move(LeafOwner)));
    ASSERT_TRUE(Factory.appendValue(*First, std::move(ChildOwner)));
    EXPECT_EQ(ChildOwner, nullptr);
    auto Removed = Factory.removeValue(*First, *Child);
    ASSERT_EQ(Removed.get(), Child);
    EXPECT_EQ(Child->outer(), nullptr);
    EXPECT_EQ(Leaf->outer(), Child);
    First.reset();
    EXPECT_EQ(Destructions, 0U);
    ASSERT_TRUE(Factory.appendValue(*Second, std::move(Removed)));
    EXPECT_EQ(Removed, nullptr);
    EXPECT_EQ(Child->outer(), Second.get());
    Second.reset();
    EXPECT_EQ(Destructions, 1U);
  }

  // Explicit erase destroys only the selected child, while retained detached owners are destroyed independently.
  TEST(IROwnershipTest, EraseAndDetachHaveDistinctLifetimes)
  {
    core::CompilationContext Compilation;
    unsigned Destructions = 0;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    auto Block = Factory.createBasicBlock();
    auto FirstOwner = std::make_unique<ObservedValue>(Context, Destructions);
    auto SecondOwner = std::make_unique<ObservedValue>(Context, Destructions);
    Value *First = FirstOwner.get();
    Value *Second = SecondOwner.get();
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(FirstOwner)));
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(SecondOwner)));
    auto Detached = Factory.removeValue(*Block, *First);
    ASSERT_EQ(Detached.get(), First);
    EXPECT_FALSE(Factory.eraseValue(*Block, *First));
    EXPECT_EQ(Destructions, 0U);
    EXPECT_TRUE(Factory.eraseValue(*Block, *Second));
    EXPECT_EQ(Destructions, 1U);
    EXPECT_TRUE(Block->values().empty());
    Detached.reset();
    EXPECT_EQ(Destructions, 2U);
  }

  // Module ownership can move from roots to nested blocks and back; foreign contexts cannot consume the owner.
  TEST(IROwnershipTest, ModulesMoveBetweenRootAndNestedOwnership)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Foreign(Compilation);
    IRBuilder ForeignFactory(Foreign);
    Module *Root = Factory.createModule(Context.namePool().intern("Root"));
    Module *Child = Factory.createModule(Context.namePool().intern("Child"));
    auto ChildOwner = Factory.removeModule(*Child);
    ASSERT_EQ(ChildOwner.get(), Child);
    EXPECT_EQ(Context.modules().size(), 1U);
    EXPECT_EQ(ForeignFactory.removeModule(*Root), nullptr);
    EXPECT_FALSE(ForeignFactory.appendModule(std::move(ChildOwner)));
    EXPECT_EQ(ChildOwner.get(), Child);
    ASSERT_TRUE(Factory.appendValue(Root->entryBlock(), std::move(ChildOwner)));
    EXPECT_EQ(Factory.removeModule(*Child), nullptr);
    EXPECT_FALSE(Factory.eraseModule(*Child));
    auto Detached = Factory.removeValue(Root->entryBlock(), *Child);
    ASSERT_EQ(Detached.get(), Child);
    ASSERT_TRUE(Factory.appendModule(std::move(Detached)));
    EXPECT_EQ(Detached, nullptr);
    EXPECT_EQ(Child->outer(), nullptr);
    ASSERT_EQ(Context.modules().size(), 2U);
    EXPECT_EQ(Context.modules()[1].get(), Child);
    EXPECT_EQ(Child->entryBlock().outer(), Child);
    auto WrongKind = Factory.createBasicBlock();
    EXPECT_FALSE(Factory.appendModule(std::move(WrongKind)));
    EXPECT_NE(WrongKind, nullptr);
    EXPECT_TRUE(Factory.eraseModule(*Child));
    EXPECT_EQ(Context.modules().size(), 1U);
  }

  // Deleting a module subtree frees its nodes immediately without invalidating context-shared types or constants.
  TEST(IROwnershipTest, ModuleDeletionKeepsSharedPoolsAlive)
  {
    core::CompilationContext Compilation;
    unsigned Destructions = 0;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const BuiltinType *BoolType = &Context.typePool().getType<TypeKind::Bool>();
    const BoolConstant *TrueValue = &Context.constantPool().getBoolConstant(true);
    Module *Root = Factory.createModule(Context.namePool().intern("Root"));
    ASSERT_TRUE(Factory.appendValue(Root->entryBlock(), std::make_unique<ObservedValue>(Context, Destructions)));
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(Root->entryBlock()));
    ASSERT_NE(Builder.createAllocaInstruction(*BoolType), nullptr);
    Builder.clearInsertPoint();
    EXPECT_TRUE(Factory.eraseModule(*Root));
    EXPECT_EQ(Destructions, 1U);
    EXPECT_TRUE(Context.modules().empty());
    EXPECT_EQ(&Context.typePool().getType<TypeKind::Bool>(), BoolType);
    EXPECT_EQ(&Context.constantPool().getBoolConstant(true), TrueValue);
    EXPECT_EQ(Builder.insertBlock(), nullptr);
  }
} // namespace ink::ir::test
