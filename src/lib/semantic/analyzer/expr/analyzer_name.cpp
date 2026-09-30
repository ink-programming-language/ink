#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ink::ir;

  Analyzer::ExpressionResult Analyzer::analyzeNameExpr(AnalysisState &State, const parser::NameExpr &Node)
  {
    const auto NameToken = Node.name();
    const Name Symbol = State.Context.namePool().find(NameToken.Text);
    const auto *Binding = State.Resolver.lookup(Symbol);
    if (!Binding)
    {
      if (State.Resolver.lookup<Decl *>(Symbol))
      {
        reportUnsupported(State, Node);
        return {};
      }
      if (NameToken.Text == "true" || NameToken.Text == "false")
      {
        return {&State.Context.constantPool().getBoolConstant(NameToken.Text == "true")};
      }
      State.report<core::DiagnosticKind::SemanticUnknownName>(NameToken.Range, NameToken.Text);
      return {};
    }
    if (Binding->targets().size() != 1)
    {
      State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
      return {};
    }
    const Value *Result = Binding->targets().front();
    if (FunctionParameter::classof(Result) && &static_cast<const FunctionParameter &>(*Result).function() != State.CurrentFunction)
    {
      State.report<core::DiagnosticKind::SemanticInvalidCapture>(Node.getSourceRange(), NameToken.Text);
      return {};
    }
    return {Result};
  }
} // namespace ink::semantic
