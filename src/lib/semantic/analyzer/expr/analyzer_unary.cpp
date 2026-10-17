#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/unary_expr.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeUnaryExpr(AnalyzerState &State, const frontend::parser::UnaryExpr &Node, ExprHints Hints)
  {
    using namespace frontend::parser;
    if (Node.op() != TokenKind::Star && Node.op() != TokenKind::Amp && Node.op() != TokenKind::Minus)
    {
      reportUnsupported(State, Node);
      return nullptr;
    }
    auto &Context = State.semanticContext();
    if (Node.op() == TokenKind::Minus)
    {
      // TODO: Preserve untyped integer values through negation before materialization; signed-minimum literals such as -128 for i8 currently fail while typing their positive operand.
      Hints.IsCallCallee = false;
      const auto *Operand = analyzeExpr(State, *Node.operand(), Hints);
      if (!Operand)
      {
        return nullptr;
      }
      const auto *OperandType = Operand->type();
      if (ir::GlobalVariable::classof(Operand) || (!ir::IntegerType::classof(OperandType) && !ir::FloatType::classof(OperandType)))
      {
        reportUnsupported(State, Node);
        return nullptr;
      }
      const auto *Result = State.irBuilder().createNeg(*Operand);
      if (!Result)
      {
        Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      }
      return Result;
    }

    const ir::Value *Operand = analyzeExpr(State, *Node.operand());
    if (!Operand)
    {
      return nullptr;
    }
    if (ir::Type::classof(Operand))
    {
      // analyzeExpr has already validated this type's membership in the current IR context.
      const auto *Type = static_cast<const ir::Type *>(Operand);
      if (ir::MetaType::classof(Type))
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.operand()->getSourceRange(), "a type with runtime storage", Type->toString());
        return nullptr;
      }
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
