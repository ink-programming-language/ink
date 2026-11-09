#include "ink/core/support/core_define.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_same_v<decltype(std::declval<const CallInstruction &>().getCallee()), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<const CallInstruction &>().signature()), const FunctionType &>);
  static_assert(std::is_same_v<decltype(std::declval<const CallInstruction &>().arguments()), std::span<const ValueRef>>);
  static_assert(std::is_same_v<decltype(std::declval<const CallInstruction &>().argument(0)), const Value &>);
  static_assert(std::is_same_v<decltype(std::declval<InstructionBuilder &>().createCall(std::declval<ValueRef>())), CallInstruction &>);
  static_assert(std::is_constructible_v<CallInstruction, const Function &, std::span<const ValueRef>>);
  static_assert(!std::is_constructible_v<CallInstruction, Function &&>);
  static_assert(!std::is_copy_constructible_v<CallInstruction> && !std::is_move_constructible_v<CallInstruction>);

  // A call owns its reference list even after the caller changes and destroys the source argument array.
  TEST(IRCallInstructionTest, CopiesArgumentReferencesAndInfersTheResultType)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    const IntegerConstant Two(Int32, 2);
    const FunctionType &Signature = Types.getFunctionType(Int32, std::array{Parameter(Int32), Parameter(Int32)});
    Function Callee(Signature);
    std::unique_ptr<CallInstruction> Call;
    {
      std::array<ValueRef, 2> Arguments{One, Two};
      Call = std::make_unique<CallInstruction>(Callee, Arguments);
      Arguments[0] = Two;
      Arguments[1] = One;
    }

    EXPECT_EQ(&Call->getCallee(), &Callee);
    EXPECT_EQ(&Call->signature(), &Signature);
    EXPECT_EQ(&Call->getType(), &Int32);
    ASSERT_EQ(Call->argumentCount(), 2U);
    EXPECT_EQ(&Call->argument(0), &One);
    EXPECT_EQ(&Call->argument(1), &Two);
    ASSERT_EQ(Call->arguments().size(), 2U);
    EXPECT_EQ(&Call->arguments()[0].get(), &One);
    EXPECT_EQ(&Call->arguments()[1].get(), &Two);
    ASSERT_EQ(Call->operandCount(), 3U);
    EXPECT_EQ(&Call->operand(0), &Callee);
    EXPECT_EQ(&Call->operand(1), &One);
    EXPECT_EQ(&Call->operand(2), &Two);
    EXPECT_EQ(Call->getKind(), InstructionKind::Call);
    EXPECT_EQ(Call->getName(), "call");
    EXPECT_FALSE(Call->isTerminator());
    EXPECT_TRUE(CallInstruction::classof(Call.get()));
    EXPECT_FALSE(CallInstruction::classof(nullptr));
  }

  // An instance method receives its reference-semantic object as an ordinary explicit first argument.
  TEST(IRCallInstructionTest, PreservesAnExplicitReceiverBeforeTheMethodArguments)
  {
    TypePool Types;
    ObjectType &Object = Types.createObjectType();
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const std::array Parameters{Parameter(Object), Parameter(Int32)};
    Method &MethodValue = Object.addMethod(Types.getFunctionType(Int32, Parameters));
    Function Caller(Types.getFunctionType(Types.getVoidType(), Parameters));
    const CallInstruction Call(MethodValue.function(), std::array<ValueRef, 2>{Caller.argument(0), Caller.argument(1)});

    EXPECT_EQ(&Call.getCallee(), &MethodValue.function());
    ASSERT_EQ(Call.argumentCount(), 2U);
    EXPECT_EQ(&Call.argument(0), &Caller.argument(0));
    EXPECT_EQ(&Call.argument(0).getType(), &Object);
    EXPECT_EQ(&Call.argument(1), &Caller.argument(1));
    EXPECT_EQ(&Call.getType(), &Int32);
  }

  // A zero-argument void call still exposes its callee operand and does not terminate its block.
  TEST(IRCallInstructionTest, RepresentsVoidCallsWithoutArguments)
  {
    TypePool Types;
    Function Callee(Types.getFunctionType(Types.getVoidType()));
    const CallInstruction Call(Callee);
    ReturnInstruction Return(Types.getVoidType());

    EXPECT_EQ(&Call.getType(), &Types.getVoidType());
    EXPECT_EQ(Call.argumentCount(), 0U);
    EXPECT_TRUE(Call.arguments().empty());
    ASSERT_EQ(Call.operandCount(), 1U);
    EXPECT_EQ(&Call.operand(0), &Callee);
    EXPECT_FALSE(Call.isTerminator());
    EXPECT_FALSE(CallInstruction::classof(&Return));
  }

  // Function-typed SSA arguments are callable without adding a pointer layer to their signature.
  TEST(IRCallInstructionTest, AcceptsFunctionTypedArgumentCallees)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const FunctionType &Signature = Types.getFunctionType(Int32, std::array{Parameter(Int32)});
    Function Caller(Types.getFunctionType(Int32, std::array{Parameter(Signature), Parameter(Int32)}));
    const CallInstruction Call(Caller.argument(0), std::array<ValueRef, 1>{Caller.argument(1)});

    EXPECT_EQ(&Call.getCallee(), &Caller.argument(0));
    EXPECT_EQ(&Call.signature(), &Signature);
    EXPECT_EQ(&Call.argument(0), &Caller.argument(1));
    EXPECT_EQ(&Call.getType(), &Int32);
  }

  // Operand replacement preserves call shape and rejects different callee signatures or argument type identities atomically.
  TEST(IRCallInstructionTest, ReplacesCalleeAndArgumentsOnlyWithIdenticalTypes)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType OtherInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    const IntegerConstant Two(Int32, 2);
    const IntegerConstant OtherOne(OtherInt32, 1);
    const FunctionType &Signature = Types.getFunctionType(Int32, std::array{Parameter(Int32)});
    const FunctionType OtherSignature(Int32, std::array{Parameter(Int32)});
    Function First(Signature);
    Function Second(Signature);
    Function Other(OtherSignature);
    CallInstruction Call(First, std::array<ValueRef, 1>{One});
    Instruction &Base = Call;

    ASSERT_TRUE(Base.setOperand(0, Second));
    ASSERT_TRUE(Base.setOperand(1, Two));
    EXPECT_FALSE(Base.setOperand(0, Other));
    EXPECT_FALSE(Base.setOperand(0, One));
    EXPECT_FALSE(Base.setOperand(1, OtherOne));
    EXPECT_FALSE(Base.setOperand(2, Two));
    EXPECT_FALSE(Base.setOperand(std::numeric_limits<std::size_t>::max(), Two));
    EXPECT_EQ(&Call.getCallee(), &Second);
    EXPECT_EQ(&Call.signature(), &Signature);
    EXPECT_EQ(&Call.argument(0), &Two);
    EXPECT_EQ(&Call.getType(), &Int32);
    EXPECT_EQ(Call.argumentCount(), 1U);
    EXPECT_EQ(Call.operandCount(), 2U);
  }

  // Builder insertion before a return keeps call creation order and block ownership after the builder is destroyed.
  TEST(IRCallInstructionTest, BuildsOwnedCallsBeforeAnExistingTerminator)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    Function Callee(Types.getFunctionType(Int32, std::array{Parameter(Int32)}));
    Function Caller(Types.getFunctionType(Types.getVoidType()));
    BasicBlock &Entry = Caller.createBasicBlock();
    CallInstruction *First = nullptr;
    CallInstruction *Second = nullptr;
    ReturnInstruction *Return = nullptr;
    {
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Entry);
      Return = &Builder.createReturn();
      ASSERT_TRUE(Builder.setInsertPoint(*Return));
      First = &Builder.createCall(Callee, std::array<ValueRef, 1>{One});
      Second = &Builder.createCall(Callee, std::array<ValueRef, 1>{*First});
      EXPECT_EQ(Builder.getInsertBefore(), Return);
    }

    ASSERT_EQ(Entry.instructionCount(), 3U);
    auto Position = Entry.instructions().begin();
    EXPECT_EQ(&*Position++, First);
    EXPECT_EQ(&*Position++, Second);
    EXPECT_EQ(&*Position++, Return);
    EXPECT_EQ(Position, Entry.instructions().end());
    EXPECT_EQ(First->parent(), &Entry);
    EXPECT_EQ(Second->parent(), &Entry);
    EXPECT_EQ(&Second->argument(0), First);
    EXPECT_EQ(Entry.terminator(), Return);
  }

  // Variadic calls preserve a typed fixed prefix and the ordered non-void extra arguments.
  TEST(IRCallInstructionTest, BuildsVariadicCallsWithAndWithoutExtraArguments)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType &Int64 = Types.getIntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    const IntegerConstant Two(Int64, 2);
    Function Callee(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Int32)}, {}, true));
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);
    CallInstruction &FixedOnly = Builder.createCall(Callee, std::array<ValueRef, 1>{One});
    CallInstruction &WithExtra = Builder.createCall(Callee, std::array<ValueRef, 2>{One, Two});

    EXPECT_EQ(FixedOnly.argumentCount(), 1U);
    EXPECT_EQ(WithExtra.argumentCount(), 2U);
    EXPECT_EQ(&WithExtra.argument(0), &One);
    EXPECT_EQ(&WithExtra.argument(1), &Two);
    EXPECT_TRUE(WithExtra.signature().isVariadic());
  }

