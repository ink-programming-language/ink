#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // C string instructions preserve NUL-terminated source bytes and create distinct writable pointer values.
  TEST(IRCStringInstructionTest, CreatesFunctionLocalCopies)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *U8 = Context.typePool().getType<TypeKind::Integer>(8, false);
    const auto *Slice = Context.typePool().getType<TypeKind::Slice>(*U8, AccessKind::ReadOnly);
    const auto *Source = Context.constantPool().getStringConstant(*Slice, "hello");
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    auto FunctionValue = Builder.createFunction(Context.namePool().intern("f"), *Signature);
    BasicBlock *Body = Builder.createFunctionBody(*FunctionValue);
    ASSERT_TRUE(Builder.setInsertPoint(*Body));
    CStringInstruction *First = Builder.createCStringInstruction(*Source);
    CStringInstruction *Second = Builder.createCStringInstruction(*Source);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_EQ(&First->source(), Source);
    EXPECT_EQ(First->source().nullTerminatedValue().size(), 6U);
    EXPECT_EQ(First->outer(), Body);
    EXPECT_EQ(First->kind(), ValueKind::CStringInstruction);
    EXPECT_EQ(&First->type(), Context.typePool().getType<TypeKind::Pointer>(*U8, AccessKind::ReadWrite));
    EXPECT_FALSE(Constant::classof(First));
    auto Detached = Builder.createDetachedCStringInstruction(*Source);
    EXPECT_EQ(Detached->outer(), nullptr);
    ReturnInstruction *End = Builder.createReturnInstruction();
    ASSERT_NE(End, nullptr);
    EXPECT_EQ(Builder.createCStringInstruction(*Source), nullptr);
    EXPECT_FALSE(Builder.appendValue(*Body, std::move(Detached)));
    ASSERT_NE(Detached, nullptr);
    EXPECT_TRUE(Builder.insertValue(*Body, std::move(Detached), End));
    EXPECT_EQ(Body->values().size(), 4U);
  }

  // Foreign constants, embedded NUL and module-owned storage are rejected without consuming detached ownership.
  TEST(IRCStringInstructionTest, RejectsInvalidSourcesAndOwners)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRContext Other(Compilation);
    IRBuilder Builder(Context);
    const auto *Slice = Context.typePool().getType<TypeKind::Slice>(*Context.typePool().getType<TypeKind::Integer>(8, false), AccessKind::ReadOnly);
    const auto *ForeignSlice = Other.typePool().getType<TypeKind::Slice>(*Other.typePool().getType<TypeKind::Integer>(8, false), AccessKind::ReadOnly);
    const auto *Empty = Context.constantPool().getStringConstant(*Slice, "");
    const auto *Embedded = Context.constantPool().getStringConstant(*Slice, std::string_view("a\0b", 3));
    const auto *Foreign = Other.constantPool().getStringConstant(*ForeignSlice, "hello");
    EXPECT_EQ(Builder.createDetachedCStringInstruction(*Embedded), nullptr);
    EXPECT_EQ(Builder.createDetachedCStringInstruction(*Foreign), nullptr);
    auto Detached = Builder.createDetachedCStringInstruction(*Empty);
    ASSERT_NE(Detached, nullptr);
    Module *ModuleValue = Builder.createModule(Context.namePool().intern("module"));
    EXPECT_FALSE(Builder.appendValue(ModuleValue->entryBlock(), std::move(Detached)));
    EXPECT_NE(Detached, nullptr);
    auto Block = Builder.createBasicBlock();
    EXPECT_FALSE(Builder.appendValue(*Block, std::move(Detached)));
    EXPECT_NE(Detached, nullptr);
    EXPECT_TRUE(ModuleValue->entryBlock().values().empty());
  }
} // namespace ink::ir::test
