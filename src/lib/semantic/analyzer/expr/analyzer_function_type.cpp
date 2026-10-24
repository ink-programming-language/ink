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
  const ir::Value *Analyzer::analyzeFunctionTypeExpr(AnalyzerState &State, const frontend::parser::FunctionTypeExpr &Node)
  {
    auto &Context = State.semanticContext();
    const auto Site = State.at(Node);
    CHECK_ICE(Site, Node.returnType(), core::DiagnosticKind::SemanticInvalidAST, "FunctionTypeExpr requires a return type annotation");
    const ir::Type *Return = analyzeType(State, *Node.returnType());
    RETURN_IF_NULL(Return, nullptr);
    CHECK_HAS_LAYOUT(State.at(*Node.returnType()), Return, "a return type with runtime layout", nullptr);
    std::vector<ir::FunctionTypeParameter> Parameters;
    Parameters.reserve(Node.parameters().size());
    for (const auto &Parameter : Node.parameters())
    {
      CHECK_ICE(Site, Parameter.type(), core::DiagnosticKind::SemanticInvalidAST, "FunctionTypeExpr parameter requires a type annotation");
      const ir::Type *Type = analyzeType(State, *Parameter.type());
      RETURN_IF_NULL(Type, nullptr);
      CHECK_HAS_LAYOUT(State.at(*Parameter.type()), Type, "a parameter type with runtime layout", nullptr);
      Parameters.emplace_back(Type, Parameter.variadic());
    }
    const ir::Type *Result = Context.typePool().getFunctionType(Return, Parameters);
    CHECK_ICE(Site, Result, core::DiagnosticKind::SemanticConstructionFailed);
    return Result;
  }
} // namespace ink::semantic
