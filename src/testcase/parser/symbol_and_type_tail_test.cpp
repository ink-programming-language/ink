#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    using test::countKind;
    using test::expectFullFidelity;
    using test::hasDiagnostic;
    using test::hasKind;
    using test::nodeTextsOfKind;
    using test::parseSource;

    bool containsText(const std::vector<std::string> &Values, const std::string &Expected)
    {
      return std::find(Values.begin(), Values.end(), Expected) != Values.end();
    }

    // Verifies longest-match grouping for every compound operator that participates in binary or assignment expressions.
    TEST(ParserSymbolSequenceTest, CompositeExpressionOperatorsRemainSingleCstTerms)
    {
      const std::vector<std::string> AssignmentOperators = {
          "=",
          "+=",
          "-=",
          "*=",
          "/=",
          "%=",
          "&=",
          "|=",
          "^=",
          "<<=",
          ">>=",
      };
      const std::vector<std::string> BinaryOperators = {
          "<=",
          ">=",
          "==",
          "!=",
          "<<",
          ">>",
          "&&",
          "||",
      };
      std::string Source = "func symbols() {";
      for (const std::string &Sequence : AssignmentOperators)
      {
        Source += " left " + Sequence + " right;";
      }
      for (const std::string &Sequence : BinaryOperators)
      {
        Source += " left " + Sequence + " right;";
      }
      Source += " }";

      const ParsedFile File = parseSource(Source);
      const std::vector<std::string> OperatorTexts = nodeTextsOfKind(File, CstKind::Operator);

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::AssignmentStatement), AssignmentOperators.size());
      for (const std::string &Sequence : AssignmentOperators)
      {
        SCOPED_TRACE(Sequence);
        EXPECT_TRUE(containsText(OperatorTexts, Sequence));
      }
      for (const std::string &Sequence : BinaryOperators)
      {
        SCOPED_TRACE(Sequence);
        EXPECT_TRUE(containsText(OperatorTexts, Sequence));
      }
      expectFullFidelity(File);
    }

    // Verifies the non-operator compound terminals for expansion, generic application, range, and pointer members.
    TEST(ParserSymbolSequenceTest, ContextualCompoundTerminalsSelectTheirDedicatedGrammar)
    {
      const ParsedFile File = parseSource("func terminals() { forward(...); expand(...arguments); Generic::<>; for (const item in begin..end) {} value->member; }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(hasKind(File, CstKind::ForwardAllArguments));
      EXPECT_TRUE(hasKind(File, CstKind::ListExpansion));
      EXPECT_TRUE(hasKind(File, CstKind::GenericArgumentClause));
      EXPECT_TRUE(hasKind(File, CstKind::ForStatement));
      EXPECT_TRUE(hasKind(File, CstKind::PointerMemberExpression));
      expectFullFidelity(File);
    }

    // Verifies that adjacent increment and decrement spellings are reserved errors rather than nested unary operators.
    TEST(ParserSymbolSequenceTest, ReservedIncrementAndDecrementSequencesRecoverAsWholeTerms)
    {
      const ParsedFile File = parseSource("func reserved() { ++value; --value; }");

      ASSERT_FALSE(File.succeeded());
      EXPECT_EQ(static_cast<std::size_t>(std::count_if(test::testDiagnostics(File).begin(), test::testDiagnostics(File).end(), [](const core::Diagnostic &Diagnostic)
                                                       {
                                                         return Diagnostic.Kind == core::DiagnosticKind::ReservedSymbolSequence;
                                                       })),
                2u);
      EXPECT_GE(countKind(File, CstKind::Error), 2u);
      expectFullFidelity(File);
    }

    // Verifies that trivia interrupts a compound terminal while trivia-separated unary and binary ampersands remain valid.
    TEST(ParserSymbolSequenceTest, TriviaPreventsCrossTokenCompoundMatching)
    {
      const ParsedFile SplitComparison = parseSource("func split() { left < /* gap */ = right; }");
      const ParsedFile SplitAmpersands = parseSource("func split() { left & &right; }");
      const std::vector<std::string> SplitAmpersandOperators = nodeTextsOfKind(SplitAmpersands, CstKind::Operator);

      EXPECT_FALSE(SplitComparison.succeeded());
      EXPECT_FALSE(containsText(nodeTextsOfKind(SplitComparison, CstKind::Operator), "<="));
      ASSERT_TRUE(SplitAmpersands.succeeded());
      EXPECT_FALSE(containsText(SplitAmpersandOperators, "&&"));
      EXPECT_EQ(static_cast<std::size_t>(std::count(SplitAmpersandOperators.begin(), SplitAmpersandOperators.end(), "&")), 2u);
      expectFullFidelity(SplitComparison);
      expectFullFidelity(SplitAmpersands);
    }

    // Verifies nested generic closers remain individual delimiters while ordinary greater-than operators work inside parentheses.
    TEST(ParserGenericArgumentTest, NestedClosersAndParenthesizedGreaterThanOperatorsAreUnambiguous)
    {
      const ParsedFile File = parseSource("func generics() { Map::<String, Vector::<i32>>; Predicate::<(N > 0), (N >= 0), (N >> 1)>; Generic::<>; }");
      const std::vector<std::string> ClauseTexts = nodeTextsOfKind(File, CstKind::GenericArgumentClause);
      const std::vector<std::string> OperatorTexts = nodeTextsOfKind(File, CstKind::Operator);

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::GenericArgumentClause), 4u);
      EXPECT_TRUE(containsText(ClauseTexts, "Map::<String, Vector::<i32>>"));
      EXPECT_TRUE(containsText(OperatorTexts, ">"));
      EXPECT_TRUE(containsText(OperatorTexts, ">="));
      EXPECT_TRUE(containsText(OperatorTexts, ">>"));
      expectFullFidelity(File);
    }

    // Verifies that only the adjacent three-character generic introducer commits to a generic argument clause.
    TEST(ParserGenericArgumentTest, SplitGenericIntroducerDoesNotCommit)
    {
      const ParsedFile File = parseSource("func split() { Generic:: <i32>; }");

      EXPECT_FALSE(File.succeeded());
      EXPECT_FALSE(hasKind(File, CstKind::GenericArgumentClause));
      expectFullFidelity(File);
    }

    struct TypeTailCase
    {
        const char *Name;
        const char *Source;
        std::size_t ExpectedTypeConstructors;
    };

    // Verifies type-constructor tail commitment at every caller-specific expression terminator defined by the grammar.
    TEST(ParserTypeConstructorTailTest, CommitsAtEveryRequiredEndSet)
    {
      const std::vector<TypeTailCase> Cases = {
          {"ReturnSemicolon", "func tail() { return T*[]; }", 1},
          {"ForRange", "func tail() { for (const item in T* .. U*) {} }", 2},
          {"SliceColonAndBracket", "func tail() { values[T*:U*]; }", 2},
          {"IfElse", "const Selected = if (condition) T* else U*;", 2},
          {"GenericCloser", "const Selected = Wrapper::<T*[N]>;", 1},
          {"EnumComma", "enum Kind { Pointer = T*, Other }", 1},
          {"AggregateComma", "const Selected = Record { field: T* };", 1},
          {"CallParenthesis", "func tail() { inspect(T&); }", 1},
      };

      for (const TypeTailCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile File = parseSource(TestCase.Source);
        EXPECT_TRUE(File.succeeded());
        EXPECT_EQ(countKind(File, CstKind::TypeConstructorExpression), TestCase.ExpectedTypeConstructors);
        expectFullFidelity(File);
      }
    }

    // Verifies failed maximal type-tail probes roll back completely to multiplicative, bitwise, or unary expression parsing.
    TEST(ParserTypeConstructorTailTest, RollsBackWhenAnOperandOrCallFollowsTheCandidateTail)
    {
      const ParsedFile File = parseSource("func rollback() { var x = T * n; var y = T & mask; var z = T**pointer; T*&reference; T*(value); }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::TypeConstructorExpression), 0u);
      EXPECT_GE(countKind(File, CstKind::BinaryExpression), 5u);
      EXPECT_GE(countKind(File, CstKind::UnaryExpression), 2u);
      expectFullFidelity(File);
    }

    // Verifies parentheses explicitly close type values before calls, members, generics, and later comparison operators.
    TEST(ParserTypeConstructorTailTest, ParenthesesAllowConstructedTypesToContinueThroughPostfixAndInfixSyntax)
    {
      const ParsedFile File = parseSource("func grouped() { (T*)(value); (T*).metadata; (T*)::<Argument>; (T*) == (U*); }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::TypeConstructorExpression), 5u);
      EXPECT_TRUE(hasKind(File, CstKind::CallExpression));
      EXPECT_TRUE(hasKind(File, CstKind::MemberExpression));
      EXPECT_TRUE(hasKind(File, CstKind::GenericArgumentClause));
      EXPECT_TRUE(containsText(nodeTextsOfKind(File, CstKind::Operator), "=="));
      expectFullFidelity(File);
    }

    // Verifies compound assignments are never split as type suffixes, while trivia before a plain equals permits type-valued left sides.
    TEST(ParserTypeConstructorTailTest, AssignmentOperatorsTakePriorityOverTypeSuffixProbes)
    {
      const ParsedFile File = parseSource("func assignments() { T*=value; T&=mask; T* = value; T& = value; }");
      const std::vector<std::string> OperatorTexts = nodeTextsOfKind(File, CstKind::Operator);

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::AssignmentStatement), 4u);
      EXPECT_EQ(countKind(File, CstKind::TypeConstructorExpression), 2u);
      EXPECT_TRUE(containsText(OperatorTexts, "*="));
      EXPECT_TRUE(containsText(OperatorTexts, "&="));
      EXPECT_EQ(static_cast<std::size_t>(std::count(OperatorTexts.begin(), OperatorTexts.end(), "=")), 2u);
      expectFullFidelity(File);
    }

    // Verifies explicit type syntax consumes adjacent pointer and reference suffix characters one at a time.
    TEST(ParserTypeConstructorTailTest, ExplicitTypeContextConsumesAdjacentSuffixCharactersIndividually)
    {
      const ParsedFile File = parseSource("func suffixes(value: Data&&) -> Result**;");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::ReferenceTypeSuffix), 2u);
      EXPECT_EQ(countKind(File, CstKind::PointerTypeSuffix), 2u);
      EXPECT_EQ(countKind(File, CstKind::TypeConstructorExpression), 0u);
      expectFullFidelity(File);
    }
  } // namespace
} // namespace ink::parser
