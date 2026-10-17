#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/analyzer/block_depth_guard.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeExpr(AnalyzerState &State, const frontend::parser::Expr &Node, ExprHints Hints)
  {
    auto &Context = State.semanticContext();
    if (State.recursionDepth() >= State.recursionDepthLimit())
    {
      Context.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return nullptr;
    }
    BlockDepthGuard DepthGuard(State.recursionDepth());
    if (Node.isComptime())
    {
      reportUnsupported(State, Node);
      return nullptr;
    }
    const ir::Type *ExpectedType = Hints.ExpectedType;
    // Hints are supplied by the analyzer; foreign types violate its internal context contract.
    if (ExpectedType && !Context.irContext().containsType(ExpectedType))
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    if (Hints.DefaultIntegerLiteralType && !Context.irContext().containsType(Hints.DefaultIntegerLiteralType))
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }

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
        if (categoryOf(Node.getKind()) != ASTCategory::Expr)
        {
          Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "expression node kind must belong to the expression AST category");
          return nullptr;
        }
        // TODO: Restore generic applications, remaining operators and type-valued compile-time evaluation.
        reportUnsupported(State, Node);
        return nullptr;
    }
    if (!Result || !ExpectedType)
    {
      return Result;
    }
    if (ir::MetaType::classof(ExpectedType))
    {
      return requireType(Context, Node.getSourceRange(), Result);
    }
    if (!requireRuntimeValue(Context, Node.getSourceRange(), Result))
    {
      return nullptr;
    }
    const bool Compatible = ir::Constant::classof(Result) ? static_cast<const ir::Constant *>(Result)->canInitialize(ExpectedType) : Result->type() == ExpectedType;
    if (!Compatible)
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), ExpectedType->toString(), Result->type()->toString());
      return nullptr;
    }
    return Result;
  }
} // namespace ink::semantic
