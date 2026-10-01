#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "../core/environment_test_support.h"

#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
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

  // Declaring a compile-time function does not execute it; each actual call performs its module write once.
  TEST(SemanticComptimeCallTest, ExecutesModuleSideEffectsPerActualCall)
  {
    CallAnalysis Input(R"ink(
comptime var Counter: i32 = 0;
comptime func Next(): i32
{
    Counter += 1;
    return Counter;
}
func Before(): i32 { return comptime Counter; }
comptime var First: i32 = Next();
comptime var Second: i32 = Next();
func ReadFirst(): i32 { return comptime First; }
func ReadSecond(): i32 { return comptime Second; }
func After(): i32 { return comptime Counter; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Before", 0);
    Input.expectIntegerReturn(*Result, "ReadFirst", 1);
    Input.expectIntegerReturn(*Result, "ReadSecond", 2);
    Input.expectIntegerReturn(*Result, "After", 2);
    const Function *Next = Input.function(*Result, "Next");
    ASSERT_NE(Next, nullptr);
    EXPECT_FALSE(Next->hasBody());
  }

  // Mutating a parameter and local in one call does not change the caller's variable or the next call's storage.
  TEST(SemanticComptimeCallTest, IsolatesMutableParametersAndLocals)
  {
    CallAnalysis Input(R"ink(
comptime func Alter(X: i32): i32
{
    var Local: i32 = X;
    X += 10;
    Local += X;
    return Local;
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

  // Each argument executes once in source order before its value is installed in the callee's parameter frame.
  TEST(SemanticComptimeCallTest, EvaluatesArgumentsOnceInSourceOrder)
  {
    CallAnalysis Input(R"ink(
comptime var Counter: i32 = 0;
comptime func Pair(First: i32, Second: i32): i32 { return First * 10 + Second; }
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
    Input.expectIntegerReturn(*Result, "ReadFirst", 12);
    Input.expectIntegerReturn(*Result, "ReadSecond", 34);
    Input.expectIntegerReturn(*Result, "ReadCounter", 4);
  }

  // The same parameter-dependent branch is selected independently for each compile-time call.
  TEST(SemanticComptimeCallTest, SelectsBranchesUsingEachCallsArguments)
  {
    CallAnalysis Input(R"ink(
comptime func Choose(Flag: bool, X: i32): i32
{
    if (Flag)
    {
        return X + 1;
    }
    return X - 1;
}
func First(): i32 { return comptime Choose(true, 10); }
func Second(): i32 { return comptime Choose(false, 10); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "First", 11);
    Input.expectIntegerReturn(*Result, "Second", 9);
  }

  // Compile-time function loops preserve locals across iterations and apply continue, break and return to active paths.
  TEST(SemanticComptimeCallTest, ExecutesForAndWhileInsideFunctionCalls)
  {
    CallAnalysis Input(R"ink(
comptime func Accumulate(X: i32): i32
{
    var Sum: i32 = 0;
    for (var I: i32 = 0; I < X; I++)
    {
        if (I == 2) { continue; }
        Sum += I;
    }
    while (X > 0)
    {
        X -= 1;
        if (X == 1) { break; }
        Sum += 10;
    }
    return Sum;
}
func First(): i32 { return comptime Accumulate(4); }
func Second(): i32 { return comptime Accumulate(1); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "First", 24);
    Input.expectIntegerReturn(*Result, "Second", 10);
  }

  // Recursive calls preserve each invocation's parameter until its nested call returns.
  TEST(SemanticComptimeCallTest, EvaluatesTerminatingRecursion)
  {
    CallAnalysis Input(R"ink(
comptime func Factorial(N: i32): i32
{
    if (N <= 1) { return 1; }
    return N * Factorial(N - 1);
}
func Six(): i32 { return comptime Factorial(6); }
func Zero(): i32 { return comptime Factorial(0); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Six", 720);
    Input.expectIntegerReturn(*Result, "Zero", 1);
  }

  // Void callbacks accept both fallthrough and explicit return and can be invoked by another compile-time function.
  TEST(SemanticComptimeCallTest, ExecutesNestedVoidCallsAndExplicitReturns)
  {
    CallAnalysis Input(R"ink(
comptime var Counter: i32 = 0;
comptime func Fallthrough(): void { Counter += 1; }
comptime func Explicit(): void { Counter += 2; return; }
comptime func Run(): i32
{
    Fallthrough();
    Explicit();
    return Counter;
}
func First(): i32 { return comptime Run(); }
func Second(): i32 { return comptime Run(); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "First", 3);
    Input.expectIntegerReturn(*Result, "Second", 6);
  }

  // The callee signature supplies the width for integer arguments while a bool return retains its own type.
  TEST(SemanticComptimeCallTest, PreservesParameterWidthAndBooleanReturnType)
  {
    CallAnalysis Input(R"ink(
comptime func Identity(X: i64): i64 { return X; }
comptime func Positive(X: i64): bool { return X > 0; }
func Wide(): i64 { return comptime Identity(2147483648); }
func Check(): bool { return comptime Positive(2147483648); }
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

  // Return mismatches and missing non-void returns report the body error once rather than adding an engine failure.
  TEST(SemanticComptimeCallTest, RejectsInvalidReturnsWithoutDuplicateDiagnostics)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"comptime func F(): i32 { return true; } comptime F();", core::DiagnosticKind::SemanticTypeMismatch},
        {"comptime func F(): void { return 1; } comptime F();", core::DiagnosticKind::SemanticTypeMismatch},
        {"comptime func F(): i32 { return; } comptime F();", core::DiagnosticKind::SemanticMissingReturn},
        {"comptime func F(): i32 { var X: i32 = 1; } comptime F();", core::DiagnosticKind::SemanticMissingReturn},
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

  // Missing external symbols and Ink declarations without executable AST bodies produce compile-time diagnostics.
  TEST(SemanticComptimeCallTest, RejectsMissingExternalSymbolsAndInkBodies)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Kind;
    };
    const Case Cases[] = {
        {"extern \"C\" func InkMissingExternalSymbol94c8f17e(): i32; comptime InkMissingExternalSymbol94c8f17e();", core::DiagnosticKind::ExecutionSymbolNotFound},
        {"func F(): i32; comptime F();", core::DiagnosticKind::ExecutionMissingBody},
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

  // A compile-time function's missing runtime IR body does not make a duplicate definition a forward declaration.
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

  // Compile-time function execution skips an inactive branch even when that branch contains an unresolved name.
  TEST(SemanticComptimeCallTest, AnalyzesOnlyTheActiveFunctionPath)
  {
    CallAnalysis Input(R"ink(
comptime func Select(Flag: bool): i32
{
    if (Flag) { return 7; }
    return Missing;
}
func Read(): i32 { return comptime Select(true); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Read", 7);
  }

  // Delaying compile-time-only bodies does not suppress diagnostics in an unused ordinary function.
  TEST(SemanticComptimeCallTest, StillChecksUnusedOrdinaryFunctionBodies)
  {
    CallAnalysis Input("func Broken(): i32 { return Missing; } comptime func Deferred(): i32 { return 1; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    EXPECT_EQ(Input.diagnosticCount(core::DiagnosticKind::SemanticUnknownName), 1U);
  }

  // An arithmetic failure propagating through nested calls retains one concrete diagnostic instead of a wrapper cascade.
  TEST(SemanticComptimeCallTest, NestedCallsReportArithmeticFailureOnce)
  {
    CallAnalysis Input("comptime func Divide(Value: i32): i32 { return 1 / Value; } comptime func Middle(): i32 { return Divide(0); } comptime func Outer(): i32 { return Middle(); } comptime Outer();");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
    const auto &Diagnostic = Input.Diagnostics.diagnostics().front();
    EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::ExecutionDivisionByZero);
    EXPECT_EQ(core::DiagnosticFormatter{}.format(Diagnostic).Message, "compile-time execution failed: division by zero");
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
