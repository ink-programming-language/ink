#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ink::ir;

  bool Analyzer::analyzeIfStmt(AnalysisState &State, const parser::IfStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeWhileStmt(AnalysisState &State, const parser::WhileStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeClassicForStmt(AnalysisState &State, const parser::ClassicForStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeForInStmt(AnalysisState &State, const parser::ForInStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeSwitchStmt(AnalysisState &State, const parser::SwitchStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeReturnStmt(AnalysisState &State, const parser::ReturnStmt &Node)
  {
    if (!State.CurrentFunction)
    {
      State.report<core::DiagnosticKind::SemanticReturnOutsideFunction>(Node.getSourceRange());
      return false;
    }
    State.Terminated = true;
    const Type &ReturnType = State.CurrentFunction->functionType().returnType();
    const Value *ReturnedValue = nullptr;
    if (Node.value())
    {
      const ExpressionResult Result = analyzeExpr(State, *Node.value());
      if (!Result)
      {
        return false;
      }
      if (ReturnType.typeKind() == TypeKind::Void)
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.value()->getSourceRange(), "no return value", Result.IntegerLiteral ? "integer literal" : describeType(Result.ValueObject->type()));
        return false;
      }
      ReturnedValue = convertExpression(State, Result, ReturnType, *Node.value());
      if (!ReturnedValue)
      {
        return false;
      }
    }
    else if (ReturnType.typeKind() != TypeKind::Void)
    {
      State.report<core::DiagnosticKind::SemanticMissingReturn>(Node.getSourceRange(), State.Context.namePool().text(State.CurrentFunction->name()));
      return false;
    }
    if (!State.Builder.createReturnInstruction(ReturnedValue))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    return true;
  }

  bool Analyzer::analyzeBreakStmt(AnalysisState &State, const parser::BreakStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeContinueStmt(AnalysisState &State, const parser::ContinueStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeYieldStmt(AnalysisState &State, const parser::YieldStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeDeferStmt(AnalysisState &State, const parser::DeferStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
