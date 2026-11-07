#include "ink/ir/ir_context.h"
#include "ink/ir/module/module.h"
#include "ink/ir/text/module_text.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(!std::is_copy_constructible_v<IRContext> && !std::is_copy_assignable_v<IRContext>);
  static_assert(!std::is_move_constructible_v<IRContext> && !std::is_move_assignable_v<IRContext>);
  static_assert(std::is_same_v<decltype(std::declval<IRContext &>().types()), TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<const IRContext &>().types()), const TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<IRContext &&>().types()), TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<const IRContext &&>().types()), const TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<IRContext &>().constants()), ConstantPool &>);
  static_assert(std::is_same_v<decltype(std::declval<const IRContext &>().constants()), const ConstantPool &>);
  static_assert(std::is_same_v<decltype(std::declval<IRContext &&>().constants()), ConstantPool &>);
  static_assert(std::is_same_v<decltype(std::declval<const IRContext &&>().constants()), const ConstantPool &>);
  static_assert(std::is_same_v<decltype(std::declval<IRContext &>().builder()), InstructionBuilder &>);
  static_assert(std::is_same_v<decltype(std::declval<const IRContext &>().builder()), const InstructionBuilder &>);
  static_assert(std::is_same_v<decltype(std::declval<IRContext &&>().builder()), InstructionBuilder &>);
  static_assert(std::is_same_v<decltype(std::declval<const IRContext &&>().builder()), const InstructionBuilder &>);
  static_assert(std::is_same_v<decltype(std::declval<ParsedModule &>().context()), IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<const ParsedModule &>().context()), const IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<ParsedModule &&>().context()), IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<const ParsedModule &&>().context()), const IRContext &>);
  static_assert(std::is_same_v<decltype(std::declval<ParsedModule &>().types()), TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<const ParsedModule &>().types()), const TypePool &>);
  static_assert(std::is_same_v<decltype(std::declval<ParsedModule &>().constants()), ConstantPool &>);
  static_assert(std::is_same_v<decltype(std::declval<const ParsedModule &>().constants()), const ConstantPool &>);

  // Constants canonicalize external numeric types and string array types into the context's single type pool.
  TEST(IRContextTest, SharesCanonicalTypesBetweenPools)
  {
    IRContext Context;
    const IntegerType ExternalInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType &Int32 = Context.types().getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *Answer = Context.constants().getSignedInteger(ExternalInt32, 42);
    ASSERT_NE(Answer, nullptr);
    EXPECT_EQ(&Answer->getType(), &Int32);
    EXPECT_EQ(Answer, Context.constants().getSignedInteger(Int32, 42));
    EXPECT_EQ(&Context.constants().getFloat(1.5f).getType(), &Context.types().getFloatType(FloatType::Width::Bits32));

    const IntegerType &UInt8 = Context.types().getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const StringConstant &Text = Context.constants().getString("ink");
    EXPECT_EQ(&Text.getType(), &Context.types().getArrayType(UInt8, 3));
    EXPECT_EQ(&Text, &Context.constants().getString("ink"));
  }

  // The managed builder uses context-owned pointer and void types while blocks retain ownership of the created instructions.
  TEST(IRContextTest, BuildsInstructionsWithContextOwnedTypesAndConstants)
  {
    IRContext Context;
    Module ModuleValue;
    const IntegerType &Int32 = Context.types().getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *Answer = Context.constants().getSignedInteger(Int32, 42);
    ASSERT_NE(Answer, nullptr);
    Function &FunctionValue = ModuleValue.createFunction(Context.types().getFunctionType(Int32));
    BasicBlock &Block = FunctionValue.createBasicBlock();
    InstructionBuilder &Builder = Context.builder();
    EXPECT_EQ(Builder.getInsertBlock(), nullptr);
    EXPECT_EQ(Builder.getInsertBefore(), nullptr);
    Builder.setInsertPoint(Block);
    AllocaInstruction &Slot = Builder.createAlloca(Int32);
    StoreInstruction &Store = Builder.createStore(Slot, *Answer);
    LoadInstruction &Loaded = Builder.createLoad(Slot);
    ReturnInstruction &Return = Builder.createReturn(Loaded);
    Builder.clearInsertPoint();

    EXPECT_EQ(&Slot.getType(), &Context.types().getPointerType(Int32));
    EXPECT_EQ(&Slot.getAllocatedType(), &Int32);
    EXPECT_EQ(&Store.getType(), &Context.types().getVoidType());
    EXPECT_EQ(&Store.getSource(), Answer);
    EXPECT_EQ(&Loaded.getType(), &Int32);
    EXPECT_EQ(Return.getReturnValue(), &Loaded);
    EXPECT_EQ(&Return.getType(), &Context.types().getVoidType());
    EXPECT_EQ(Return.parent(), &Block);
    EXPECT_EQ(Block.instructionCount(), 4U);
    EXPECT_TRUE(FunctionValue.hasValidControlFlow());
  }

  // Independent contexts keep their type/constant identities and builder insertion points isolated.
  TEST(IRContextTest, IsolatesResourcesAndInsertionPointsAcrossContexts)
  {
    IRContext First;
    IRContext Second;
    BasicBlock FirstBlock;
    BasicBlock SecondBlock;
    const IntegerType &FirstInt32 = First.types().getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType &SecondInt32 = Second.types().getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *FirstConstant = First.constants().getSignedInteger(FirstInt32, 42);
    const IntegerConstant *SecondConstant = Second.constants().getSignedInteger(SecondInt32, 42);
    ASSERT_NE(FirstConstant, nullptr);
    ASSERT_NE(SecondConstant, nullptr);
    EXPECT_NE(&FirstInt32, &SecondInt32);
    EXPECT_NE(FirstConstant, SecondConstant);
    EXPECT_EQ(&FirstConstant->getType(), &FirstInt32);
    EXPECT_EQ(&SecondConstant->getType(), &SecondInt32);
    First.builder().setInsertPoint(FirstBlock);
    Second.builder().setInsertPoint(SecondBlock);
    ReturnInstruction &FirstReturn = First.builder().createReturn(*FirstConstant);
    First.builder().clearInsertPoint();
    EXPECT_EQ(First.builder().getInsertBlock(), nullptr);
    EXPECT_EQ(Second.builder().getInsertBlock(), &SecondBlock);
    ReturnInstruction &SecondReturn = Second.builder().createReturn(*SecondConstant);
    Second.builder().clearInsertPoint();
    EXPECT_EQ(FirstReturn.parent(), &FirstBlock);
    EXPECT_EQ(SecondReturn.parent(), &SecondBlock);
    EXPECT_EQ(&FirstReturn.getType(), &First.types().getVoidType());
    EXPECT_EQ(&SecondReturn.getType(), &Second.types().getVoidType());
  }

  // Modules share canonical resources without acquiring them, and the context can build again after a temporary module is destroyed.
  TEST(IRContextTest, SharesResourcesAcrossModulesWithIndependentLifetimes)
  {
    IRContext Context;
    Module Retained;
    const IntegerType &Int32 = Context.types().getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FunctionType &Signature = Context.types().getFunctionType(Int32);
    const IntegerConstant *Answer = Context.constants().getSignedInteger(Int32, 42);
    ASSERT_NE(Answer, nullptr);
    Function &RetainedFunction = Retained.createFunction(Signature);
    BasicBlock &RetainedBlock = RetainedFunction.createBasicBlock();
    {
      Module Temporary;
      Function &TemporaryFunction = Temporary.createFunction(Context.types().getFunctionType(Int32));
      EXPECT_EQ(&TemporaryFunction.signature(), &RetainedFunction.signature());
      Context.builder().setInsertPoint(TemporaryFunction.createBasicBlock());
      ReturnInstruction &Return = Context.builder().createReturn(*Answer);
      EXPECT_EQ(Return.getReturnValue(), Answer);
      Context.builder().clearInsertPoint();
    }

    Context.builder().setInsertPoint(RetainedBlock);
    const IntegerConstant *ReusedAnswer = Context.constants().getSignedInteger(Int32, 42);
    ASSERT_EQ(ReusedAnswer, Answer);
    ReturnInstruction &Return = Context.builder().createReturn(*ReusedAnswer);
    Context.builder().clearInsertPoint();
    EXPECT_EQ(&RetainedFunction.signature(), &Signature);
    EXPECT_EQ(Return.getReturnValue(), Answer);
    EXPECT_TRUE(RetainedFunction.hasValidControlFlow());
  }

  // ParsedModule forwards its pools to one context, and deserialization leaves that context's builder ready for a new selection.
  TEST(IRContextTest, ReusesParsedModuleContextAfterDeserialization)
  {
    ParsedModule Original;
    EXPECT_EQ(&Original.types(), &Original.context().types());
    EXPECT_EQ(&Original.constants(), &Original.context().constants());
    const IntegerType &Int32 = Original.types().getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant *Answer = Original.constants().getSignedInteger(Int32, 42);
    ASSERT_NE(Answer, nullptr);
    Function &OriginalFunction = Original.module().createFunction(Original.types().getFunctionType(Int32));
    Original.context().builder().setInsertPoint(OriginalFunction.createBasicBlock());
    static_cast<void>(Original.context().builder().createReturn(*Answer));
    Original.context().builder().clearInsertPoint();
    auto Text = writeModuleText(Original.module());
    ASSERT_TRUE(Text.has_value());
    auto Parsed = readModuleText(*Text);
    ASSERT_TRUE(Parsed.has_value());
    ParsedModule &Copy = **Parsed;
    EXPECT_EQ(&Copy.types(), &Copy.context().types());
    EXPECT_EQ(&Copy.constants(), &Copy.context().constants());
    EXPECT_EQ(Copy.context().builder().getInsertBlock(), nullptr);
    EXPECT_EQ(Copy.context().builder().getInsertBefore(), nullptr);
    ASSERT_EQ(Copy.module().functionCount(), 1U);
    const Function &CopiedFunction = *Copy.module().functions().begin();
    const IntegerType &CopiedInt32 = Copy.types().getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    EXPECT_EQ(&CopiedFunction.signature(), &Copy.types().getFunctionType(CopiedInt32));
    const IntegerConstant *CopiedAnswer = Copy.constants().getSignedInteger(CopiedInt32, 42);
    ASSERT_NE(CopiedAnswer, nullptr);
    ASSERT_EQ(CopiedFunction.basicBlockCount(), 1U);
    ASSERT_NE(CopiedFunction.basicBlocks().begin()->terminator(), nullptr);
    ASSERT_EQ(CopiedFunction.basicBlocks().begin()->terminator()->getKind(), InstructionKind::Return);
    const auto &CopiedReturn = static_cast<const ReturnInstruction &>(*CopiedFunction.basicBlocks().begin()->terminator());
    EXPECT_EQ(CopiedReturn.getReturnValue(), CopiedAnswer);

    Function &Added = Copy.module().createFunction(Copy.types().getFunctionType(Copy.types().getVoidType()));
    Copy.context().builder().setInsertPoint(Added.createBasicBlock());
    ReturnInstruction &AddedReturn = Copy.context().builder().createReturn();
    Copy.context().builder().clearInsertPoint();
    EXPECT_EQ(&AddedReturn.getType(), &Copy.types().getVoidType());
    EXPECT_TRUE(Added.hasValidControlFlow());
  }
} // namespace ink::ir::test
