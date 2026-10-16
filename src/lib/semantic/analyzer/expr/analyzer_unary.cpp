#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/unary_expr.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeUnaryExpr(AnalyzerState &State, const frontend::parser::UnaryExpr &Node)
  {
    using namespace frontend::parser;
    if (Node.op() != TokenKind::Star && Node.op() != TokenKind::Amp)
    {
      reportUnsupported(State, Node);
      return nullptr;
    }
    const ir::Value *Operand = analyzeExpr(State, *Node.operand());
    if (!Operand)
    {
      return nullptr;
    }
    auto &Context = State.semanticContext();
    if (ir::Type::classof(Operand))
    {
      // analyzeExpr has already validated this type's membership in the current IR context.
      const auto *Type = static_cast<const ir::Type *>(Operand);
      auto &Types = Context.typePool();
      return Node.op() == TokenKind::Star ? static_cast<const ir::Type *>(Types.getPointerType(Type)) : Types.getReferenceType(Type);
    }
    if (ir::Module::classof(Operand))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.operand()->getSourceRange(), "a type or runtime operand", ir::valueKindName(Operand->kind()));
      return nullptr;
    }
    // TODO: Emit runtime dereference and address operations when their IR is supported.
    reportUnsupported(State, Node);
    return nullptr;
  }
} // namespace ink::semantic
