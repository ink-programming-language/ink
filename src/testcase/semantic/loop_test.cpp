#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/function/function.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <string_view>

namespace ink::semantic::test
{
  namespace
  {
    class LoopAnalysis final
    {
      public:
        explicit LoopAnalysis(std::string_view Source)
            : Frontend(Compilation),
              Parsed(parser::parse(Frontend, tokenizer::tokenize(Frontend, std::string(Source)))),
              Context(Compilation)
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        ir::Module *analyze()
        {
          return Analyzer{}.analyze(Context, Parsed);
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    void expectLoop(std::string_view Source, std::uint64_t Expected)
    {
      SCOPED_TRACE(Source);
      LoopAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      auto *Module = Input.analyze();
      ASSERT_NE(Module, nullptr);
      ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
      const auto *Binding = NameResolver(Input.Context).lookupMember(*Module, Input.Context.namePool().find("Entry"));
      ASSERT_NE(Binding, nullptr);
      ASSERT_EQ(Binding->targets().size(), 1U);
      const auto *Function = static_cast<const ir::Function *>(Binding->targets().front());
      execution::ExecutionEngine Engine(Input.Context.irContext());
      const auto Result = Engine.execute(*Function);
      ASSERT_TRUE(Result) << static_cast<int>(Result.Status);
      ASSERT_EQ(Result.Value.kind(), execution::RuntimeKind::Integer);
      EXPECT_EQ(Result.Value.integer().lowWord(), Expected);
    }

    void expectLoopDiagnostic(std::string_view Source, core::DiagnosticKind Kind)
    {
      SCOPED_TRACE(Source);
      LoopAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      const auto &Diagnostics = Input.Diagnostics.diagnostics();
      EXPECT_TRUE(std::any_of(Diagnostics.begin(), Diagnostics.end(), [Kind](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == Kind;
      }));
    }
  } // namespace

  // While checks its condition before the first iteration and reloads mutated locals on every back edge.
  TEST(SemanticLoopTest, WhileZeroOneAndManyIterations)
  {
    for (unsigned Count : {0U, 1U, 8U})
    {
      expectLoop("func Entry(): i32 { var I = 0; var Sum = 0; while (I < " + std::to_string(Count) + ") { I++; Sum += I; } return Sum; }", Count * (Count + 1) / 2);
    }
  }

  // Continue returns to the while condition, whereas break skips all remaining iterations.
  TEST(SemanticLoopTest, WhileBreakContinueAndEarlyReturn)
  {
    expectLoop("func Entry(): i32 { var I = 0; var Sum = 0; while (true) { ++I; if (I == 2) continue; if (I == 5) break; Sum += I; } return Sum; }", 8);
    expectLoop("func Entry(): i32 { while (true) { return 42; } }", 42);
    expectLoop("func Entry(): i32 { var I = 0; while (I < 10) { if (I == 3) return I; I++; } return 99; }", 3);
  }

  // Classic for supports every omitted header clause, declaration and expression initializers, and bare bodies.
  TEST(SemanticLoopTest, ClassicForHeaderForms)
  {
    const char *Cases[] = {
        "func Entry(): i32 { var N = 0; for (var I = 0; I < 6; I++) N += 7; return N; }",
        "func Entry(): i32 { var N = 0; for (; N < 42;) N++; return N; }",
        "func Entry(): i32 { var N = 0; for (;;) { N++; if (N == 42) break; } return N; }",
        "func Entry(): i32 { var N = 0; for (N = 40;; ++N) if (N == 42) break; return N; }",
        "func Entry(): i32 { var N = 0; for (; N < 42; N++) {} return N; }",
        "func Entry(): i32 { for (;;) return 42; }",
    };
    for (const auto *Source : Cases)
    {
      expectLoop(Source, 42);
    }
  }

  // Multiple initializers and steps run left to right, continue executes steps, and break bypasses them.
  TEST(SemanticLoopTest, ClassicForStepOrdering)
  {
    expectLoop("func Entry(): i32 { var I = 9; var Steps = 0; var Sum = 0; for (I = 0, Steps = I + 10; I < 5; I++, Steps += I) { if (I == 1) continue; if (I == 4) break; Sum += I; } return Steps + Sum; }", 25);
  }

