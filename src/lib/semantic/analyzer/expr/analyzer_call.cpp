#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/call_expr.h"
#include "ink/frontend/parser/ast/expr.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

#include <utility>
#include <vector>

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeCallExpr(AnalyzerState &State, const frontend::parser::CallExpr &Node)
  {
    auto &Context = State.semanticContext();
    RETURN_REPORT_IF_FALSE(Context, Node.callee(), core::DiagnosticKind::SemanticInvalidAST, nullptr, Node.getSourceRange(), "CallExpr requires a callee expression");
    for (const auto &Argument : Node.arguments())
    {
      using frontend::parser::ArgumentKind;
      RETURN_REPORT_IF(Context, Argument.form() != ArgumentKind::Named && Argument.form() != ArgumentKind::Positional && Argument.form() != ArgumentKind::SpreadPositional, core::DiagnosticKind::SemanticInvalidAST, nullptr, Argument.range(), "call argument kind must be Named, Positional or SpreadPositional");
      RETURN_REPORT_IF_FALSE(Context, Argument.value(), core::DiagnosticKind::SemanticInvalidAST, nullptr, Argument.range(), "call argument requires a value expression");
      if (Argument.form() == ArgumentKind::Named)
      {
        RETURN_REPORT_IF_FALSE(Context, Argument.name(), core::DiagnosticKind::SemanticInvalidAST, nullptr, Argument.range(), "named call argument requires a name");
        RETURN_REPORT_IF(Context, Argument.name()->Text.empty(), core::DiagnosticKind::SemanticInvalidAST, nullptr, Argument.range(), "named call argument requires a nonempty name");
      }
      else
      {
        RETURN_REPORT_IF(Context, Argument.name(), core::DiagnosticKind::SemanticInvalidAST, nullptr, Argument.range(), "positional or spread call argument must not have a name");
      }
    }
    // TODO: Lower optional calls with conditional evaluation and the appropriate result type.
    UNSUPPORT_IF(State, Node.optional(), nullptr, Node.getSourceRange(), "optional calls");
    for (const auto &Argument : Node.arguments())
    {
      // TODO: Map named arguments to parameter slots without changing source evaluation order.
      UNSUPPORT_IF(State, Argument.form() == frontend::parser::ArgumentKind::Named, nullptr, Argument.range(), "named call arguments");
      // TODO: Expand spread operands when variadic argument lowering is available.
      UNSUPPORT_IF(State, Argument.form() == frontend::parser::ArgumentKind::SpreadPositional, nullptr, Argument.range(), "spread call arguments");
    }

    auto &Builder = State.irBuilder();
    const ir::BasicBlock *Block = Builder.insertBlock();
    // TODO: Lower module-level runtime calls into the module initialization function.
    UNSUPPORT_IF(State, !Block, nullptr, Node.getSourceRange(), "module-level calls");
    RETURN_REPORT_IF(Context, !Builder.insertFunction() || Block->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed, nullptr, Node.getSourceRange());
    const ir::Value *Callee = analyzeExpr(State, *Node.callee(), {.IsCallCallee = true});
    RETURN_IF(!Callee, nullptr);
    // TODO: Analyze type construction and explicit conversion calls separately from runtime function calls.
    UNSUPPORT_IF(State, ir::Type::classof(Callee), nullptr, Node.callee()->getSourceRange(), "type construction and conversion calls");
    // TODO: Materialize a function value or function pointer from variable storage before calling it.
    UNSUPPORT_IF(State, ir::GlobalVariable::classof(Callee), nullptr, Node.callee()->getSourceRange(), "loading a callable from variable storage");
    RETURN_REPORT_IF(Context, ir::Module::classof(Callee), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.callee()->getSourceRange(), "a callable function or function pointer", ir::valueKindName(Callee->kind()));
    RETURN_IF(!requireRuntimeValue(Context, Node.callee()->getSourceRange(), Callee), nullptr);
    const ir::Type *CalleeType = Callee->type();
    const ir::Type *SignatureType = ir::PointerType::classof(CalleeType) ? static_cast<const ir::PointerType *>(CalleeType)->pointeeType() : CalleeType;
    RETURN_REPORT_IF_FALSE(Context, ir::FunctionType::classof(SignatureType), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.callee()->getSourceRange(), "a callable function or function pointer", CalleeType->toString());
    const auto &Signature = static_cast<const ir::FunctionType &>(*SignatureType);
    for (const auto &Parameter : Signature.parameters())
    {
      // TODO: Match variadic argument packs once the signature and operand lowering support them.
      UNSUPPORT_IF(State, Parameter.Variadic, nullptr, Node.getSourceRange(), "variadic calls");
    }
    RETURN_REPORT_IF(Context, Node.arguments().size() != Signature.parameterCount(), core::DiagnosticKind::SemanticArgumentCount, nullptr, Node.getSourceRange(), Signature.parameterCount(), Node.arguments().size());

    std::vector<ir::Argument> Arguments;
    Arguments.reserve(Node.arguments().size());
    // Evaluate the callee first and each positional argument once, in source order.
    for (std::size_t Index = 0; Index < Node.arguments().size(); ++Index)
    {
      const auto &Argument = Node.arguments()[Index];
      const ir::Type *ExpectedType = Signature.parameters()[Index].ParameterType;
      const ir::Value *Value = requireRuntimeValue(Context, Argument.range(), analyzeExpr(State, *Argument.value(), {.ExpectedType = ExpectedType}));
      RETURN_IF(!Value, nullptr);
      // TODO: Materialize argument storage reads before constructing the call operands.
      UNSUPPORT_IF(State, ir::GlobalVariable::classof(Value), nullptr, Argument.range(), "loading call arguments from variable storage");
      Arguments.emplace_back(*Value);
    }
    const ir::CallInstruction *Call = Builder.createCall(Signature, *Callee, std::move(Arguments));
    RETURN_REPORT_IF_FALSE(Context, Call, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Node.getSourceRange());
    return Call;
  }
} // namespace ink::semantic
