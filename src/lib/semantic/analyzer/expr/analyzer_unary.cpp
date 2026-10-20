#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/unary_expr.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeUnaryExpr(AnalyzerState &State, const frontend::parser::UnaryExpr &Node, ExprHints Hints)
  {
    using namespace frontend::parser;
    auto &Context = State.semanticContext();
    const auto Qualifier = Node.qualifier();
    RETURN_REPORT_IF(Context, Qualifier && (Qualifier->Kind != TypeQualifierKind::Const || (Node.op() != TokenKind::Star && Node.op() != TokenKind::Amp)), core::DiagnosticKind::SemanticInvalidAST, nullptr, Node.getSourceRange(), "a const qualifier requires a pointer or reference type constructor");
    UNSUPPORT_IF(State, Node.op() != TokenKind::Star && Node.op() != TokenKind::Amp && Node.op() != TokenKind::Minus, nullptr, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    if (Node.op() == TokenKind::Minus)
    {
      // TODO: Preserve untyped integer values through negation before materialization; signed-minimum literals such as -128 for i8 currently fail while typing their positive operand.
      Hints.IsCallCallee = false;
      const ir::Value *Operand = emitValue(State, *Node.operand(), Hints);
      RETURN_IF(!Operand, nullptr);
      const ir::Type *OperandType = Operand->type();
      UNSUPPORT_IF(State, ir::Type::classof(Operand) || ir::Module::classof(Operand) || (!ir::IntegerType::classof(OperandType) && !ir::FloatType::classof(OperandType)), nullptr, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
      Operand = requireRuntimeValue(Context, Node.operand()->getSourceRange(), Operand);
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
      if (Qualifier)
      {
        const ir::Type *Qualified = Types.getQualifiedType(Type, false);
        RETURN_REPORT_IF_FALSE(Context, Qualified, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Qualifier->Range, "a const-qualified object type", Type->toString());
        Type = Qualified;
      }
      return Node.op() == TokenKind::Star ? static_cast<const ir::Type *>(Types.getPointerType(Type)) : Types.getReferenceType(Type);
    }
    RETURN_REPORT_IF(Context, Qualifier, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Qualifier->Range, "a type after a pointer or reference qualifier", Operand->toString());
    RETURN_REPORT_IF(Context, ir::Module::classof(Operand), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.operand()->getSourceRange(), "a type or runtime operand", ir::valueKindName(Operand->kind()));
    auto &Builder = State.irBuilder();
    if (Node.op() == TokenKind::Amp)
    {
      const bool Local = ir::AllocaInstruction::classof(Operand);
      RETURN_REPORT_IF(Context, !Local && !ir::DereferenceInstruction::classof(Operand), core::DiagnosticKind::SemanticInvalidAddressOperand, nullptr, Node.getSourceRange());
      RETURN_REPORT_IF(Context, !Builder.insertFunction() || !Builder.insertBlock() || Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed, nullptr, Node.getSourceRange());
      // Taking an address preserves the object's qualifiers and does not read its contents.
      const auto *Result = Builder.createAddressOf(*Operand);
      RETURN_REPORT_IF_FALSE(Context, Result, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Node.getSourceRange());
      if (Local && !Operand->isInitialized())
      {
        const core::Name Name = static_cast<const ir::AllocaInstruction *>(Operand)->name();
        Context.report<core::DiagnosticKind::SemanticUninitializedAddress>(Node.getSourceRange(), core::NamePool::instance().text(Name));
      }
      return Result;
    }

    // The operand was already analyzed to distinguish type construction; load a stored pointer without evaluating its expression again.
    Operand = Operand->isStorage() ? loadStorage(State, Node.operand()->getSourceRange(), *Operand) : Operand;
    Operand = requireRuntimeValue(Context, Node.operand()->getSourceRange(), Operand);
    RETURN_IF(!Operand, nullptr);
    RETURN_REPORT_IF_FALSE(Context, ir::PointerType::classof(Operand->type()), core::DiagnosticKind::SemanticInvalidDereference, nullptr, Node.getSourceRange());
    const auto *PointerType = static_cast<const ir::PointerType *>(Operand->type());
    RETURN_REPORT_IF(Context, PointerType->isOpaque() || !PointerType->pointeeType()->hasLayout(), core::DiagnosticKind::SemanticInvalidDereference, nullptr, Node.getSourceRange());
    RETURN_REPORT_IF(Context, !Builder.insertFunction() || !Builder.insertBlock() || Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed, nullptr, Node.getSourceRange());
    // Selecting the pointed-to storage defers its load until a runtime value is requested.
    const auto *Result = Builder.createDereference(*Operand);
    RETURN_REPORT_IF_FALSE(Context, Result, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Node.getSourceRange());
    return Result;
  }
} // namespace ink::semantic
