#include "parser_test_support.h"
namespace ink::parser::test
{
  // Multiplication binds more tightly than addition and subtraction associates left.
  TEST_F(ParserTest, PrecedenceAndLeftAssociation)
  {
    const auto Result = read("a + b * c - d - e;");
    ASSERT_TRUE(Result.succeeded());
    const auto *Outer = cast<BinaryExpr>(expression(Result));
    EXPECT_EQ(Outer->op(), TokenKind::Minus);
    const auto *Middle = cast<BinaryExpr>(Outer->left());
    EXPECT_EQ(Middle->op(), TokenKind::Minus);
    const auto *Add = cast<BinaryExpr>(Middle->left());
    EXPECT_EQ(Add->op(), TokenKind::Plus);
    EXPECT_EQ(cast<BinaryExpr>(Add->right())->op(), TokenKind::Star);
  }
  // Every Pratt precedence boundary produces the expected nested binary tree.
  TEST_F(ParserTest, EveryBinaryPrecedence)
  {
    const std::vector<std::pair<std::string, TokenKind>> Operators = {
        {"??", TokenKind::NullCoalesce},
        {"||", TokenKind::PipePipe},
        {"&&", TokenKind::AmpAmp},
        {"|", TokenKind::Pipe},
        {"^", TokenKind::Caret},
        {"&", TokenKind::Amp},
        {"==", TokenKind::EqualEqual},
        {"<", TokenKind::Less},
        {"<<", TokenKind::ShiftLeft},
        {"+", TokenKind::Plus},
        {"*", TokenKind::Star},
    };
    for (std::size_t Index = 1; Index < Operators.size(); ++Index)
    {
      const auto Source = "a " + Operators[Index - 1].first + " b " + Operators[Index].first + " c;";
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      ASSERT_TRUE(Result.succeeded());
      const auto *Outer = cast<BinaryExpr>(expression(Result));
      EXPECT_EQ(Outer->op(), Operators[Index - 1].second);
      EXPECT_EQ(cast<BinaryExpr>(Outer->right())->op(), Operators[Index].second);
    }
  }

  // Null coalescing and conditional tails associate right, with nested middle colons owned locally.
  TEST_F(ParserTest, RightAssociativeOperators)
  {
    const auto Result = read("a ?? b ?? c; a ? b ? c : d : e ? f : g;");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_TRUE(isa<BinaryExpr>(cast<BinaryExpr>(expression(Result))->right()));
    const auto *Conditional = cast<ConditionalExpr>(expression(Result, 1));
    EXPECT_TRUE(isa<ConditionalExpr>(Conditional->then()));
    EXPECT_TRUE(isa<ConditionalExpr>(Conditional->elseValue()));
  }

  // Comptime consumes one unary expression while parentheses explicitly widen its operand.
  TEST_F(ParserTest, ComptimeUnaryScope)
  {
    const auto Result = read("comptime a + b; comptime(a + b); comptime if (x) {}");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_TRUE(isa<ComptimeExpr>(cast<BinaryExpr>(expression(Result))->left()));
    EXPECT_TRUE(isa<ParenExpr>(cast<ComptimeExpr>(expression(Result, 1))->operand()));
    EXPECT_TRUE(isa<IfStmt>(cast<ComptimeStmt>(Result.Unit->root()->statements()[2])->body()));
  }

  // Assignment chains belong to SimpleItem and commas divide independent items.
  TEST_F(ParserTest, AssignmentItems)
  {
    const auto Result = read("a = b = c; a, b = c, d; a + b = c;");
    ASSERT_TRUE(Result.succeeded());
    const auto *First = cast<AssignmentItem>(cast<SimpleStmt>(Result.Unit->root()->statements()[0])->items()[0]);
    EXPECT_TRUE(isa<AssignmentItem>(First->right()));
    const auto *Second = cast<SimpleStmt>(Result.Unit->root()->statements()[1]);
    ASSERT_EQ(Second->items().size(), 3U);
    EXPECT_TRUE(isa<ExprItem>(Second->items()[0]));
    EXPECT_TRUE(isa<AssignmentItem>(Second->items()[1]));
    EXPECT_TRUE(isa<ExprItem>(Second->items()[2]));
    EXPECT_TRUE(isa<BinaryExpr>(cast<AssignmentItem>(cast<SimpleStmt>(Result.Unit->root()->statements()[2])->items()[0])->left()));
  }

