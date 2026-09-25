#include "ink/semantic/model/context.h"

#include <gtest/gtest.h>

namespace ink::semantic::test
{
  // Add accepts only same-context, exactly matching integer types and preserves operand identity.
  TEST(SemanticInstructionTest, ChecksIntegerAdditionOperands)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Foreign(Compilation);
    const auto *Int32 = Context.getIntegerType(32, true);
    const auto *Two = Context.getIntegerConstant(*Int32, IntegerBits(32, 2));
    const auto *Four = Context.getIntegerConstant(*Int32, IntegerBits(32, 4));
    const auto *Unsigned = Context.getIntegerConstant(*Context.getIntegerType(32, false), IntegerBits(32, 4));
    const auto *Other = Foreign.getIntegerConstant(*Foreign.getIntegerType(32, true), IntegerBits(32, 2));
    const auto *Add = Context.createAddInstruction(*Two, *Four);
    ASSERT_NE(Add, nullptr);
    EXPECT_EQ(&Add->type(), Int32);
    EXPECT_EQ(&Add->left(), Two);
    EXPECT_EQ(&Add->right(), Four);
    EXPECT_TRUE(AddInstruction::classof(Add));
    EXPECT_FALSE(AddInstruction::classof(Two));
    EXPECT_FALSE(AddInstruction::classof(nullptr));
    EXPECT_EQ(Add->outer(), nullptr);
    EXPECT_NE(Context.createAddInstruction(*Two, *Four), Add);
    EXPECT_EQ(Context.createAddInstruction(*Two, *Unsigned), nullptr);
    EXPECT_EQ(Context.createAddInstruction(*Two, *Other), nullptr);
    EXPECT_EQ(Context.createAddInstruction(Context.getBoolConstant(true), Context.getBoolConstant(false)), nullptr);
  }

  // Return creation checks operands; attachment checks the destination function's signature and parent chain.
  TEST(SemanticInstructionTest, ChecksReturnOperandsAndAttachedSignature)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Foreign(Compilation);
    auto *FunctionValue = Context.createFunction(Context.namePool().intern("value"), *Context.getFunctionType(Context.getBoolType()));
    auto *VoidFunction = Context.createFunction(Context.namePool().intern("done"), *Context.getFunctionType(Context.getVoidType()));
    auto *Return = Context.createReturnInstruction(&Context.getBoolConstant(true));
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->function(), nullptr);
    EXPECT_EQ(Return->outer(), nullptr);
    EXPECT_EQ(Return->returnedValue(), &Context.getBoolConstant(true));
    EXPECT_EQ(&Return->type(), &Context.getVoidType());
    EXPECT_TRUE(ReturnInstruction::classof(Return));
    EXPECT_EQ(Context.createReturnInstruction(&Foreign.getBoolConstant(true)), nullptr);
    EXPECT_EQ(Foreign.createReturnInstruction(&Context.getBoolConstant(true)), nullptr);
    auto *VoidReturn = Context.createReturnInstruction();
    ASSERT_NE(VoidReturn, nullptr);
    EXPECT_EQ(VoidReturn->returnedValue(), nullptr);
    EXPECT_EQ(Context.createReturnInstruction(VoidReturn), nullptr);
    auto *Body = Context.createFunctionBody(*FunctionValue);
    auto *VoidBody = Context.createFunctionBody(*VoidFunction);
    ASSERT_NE(Body, nullptr);
    ASSERT_NE(VoidBody, nullptr);
    EXPECT_FALSE(Context.appendValue(*Body, *VoidReturn));
    EXPECT_FALSE(Context.appendValue(*VoidBody, *Return));
    EXPECT_EQ(Return->function(), nullptr);
    EXPECT_EQ(VoidReturn->function(), nullptr);
    EXPECT_TRUE(Body->values().empty());
    EXPECT_TRUE(VoidBody->values().empty());
    auto *DetachedBlock = Context.createBasicBlock();
    ASSERT_NE(DetachedBlock, nullptr);
    EXPECT_FALSE(Context.appendValue(*DetachedBlock, *Return));
    auto *ModuleValue = Context.createModule(Context.namePool().intern("module"));
    ASSERT_NE(ModuleValue, nullptr);
    EXPECT_FALSE(Context.appendValue(ModuleValue->entryBlock(), *VoidReturn));
    ASSERT_TRUE(Context.appendValue(*Body, *Return));
    ASSERT_TRUE(Context.appendValue(*VoidBody, *VoidReturn));
    EXPECT_EQ(Return->function(), FunctionValue);
    EXPECT_EQ(VoidReturn->function(), VoidFunction);
    EXPECT_EQ(Return->outer(), Body);
    EXPECT_EQ(Return->outer()->outer(), Return->function());
  }

  // Removing and reattaching a return derives its new function from the block and rechecks type compatibility.
  TEST(SemanticInstructionTest, ReturnFunctionFollowsRemovalAndReattachment)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const auto *Signature = Context.getFunctionType(Context.getBoolType());
    auto *First = Context.createFunction(Context.namePool().intern("first"), *Signature);
    auto *Second = Context.createFunction(Context.namePool().intern("second"), *Signature);
    auto *Wrong = Context.createFunction(Context.namePool().intern("wrong"), *Context.getFunctionType(*Context.getIntegerType(32, true)));
    auto *FirstBlock = Context.createFunctionBody(*First);
    auto *SecondBlock = Context.createFunctionBody(*Second);
    auto *WrongBlock = Context.createFunctionBody(*Wrong);
    auto *Return = Context.createReturnInstruction(&Context.getBoolConstant(true));
    ASSERT_NE(Return, nullptr);
    ASSERT_TRUE(Context.appendValue(*FirstBlock, *Return));
    EXPECT_EQ(Return->function(), First);
    EXPECT_FALSE(Context.removeValue(*SecondBlock, *Return));
    EXPECT_FALSE(Context.appendValue(*SecondBlock, *Return));
    EXPECT_EQ(Return->function(), First);
    ASSERT_TRUE(Context.removeValue(*FirstBlock, *Return));
    EXPECT_EQ(Return->function(), nullptr);
    EXPECT_FALSE(Context.appendValue(*WrongBlock, *Return));
    EXPECT_EQ(Return->function(), nullptr);
    EXPECT_TRUE(WrongBlock->values().empty());
    ASSERT_TRUE(Context.appendValue(*SecondBlock, *Return));
    EXPECT_EQ(Return->function(), Second);
    EXPECT_EQ(Return->outer()->outer(), Second);
  }

  // Incoming parameters retain their function, index and type while context storage grows.
  TEST(SemanticFunctionTest, OwnsIndexedParameterValues)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type *ParameterTypes[] = {Context.getIntegerType(32, true), &Context.getBoolType()};
    auto *FunctionValue = Context.createFunction(Context.namePool().intern("f"), *Context.getFunctionType(Context.getVoidType(), ParameterTypes));
    ASSERT_NE(FunctionValue, nullptr);
    ASSERT_EQ(FunctionValue->parameters().size(), 2U);
    const auto *First = FunctionValue->parameters()[0];
    EXPECT_EQ(First->index(), 0U);
    EXPECT_EQ(&First->type(), ParameterTypes[0]);
    EXPECT_EQ(First->outer(), FunctionValue);
    EXPECT_EQ(&First->function(), FunctionValue);
    EXPECT_EQ(First->parameterKind(), ParameterKind::Positional);
    EXPECT_TRUE(FunctionParameter::classof(First));
    auto *Body = Context.createFunctionBody(*FunctionValue);
    ASSERT_NE(Body, nullptr);
    for (std::size_t Index = 0; Index < 256; ++Index)
    {
      ASSERT_NE(Context.createFunction(Context.namePool().intern("other"), FunctionValue->type()), nullptr);
    }
    EXPECT_EQ(FunctionValue->parameters()[0], First);
    EXPECT_EQ(&First->type(), ParameterTypes[0]);
  }
} // namespace ink::semantic::test
