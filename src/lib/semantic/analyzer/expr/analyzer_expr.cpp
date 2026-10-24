#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/analyzer/block_depth_guard.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::emitValue(AnalyzerState &State, const frontend::parser::Expr &Node, ExprHints Hints)
  {
    const ir::Value *Value = analyzeExpr(State, Node, Hints);
    RETURN_IF(!Value, nullptr);
    return Value->isStorage() ? loadStorage(State, State.at(Node), *Value) : Value;
  }

  const ir::Value *Analyzer::emitRuntimeValue(AnalyzerState &State, const frontend::parser::Expr &Node, ExprHints Hints)
  {
    return emitRuntimeValue(State, State.at(Node), Node, Hints);
  }

  const ir::Value *Analyzer::emitRuntimeValue(AnalyzerState &State, const AnalysisSite &Site, const frontend::parser::Expr &Node, ExprHints Hints)
  {
    const ir::Value *Value = emitValue(State, Node, Hints);
    return requireRuntimeValue(Site, Value);
  }

  const ir::Value *Analyzer::analyzeExpr(AnalyzerState &State, const frontend::parser::Expr &Node, ExprHints Hints)
  {
    auto &Context = State.semanticContext();
    const auto Site = State.at(Node);
    CHECK_ICE(Site, State.recursionDepth() < State.recursionDepthLimit(), core::DiagnosticKind::SemanticNestingLimit);
    BlockDepthGuard DepthGuard(State.recursionDepth());
    CHECK_ICE(Site, !Node.isComptime(), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    const ir::Type *ExpectedType = Hints.ExpectedType;
    CHECK_ICE(Site, !ExpectedType || Context.irContext().containsType(ExpectedType), core::DiagnosticKind::SemanticConstructionFailed);
    const ir::Type *DefaultIntegerLiteralType = Hints.DefaultIntegerLiteralType;
    CHECK_ICE(Site, !DefaultIntegerLiteralType || Context.irContext().containsType(DefaultIntegerLiteralType), core::DiagnosticKind::SemanticConstructionFailed);

    const ir::Value *Result = nullptr;
    using namespace frontend::parser;
    switch (Node.getKind())
    {
      case ASTKind::NameExpr:
        Result = analyzeNameExpr(State, static_cast<const NameExpr &>(Node), Hints.IsCallCallee);
        break;
      case ASTKind::LiteralExpr:
        Result = analyzeLiteralExpr(State, static_cast<const LiteralExpr &>(Node), Hints);
        break;
      case ASTKind::ParenExpr:
        Result = analyzeParenExpr(State, static_cast<const ParenExpr &>(Node), Hints);
        break;
      case ASTKind::MemberExpr:
        Result = analyzeMemberExpr(State, static_cast<const MemberExpr &>(Node), Hints.IsCallCallee);
        break;
      case ASTKind::CallExpr:
        Result = analyzeCallExpr(State, static_cast<const CallExpr &>(Node));
        break;
      case ASTKind::UnaryExpr:
        Result = analyzeUnaryExpr(State, static_cast<const UnaryExpr &>(Node), Hints);
        break;
      case ASTKind::ArrayRepeatExpr:
        Result = analyzeArrayRepeatExpr(State, static_cast<const ArrayRepeatExpr &>(Node));
        break;
      case ASTKind::FunctionTypeExpr:
        Result = analyzeFunctionTypeExpr(State, static_cast<const FunctionTypeExpr &>(Node));
        break;
      default:
        CHECK_ICE(Site, categoryOf(Node.getKind()) == ASTCategory::Expr, core::DiagnosticKind::SemanticInvalidAST, "expression node kind must belong to the expression AST category");
        // TODO: Restore generic applications, remaining operators and type-valued compile-time evaluation.
        Site.reportICE<core::DiagnosticKind::SemanticUnsupported>(astKindName(Node.getKind()));
    }
    RETURN_IF(!Result || !ExpectedType, Result);
    return convertToExpectedType(State, Site, *Result, *ExpectedType);
  }
} // namespace ink::semantic
