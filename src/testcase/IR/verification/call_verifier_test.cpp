#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/verification/verifier.h"

#include <gtest/gtest.h>

#include <array>

namespace ink::ir::test
{
  namespace
  {
    class CallVerificationContext final
    {
      public:
        CallVerificationContext()
            : Int32(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed)),
              One(Int32, 1),
              Builder(Types)
        {
        }

        TypePool Types;
        const IntegerType &Int32;
        const IntegerConstant One;
        const core::DiagnosticEngine Engine;
        InstructionBuilder Builder;
    };

    class UnknownCallable final : public Value
    {
      public:
        explicit UnknownCallable(const FunctionType &Signature) noexcept
            : Value(Signature, ValueKind::Unknown)
        {
        }
    };
  } // namespace

  // Direct function declarations and function-valued parameters share the same call argument and result checks.
  TEST(IRCallVerifierTest, AcceptsDirectAndFunctionArgumentCalls)
  {
    CallVerificationContext Context;
    const FunctionType &Signature = Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Context.Int32)});
    Function Target(Signature);
    Function Owner(Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Signature), Parameter(Context.Int32)}));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    CallInstruction &Direct = Context.Builder.createCall(Target, std::array<ValueRef, 1>{Owner.argument(1)});
    CallInstruction &Indirect = Context.Builder.createCall(Owner.argument(0), std::array<ValueRef, 1>{Direct});
    static_cast<void>(Context.Builder.createReturn(Indirect));
    verifyFunction(Owner, Context.Engine);
    EXPECT_EQ(&Direct.getCallee(), &Target);
    EXPECT_EQ(&Indirect.getCallee(), &Owner.argument(0));
  }

  // An instance method receives its object reference as an ordinary first argument with no implicit receiver insertion.
  TEST(IRCallVerifierTest, AcceptsExplicitObjectReceiverAsFirstArgument)
  {
    CallVerificationContext Context;
    ObjectType &Object = Context.Types.createObjectType();
    const FunctionType &Signature = Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Object), Parameter(Context.Int32)});
    Method &MethodValue = Object.addMethod(Signature);
    Function Owner(Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Object)}));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    CallInstruction &Call = Context.Builder.createCall(MethodValue.function(), std::array<ValueRef, 2>{Owner.argument(0), Context.One});
    static_cast<void>(Context.Builder.createReturn(Call));
    verifyFunction(Owner, Context.Engine);
    ASSERT_EQ(Call.argumentCount(), 2U);
    EXPECT_EQ(&Call.argument(0), &Owner.argument(0));
    EXPECT_EQ(&Call.argument(0).getType(), &Object);
  }

  // Variadic void calls accept their fixed arguments alone or additional typed SSA operands without becoming terminators.
  TEST(IRCallVerifierTest, AcceptsVariadicCallsWithAndWithoutExtraArguments)
  {
    CallVerificationContext Context;
    const FunctionType &Signature = Context.Types.getFunctionType(Context.Types.getVoidType(), std::array{Parameter(Context.Int32)}, CallABI{ABIKind::C, CallingConvention::CDecl}, true);
    Function Target(Signature);
    Function Owner(Context.Types.getFunctionType(Context.Types.getVoidType(), std::array{Parameter(Context.Types.getBoolType())}));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    CallInstruction &FixedOnly = Context.Builder.createCall(Target, std::array<ValueRef, 1>{Context.One});
    CallInstruction &Extra = Context.Builder.createCall(Target, std::array<ValueRef, 2>{Context.One, Owner.argument(0)});
    static_cast<void>(Context.Builder.createReturn());
    verifyFunction(Owner, Context.Engine);
    EXPECT_EQ(&FixedOnly.getType(), &Context.Types.getVoidType());
    EXPECT_EQ(Extra.argumentCount(), 2U);
    EXPECT_FALSE(Extra.isTerminator());
  }

  // A function value returned by one call can be the callee of a later call when its definition dominates that use.
  TEST(IRCallVerifierTest, AcceptsFunctionValuedCallResultsAsCallees)
  {
    CallVerificationContext Context;
    const FunctionType &TargetSignature = Context.Types.getFunctionType(Context.Int32);
    Function Factory(Context.Types.getFunctionType(TargetSignature));
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    CallInstruction &Callee = Context.Builder.createCall(Factory);
    CallInstruction &Call = Context.Builder.createCall(Callee);
    static_cast<void>(Context.Builder.createReturn(Call));
    verifyFunction(Owner, Context.Engine);
    EXPECT_EQ(&Callee.getType(), &TargetSignature);
    EXPECT_EQ(&Call.getCallee(), &Callee);
  }

  // A function-typed parameter owned by another function cannot be used as the current function's callee.
  TEST(IRCallVerifierDeathTest, RejectsCalleeArgumentsOwnedByAnotherFunction)
  {
    CallVerificationContext Context;
    const FunctionType &Signature = Context.Types.getFunctionType(Context.Int32);
    Function Other(Context.Types.getFunctionType(Context.Types.getVoidType(), std::array{Parameter(Signature)}));
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    CallInstruction &Call = Context.Builder.createCall(Other.argument(0));
    static_cast<void>(Context.Builder.createReturn(Call));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*operand belongs to another function");
  }

  // Matching a callee's parameter type does not permit borrowing an actual argument from another function body.
  TEST(IRCallVerifierDeathTest, RejectsActualArgumentsOwnedByAnotherFunction)
  {
    CallVerificationContext Context;
    const FunctionType &Signature = Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Context.Int32)});
    Function Target(Signature);
    Function Other(Signature);
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    CallInstruction &Call = Context.Builder.createCall(Target, std::array<ValueRef, 1>{Other.argument(0)});
    static_cast<void>(Context.Builder.createReturn(Call));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*operand belongs to another function");
  }

  // A function-valued instruction must precede a call that uses it even when both instructions belong to the same block.
  TEST(IRCallVerifierDeathTest, RejectsCalleeDefinitionsAfterTheirUse)
  {
    CallVerificationContext Context;
    const FunctionType &TargetSignature = Context.Types.getFunctionType(Context.Int32);
    Function Factory(Context.Types.getFunctionType(TargetSignature));
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    BasicBlock &Entry = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    CallInstruction &Callee = Context.Builder.createCall(Factory);
    ASSERT_TRUE(Context.Builder.setInsertPoint(Callee));
    CallInstruction &Call = Context.Builder.createCall(Callee);
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createReturn(Call));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*definition does not precede its use");
  }

  // An argument defined on the left arm of a branch cannot be passed to a call executed on the right arm.
  TEST(IRCallVerifierDeathTest, RejectsActualArgumentsThatDoNotDominateTheCall)
  {
    CallVerificationContext Context;
    Function Target(Context.Types.getFunctionType(Context.Types.getVoidType(), std::array{Parameter(Context.Int32)}));
    Function Owner(Context.Types.getFunctionType(Context.Types.getVoidType(), std::array{Parameter(Context.Types.getBoolType())}));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Left = Owner.createBasicBlock();
    BasicBlock &Right = Owner.createBasicBlock();
    BasicBlock &Merge = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createCondBranch(Owner.argument(0), Left, Right));
    Context.Builder.setInsertPoint(Left);
    AddInstruction &LeftValue = Context.Builder.createAdd(Context.One, Context.One);
    static_cast<void>(Context.Builder.createBranch(Merge));
    Context.Builder.setInsertPoint(Right);
    static_cast<void>(Context.Builder.createCall(Target, std::array<ValueRef, 1>{LeftValue}));
    static_cast<void>(Context.Builder.createBranch(Merge));
    Context.Builder.setInsertPoint(Merge);
    static_cast<void>(Context.Builder.createReturn());
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*definition does not dominate its use");
  }

  // Having a function type alone does not make an unsupported Value subclass a valid callable operand.
  TEST(IRCallVerifierDeathTest, RejectsUnsupportedCallableValueKinds)
  {
    CallVerificationContext Context;
    const UnknownCallable Callee(Context.Types.getFunctionType(Context.Int32));
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    CallInstruction &Call = Context.Builder.createCall(Callee);
    static_cast<void>(Context.Builder.createReturn(Call));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*operand has an unsupported value kind");
  }
} // namespace ink::ir::test
