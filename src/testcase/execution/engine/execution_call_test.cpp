#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <cstdint>

namespace ink::execution::test
{
  namespace
  {
    class CallContext final
    {
      public:
        explicit CallContext(ExecutionLimits Limits = {})
            : Context(Compilation),
              Builder(Context),
              Engine(Context, Limits),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true))
        {
        }

        const ir::IntegerConstant &integer(std::uint64_t Value)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Value));
        }

        std::unique_ptr<ir::Function> function(bool ReturnsVoid = false, ir::LanguageLinkage Linkage = ir::LanguageLinkage::Ink)
        {
          const ir::Type *Parameters[] = {&Int32};
          const ir::Type &ReturnType = ReturnsVoid ? Context.typePool().getType<ir::TypeKind::Void>() : static_cast<const ir::Type &>(Int32);
          const ir::FunctionType *Signature = Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern(Linkage == ir::LanguageLinkage::C ? "InkMissingExternalSymbol94c8f17e" : "F"), *Signature, {}, {}, ir::CallingConvention::C, Linkage) : nullptr;
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionEngine Engine;
        const ir::IntegerType &Int32;
    };
  } // namespace

  // Each invocation binds parameters in fresh storage, runs its body and retains its returned constant after cleanup.
  TEST(ExecutionCallTest, CreatesFreshParameterStorageAndPreservesReturnedSnapshots)
  {
    CallContext Test;
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    EXPECT_FALSE(Function->hasBody());
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    int CounterBinding = 0;
    const auto Counter = Test.Engine.allocate(*Module, &CounterBinding, Test.Int32, true, &Test.integer(0));
    ASSERT_TRUE(Counter);
    std::vector<ExecutionFrame *> Frames;
    std::vector<ExecutionPlace> Parameters;
    unsigned Executions = 0;
    const auto Body = [&](ExecutionFrame &Frame) -> ExecutionResult
    {
      ++Executions;
      Frames.push_back(&Frame);
      const auto Parameter = Test.Engine.lookup(Frame, Function->parameters().front().get());
      if (!Parameter)
      {
        return {Parameter.Status};
      }
      Parameters.push_back(Parameter.Place);
      const ExecutionResult Value = Test.Engine.load(Parameter.Place);
      if (!Value)
      {
        return Value;
      }
      const ExecutionStatus Updated = Test.Engine.store(Parameter.Place, Test.integer(99));
      if (Updated != ExecutionStatus::Success)
      {
        return {Updated};
      }
      const ExecutionStatus Stored = Test.Engine.store(Counter.Place, Test.integer(Executions));
      return {Stored, Stored == ExecutionStatus::Success ? Value.Value : nullptr};
    };
    const ir::Value *FirstArguments[] = {&Test.integer(11)};
    const ir::Value *SecondArguments[] = {&Test.integer(22)};
    const auto First = Test.Engine.call(*Function, *Module, FirstArguments, Body);
    const auto Second = Test.Engine.call(*Function, *Module, SecondArguments, Body);
    ASSERT_TRUE(First);
    ASSERT_TRUE(Second);
    EXPECT_EQ(First.Value, &Test.integer(11));
    EXPECT_EQ(Second.Value, &Test.integer(22));
    EXPECT_EQ(Executions, 2U);
    ASSERT_EQ(Frames.size(), 2U);
    ASSERT_EQ(Parameters.size(), 2U);
    EXPECT_NE(Frames[0], Frames[1]);
    EXPECT_NE(Parameters[0], Parameters[1]);
    EXPECT_EQ(Frames[0]->parent(), Module);
    EXPECT_EQ(Frames[0]->kind(), ExecutionFrameKind::Call);
    EXPECT_FALSE(Frames[0]->active());
    EXPECT_FALSE(Frames[1]->active());
    EXPECT_EQ(Test.Engine.load(Parameters[0]).Status, ExecutionStatus::ExpiredPlace);
    EXPECT_EQ(Test.Engine.load(Counter.Place).Value, &Test.integer(2));
  }

  // A call follows its definition environment and does not inherit unrelated caller-local bindings.
  TEST(ExecutionCallTest, UsesDefinitionEnvironmentForGlobalLookup)
  {
    CallContext Test;
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *Caller = Test.Engine.createFrame(ExecutionFrameKind::Call, Module);
    ASSERT_NE(Caller, nullptr);
    int GlobalBinding = 0;
    int LocalBinding = 0;
    ASSERT_TRUE(Test.Engine.allocate(*Module, &GlobalBinding, Test.Int32, true, &Test.integer(7)));
    ASSERT_TRUE(Test.Engine.allocate(*Caller, &LocalBinding, Test.Int32, true, &Test.integer(9)));
    const ir::Value *Arguments[] = {&Test.integer(1)};
    const auto Result = Test.Engine.call(*Function, *Module, Arguments, [&](ExecutionFrame &Frame) -> ExecutionResult
    {
      EXPECT_EQ(Test.Engine.lookup(Frame, &LocalBinding).Status, ExecutionStatus::UnknownBinding);
      const auto Global = Test.Engine.lookup(Frame, &GlobalBinding);
      return Global ? Test.Engine.load(Global.Place) : ExecutionResult{Global.Status};
    });
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value, &Test.integer(7));
    EXPECT_TRUE(Caller->active());
  }

  // Invalid arguments, foreign owners, missing external symbols and missing Ink callbacks never enter the function body.
  TEST(ExecutionCallTest, RejectsInvalidCallBoundariesBeforeExecutingBody)
  {
    CallContext Test;
    CallContext Other;
    auto Function = Test.function();
    auto ForeignFunction = Other.function();
    auto ExternalFunction = Test.function(false, ir::LanguageLinkage::C);
    ASSERT_NE(Function, nullptr);
    ASSERT_NE(ForeignFunction, nullptr);
    ASSERT_NE(ExternalFunction, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ExecutionFrame *ForeignModule = Other.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ASSERT_NE(ForeignModule, nullptr);
    unsigned Executions = 0;
    const auto Body = [&](ExecutionFrame &) -> ExecutionResult
    {
      ++Executions;
      return {ExecutionStatus::Success, &Test.integer(1)};
    };
    const ir::Value *Arguments[] = {&Test.integer(1)};
    const ir::Value *NullArguments[] = {nullptr};
    const ir::Value *WrongArguments[] = {&Test.Context.constantPool().getBoolConstant(true)};
    const ir::Value *ForeignArguments[] = {&Other.integer(1)};
    EXPECT_EQ(Test.Engine.call(*Function, *Module, {}, Body).Status, ExecutionStatus::InvalidArguments);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, NullArguments, Body).Status, ExecutionStatus::InvalidArguments);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, WrongArguments, Body).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, ForeignArguments, Body).Status, ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.call(*ForeignFunction, *Module, Arguments, Body).Status, ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.call(*Function, *ForeignModule, Arguments, Body).Status, ExecutionStatus::InvalidFrame);
    EXPECT_EQ(Test.Engine.call(*ExternalFunction, *Module, Arguments, Body).Status, ExecutionStatus::SymbolNotFound);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Arguments, {}).Status, ExecutionStatus::MissingBody);
    EXPECT_EQ(Executions, 0U);
  }

  // An unevaluated IR parameter passed as a Value is rejected before an Ink invocation creates observable effects.
  TEST(ExecutionCallTest, RejectsUnevaluatedValuesBeforeExecutingBody)
  {
    CallContext Test;
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    ASSERT_EQ(Function->parameters().size(), 1U);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    unsigned Executions = 0;
    const auto Body = [&](ExecutionFrame &) -> ExecutionResult
    {
      ++Executions;
      return {ExecutionStatus::Success, &Test.integer(7)};
    };
    const ir::Value *Arguments[] = {Function->parameters().front().get()};
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Arguments, Body).Status, ExecutionStatus::RuntimeValue);
    EXPECT_EQ(Executions, 0U);
    Arguments[0] = &Test.integer(3);
    const auto Result = Test.Engine.call(*Function, *Module, Arguments, Body);
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value, &Test.integer(7));
    EXPECT_EQ(Executions, 1U);
  }

  // Return values must match the function's exact type, belong to its context and be absent for void functions.
  TEST(ExecutionCallTest, ValidatesReturnedValuesAndVoidResults)
  {
    CallContext Test;
    CallContext Other;
    auto Function = Test.function();
    auto VoidFunction = Test.function(true);
    ASSERT_NE(Function, nullptr);
    ASSERT_NE(VoidFunction, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *Arguments[] = {&Test.integer(1)};
    const auto ReturnVoid = [](ExecutionFrame &) -> ExecutionResult
    {
      return {};
    };
    const auto ReturnInteger = [&](ExecutionFrame &) -> ExecutionResult
    {
      return {ExecutionStatus::Success, &Test.integer(1)};
    };
    const auto ReturnBool = [&](ExecutionFrame &) -> ExecutionResult
    {
      return {ExecutionStatus::Success, &Test.Context.constantPool().getBoolConstant(true)};
    };
    const auto ReturnForeign = [&](ExecutionFrame &) -> ExecutionResult
    {
      return {ExecutionStatus::Success, &Other.integer(1)};
    };
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Arguments, ReturnVoid).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Arguments, ReturnBool).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Arguments, ReturnForeign).Status, ExecutionStatus::ForeignContext);
    EXPECT_EQ(Test.Engine.call(*VoidFunction, *Module, Arguments, ReturnInteger).Status, ExecutionStatus::TypeMismatch);
    const auto Void = Test.Engine.call(*VoidFunction, *Module, Arguments, ReturnVoid);
    EXPECT_EQ(Void.Status, ExecutionStatus::Success);
    EXPECT_EQ(Void.Value, nullptr);
  }

  // A failed body keeps its explicit error after frame cleanup and does not leave parameter storage alive.
  TEST(ExecutionCallTest, ClosesCallFrameWithoutOverwritingBodyFailure)
  {
    CallContext Test;
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *Called = nullptr;
    const ir::Value *Arguments[] = {&Test.integer(1)};
    const auto Result = Test.Engine.call(*Function, *Module, Arguments, [&](ExecutionFrame &Frame) -> ExecutionResult
    {
      Called = &Frame;
      return {ExecutionStatus::DivisionByZero};
    });
    EXPECT_EQ(Result.Status, ExecutionStatus::DivisionByZero);
    EXPECT_EQ(Result.Value, nullptr);
    EXPECT_EQ(Test.Engine.lastStatus(), ExecutionStatus::DivisionByZero);
    ASSERT_NE(Called, nullptr);
    EXPECT_FALSE(Called->active());
    EXPECT_TRUE(Module->active());
  }

  // Recursive calls share the depth budget, release successful frames and cannot disguise budget failure as a valid result.
  TEST(ExecutionCallTest, RecursionSharesDepthBudgetAndUnwindsFrames)
  {
    ExecutionLimits Limits;
    Limits.MaxCallDepth = 2;
    CallContext Test(Limits);
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    std::vector<ExecutionFrame *> Frames;
    std::function<ExecutionResult(ExecutionFrame &)> Body;
    Body = [&](ExecutionFrame &Frame) -> ExecutionResult
    {
      Frames.push_back(&Frame);
      const auto Parameter = Test.Engine.lookup(Frame, Function->parameters().front().get());
      if (!Parameter)
      {
        return {Parameter.Status};
      }
      const auto Argument = Test.Engine.load(Parameter.Place);
      if (!Argument)
      {
        return Argument;
      }
      const auto Value = static_cast<const ir::IntegerConstant *>(Argument.Value)->value().words().front();
      if (Value)
      {
        const ir::Value *Next[] = {&Test.integer(Value - 1)};
        Test.Engine.call(*Function, *Module, Next, Body);
      }
      return {ExecutionStatus::Success, &Test.integer(7)};
    };
    const ir::Value *Finite[] = {&Test.integer(1)};
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Finite, Body).Value, &Test.integer(7));
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Finite, Body).Value, &Test.integer(7));
    const ir::Value *Excessive[] = {&Test.integer(2)};
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Excessive, Body).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.consumeStep(), ExecutionStatus::BudgetExceeded);
    for (const ExecutionFrame *Frame : Frames)
    {
      EXPECT_FALSE(Frame->active());
    }
  }

  // Cancellation inside a body takes priority over its returned success and still destroys the invocation's storage.
  TEST(ExecutionCallTest, CancellationOverridesCallbackSuccessAndCleansUp)
  {
    CallContext Test;
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    ExecutionFrame *Called = nullptr;
    const ir::Value *Arguments[] = {&Test.integer(1)};
    const auto Result = Test.Engine.call(*Function, *Module, Arguments, [&](ExecutionFrame &Frame) -> ExecutionResult
    {
      Called = &Frame;
      Test.Engine.cancel();
      return {ExecutionStatus::Success, &Test.integer(1)};
    });
    EXPECT_EQ(Result.Status, ExecutionStatus::Cancelled);
    EXPECT_EQ(Result.Value, nullptr);
    EXPECT_EQ(Test.Engine.lastStatus(), ExecutionStatus::Cancelled);
    ASSERT_NE(Called, nullptr);
    EXPECT_FALSE(Called->active());
  }

  // A body-reported budget failure stops later calls so partially executed side effects cannot be replayed.
  TEST(ExecutionCallTest, ReportedBudgetFailureStopsLaterCalls)
  {
    CallContext Test;
    auto Function = Test.function();
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    unsigned Executions = 0;
    const ir::Value *Arguments[] = {&Test.integer(1)};
    const auto Body = [&](ExecutionFrame &) -> ExecutionResult
    {
      ++Executions;
      return {ExecutionStatus::BudgetExceeded};
    };
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Arguments, Body).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Arguments, Body).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Executions, 1U);
  }
} // namespace ink::execution::test