  // Named and spread arguments retain distinct tags, and assignment is forbidden inside grouped arguments.
  TEST_F(ParserTest, ArgumentForms)
  {
    const auto Result = read("foo(a = 1, xs..., 3);");
    ASSERT_TRUE(Result.succeeded());
    const auto Arguments = cast<CallExpr>(expression(Result))->arguments();
    ASSERT_EQ(Arguments.size(), 3U);
    EXPECT_EQ(Arguments[0].form(), ArgumentKind::Named);
    EXPECT_EQ(Arguments[0].name()->Text, "a");
    EXPECT_EQ(Arguments[1].form(), ArgumentKind::SpreadPositional);
    EXPECT_EQ(Arguments[2].form(), ArgumentKind::Positional);
    EXPECT_FALSE(read("foo((a = 1));").succeeded());
    EXPECT_FALSE(read("var x = a = b;").succeeded());
    EXPECT_FALSE(read("foo(a = xs...);").succeeded());
  }

  // Postfix operations preserve source order, optional access modes and generic arguments.
  TEST_F(ParserTest, PostfixChain)
  {
    const auto Result = read("a::[T](x)?.[i]?.member->next++;");
    ASSERT_TRUE(Result.succeeded());
    const auto *Update = cast<PostfixUpdateExpr>(expression(Result));
    const auto *Arrow = cast<MemberExpr>(Update->operand());
    EXPECT_EQ(Arrow->access(), TokenKind::Arrow);
    const auto *Member = cast<MemberExpr>(Arrow->object());
    EXPECT_EQ(Member->access(), TokenKind::QuestionDot);
    const auto *Index = cast<IndexExpr>(Member->object());
    EXPECT_TRUE(Index->optional());
    EXPECT_TRUE(isa<GenericApplyExpr>(cast<CallExpr>(Index->object())->callee()));
    const auto OptionalCall = read("f?.(x);");
    ASSERT_TRUE(OptionalCall.succeeded());
    EXPECT_TRUE(cast<CallExpr>(expression(OptionalCall))->optional());
  }

  // Grouping, empty tuples, singleton tuples, arrays and repeat arrays retain different node kinds.
  TEST_F(ParserTest, TupleArrayForms)
  {
    const auto Result = read("(); (x); (x,); (x,y); []; [x,y]; [do { yield 1; }; n];");
    ASSERT_TRUE(Result.succeeded());
    EXPECT_TRUE(isa<TupleExpr>(expression(Result, 0)));
    EXPECT_TRUE(isa<ParenExpr>(expression(Result, 1)));
    EXPECT_EQ(cast<TupleExpr>(expression(Result, 2))->elements().size(), 1U);
    EXPECT_EQ(cast<TupleExpr>(expression(Result, 3))->elements().size(), 2U);
    EXPECT_TRUE(cast<ArrayExpr>(expression(Result, 4))->elements().empty());
    EXPECT_EQ(cast<ArrayExpr>(expression(Result, 5))->elements().size(), 2U);
    EXPECT_TRUE(isa<BlockExpr>(cast<ArrayRepeatExpr>(expression(Result, 6))->value()));
    for (const auto *Source : {"(x,y,);", "[x,];", "f(x,);", "f::[T,];"})
    {
      SCOPED_TRACE(Source);
      EXPECT_FALSE(read(Source).succeeded());
    }
  }

  // Each unary, binary and assignment token accepted by the BNF reaches the parser.
  TEST_F(ParserTest, OperatorCoverage)
  {
    for (const auto *Op : {"+", "-", "!", "~", "++", "--", "&", "*"})
    {
      EXPECT_TRUE(read(std::string(Op) + "x;").succeeded()) << Op;
    }
    for (const auto *Op : {"??", "||", "&&", "|", "^", "&", "==", "!=", "<", "<=", ">", ">=", "<<", ">>", "+", "-", "*", "/", "%"})
    {
      EXPECT_TRUE(read("a " + std::string(Op) + " b;").succeeded()) << Op;
    }
    for (const auto *Op : {"=", "+=", "-=", "*=", "/=", "%=", "<<=", ">>=", "&=", "^=", "|="})
    {
      EXPECT_TRUE(read("a " + std::string(Op) + " b;").succeeded()) << Op;
    }
  }
} // namespace ink::parser::test
