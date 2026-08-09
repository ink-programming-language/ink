#include "ink/execution/interpreter.h"
#include "ink/execution/runtime_world.h"
#include "ink/ir/builder.h"
#include "ink/ir/verifier.h"
#include "ink/target/target_context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    target::TargetKey hostTargetKey()
    {
      return target::TargetContext::host().key();
    }

    ir::VerifiedClosedModule finishAndClose(ir::IrBuilder &Builder, const target::TargetKey &TargetKey)
    {
      const ir::UnverifiedStagedModule Unverified = Builder.finish();
      ir::IrStagedVerificationResult Staged = ir::verifyStaged(Unverified);
      if (!Staged.succeeded())
      {
        const std::string Message = Staged.errors().empty() ? "staged verification failed" : Staged.errors().front().Message;
        throw std::runtime_error(Message);
      }
      ir::IrClosedVerificationResult Closed = ir::closeAndVerify(Staged.verified(), TargetKey);
      if (!Closed.succeeded())
      {
        const std::string Message = Closed.errors().empty() ? "closed verification failed" : Closed.errors().front().Message;
        throw std::runtime_error(Message);
      }
      return Closed.takeVerified();
    }

    ExecutionResult runWithoutArguments(const ir::VerifiedClosedModule &Module, ir::IrFunctionId Entry, ExecutionLimits Limits = {})
    {
      RuntimeWorld World(Module.targetKey());
      return interpret(Module, Entry, World, {}, Limits);
    }

    struct ClosedEntry
    {
      ir::VerifiedClosedModule Module;
      ir::IrFunctionId Entry;
    };

    ClosedEntry buildIntegerCastModule(ir::IrSignedness Signedness)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId SourceType = Builder.integerType(32, Signedness);
      const ir::IrTypeId ResultType = Builder.integerType(64, Signedness);
      const ir::IrTypeId Signature = Builder.functionType({}, ResultType);
      const ir::IrFunctionId Main = Builder.addFunction("main", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
      const ir::IrConstantId NegativeOne = Builder.integerConstant(SourceType, 0xffffffffU);
      const ir::IrValueId Source = Builder.createIntegerConstant(Entry.Block, NegativeOne);
      const ir::IrValueId Cast = Builder.createIntegerCast(Entry.Block, Source, ResultType);
      Builder.createReturn(Entry.Block, Cast);
      return {finishAndClose(Builder, hostTargetKey()), Main};
    }

    ClosedEntry buildComparisonModule(ir::IrSignedness Signedness, ir::IrComparePredicate Predicate)
    {
      ir::IrBuilder Builder;
      const ir::IrTypeId IntegerType = Builder.integerType(32, Signedness);
      const ir::IrTypeId BoolType = Builder.boolType();
      const ir::IrTypeId Signature = Builder.functionType({}, BoolType);
      const ir::IrFunctionId Main = Builder.addFunction("main", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
      const ir::IrValueId AllOnes = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(IntegerType, 0xffffffffU));
      const ir::IrValueId Zero = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(IntegerType, 0));
      const ir::IrValueId Compared = Builder.createIntegerCompare(Entry.Block, Predicate, AllOnes, Zero);
      Builder.createReturn(Entry.Block, Compared);
      return {finishAndClose(Builder, hostTargetKey()), Main};
    }
  } // namespace

  // Verifies add, subtract, and multiply retain the low N bits instead of inheriting signed host overflow.
  TEST(RuntimeWorldTest, ExecutesExactWidthWrappingArithmetic)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId Signature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", Signature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
    const ir::IrValueId Maximum = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 0x7fffffffU));
    const ir::IrValueId One = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 1));
    const ir::IrValueId Two = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 2));
    const ir::IrValueId Minimum = Builder.createIntegerBinary(Entry.Block, ir::IrOpcode::IntAdd, Maximum, One);
    const ir::IrValueId MaximumAgain = Builder.createIntegerBinary(Entry.Block, ir::IrOpcode::IntSub, Minimum, One);
    const ir::IrValueId Product = Builder.createIntegerBinary(Entry.Block, ir::IrOpcode::IntMul, MaximumAgain, Two);
    Builder.createReturn(Entry.Block, Product);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

    const ExecutionResult Result = runWithoutArguments(Module, Main);

    ASSERT_EQ(Result.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(Result.Value.has_value());
    EXPECT_EQ(Result.Value->type(), I32);
    EXPECT_EQ(Result.Value->bits(), 0xfffffffeU);
  }

  // Verifies negation wraps at the declared width and signed versus unsigned widening chooses sign or zero extension.
  TEST(RuntimeWorldTest, ExecutesNegationAndSignednessDirectedIntegerCasts)
  {
    ir::IrBuilder NegateBuilder;
    const ir::IrTypeId I32 = NegateBuilder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId NegateSignature = NegateBuilder.functionType({}, I32);
    const ir::IrFunctionId NegateMain = NegateBuilder.addFunction("main", NegateSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock NegateEntry = NegateBuilder.addBlock(NegateMain);
    const ir::IrValueId Minimum = NegateBuilder.createIntegerConstant(NegateEntry.Block, NegateBuilder.integerConstant(I32, 0x80000000U));
    const ir::IrValueId Negated = NegateBuilder.createIntegerNegate(NegateEntry.Block, Minimum);
    NegateBuilder.createReturn(NegateEntry.Block, Negated);
    const ir::VerifiedClosedModule NegateModule = finishAndClose(NegateBuilder, hostTargetKey());
    const ExecutionResult NegateResult = runWithoutArguments(NegateModule, NegateMain);
    ASSERT_EQ(NegateResult.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(NegateResult.Value.has_value());
    EXPECT_EQ(NegateResult.Value->bits(), 0x80000000U);

    const ClosedEntry Signed = buildIntegerCastModule(ir::IrSignedness::Signed);
    const ClosedEntry Unsigned = buildIntegerCastModule(ir::IrSignedness::Unsigned);
    const ExecutionResult SignedResult = runWithoutArguments(Signed.Module, Signed.Entry);
    const ExecutionResult UnsignedResult = runWithoutArguments(Unsigned.Module, Unsigned.Entry);
    ASSERT_TRUE(SignedResult.Value.has_value());
    ASSERT_TRUE(UnsignedResult.Value.has_value());
    EXPECT_EQ(SignedResult.Value->bits(), 0xffffffffffffffffULL);
    EXPECT_EQ(UnsignedResult.Value->bits(), 0x00000000ffffffffULL);
  }

  // Verifies signed and unsigned comparisons interpret the same bit pattern using their explicit predicates.
  TEST(RuntimeWorldTest, DistinguishesSignedAndUnsignedComparisonPredicates)
  {
    const ClosedEntry Signed = buildComparisonModule(ir::IrSignedness::Signed, ir::IrComparePredicate::SignedLess);
    const ClosedEntry Unsigned = buildComparisonModule(ir::IrSignedness::Unsigned, ir::IrComparePredicate::UnsignedGreater);

    const ExecutionResult SignedResult = runWithoutArguments(Signed.Module, Signed.Entry);
    const ExecutionResult UnsignedResult = runWithoutArguments(Unsigned.Module, Unsigned.Entry);

    ASSERT_EQ(SignedResult.Status, ExecutionStatus::Returned);
    ASSERT_EQ(UnsignedResult.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(SignedResult.Value.has_value());
    ASSERT_TRUE(UnsignedResult.Value.has_value());
    EXPECT_EQ(SignedResult.Value->bits(), 1U);
    EXPECT_EQ(UnsignedResult.Value->bits(), 1U);
  }

  // Verifies conditional branches select one edge and bind its arguments atomically to the destination block.
  TEST(RuntimeWorldTest, TransfersSelectedConditionalBranchArguments)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId BoolType = Builder.boolType();
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId Signature = Builder.functionType({BoolType}, I32);
    const ir::IrFunctionId Choose = Builder.addFunction("choose", Signature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Choose, {BoolType});
    const ir::IrBuiltBlock Merge = Builder.addBlock(Choose, {I32});
    const ir::IrValueId TrueValue = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 42));
    const ir::IrValueId FalseValue = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 7));
    Builder.createConditionalBranch(Entry.Block, Entry.Arguments.front(), Merge.Block, {TrueValue}, Merge.Block, {FalseValue});
    Builder.createReturn(Merge.Block, Merge.Arguments.front());
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());
    RuntimeWorld World(Module.targetKey());

    const ExecutionResult TrueResult = interpret(Module, Choose, World, {RuntimeValue::fromBool(BoolType, true)});
    const ExecutionResult FalseResult = interpret(Module, Choose, World, {RuntimeValue::fromBool(BoolType, false)});

    ASSERT_TRUE(TrueResult.Value.has_value());
    ASSERT_TRUE(FalseResult.Value.has_value());
    EXPECT_EQ(TrueResult.Value->bits(), 42U);
    EXPECT_EQ(FalseResult.Value->bits(), 7U);
  }

  // Verifies direct calls establish a fresh frame, bind scalar parameters, and return into the caller SSA result.
  TEST(RuntimeWorldTest, ExecutesNestedDirectCalls)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId AddSignature = Builder.functionType({I32, I32}, I32);
    const ir::IrFunctionId Add = Builder.addFunction("add", AddSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock AddEntry = Builder.addBlock(Add, {I32, I32});
    const ir::IrValueId Sum = Builder.createIntegerBinary(AddEntry.Block, ir::IrOpcode::IntAdd, AddEntry.Arguments[0], AddEntry.Arguments[1]);
    Builder.createReturn(AddEntry.Block, Sum);
    const ir::IrTypeId MainSignature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", MainSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock MainEntry = Builder.addBlock(Main);
    const ir::IrValueId Twenty = Builder.createIntegerConstant(MainEntry.Block, Builder.integerConstant(I32, 20));
    const ir::IrValueId TwentyTwo = Builder.createIntegerConstant(MainEntry.Block, Builder.integerConstant(I32, 22));
    const ir::IrBuiltOperation Call = Builder.createDirectCall(MainEntry.Block, Add, {Twenty, TwentyTwo});
    Builder.createReturn(MainEntry.Block, Call.Results.front());
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

    const ExecutionResult Result = runWithoutArguments(Module, Main);

    ASSERT_EQ(Result.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(Result.Value.has_value());
    EXPECT_EQ(Result.Value->bits(), 42U);
    EXPECT_EQ(Result.Statistics.MaximumCallDepth, 2U);
  }

  // Verifies void returns have no runtime value and a void direct call resumes its caller without an SSA result.
  TEST(RuntimeWorldTest, ResumesCallerAfterVoidDirectCall)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId VoidSignature = Builder.functionType({}, std::nullopt);
    const ir::IrFunctionId Noop = Builder.addFunction("noop", VoidSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock NoopEntry = Builder.addBlock(Noop);
    Builder.createReturn(NoopEntry.Block);
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId MainSignature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", MainSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock MainEntry = Builder.addBlock(Main);
    const ir::IrBuiltOperation Call = Builder.createDirectCall(MainEntry.Block, Noop, {});
    EXPECT_TRUE(Call.Results.empty());
    const ir::IrValueId FortyTwo = Builder.createIntegerConstant(MainEntry.Block, Builder.integerConstant(I32, 42));
    Builder.createReturn(MainEntry.Block, FortyTwo);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

    const ExecutionResult VoidResult = runWithoutArguments(Module, Noop);
    const ExecutionResult Result = runWithoutArguments(Module, Main);

    EXPECT_EQ(VoidResult.Status, ExecutionStatus::Returned);
    EXPECT_FALSE(VoidResult.Value.has_value());
    ASSERT_EQ(Result.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(Result.Value.has_value());
    EXPECT_EQ(Result.Value->bits(), 42U);
    EXPECT_EQ(Result.Statistics.MaximumCallDepth, 2U);
  }

  // Verifies the retained boolean extension computes canonical zero-or-one values for not, and, and or.
  TEST(RuntimeWorldTest, ExecutesCanonicalBooleanOperations)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId BoolType = Builder.boolType();
    const ir::IrTypeId Signature = Builder.functionType({BoolType, BoolType}, BoolType);
    const ir::IrFunctionId Logic = Builder.addFunction("logic", Signature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Logic, {BoolType, BoolType});
    const ir::IrValueId NotLeft = Builder.createBoolUnary(Entry.Block, ir::IrOpcode::BoolNot, Entry.Arguments[0]);
    const ir::IrValueId Both = Builder.createBoolBinary(Entry.Block, ir::IrOpcode::BoolAnd, NotLeft, Entry.Arguments[1]);
    const ir::IrValueId ResultValue = Builder.createBoolBinary(Entry.Block, ir::IrOpcode::BoolOr, Both, Entry.Arguments[0]);
    Builder.createReturn(Entry.Block, ResultValue);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());
    RuntimeWorld World(Module.targetKey());

    const ExecutionResult Result = interpret(Module, Logic, World, {RuntimeValue::fromBool(BoolType, false), RuntimeValue::fromBool(BoolType, true)});

    ASSERT_EQ(Result.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(Result.Value.has_value());
    EXPECT_EQ(Result.Value->bits(), 1U);
  }

  // Verifies the retained local-memory extension starts uninitialized, stores a typed value, and loads it from the same activation.
  TEST(RuntimeWorldTest, ExecutesTypedAllocaStoreAndLoad)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId Signature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", Signature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
    const ir::IrValueId Place = Builder.createAlloca(Entry.Block, I32, ir::IrPlaceAccess::ReadWrite);
    const ir::IrValueId FortyTwo = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 42));
    Builder.createStore(Entry.Block, Place, FortyTwo);
    const ir::IrValueId Loaded = Builder.createLoad(Entry.Block, Place);
    Builder.createReturn(Entry.Block, Loaded);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

    const ExecutionResult Result = runWithoutArguments(Module, Main);

    ASSERT_EQ(Result.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(Result.Value.has_value());
    EXPECT_EQ(Result.Value->bits(), 42U);
    EXPECT_GT(Result.Statistics.PeakStackBytes, 0U);
  }

  // Verifies a trap in a never-returning direct callee propagates as a language trap and never executes the caller's unreachable marker.
  TEST(RuntimeWorldTest, PropagatesTrapThroughNeverDirectCall)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId NeverType = Builder.neverType();
    const ir::IrTypeId DieSignature = Builder.functionType({}, NeverType);
    const ir::IrFunctionId Die = Builder.addFunction("die", DieSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock DieEntry = Builder.addBlock(Die);
    Builder.createTrap(DieEntry.Block, ir::IrTrapKind::Bounds);
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId MainSignature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", MainSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock MainEntry = Builder.addBlock(Main);
    const ir::IrBuiltOperation Call = Builder.createDirectCall(MainEntry.Block, Die, {});
    EXPECT_TRUE(Call.Results.empty());
    Builder.createUnreachable(MainEntry.Block);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

    const ExecutionResult Result = runWithoutArguments(Module, Main);

    EXPECT_EQ(Result.Status, ExecutionStatus::LanguageTrap);
    ASSERT_TRUE(Result.Trap.has_value());
    EXPECT_EQ(*Result.Trap, ir::IrTrapKind::Bounds);
  }

  // Verifies an infinite CFG backedge terminates with the explicit fuel limit instead of hanging or fabricating a language value.
  TEST(RuntimeWorldTest, StopsInfiniteControlFlowAtFuelLimit)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId Signature = Builder.functionType({}, std::nullopt);
    const ir::IrFunctionId Loop = Builder.addFunction("loop", Signature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Loop);
    Builder.createBranch(Entry.Block, Entry.Block);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());
    ExecutionLimits Limits;
    Limits.Fuel = 3;

    const ExecutionResult Result = runWithoutArguments(Module, Loop, Limits);

    EXPECT_EQ(Result.Status, ExecutionStatus::LimitExceeded);
    EXPECT_EQ(Result.Limit, ExecutionLimitKind::Fuel);
    EXPECT_EQ(Result.Statistics.FuelConsumed, 3U);
  }

  // Verifies recursive direct calls stop before creating a frame beyond the configured call-depth budget.
  TEST(RuntimeWorldTest, StopsRecursionAtCallDepthLimit)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId Signature = Builder.functionType({}, I32);
    const ir::IrFunctionId Recurse = Builder.addFunction("recurse", Signature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Recurse);
    const ir::IrBuiltOperation Call = Builder.createDirectCall(Entry.Block, Recurse, {});
    Builder.createReturn(Entry.Block, Call.Results.front());
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());
    ExecutionLimits Limits;
    Limits.MaxCallDepth = 3;

    const ExecutionResult Result = runWithoutArguments(Module, Recurse, Limits);

    EXPECT_EQ(Result.Status, ExecutionStatus::LimitExceeded);
    EXPECT_EQ(Result.Limit, ExecutionLimitKind::CallDepth);
    EXPECT_EQ(Result.Statistics.MaximumCallDepth, 3U);
  }

  // Verifies even the entry activation is rejected when its deterministic abstract frame exceeds the stack budget.
  TEST(RuntimeWorldTest, RejectsEntryFrameBeyondStackBudget)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId Signature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", Signature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
    const ir::IrValueId Zero = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 0));
    Builder.createReturn(Entry.Block, Zero);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());
    ExecutionLimits Limits;
    Limits.MaxStackBytes = 0;

    const ExecutionResult Result = runWithoutArguments(Module, Main, Limits);

    EXPECT_EQ(Result.Status, ExecutionStatus::LimitExceeded);
    EXPECT_EQ(Result.Limit, ExecutionLimitKind::Stack);
    EXPECT_EQ(Result.Statistics.FuelConsumed, 0U);
  }

  // Verifies RuntimeWorld cannot execute a Closed capability verified for a different complete target key.
  TEST(RuntimeWorldTest, RejectsTargetKeyMismatchBeforeExecution)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId Signature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", Signature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
    const ir::IrValueId Zero = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 0));
    Builder.createReturn(Entry.Block, Zero);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());
    target::TargetKey OtherTarget = Module.targetKey();
    OtherTarget.Cpu += "-different";
    RuntimeWorld World(std::move(OtherTarget));

    const ExecutionResult Result = interpret(Module, Main, World, {});

    EXPECT_EQ(Result.Status, ExecutionStatus::InternalInvariantFailure);
    EXPECT_EQ(Result.Statistics.FuelConsumed, 0U);
  }

  // Verifies an unused external declaration does not prevent execution of a fully internal entry function.
  TEST(RuntimeWorldTest, AllowsUnusedExternalFunction)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId ExternalSignature = Builder.functionType({}, I32);
    Builder.addFunction("unused", ExternalSignature, ir::IrFunctionKind::External);
    const ir::IrTypeId MainSignature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", MainSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
    const ir::IrValueId FortyTwo = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 42));
    Builder.createReturn(Entry.Block, FortyTwo);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

    const ExecutionResult Result = runWithoutArguments(Module, Main);

    ASSERT_EQ(Result.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(Result.Value.has_value());
    EXPECT_EQ(Result.Value->bits(), 42U);
  }

  // Verifies selecting an external function as the entry reports an unsupported runtime capability before consuming fuel.
  TEST(RuntimeWorldTest, ReportsUnsupportedExternalEntry)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId Signature = Builder.functionType({}, I32);
    const ir::IrFunctionId External = Builder.addFunction("external_entry", Signature, ir::IrFunctionKind::External);
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

    const ExecutionResult Result = runWithoutArguments(Module, External);

    EXPECT_EQ(Result.Status, ExecutionStatus::Unsupported);
    EXPECT_EQ(Result.Statistics.FuelConsumed, 0U);
  }

  // Verifies an external direct call reports unsupported only when dispatch reaches the call operation.
  TEST(RuntimeWorldTest, ReportsUnsupportedExecutedExternalDirectCall)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId ExternalSignature = Builder.functionType({I32}, I32);
    const ir::IrFunctionId External = Builder.addFunction("external_identity", ExternalSignature, ir::IrFunctionKind::External);
    const ir::IrTypeId MainSignature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", MainSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
    const ir::IrValueId FortyTwo = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 42));
    const ir::IrBuiltOperation Call = Builder.createDirectCall(Entry.Block, External, {FortyTwo});
    Builder.createReturn(Entry.Block, Call.Results.front());
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

    const ExecutionResult Result = runWithoutArguments(Module, Main);

    EXPECT_EQ(Result.Status, ExecutionStatus::Unsupported);
    EXPECT_EQ(Result.Statistics.FuelConsumed, 2U);
  }

  // Verifies RuntimeWorld dispatches a bound external function and validates its scalar result before resuming the caller.
  TEST(RuntimeWorldTest, DispatchesBoundExternalDirectCall)
  {
    ir::IrBuilder Builder;
    const ir::IrTypeId I32 = Builder.integerType(32, ir::IrSignedness::Signed);
    const ir::IrTypeId ExternalSignature = Builder.functionType({}, I32);
    const ir::IrFunctionId External = Builder.addFunction("external_answer", ExternalSignature, ir::IrFunctionKind::External);
    const ir::IrTypeId MainSignature = Builder.functionType({}, I32);
    const ir::IrFunctionId Main = Builder.addFunction("main", MainSignature, ir::IrFunctionKind::Definition);
    const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
    const ir::IrBuiltOperation Call = Builder.createDirectCall(Entry.Block, External, {});
    Builder.createReturn(Entry.Block, Call.Results.front());
    const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());
    RuntimeWorld World(Module.targetKey());
    ASSERT_TRUE(World.bindExternalFunction("external_answer", [I32](const std::vector<RuntimeValue> &Arguments) -> std::optional<RuntimeValue>
    {
      EXPECT_TRUE(Arguments.empty());
      return RuntimeValue::fromBits(I32, 42);
    }));

    const ExecutionResult Result = interpret(Module, Main, World, {});

    ASSERT_EQ(Result.Status, ExecutionStatus::Returned);
    ASSERT_TRUE(Result.Value.has_value());
    EXPECT_EQ(Result.Value->bits(), 42U);
  }

  // Verifies all unsupported PDB division and remainder operations are rejected before dispatch instead of inheriting host behavior.
  TEST(RuntimeWorldTest, RejectsPdbArithmeticWithoutTargetRuleHandler)
  {
    constexpr ir::IrOpcode Opcodes[] = {
        ir::IrOpcode::IntSignedDiv,
        ir::IrOpcode::IntUnsignedDiv,
        ir::IrOpcode::IntSignedRem,
        ir::IrOpcode::IntUnsignedRem,
    };
    for (const ir::IrOpcode Opcode : Opcodes)
    {
      SCOPED_TRACE(ir::irOpcodeName(Opcode));
      ir::IrBuilder Builder;
      const ir::IrSignedness Signedness = Opcode == ir::IrOpcode::IntSignedDiv || Opcode == ir::IrOpcode::IntSignedRem ? ir::IrSignedness::Signed : ir::IrSignedness::Unsigned;
      const ir::IrTypeId I32 = Builder.integerType(32, Signedness);
      const ir::IrTypeId Signature = Builder.functionType({}, I32);
      const ir::IrFunctionId Main = Builder.addFunction("main", Signature, ir::IrFunctionKind::Definition);
      const ir::IrBuiltBlock Entry = Builder.addBlock(Main);
      const ir::IrValueId Four = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 4));
      const ir::IrValueId Two = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 2));
      const ir::IrValueId ResultValue = Builder.createIntegerBinary(Entry.Block, Opcode, Four, Two);
      Builder.createReturn(Entry.Block, ResultValue);
      const ir::VerifiedClosedModule Module = finishAndClose(Builder, hostTargetKey());

      const ExecutionResult Result = runWithoutArguments(Module, Main);

      EXPECT_EQ(Result.Status, ExecutionStatus::Unsupported);
      EXPECT_EQ(Result.Statistics.FuelConsumed, 0U);
    }
  }
} // namespace ink::execution