  // Prefix/postfix updates return the correct value and evaluate array/pointer destinations only once.
  TEST(SemanticLoopTest, UpdatesHaveValueAndSingleEvaluation)
  {
    expectLoop("func Entry(): i32 { var N = 4; var A = N++; var B = ++N; var C = N--; var D = --N; return A + B + C + D + N; }", 24);
    expectLoop("func Entry(): i32 { var A = [4, 8]; var I = 0; A[I++]++; var P = &A[1]; --*P; return A[0] + A[1] + I; }", 13);
    expectLoop("func Entry(): i32 { var I: u8 = 0; --I; ++I; if (I == 0) return 42; return 0; }", 42);
    expectLoop("func Entry(): i32 { var I: u128 = 0; --I; ++I; if (I == 0) return 42; return 0; }", 42);
    expectLoop("func Entry(): i32 { var I = 3; I += ++I; return I; }", 7);
  }

  // Nested for and while loops route break and continue to the nearest enclosing loop.
  TEST(SemanticLoopTest, NestedLoopsKeepIndependentTargets)
  {
    expectLoop("func Entry(): i32 { var Sum = 0; for (var I = 0; I < 3; I++) { var J = 0; while (J < 5) { J++; if (J == 2) continue; if (J == 4) break; Sum += I + J; } } return Sum; }", 18);
    expectLoop("func Entry(): i32 { var I = 0; var Sum = 0; while (I < 3) { I++; for (;;) { Sum += I; break; } continue; } return Sum; }", 6);
  }

  // Conditions with short circuit and calls are evaluated once at each test, including the final failed test.
  TEST(SemanticLoopTest, ConditionEffectsAndShortCircuit)
  {
    expectLoop("func Check(P: *i32): bool { (*P)++; return *P < 4; } func Entry(): i32 { var Calls = 0; var N = 0; while (Check(&Calls) && N < 10) N++; return Calls + N; }", 7);
    expectLoop("func Check(P: *i32): bool { (*P)++; return true; } func Entry(): i32 { var Calls = 0; for (; false && Check(&Calls);) {} return Calls; }", 0);
  }

  // Array iteration snapshots the iterable once, gives each iteration a mutable value copy, and supports wildcard bindings.
  TEST(SemanticLoopTest, ForInSnapshotsArraysAndScopesBindings)
  {
    expectLoop("func Entry(): i32 { var A = [1, 2, 3]; var Sum = 0; for (X in A) { A[1] = 99; X += 10; Sum += X; } var X = 6; return Sum + X; }", 42);
    expectLoop("func Entry(): i32 { var N = 0; for (_ in [1, 2, 3]) N++; return N; }", 3);
    expectLoop("func Entry(): i32 { var A: [i32; 0] = []; var N = 42; for (X in A) N = X; return N; }", 42);
    expectLoop("func Entry(): i32 { var Sum = 0; for (X in [1, 2, 3, 4, 5]) { if (X == 2) continue; if (X == 5) break; Sum += X; } return Sum; }", 8);
  }

  // Iterable-returning calls run once, and nested array iteration has independent counters.
  TEST(SemanticLoopTest, ForInCallAndNestedArrays)
  {
    expectLoop("func Make(P: *i32): [i32; 3] { (*P)++; return [4, 5, 6]; } func Entry(): i32 { var Calls = 0; var Sum = 0; for (X in Make(&Calls)) Sum += X; return Sum + Calls; }", 16);
    expectLoop("func Entry(): i32 { var Sum = 0; for (Row in [[1, 2], [3, 4]]) for (X in Row) Sum += X; return Sum; }", 10);
  }

  // Array patterns destructure recursively, including named/ignored tails and empty tails.
  TEST(SemanticLoopTest, ForInArrayDestructuring)
  {
    expectLoop("func Entry(): i32 { var Sum = 0; for ([[A, _], [B, _]] in [[[1, 2], [3, 4]]]) Sum += A + B; return Sum; }", 4);
    expectLoop("func Entry(): i32 { var Sum = 0; for ([First, Tail...] in [[1, 2, 3], [4, 5, 6]]) { Sum += First; for (X in Tail) Sum += X; } return Sum; }", 21);
    expectLoop("func Entry(): i32 { var Sum = 0; for ([X, _...] in [[2, 9], [3, 8]]) Sum += X; return Sum; }", 5);
    expectLoop("func Entry(): i32 { var Sum = 0; for ([X, Tail...] in [[42]]) { Sum += X; for (_ in Tail) Sum++; } return Sum; }", 42);
  }