#if INK_ENABLE_ASSERTIONS
  // Call creation requires an insertion point and rejects non-function values, mismatched counts, and nonidentical fixed types.
  TEST(IRCallInstructionDeathTest, ChecksBuilderCalleeArityAndFixedArgumentTypes)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerType OtherInt32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    const IntegerConstant OtherOne(OtherInt32, 1);
    Function Callee(Types.getFunctionType(Int32, std::array{Parameter(Int32)}));
    Function ZeroArgumentCallee(Types.getFunctionType(Types.getVoidType()));
    const std::array<ValueRef, 1> Arguments{One};
    const std::array<ValueRef, 1> OtherArguments{OtherOne};
    BasicBlock Block;
    InstructionBuilder Builder(Types);

    EXPECT_DEATH(static_cast<void>(Builder.createCall(ZeroArgumentCallee)), "insertion point");
    Builder.setInsertPoint(Block);
    EXPECT_DEATH(static_cast<void>(Builder.createCall(One)), "Call requires a function-typed callee");
    EXPECT_DEATH(static_cast<void>(Builder.createCall(Callee)), "Call argument count must match the callee signature");
    EXPECT_DEATH(static_cast<void>(Builder.createCall(ZeroArgumentCallee, Arguments)), "Call argument count must match the callee signature");
    EXPECT_DEATH(static_cast<void>(Builder.createCall(Callee, OtherArguments)), "Call argument type must match the callee parameter type");
    EXPECT_EQ(Block.instructionCount(), 0U);
  }

  // A variadic call still requires every fixed parameter and rejects void-typed extra values.
  TEST(IRCallInstructionDeathTest, RejectsMissingFixedAndVoidVariadicArguments)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant One(Int32, 1);
    Function Callee(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Int32)}, {}, true));
    ReturnInstruction VoidValue(Types.getVoidType());
    const std::array<ValueRef, 2> Arguments{One, VoidValue};
    BasicBlock Block;
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Block);

    EXPECT_DEATH(static_cast<void>(Builder.createCall(Callee)), "Call argument count must match the callee signature");
    EXPECT_DEATH(static_cast<void>(Builder.createCall(Callee, Arguments)), "Call arguments must have non-void types");
    EXPECT_EQ(Block.instructionCount(), 0U);
  }
#endif
} // namespace ink::ir::test
