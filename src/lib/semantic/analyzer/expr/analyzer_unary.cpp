#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/unary_expr.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeUnaryExpr(AnalyzerState &State, const frontend::parser::UnaryExpr &Node, ExprHints Hints)
  {
    using namespace frontend::parser;
    UNSUPPORT_IF(State, Node.op() != TokenKind::Star && Node.op() != TokenKind::Amp && Node.op() != TokenKind::Minus, nullptr, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    auto &Context = State.semanticContext();
    if (Node.op() == TokenKind::Minus)
    {
      // TODO: Preserve untyped integer values through negation before materialization; signed-minimum literals such as -128 for i8 currently fail while typing their positive operand.
      Hints.IsCallCallee = false;
      const ir::Value *Operand = analyzeExpr(State, *Node.operand(), Hints);
      RETURN_IF(!Operand, nullptr);
      const ir::Type *OperandType = Operand->type();
      UNSUPPORT_IF(State, ir::GlobalVariable::classof(Operand) || ir::Type::classof(Operand) || ir::Module::classof(Operand) || (!ir::IntegerType::classof(OperandType) && !ir::FloatType::classof(OperandType)), nullptr, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
      Operand = Operand->isStorage() ? loadStorage(State, Node.operand()->getSourceRange(), *Operand) : requireRuntimeValue(Context, Node.operand()->getSourceRange(), Operand);
      RETURN_IF(!Operand, nullptr);
      const auto *Result = State.irBuilder().createNeg(*Operand);
      RETURN_REPORT_IF_FALSE(Context, Result, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Node.getSourceRange());
      return Result;
    }

    const ir::Value *Operand = analyzeExpr(State, *Node.operand());
    RETURN_IF(!Operand, nullptr);
    if (ir::Type::classof(Operand))
    {
      // analyzeExpr has already validated this type's membership in the current IR context.
      const auto *Type = static_cast<const ir::Type *>(Operand);
      RETURN_REPORT_IF(Context, ir::MetaType::classof(Type), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.operand()->getSourceRange(), "a type with runtime storage", Type->toString());
      auto &Types = Context.typePool();
      return Node.op() == TokenKind::Star ? static_cast<const ir::Type *>(Types.getPointerType(Type)) : Types.getReferenceType(Type);
    }
    RETURN_REPORT_IF(Context, ir::Module::classof(Operand), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.operand()->getSourceRange(), "a type or runtime operand", ir::valueKindName(Operand->kind()));
    // TODO: Emit runtime dereference and address operations when their IR is supported.
    reportUnsupported(State, Node);
    return nullptr;
  }
} // namespace ink::semantic