  // Ordinary loops in comptime functions and forced calls execute their lowered control flow.
  TEST(SemanticLoopTest, ExecutesAllLoopFormsInComptimeCalls)
  {
    expectLoop("comptime func Sum(): i32 { var N = 0; for (var I = 0; I < 3; I++) N += I; while (N < 6) N++; for (X in [10, 11, 15]) N += X; return N; } func Entry(): i32 { return comptime Sum(); }", 42);
  }

  // Static for-in works in both evaluation blocks and source expansion with break/continue.
  TEST(SemanticLoopTest, ExpandsAndEvaluatesComptimeForIn)
  {
    expectLoop("comptime var Sum = 0; comptime { for (X in [1, 2, 3, 4, 5]) { if (X == 2) continue; if (X == 5) break; Sum += X; } } func Entry(): i32 { return Sum; }", 8);
    expectLoop("func Entry(): i32 { var Sum = 0; comptime for (X in [1, 2, 3, 4]) { comptime if (X == 2) continue; comptime if (X == 4) break; Sum += X; } return Sum; }", 4);
    expectLoop("comptime var Sum = 0; comptime for ([A, B] in [[1, 2], [3, 4]]) { comptime { Sum += A + B; } } func Entry(): i32 { return Sum; }", 10);
  }

  // Static and runtime loops can nest in either order without consuming the other's transfer targets.
  TEST(SemanticLoopTest, MixesStaticAndRuntimeLoops)
  {
    expectLoop("func Entry(): i32 { var Sum = 0; for (var I = 0; I < 3; I++) { comptime for (var J = 0; J < 4; J++) { Sum++; break; } } return Sum; }", 3);
    expectLoop("func Entry(): i32 { var Sum = 0; comptime for (var I = 0; I < 3; I++) { for (var J = 0; J < 4; J++) { if (J == 2) break; Sum++; } } return Sum; }", 6);
  }

  // Break paths of unconditional loops participate in definite initialization; returning paths do not.
  TEST(SemanticLoopTest, MergesInitializationAtLoopExits)
  {
    expectLoop("func Entry(): i32 { var X: i32; for (;;) { X = 42; break; } return X; }", 42);
    expectLoop("func Run(B: bool): i32 { var X: i32; while (true) { if (B) { X = 42; break; } else return 9; } return X; } func Entry(): i32 { return Run(true); }", 42);
    expectLoop("func Entry(): i32 { var X: i32; for (X = 42; false;) {} return X; }", 42);
  }

  // Zero-trip exits and continue edges cannot claim initialization performed only on another body path.
  TEST(SemanticLoopTest, RejectsConditionalOrSkippedInitialization)
  {
    const char *Cases[] = {
        "func Entry(B: bool): i32 { var X: i32; while (B) { X = 1; break; } return X; }",
        "func Entry(): i32 { var X: i32; for (; false;) X = 1; return X; }",
        "func Entry(B: bool): i32 { var X: i32; for (;;) { if (B) break; X = 1; break; } return X; }",
        "func Entry(B: bool): void { var X: i32; for (var I = 0; I < 2; I++, X++) { if (B) continue; X = 1; } }",
        "func Entry(): i32 { var X: i32; while (false) X = 1; return X; }",
        "func Entry(): i32 { var X: i32; for (_ in [1]) X = 1; return X; }",
        "func Entry(): void { var X: i32; while (true) { X++; break; } }",
        "func Entry(): void { var X: i32; for (;;) { X += 1; break; } }",
    };
    for (const auto *Source : Cases)
    {
      expectLoopDiagnostic(Source, core::DiagnosticKind::SemanticUninitializedRead);
    }
  }

  // Header, binding and bare-body locals remain inside the loop and cannot leak into its step or continuation.
  TEST(SemanticLoopTest, EnforcesLoopScopes)
  {
    const char *Cases[] = {
        "func Entry(): i32 { for (var I = 0; I < 1; I++) {} return I; }",
        "func Entry(): i32 { while (false) var X = 1; return X; }",
        "func Entry(): i32 { for (X in [1]) {} return X; }",
        "func Entry(): void { for (var I = 0; I < 1; X++) var X = 0; }",
        "func Entry(): i32 { comptime for (X in [1]) {} return X; }",
    };
    for (const auto *Source : Cases)
    {
      expectLoopDiagnostic(Source, core::DiagnosticKind::SemanticUnknownName);
    }
    expectLoop("func Entry(): i32 { var I = 42; for (var I = 0; I < 1; I++) var X = I; for (I in [1]) var X = I; return I; }", 42);
  }

