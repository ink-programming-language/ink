#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    using test::expectFullFidelity;
    using test::hasDiagnostic;
    using test::hasKind;
    using test::missingTokens;
    using test::nodeTextsOfKind;
    using test::parseSource;

    struct MalformedSyntaxCase
    {
        const char *Name;
        const char *Source;
        core::DiagnosticKind ExpectedDiagnostic;
        bool ExpectsErrorNode;
        bool ExpectsMissingToken;
        CstKind RecoveredKind = CstKind::Unknown;
    };

    bool containsRecoveredAfterDeclaration(const ParsedFile &File)
    {
      const std::vector<std::string> Bindings = nodeTextsOfKind(File, CstKind::TopLevelBindingDeclaration);
      return std::any_of(Bindings.begin(), Bindings.end(), [](const std::string &Text)
                         {
                           return Text.find("After = 1;") != std::string::npos;
                         });
    }

    void expectMalformedSyntax(const MalformedSyntaxCase &TestCase)
    {
      SCOPED_TRACE(TestCase.Name);
      const ParsedFile First = parseSource(TestCase.Source);
      const ParsedFile Second = parseSource(TestCase.Source);

      EXPECT_FALSE(First.succeeded());
      EXPECT_EQ(First.completeness(), ParseCompleteness::Complete);
      EXPECT_TRUE(hasDiagnostic(First, TestCase.ExpectedDiagnostic)) << "missing " << core::diagnosticKindName(TestCase.ExpectedDiagnostic);
      if (TestCase.ExpectsErrorNode)
      {
        EXPECT_TRUE(hasKind(First, CstKind::Error));
        EXPECT_TRUE(hasFlag(First.cst().node(First.cst().root()).Flags, CstNodeFlags::HasError));
      }
      if (TestCase.ExpectsMissingToken)
      {
        EXPECT_FALSE(missingTokens(First).empty());
        EXPECT_TRUE(hasFlag(First.cst().node(First.cst().root()).Flags, CstNodeFlags::HasMissing));
      }
      if (TestCase.RecoveredKind != CstKind::Unknown)
      {
        EXPECT_TRUE(hasKind(First, TestCase.RecoveredKind)) << "recovery did not reach " << cstKindName(TestCase.RecoveredKind);
      }
      EXPECT_TRUE(containsRecoveredAfterDeclaration(First));
      EXPECT_EQ(First.completeness(), Second.completeness());
      EXPECT_EQ(First.cst().nodes(), Second.cst().nodes());
      EXPECT_EQ(First.cst().children(), Second.cst().children());
      EXPECT_TRUE(test::diagnosticsEqual(First, Second));
      expectFullFidelity(First);
      expectFullFidelity(Second);
    }

    // Verifies malformed module paths and forbidden semicolon-only constructs report their committed error and resume at a later declaration.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversMalformedImportsAndSemicolonBoundaries)
    {
      const std::vector<MalformedSyntaxCase> Cases = {
          {"SingleSegmentAbsoluteImport", "import core; const After = 1;", core::DiagnosticKind::ExpectedToken, false, true},
          {"TriviaSplitsRelativePrefix", "import . /* gap */ .common; const After = 1;", core::DiagnosticKind::ExpectedToken, false, true},
          {"EmptyStatement", "func Broken() { ; return; } const After = 1;", core::DiagnosticKind::ExpectedSyntax, true, true, CstKind::ReturnStatement},
          {"SemicolonAfterNestedBlock", "func Broken() { {} ; return; } const After = 1;", core::DiagnosticKind::ExpectedSyntax, true, true, CstKind::ReturnStatement},
          {"SemicolonAfterFunctionDeclaration", "func Broken() {}; const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false},
      };

      for (const MalformedSyntaxCase &TestCase : Cases)
      {
        expectMalformedSyntax(TestCase);
      }
    }

    // Verifies incomplete bindings, local access prefixes, argument ordering, and mixed forward-all syntax preserve missing or rejected input and keep parsing.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversBindingsAndCallArguments)
    {
      const std::vector<MalformedSyntaxCase> Cases = {
          {"VarMissingNameAndValue", "func Broken() { var; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"VarMissingTypeOrInitializer", "func Broken() { var Value; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"ConstMissingInitializer", "func Broken() { const Value: i32; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"TupleBindingMissingInitializer", "func Broken() { const (First, Second); return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"LocalAccessModifier", "func Broken() { public var Value = 1; return; } const After = 1;", core::DiagnosticKind::ExpectedSyntax, true, true, CstKind::ReturnStatement},
          {"PositionalAfterNamed", "func Broken() { call(name = Value, Positional); return; } const After = 1;", core::DiagnosticKind::ExpectedSyntax, true, true, CstKind::ReturnStatement},
          {"ForwardAllAfterPositional", "func Broken() { call(Value, ...); return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"ForwardAllBeforePositional", "func Broken() { call(..., Value); return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"ForwardAllBeforeNamed", "func Broken() { call(..., name = Value); return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"AttributeBareForwardAll", "[reflect(...)] func Broken() {} const After = 1;", core::DiagnosticKind::ExpectedToken, false, true},
          {"DecoratorBareForwardAll", "@trace(...) func Broken() {} const After = 1;", core::DiagnosticKind::ExpectedToken, false, true},
      };

      for (const MalformedSyntaxCase &TestCase : Cases)
      {
        expectMalformedSyntax(TestCase);
      }
    }

    // Verifies generic terminator ambiguity, malformed slices, and postfixes on unparenthesized function-type values recover at the enclosing semicolon.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversGenericSliceAndFunctionTypeBoundaries)
    {
      const std::vector<MalformedSyntaxCase> Cases = {
          {"UnparenthesizedGenericGreaterThan", "const Broken = Box::<N > 0>; const After = 1;", core::DiagnosticKind::ExpectedToken, true, true},
          {"UnparenthesizedGenericGreaterEqual", "const Broken = Box::<N >= 0>; const After = 1;", core::DiagnosticKind::ExpectedToken, true, true},
          {"UnparenthesizedGenericRightShift", "const Broken = Box::<N >> 1>; const After = 1;", core::DiagnosticKind::ExpectedToken, true, true},
          {"RepeatedSliceColon", "func Broken() { Value[Low::High]; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, true, true, CstKind::ReturnStatement},
          {"ThirdSliceBound", "func Broken() { Value[Low:High:Step]; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, true, true, CstKind::ReturnStatement},
          {"RepeatedEmptySliceColon", "func Broken() { Value[::]; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, true, true, CstKind::ReturnStatement},
          {"DirectFunctionTypePointer", "const Broken = func()*; const After = 1;", core::DiagnosticKind::ExpectedToken, false, true},
          {"DirectFunctionTypeReference", "const Broken = func()&; const After = 1;", core::DiagnosticKind::ExpectedToken, false, true},
          {"DirectFunctionTypeEmptyBrackets", "const Broken = func()[]; const After = 1;", core::DiagnosticKind::ExpectedToken, true, true},
          {"DirectFunctionTypeCall", "const Broken = func()(Value); const After = 1;", core::DiagnosticKind::ExpectedToken, true, true},
      };

      for (const MalformedSyntaxCase &TestCase : Cases)
      {
        expectMalformedSyntax(TestCase);
      }
    }

    // Verifies malformed loop headers, missing control-flow blocks, throw causes, and unsupported handler forms do not absorb following statements or declarations.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversControlFlowAndExceptionSyntax)
    {
      const std::vector<MalformedSyntaxCase> Cases = {
          {"TupleForPattern", "func Broken() { for (var (First, Second) in Values) {} return; } const After = 1;", core::DiagnosticKind::ExpectedToken, true, true, CstKind::ReturnStatement},
          {"ForMissingModeAndPattern", "func Broken() { for (in Values) {} return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"IfMissingBlock", "func Broken() { if (Ready) return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"WhileMissingBlock", "func Broken() { while (Ready) return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"ThrowNumericCause", "func Broken() { throw Failure from 1; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, true, true, CstKind::ReturnStatement},
          {"ThrowPostfixCause", "func Broken() { throw Failure from Cause.member; return; } const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false, CstKind::ReturnStatement},
          {"ThrowMissingCause", "func Broken() { throw Failure from; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"TryMissingCatch", "func Broken() { try {} return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"UnsupportedTryFilter", "func Broken() { try {} filter (Ready) {} return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"UnsupportedFinally", "func Broken() { try {} finally {} return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"UnsupportedCatchFilter", "func Broken() { try {} catch Failure if (Ready) {} return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"TypedCatchAfterCatchAll", "func Broken() { try {} catch {} catch Failure {} return; } const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false, CstKind::ReturnStatement},
          {"DeclarationAsDirectMatchArm", "func Broken() { match (Value) { _ => var Local = 1; .next => return; } return; } const After = 1;", core::DiagnosticKind::DeclarationRequiresBlock, true, false, CstKind::ReturnStatement},
      };

      for (const MalformedSyntaxCase &TestCase : Cases)
      {
        expectMalformedSyntax(TestCase);
      }
    }

    // Verifies each parser region rejects an item from another region, synchronizes locally, and still recognizes a later top-level declaration.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversItemsPlacedInWrongRegions)
    {
      const std::vector<MalformedSyntaxCase> Cases = {
          {"RuntimeStatementAtTopLevel", "return; const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false},
          {"ExpressionStatementAtTopLevel", "Value; const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false},
          {"PackageHeaderAtTopLevel", "package demo; const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false},
          {"ModuleHeaderAtTopLevel", "module demo; const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false},
          {"ImportInStatementRegion", "func Broken() { import core.io; return; } const After = 1;", core::DiagnosticKind::ExpectedSyntax, true, true, CstKind::ReturnStatement},
          {"RuntimeStatementInClassRegion", "class Broken { return; var Good: i32; } const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false, CstKind::FieldDeclaration},
          {"DecoratorDeclarationInInterfaceRegion", "interface Broken { decorator Bad(); func Good(); } const After = 1;", core::DiagnosticKind::UnexpectedToken, true, false, CstKind::FunctionDeclaration},
          {"FunctionDeclarationInEnumRegion", "enum Broken { func Bad(); Good } const After = 1;", core::DiagnosticKind::UnexpectedToken, true, true, CstKind::EnumBranch},
      };

      for (const MalformedSyntaxCase &TestCase : Cases)
      {
        expectMalformedSyntax(TestCase);
      }
    }

    // Verifies invalid patterns, assignment nesting, and unsupported comptime operands retain their tokens and recover through the next valid construct.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversPatternsAssignmentsAndComptimeOperands)
    {
      const std::vector<MalformedSyntaxCase> Cases = {
          {"EmptyVariantPayloadPattern", "func Broken() { match (Value) { .some() => return; _ => return; } return; } const After = 1;", core::DiagnosticKind::ExpectedSyntax, false, true, CstKind::ReturnStatement},
          {"TrailingVariantPayloadComma", "func Broken() { match (Value) { .some(Item,) => return; _ => return; } return; } const After = 1;", core::DiagnosticKind::TrailingComma, true, false, CstKind::ReturnStatement},
          {"TuplePatternMissingComma", "func Broken() { if (match .some((Item)) = Value) {} return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
          {"TuplePatternLeadingComma", "func Broken() { if (match .some((,Item)) = Value) {} return; } const After = 1;", core::DiagnosticKind::UnexpectedToken, true, true, CstKind::ReturnStatement},
          {"EmbeddedAssignment", "const Broken = (Left = Right); const After = 1;", core::DiagnosticKind::ExpectedToken, true, true},
          {"ChainedAssignment", "func Broken() { Left = Middle = Right; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, true, true, CstKind::ReturnStatement},
          {"TopLevelComptimeValue", "comptime Value; const After = 1;", core::DiagnosticKind::ExpectedSyntax, true, true},
          {"StatementComptimeReturn", "func Broken() { comptime return; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, false, true, CstKind::ReturnStatement},
      };

      for (const MalformedSyntaxCase &TestCase : Cases)
      {
        expectMalformedSyntax(TestCase);
      }
    }

    // Verifies every ordinary comma-list family rejects a trailing comma while retaining the complete following declaration.
    TEST(ParserMalformedSyntaxRobustnessTest, RejectsTrailingCommasAcrossOrdinaryLists)
    {
      const std::vector<MalformedSyntaxCase> Cases = {
          {"AttributeList", "[reflect,] func Broken() {} const After = 1;", core::DiagnosticKind::TrailingComma, true, false},
          {"InheritanceList", "class Broken : Base, {} const After = 1;", core::DiagnosticKind::TrailingComma, true, false},
          {"ConstructorInitializerList", "func Broken() : Base(), {} const After = 1;", core::DiagnosticKind::TrailingComma, true, false},
          {"EnumPayloadList", "enum Broken { Item(i32,) } const After = 1;", core::DiagnosticKind::TrailingComma, true, false},
          {"ArrayElementList", "const Broken = [1,]; const After = 1;", core::DiagnosticKind::TrailingComma, true, false},
          {"TupleValueList", "const Broken = (1, 2,); const After = 1;", core::DiagnosticKind::TrailingComma, true, false},
          {"TupleTypeList", "var Broken: (i32, u32,); const After = 1;", core::DiagnosticKind::TrailingComma, true, false},
          {"FunctionTypeParameterList", "var Broken: func(i32,); const After = 1;", core::DiagnosticKind::TrailingComma, true, false},
      };

      for (const MalformedSyntaxCase &TestCase : Cases)
      {
        expectMalformedSyntax(TestCase);
      }
    }

    // Verifies both prefix and postfix increment/decrement spellings use the reserved-sequence diagnostic and do not disrupt later parsing.
    TEST(ParserMalformedSyntaxRobustnessTest, RejectsReservedIncrementAndDecrementSequences)
    {
      const std::vector<MalformedSyntaxCase> Cases = {
          {"PrefixIncrement", "func Broken() { ++Value; return; } const After = 1;", core::DiagnosticKind::ReservedSymbolSequence, true, false, CstKind::ReturnStatement},
          {"PrefixDecrement", "func Broken() { --Value; return; } const After = 1;", core::DiagnosticKind::ReservedSymbolSequence, true, false, CstKind::ReturnStatement},
          {"PostfixIncrement", "func Broken() { Value++; return; } const After = 1;", core::DiagnosticKind::ReservedSymbolSequence, true, false, CstKind::ReturnStatement},
          {"PostfixDecrement", "func Broken() { Value--; return; } const After = 1;", core::DiagnosticKind::ReservedSymbolSequence, true, false, CstKind::ReturnStatement},
      };

      for (const MalformedSyntaxCase &TestCase : Cases)
      {
        expectMalformedSyntax(TestCase);
      }
    }

    // Verifies statement entry commits a bare match to statement grammar while parentheses force the same value-form match into an expression statement.
    TEST(ParserMalformedSyntaxRobustnessTest, DistinguishesBareAndParenthesizedMatchValues)
    {
      expectMalformedSyntax({"BareMatchAtStatementEntry", "func Broken() { match (Value) { _ => 1, }; return; } const After = 1;", core::DiagnosticKind::ExpectedToken, true, true, CstKind::MatchStatement});

      const std::string Source = "func Valid() { (match (Value) { _ => 1, }); } const After = 1;";
      const ParsedFile First = parseSource(Source);
      const ParsedFile Second = parseSource(Source);

      ASSERT_TRUE(First.succeeded());
      EXPECT_TRUE(test::testDiagnostics(First).empty());
      EXPECT_TRUE(hasKind(First, CstKind::ExpressionStatement));
      EXPECT_TRUE(hasKind(First, CstKind::MatchExpression));
      EXPECT_FALSE(hasKind(First, CstKind::MatchStatement));
      EXPECT_TRUE(containsRecoveredAfterDeclaration(First));
      EXPECT_EQ(First.cst().nodes(), Second.cst().nodes());
      EXPECT_EQ(First.cst().children(), Second.cst().children());
      EXPECT_TRUE(test::diagnosticsEqual(First, Second));
      expectFullFidelity(First);
      expectFullFidelity(Second);
    }

    // Verifies configured syntax-depth exhaustion reports its dedicated diagnostic, preserves an Error node, and resumes after the bounded expression.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversAfterSyntaxNestingLimit)
    {
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 2;
      const std::string Source = "const Broken = [[0]]; const After = 1;";
      const ParsedFile First = parseSource(Source, Options);
      const ParsedFile Second = parseSource(Source, Options);

      EXPECT_FALSE(First.succeeded());
      EXPECT_EQ(First.completeness(), ParseCompleteness::Complete);
      EXPECT_TRUE(hasDiagnostic(First, core::DiagnosticKind::SyntaxNestingLimit));
      EXPECT_TRUE(hasKind(First, CstKind::Error));
      EXPECT_TRUE(hasFlag(First.cst().node(First.cst().root()).Flags, CstNodeFlags::HasError));
      EXPECT_TRUE(containsRecoveredAfterDeclaration(First));
      EXPECT_EQ(First.cst().nodes(), Second.cst().nodes());
      EXPECT_EQ(First.cst().children(), Second.cst().children());
      EXPECT_TRUE(test::diagnosticsEqual(First, Second));
      expectFullFidelity(First);
      expectFullFidelity(Second);
    }
  } // namespace
} // namespace ink::parser
