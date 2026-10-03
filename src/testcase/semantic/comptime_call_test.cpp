#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "../core/environment_test_support.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/parser/parser.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::semantic::test
{
  using namespace ink::ir;

  namespace
  {
    class CallAnalysis final
    {
      public:
        explicit CallAnalysis(std::string_view Source)
            : Frontend(Compilation),
              Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source)))),
              Context(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        Module *analyze()
        {
          return Analyzer{}.analyze(Context, Parsed);
        }

        const Function *function(Module &Owner, std::string_view Name)
        {
          const auto *Binding = NameResolver(Context).lookupMember(Owner, Context.namePool().find(Name));
          if (!Binding || Binding->targets().size() != 1 || !Function::classof(Binding->targets().front()))
          {
            return nullptr;
          }
          return static_cast<const Function *>(Binding->targets().front());
        }

        const Value *returnedValue(Module &Owner, std::string_view Name)
        {
          const Function *Target = function(Owner, Name);
          if (!Target || !Target->entryBlock())
          {
            return nullptr;
          }
          for (const auto &Instruction : Target->entryBlock()->values())
          {
            if (ReturnInstruction::classof(Instruction.get()))
            {
              return static_cast<const ReturnInstruction &>(*Instruction).returnedValue();
            }
          }
          return nullptr;
        }

        void expectIntegerReturn(Module &Owner, std::string_view Name, std::uint64_t Expected, std::uint32_t Width = 32)
        {
          SCOPED_TRACE(Name);
          const Value *Returned = returnedValue(Owner, Name);
          ASSERT_TRUE(IntegerConstant::classof(Returned));
          const auto &Bits = static_cast<const IntegerConstant &>(*Returned).value();
          EXPECT_EQ(Bits.bitWidth(), Width);
          ASSERT_EQ(Bits.words().size(), 1U);
          EXPECT_EQ(Bits.words().front(), Expected);
        }

        std::size_t diagnosticCount(core::DiagnosticKind Kind) const
        {
          const auto &Entries = Diagnostics.diagnostics();
          return static_cast<std::size_t>(std::count_if(Entries.begin(), Entries.end(), [Kind](const core::Diagnostic &Entry)
          {
            return Entry.Kind == Kind;
          }));
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };
  } // namespace

  // Ordinary functions retain runtime bodies while nested compile-time calls use fresh arguments and locals each time.
  TEST(SemanticComptimeCallTest, EvaluatesNestedOrdinaryCallsWithIndependentLocals)
  {
    CallAnalysis Input(R"ink(
func AddOne(X: i32): i32
{
    var Local: i32 = X;
    Local = Local + 1;
    return Local;
}
func AddTwo(X: i32): i32
{
    return AddOne(AddOne(X));
}
func First(): i32 { return comptime AddTwo(1); }
func Second(): i32 { return comptime AddTwo(10); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "First", 3);
    Input.expectIntegerReturn(*Result, "Second", 12);
    const Function *AddTwo = Input.function(*Result, "AddTwo");
    ASSERT_NE(AddTwo, nullptr);
    EXPECT_TRUE(AddTwo->hasBody());
  }

  // Both ordinary and compile-time calls execute constants captured while lowering the definition.
  TEST(SemanticComptimeCallTest, ExecutesStoredIrAfterModuleStateChanges)
  {
    CallAnalysis Input(R"ink(
comptime var Counter: i32 = 1;
func Ordinary(): i32 { return Counter; }
comptime func CompileTime(): i32 { return Counter; }
comptime { Counter = 10; }
func ReadOrdinary(): i32 { return comptime Ordinary(); }
func ReadCompileTime(): i32 { return comptime CompileTime(); }
func ReadCurrent(): i32 { return comptime Counter; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "ReadOrdinary", 1);
    Input.expectIntegerReturn(*Result, "ReadCompileTime", 1);
    Input.expectIntegerReturn(*Result, "ReadCurrent", 10);
    const Function *CompileTime = Input.function(*Result, "CompileTime");
    ASSERT_NE(CompileTime, nullptr);
    EXPECT_TRUE(CompileTime->hasBody());
  }

  // A non-generic compile-time function owns executable IR before its first call.
  TEST(SemanticComptimeCallTest, LowersCompileTimeFunctionBeforeAnyCall)
  {
    CallAnalysis Input("comptime func AddOne(X: i32): i32 { return X + 1; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *AddOne = Input.function(*Result, "AddOne");
    ASSERT_NE(AddOne, nullptr);
    ASSERT_TRUE(AddOne->hasBody());
    EXPECT_TRUE(AddInstruction::classof(Input.returnedValue(*Result, "AddOne")));
    execution::ExecutionEngine Engine(Input.Context.irContext());
    const auto &Type = static_cast<const IntegerType &>(*AddOne->functionType().parameterTypes().front());
    const execution::ExecutionValueRef Arguments[] = {Engine.heap().integer(Type, execution::ExecutionInteger(32, 10))};
    const auto Executed = Engine.execute(*AddOne, Arguments);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.integer().bits().words().front(), 11U);
  }

  // Mutating a local does not change the caller's scalar argument or the next call's local storage.
  TEST(SemanticComptimeCallTest, IsolatesLocalStorageAcrossCompileTimeCalls)
  {
    CallAnalysis Input(R"ink(
comptime func Alter(X: i32): i32
{
    var Local: i32 = X;
    Local = Local + 10;
    return Local + X;
}
comptime var Argument: i32 = 3;
comptime var First: i32 = Alter(Argument);
comptime var Second: i32 = Alter(Argument);
func ReadFirst(): i32 { return comptime First; }
func ReadSecond(): i32 { return comptime Second; }
func ReadArgument(): i32 { return comptime Argument; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "ReadFirst", 16);
    Input.expectIntegerReturn(*Result, "ReadSecond", 16);
    Input.expectIntegerReturn(*Result, "ReadArgument", 3);
  }

  // Each argument executes once in source order, including an unused second argument, before entering the stored IR body.
  TEST(SemanticComptimeCallTest, EvaluatesArgumentsOnceInSourceOrder)
  {
    CallAnalysis Input(R"ink(
comptime var Counter: i32 = 0;
comptime func Pair(First: i32, Second: i32): i32 { return First; }
comptime var First: i32 = Pair(++Counter, ++Counter);
comptime var Second: i32 = Pair(++Counter, ++Counter);
func ReadFirst(): i32 { return comptime First; }
func ReadSecond(): i32 { return comptime Second; }
func ReadCounter(): i32 { return comptime Counter; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "ReadFirst", 1);
    Input.expectIntegerReturn(*Result, "ReadSecond", 3);
    Input.expectIntegerReturn(*Result, "ReadCounter", 4);
  }

  // Compile-time functions reject loops and unsupported arithmetic until the corresponding IR is supported.
  TEST(SemanticComptimeCallTest, RejectsControlFlowWithoutIrSupportAtDefinition)
  {
    const char *Cases[] = {
        "comptime func F(Flag: bool): void { while (Flag) {} }",
        "comptime func F(X: i32): void { for (var I: i32 = 0; I < X; I++) {} }",
        "comptime func F(N: i32): i32 { if (N <= 1) { return 1; } return N * F(N - 1); }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      CallAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-S0012\\]");
    }
  }

  // Unsupported ordinary IR operators are diagnosed even when the compile-time function is never called.
  TEST(SemanticComptimeCallTest, RejectsOperatorsWithoutIrSupportAtDefinition)
  {
    const char *Cases[] = {
        "comptime func F(X: i32): i32 { return X * 10; }",
        "comptime func F(X: i32): i32 { return 1 / X; }",
        "comptime func F(X: i32): i32 { var Local = X; Local += 1; return Local; }",
        "comptime func F(X: i32): i32 { var Local = X; return ++Local; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      CallAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-S0012\\]");
    }
  }

  // Compile-time-only definitions use the ordinary rules for writes to parameters and module objects.
  TEST(SemanticComptimeCallTest, RejectsParameterAndModuleWritesAtDefinition)
  {
    const char *Cases[] = {
        "comptime func F(X: i32): i32 { X = 10; return X; }",
        "comptime var Counter: i32 = 0; comptime func F(): i32 { Counter = 1; return Counter; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      CallAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(core::DiagnosticKind::SemanticInvalidAssignment), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    }
  }

  // Lowered void bodies support fallthrough and explicit return when invoked by another compile-time function.
  TEST(SemanticComptimeCallTest, ExecutesNestedVoidCallsAndExplicitReturns)
  {
    CallAnalysis Input(R"ink(
comptime func Fallthrough(): void { var Local: i32 = 1; }
comptime func Explicit(): void { return; }
comptime func Run(): i32
{
    Fallthrough();
    Explicit();
    return 7;
}
func First(): i32 { return comptime Run(); }
func Second(): i32 { return comptime Run(); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "First", 7);
    Input.expectIntegerReturn(*Result, "Second", 7);
  }

  // The callee signature supplies the width for integer arguments while a bool return retains its own type.
  TEST(SemanticComptimeCallTest, PreservesParameterWidthAndBooleanReturnType)
  {
    CallAnalysis Input(R"ink(
comptime func Identity(X: i64): i64 { return X; }
comptime func Boolean(X: bool): bool { return X; }
func Wide(): i64 { return comptime Identity(2147483648); }
func Check(): bool { return comptime Boolean(true); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Wide", 2147483648ULL, 64);
    const Value *Returned = Input.returnedValue(*Result, "Check");
    ASSERT_TRUE(BoolConstant::classof(Returned));
    EXPECT_TRUE(static_cast<const BoolConstant &>(*Returned).value());
  }

  // Invalid call arity or argument types fail before the body executes and emit one specific semantic diagnostic.
  TEST(SemanticComptimeCallTest, RejectsInvalidArgumentsWithoutDuplicateDiagnostics)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"comptime func F(X: i32): i32 { return X; } comptime F();", core::DiagnosticKind::SemanticArgumentCount},
        {"comptime func F(X: i32): i32 { return X; } comptime F(true);", core::DiagnosticKind::SemanticTypeMismatch},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      CallAnalysis Input(Entry.Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(Entry.Kind), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    }
  }

  // Return mismatches and missing returns are diagnosed once while checking an unused definition.
  TEST(SemanticComptimeCallTest, RejectsUnusedInvalidReturnsWithoutDuplicateDiagnostics)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"comptime func F(): i32 { return true; }", core::DiagnosticKind::SemanticTypeMismatch},
        {"comptime func F(): void { return 1; }", core::DiagnosticKind::SemanticTypeMismatch},
        {"comptime func F(): i32 { return; }", core::DiagnosticKind::SemanticMissingReturn},
        {"comptime func F(): i32 { var X: i32 = 1; }", core::DiagnosticKind::SemanticMissingReturn},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      CallAnalysis Input(Entry.Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(Entry.Kind), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    }
  }

  // Missing external symbols and Ink declarations without executable IR bodies produce compile-time diagnostics.
  TEST(SemanticComptimeCallTest, RejectsMissingExternalSymbolsAndInkBodies)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"import \"C\" func InkMissingExternalSymbol94c8f17e(): i32; comptime InkMissingExternalSymbol94c8f17e();", core::DiagnosticKind::ExecutionSymbolNotFound},
        {"func F(): i32;", core::DiagnosticKind::SemanticFunctionRequiresBody},
        {"comptime func F(): i32;", core::DiagnosticKind::SemanticComptimeFunctionRequiresBody},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      CallAnalysis Input(Entry.Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(Entry.Kind), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    }
  }

  // A compile-time-only function cannot become a runtime call or a runtime local function value.
  TEST(SemanticComptimeCallTest, RejectsRuntimeUseOfCompileTimeFunctions)
  {
    const char *Cases[] = {
        "comptime func F(X: i32): i32 { return X; } func Runtime(X: i32): i32 { return F(X); }",
        "comptime func F(X: i32): i32 { return X; } func Runtime(): void { var Target = F; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      CallAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(core::DiagnosticKind::SemanticComptimeFunctionAtRuntime), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    }
  }

  // A compile-time function with a stored IR body conflicts with a second definition of the same signature.
  TEST(SemanticComptimeCallTest, RejectsDuplicateCompileTimeFunctionDefinitions)
  {
    const char *Cases[] = {
        "comptime func F(): i32 { return 1; } comptime func F(): i32 { return 2; }",
        "comptime func F(): i32 { return 1; } func F(): i32 { return 2; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      CallAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(core::DiagnosticKind::SemanticDuplicateName), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    }
  }

  // Explicit compile-time branches still select code during lowering before the stored IR function executes.
  TEST(SemanticComptimeCallTest, ExpandsExplicitCompileTimeBranchBeforeIrExecution)
  {
    CallAnalysis Input("comptime func Select(): i32 { comptime if (false) { return Missing; } return 7; } func Read(): i32 { return comptime Select(); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Read", 7);
    const Function *Select = Input.function(*Result, "Select");
    ASSERT_NE(Select, nullptr);
    EXPECT_TRUE(Select->hasBody());
  }

  // Unused ordinary and compile-time definitions both resolve body names immediately.
  TEST(SemanticComptimeCallTest, ChecksUnusedNonGenericFunctionBodies)
  {
    const char *Cases[] = {
        "func Broken(): i32 { return Missing; }",
        "comptime func Broken(): i32 { return Missing; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      CallAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(core::DiagnosticKind::SemanticUnknownName), 1U);
      EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    }
  }

  // Execution errors in a nested IR call propagate to one source diagnostic without an AST callback wrapper.
  TEST(SemanticComptimeCallTest, NestedCallsReportExecutionFailureOnce)
  {
    CallAnalysis Input("import \"C\" func InkMissingExternalSymbol94c8f17e(): i32; comptime func Middle(): i32 { return InkMissingExternalSymbol94c8f17e(); } comptime func Outer(): i32 { return Middle(); } comptime Outer();");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(Input.Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::ExecutionSymbolNotFound);
  }

  // Non-terminating recursion reaches the configured dynamic call depth and reports the execution budget ICE.
  TEST(SemanticComptimeCallTest, StopsRecursiveCallsAtConfiguredDepth)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_EXECUTION_MAX_CALL_DEPTH");
    ASSERT_TRUE(Environment.set("8"));
    CallAnalysis Input("comptime func Recurse(): i32 { return Recurse(); } comptime Recurse();");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-E0022\\]: compile-time execution failed: execution budget exceeded");
  }
} // namespace ink::semantic::test
