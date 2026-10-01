#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "../core/environment_test_support.h"

#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
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
    class ComptimeAnalysis
    {
      public:
        explicit ComptimeAnalysis(std::string_view Source)
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

        void expectIntegerReturn(Module &Owner, std::string_view Name, std::uint64_t Expected, std::uint32_t ExpectedWidth = 32)
        {
          SCOPED_TRACE(Name);
          const Value *Returned = returnedValue(Owner, Name);
          ASSERT_TRUE(IntegerConstant::classof(Returned));
          const auto &Bits = static_cast<const IntegerConstant &>(*Returned).value();
          EXPECT_EQ(Bits.bitWidth(), ExpectedWidth);
          ASSERT_EQ(Bits.words().size(), 1U);
          EXPECT_EQ(Bits.words().front(), Expected);
        }

        void expectBoolReturn(Module &Owner, std::string_view Name, bool Expected)
        {
          SCOPED_TRACE(Name);
          const Value *Returned = returnedValue(Owner, Name);
          ASSERT_TRUE(BoolConstant::classof(Returned));
          EXPECT_EQ(static_cast<const BoolConstant &>(*Returned).value(), Expected);
        }

        bool hasDiagnostic(core::DiagnosticKind Kind) const
        {
          const auto &Entries = Diagnostics.diagnostics();
          return std::any_of(Entries.begin(), Entries.end(), [Kind](const core::Diagnostic &Entry)
          {
            return Entry.Kind == Kind;
          });
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };
  } // namespace

  // Initializing X increments the module object once even when later declarations and functions reuse its result.
  TEST(SemanticComptimeTest, ExecutesInitializerSideEffectsOnce)
  {
    ComptimeAnalysis Input(R"ink(
comptime var Counter: i32 = 0;
comptime var X: i32 = ++Counter;
comptime var Y: i32 = Counter;
func ReadX(): i32 { return comptime X; }
func ReadY(): i32 { return comptime Y; }
func ReadCounter(): i32 { return comptime Counter; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "ReadX", 1);
    Input.expectIntegerReturn(*Result, "ReadY", 1);
    Input.expectIntegerReturn(*Result, "ReadCounter", 1);
  }

  // Functions retain the compile-time value observed at their source position while the module object remains writable.
  TEST(SemanticComptimeTest, PreservesSnapshotsAcrossModuleWrites)
  {
    ComptimeAnalysis Input(R"ink(
comptime var A: i32 = 1;
func Before(): i32 { return comptime(A + 1); }
comptime { A = 10; }
func After(): i32 { return comptime(A + 1); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Before", 2);
    Input.expectIntegerReturn(*Result, "After", 11);
  }

  // A compile-time local remains in the function analysis frame for a later multiplication expression.
  TEST(SemanticComptimeTest, RetainsCompileTimeLocalsAcrossStatements)
  {
    ComptimeAnalysis Input(R"ink(
comptime var A: i32 = 1;
func Square(): i32
{
    comptime var B: i32 = A + 1;
    return comptime(B * B);
}
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Square", 4);
  }

  // A runtime local receives a frozen compile-time initializer through an ordinary IR store.
  TEST(SemanticComptimeTest, LowersCompileTimeInitializerIntoRuntimeStorage)
  {
    ComptimeAnalysis Input("comptime var A: i32 = 1; func F(): i32 { var B: i32 = comptime(A + 1); return B; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *Target = Input.function(*Result, "F");
    ASSERT_NE(Target, nullptr);
    ASSERT_NE(Target->entryBlock(), nullptr);
    const StoreInstruction *Initializer = nullptr;
    for (const auto &Instruction : Target->entryBlock()->values())
    {
      if (StoreInstruction::classof(Instruction.get()))
      {
        ASSERT_EQ(Initializer, nullptr);
        Initializer = static_cast<const StoreInstruction *>(Instruction.get());
      }
    }
    ASSERT_NE(Initializer, nullptr);
    ASSERT_TRUE(IntegerConstant::classof(&Initializer->storedValue()));
    const auto &Bits = static_cast<const IntegerConstant &>(Initializer->storedValue()).value();
    EXPECT_EQ(Bits.bitWidth(), 32U);
    ASSERT_EQ(Bits.words().size(), 1U);
    EXPECT_EQ(Bits.words().front(), 2U);
    const Value *Returned = Input.returnedValue(*Result, "F");
    ASSERT_NE(Returned, nullptr);
    EXPECT_FALSE(Constant::classof(Returned));
  }

  // A runtime local cannot be read by a later forced compile-time expression merely because its initializer was constant.
  TEST(SemanticComptimeTest, RejectsRuntimeLocalInCompileTimeExpression)
  {
    ComptimeAnalysis Input("comptime var A: i32 = 1; func F(): i32 { var B: i32 = comptime(A + 1); return comptime(B * B); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    EXPECT_TRUE(Input.hasDiagnostic(core::DiagnosticKind::ExecutionRuntimeValue));
    EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
  }

  // Arithmetic failures receive one concrete execution diagnostic with the expression's source range.
  TEST(SemanticComptimeTest, ReportsSpecificArithmeticFailuresOnce)
  {
    struct Case
    {
        const char *Expression;
        core::DiagnosticKind Kind;
        const char *Message;
    };
    const Case Cases[] = {
        {"1 / 0", core::DiagnosticKind::ExecutionDivisionByZero, "compile-time execution failed: division by zero"},
        {"1 << 32", core::DiagnosticKind::ExecutionInvalidShift, "compile-time execution failed: invalid shift count"},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Expression);
      const std::string Prefix = "comptime var Result: i32 = ";
      ComptimeAnalysis Input(Prefix + Entry.Expression + ";");
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      ASSERT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
      const auto &Diagnostic = Input.Diagnostics.diagnostics().front();
      EXPECT_EQ(Diagnostic.Kind, Entry.Kind);
      EXPECT_TRUE(Diagnostic.Source.valid());
      EXPECT_EQ(Diagnostic.Span, core::SourceRange::fromByteOffsets(Prefix.size(), Prefix.size() + std::string_view(Entry.Expression).size()));
      EXPECT_EQ(core::DiagnosticFormatter{}.format(Diagnostic).Message, Entry.Message);
    }
  }

  // Cancelling the shared compile-time engine returns failure before evaluation without manufacturing a user error.
  TEST(SemanticComptimeTest, CancellationStopsAnalysisWithoutDiagnostic)
  {
    ComptimeAnalysis Input("comptime var Result: i32 = 1;");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Input.Context.comptimeState().Engine.cancel();
    EXPECT_EQ(Input.analyze(), nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
  }

  // Two calls of the same ordinary function execute with different parameter storage rather than a cached first result.
  TEST(SemanticComptimeTest, SeparatesActualFunctionCallFrames)
  {
    ComptimeAnalysis Input(R"ink(
func AddOne(X: i32): i32 { return X + 1; }
func First(): i32 { return comptime AddOne(1); }
func Second(): i32 { return comptime AddOne(10); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "First", 2);
    Input.expectIntegerReturn(*Result, "Second", 11);
  }

  // While iterations reload mutable counters, and continue and break each affect only the active loop.
  TEST(SemanticComptimeTest, ReexecutesWhileBodyWithContinueAndBreak)
  {
    ComptimeAnalysis Input(R"ink(
comptime var Total: i32 = 0;
comptime
{
    var I: i32 = 0;
    while (I < 4)
    {
        I += 1;
        if (I == 2) { continue; }
        Total += I;
        if (I == 3) { break; }
    }
}
func Result(): i32 { return comptime Total; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Result", 4);
  }

  // Classic for loops perform their step after continue and stop before the step following break.
  TEST(SemanticComptimeTest, ReexecutesForStepAndBody)
  {
    ComptimeAnalysis Input(R"ink(
comptime var Total: i32 = 0;
comptime var Steps: i32 = 0;
comptime
{
    for (var I: i32 = 0; I < 5; I++, Steps++)
    {
        if (I == 1) { continue; }
        if (I == 4) { break; }
        Total += I;
    }
}
func Sum(): i32 { return comptime Total; }
func StepCount(): i32 { return comptime Steps; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Sum", 5);
    Input.expectIntegerReturn(*Result, "StepCount", 4);
  }

  // A compile-time local with the same spelling uses its own binding and leaves the enclosing module object unchanged.
  TEST(SemanticComptimeTest, SeparatesShadowedCompileTimeBindings)
  {
    ComptimeAnalysis Input(R"ink(
comptime var A: i32 = 1;
func Inner(): i32 { comptime var A: i32 = 3; return comptime(A * A); }
func Outer(): i32 { return comptime A; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Inner", 9);
    Input.expectIntegerReturn(*Result, "Outer", 1);
  }

  // Compile-time constants reject a later write instead of silently changing the interned initializer value.
  TEST(SemanticComptimeTest, RejectsWritingCompileTimeConstant)
  {
    ComptimeAnalysis Input("comptime const A: i32 = 1; comptime { A = 2; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    EXPECT_TRUE(Input.hasDiagnostic(core::DiagnosticKind::SemanticInvalidAssignment));
  }

  // A declared but uninitialized compile-time slot cannot be converted into a constant return value.
  TEST(SemanticComptimeTest, RejectsReadingUninitializedCompileTimeObject)
  {
    ComptimeAnalysis Input("comptime var A: i32; func Read(): i32 { return comptime A; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    EXPECT_TRUE(Input.hasDiagnostic(core::DiagnosticKind::ExecutionUninitialized));
    EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
  }

  // Compile-time control flow does not bind or evaluate unknown names in branches that are never selected.
  TEST(SemanticComptimeTest, SkipsUnselectedBranchNames)
  {
    ComptimeAnalysis Input(R"ink(
comptime var Value: i32 = 0;
comptime
{
    if (true) { Value = 7; } else { Missing = 12; }
    if (false) { var Broken: i32 = Missing; }
}
func Result(): i32 { return comptime Value; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Result", 7);
  }

  // Boolean short-circuiting preserves its constant result without executing either skipped increment operand.
  TEST(SemanticComptimeTest, ShortCircuitsBooleanSideEffects)
  {
    ComptimeAnalysis Input(R"ink(
comptime var Counter: i32 = 0;
comptime const Left: bool = false && (++Counter == 1);
comptime const Right: bool = true || (++Counter == 1);
func ReadLeft(): bool { return comptime Left; }
func ReadRight(): bool { return comptime Right; }
func ReadCounter(): i32 { return comptime Counter; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectBoolReturn(*Result, "ReadLeft", false);
    Input.expectBoolReturn(*Result, "ReadRight", true);
    Input.expectIntegerReturn(*Result, "ReadCounter", 0);
  }

  // Static for expansion must stop processing the current body after break or continue even outside evaluation mode.
  TEST(SemanticComptimeTest, SkipsSideEffectsAfterStaticForControlTransfer)
  {
    ComptimeAnalysis Input(R"ink(
comptime var Counter: i32 = 0;
comptime for (var I: i32 = 0; I < 4; I++)
{
    comptime if (I == 1) { continue; }
    comptime if (I == 3) { break; }
    comptime ++Counter;
}
func Result(): i32 { return comptime Counter; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Result", 2);
  }

  // Static while expansion reloads its condition and skips remaining compile-time effects after each control transfer.
  TEST(SemanticComptimeTest, SkipsSideEffectsAfterStaticWhileControlTransfer)
  {
    ComptimeAnalysis Input(R"ink(
comptime var I: i32 = 0;
comptime var Counter: i32 = 0;
comptime while (I < 4)
{
    comptime ++I;
    comptime if (I == 2) { continue; }
    comptime if (I == 4) { break; }
    comptime ++Counter;
}
func Result(): i32 { return comptime Counter; }
func Iterations(): i32 { return comptime I; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Result", 2);
    Input.expectIntegerReturn(*Result, "Iterations", 4);
  }

  // Integer literals acquire the required i64 type in actual calls, inferred variables and forced compile-time returns.
  TEST(SemanticComptimeTest, PreservesExpectedIntegerWidthDuringEvaluation)
  {
    ComptimeAnalysis Input(R"ink(
func Wide(X: i64): i64 { return 1; }
comptime var R = Wide(1);
func Read(): i64 { return comptime R; }
func Static(): i64 { return comptime 1; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Read", 1, 64);
    Input.expectIntegerReturn(*Result, "Static", 1, 64);
  }

  // Compile-time overload selection considers literal range before materializing arguments in a candidate type.
  TEST(SemanticComptimeTest, SelectsOverloadBeforeMaterializingIntegerLiteral)
  {
    ComptimeAnalysis Input(R"ink(
func F(X: i32): i32 { return 1; }
func F(X: i64): i32 { return 2; }
func Small(): i32 { return comptime F(1); }
func Large(): i32 { return comptime F(2147483648); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Small", 1);
    Input.expectIntegerReturn(*Result, "Large", 2);
  }

  // Void compile-time results are diagnosed when used as a callee or explicit return operand rather than dereferenced.
  TEST(SemanticComptimeTest, RejectsVoidCompileTimeResultsInValuePositions)
  {
    const char *Cases[] = {
        "func V(): void {} (comptime V())();",
        "func V(): void {} func F(): void { return comptime V(); }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      ComptimeAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_TRUE(Input.hasDiagnostic(core::DiagnosticKind::SemanticTypeMismatch));
    }
  }

  // A compile-time block cannot consume a runtime function's return and leave a successful function without runtime IR.
  TEST(SemanticComptimeTest, RejectsReturningFromCompileTimeBlockIntoRuntimeFunction)
  {
    ComptimeAnalysis Input("func F(): i32 { comptime { return 1; } }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_EQ(Input.analyze(), nullptr);
    EXPECT_TRUE(Input.hasDiagnostic(core::DiagnosticKind::SemanticComptimeReturnAcrossRuntimeBoundary));
    EXPECT_EQ(Input.Diagnostics.diagnostics().size(), 1U);
  }

  // The same explicit compile-time increment gets a fresh semantic event for every static for iteration.
  TEST(SemanticComptimeTest, RepeatsStaticExpressionEventsForEachExpansion)
  {
    ComptimeAnalysis Input(R"ink(
comptime var Counter: i32 = 0;
comptime for (var I: i32 = 0; I < 3; I++)
{
    comptime ++Counter;
}
func Result(): i32 { return comptime Counter; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "Result", 3);
  }

  // Unbounded actual compile-time recursion reaches the shared execution budget and reports its resource-limit ICE.
  TEST(SemanticComptimeTest, StopsUnboundedCompileTimeRecursionAtBudget)
  {
    ComptimeAnalysis Input("func F(): i32 { return F(); } comptime F();");
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-E0022\\]: compile-time execution failed: execution budget exceeded");
  }

  // Iteratively parsed assignment chains still consume the shared compile-time traversal depth before the host stack grows unbounded.
  TEST(SemanticComptimeTest, StopsLongCompileTimeAssignmentChainAtBudget)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_SEMANTIC_EXPRESSION_DEPTH_LIMIT");
    ASSERT_TRUE(Environment.set(nullptr));
    std::string Source = "comptime { var A: i32 = 0; ";
    for (std::size_t Index = 0; Index < 1024; ++Index)
    {
      Source += "A = ";
    }
    Source += "1; }";
    ComptimeAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-E0022\\]: compile-time execution failed: execution budget exceeded");
  }

  // Runtime assignment chains share the expression nesting limit and retain the existing nesting ICE at its boundary.
  TEST(SemanticComptimeTest, BoundsRuntimeAssignmentChainsByExpressionDepth)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_SEMANTIC_EXPRESSION_DEPTH_LIMIT");
    ASSERT_TRUE(Environment.set("8"));
    const auto Source = [](std::size_t Count)
    {
      std::string Result = "func F(): i32 { var A: i32 = 0; ";
      for (std::size_t Index = 0; Index < Count; ++Index)
      {
        Result += "A = ";
      }
      return Result + "1; return A; }";
    };
    ComptimeAnalysis AtLimit(Source(7));
    ComptimeAnalysis OverLimit(Source(8));
    ASSERT_TRUE(AtLimit.Parsed.succeeded());
    ASSERT_TRUE(OverLimit.Parsed.succeeded());
    EXPECT_NE(AtLimit.analyze(), nullptr);
    EXPECT_DEATH(OverLimit.analyze(), "internal compiler error\\[INK-S0014\\]");
  }

  // Later overloads must not change the binding set captured by an earlier function definition.
  TEST(SemanticComptimeTest, PreservesDefinitionTimeOverloadSet)
  {
    ComptimeAnalysis Input(R"ink(
func Pick(X: i64): i32 { return 1; }
func Use(): i32 { return Pick(1); }
func Pick(X: i32): i32 { return 2; }
comptime var Result: i32 = Use();
func Read(): i32 { return comptime Result; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    Input.expectIntegerReturn(*Result, "Read", 1);
  }

  // A later local with the same spelling does not redirect a function's already established outer binding.
  TEST(SemanticComptimeTest, PreservesDefinitionTimeOuterBinding)
  {
    ComptimeAnalysis Input(R"ink(
comptime var A: i32 = 1;
comptime var Result: i32 = 0;
{
    func Read(): i32 { return comptime A; }
    comptime var A: i32 = 10;
    comptime { Result = Read(); }
}
func Get(): i32 { return comptime Result; }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    Input.expectIntegerReturn(*Result, "Get", 1);
  }
} // namespace ink::semantic::test
