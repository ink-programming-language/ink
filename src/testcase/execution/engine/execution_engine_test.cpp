#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/context.h"
#include "ink/tokenizer/token.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>

namespace ink::execution::test
{
  namespace
  {
    class TestContext final
    {
      public:
        explicit TestContext(ExecutionLimits Limits = {})
            : Context(Compilation),
              Engine(Context, Limits),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true))
        {
        }

        const ir::IntegerConstant &integer(std::uint64_t Bits)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Bits));
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ExecutionEngine Engine;
        const ir::IntegerType &Int32;
    };
  } // namespace

  // Reading a mutable module object observes preceding writes without changing an earlier constant snapshot.
  TEST(ExecutionEngineTest, ModuleWritesAreVisibleInOrderAndPreserveSnapshots)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Binding = 0;
    const auto Object = Test.Engine.allocate(*Module, &Binding, Test.Int32, true, &Test.integer(1));
    ASSERT_TRUE(Object.succeeded());
    const ExecutionResult Before = Test.Engine.load(Object.Place);
    ASSERT_TRUE(Before.succeeded());
    EXPECT_EQ(Before.Value, &Test.integer(1));
    ASSERT_EQ(Test.Engine.store(Object.Place, Test.integer(10)), ExecutionStatus::Success);
    const ExecutionResult After = Test.Engine.load(Object.Place);
    ASSERT_TRUE(After.succeeded());
    EXPECT_EQ(After.Value, &Test.integer(10));
    EXPECT_EQ(Before.Value, &Test.integer(1));
  }

  // Repeated semantic requests for one initializer return its saved result and do not increment the module twice.
  TEST(ExecutionEngineTest, RepeatedEventCommitsItsSideEffectOnce)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int CounterBinding = 0;
    int Initializer = 0;
    const auto Counter = Test.Engine.allocate(*Module, &CounterBinding, Test.Int32, true, &Test.integer(0));
    ASSERT_TRUE(Counter.succeeded());
    unsigned Executions = 0;
    auto Increment = [&]() -> ExecutionResult
    {
      ++Executions;
      const ExecutionResult Current = Test.Engine.load(Counter.Place);
      if (!Current.succeeded())
      {
        return Current;
      }
      const ExecutionResult Next = Test.Engine.evaluateBinary(tokenizer::TokenKind::Plus, *Current.Value, Test.integer(1));
      if (!Next.succeeded())
      {
        return Next;
      }
      const ExecutionStatus Stored = Test.Engine.store(Counter.Place, *Next.Value);
      return {Stored, Stored == ExecutionStatus::Success ? Next.Value : nullptr};
    };
    const ExecutionResult First = Test.Engine.executeOnce(*Module, &Initializer, Increment);
    ASSERT_TRUE(First.succeeded());
    ASSERT_EQ(Test.Engine.store(Counter.Place, Test.integer(10)), ExecutionStatus::Success);
    const ExecutionResult Repeated = Test.Engine.executeOnce(*Module, &Initializer, Increment);
    ASSERT_TRUE(Repeated.succeeded());
    EXPECT_EQ(Executions, 1U);
    EXPECT_EQ(First.Value, &Test.integer(1));
    EXPECT_EQ(Repeated.Value, First.Value);
    EXPECT_EQ(Test.Engine.load(Counter.Place).Value, &Test.integer(10));
    int LaterInitializer = 0;
    const ExecutionResult Later = Test.Engine.executeOnce(*Module, &LaterInitializer, Increment);
    ASSERT_TRUE(Later.succeeded());
    EXPECT_EQ(Later.Value, &Test.integer(11));
    EXPECT_EQ(Test.Engine.load(Counter.Place).Value, &Test.integer(11));
    EXPECT_EQ(Executions, 2U);
  }

  // A new call or iteration frame executes the same source event again with independent completion state.
  TEST(ExecutionEngineTest, SameEventRunsAgainInANewFrame)
  {
    ExecutionLimits Limits;
    Limits.MaxCallDepth = 1;
    TestContext Test(Limits);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Event = 0;
    unsigned Executions = 0;
    auto Evaluate = [&]() -> ExecutionResult
    {
      ++Executions;
      return {ExecutionStatus::Success, &Test.integer(Executions)};
    };
    ExecutionFrame *First = Test.Engine.createFrame(ExecutionFrameKind::Call, Module);
    ASSERT_NE(First, nullptr);
    EXPECT_EQ(Test.Engine.executeOnce(*First, &Event, Evaluate).Value, &Test.integer(1));
    ASSERT_EQ(Test.Engine.endFrame(*First), ExecutionStatus::Success);
    ExecutionFrame *Second = Test.Engine.createFrame(ExecutionFrameKind::Call, Module);
    ASSERT_NE(Second, nullptr);
    EXPECT_EQ(Test.Engine.executeOnce(*Second, &Event, Evaluate).Value, &Test.integer(2));
    EXPECT_EQ(Executions, 2U);
  }

  // A failed initializer is cached without replaying side effects or reporting diagnostics at the execution layer.
  TEST(ExecutionEngineTest, FailedEventDoesNotRepeatPartialSideEffects)
  {
    TestContext Test;
    core::CollectingDiagnosticConsumer Diagnostics;
    Test.Compilation.diagnosticEngine().addConsumer(Diagnostics);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Event = 0;
    unsigned Executions = 0;
    auto Fail = [&]() -> ExecutionResult
    {
      ++Executions;
      return {ExecutionStatus::DivisionByZero, nullptr};
    };
    bool ReusedResult = true;
    EXPECT_EQ(Test.Engine.executeOnce(*Module, &Event, Fail, &ReusedResult).Status, ExecutionStatus::DivisionByZero);
    EXPECT_FALSE(ReusedResult);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_EQ(Test.Engine.executeOnce(*Module, &Event, Fail, &ReusedResult).Status, ExecutionStatus::DivisionByZero);
    EXPECT_TRUE(ReusedResult);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_EQ(Executions, 1U);
  }

  // Reentering an unfinished event reports a dependency cycle instead of recursively rerunning its callback.
  TEST(ExecutionEngineTest, RecursiveEventReportsCycleWithoutReentry)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Event = 0;
    unsigned NestedExecutions = 0;
    auto Nested = [&]() -> ExecutionResult
    {
      ++NestedExecutions;
      return {ExecutionStatus::Success, &Test.integer(1)};
    };
    auto Outer = [&]() -> ExecutionResult
    {
      bool ReusedResult = true;
      const ExecutionResult Result = Test.Engine.executeOnce(*Module, &Event, Nested, &ReusedResult);
      EXPECT_FALSE(ReusedResult);
      return Result;
    };
    EXPECT_EQ(Test.Engine.executeOnce(*Module, &Event, Outer).Status, ExecutionStatus::RecursiveEvent);
    EXPECT_EQ(NestedExecutions, 0U);
  }

  // A runtime local stays unavailable to compilation even when another compile-time expression produced its initializer.
  TEST(ExecutionEngineTest, RuntimeBindingCannotBeReadAsACompileTimeValue)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *Function = Test.Engine.createFrame(ExecutionFrameKind::Analysis, Module);
    ASSERT_NE(Function, nullptr);
    int Binding = 0;
    const auto Runtime = Test.Engine.bindRuntime(*Function, &Binding, Test.Int32);
    ASSERT_TRUE(Runtime.succeeded());
    const auto Found = Test.Engine.lookup(*Function, &Binding);
    ASSERT_TRUE(Found.succeeded());
    EXPECT_EQ(Found.Place, Runtime.Place);
    const ExecutionResult Loaded = Test.Engine.load(Found.Place);
    EXPECT_EQ(Loaded.Status, ExecutionStatus::RuntimeValue);
    EXPECT_EQ(Loaded.Value, nullptr);
  }

  // Function-local storage expires on scope exit while inherited module storage remains writable and readable.
  TEST(ExecutionEngineTest, EndingLocalFramePreservesModuleStorage)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *Function = Test.Engine.createFrame(ExecutionFrameKind::Analysis, Module);
    ASSERT_NE(Function, nullptr);
    int GlobalBinding = 0;
    int LocalBinding = 0;
    const auto Global = Test.Engine.allocate(*Module, &GlobalBinding, Test.Int32, true, &Test.integer(1));
    const auto Local = Test.Engine.allocate(*Function, &LocalBinding, Test.Int32, true, &Test.integer(2));
    ASSERT_TRUE(Global.succeeded());
    ASSERT_TRUE(Local.succeeded());
    EXPECT_EQ(Test.Engine.lookup(*Function, &GlobalBinding).Place, Global.Place);
    ASSERT_EQ(Test.Engine.endFrame(*Function), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.load(Local.Place).Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Test.Engine.store(Local.Place, Test.integer(3)), ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Test.Engine.allocate(*Function, &LocalBinding, Test.Int32).Status, ExecutionStatus::InvalidFrame);
    ASSERT_EQ(Test.Engine.store(Global.Place, Test.integer(10)), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.load(Global.Place).Value, &Test.integer(10));
    ASSERT_EQ(Test.Engine.endFrame(*Module), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.load(Global.Place).Value, &Test.integer(10));
  }

  // Ending a parent environment invalidates its nested blocks without requiring separate cleanup calls.
  TEST(ExecutionEngineTest, EndingParentExpiresNestedLocalStorage)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *Function = Test.Engine.createFrame(ExecutionFrameKind::Analysis, Module);
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Block = Test.Engine.createFrame(ExecutionFrameKind::Block, Function);
    ASSERT_NE(Block, nullptr);
    int Binding = 0;
    const auto Local = Test.Engine.allocate(*Block, &Binding, Test.Int32, true, &Test.integer(2));
    ASSERT_TRUE(Local.succeeded());
    ASSERT_EQ(Test.Engine.endFrame(*Function), ExecutionStatus::Success);
    EXPECT_FALSE(Function->active());
    EXPECT_FALSE(Block->active());
    EXPECT_EQ(Test.Engine.load(Local.Place).Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Test.Engine.lookup(*Block, &Binding).Status, ExecutionStatus::InvalidFrame);
    EXPECT_EQ(Test.Engine.createFrame(ExecutionFrameKind::Block, Block), nullptr);
    EXPECT_EQ(Test.Engine.lastStatus(), ExecutionStatus::InvalidFrame);
  }

  // A call follows its supplied definition environment and cannot see an unrelated caller's local binding.
  TEST(ExecutionEngineTest, LookupFollowsTheDefinitionEnvironment)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *Caller = Test.Engine.createFrame(ExecutionFrameKind::Call, Module);
    ExecutionFrame *Callee = Test.Engine.createFrame(ExecutionFrameKind::Call, Module);
    ASSERT_NE(Caller, nullptr);
    ASSERT_NE(Callee, nullptr);
    int SharedBinding = 0;
    int CallerBinding = 0;
    const auto Global = Test.Engine.allocate(*Module, &SharedBinding, Test.Int32, true, &Test.integer(1));
    const auto Local = Test.Engine.allocate(*Caller, &CallerBinding, Test.Int32, true, &Test.integer(2));
    ASSERT_TRUE(Global.succeeded());
    ASSERT_TRUE(Local.succeeded());
    EXPECT_EQ(Test.Engine.lookup(*Callee, &SharedBinding).Place, Global.Place);
    EXPECT_EQ(Test.Engine.lookup(*Callee, &CallerBinding).Status, ExecutionStatus::UnknownBinding);
  }

  // Reading an uninitialized object fails explicitly and a successful write makes its value available.
  TEST(ExecutionEngineTest, UninitializedObjectBecomesReadableAfterStore)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Binding = 0;
    const auto Object = Test.Engine.allocate(*Module, &Binding, Test.Int32);
    ASSERT_TRUE(Object.succeeded());
    EXPECT_EQ(Test.Engine.load(Object.Place).Status, ExecutionStatus::Uninitialized);
    ASSERT_EQ(Test.Engine.store(Object.Place, Test.integer(7)), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.load(Object.Place).Value, &Test.integer(7));
  }

  // A rejected write to an immutable object leaves its original value intact.
  TEST(ExecutionEngineTest, ReadOnlyObjectRejectsMutation)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Binding = 0;
    const auto Object = Test.Engine.allocate(*Module, &Binding, Test.Int32, false, &Test.integer(1));
    ASSERT_TRUE(Object.succeeded());
    EXPECT_EQ(Test.Engine.store(Object.Place, Test.integer(2)), ExecutionStatus::ReadOnly);
    EXPECT_EQ(Test.Engine.load(Object.Place).Value, &Test.integer(1));
  }

  // An immutable declaration can receive its initial value once after its storage has been allocated.
  TEST(ExecutionEngineTest, ReadOnlyObjectAllowsItsFirstInitialization)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Binding = 0;
    const auto Object = Test.Engine.allocate(*Module, &Binding, Test.Int32, false);
    ASSERT_TRUE(Object.succeeded());
    EXPECT_EQ(Test.Engine.load(Object.Place).Status, ExecutionStatus::Uninitialized);
    ASSERT_EQ(Test.Engine.store(Object.Place, Test.integer(1)), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.store(Object.Place, Test.integer(2)), ExecutionStatus::ReadOnly);
    EXPECT_EQ(Test.Engine.load(Object.Place).Value, &Test.integer(1));
  }

  // Invalid binding identities and duplicate declarations cannot replace the first variable's storage.
  TEST(ExecutionEngineTest, InvalidAndDuplicateBindingsDoNotReplaceObjects)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Binding = 0;
    const auto Original = Test.Engine.allocate(*Module, &Binding, Test.Int32, true, &Test.integer(1));
    ASSERT_TRUE(Original.succeeded());
    EXPECT_EQ(Test.Engine.allocate(*Module, nullptr, Test.Int32).Status, ExecutionStatus::InvalidBinding);
    EXPECT_EQ(Test.Engine.allocate(*Module, &Binding, Test.Int32, true, &Test.integer(2)).Status, ExecutionStatus::DuplicateBinding);
    EXPECT_EQ(Test.Engine.lookup(*Module, &Binding).Place, Original.Place);
    EXPECT_EQ(Test.Engine.load(Original.Place).Value, &Test.integer(1));
  }

  // Values, types, frames and addresses owned by another execution environment are rejected at their boundaries.
  TEST(ExecutionEngineTest, RejectsForeignContextsFramesAndPlaces)
  {
    TestContext Test;
    TestContext Other;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Binding = 0;
    int ForeignTypeBinding = 0;
    int ForeignValueBinding = 0;
    const auto Object = Test.Engine.allocate(*Module, &Binding, Test.Int32, true, &Test.integer(1));
    ASSERT_TRUE(Object.succeeded());
    EXPECT_EQ(Other.Engine.allocate(*Module, &Binding, Other.Int32).Status, ExecutionStatus::InvalidFrame);
    EXPECT_EQ(Other.Engine.load(Object.Place).Status, ExecutionStatus::InvalidPlace);
    EXPECT_EQ(Test.Engine.load(ExecutionPlace()).Status, ExecutionStatus::InvalidPlace);
    EXPECT_EQ(Test.Engine.allocate(*Module, &ForeignTypeBinding, Other.Int32).Status, ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.allocate(*Module, &ForeignValueBinding, Test.Int32, true, &Other.integer(2)).Status, ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.store(Object.Place, Other.integer(2)), ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.load(Object.Place).Value, &Test.integer(1));
  }

  // A type-invalid assignment cannot overwrite a valid integer object's current value.
  TEST(ExecutionEngineTest, TypeMismatchPreservesThePreviousValue)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Binding = 0;
    const auto Object = Test.Engine.allocate(*Module, &Binding, Test.Int32, true, &Test.integer(1));
    ASSERT_TRUE(Object.succeeded());
    EXPECT_EQ(Test.Engine.store(Object.Place, Test.Context.constantPool().getBoolConstant(true)), ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.load(Object.Place).Value, &Test.integer(1));
  }

  // All work shares a finite step budget and exceeding it returns a recoverable execution status.
  TEST(ExecutionEngineTest, StepBudgetStopsFurtherExecution)
  {
    ExecutionLimits Limits;
    Limits.MaxSteps = 2;
    TestContext Test(Limits);
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::BudgetExceeded);
  }

  // Leaving a successful AST traversal releases its shared depth slot for the next sibling traversal.
  TEST(ExecutionEngineTest, EvaluationDepthIsReleasedAfterLeaving)
  {
    ExecutionLimits Limits;
    Limits.MaxEvaluationDepth = 2;
    TestContext Test(Limits);
    ASSERT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::Success);
    ASSERT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::Success);
    Test.Engine.leaveEvaluation();
    ASSERT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::Success);
    Test.Engine.leaveEvaluation();
    Test.Engine.leaveEvaluation();
    ASSERT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::Success);
    ASSERT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::Success);
    Test.Engine.leaveEvaluation();
    Test.Engine.leaveEvaluation();
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::Success);
  }

  // Exceeding combined AST traversal depth stops the engine even after outstanding traversals unwind.
  TEST(ExecutionEngineTest, EvaluationDepthLimitRemainsTerminalAfterCleanup)
  {
    ExecutionLimits Limits;
    Limits.MaxEvaluationDepth = 2;
    TestContext Test(Limits);
    ASSERT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::Success);
    ASSERT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::BudgetExceeded);
    Test.Engine.leaveEvaluation();
    Test.Engine.leaveEvaluation();
    EXPECT_EQ(Test.Engine.lastStatus(), ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.enterEvaluation(), ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::BudgetExceeded);
  }

  // Cancellation prevents an event callback from starting or committing any side effect.
  TEST(ExecutionEngineTest, CancellationPreventsEventExecution)
  {
    TestContext Test;
    core::CollectingDiagnosticConsumer Diagnostics;
    Test.Compilation.diagnosticEngine().addConsumer(Diagnostics);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Event = 0;
    unsigned Executions = 0;
    auto Evaluate = [&]() -> ExecutionResult
    {
      ++Executions;
      return {ExecutionStatus::Success, &Test.integer(1)};
    };
    Test.Engine.cancel();
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::Cancelled);
    EXPECT_EQ(Test.Engine.executeOnce(*Module, &Event, Evaluate).Status, ExecutionStatus::Cancelled);
    EXPECT_EQ(Executions, 0U);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // A callback cannot hide cancellation by returning success after the engine has been stopped.
  TEST(ExecutionEngineTest, CancellationDuringEventOverridesItsReturnedValue)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Event = 0;
    unsigned Executions = 0;
    auto Evaluate = [&]() -> ExecutionResult
    {
      ++Executions;
      Test.Engine.cancel();
      return {ExecutionStatus::Success, &Test.integer(1)};
    };
    const ExecutionResult Result = Test.Engine.executeOnce(*Module, &Event, Evaluate);
    EXPECT_EQ(Result.Status, ExecutionStatus::Cancelled);
    EXPECT_EQ(Result.Value, nullptr);
    EXPECT_EQ(Test.Engine.executeOnce(*Module, &Event, Evaluate).Status, ExecutionStatus::Cancelled);
    EXPECT_EQ(Executions, 1U);
  }

  // Recursive calls sharing a module definition environment still count against the dynamic call-depth budget.
  TEST(ExecutionEngineTest, CallDepthBudgetBoundsRecursion)
  {
    ExecutionLimits Limits;
    Limits.MaxCallDepth = 2;
    TestContext Test(Limits);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *First = Test.Engine.createFrame(ExecutionFrameKind::Call, Module);
    ASSERT_NE(First, nullptr);
    ExecutionFrame *Second = Test.Engine.createFrame(ExecutionFrameKind::Call, Module);
    ASSERT_NE(Second, nullptr);
    EXPECT_EQ(Test.Engine.createFrame(ExecutionFrameKind::Call, Module), nullptr);
    EXPECT_EQ(Test.Engine.lastStatus(), ExecutionStatus::BudgetExceeded);
  }

  // Exhausting object storage stops subsequent evaluation while allowing local frame cleanup.
  TEST(ExecutionEngineTest, ObjectBudgetStopsExecutionAndAllowsCleanup)
  {
    ExecutionLimits Limits;
    Limits.MaxObjects = 1;
    TestContext Test(Limits);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *Call = Test.Engine.createFrame(ExecutionFrameKind::Call, Module);
    ASSERT_NE(Call, nullptr);
    int FirstBinding = 0;
    int SecondBinding = 0;
    const auto First = Test.Engine.allocate(*Call, &FirstBinding, Test.Int32, true, &Test.integer(1));
    ASSERT_TRUE(First.succeeded());
    const ExecutionResult Snapshot = Test.Engine.load(First.Place);
    ASSERT_TRUE(Snapshot.succeeded());
    EXPECT_EQ(Test.Engine.allocate(*Call, &SecondBinding, Test.Int32).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.load(First.Place).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.store(First.Place, Test.integer(2)), ExecutionStatus::BudgetExceeded);
    ASSERT_EQ(Test.Engine.endFrame(*Call), ExecutionStatus::Success);
    EXPECT_FALSE(Call->active());
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Snapshot.Value, &Test.integer(1));
  }

  // A callback that exhausts a budget cannot replay already performed side effects on a later request.
  TEST(ExecutionEngineTest, BudgetFailurePreventsEventReplay)
  {
    TestContext Test;
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Event = 0;
    unsigned Executions = 0;
    auto Exhaust = [&]() -> ExecutionResult
    {
      ++Executions;
      return {ExecutionStatus::BudgetExceeded, nullptr};
    };
    EXPECT_EQ(Test.Engine.executeOnce(*Module, &Event, Exhaust).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.executeOnce(*Module, &Event, Exhaust).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Executions, 1U);
  }

  // An exhausted engine rejects an event result even when its callback accidentally returns success.
  TEST(ExecutionEngineTest, ExhaustedStepBudgetOverridesCallbackSuccess)
  {
    ExecutionLimits Limits;
    Limits.MaxSteps = 2;
    TestContext Test(Limits);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int Event = 0;
    auto Evaluate = [&]() -> ExecutionResult
    {
      EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::BudgetExceeded);
      return {ExecutionStatus::Success, &Test.integer(1)};
    };
    const ExecutionResult Result = Test.Engine.executeOnce(*Module, &Event, Evaluate);
    EXPECT_EQ(Result.Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Result.Value, nullptr);
  }

  // Boolean operators produce canonical boolean constants and reject integer-only arithmetic on bool values.
  TEST(ExecutionEngineTest, BooleanOperationsPreserveBooleanType)
  {
    TestContext Test;
    const ir::BoolConstant &True = Test.Context.constantPool().getBoolConstant(true);
    const ir::BoolConstant &False = Test.Context.constantPool().getBoolConstant(false);
    EXPECT_EQ(Test.Engine.evaluateUnary(tokenizer::TokenKind::Bang, True).Value, &False);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::AmpAmp, True, False).Value, &False);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::PipePipe, True, False).Value, &True);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::EqualEqual, True, False).Value, &False);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::BangEqual, True, False).Value, &True);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Plus, True, False).Status, ExecutionStatus::UnsupportedOperation);
  }

  // Signed arithmetic wraps at the target width, including the minimum signed value divided by minus one.
  TEST(ExecutionEngineTest, SignedIntegerArithmeticUsesTargetWidth)
  {
    TestContext Test;
    const ir::IntegerConstant &Maximum = Test.integer(0x7fffffff);
    const ir::IntegerConstant &Minimum = Test.integer(0x80000000);
    const ir::IntegerConstant &MinusOne = Test.integer(0xffffffff);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Plus, Maximum, Test.integer(1)).Value, &Minimum);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Minus, Minimum, Test.integer(1)).Value, &Maximum);
    EXPECT_EQ(Test.Engine.evaluateUnary(tokenizer::TokenKind::Minus, Minimum).Value, &Minimum);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Slash, Minimum, MinusOne).Value, &Minimum);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Percent, Minimum, MinusOne).Value, &Test.integer(0));
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Less, MinusOne, Test.integer(1)).Value, &Test.Context.constantPool().getBoolConstant(true));
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::ShiftRight, Minimum, Test.integer(31)).Value, &MinusOne);
  }

  // Multiplication and carry propagation retain upper words and wrap at 128 bits rather than the host width.
  TEST(ExecutionEngineTest, WideIntegerArithmeticPreservesUpperWords)
  {
    TestContext Test;
    const ir::IntegerType *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(128, false);
    ASSERT_NE(Wide, nullptr);
    constexpr std::uint64_t InputWords[] = {
        1,
        1,
    };
    constexpr std::uint64_t ProductWords[] = {
        1,
        2,
    };
    constexpr std::uint64_t MaximumWords[] = {
        std::numeric_limits<std::uint64_t>::max(),
        std::numeric_limits<std::uint64_t>::max(),
    };
    const ir::IntegerConstant *Input = Test.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, InputWords));
    const ir::IntegerConstant *ExpectedProduct = Test.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, ProductWords));
    const ir::IntegerConstant *Maximum = Test.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, MaximumWords));
    const ir::IntegerConstant *One = Test.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, 1));
    const ir::IntegerConstant *Zero = Test.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, 0));
    ASSERT_NE(Input, nullptr);
    ASSERT_NE(ExpectedProduct, nullptr);
    ASSERT_NE(Maximum, nullptr);
    ASSERT_NE(One, nullptr);
    ASSERT_NE(Zero, nullptr);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Star, *Input, *Input).Value, ExpectedProduct);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Plus, *Maximum, *One).Value, Zero);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Slash, *ExpectedProduct, *Input).Value, One);
  }

  // Zero divisors and shifts outside the operand width return failures instead of invoking host undefined behavior.
  TEST(ExecutionEngineTest, DivisionAndShiftErrorsReturnExplicitStatus)
  {
    TestContext Test;
    const ir::IntegerConstant &One = Test.integer(1);
    const ir::IntegerConstant &Zero = Test.integer(0);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Slash, One, Zero).Status, ExecutionStatus::DivisionByZero);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Percent, One, Zero).Status, ExecutionStatus::DivisionByZero);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::ShiftLeft, One, Test.integer(32)).Status, ExecutionStatus::InvalidShift);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::ShiftRight, One, Test.integer(0xffffffff)).Status, ExecutionStatus::InvalidShift);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::ShiftLeft, One, Test.integer(31)).Value, &Test.integer(0x80000000));
  }

  // Arithmetic cannot silently convert a foreign constant or mix integer types with different signedness.
  TEST(ExecutionEngineTest, ArithmeticRejectsForeignAndMismatchedOperands)
  {
    TestContext Test;
    TestContext Other;
    const ir::IntegerType *Unsigned = Test.Context.typePool().getType<ir::TypeKind::Integer>(32, false);
    ASSERT_NE(Unsigned, nullptr);
    const ir::IntegerConstant *UnsignedOne = Test.Context.constantPool().getIntegerConstant(*Unsigned, ir::IntegerBits(32, 1));
    ASSERT_NE(UnsignedOne, nullptr);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Plus, Test.integer(1), Other.integer(1)).Status, ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.evaluateUnary(tokenizer::TokenKind::Minus, Other.integer(1)).Status, ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.evaluateBinary(tokenizer::TokenKind::Plus, Test.integer(1), *UnsignedOne).Status, ExecutionStatus::TypeMismatch);
  }
} // namespace ink::execution::test
