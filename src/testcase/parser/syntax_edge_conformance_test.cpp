#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    using test::countKind;
    using test::expectAstIntegrity;
    using test::nodesOfKind;
    using test::nodeText;
    using test::parseSource;

    std::vector<AstNodeId> directChildrenOfKind(const ParsedFile &File, AstNodeId Parent, AstKind Kind)
    {
      std::vector<AstNodeId> Result;
      for (const AstChildEdge &Child : File.ast().children(Parent))
      {
        if (File.ast().node(Child.Id).kind() == Kind)
        {
          Result.push_back(Child.Id);
        }
      }
      return Result;
    }

    std::vector<AstNodeId> nodesOfKindWithText(const ParsedFile &File, AstKind Kind, const std::string &ExpectedText)
    {
      std::vector<AstNodeId> Result;
      for (AstNodeId Id : nodesOfKind(File, Kind))
      {
        if (nodeText(File, Id) == ExpectedText)
        {
          Result.push_back(Id);
        }
      }
      return Result;
    }

    bool isDescendant(const ParsedFile &File, AstNodeId Ancestor, AstNodeId Candidate)
    {
      std::vector<AstNodeId> Work = {Ancestor};
      while (!Work.empty())
      {
        const AstNodeId Parent = Work.back();
        Work.pop_back();
        for (const AstChildEdge &Child : File.ast().children(Parent))
        {
          if (Child.Id == Candidate)
          {
            return true;
          }
          Work.push_back(Child.Id);
        }
      }
      return false;
    }

    std::size_t countDescendantsOfKind(const ParsedFile &File, AstNodeId Ancestor, AstKind Kind)
    {
      std::size_t Result = 0;
      for (AstNodeId Candidate : nodesOfKind(File, Kind))
      {
        if (isDescendant(File, Ancestor, Candidate))
        {
          ++Result;
        }
      }
      return Result;
    }

    void expectSuccessfulCompleteParse(const ParsedFile &File)
    {
      EXPECT_TRUE(File.succeeded());
      EXPECT_TRUE(test::testDiagnostics(File).empty());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      expectAstIntegrity(File);
    }

    // Verifies calls and construction both expose callee and arguments, including computed type targets and generic postfixes.
    TEST(ParserSyntaxEdgeConformanceTest, UnifiesCallsAndConstruction)
    {
      const ParsedFile File = parseSource("f(1, 2); Point(1, 2); Point::[T](value); makeType()(value); (SelectedType)(value).field;");
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, AstKind::CallExpression), 6u);
      EXPECT_EQ(nodesOfKindWithText(File, AstKind::CallExpression, "f(1, 2)").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, AstKind::CallExpression, "Point(1, 2)").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, AstKind::CallExpression, "makeType()(value)").size(), 1u);
      const AstNodeId ComputedId = nodesOfKindWithText(File, AstKind::CallExpression, "makeType()(value)")[0];
      const CallExpression &Computed = File.ast().node(ComputedId).get<CallExpression>();
      EXPECT_EQ(File.ast().node(Computed.Callee).kind(), AstKind::CallExpression);
      ASSERT_EQ(Computed.Arguments.size(), 1u);
      EXPECT_EQ(nodeText(File, Computed.Arguments[0].Expression), "value");
      expectSuccessfulCompleteParse(File);
    }

    // Verifies grouping, multi-element tuples, and nested binding patterns stay distinct without type lookup.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesTupleAndGroupingStructure)
    {
      const ParsedFile File = parseSource("let (First, (Second, _)): (T, (U, V)) = (first, (second, third)); (value); []; (T, U);");
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, AstKind::TuplePattern), 2u);
      EXPECT_EQ(countKind(File, AstKind::WildcardPattern), 1u);
      EXPECT_EQ(countKind(File, AstKind::TupleExpression), 5u);
      EXPECT_EQ(countKind(File, AstKind::ParenthesizedExpression), 1u);
      EXPECT_EQ(countKind(File, AstKind::ArrayExpression), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies the nested class owns its generated field syntactically through the comptime branch.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesNestedClassAndComptimeFieldOwnership)
    {
      const ParsedFile File = parseSource("class C { class D { comptime if (Enabled) { class_field Value: int32; } } }");
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, AstKind::ClassDeclaration), 2u);
      ASSERT_EQ(countKind(File, AstKind::ClassFieldDeclaration), 1u);
      const std::vector<AstNodeId> NestedClasses = nodesOfKindWithText(File, AstKind::ClassDeclaration, "class D { comptime if (Enabled) { class_field Value: int32; } }");
      ASSERT_EQ(NestedClasses.size(), 1u);
      const AstNodeId Nested = NestedClasses[0];
      const AstNodeId Field = nodesOfKind(File, AstKind::ClassFieldDeclaration)[0];
      EXPECT_TRUE(isDescendant(File, Nested, Field));
      EXPECT_EQ(countDescendantsOfKind(File, Nested, AstKind::ComptimeStatement), 1u);
      EXPECT_EQ(countDescendantsOfKind(File, Nested, AstKind::IfStatement), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies fixed precedence from conditional through postfix operators and left/right associativity using child relationships.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesCompleteExpressionPrecedenceAndAssociativity)
    {
      const ParsedFile File = parseSource("Condition ? A || B && C | D ^ E & F == G < H << I + J * -K.member() : Other; A - B - C; !~-Value;");
      const std::vector<std::string> OrderedBinaryTexts = {
          "A || B && C | D ^ E & F == G < H << I + J * -K.member()",
          "B && C | D ^ E & F == G < H << I + J * -K.member()",
          "C | D ^ E & F == G < H << I + J * -K.member()",
          "D ^ E & F == G < H << I + J * -K.member()",
          "E & F == G < H << I + J * -K.member()",
          "F == G < H << I + J * -K.member()",
          "G < H << I + J * -K.member()",
          "H << I + J * -K.member()",
          "I + J * -K.member()",
          "J * -K.member()",
      };
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, AstKind::BinaryExpression), 12u);
      for (std::size_t Index = 0; Index < OrderedBinaryTexts.size(); ++Index)
      {
        SCOPED_TRACE(OrderedBinaryTexts[Index]);
        const std::vector<AstNodeId> Matching = nodesOfKindWithText(File, AstKind::BinaryExpression, OrderedBinaryTexts[Index]);
        ASSERT_EQ(Matching.size(), 1u);
        if (Index + 1 < OrderedBinaryTexts.size())
        {
          EXPECT_EQ(directChildrenOfKind(File, Matching[0], AstKind::BinaryExpression), nodesOfKindWithText(File, AstKind::BinaryExpression, OrderedBinaryTexts[Index + 1]));
        }
      }
      const std::vector<AstNodeId> Subtraction = nodesOfKindWithText(File, AstKind::BinaryExpression, "A - B - C");
      ASSERT_EQ(Subtraction.size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Subtraction[0], AstKind::BinaryExpression), nodesOfKindWithText(File, AstKind::BinaryExpression, "A - B"));
      const BinaryExpression &OuterSubtraction = File.ast().node(Subtraction[0]).get<BinaryExpression>();
      EXPECT_EQ(nodeText(File, OuterSubtraction.Left), "A - B");
      EXPECT_EQ(nodeText(File, OuterSubtraction.Right), "C");
      EXPECT_EQ(tokenizer::symbolSpelling(OuterSubtraction.Operator), "-");
      EXPECT_TRUE(nodesOfKindWithText(File, AstKind::BinaryExpression, "B - C").empty());
      const std::vector<AstNodeId> Unary = nodesOfKindWithText(File, AstKind::UnaryExpression, "!~-Value");
      ASSERT_EQ(Unary.size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Unary[0], AstKind::UnaryExpression), nodesOfKindWithText(File, AstKind::UnaryExpression, "~-Value"));
      EXPECT_EQ(nodeText(File, File.ast().node(Unary[0]).get<UnaryExpression>().Operand), "~-Value");
      expectSuccessfulCompleteParse(File);
    }

    // Verifies both conditional branches may contain another conditional and the false branch associates right.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesConditionalBranchOwnership)
    {
      const ParsedFile File = parseSource("A ? B ? C : D : E ? F : G;");
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, AstKind::ConditionalExpression), 3u);
      const std::vector<AstNodeId> Outer = nodesOfKindWithText(File, AstKind::ConditionalExpression, "A ? B ? C : D : E ? F : G");
      ASSERT_EQ(Outer.size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Outer[0], AstKind::ConditionalExpression).size(), 2u);
      const ConditionalExpression &Conditional = File.ast().node(Outer[0]).get<ConditionalExpression>();
      EXPECT_EQ(nodeText(File, Conditional.Condition), "A");
      EXPECT_EQ(nodeText(File, Conditional.Then), "B ? C : D");
      EXPECT_EQ(nodeText(File, Conditional.Else), "E ? F : G");
      expectSuccessfulCompleteParse(File);
    }

    // Verifies unary type boundaries leave outer binary operators outside prefix and function-type nodes.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesUnaryTypeOperandBoundaries)
    {
      const ParsedFile File = parseSource("ptr T + U; ref T & U; func() -> T + U; ptr (Ready ? T : U); (func() -> T)(value);");
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(nodesOfKindWithText(File, AstKind::PointerTypeExpression, "ptr T").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, AstKind::ReferenceTypeExpression, "ref T").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, AstKind::FunctionTypeExpression, "func() -> T").size(), 2u);
      EXPECT_EQ(countKind(File, AstKind::CallExpression), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies comptime at statement start covers the whole assignment or expression but expression-position comptime covers one unary operand.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesComptimeStatementAndExpressionScopes)
    {
      const ParsedFile File = parseSource("comptime A + B; comptime X = Y; A + comptime B; comptime if (A) {} else if (B) {} else {}");
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, AstKind::ComptimeStatement), 3u);
      EXPECT_EQ(countKind(File, AstKind::ComptimeExpression), 1u);
      const std::vector<AstNodeId> Assignment = nodesOfKindWithText(File, AstKind::ComptimeStatement, "comptime X = Y;");
      ASSERT_EQ(Assignment.size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Assignment[0], AstKind::AssignmentStatement).size(), 1u);
      const ComptimeStatement &ComptimeAssignment = File.ast().node(Assignment[0]).get<ComptimeStatement>();
      const AssignmentStatement &InnerAssignment = File.ast().node(ComptimeAssignment.Statement).get<AssignmentStatement>();
      EXPECT_EQ(nodeText(File, InnerAssignment.Target), "X");
      EXPECT_EQ(nodeText(File, InnerAssignment.Value), "Y");
      const ComptimeExpression &UnaryComptime = File.ast().node(nodesOfKind(File, AstKind::ComptimeExpression)[0]).get<ComptimeExpression>();
      EXPECT_EQ(nodeText(File, UnaryComptime.Operand), "B");
      const std::vector<AstNodeId> Branch = nodesOfKindWithText(File, AstKind::ComptimeStatement, "comptime if (A) {} else if (B) {} else {}");
      ASSERT_EQ(Branch.size(), 1u);
      EXPECT_EQ(countDescendantsOfKind(File, Branch[0], AstKind::IfStatement), 2u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies else-if is represented by a nested if and the terminal else block belongs to that nested statement.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesElseIfOwnership)
    {
      const ParsedFile File = parseSource("if (First) {} else if (Second) {} else {}");
      ASSERT_TRUE(File.succeeded());
      const std::vector<AstNodeId> Outer = nodesOfKindWithText(File, AstKind::IfStatement, "if (First) {} else if (Second) {} else {}");
      const std::vector<AstNodeId> Nested = nodesOfKindWithText(File, AstKind::IfStatement, "if (Second) {} else {}");
      ASSERT_EQ(Outer.size(), 1u);
      ASSERT_EQ(Nested.size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Outer[0], AstKind::IfStatement), Nested);
      const IfStatement &OuterIf = File.ast().node(Outer[0]).get<IfStatement>();
      EXPECT_EQ(nodeText(File, OuterIf.Condition), "First");
      EXPECT_EQ(OuterIf.Else, Nested[0]);
      const IfStatement &NestedIf = File.ast().node(Nested[0]).get<IfStatement>();
      EXPECT_EQ(nodeText(File, NestedIf.Condition), "Second");
      EXPECT_EQ(File.ast().node(NestedIf.Else).kind(), AstKind::BlockStatement);
      EXPECT_EQ(directChildrenOfKind(File, Outer[0], AstKind::BlockStatement).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Nested[0], AstKind::BlockStatement).size(), 2u);
      expectSuccessfulCompleteParse(File);
    }
  } // namespace
} // namespace ink::parser
