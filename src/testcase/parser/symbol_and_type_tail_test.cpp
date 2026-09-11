#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    // Verifies every adjacent precedence level binds correctly with the tighter operator on either side of the looser operator.
    TEST(ParserSymbolSequenceTest, BinaryPrecedenceLevelsPreserveBothOperandOrders)
    {
      const std::vector<std::string> Operators = {"||", "&&", "|", "^", "&", "==", "<", "<<", "+", "*"};
      for (std::size_t Index = 1; Index < Operators.size(); ++Index)
      {
        const std::string &Lower = Operators[Index - 1];
        const std::string &Higher = Operators[Index];
        for (bool HigherFirst : {false, true})
        {
          const std::string Source = "A " + (HigherFirst ? Higher : Lower) + " B " + (HigherFirst ? Lower : Higher) + " C;";
          SCOPED_TRACE(Source);
          const ParsedFile File = test::parseSource(Source);
          ASSERT_TRUE(File.succeeded());
          const AstNodeId StatementId = File.ast().node(File.ast().root()).get<SourceFile>().Statements[0];
          const AstNodeId ExpressionId = File.ast().node(StatementId).get<ExpressionStatement>().Expression;
          const BinaryExpression &Expression = File.ast().node(ExpressionId).get<BinaryExpression>();
          EXPECT_EQ(tokenizer::symbolSpelling(Expression.Operator), Lower);
          const BinaryExpression &Tighter = File.ast().node(HigherFirst ? Expression.Left : Expression.Right).get<BinaryExpression>();
          EXPECT_EQ(tokenizer::symbolSpelling(Tighter.Operator), Higher);
          test::expectAstIntegrity(File);
        }
      }
    }

    // Verifies compound operators become typed binary operator attributes with named left and right operands.
    TEST(ParserSymbolSequenceTest, CompositeExpressionOperatorsBecomeTypedAttributes)
    {
      const std::vector<std::string> Operators = {"||", "&&", "==", "!=", "<=", ">=", "<<", ">>"};
      for (const std::string &Operator : Operators)
      {
        SCOPED_TRACE(Operator);
        const ParsedFile File = test::parseSource("Left " + Operator + " Right;");
        ASSERT_TRUE(File.succeeded());
        ASSERT_EQ(test::countKind(File, AstKind::BinaryExpression), 1u);
        const AstNodeId Id = test::nodesOfKind(File, AstKind::BinaryExpression)[0];
        const BinaryExpression &Binary = File.ast().node(Id).get<BinaryExpression>();
        EXPECT_EQ(tokenizer::symbolSpelling(Binary.Operator), Operator);
        EXPECT_EQ(test::nodeText(File, Binary.Left), "Left");
        EXPECT_EQ(test::nodeText(File, Binary.Right), "Right");
        test::expectAstIntegrity(File);
      }
    }

    // Verifies every assignment operator is an attribute of a statement with direct target and value fields.
    TEST(ParserSymbolSequenceTest, ParsesEveryAssignmentOperator)
    {
      const std::vector<std::string> Operators = {"=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>="};
      for (const std::string &Operator : Operators)
      {
        SCOPED_TRACE(Operator);
        const ParsedFile File = test::parseSource("Target " + Operator + " Value;");
        ASSERT_TRUE(File.succeeded());
        EXPECT_EQ(test::countKind(File, AstKind::AssignmentStatement), 1u);
        const AstNodeId Id = test::nodesOfKind(File, AstKind::AssignmentStatement)[0];
        const AssignmentStatement &Assignment = File.ast().node(Id).get<AssignmentStatement>();
        EXPECT_EQ(tokenizer::symbolSpelling(Assignment.Operator), Operator);
        EXPECT_EQ(test::nodeText(File, Assignment.Target), "Target");
        EXPECT_EQ(test::nodeText(File, Assignment.Value), "Value");
        test::expectAstIntegrity(File);
      }
    }

    // Verifies comments and whitespace cannot merge separate lexer symbols into compound syntax.
    TEST(ParserSymbolSequenceTest, TriviaPreventsCrossTokenCompoundMatching)
    {
      const std::vector<std::string> Invalid = {"Left < /* gap */ = Right;", "Left : : [T];", "func f() - > void;", "Left -> /* gap */ ;"};
      for (const std::string &Source : Invalid)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = test::parseSource(Source);
        EXPECT_FALSE(File.succeeded());
        test::expectAstIntegrity(File);
      }
      const ParsedFile Valid = test::parseSource("Left + +Right; Left - -Right; ++Value; --Value;");
      EXPECT_TRUE(Valid.succeeded());
      EXPECT_EQ(test::countKind(Valid, AstKind::UnaryExpression), 6u);
      test::expectAstIntegrity(Valid);
    }

    // Verifies square generic arguments keep comparisons and shifts unambiguous, including nested instantiations.
    TEST(ParserGenericArgumentTest, NestedClosersComparisonsAndShiftsAreUnambiguous)
    {
      const std::vector<std::string> Sources = {
          "Outer::[Inner::[T]](value);",
          "Factory::[N > 0, M >= 0, Bits >> 1, A < B, X << 2];",
          "Factory::[Ready ? T : U, makeType(), (T, U), Args...];",
          "Factory::[Args...];",
          "Object::[T]::[U].method::[V](first, rest...);",
      };
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = test::parseSource(Source);
        ASSERT_TRUE(File.succeeded());
        EXPECT_TRUE(test::hasKind(File, AstKind::GenericInstantiationExpression));
        test::expectAstIntegrity(File);
      }
    }

    // Verifies empty generic arguments, old angle arguments, and misplaced pack expansions are rejected.
    TEST(ParserGenericArgumentTest, RejectsOldDelimitersEmptyArgumentsAndMisplacedPacks)
    {
      const std::vector<std::string> Sources = {"F::[];", "F::<T>;", "F::[T,];", "F::[Args..., T];", "F(...Args);", "F(Args..., Value);", "F(Value,);"};
      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = test::parseSource(Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_FALSE(test::testDiagnostics(File).empty());
        test::expectAstIntegrity(File);
      }
    }

    // Verifies explicit ref and ptr prefixes replace terminal type-suffix speculation in every expression context.
    TEST(ParserTypeConstructorTailTest, UsesPrefixTypesWithoutSuffixSpeculation)
    {
      const ParsedFile File = test::parseSource("ptr T; const ptr T; ref T; const ref T; let Value: ptr makeType() = Input; func f() -> ref T;");
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(test::countKind(File, AstKind::PointerTypeExpression), 3u);
      EXPECT_EQ(test::countKind(File, AstKind::ReferenceTypeExpression), 3u);
      for (AstNodeId Id : test::nodesOfKind(File, AstKind::PointerTypeExpression))
      {
        EXPECT_NE(File.ast().node(Id).get<PointerTypeExpression>().Operand, InvalidAstNodeId);
      }
      test::expectAstIntegrity(File);
    }

    // Verifies ordinary binary operators retain their meaning in type syntax and cannot become postfix pointer or reference suffixes.
    TEST(ParserTypeConstructorTailTest, KeepsStarAndAmpersandAsOrdinaryOperators)
    {
      const ParsedFile Valid = test::parseSource("T * []; T & []; let Value: A * B = Input;");
      ASSERT_TRUE(Valid.succeeded());
      EXPECT_EQ(test::countKind(Valid, AstKind::BinaryExpression), 3u);
      EXPECT_EQ(test::countKind(Valid, AstKind::PointerTypeExpression), 0u);
      EXPECT_EQ(test::countKind(Valid, AstKind::ReferenceTypeExpression), 0u);
      test::expectAstIntegrity(Valid);
      for (const std::string Source : {"T*;", "T&;", "let Value: T* = Input;", "func f(Value: T&) -> void;"})
      {
        const ParsedFile Invalid = test::parseSource(Source);
        EXPECT_FALSE(Invalid.succeeded());
        test::expectAstIntegrity(Invalid);
      }
    }

    // Verifies equal-precedence comparison chains are rejected while explicitly grouped or lower-precedence-separated comparisons remain valid.
    TEST(ParserSymbolSequenceTest, EnforcesNonAssociativeComparisons)
    {
      const std::vector<std::string> Invalid = {"A < B < C;", "A < B >= C;", "A == B != C;", "A < B == C < D < E;", "A == B < C == D;"};
      for (const std::string &Source : Invalid)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = test::parseSource(Source);
        EXPECT_FALSE(File.succeeded());
        test::expectAstIntegrity(File);
      }
      const std::vector<std::string> Valid = {"(A < B) < C;", "A < (B < C);", "A < B == C < D;", "A < B && C < D;", "A == B | C == D;"};
      for (const std::string &Source : Valid)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = test::parseSource(Source);
        EXPECT_TRUE(File.succeeded());
        test::expectAstIntegrity(File);
      }
    }
  } // namespace
} // namespace ink::parser
