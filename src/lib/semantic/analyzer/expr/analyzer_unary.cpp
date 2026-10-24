#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/unary_expr.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  ExprResult Analyzer::analyzeUnaryExpr(AnalyzerState &State, const frontend::parser::UnaryExpr &Node, ExprHints Hints)
  {
    using namespace frontend::parser;
    auto &Context = State.semanticContext();
    const auto Site = State.at(Node);
    const auto Qualifier = Node.qualifier();
    CHECK_ICE(Site, !Qualifier || (Qualifier->Kind == TypeQualifierKind::Const && (Node.op() == TokenKind::Star || Node.op() == TokenKind::Amp)), core::DiagnosticKind::SemanticInvalidAST, "a const qualifier requires a pointer or reference type constructor");
    CHECK_ICE(Site, Node.op() == TokenKind::Star || Node.op() == TokenKind::Amp || Node.op() == TokenKind::Minus, core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    const auto OperandSite = State.at(*Node.operand());
    if (Node.op() == TokenKind::Minus)
    {
      // TODO: Preserve untyped integer values through negation before materialization; signed-minimum literals such as -128 for i8 currently fail while typing their positive operand.
      Hints.IsCallCallee = false;
      const ir::Value *Operand = emitValue(State, *Node.operand(), Hints);
      RETURN_IF_NOT(Operand, nullptr);
      const ir::Type *OperandType = Operand->type();
      CHECK_ICE(Site, !ir::Type::classof(Operand) && !ir::Module::classof(Operand) && (ir::IntegerType::classof(OperandType) || ir::FloatType::classof(OperandType)), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
      Operand = requireRuntimeValue(OperandSite, Operand);
      RETURN_IF_NOT(Operand, nullptr);
      const auto *Result = State.irBuilder().createNeg(*Operand);
      CHECK_ICE(Site, Result, core::DiagnosticKind::SemanticConstructionFailed);
      return Result;
    }

    const ExprResult Expression = analyzeExpr(State, *Node.operand());
    const ir::Value *Operand = Expression.value();
    RETURN_IF_NOT(Operand, nullptr);
    if (ir::Type::classof(Operand))
    {
      // analyzeExpr has already validated this type's membership in the current IR context.
      const auto *Type = static_cast<const ir::Type *>(Operand);
      RETURN_ERROR_IF(OperandSite, ir::MetaType::classof(Type), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type with runtime storage", Type->toString());
      auto &Types = Context.typePool();
      if (Qualifier)
      {
        const ir::Type *Qualified = Types.get(Type, false);
        RETURN_ERROR_IF_NOT(State.at(Qualifier->Range), Qualified, core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a const-qualified object type", Type->toString());
        Type = Qualified;
      }
      return Node.op() == TokenKind::Star ? static_cast<const ir::Type *>(Types.get<ir::PointerType>(Type)) : Types.get<ir::ReferenceType>(Type);
    }
    RETURN_ERROR_IF(State.at(Qualifier->Range), Qualifier, core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type after a pointer or reference qualifier", Operand->toString());
    RETURN_ERROR_IF(OperandSite, ir::Module::classof(Operand), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type or runtime operand", ir::valueKindName(Operand->kind()));
    if (Node.op() == TokenKind::Amp)
    {
      RETURN_ERROR_IF_NOT(Site, Expression.isLocation(), core::DiagnosticKind::SemanticInvalidAddressOperand, nullptr);
      // Taking an address preserves the object's qualifiers and does not read its contents.
      const auto *Local = ir::AllocaInstruction::classof(Expression.declaration()) ? static_cast<const ir::AllocaInstruction *>(Expression.declaration()) : nullptr;
      WARNING_IF(Site, Local && !Local->isInitialized(), core::DiagnosticKind::SemanticUninitializedAddress, core::NamePool::instance().text(Local->name()));
      return Operand;
    }

    // The operand was already analyzed to distinguish type construction; load a stored pointer without evaluating its expression again.
    Operand = Expression.isLocation() ? loadLocation(State, OperandSite, Expression) : Operand;
    Operand = requireRuntimeValue(OperandSite, Operand);
    RETURN_IF_NOT(Operand, nullptr);
    RETURN_ERROR_IF_NOT(Site, ir::PointerType::classof(Operand->type()), core::DiagnosticKind::SemanticInvalidDereference, nullptr);
    const auto *PointerType = static_cast<const ir::PointerType *>(Operand->type());
    RETURN_ERROR_IF(Site, PointerType->isOpaque() || !PointerType->pointeeType()->hasLayout(), core::DiagnosticKind::SemanticInvalidDereference, nullptr);
    // Dereferencing changes the expression category, not the pointer IR value; reading remains explicit.
    return ExprResult::location(*Operand);
  }
} // namespace ink::semantic
