#include "../execution/ffi/external_call_test_support.h"
#include "../core/environment_test_support.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <bit>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::semantic::test
{
  using namespace ink::ir;
  using namespace ink::execution;

  namespace
  {
    class LogicalAnalysis final
    {
      public:
        explicit LogicalAnalysis(std::string_view Source)
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

        ExecutionValueRef boolean(ExecutionEngine &Engine, bool Value)
        {
          return Engine.heap().boolean(Context.typePool().getType<TypeKind::Bool>(), Value);
        }

        ExecutionValueRef integer(ExecutionEngine &Engine, std::uint32_t Width, bool Signed, std::uint64_t Bits)
        {
          return Engine.heap().integer(*Context.typePool().getType<TypeKind::Integer>(Width, Signed), ExecutionInteger(Width, Bits));
        }

        std::size_t diagnosticCount(core::DiagnosticKind Kind) const
        {
          const auto &Entries = Diagnostics.diagnostics();
          return static_cast<std::size_t>(std::count_if(Entries.begin(), Entries.end(), [Kind](const core::Diagnostic &Entry)
          {
            return Entry.Kind == Kind;
          }));
        }

        void expectConstantBoolean(Module &Owner, std::string_view Name, bool Expected)
        {
          SCOPED_TRACE(Name);
          const Function *Target = function(Owner, Name);
          ASSERT_NE(Target, nullptr);
          ASSERT_NE(Target->entryBlock(), nullptr);
          ASSERT_FALSE(Target->entryBlock()->values().empty());
          const Value *Last = Target->entryBlock()->values().back().get();
          ASSERT_TRUE(ReturnInstruction::classof(Last));
          const Value *Returned = static_cast<const ReturnInstruction &>(*Last).returnedValue();
          ASSERT_TRUE(BoolConstant::classof(Returned));
          EXPECT_EQ(static_cast<const BoolConstant &>(*Returned).value(), Expected);
        }

        core::CompilationContext Compilation;
        core::FrontendContext Frontend;
        parser::ParseResult Parsed;
        SemanticContext Context;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    void expectBoolean(const ExecutionValueResult &Result, bool Expected)
    {
      ASSERT_TRUE(Result);
      ASSERT_EQ(Result.Value.kind(), ExecutionValueKind::Boolean);
      EXPECT_EQ(Result.Value.boolean(), Expected);
    }
  } // namespace

  // Every three-input combination observes logical precedence, negation, parentheses and nested short-circuit merges.
  TEST(SemanticLogicalTest, ExecutesNestedLogicWithSourcePrecedence)
  {
    LogicalAnalysis Input("func First(A: bool, B: bool, C: bool): bool { return !A || B && C; } func Second(A: bool, B: bool, C: bool): bool { return !(A || B) && !C; } func Third(A: bool, B: bool, C: bool): bool { return (A && B) || (!B && C); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *First = Input.function(*Result, "First");
    const Function *Second = Input.function(*Result, "Second");
    const Function *Third = Input.function(*Result, "Third");
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Third, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    for (unsigned Bits = 0; Bits < 8; ++Bits)
    {
      const bool A = (Bits & 1U) != 0;
      const bool B = (Bits & 2U) != 0;
      const bool C = (Bits & 4U) != 0;
      const ExecutionValueRef Arguments[] = {Input.boolean(Engine, A), Input.boolean(Engine, B), Input.boolean(Engine, C)};
      expectBoolean(Engine.execute(*First, Arguments), !A || (B && C));
      expectBoolean(Engine.execute(*Second, Arguments), !(A || B) && !C);
      expectBoolean(Engine.execute(*Third, Arguments), (A && B) || (!B && C));
    }
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
  }

  // A bodyless RHS is harmless on the skipped edge and reports MissingBody precisely when that edge is taken.
  TEST(SemanticLogicalTest, SkipsBodylessCallsOnlyOnShortCircuitEdges)
  {
    LogicalAnalysis Input("func Missing(): bool; func And(Flag: bool): bool { return Flag && Missing(); } func Or(Flag: bool): bool { return Flag || Missing(); }");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *And = Input.function(*Result, "And");
    const Function *Or = Input.function(*Result, "Or");
    ASSERT_NE(And, nullptr);
    ASSERT_NE(Or, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    const ExecutionValueRef False[] = {Input.boolean(Engine, false)};
    const ExecutionValueRef True[] = {Input.boolean(Engine, true)};
    expectBoolean(Engine.execute(*And, False), false);
    expectBoolean(Engine.execute(*Or, True), true);
    EXPECT_EQ(Engine.execute(*And, True).Status, ExecutionStatus::MissingBody);
    EXPECT_EQ(Engine.execute(*Or, False).Status, ExecutionStatus::MissingBody);
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
  }

  // Runtime RHS syntax must type-check even when its constant left operand will always skip it during execution.
  TEST(SemanticLogicalTest, RequiresBooleanOperandsOnBothRuntimePaths)
  {
    const char *Cases[] = {
        "func F(): bool { return !1; }",
        "func F(): bool { return false && 1; }",
        "func F(): bool { return true || 1; }",
        "func F(): bool { return 1 && true; }",
        "func F(): bool { return 1 || false; }",
        "func Void(): void {} func F(): bool { return false && Void(); }",
        "func F(Value: *i32): bool { return !Value; }",
        "func F(): bool { return !(1 + 2); }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      LogicalAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_NE(Input.diagnosticCount(core::DiagnosticKind::SemanticTypeMismatch), 0U);
    }
    LogicalAnalysis Unknown("func F(): bool { return false && MissingName; }");
    ASSERT_TRUE(Unknown.Parsed.succeeded());
    EXPECT_EQ(Unknown.analyze(), nullptr);
    EXPECT_EQ(Unknown.diagnosticCount(core::DiagnosticKind::SemanticUnknownName), 1U);
  }

  // Both integer ordering directions, equality and inequality use the declared signed integer type.
  TEST(SemanticLogicalTest, ExecutesEveryIntegerComparisonPredicate)
  {
    struct Case
    {
        const char *Operator;
        bool Less;
        bool Equal;
        bool Greater;
    };
    constexpr Case Cases[] = {
        {"==", false, true, false},
        {"!=", true, false, true},
        {"<", true, false, false},
        {"<=", true, true, false},
        {">", false, false, true},
        {">=", false, true, true},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Operator);
      LogicalAnalysis Input(std::string("func F(Left: i32, Right: i32): bool { return Left ") + Entry.Operator + " Right; }");
      ASSERT_TRUE(Input.Parsed.succeeded());
      Module *Result = Input.analyze();
      ASSERT_NE(Result, nullptr);
      const Function *FunctionValue = Input.function(*Result, "F");
      ASSERT_NE(FunctionValue, nullptr);
      ExecutionEngine Engine(Input.Context.irContext());
      const ExecutionValueRef Less[] = {Input.integer(Engine, 32, true, std::bit_cast<std::uint32_t>(std::int32_t{-2})), Input.integer(Engine, 32, true, 1)};
      const ExecutionValueRef Equal[] = {Input.integer(Engine, 32, true, 1), Input.integer(Engine, 32, true, 1)};
      const ExecutionValueRef Greater[] = {Input.integer(Engine, 32, true, 1), Input.integer(Engine, 32, true, std::bit_cast<std::uint32_t>(std::int32_t{-2}))};
      expectBoolean(Engine.execute(*FunctionValue, Less), Entry.Less);
      expectBoolean(Engine.execute(*FunctionValue, Equal), Entry.Equal);
      expectBoolean(Engine.execute(*FunctionValue, Greater), Entry.Greater);
    }
  }

  // Bool equality works for compound logical values, while ordering, mixed integer widths and non-integer scalars are rejected.
  TEST(SemanticLogicalTest, RestrictsComparisonsToMatchingIntegersOrBoolEquality)
  {
    LogicalAnalysis Valid("func Equal(A: bool, B: bool): bool { return (A && true) == (B || false); } func Different(A: bool, B: bool): bool { return A != B; }");
    ASSERT_TRUE(Valid.Parsed.succeeded());
    Module *Result = Valid.analyze();
    ASSERT_NE(Result, nullptr);
    const Function *Equal = Valid.function(*Result, "Equal");
    const Function *Different = Valid.function(*Result, "Different");
    ASSERT_NE(Equal, nullptr);
    ASSERT_NE(Different, nullptr);
    ExecutionEngine Engine(Valid.Context.irContext());
    for (unsigned Bits = 0; Bits < 4; ++Bits)
    {
      const bool A = (Bits & 1U) != 0;
      const bool B = (Bits & 2U) != 0;
      const ExecutionValueRef Arguments[] = {Valid.boolean(Engine, A), Valid.boolean(Engine, B)};
      expectBoolean(Engine.execute(*Equal, Arguments), A == B);
      expectBoolean(Engine.execute(*Different, Arguments), A != B);
    }
    const char *Cases[] = {
        "func F(A: bool, B: bool): bool { return A < B; }",
        "func F(A: bool, B: bool): bool { return A >= B; }",
        "func F(A: i32, B: i64): bool { return A == B; }",
        "func F(A: i32, B: u32): bool { return A <= B; }",
        "func F(A: bool): bool { return A == 1; }",
        "func F(A: *i32, B: *i32): bool { return A == B; }",
        "func F(A: f32, B: f32): bool { return A < B; }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      LogicalAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_NE(Input.diagnosticCount(core::DiagnosticKind::SemanticTypeMismatch), 0U);
    }
  }

  // Boolean result contexts do not force nested integer additions or comparison literals to bool.
  TEST(SemanticLogicalTest, SeparatesBooleanResultsFromIntegerOperandInference)
  {
    LogicalAnalysis Input(R"ink(
func Constant(): bool { return 1 + 2 < 4 && !(2 + 3 == 4); }
func Value(X: i64): bool { return X + 1 >= 2 && 0 < X && X != 7; }
func Condition(X: i32): bool { if (X + 1 > 2 && !(X == 4)) return true; else return false; }
func ReadConstant(): bool { return comptime Constant(); }
func ReadValue(): bool { return comptime Value(2); }
func ReadCondition(): bool { return comptime Condition(2); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectConstantBoolean(*Result, "ReadConstant", true);
    Input.expectConstantBoolean(*Result, "ReadValue", true);
    Input.expectConstantBoolean(*Result, "ReadCondition", true);
  }

  // Wide literals infer the other operand's width on either side, and signedness changes ordering of the same high bit.
  TEST(SemanticLogicalTest, PreservesSignednessAndWideLiteralOperands)
  {
    LogicalAnalysis Input(R"ink(
func Unsigned(X: u32): bool { return X > 1; }
func Signed(X: i32): bool { return X < 0; }
func Maximum(X: u64): bool { return 18446744073709551615 == X && X >= 18446744073709551615; }
func Minimum(X: i64): bool { return -9223372036854775808 == X && X <= -9223372036854775808; }
func Wide(X: u128): bool { return 340282366920938463463374607431768211455 == X; }
func ReadMaximum(): bool { return comptime Maximum(18446744073709551615); }
func ReadMinimum(): bool { return comptime Minimum(-9223372036854775808); }
func ReadWide(): bool { return comptime Wide(340282366920938463463374607431768211455); }
comptime const Max: u64 = 18446744073709551615;
comptime const Min: i64 = -9223372036854775808;
func ReadAst(): bool { return comptime (18446744073709551615 == Max && Min == -9223372036854775808); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectConstantBoolean(*Result, "ReadMaximum", true);
    Input.expectConstantBoolean(*Result, "ReadMinimum", true);
    Input.expectConstantBoolean(*Result, "ReadWide", true);
    Input.expectConstantBoolean(*Result, "ReadAst", true);
    const Function *Unsigned = Input.function(*Result, "Unsigned");
    const Function *Signed = Input.function(*Result, "Signed");
    ASSERT_NE(Unsigned, nullptr);
    ASSERT_NE(Signed, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    const ExecutionValueRef UnsignedArgument[] = {Input.integer(Engine, 32, false, 0x80000000U)};
    const ExecutionValueRef SignedArgument[] = {Input.integer(Engine, 32, true, 0x80000000U)};
    expectBoolean(Engine.execute(*Unsigned, UnsignedArgument), true);
    expectBoolean(Engine.execute(*Signed, SignedArgument), true);
  }

  // Parentheses retain a signed literal's minimum value, while surrounding unary operators retain target-width wraparound.
  TEST(SemanticLogicalTest, MaterializesParenthesizedSignedMinimaAtTheTargetWidth)
  {
    LogicalAnalysis Input(R"ink(
comptime const Min: i8 = -(128);
comptime const WideMin: i128 = -((170141183460469231731687303715884105728));
comptime const One: u8 = 1;
func Ordinary(X: i8): bool { return -(128) == X; }
func Left(): bool { return comptime (-(128) == Min); }
func Right(): bool { return comptime (Min == -((128))); }
func Nested(): bool { return comptime (-(-(128)) == Min && +(-(128)) == Min); }
func Wide(): bool { return comptime (-((170141183460469231731687303715884105728)) == WideMin); }
func ComputedUnsigned(): bool { return comptime (-One == 255); }
func RuntimeIr(): bool { return comptime Ordinary(-128); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    for (std::string_view Name : {"Left", "Right", "Nested", "Wide", "ComputedUnsigned", "RuntimeIr"})
    {
      Input.expectConstantBoolean(*Result, Name, true);
    }
  }

  // Deferred literal materialization rejects out-of-range magnitudes instead of widening or masking intermediate values.
  TEST(SemanticLogicalTest, RejectsOutOfRangeParenthesizedSignedLiterals)
  {
    const char *Cases[] = {
        "comptime const Value: i8 = -(129);",
        "comptime const Value: i8 = +(128);",
        "comptime const Value: i8 = -(+128);",
        "comptime const Value: i8 = -(-129);",
        "comptime const Value: u8 = -(1);",
        "comptime const Min: i8 = -128; func F(): bool { return comptime (-(129) == Min); }",
    };
    for (const char *Source : Cases)
    {
      SCOPED_TRACE(Source);
      LogicalAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      EXPECT_EQ(Input.analyze(), nullptr);
      EXPECT_EQ(Input.diagnosticCount(core::DiagnosticKind::SemanticIntegerOutOfRange), 1U);
    }
  }

  // Compile-time AST evaluation stays lazy, while compile-time functions execute the same short-circuit IR as ordinary calls.
  TEST(SemanticLogicalTest, PreservesComptimeShortCircuitAndSharedFunctionExecution)
  {
    LogicalAnalysis Input(R"ink(
func Missing(): bool;
comptime func Select(A: bool, B: bool): bool { return !A || B && true; }
comptime func Skip(): bool { return false && Missing() || true; }
func First(): bool { return comptime Select(false, false); }
func Second(): bool { return comptime Select(true, false); }
func Third(): bool { return comptime Select(true, true); }
func Fourth(): bool { return comptime Skip(); }
func Fifth(): bool { return comptime (false && Unknown); }
func Sixth(): bool { return comptime (true || Unknown); }
)ink");
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    EXPECT_TRUE(Input.Diagnostics.diagnostics().empty());
    Input.expectConstantBoolean(*Result, "First", true);
    Input.expectConstantBoolean(*Result, "Second", false);
    Input.expectConstantBoolean(*Result, "Third", true);
    Input.expectConstantBoolean(*Result, "Fourth", true);
    Input.expectConstantBoolean(*Result, "Fifth", false);
    Input.expectConstantBoolean(*Result, "Sixth", true);
    LogicalAnalysis Required("func Missing(): bool; comptime func Required(): bool { return true && Missing(); } func Read(): bool { return comptime Required(); }");
    ASSERT_TRUE(Required.Parsed.succeeded());
    EXPECT_EQ(Required.analyze(), nullptr);
    EXPECT_EQ(Required.diagnosticCount(core::DiagnosticKind::ExecutionMissingBody), 1U);
  }

#if defined(_WIN32) || defined(__linux__)
  // A deferred left literal exhausts traversal depth or steps before the RHS can write or fail through a bodyless call.
  TEST(SemanticLogicalTest, ChecksDeferredLiteralBudgetsBeforeRightOperandEffects)
  {
    struct Case
    {
        const char *MaxDepth;
        const char *MaxSteps;
        std::size_t Parentheses;
    };
    constexpr Case Cases[] = {
        {"16", "100000", 32},
        {"128", "32", 48},
    };
    for (const Case &Entry : Cases)
    {
      SCOPED_TRACE(Entry.MaxDepth);
      SCOPED_TRACE(Entry.MaxSteps);
      core::test::ScopedEnvironmentVariable Depth("INK_EXECUTION_MAX_EVALUATION_DEPTH");
      core::test::ScopedEnvironmentVariable Steps("INK_EXECUTION_MAX_STEPS");
      ASSERT_TRUE(Depth.set(Entry.MaxDepth));
      ASSERT_TRUE(Steps.set(Entry.MaxSteps));
      execution::test::NativePipe Pipe;
      ASSERT_TRUE(Pipe.valid());
      const std::string Left = std::string(Entry.Parentheses, '(') + "1" + std::string(Entry.Parentheses, ')');
      const std::string Source = execution::test::writeDeclaration() + "func Missing(): i8; func Right(): i8 { " + std::string(execution::test::WriteSymbol) + "(" + std::to_string(Pipe.writer()) + ", \"R\", 1); return Missing(); } func Read(): bool { return comptime (" + Left + " == Right()); }";
      LogicalAnalysis Input(Source);
      ASSERT_TRUE(Input.Parsed.succeeded());
      // Calling Right first would write and then report MissingBody without a panic.
      // This also distinguishes ordering on Windows death tests, which recreate pipes.
      EXPECT_DEATH(Input.analyze(), "internal compiler error\\[INK-E0022\\]");
      std::string Output;
      ASSERT_TRUE(Pipe.readAll(Output));
      EXPECT_TRUE(Output.empty());
    }
  }

  // Left and right native writes occur once in source order, and every short-circuited write is absent from the pipe.
  TEST(SemanticLogicalTest, ExecutesOperandSideEffectsExactlyOnce)
  {
    execution::test::NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    const std::string Write = std::string(execution::test::WriteSymbol);
    const std::string Source = execution::test::writeDeclaration() + "func Left(Fd: i32, Value: bool): bool { " + Write + "(Fd, \"L\", 1); return Value; } func Right(Fd: i32, Value: bool): bool { " + Write + "(Fd, \"R\", 1); return Value; } func And(Fd: i32, Flag: bool): bool { return Left(Fd, Flag) && Right(Fd, true); } func Or(Fd: i32, Flag: bool): bool { return Left(Fd, Flag) || Right(Fd, false); } func Nested(Fd: i32): bool { return !(Left(Fd, false) || Right(Fd, true)) || Left(Fd, true) && Right(Fd, true); }";
    LogicalAnalysis Input(Source);
    ASSERT_TRUE(Input.Parsed.succeeded());
    Module *Result = Input.analyze();
    ASSERT_NE(Result, nullptr);
    ASSERT_TRUE(Input.Diagnostics.diagnostics().empty());
    const Function *And = Input.function(*Result, "And");
    const Function *Or = Input.function(*Result, "Or");
    const Function *Nested = Input.function(*Result, "Nested");
    ASSERT_NE(And, nullptr);
    ASSERT_NE(Or, nullptr);
    ASSERT_NE(Nested, nullptr);
    ExecutionEngine Engine(Input.Context.irContext());
    const ExecutionValueRef False[] = {Input.integer(Engine, 32, true, Pipe.writer()), Input.boolean(Engine, false)};
    const ExecutionValueRef True[] = {Input.integer(Engine, 32, true, Pipe.writer()), Input.boolean(Engine, true)};
    const ExecutionValueRef Descriptor[] = {Input.integer(Engine, 32, true, Pipe.writer())};
    expectBoolean(Engine.execute(*And, False), false);
    expectBoolean(Engine.execute(*And, True), true);
    expectBoolean(Engine.execute(*Or, True), true);
    expectBoolean(Engine.execute(*Or, False), false);
    expectBoolean(Engine.execute(*Nested, Descriptor), true);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, "LLRLLRLRLR");
  }
#endif
} // namespace ink::semantic::test
