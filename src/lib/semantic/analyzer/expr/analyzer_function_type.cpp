#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/function_type_expr.h"
#include "ink/ir/type/structure_type/function_type_parameter.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

#include <vector>

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeFunctionTypeExpr(AnalyzerState &State, const frontend::parser::FunctionTypeExpr &Node)
  {
    auto &Context = State.semanticContext();
    const ir::Type *Return = analyzeType(State, *Node.returnType());
    if (!Return)
    {
      return nullptr;
    }
    std::vector<ir::FunctionTypeParameter> Parameters;
    Parameters.reserve(Node.parameters().size());
    for (const auto &Parameter : Node.parameters())
    {
      const ir::Type *Type = analyzeType(State, *Parameter.type());
      if (!Type)
      {
        return nullptr;
      }
      Parameters.emplace_back(Type, Parameter.variadic());
    }
    const ir::Type *Result = Context.typePool().getFunctionType(Return, Parameters);
    if (!Result)
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a function signature with runtime layouts", "a non-runtime signature type");
      return nullptr;
    }
    return Result;
  }
} // namespace ink::semantic
