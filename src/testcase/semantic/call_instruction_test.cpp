#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <vector>

namespace ink::semantic::test
{
  static_assert(std::is_base_of_v<Value, CallInstruction>);
  static_assert(!std::is_copy_constructible_v<CallInstruction>);
  static_assert(!std::is_move_constructible_v<CallInstruction>);

  // A direct call keeps the resolved function and ordered operands, produces its return type and is never interned.
  TEST(SemanticCallInstructionTest, DirectCallsPreserveTargetArgumentsAndIndependentIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Integer = Context.getIntegerType(32, true);
    ASSERT_NE(Integer, nullptr);
    const Type *Parameters[] = {
        Integer,
        &Context.getBoolType(),
    };
    const FunctionType *Signature = Context.getFunctionType(*Integer, Parameters);
    ASSERT_NE(Signature, nullptr);
    const Function *Target = Context.createFunction(Context.namePool().intern("Choose"), *Signature);
    const IntegerConstant *One = Context.getIntegerConstant(*Integer, IntegerBits(32, 1));
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(One, nullptr);
    const Value *Arguments[] = {
        One,
        &Context.getBoolConstant(true),
    };
    const CallInstruction *First = Context.createCallInstruction(*Target, Arguments);
    const CallInstruction *Second = Context.createCallInstruction(*Target, Arguments);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_EQ(First->directCallee(), Target);
    EXPECT_EQ(&First->callee(), Target);
    EXPECT_EQ(First->indirectCallee(), nullptr);
    EXPECT_EQ(&First->functionType(), Signature);
    EXPECT_EQ(&First->type(), Integer);
    ASSERT_EQ(First->arguments().size(), 2U);
    EXPECT_EQ(First->arguments()[0], One);
    EXPECT_EQ(First->arguments()[1], &Context.getBoolConstant(true));
    EXPECT_EQ(First->kind(), ValueKind::CallInstruction);
    EXPECT_TRUE(CallInstruction::classof(First));
    EXPECT_FALSE(Constant::classof(First));
    EXPECT_FALSE(Type::classof(First));
    AllocaInstruction *Slot = Context.createAllocaInstruction(*Integer);
    ASSERT_NE(Slot, nullptr);
    const StoreInstruction *Store = Context.createStoreInstruction(*Slot, *First);
    ASSERT_NE(Store, nullptr);
    EXPECT_EQ(&Store->storedValue(), First);
    Arguments[0] = First;
    const CallInstruction *Nested = Context.createCallInstruction(*Target, Arguments);
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested->arguments()[0], First);
  }

  // Zero-argument procedures return the canonical void type and repeated occurrences remain distinct.
  TEST(SemanticCallInstructionTest, ZeroArgumentCallsSupportVoidResults)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const Function *Target = Context.createFunction(Context.namePool().intern("Flush"), *Signature);
    ASSERT_NE(Target, nullptr);
    const CallInstruction *First = Context.createCallInstruction(*Target);
    const CallInstruction *Second = Context.createCallInstruction(*Target);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_TRUE(First->arguments().empty());
    EXPECT_EQ(&First->type(), &Context.getVoidType());
    EXPECT_EQ(First->directCallee(), Target);
  }

  // Both function-typed parameters and the result of a higher-order call can be indirect callees.
  TEST(SemanticCallInstructionTest, FunctionValuesAndCallResultsSupportIndirectCalls)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type *Parameters[] = {&Context.getBoolType()};
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType(), Parameters);
    ASSERT_NE(Signature, nullptr);
    const Type *CallerParameters[] = {Signature};
    const FunctionType *CallerType = Context.getFunctionType(Context.getBoolType(), CallerParameters);
    ASSERT_NE(CallerType, nullptr);
    const Function *Caller = Context.createFunction(Context.namePool().intern("Caller"), *CallerType);
    ASSERT_NE(Caller, nullptr);
    const FunctionParameter *Callee = Caller->parameters()[0];
    const Value *Arguments[] = {&Context.getBoolConstant(false)};
    const CallInstruction *Call = Context.createCallInstruction(*Callee, Arguments);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), nullptr);
    EXPECT_EQ(Call->indirectCallee(), Callee);
    EXPECT_EQ(&Call->functionType(), Signature);
    EXPECT_EQ(&Call->type(), &Context.getBoolType());
    const FunctionType *FactoryType = Context.getFunctionType(*Signature);
    ASSERT_NE(FactoryType, nullptr);
    const Function *Factory = Context.createFunction(Context.namePool().intern("Factory"), *FactoryType);
    ASSERT_NE(Factory, nullptr);
    const CallInstruction *FunctionValue = Context.createCallInstruction(*Factory);
    ASSERT_NE(FunctionValue, nullptr);
    EXPECT_EQ(&FunctionValue->type(), Signature);
    const CallInstruction *Nested = Context.createCallInstruction(*FunctionValue, Arguments);
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested->indirectCallee(), FunctionValue);
    EXPECT_EQ(&Nested->functionType(), Signature);
    EXPECT_EQ(&Nested->type(), &Context.getBoolType());
  }

  // Direct and indirect calls reject wrong arity, nulls, ordering, signedness, width and foreign arguments.
  TEST(SemanticCallInstructionTest, ArgumentsMustExactlyMatchTheResolvedSignature)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Integer = Context.getIntegerType(32, true);
    const IntegerType *Unsigned = Context.getIntegerType(32, false);
    const IntegerType *Wide = Context.getIntegerType(64, true);
    const IntegerType *ForeignInteger = Other.getIntegerType(32, true);
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Unsigned, nullptr);
    ASSERT_NE(Wide, nullptr);
    ASSERT_NE(ForeignInteger, nullptr);
    const Type *Parameters[] = {
        Integer,
        &Context.getBoolType(),
    };
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType(), Parameters);
    ASSERT_NE(Signature, nullptr);
    const Function *Target = Context.createFunction(Context.namePool().intern("Target"), *Signature);
    const FunctionType *FactoryType = Context.getFunctionType(*Signature);
    ASSERT_NE(FactoryType, nullptr);
    const Function *Factory = Context.createFunction(Context.namePool().intern("Factory"), *FactoryType);
    ASSERT_NE(Factory, nullptr);
    const CallInstruction *FunctionValue = Context.createCallInstruction(*Factory);
    const IntegerConstant *One = Context.getIntegerConstant(*Integer, IntegerBits(32, 1));
    const IntegerConstant *UnsignedOne = Context.getIntegerConstant(*Unsigned, IntegerBits(32, 1));
    const IntegerConstant *WideOne = Context.getIntegerConstant(*Wide, IntegerBits(64, 1));
    const IntegerConstant *ForeignOne = Other.getIntegerConstant(*ForeignInteger, IntegerBits(32, 1));
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(FunctionValue, nullptr);
    ASSERT_NE(One, nullptr);
    ASSERT_NE(UnsignedOne, nullptr);
    ASSERT_NE(WideOne, nullptr);
    ASSERT_NE(ForeignOne, nullptr);
    const Value *True = &Context.getBoolConstant(true);
    const std::vector<const Value *> InvalidArguments[] = {
        {},
        {One},
        {One, True, True},
        {nullptr, True},
        {One, nullptr},
        {True, One},
        {UnsignedOne, True},
        {WideOne, True},
        {ForeignOne, True},
        {One, &Other.getBoolConstant(true)},
    };
    for (const auto &Arguments : InvalidArguments)
    {
      EXPECT_EQ(Context.createCallInstruction(*Target, Arguments), nullptr);
      EXPECT_EQ(Context.createCallInstruction(*FunctionValue, Arguments), nullptr);
    }
    const Value *ValidArguments[] = {
        One,
        True,
    };
    EXPECT_NE(Context.createCallInstruction(*Target, ValidArguments), nullptr);
    EXPECT_NE(Context.createCallInstruction(*FunctionValue, ValidArguments), nullptr);
  }

  // Calls require a local function value; a function type itself and values from another context cannot be called.
  TEST(SemanticCallInstructionTest, ForeignAndNonFunctionTargetsAreRejected)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType());
    const FunctionType *ForeignSignature = Other.getFunctionType(Other.getBoolType());
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(ForeignSignature, nullptr);
    const Function *Target = Context.createFunction(Context.namePool().intern("Target"), *Signature);
    const Function *Foreign = Other.createFunction(Other.namePool().intern("Target"), *ForeignSignature);
    const Type *CallerParameters[] = {ForeignSignature};
    const FunctionType *CallerType = Other.getFunctionType(Other.getBoolType(), CallerParameters);
    ASSERT_NE(CallerType, nullptr);
    const Function *Caller = Other.createFunction(Other.namePool().intern("Caller"), *CallerType);
    ASSERT_NE(Caller, nullptr);
    const FunctionParameter *ForeignValue = Caller->parameters()[0];
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(Foreign, nullptr);
    ASSERT_NE(ForeignValue, nullptr);
    EXPECT_EQ(Context.createCallInstruction(*Foreign), nullptr);
    EXPECT_EQ(Context.createCallInstruction(*ForeignValue), nullptr);
    EXPECT_EQ(Context.createCallInstruction(Context.getBoolConstant(true)), nullptr);
    EXPECT_EQ(Context.createCallInstruction(*Signature), nullptr);
    const BasicBlock *Block = Context.createBasicBlock();
    ASSERT_NE(Block, nullptr);
    EXPECT_EQ(Context.createCallInstruction(*Block), nullptr);
    const CallInstruction *Call = Context.createCallInstruction(*Target);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), Target);
    EXPECT_EQ(Other.createCallInstruction(*Target), nullptr);
    EXPECT_FALSE(CallInstruction::classof(nullptr));
    EXPECT_FALSE(CallInstruction::classof(&Context.getBoolConstant(true)));
  }

  // Calls copy the operand list, and later call/function allocations preserve targets, operands and views.
  TEST(SemanticCallInstructionTest, OwnedOperandsAndReferencesSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type *Parameters[] = {&Context.getBoolType()};
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType(), Parameters);
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Target");
    const Function *Target = Context.createFunction(NameValue, *Signature);
    ASSERT_NE(Target, nullptr);
    const CallInstruction *First = nullptr;
    {
      std::vector<const Value *> Arguments = {&Context.getBoolConstant(true)};
      First = Context.createCallInstruction(*Target, Arguments);
      ASSERT_NE(First, nullptr);
      Arguments[0] = &Context.getBoolConstant(false);
      Arguments.clear();
    }
    const auto Arguments = First->arguments();
    for (unsigned Index = 0; Index < 2048; ++Index)
    {
      const Function *Next = Context.createFunction(NameValue, *Signature);
      ASSERT_NE(Next, nullptr);
      ASSERT_NE(Context.createCallInstruction(*Next, Arguments), nullptr);
    }
    EXPECT_EQ(First->arguments().data(), Arguments.data());
    ASSERT_EQ(Arguments.size(), 1U);
    EXPECT_EQ(Arguments[0], &Context.getBoolConstant(true));
    EXPECT_EQ(First->directCallee(), Target);
    EXPECT_EQ(&Target->type(), Signature);
    EXPECT_EQ(&First->type(), &Context.getBoolType());
    EXPECT_EQ(&First->functionType(), Signature);
  }
} // namespace ink::semantic::test
