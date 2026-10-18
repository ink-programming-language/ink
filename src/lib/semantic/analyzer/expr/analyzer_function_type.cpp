#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/function_type_expr.h"
#include "ink/frontend/parser/ast/type_syntax.h"
#include "ink/ir/type/structure_type/function_type_parameter.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

#include <vector>

namespace ink::semantic
{
  ExprResult Analyzer::analyzeFunctionTypeExpr(AnalyzerState &State, const frontend::parser::FunctionTypeExpr &Node)
  {
    auto &Context = State.semanticContext();
    RETURN_REPORT_IF_FALSE(Context, Node.returnType(), core::DiagnosticKind::SemanticInvalidAST, ExprResult{}, Node.getSourceRange(), "FunctionTypeExpr requires a return type annotation");
    const ir::Type *Return = analyzeType(State, *Node.returnType());
    RETURN_IF_NULL(Return, ExprResult{});
    CHECK_HAS_LAYOUT(Context, Node.returnType()->getSourceRange(), Return, "a return type with runtime layout", ExprResult{});
    std::vector<ir::FunctionTypeParameter> Parameters;
    Parameters.reserve(Node.parameters().size());
    for (const auto &Parameter : Node.parameters())
    {
      RETURN_REPORT_IF_FALSE(Context, Parameter.type(), core::DiagnosticKind::SemanticInvalidAST, ExprResult{}, Node.getSourceRange(), "FunctionTypeExpr parameter requires a type annotation");
      const ir::Type *Type = analyzeType(State, *Parameter.type());
      RETURN_IF_NULL(Type, ExprResult{});
      CHECK_HAS_LAYOUT(Context, Parameter.type()->getSourceRange(), Type, "a parameter type with runtime layout", ExprResult{});
      Parameters.emplace_back(Type, Parameter.variadic());
    }
    const ir::Type *Result = Context.typePool().getFunctionType(Return, Parameters);
    RETURN_REPORT_IF_FALSE(Context, Result, core::DiagnosticKind::SemanticConstructionFailed, ExprResult{}, Node.getSourceRange());
    return ExprResult::fromValue(Result);
  }
} // namespace ink::semantic
