#include "ink/core/name/name.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/value/value.h"

#include <gtest/gtest.h>

#include <ranges>
#include <string>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Value, Function>);
  static_assert(!std::is_base_of_v<Value, BasicBlock>);
  static_assert(!std::is_copy_constructible_v<Function>);
  static_assert(!std::is_copy_assignable_v<Function>);
  static_assert(!std::is_move_constructible_v<Function>);
  static_assert(!std::is_move_assignable_v<Function>);
  static_assert(!std::is_copy_constructible_v<BasicBlock>);
  static_assert(!std::is_copy_assignable_v<BasicBlock>);
  static_assert(!std::is_move_constructible_v<BasicBlock>);
  static_assert(!std::is_move_assignable_v<BasicBlock>);
  static_assert(!std::is_constructible_v<Function, FunctionType &&>);
  static_assert(!std::is_constructible_v<Function, const FunctionType &&>);
  static_assert(!std::is_constructible_v<Function, FunctionType &&, core::Name>);
  static_assert(!std::is_constructible_v<Function, const FunctionType &&, core::Name>);
  static_assert(!std::is_constructible_v<Function, const FunctionType &, std::string>);
  static_assert(!std::is_constructible_v<Function, const FunctionType &, const char *>);
  static_assert(!std::is_constructible_v<BasicBlock, std::string>);
  static_assert(!std::is_constructible_v<BasicBlock, const char *>);
  static_assert(std::is_same_v<decltype(std::declval<const Function &>().name()), core::Name>);
  static_assert(std::is_same_v<decltype(std::declval<const BasicBlock &>().name()), core::Name>);
  static_assert(std::is_same_v<decltype(std::declval<BasicBlock &>().parent()), Function *>);
  static_assert(std::is_same_v<decltype(std::declval<const BasicBlock &>().parent()), const Function *>);
  static_assert(std::is_same_v<decltype(std::declval<BasicBlock &&>().parent()), Function *>);
  static_assert(std::is_same_v<decltype(std::declval<const BasicBlock &&>().parent()), const Function *>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<Function &>().basicBlocks())>, BasicBlock &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const Function &>().basicBlocks())>, const BasicBlock &>);

  template <typename NameType>
  concept FunctionAcceptsBlockName = requires(Function &Value, NameType Name)
  {
    Value.createBasicBlock(Name);
  };

  static_assert(FunctionAcceptsBlockName<core::Name>);
  static_assert(!FunctionAcceptsBlockName<std::string>);
  static_assert(!FunctionAcceptsBlockName<const char *>);

  // A function keeps the canonical signature identity through both its own API and the Value base.
  TEST(IRFunctionTest, BorrowsCanonicalSignatureThroughValue)
  {
    TypePool Types;
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType());
    Function FunctionValue(Signature);
    const Value *Base = &FunctionValue;
    EXPECT_EQ(&FunctionValue.signature(), &Signature);
    EXPECT_EQ(&Base->getType(), &Signature);
    EXPECT_TRUE(FunctionType::classof(&Base->getType()));
    EXPECT_EQ(FunctionValue.name(), core::NAME_None);
    EXPECT_EQ(FunctionValue.basicBlockCount(), 0u);
    EXPECT_TRUE(std::ranges::empty(FunctionValue.basicBlocks()));
    BasicBlock &Unnamed = FunctionValue.createBasicBlock();
    EXPECT_EQ(Unnamed.name(), core::NAME_None);
    EXPECT_EQ(Unnamed.parent(), &FunctionValue);
  }

  // Standalone blocks start detached, with either NAME_None or an explicit interned name.
  TEST(IRFunctionTest, CreatesDetachedBasicBlocksWithoutAParent)
  {
    BasicBlock Empty;
    const core::Name DetachedName = core::NamePool::instance().intern("detached");
    const BasicBlock Named(DetachedName);
    EXPECT_EQ(Empty.name(), core::NAME_None);
    EXPECT_EQ(Empty.parent(), nullptr);
    EXPECT_EQ(Named.name(), DetachedName);
    EXPECT_EQ(Named.parent(), nullptr);
  }

  // Function and block name handles stay unchanged after their original interned input strings change.
  TEST(IRFunctionTest, StoresInternedFunctionAndBasicBlockNames)
  {
    TypePool Types;
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType());
    core::NamePool &Names = core::NamePool::instance();
    std::string FunctionSpelling = "stored_function";
    std::string BlockSpelling = "stored_block";
    const core::Name FunctionName = Names.intern(FunctionSpelling);
    const core::Name BlockName = Names.intern(BlockSpelling);
    Function FunctionValue(Signature, FunctionName);
    BasicBlock &Block = FunctionValue.createBasicBlock(BlockName);
    FunctionSpelling.assign("changed_function");
    BlockSpelling.assign("changed_block");
    EXPECT_EQ(FunctionValue.name(), FunctionName);
    EXPECT_EQ(Block.name(), BlockName);
    EXPECT_EQ(Block.parent(), &FunctionValue);
  }

  // Created blocks retain declaration order and expose their parent with the constness of the block.
  TEST(IRFunctionTest, OwnsOrderedBasicBlocksWithCorrectParentLinks)
  {
    TypePool Types;
    core::NamePool &Names = core::NamePool::instance();
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType());
    const core::Name EntryName = Names.intern("entry");
    const core::Name ExitName = Names.intern("exit");
    Function FunctionValue(Signature, Names.intern("run"));
    BasicBlock &Entry = FunctionValue.createBasicBlock(EntryName);
    BasicBlock &Exit = FunctionValue.createBasicBlock(ExitName);
    ASSERT_EQ(FunctionValue.basicBlockCount(), 2u);
    auto Iterator = FunctionValue.basicBlocks().begin();
    EXPECT_EQ(&*Iterator, &Entry);
    EXPECT_EQ(Iterator->name(), EntryName);
    EXPECT_EQ(Iterator->parent(), &FunctionValue);
    ++Iterator;
    EXPECT_EQ(&*Iterator, &Exit);
    EXPECT_EQ(Iterator->name(), ExitName);
    EXPECT_EQ(Iterator->parent(), &FunctionValue);
    ++Iterator;
    EXPECT_EQ(Iterator, FunctionValue.basicBlocks().end());
    const Function &ConstFunction = FunctionValue;
    const BasicBlock &ConstEntry = *ConstFunction.basicBlocks().begin();
    EXPECT_EQ(&ConstEntry, &Entry);
    EXPECT_EQ(ConstEntry.parent(), &ConstFunction);
  }

  // Adding more blocks keeps previously returned block references and iterators valid.
  TEST(IRFunctionTest, PreservesBasicBlockIdentityWhileTheFunctionGrows)
  {
    TypePool Types;
    core::NamePool &Names = core::NamePool::instance();
    const FunctionType &Signature = Types.getFunctionType(Types.getVoidType());
    Function FunctionValue(Signature);
    const core::Name EntryName = Names.intern("entry");
    BasicBlock &Entry = FunctionValue.createBasicBlock(EntryName);
    auto EntryIterator = FunctionValue.basicBlocks().begin();
    for (unsigned Index = 0; Index < 128; ++Index)
    {
      FunctionValue.createBasicBlock(Names.intern("block_" + std::to_string(Index)));
    }
    EXPECT_EQ(FunctionValue.basicBlockCount(), 129u);
    EXPECT_EQ(&*EntryIterator, &Entry);
    EXPECT_EQ(&*FunctionValue.basicBlocks().begin(), &Entry);
    EXPECT_EQ(Entry.name(), EntryName);
    EXPECT_EQ(Entry.parent(), &FunctionValue);
  }
} // namespace ink::ir::test