  // Loop tests require bool and for-in requires an array; skipped runtime bodies are still checked.
  TEST(SemanticLoopTest, RejectsInvalidConditionsAndIterables)
  {
    const char *Cases[] = {
        "func Entry(): void { while (1) {} }",
        "func Entry(): void { for (; 1;) {} }",
        "func Entry(): void { for (X in 1) {} }",
        "func Entry(): void { for (X in true) {} }",
        "func Nothing(): void {} func Entry(): void { while (Nothing()) {} }",
    };
    for (const auto *Source : Cases)
    {
      expectLoopDiagnostic(Source, core::DiagnosticKind::SemanticTypeMismatch);
    }
    expectLoopDiagnostic("func Entry(): void { while (false) Missing(); }", core::DiagnosticKind::SemanticUnknownName);
  }

  // Updates reject immutable, uninitialized and noninteger destinations rather than silently changing a copy.
  TEST(SemanticLoopTest, RejectsInvalidUpdateOperands)
  {
    expectLoopDiagnostic("func Entry(): void { const I = 1; for (; I < 3; I++) {} }", core::DiagnosticKind::SemanticInvalidAddressOperand);
    expectLoopDiagnostic("func Entry(): void { var I = true; while (false) I++; }", core::DiagnosticKind::SemanticTypeMismatch);
    expectLoopDiagnostic("func Entry(): void { for (; false; 1++) {} }", core::DiagnosticKind::SemanticInvalidAddressOperand);
  }

  // Constructor fields initialized along every break path are complete, but zero-trip bodies cannot initialize fields.
  TEST(SemanticLoopTest, TracksConstructorFieldsThroughLoops)
  {
    expectLoop("class P { property X: i32; func __init__(): void { for (;;) { this.X = 42; break; } } }; func Entry(): i32 { var A = P(); return A.X; }", 42);
    expectLoopDiagnostic("class P { property X: i32; func __init__(B: bool): void { while (B) { this.X = 42; break; } } };", core::DiagnosticKind::SemanticInvalidConstruction);
  }

  // Transfer statements outside loops, across execution stages, or in nested functions report user errors.
  TEST(SemanticLoopTest, RejectsInvalidLoopTransfers)
  {
    const char *Cases[] = {
        "break;",
        "continue;",
        "func Entry(): void { break; }",
        "func Entry(): void { continue; }",
        "func Entry(): void { while (true) { comptime { break; } } }",
        "func Entry(): void { for (;;) { comptime { continue; } } }",
        "func Entry(B: bool): void { comptime for (var I = 0; I < 1; I++) if (B) break; }",
        "func Entry(B: bool): void { while (true) { comptime for (var I = 0; I < 1; I++) if (B) continue; break; } }",
        "func Entry(): void { for (;;) { func Inner(): void { break; } break; } }",
    };
    for (const auto *Source : Cases)
    {
      expectLoopDiagnostic(Source, core::DiagnosticKind::SemanticInvalidLoopControl);
    }
  }

  // Destructuring rejects incompatible element kinds, wrong lengths and duplicate names.
  TEST(SemanticLoopTest, RejectsInvalidIterationPatterns)
  {
    expectLoopDiagnostic("func Entry(): void { for ([A] in [1]) {} }", core::DiagnosticKind::SemanticInvalidIterationBinding);
    expectLoopDiagnostic("func Entry(): void { for ([A, B] in [[1]]) {} }", core::DiagnosticKind::SemanticInvalidIterationBinding);
    expectLoopDiagnostic("func Entry(): void { for ((A, B) in [[1, 2]]) {} }", core::DiagnosticKind::SemanticInvalidIterationBinding);
    expectLoopDiagnostic("func Entry(): void { for ([A, A] in [[1, 2]]) {} }", core::DiagnosticKind::SemanticDuplicateName);
    expectLoopDiagnostic("comptime var A: [i32; 0] = []; comptime for ([X] in A) {}", core::DiagnosticKind::SemanticInvalidIterationBinding);
    expectLoopDiagnostic("func Entry(): void { for ([A, A...] in [[1, 2]]) {} }", core::DiagnosticKind::SemanticDuplicateName);
  }

