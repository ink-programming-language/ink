#include "ink/ir/ir_builder.h"
#include "ink/ir/ir_builder.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <vector>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Value, CallInstruction>);
  static_assert(!std::is_copy_constructible_v<CallInstruction>);
  static_assert(!std::is_move_constructible_v<CallInstruction>);

  // A direct call keeps the resolved function and ordered operands, produces its return type and is never interned.
  TEST(IRCallInstructionTest, DirectCallsPreserveTargetArgumentsAndIndependentIdentity)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const IntegerType *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Integer, nullptr);
    const Type *Parameters[] = {
        Integer,
        &Context.typePool().getType<TypeKind::Bool>(),
    };
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(*Integer, Parameters);
    ASSERT_NE(Signature, nullptr);
    auto TargetOwner = Builder.createFunction(Context.namePool().intern("Choose"), *Signature);
    const Function *Target = TargetOwner.get();
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(One, nullptr);
    const Value *Arguments[] = {
        One,
        &Context.constantPool().getBoolConstant(true),
    };
    auto FirstOwner = Builder.createDetachedCallInstruction(*Target, Arguments);
    const CallInstruction *First = FirstOwner.get();
    auto SecondOwner = Builder.createDetachedCallInstruction(*Target, Arguments);
    const CallInstruction *Second = SecondOwner.get();
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
    EXPECT_EQ(First->arguments()[1], &Context.constantPool().getBoolConstant(true));
    EXPECT_EQ(First->kind(), ValueKind::CallInstruction);
    EXPECT_TRUE(CallInstruction::classof(First));
    EXPECT_FALSE(Constant::classof(First));
    EXPECT_FALSE(Type::classof(First));
    auto SlotOwner = Builder.createDetachedAllocaInstruction(*Integer);
    AllocaInstruction *Slot = SlotOwner.get();
    ASSERT_NE(Slot, nullptr);
    auto StoreOwner = Builder.createDetachedStoreInstruction(*Slot, *First);
    const StoreInstruction *Store = StoreOwner.get();
    ASSERT_NE(Store, nullptr);
    EXPECT_EQ(&Store->storedValue(), First);
    Arguments[0] = First;
    auto NestedOwner = Builder.createDetachedCallInstruction(*Target, Arguments);
    const CallInstruction *Nested = NestedOwner.get();
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested->arguments()[0], First);
  }

  // Zero-argument procedures return the canonical void type and repeated occurrences remain distinct.
  TEST(IRCallInstructionTest, ZeroArgumentCallsSupportVoidResults)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    ASSERT_NE(Signature, nullptr);
    auto TargetOwner = Builder.createFunction(Context.namePool().intern("Flush"), *Signature);
    const Function *Target = TargetOwner.get();
    ASSERT_NE(Target, nullptr);
    auto FirstOwner = Builder.createDetachedCallInstruction(*Target);
    const CallInstruction *First = FirstOwner.get();
    auto SecondOwner = Builder.createDetachedCallInstruction(*Target);
    const CallInstruction *Second = SecondOwner.get();
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_TRUE(First->arguments().empty());
    EXPECT_EQ(&First->type(), &Context.typePool().getType<TypeKind::Void>());
    EXPECT_EQ(First->directCallee(), Target);
  }

  // Both function-typed parameters and the result of a higher-order call can be indirect callees.
  TEST(IRCallInstructionTest, FunctionValuesAndCallResultsSupportIndirectCalls)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const Type *Parameters[] = {&Context.typePool().getType<TypeKind::Bool>()};
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters);
    ASSERT_NE(Signature, nullptr);
    const Type *CallerParameters[] = {Signature};
    const FunctionType *CallerType = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), CallerParameters);
    ASSERT_NE(CallerType, nullptr);
    auto CallerOwner = Builder.createFunction(Context.namePool().intern("Caller"), *CallerType);
    const Function *Caller = CallerOwner.get();
    ASSERT_NE(Caller, nullptr);
    const FunctionParameter *Callee = Caller->parameters()[0].get();
    const Value *Arguments[] = {&Context.constantPool().getBoolConstant(false)};
    auto CallOwner = Builder.createDetachedCallInstruction(*Callee, Arguments);
    const CallInstruction *Call = CallOwner.get();
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), nullptr);
    EXPECT_EQ(Call->indirectCallee(), Callee);
    EXPECT_EQ(&Call->functionType(), Signature);
    EXPECT_EQ(&Call->type(), &Context.typePool().getType<TypeKind::Bool>());
    const FunctionType *FactoryType = Context.typePool().getType<TypeKind::Function>(*Signature);
    ASSERT_NE(FactoryType, nullptr);
    auto FactoryOwner = Builder.createFunction(Context.namePool().intern("Factory"), *FactoryType);
    const Function *Factory = FactoryOwner.get();
    ASSERT_NE(Factory, nullptr);
    auto FunctionValueOwner = Builder.createDetachedCallInstruction(*Factory);
    const CallInstruction *FunctionValue = FunctionValueOwner.get();
    ASSERT_NE(FunctionValue, nullptr);
    EXPECT_EQ(&FunctionValue->type(), Signature);
    auto NestedOwner = Builder.createDetachedCallInstruction(*FunctionValue, Arguments);
    const CallInstruction *Nested = NestedOwner.get();
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested->indirectCallee(), FunctionValue);
    EXPECT_EQ(&Nested->functionType(), Signature);
    EXPECT_EQ(&Nested->type(), &Context.typePool().getType<TypeKind::Bool>());
  }

  // Direct and indirect calls reject wrong arity, nulls, ordering, signedness, width and foreign arguments.
  TEST(IRCallInstructionTest, ArgumentsMustExactlyMatchTheResolvedSignature)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    IRContext Other(Compilation);
    const IntegerType *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const IntegerType *Unsigned = Context.typePool().getType<TypeKind::Integer>(32, false);
    const IntegerType *Wide = Context.typePool().getType<TypeKind::Integer>(64, true);
    const IntegerType *ForeignInteger = Other.typePool().getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Unsigned, nullptr);
    ASSERT_NE(Wide, nullptr);
    ASSERT_NE(ForeignInteger, nullptr);
    const Type *Parameters[] = {
        Integer,
        &Context.typePool().getType<TypeKind::Bool>(),
    };
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters);
    ASSERT_NE(Signature, nullptr);
    auto TargetOwner = Builder.createFunction(Context.namePool().intern("Target"), *Signature);
    const Function *Target = TargetOwner.get();
    const FunctionType *FactoryType = Context.typePool().getType<TypeKind::Function>(*Signature);
    ASSERT_NE(FactoryType, nullptr);
    auto FactoryOwner = Builder.createFunction(Context.namePool().intern("Factory"), *FactoryType);
    const Function *Factory = FactoryOwner.get();
    ASSERT_NE(Factory, nullptr);
    auto FunctionValueOwner = Builder.createDetachedCallInstruction(*Factory);
    const CallInstruction *FunctionValue = FunctionValueOwner.get();
    const IntegerConstant *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    const IntegerConstant *UnsignedOne = Context.constantPool().getIntegerConstant(*Unsigned, IntegerBits(32, 1));
    const IntegerConstant *WideOne = Context.constantPool().getIntegerConstant(*Wide, IntegerBits(64, 1));
    const IntegerConstant *ForeignOne = Other.constantPool().getIntegerConstant(*ForeignInteger, IntegerBits(32, 1));
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(FunctionValue, nullptr);
    ASSERT_NE(One, nullptr);
    ASSERT_NE(UnsignedOne, nullptr);
    ASSERT_NE(WideOne, nullptr);
    ASSERT_NE(ForeignOne, nullptr);
    const Value *True = &Context.constantPool().getBoolConstant(true);
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
        {One, &Other.constantPool().getBoolConstant(true)},
    };
    for (const auto &Arguments : InvalidArguments)
    {
      EXPECT_EQ(Builder.createDetachedCallInstruction(*Target, Arguments), nullptr);
      EXPECT_EQ(Builder.createDetachedCallInstruction(*FunctionValue, Arguments), nullptr);
    }
    const Value *ValidArguments[] = {
        One,
        True,
    };
    EXPECT_NE(Builder.createDetachedCallInstruction(*Target, ValidArguments), nullptr);
    EXPECT_NE(Builder.createDetachedCallInstruction(*FunctionValue, ValidArguments), nullptr);
  }

  // Calls require a local function value; a function type itself and values from another context cannot be called.
  TEST(IRCallInstructionTest, ForeignAndNonFunctionTargetsAreRejected)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    IRContext Other(Compilation);
    IRBuilder OtherBuilder(Other);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>());
    const FunctionType *ForeignSignature = Other.typePool().getType<TypeKind::Function>(Other.typePool().getType<TypeKind::Bool>());
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(ForeignSignature, nullptr);
    auto TargetOwner = Builder.createFunction(Context.namePool().intern("Target"), *Signature);
    const Function *Target = TargetOwner.get();
    auto ForeignOwner = OtherBuilder.createFunction(Other.namePool().intern("Target"), *ForeignSignature);
    const Function *Foreign = ForeignOwner.get();
    const Type *CallerParameters[] = {ForeignSignature};
    const FunctionType *CallerType = Other.typePool().getType<TypeKind::Function>(Other.typePool().getType<TypeKind::Bool>(), CallerParameters);
    ASSERT_NE(CallerType, nullptr);
    auto CallerOwner = OtherBuilder.createFunction(Other.namePool().intern("Caller"), *CallerType);
    const Function *Caller = CallerOwner.get();
    ASSERT_NE(Caller, nullptr);
    const FunctionParameter *ForeignValue = Caller->parameters()[0].get();
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(Foreign, nullptr);
    ASSERT_NE(ForeignValue, nullptr);
    EXPECT_EQ(Builder.createDetachedCallInstruction(*Foreign), nullptr);
    EXPECT_EQ(Builder.createDetachedCallInstruction(*ForeignValue), nullptr);
    EXPECT_EQ(Builder.createDetachedCallInstruction(Context.constantPool().getBoolConstant(true)), nullptr);
    EXPECT_EQ(Builder.createDetachedCallInstruction(*Signature), nullptr);
    auto BlockOwner = Builder.createBasicBlock();
    const BasicBlock *Block = BlockOwner.get();
    ASSERT_NE(Block, nullptr);
    EXPECT_EQ(Builder.createDetachedCallInstruction(*Block), nullptr);
    auto CallOwner = Builder.createDetachedCallInstruction(*Target);
    const CallInstruction *Call = CallOwner.get();
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), Target);
    EXPECT_EQ(OtherBuilder.createDetachedCallInstruction(*Target), nullptr);
    EXPECT_FALSE(CallInstruction::classof(nullptr));
    EXPECT_FALSE(CallInstruction::classof(&Context.constantPool().getBoolConstant(true)));
  }

  // Calls copy the operand list, and later call/function allocations preserve targets, operands and views.
  TEST(IRCallInstructionTest, OwnedOperandsAndReferencesSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const Type *Parameters[] = {&Context.typePool().getType<TypeKind::Bool>()};
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Bool>(), Parameters);
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Target");
    auto TargetOwner = Builder.createFunction(NameValue, *Signature);
    const Function *Target = TargetOwner.get();
    ASSERT_NE(Target, nullptr);
    const CallInstruction *First = nullptr;
    std::unique_ptr<CallInstruction> FirstOwner;
    {
      std::vector<const Value *> Arguments = {&Context.constantPool().getBoolConstant(true)};
      FirstOwner = Builder.createDetachedCallInstruction(*Target, Arguments);
      First = FirstOwner.get();
      ASSERT_NE(First, nullptr);
      Arguments[0] = &Context.constantPool().getBoolConstant(false);
      Arguments.clear();
    }
    const auto Arguments = First->arguments();
    auto Block = Builder.createBasicBlock();
    ASSERT_TRUE(Builder.appendValue(*Block, std::move(TargetOwner)));
    ASSERT_TRUE(Builder.appendValue(*Block, std::move(FirstOwner)));
    for (unsigned Index = 0; Index < 2048; ++Index)
    {
      auto NextOwner = Builder.createFunction(NameValue, *Signature);
      const Function *Next = NextOwner.get();
      ASSERT_NE(Next, nullptr);
      ASSERT_TRUE(Builder.appendValue(*Block, std::move(NextOwner)));
      ASSERT_TRUE(Builder.appendValue(*Block, Builder.createDetachedCallInstruction(*Next, Arguments)));
    }
    EXPECT_EQ(First->arguments().data(), Arguments.data());
    ASSERT_EQ(Arguments.size(), 1U);
    EXPECT_EQ(Arguments[0], &Context.constantPool().getBoolConstant(true));
    EXPECT_EQ(First->directCallee(), Target);
    EXPECT_EQ(&Target->type(), Signature);
    EXPECT_EQ(&First->type(), &Context.typePool().getType<TypeKind::Bool>());
    EXPECT_EQ(&First->functionType(), Signature);
  }
} // namespace ink::ir::test
