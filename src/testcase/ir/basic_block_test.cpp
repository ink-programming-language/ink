#include "ink/core/name/name_pool.h"
#include "ink/ir/context.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/type/type.h"

#include <gtest/gtest.h>

#include <memory>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    template <typename ValueType, typename = void>
    struct HasSize : std::false_type
    {
    };

    template <typename ValueType>
    struct HasSize<ValueType, std::void_t<decltype(std::declval<const ValueType &>().size())>> : std::true_type
    {
    };

    template <typename ValueType, typename = void>
    struct HasAlignment : std::false_type
    {
    };

    template <typename ValueType>
    struct HasAlignment<ValueType, std::void_t<decltype(std::declval<const ValueType &>().alignment())>> : std::true_type
    {
    };
  } // namespace

  static_assert(std::is_abstract_v<Value>);
  static_assert(std::is_abstract_v<Type>);
  static_assert(std::is_base_of_v<Value, BasicBlock>);
  static_assert(!std::is_base_of_v<Type, BasicBlock>);
  static_assert(std::has_virtual_destructor_v<BasicBlock>);
  static_assert(!std::is_copy_constructible_v<BasicBlock>);
  static_assert(!std::is_copy_assignable_v<BasicBlock>);
  static_assert(!std::is_move_constructible_v<BasicBlock>);
  static_assert(!std::is_move_assignable_v<BasicBlock>);
  static_assert(!HasSize<Value>::value);
  static_assert(!HasAlignment<Value>::value);
  static_assert(!HasSize<BasicBlock>::value);
  static_assert(!HasAlignment<BasicBlock>::value);
  static_assert(HasSize<Type>::value);
  static_assert(HasAlignment<Type>::value);

  // A default basic block is an unnamed Value and does not classify as a Type.
  TEST(IRBasicBlockTest, DefaultsToUnnamedNonTypeValue)
  {
    const BasicBlock Block;
    const Value *Value = &Block;
    EXPECT_EQ(Value->kind(), ValueKind::BasicBlock);
    EXPECT_EQ(Block.name(), core::NAME_None);
    EXPECT_FALSE(Block.hasName());
    EXPECT_FALSE(Block.hasTerminator());
    EXPECT_TRUE(BasicBlock::classof(Value));
    EXPECT_FALSE(Type::classof(Value));
    EXPECT_FALSE(BasicBlock::classof(nullptr));
  }

  // A named block preserves its interned name when owned and accessed through the Value interface.
  TEST(IRBasicBlockTest, OwnedValuePreservesNamedBlockIdentity)
  {
    const core::Name Name = core::NamePool::instance().intern("IRBasicBlockTest.Entry");
    ASSERT_TRUE(Name.valid());
    std::unique_ptr<Value> Block = std::make_unique<BasicBlock>(Name);
    EXPECT_EQ(Block->kind(), ValueKind::BasicBlock);
    ASSERT_TRUE(BasicBlock::classof(Block.get()));
    const auto *NamedBlock = static_cast<const BasicBlock *>(Block.get());
    EXPECT_EQ(NamedBlock->name(), Name);
    EXPECT_TRUE(NamedBlock->hasName());
    EXPECT_FALSE(Type::classof(Block.get()));
  }

  // Naming, renaming and clearing a block update its optional name without changing its Value kind.
  TEST(IRBasicBlockTest, RenamesAndClearsOptionalName)
  {
    const core::Name EntryName = core::NamePool::instance().intern("IRBasicBlockTest.Initial");
    const core::Name ExitName = core::NamePool::instance().intern("IRBasicBlockTest.Renamed");
    ASSERT_TRUE(EntryName.valid());
    ASSERT_TRUE(ExitName.valid());
    ASSERT_NE(EntryName, ExitName);
    BasicBlock Block;
    Block.setName(EntryName);
    EXPECT_EQ(Block.name(), EntryName);
    EXPECT_TRUE(Block.hasName());
    Block.setName(ExitName);
    EXPECT_EQ(Block.name(), ExitName);
    EXPECT_TRUE(Block.hasName());
    Block.setName(core::NAME_None);
    EXPECT_EQ(Block.name(), core::NAME_None);
    EXPECT_FALSE(Block.hasName());
    EXPECT_EQ(Block.kind(), ValueKind::BasicBlock);
  }

  // Rejecting a null instruction preserves both empty and terminated block state.
  TEST(IRBasicBlockTest, NullAppendPreservesTerminatorStateAndOwnership)
  {
    IRContext Context(Target);
    BasicBlock Block;
    EXPECT_EQ(Block.appendInstruction(nullptr), nullptr);
    EXPECT_EQ(Block.instructionCount(), 0U);
    EXPECT_FALSE(Block.hasTerminator());

    auto *Terminator = Block.appendInstruction(std::make_unique<ReturnInstruction>(*Context.typePool().get<ink::ir::VoidType>()));
    ASSERT_NE(Terminator, nullptr);
    EXPECT_TRUE(Block.hasTerminator());
    EXPECT_EQ(Block.appendInstruction(nullptr), nullptr);
    EXPECT_EQ(Block.instructionCount(), 1U);
    EXPECT_EQ(Block.instruction(0), Terminator);
    EXPECT_EQ(Terminator->parentBlock(), &Block);
    EXPECT_TRUE(Block.hasTerminator());
  }
} // namespace ink::ir::test