  // Statements after unconditional transfers remain unreachable, while potentially zero-trip loops still require a return.
  TEST(SemanticLoopTest, ChecksReachabilityAndMissingReturns)
  {
    expectLoopDiagnostic("func Entry(): void { while (true) { break; var X = 1; } }", core::DiagnosticKind::SemanticUnreachableStatement);
    expectLoopDiagnostic("func Entry(): void { for (;;) { continue; var X = 1; } }", core::DiagnosticKind::SemanticUnreachableStatement);
    expectLoopDiagnostic("func Entry(B: bool): i32 { while (B) return 1; }", core::DiagnosticKind::SemanticMissingReturn);
    expectLoopDiagnostic("func Entry(): void { for (;;) {} var X = 1; }", core::DiagnosticKind::SemanticUnreachableStatement);
  }

  // Each iteration cleans locals on fallthrough, continue and break; for-header objects live until loop exit.
  TEST(SemanticLoopTest, DestroysIterationAndHeaderObjectsExactlyOnce)
  {
    const std::string Class = "class P { property Count: *i32; property N: i32; func __init__(C: *i32, N: i32): void { this.Count = C; this.N = N; } func __del__(): void { *this.Count += this.N; } }; ";
    expectLoop(Class + "func Entry(): i32 { var Count = 0; for (var I = 0; I < 5; I++) { var A = P(&Count, 1); if (I == 1) continue; { var B = P(&Count, 10); if (I == 3) break; } } return Count; }", 34);
    expectLoop(Class + "func Entry(): i32 { var Count = 0; for (var Header = P(&Count, 20); true;) { var Body = P(&Count, 22); break; } return Count; }", 42);
    expectLoop(Class + "func Run(C: *i32): void { for (var H = P(C, 20); true;) { var B = P(C, 22); return; } } func Entry(): i32 { var Count = 0; Run(&Count); return Count; }", 42);
    expectLoop(Class + "func Entry(): i32 { var Count = 0; var I = 0; while (I++ < 3) { var A: P; if (I == 2) A = P(&Count, 42); } return Count; }", 42);
  }

  // For-in owns temporary arrays and per-iteration class copies even when an iteration continues or breaks.
  TEST(SemanticLoopTest, DestroysIterableAndElementCopies)
  {
    expectLoop("class P { property C: *i32; func __init__(C: *i32): void { this.C = C; } func __del__(): void { (*this.C)++; } }; func Entry(): i32 { var N = 0; for (X in [P(&N), P(&N)]) { break; } return N; }", 3);
    expectLoop("class P { property C: *i32; func __init__(C: *i32): void { this.C = C; } func __del__(): void { (*this.C)++; } }; func Entry(): i32 { var N = 0; for (X in [P(&N), P(&N)]) { continue; } return N; }", 4);
  }

  // Condition and step temporaries are destroyed once at their full expression, including short-circuit skips.
  TEST(SemanticLoopTest, DestroysConditionAndStepTemporaries)
  {
    const std::string Class = "class P { property Count: *i32; func __init__(C: *i32): void { this.Count = C; } func yes(): bool { return true; } func __del__(): void { (*this.Count)++; } }; ";
    expectLoop(Class + "func Entry(): i32 { var N = 0; var I = 0; while (I < 3 && P(&N).yes()) { I++; continue; } return N; }", 3);
    expectLoop(Class + "func Entry(): i32 { var N = 0; for (var I = 0; I < 4; I++, P(&N)) { if (I == 1) continue; if (I == 3) break; } return N; }", 3);
  }

  // Infinite lowered loops remain budget-limited instead of hanging execution or requiring a synthetic return.
  TEST(SemanticLoopTest, BoundsInfiniteRuntimeExecution)
  {
    LoopAnalysis Input("func Entry(): void { for (;;) {} }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    auto *Module = Input.analyze();
    ASSERT_NE(Module, nullptr);
    const auto *Binding = NameResolver(Input.Context).lookupMember(*Module, Input.Context.namePool().find("Entry"));
    ASSERT_NE(Binding, nullptr);
    execution::ExecutionLimits Limits;
    Limits.MaxSteps = 100;
    execution::ExecutionEngine Engine(Input.Context.irContext(), Limits);
    EXPECT_EQ(Engine.execute(*static_cast<const ir::Function *>(Binding->targets().front())).Status, execution::ExecutionStatus::BudgetExceeded);
  }
} // namespace ink::semantic::test
