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
    return Value->isStorage() ? loadStorage(State, Node.getSourceRange(), *Value) : Value;
  }

  const ir::Value *Analyzer::emitPlace(AnalyzerState &State, const frontend::parser::Expr &Node, ExprHints Hints)
  {
    const ir::Value *Value = analyzeExpr(State, Node, Hints);
    RETURN_IF(!Value, nullptr);
    RETURN_REPORT_IF_FALSE(State.semanticContext(), Value->isStorage(), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.getSourceRange(), "an addressable expression", ir::valueKindName(Value->kind()));
    return Value;
  }

  const ir::Value *Analyzer::analyzeExpr(AnalyzerState &State, const frontend::parser::Expr &Node, ExprHints Hints)
  {
    auto &Context = State.semanticContext();
    CHECK_RECURSION_DEPTH(State, Node.getSourceRange(), nullptr);
    BlockDepthGuard DepthGuard(State.recursionDepth());
    UNSUPPORT_IF(State, Node.isComptime(), nullptr, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    const ir::Type *ExpectedType = Hints.ExpectedType;
    CHECK_VALID_TYPE(Context, ExpectedType, Node.getSourceRange());
    CHECK_VALID_TYPE(Context, Hints.DefaultIntegerLiteralType, Node.getSourceRange());

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
        RETURN_REPORT_IF(Context, categoryOf(Node.getKind()) != ASTCategory::Expr, core::DiagnosticKind::SemanticInvalidAST, nullptr, Node.getSourceRange(), "expression node kind must belong to the expression AST category");
        // TODO: Restore generic applications, remaining operators and type-valued compile-time evaluation.
        reportUnsupported(State, Node);
        return nullptr;
    }
    RETURN_IF(!Result || !ExpectedType, Result);
    const bool Storage = Result->isStorage();
    if (ir::MetaType::classof(ExpectedType))
    {
      RETURN_REPORT_IF(Context, Storage, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.getSourceRange(), "a type", "variable storage");
      return requireType(Context, Node.getSourceRange(), Result);
    }
    RETURN_IF(!Storage && !requireRuntimeValue(Context, Node.getSourceRange(), Result), nullptr);
    // Check the stored object's type before either emitter chooses whether to read it.
    const ir::Type *Type = Result->type();
    const bool Compatible = !Storage && ir::Constant::classof(Result) ? static_cast<const ir::Constant *>(Result)->canInitialize(ExpectedType) : Type == ExpectedType;
    RETURN_REPORT_IF_FALSE(Context, Compatible, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.getSourceRange(), ExpectedType->toString(), Type->toString());
    return Result;
  }
} // namespace ink::semantic
