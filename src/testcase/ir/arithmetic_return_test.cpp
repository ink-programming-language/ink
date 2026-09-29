#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // Deep addition chains retain their canonical result type without recursively walking operands.
  TEST(IRInstructionTest, DeepAdditionChainsKeepCanonicalResultTypes)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Int32, nullptr);
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 1));
    ASSERT_NE(One, nullptr);
    auto MainOwner = Factory.createFunction(Context.namePool().intern("Main"), *Context.typePool().getType<TypeKind::Function>(*Int32));
    Function *Main = MainOwner.get();
    BasicBlock *Entry = Factory.createFunctionBody(*Main);
    ASSERT_NE(Entry, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    const Value *Result = One;
    for (unsigned Index = 0; Index < 32768; ++Index)
    {
      Result = Builder.createAddInstruction(*Result, *One);
      ASSERT_NE(Result, nullptr);
    }
    EXPECT_EQ(&Result->type(), Int32);
    EXPECT_EQ(&Result->context(), &Context);
    AllocaInstruction *Slot = Builder.createAllocaInstruction(*Int32);
    ASSERT_NE(Slot, nullptr);
    EXPECT_NE(Builder.createStoreInstruction(*Slot, *Result), nullptr);
    EXPECT_NE(Builder.createReturnInstruction(Result), nullptr);
    EXPECT_EQ(Entry->values().size(), 32771U);
    EXPECT_EQ(Result->outer(), Entry);
  }

  // Add accepts only same-context, exactly matching integer types and preserves operand identity.
  TEST(IRInstructionTest, ChecksIntegerAdditionOperands)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Foreign(Compilation);
    const auto *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const auto *Two = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 2));
    const auto *Four = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 4));
    const auto *Unsigned = Context.constantPool().getIntegerConstant(*Context.typePool().getType<TypeKind::Integer>(32, false), IntegerBits(32, 4));
    const auto *Other = Foreign.constantPool().getIntegerConstant(*Foreign.typePool().getType<TypeKind::Integer>(32, true), IntegerBits(32, 2));
    auto AddOwner = Factory.createDetachedAddInstruction(*Two, *Four);
    const auto *Add = AddOwner.get();
    ASSERT_NE(Add, nullptr);
    EXPECT_EQ(&Add->type(), Int32);
    EXPECT_EQ(&Add->left(), Two);
    EXPECT_EQ(&Add->right(), Four);
    EXPECT_TRUE(AddInstruction::classof(Add));
    EXPECT_FALSE(AddInstruction::classof(Two));
    EXPECT_FALSE(AddInstruction::classof(nullptr));
    EXPECT_EQ(Add->outer(), nullptr);
    EXPECT_NE(Factory.createDetachedAddInstruction(*Two, *Four).get(), Add);
    EXPECT_EQ(Factory.createDetachedAddInstruction(*Two, *Unsigned), nullptr);
    EXPECT_EQ(Factory.createDetachedAddInstruction(*Two, *Other), nullptr);
    EXPECT_EQ(Factory.createDetachedAddInstruction(Context.constantPool().getBoolConstant(true), Context.constantPool().getBoolConstant(false)), nullptr);
  }

  // Return creation checks operands; attachment checks the destination function's signature and parent chain.
  TEST(IRInstructionTest, ChecksReturnOperandsAndAttachedSignature)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Foreign(Compilation);
    IRBuilder ForeignFactory(Foreign);
    auto FunctionValueOwner = Factory.createFunction(Context.namePool().intern("value"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>()));
    auto *FunctionValue = FunctionValueOwner.get();
    auto VoidFunctionOwner = Factory.createFunction(Context.namePool().intern("done"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>()));
    auto *VoidFunction = VoidFunctionOwner.get();
    auto ReturnOwner = Factory.createDetachedReturnInstruction(&Context.constantPool().getBoolConstant(true));
    auto *Return = ReturnOwner.get();
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->function(), nullptr);
    EXPECT_EQ(Return->outer(), nullptr);
    EXPECT_EQ(Return->returnedValue(), &Context.constantPool().getBoolConstant(true));
    EXPECT_EQ(&Return->type(), &Context.typePool().getType<TypeKind::Void>());
    EXPECT_TRUE(ReturnInstruction::classof(Return));
    EXPECT_EQ(Factory.createDetachedReturnInstruction(&Foreign.constantPool().getBoolConstant(true)), nullptr);
    EXPECT_EQ(ForeignFactory.createDetachedReturnInstruction(&Context.constantPool().getBoolConstant(true)), nullptr);
    auto VoidReturnOwner = Factory.createDetachedReturnInstruction();
    auto *VoidReturn = VoidReturnOwner.get();
    ASSERT_NE(VoidReturn, nullptr);
    EXPECT_EQ(VoidReturn->returnedValue(), nullptr);
    EXPECT_EQ(Factory.createDetachedReturnInstruction(VoidReturn), nullptr);
    auto *Body = Factory.createFunctionBody(*FunctionValue);
    auto *VoidBody = Factory.createFunctionBody(*VoidFunction);
    ASSERT_NE(Body, nullptr);
    ASSERT_NE(VoidBody, nullptr);
    EXPECT_FALSE(Factory.appendValue(*Body, std::move(VoidReturnOwner)));
    EXPECT_FALSE(Factory.appendValue(*VoidBody, std::move(ReturnOwner)));
    EXPECT_EQ(Return->function(), nullptr);
    EXPECT_EQ(VoidReturn->function(), nullptr);
    EXPECT_TRUE(Body->values().empty());
    EXPECT_TRUE(VoidBody->values().empty());
    auto DetachedBlockOwner = Factory.createBasicBlock();
    auto *DetachedBlock = DetachedBlockOwner.get();
    ASSERT_NE(DetachedBlock, nullptr);
    EXPECT_FALSE(Factory.appendValue(*DetachedBlock, std::move(ReturnOwner)));
    auto *ModuleValue = Factory.createModule(Context.namePool().intern("module"));
    ASSERT_NE(ModuleValue, nullptr);
    EXPECT_FALSE(Factory.appendValue(ModuleValue->entryBlock(), std::move(VoidReturnOwner)));
    ASSERT_TRUE(Factory.appendValue(*Body, std::move(ReturnOwner)));
    ASSERT_TRUE(Factory.appendValue(*VoidBody, std::move(VoidReturnOwner)));
    EXPECT_EQ(Return->function(), FunctionValue);
    EXPECT_EQ(VoidReturn->function(), VoidFunction);
    EXPECT_EQ(Return->outer(), Body);
    EXPECT_EQ(Return->outer()->outer(), Return->function());
  }

  // Removing and reattaching a return derives its new function from the block and rechecks type compatibility.
  TEST(IRInstructionTest, ReturnFunctionFollowsRemovalAndReattachment)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>());
    auto FirstOwner = Factory.createFunction(Context.namePool().intern("first"), *Signature);
    auto *First = FirstOwner.get();
    auto SecondOwner = Factory.createFunction(Context.namePool().intern("second"), *Signature);
    auto *Second = SecondOwner.get();
    auto WrongOwner = Factory.createFunction(Context.namePool().intern("wrong"), *Context.typePool().getType<TypeKind::Function>(*Context.typePool().getType<TypeKind::Integer>(32, true)));
    auto *Wrong = WrongOwner.get();
    auto *FirstBlock = Factory.createFunctionBody(*First);
    auto *SecondBlock = Factory.createFunctionBody(*Second);
    auto *WrongBlock = Factory.createFunctionBody(*Wrong);
    auto ReturnOwner = Factory.createDetachedReturnInstruction(&Context.constantPool().getBoolConstant(true));
    auto *Return = ReturnOwner.get();
    ASSERT_NE(Return, nullptr);
    ASSERT_TRUE(Factory.appendValue(*FirstBlock, std::move(ReturnOwner)));
    EXPECT_EQ(Return->function(), First);
    EXPECT_FALSE(Factory.removeValue(*SecondBlock, *Return));
    EXPECT_FALSE(Factory.appendValue(*SecondBlock, std::move(ReturnOwner)));
    EXPECT_EQ(Return->function(), First);
    auto RemovedReturnOwner = Factory.removeValue(*FirstBlock, *Return);
    ASSERT_EQ(RemovedReturnOwner.get(), Return);
    EXPECT_EQ(Return->function(), nullptr);
    EXPECT_FALSE(Factory.appendValue(*WrongBlock, std::move(RemovedReturnOwner)));
    EXPECT_EQ(Return->function(), nullptr);
    EXPECT_TRUE(WrongBlock->values().empty());
    ASSERT_TRUE(Factory.appendValue(*SecondBlock, std::move(RemovedReturnOwner)));
    EXPECT_EQ(Return->function(), Second);
    EXPECT_EQ(Return->outer()->outer(), Second);
  }

  // Incoming parameters retain their function, index and type while context storage grows.
  TEST(IRFunctionTest, OwnsIndexedParameterValues)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const Type *ParameterTypes[] = {Context.typePool().getType<TypeKind::Integer>(32, true), &Context.typePool().getType<TypeKind::Bool>()};
    auto FunctionValueOwner = Factory.createFunction(Context.namePool().intern("f"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>(), ParameterTypes));
    auto *FunctionValue = FunctionValueOwner.get();
    ASSERT_NE(FunctionValue, nullptr);
    ASSERT_EQ(FunctionValue->parameters().size(), 2U);
    const auto *First = FunctionValue->parameters()[0].get();
    EXPECT_EQ(First->index(), 0U);
    EXPECT_EQ(&First->type(), ParameterTypes[0]);
    EXPECT_EQ(First->outer(), FunctionValue);
    EXPECT_EQ(&First->function(), FunctionValue);
    EXPECT_EQ(First->parameterKind(), ParameterKind::Positional);
    EXPECT_TRUE(FunctionParameter::classof(First));
    auto *Body = Factory.createFunctionBody(*FunctionValue);
    ASSERT_NE(Body, nullptr);
    auto Container = Factory.createBasicBlock();
    ASSERT_TRUE(Factory.appendValue(*Container, std::move(FunctionValueOwner)));
    for (std::size_t Index = 0; Index < 256; ++Index)
    {
      ASSERT_TRUE(Factory.appendValue(*Container, Factory.createFunction(Context.namePool().intern("other"), FunctionValue->functionType())));
    }
    EXPECT_EQ(FunctionValue->parameters()[0].get(), First);
    EXPECT_EQ(&First->type(), ParameterTypes[0]);
  }
} // namespace ink::ir::test
