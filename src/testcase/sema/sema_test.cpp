#include "ink/frontend/compilation_session.h"
#include "ink/sema/analyzer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace ink::sema
{
  namespace
  {
    static_assert(!std::is_default_constructible<VerifiedSemanticModule>::value, "VerifiedSemanticModule must not be forgeable through default construction");
    static_assert(!std::is_copy_constructible<VerifiedSemanticModule>::value, "VerifiedSemanticModule must retain unique verified capability ownership");
    static_assert(!std::is_convertible<SymbolId, SymbolId::ValueType>::value, "semantic IDs must not implicitly convert to their storage type");
    static_assert(!std::is_convertible<SymbolId::ValueType, SymbolId>::value, "semantic IDs must not implicitly construct from their storage type");
    static_assert(!std::is_same<SymbolId, ScopeId>::value, "semantic table IDs must remain strongly typed");

    ast::AstFile lower(frontend::CompilationSession &Session, std::string Source)
    {
      const core::SourceFileId File = Session.addSource("sema-test.ink", std::move(Source));
      std::optional<ast::AstFile> AstFile = Session.parseAndLower(File);
      EXPECT_TRUE(AstFile.has_value());
      if (!AstFile)
      {
        throw std::logic_error("test source did not lower to an AST file");
      }
      return *AstFile;
    }

    bool hasDiagnostic(const SemanticAnalysisResult &Result, core::DiagnosticKind Kind)
    {
      return std::any_of(Result.Diagnostics.begin(), Result.Diagnostics.end(), [Kind](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == Kind;
      });
    }

    // Verifies semantic IDs preserve invalid, Boolean, equality, ordering, hashing, and table-type boundaries independently of analyzer behavior.
    TEST(SemanticIdContractTest, PreservesStrongValueSemantics)
    {
      constexpr SymbolId Invalid;
      constexpr SymbolId First = SymbolId::fromValue(4);
      constexpr SymbolId Equal = SymbolId::fromValue(4);
      constexpr SymbolId Later = SymbolId::fromValue(5);

      EXPECT_FALSE(Invalid.isValid());
      EXPECT_FALSE(static_cast<bool>(Invalid));
      EXPECT_TRUE(First.isValid());
      EXPECT_TRUE(static_cast<bool>(First));
      EXPECT_EQ(First, Equal);
      EXPECT_NE(First, Later);
      EXPECT_LT(First, Later);
      EXPECT_EQ(std::hash<SymbolId>{}(First), std::hash<SymbolId>{}(Equal));
    }

    // Verifies the first semantic slice resolves functions, parameters, locals, calls, operators, conditions, loops, assignments, and returns into a sealed module.
    TEST(SemanticAnalyzerTest, VerifiesCompleteFirstSliceProgram)
    {
      const std::string Source = "func add(Left: i32, Right: i32) -> i32 { return Left + Right; } func choose(Flag: bool, First: i32, Second: i32) -> i32 { if (Flag) { return First; } else { return Second; } } func main() { var Value: i32 = add(1, 2); const Limit: i32 = choose(true, 10, 20); while (Value < Limit && true) { Value += 1; } return; }";
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, Source);
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.Module.has_value());
      EXPECT_TRUE(Result.Diagnostics.empty());
      EXPECT_TRUE(Result.Verification.succeeded());
      const SemanticModel &Model = Result.Module->model();
      EXPECT_EQ(Model.sourceFile(), File.sourceFile());
      EXPECT_TRUE(Model.contains(Model.globalScope()));
      EXPECT_GE(Model.scopes().size(), 7U);
      EXPECT_GE(Model.symbols().size(), 10U);
      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        const ast::AstExprId Id = ast::AstExprId::fromValue(Index);
        if (Model.expressionKind(Id) == ast::AstKind::NameExpression)
        {
          EXPECT_EQ(Model.resolvedName(Id).Status, ResolvedNameStatus::Resolved);
        }
      }
    }

    // Verifies forward function signatures are canonicalized before body checking so recursion and calls to later functions resolve.
    TEST(SemanticAnalyzerTest, ResolvesForwardAndRecursiveFunctionCalls)
    {
      const std::string Source = "func first(Value: i32) -> i32 { if (Value == 0) { return second(Value); } else { return first(Value - 1); } } func second(Value: i32) -> i32 { return Value; }";
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, Source);
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      ASSERT_TRUE(Result.succeeded());
      const SemanticModel &Model = Result.Module->model();
      const type::TypeId Expected = Session.typeContext().functionType({Session.typeContext().i32Type()}, Session.typeContext().i32Type());
      for (const Symbol &Symbol : Model.symbols())
      {
        if (Symbol.Kind == SymbolKind::Function)
        {
          EXPECT_EQ(Symbol.Type, Expected);
        }
      }
    }

    // Verifies comptime force-value expressions retain the operand type and an explicit AstKind marker for later staged IR lowering.
    TEST(SemanticAnalyzerTest, RecordsComptimeExpressionKindAndOperandType)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func value() -> i32 { return comptime 7; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      ASSERT_TRUE(Result.succeeded());
      const SemanticModel &Model = Result.Module->model();
      bool Found = false;
      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        const ast::AstExprId Id = ast::AstExprId::fromValue(Index);
        if (Model.expressionKind(Id) == ast::AstKind::ComptimeExpression)
        {
          Found = true;
          EXPECT_EQ(Model.expressionType(Id), Session.typeContext().i32Type());
          EXPECT_EQ(Model.expressionCategory(Id), ExpressionCategory::Value);
          ASSERT_TRUE(Model.constantValue(Id).has_value());
          EXPECT_EQ(std::get<std::int32_t>(*Model.constantValue(Id)), 7);
        }
      }
      EXPECT_TRUE(Found);
    }

    // Verifies every scalar type representable by the first-slice force-value protocol remains a legal comptime operand.
    TEST(SemanticAnalyzerTest, AcceptsForceValueRepresentableComptimeTypes)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func values(Flag: bool, I32: i32, I64: i64, U32: u32, U64: u64) { comptime Flag; comptime I32; comptime I64; comptime U32; comptime U64; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_TRUE(Result.succeeded());
      EXPECT_TRUE(Result.Module.has_value());
    }

    // Verifies comptime rejects void, never, unit, and function channels before a verified semantic capability can reach force-value lowering.
    TEST(SemanticAnalyzerTest, RejectsComptimeOperandsWithoutForceValueRepresentation)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func noop() {} func bottom() -> never; func makeUnit() -> unit; func target() {} func invalid() { comptime noop(); comptime bottom(); comptime makeUnit(); comptime target; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_GE(std::count_if(Result.Diagnostics.begin(), Result.Diagnostics.end(), [](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == core::DiagnosticKind::UnsupportedSemanticFeature;
      }), 4);
    }

    // Verifies declaration collection reports same-scope duplicates with the previous definition and name resolution continues to diagnose missing and ambiguous names.
    TEST(SemanticAnalyzerTest, DiagnosesDeclarationAndNameFailuresWithoutStopping)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func broken(Value: i32, Value: i32) -> i32 { return Missing + Value; } func unknown(Item: User) { return; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::RedefinedName));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnresolvedName));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::AmbiguousName));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnknownTypeName));
      const auto Duplicate = std::find_if(Result.Diagnostics.begin(), Result.Diagnostics.end(), [](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == core::DiagnosticKind::RedefinedName;
      });
      ASSERT_NE(Duplicate, Result.Diagnostics.end());
      ASSERT_FALSE(Duplicate->Related.empty());
      EXPECT_EQ(Duplicate->Related.front().Kind, core::DiagnosticRelatedKind::PreviousDefinition);
      EXPECT_EQ(Duplicate->Related.front().File, File.sourceFile());
    }

    // Verifies type checking accumulates assignment, call, condition, and return failures in one analysis instead of stopping after the first error.
    TEST(SemanticAnalyzerTest, DiagnosesIndependentTypeFailuresWithoutStopping)
    {
      const std::string Source = "func id(Value: i32) -> i32 { return Value; } func broken() -> i32 { const Fixed: i32 = 1; Fixed = 2; 1 = 2; var Flag: bool = 1; id(); Fixed(); if (1) {} while (1) { break; } return true; }";
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, Source);
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::AssignmentToImmutable));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::InvalidAssignmentTarget));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::TypeMismatch));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::ArgumentCountMismatch));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::NotCallable));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::InvalidConditionType));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::ReturnTypeMismatch));
      for (const core::Diagnostic &Diagnostic : Result.Diagnostics)
      {
        EXPECT_EQ(Diagnostic.File, File.sourceFile());
      }
    }

    // Verifies invalid operators, loop control outside a loop, and a value-returning fallthrough each receive stable semantic diagnostics.
    TEST(SemanticAnalyzerTest, DiagnosesOperatorsControlFlowAndMissingReturn)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func broken() -> i32 { const Flag = true; const First = -Flag; const Second = Flag + 1; break; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::InvalidUnaryOperator));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::InvalidBinaryOperator));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::InvalidControlFlow));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::MissingReturn));
    }

    // Verifies syntactically valid out-of-slice constructs become semantic unsupported diagnostics and never produce a verified capability.
    TEST(SemanticAnalyzerTest, RejectsUnsupportedAstWithoutCrashing)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "class Box { var Value: i32; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnsupportedSemanticFeature));
    }

    // Verifies top-level storage and shift operators are rejected before a semantic capability can reach an IR generator that has no matching first-slice lowering.
    TEST(SemanticAnalyzerTest, RejectsFeaturesOutsideTheIrLoweringSlice)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "const Global: i32 = 1; func shifted(Value: i32) -> i32 { var Result: i32 = Value; Result <<= 1; return Value << 1; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnsupportedSemanticFeature));
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::InvalidBinaryOperator));
    }

    // Verifies division, remainder, and their compound assignments stay outside the verified slice until TargetContext provides PDB rules.
    TEST(SemanticAnalyzerTest, RejectsIntegerPdbOperatorsBeforeIrLowering)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func divide(Left: i32, Right: i32) -> i32 { return Left / Right; } func remainder(Left: i32, Right: i32) -> i32 { return Left % Right; } func divideAssign(Value: i32) -> i32 { var Result = Value; Result /= 2; return Result; } func remainderAssign(Value: i32) -> i32 { var Result = Value; Result %= 2; return Result; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_EQ(std::count_if(Result.Diagnostics.begin(), Result.Diagnostics.end(), [](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == core::DiagnosticKind::UnsupportedSemanticFeature;
      }), 4);
    }

    // Verifies every accepted i32 literal spelling is fully consumed with its declared base, digit separators, and optional i32 suffix.
    TEST(SemanticAnalyzerTest, ParsesExactFirstSliceIntegerLiteralSpellings)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func values() -> i32 { const Hex = 0x2A; const Binary = 0b1010; const Octal = 0o17; const Grouped = 1_000; const Suffixed = 42i32; return Hex + Binary + Octal + Grouped + Suffixed; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      ASSERT_TRUE(Result.succeeded());
      std::vector<std::int32_t> Values;
      const SemanticModel &Model = Result.Module->model();
      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        const ast::AstExprId Id = ast::AstExprId::fromValue(Index);
        if (Model.expressionKind(Id) != ast::AstKind::LiteralExpression)
        {
          continue;
        }
        const std::optional<ConstantValue> Constant = Model.constantValue(Id);
        if (Constant && std::holds_alternative<std::int32_t>(*Constant))
        {
          Values.push_back(std::get<std::int32_t>(*Constant));
        }
      }
      std::sort(Values.begin(), Values.end());
      EXPECT_EQ(Values, (std::vector<std::int32_t>{10, 15, 42, 42, 1000}));
    }

    // Verifies out-of-range and non-i32-suffixed integer literals diagnose before sealing and are independently rejected by the semantic verifier.
    TEST(SemanticAnalyzerTest, RejectsUnrepresentableIntegerLiteralsBeforeIrLowering)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func tooLarge() -> i32 { return 2147483648; } func wrongSuffix() -> i32 { return 1u32; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnsupportedSemanticFeature));
      EXPECT_FALSE(Result.Verification.succeeded());
      EXPECT_TRUE(std::any_of(Result.Verification.Errors.begin(), Result.Verification.Errors.end(), [](const SemanticVerificationError &Error)
      {
        return Error.Message.find("literal") != std::string::npos || Error.Message.find("runtime expression") != std::string::npos;
      }));
    }

    // Verifies a local binding is invisible before its declaration and an erroneous runtime name cannot be sealed into VerifiedSemanticModule.
    TEST(SemanticAnalyzerTest, RejectsUseBeforeLocalBindingDeclaration)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func value() -> i32 { return Later; const Later: i32 = 1; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnresolvedName));
      EXPECT_FALSE(Result.Verification.succeeded());
    }

    // Verifies a later inner binding does not hide an outer parameter at source positions before the local declaration.
    TEST(SemanticAnalyzerTest, ResolvesOuterNameBeforeInnerBindingDeclaration)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func value(Value: i32) -> i32 { { return Value; const Value: i32 = 1; } }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_TRUE(Result.succeeded());
    }

    // Verifies first-slice local storage always has an initializer before it can become a verified lowering input.
    TEST(SemanticAnalyzerTest, RejectsLocalBindingWithoutInitializer)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func initialize() { var Value: i32; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnsupportedSemanticFeature));
    }

    // Verifies void and never remain result channels and cannot be used as first-slice function parameter value types.
    TEST(SemanticAnalyzerTest, RejectsVoidAndNeverFunctionParameters)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func voidParameter(Value: void) {} func neverParameter(Value: never) {}");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_GE(std::count_if(Result.Diagnostics.begin(), Result.Diagnostics.end(), [](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == core::DiagnosticKind::UnsupportedSemanticFeature;
      }), 2);
    }

    // Verifies source function types, function-value materialization, and calls through non-function symbols remain outside the direct-call first slice.
    TEST(SemanticAnalyzerTest, RejectsFunctionValuesAndIndirectCalls)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func target(Value: i32) -> i32 { return Value; } func save() { const Saved = target; } func apply(Callback: func(i32) -> i32, Value: i32) -> i32 { return Callback(Value); }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnsupportedSemanticFeature));
    }

    // Verifies a call with no result cannot initialize a binding even though void remains valid as a function result channel.
    TEST(SemanticAnalyzerTest, RejectsVoidBindingValues)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func noop() {} func invalid() { const Result = noop(); }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_TRUE(hasDiagnostic(Result, core::DiagnosticKind::UnsupportedSemanticFeature));
    }

    // Verifies imports and default arguments receive explicit out-of-slice diagnostics instead of being silently discarded by IR lowering.
    TEST(SemanticAnalyzerTest, RejectsImportsAndDefaultArguments)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "import core.io; func configured(Value: i32 = 1) { return; }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Result.Module.has_value());
      EXPECT_GE(std::count_if(Result.Diagnostics.begin(), Result.Diagnostics.end(), [](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == core::DiagnosticKind::UnsupportedSemanticFeature;
      }), 2);
    }

    // Verifies a never-valued local initializer legally terminates a value-returning path and remains representable by first-slice lowering.
    TEST(SemanticAnalyzerTest, AcceptsNeverInitializerAsUnreachableControlFlow)
    {
      frontend::CompilationSession Session;
      const ast::AstFile File = lower(Session, "func stop() -> never; func diverges() -> i32 { const Bottom = stop(); }");
      SemanticAnalysisResult Result = analyze(Session.astContext(), File, Session.stringInterner(), Session.typeContext());

      EXPECT_TRUE(Result.succeeded());
      EXPECT_TRUE(Result.Module.has_value());
    }
  } // namespace
} // namespace ink::sema
