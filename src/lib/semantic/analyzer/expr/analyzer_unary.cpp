#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  Analyzer::ExpressionResult Analyzer::analyzeUnaryExpr(AnalysisState &State, const parser::UnaryExpr &Node, std::size_t Depth)
  {
    if (Node.op() == tokenizer::TokenKind::Plus || Node.op() == tokenizer::TokenKind::Minus)
    {
      ExpressionResult Result = analyzeExpr(State, *Node.operand(), Depth + 1);
      if (!Result)
      {
        return {};
      }
      if (Result.IntegerLiteral)
      {
        Result.Negative = Result.Negative != (Node.op() == tokenizer::TokenKind::Minus);
        return Result;
      }
    }
    reportUnsupported(State, Node);
    return {};
  }
} // namespace ink::semantic
