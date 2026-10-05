#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"

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
    class IfAnalysis final
    {
      public:
        explicit IfAnalysis(std::string_view Source)
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

        std::size_t diagnosticCount(core::DiagnosticKind Kind) const
        {
          const auto &Entries = Diagnostics.diagnostics();
          return static_cast<std::size_t>(std::count_if(Entries.begin(), Entries.end(), [Kind](const core::Diagnostic &Entry)
          {
            return Entry.Kind == Kind;
          }));
        }

        void expectIntegerReturn(Module &Owner, std::string_view Name, std::uint64_t Expected)
        {
          SCOPED_TRACE(Name);
          const Function *Target = function(Owner, Name);
          ASSERT_NE(Target, nullptr);
          ASSERT_NE(Target->entryBlock(), nullptr);
          ASSERT_FALSE(Target->entryBlock()->values().empty());
          const Value *Last = Target->entryBlock()->values().back().get();
          ASSERT_TRUE(ReturnInstruction::classof(Last));
          const Value *Returned = static_cast<const ReturnInstruction &>(*Last).returnedValue();
          ASSERT_TRUE(IntegerConstant::classof(Returned));
          const auto &Bits = static_cast<const IntegerConstant &>(*Returned).value();
          ASSERT_EQ(Bits.words().size(), 1U);
          EXPECT_EQ(Bits.words().front(), Expected);
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };
  } // namespace

  // Returning arms need no merge block, and a missing else keeps its false path available for the following return.
  TEST(SemanticIfTest, LowersReturningBranchesWithoutEmptyMergeBlocks)
  {
    IfAnalysis Input("func Both(Flag: bool): i32 { if (Flag) return 1; else return 2; } func Partial(Flag: bool): i32 { if (Flag) return 3; return 4; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    for (std::string_view Name : {"Both", "Partial"})
    {
      const Function *Target = Input.function(*Result, Name);
      ASSERT_NE(Target, nullptr);
      ASSERT_EQ(Target->blocks().size(), 3U);
      for (const auto &Block : Target->blocks())
      {
        EXPECT_FALSE(Block->values().empty());
      }
      EXPECT_TRUE(ReturnInstruction::classof(Target->blocks()[1]->values().back().get()));
      EXPECT_TRUE(ReturnInstruction::classof(Target->blocks()[2]->values().back().get()));
    }
  }

  // Empty arms and mixed return/fallthrough arms leave exactly the continuation needed for implicit void returns.
  TEST(SemanticIfTest, PreservesVoidFallthroughForEveryReturnCombination)
  {
    const char *Cases[] = {
        "func F(Flag: bool): void { if (Flag) {} }",
        "func F(Flag: bool): void { if (Flag) {} else {} }",
        "func F(Flag: bool): void { if (Flag) return; else {} }",
        "func F(Flag: bool): void { if (Flag) {} else return; }",
        "func F(Flag: bool): void { if (Flag) return; else return; }",
        "func F(First: bool, Second: bool): void { if (First) { if (Second) return; } else return; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      IfAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      ASSERT_NE(Input.analyze(), nullptr);
      EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    }
  }

  // Both branch initialization, existing initialization, and the sole surviving arm all permit a later read.
  TEST(SemanticIfTest, MergesInitializationFromContinuingPathsOnly)
  {
    const char *Cases[] = {
        "func F(Flag: bool): i32 { var X: i32; if (Flag) X = 1; else X = 2; return X; }",
        "func F(Flag: bool): i32 { var X: i32; if (Flag) return 1; else X = 2; return X; }",
        "func F(Flag: bool): i32 { var X: i32; if (Flag) X = 1; else return 2; return X; }",
        "func F(Flag: bool): i32 { var X: i32 = 0; if (Flag) X = 1; return X; }",
        "func F(A: bool, B: bool): i32 { var X: i32; if (A) { if (B) X = 1; else X = 2; } else X = 3; return X; }",
        "func F(Flag: bool): i32 { var X: i32; if (Flag) { comptime if (true) X = 1; } else X = 2; return X; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      IfAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      ASSERT_NE(Input.analyze(), nullptr);
      EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    }
  }

  // An assignment in an alternative or returning arm cannot initialize an unassigned surviving path.
  TEST(SemanticIfTest, RejectsReadsMissingInitializationOnAnyContinuingPath)
  {
    const char *Cases[] = {
        "func F(Flag: bool): i32 { var X: i32; if (Flag) X = 1; return X; }",
        "func F(Flag: bool): i32 { var X: i32; if (Flag) {} else X = 1; return X; }",
        "func F(Flag: bool): i32 { var X: i32; if (Flag) X = 1; else return X; return X; }",
        "func F(Flag: bool): i32 { var X: i32; if (Flag) { X = 1; return X; } return X; }",
        "func F(Flag: bool): i32 { var X: i32; if (Flag) {} else { X = 1; return X; } return X; }",
        "func F(A: bool, B: bool): i32 { var X: i32; if (A) { if (B) X = 1; } else X = 2; return X; }",
        "func F(Flag: bool): i32 { var X: i32; if (Flag) { comptime if (true) X = 1; } else return X; return 0; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      IfAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_NE(Input.diagnosticCount(core::DiagnosticKind::SemanticUninitializedRead), 0U);
    }
  }

  // Bare declarations may share names across branches and with a later declaration because each branch has its own scope.
  TEST(SemanticIfTest, GivesBareDeclarationsIndependentBranchScopes)
  {
    IfAnalysis Input("func F(Flag: bool): i32 { if (Flag) var X: i32 = 1; else var X: i32 = 2; var X: i32 = 3; return X; }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    ASSERT_NE(Input.analyze(), nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
  }

  // Branch locals do not escape into the alternative branch or the continuation, with or without braces.
  TEST(SemanticIfTest, RejectsEscapingBranchLocals)
  {
    const char *Cases[] = {
        "func F(Flag: bool): i32 { if (Flag) var X: i32 = 1; return X; }",
        "func F(Flag: bool): i32 { if (Flag) var X: i32 = 1; else return X; return 0; }",
        "func F(Flag: bool): i32 { if (Flag) { var X: i32 = 1; } return X; }",
        "func F(Flag: bool): i32 { if (Flag) {} else var X: i32 = 1; return X; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      IfAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_NE(Input.diagnosticCount(core::DiagnosticKind::SemanticUnknownName), 0U);
    }
  }

  // Integer, pointer and void conditions produce a type diagnostic instead of entering conditional IR construction.
  TEST(SemanticIfTest, RequiresBooleanConditions)
  {
    const char *Cases[] = {
        "func F(): void { if (1) {} }",
        "func F(X: i32): void { if (X) {} }",
        "func F(X: *i32): void { if (X) {} }",
        "func Empty(): void {} func F(): void { if (Empty()) {} }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      IfAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(core::DiagnosticKind::SemanticTypeMismatch), 1U);
    }
  }

  // Ordinary constant conditions still check both arms, while comptime if only analyzes the selected arm.
  TEST(SemanticIfTest, ChecksBothRuntimeArmsAndOnlySelectedComptimeArm)
  {
    IfAnalysis Runtime("func F(): i32 { if (true) return FirstMissing; else return SecondMissing; }");
    ASSERT_TRUE(Runtime.Parsed.succeeded());
    EXPECT_EQ(Runtime.analyze(), nullptr);
    EXPECT_EQ(Runtime.diagnosticCount(core::DiagnosticKind::SemanticUnknownName), 2U);
    IfAnalysis CompileTime("func F(): i32 { comptime if (true) return 7; else return Missing; }");
    ASSERT_TRUE(CompileTime.Parsed.succeeded());
    Module *Result = CompileTime.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(CompileTime.Diagnostics.diagnostics().empty());
    CompileTime.expectIntegerReturn(*Result, "F", 7);
  }

  // A surviving path still needs a return, whereas statements after two returning arms are unreachable.
  TEST(SemanticIfTest, DiagnosesMissingReturnsAndUnreachableContinuations)
  {
    struct Case
    {
        const char *Source;
        core::DiagnosticKind Diagnostic;
    };
    const Case Cases[] = {
        {"func F(Flag: bool): i32 { if (Flag) return 1; }", core::DiagnosticKind::SemanticMissingReturn},
        {"func F(Flag: bool): i32 { if (Flag) return 1; else {} }", core::DiagnosticKind::SemanticMissingReturn},
        {"func F(Flag: bool): i32 { if (Flag) return 1; else return 2; return 3; }", core::DiagnosticKind::SemanticUnreachableStatement},
        {"func F(Flag: bool): i32 { if (Flag) { return 1; return 2; } else return 3; }", core::DiagnosticKind::SemanticUnreachableStatement},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Source);
      IfAnalysis Input(Entry.Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_NE(Input.diagnosticCount(Entry.Diagnostic), 0U);
    }
  }

  // Stored IR executes only the chosen arm across bool parameters, locals, calls, constants, nesting and else-if chains.
  TEST(SemanticIfTest, ExecutesOrdinaryAndComptimeFunctionsThroughConditionalIr)
  {
    IfAnalysis Input(R"ink(
func Identity(Flag: bool): bool { return Flag; }
func Nested(First: bool, Second: bool): i32
{
    var Local: bool = First;
    var Result: i32;
    if (Identity(Local))
    {
        if (Second) Result = 1; else Result = 2;
    }
    else if (Second) Result = 3;
    else Result = 4;
    return Result;
}
comptime func Select(Flag: bool): i32 { if (Flag) return 5; return 6; }
func Constant(): i32 { if (false) return 8; else return 7; }
func A(): i32 { return comptime Nested(true, true); }
func B(): i32 { return comptime Nested(true, false); }
func C(): i32 { return comptime Nested(false, true); }
func D(): i32 { return comptime Nested(false, false); }
func E(): i32 { return comptime Select(true); }
func F(): i32 { return comptime Select(false); }
func G(): i32 { return comptime Constant(); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "A", 1);
    Input.expectIntegerReturn(*Result, "B", 2);
    Input.expectIntegerReturn(*Result, "C", 3);
    Input.expectIntegerReturn(*Result, "D", 4);
    Input.expectIntegerReturn(*Result, "E", 5);
    Input.expectIntegerReturn(*Result, "F", 6);
    Input.expectIntegerReturn(*Result, "G", 7);
  }

  // A branch may break its own static loop, and leaving the branch restores control of the enclosing static loop.
  TEST(SemanticIfTest, PreservesComptimeLoopControlInsideAndAfterRuntimeBranches)
  {
    IfAnalysis Input(R"ink(
func Choose(Flag: bool): i32
{
    var Result: i32 = 0;
    comptime for (var I: i32 = 0; I < 2; I++)
    {
        if (Flag)
        {
            comptime for (var J: i32 = 0; J < 2; J++)
            {
                Result = Result + 1;
                break;
            }
        }
        else Result = Result + 2;
        break;
    }
    return Result;
}
func First(): i32 { return comptime Choose(true); }
func Second(): i32 { return comptime Choose(false); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectIntegerReturn(*Result, "First", 1);
    Input.expectIntegerReturn(*Result, "Second", 2);
  }

  // Runtime conditions cannot conditionally terminate or advance an enclosing compile-time loop during lowering.
  TEST(SemanticIfTest, RejectsRuntimeBranchesControllingComptimeLoops)
  {
    const char *Cases[] = {
        "func F(Flag: bool): void { comptime for (var I: i32 = 0; I < 1; I++) { if (Flag) break; } }",
        "func F(Flag: bool): void { comptime for (var I: i32 = 0; I < 1; I++) { if (Flag) continue; } }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      IfAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_NE(Input.diagnosticCount(core::DiagnosticKind::SemanticInvalidLoopControl), 0U);
    }
  }
} // namespace ink::semantic::test
