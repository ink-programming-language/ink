#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/function_type_expr.h"
#include "ink/frontend/parser/ast/type_syntax.h"
#include "ink/ir/type/structure_type/function_type_parameter.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

#include <vector>

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeFunctionTypeExpr(AnalyzerState &State, const frontend::parser::FunctionTypeExpr &Node)
  {
    auto &Context = State.semanticContext();
    if (!Node.returnType())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionTypeExpr requires a return type annotation");
      return nullptr;
    }
    const ir::Type *Return = analyzeType(State, *Node.returnType());
    if (!Return)
    {
      return nullptr;
    }
    if (!Return->hasLayout())
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.returnType()->getSourceRange(), "a return type with runtime layout", Return->toString());
      return nullptr;
    }
    std::vector<ir::FunctionTypeParameter> Parameters;
    Parameters.reserve(Node.parameters().size());
    for (const auto &Parameter : Node.parameters())
    {
      if (!Parameter.type())
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionTypeExpr parameter requires a type annotation");
        return nullptr;
      }
      const ir::Type *Type = analyzeType(State, *Parameter.type());
      if (!Type)
      {
        return nullptr;
      }
      if (!Type->hasLayout())
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Parameter.type()->getSourceRange(), "a parameter type with runtime layout", Type->toString());
        return nullptr;
      }
      Parameters.emplace_back(Type, Parameter.variadic());
    }
    const ir::Type *Result = Context.typePool().getFunctionType(Return, Parameters);
    if (!Result)
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    return Result;
  }
} // namespace ink::semantic
