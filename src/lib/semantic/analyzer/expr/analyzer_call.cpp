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
    if (!Node.callee())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "CallExpr requires a callee expression");
      return nullptr;
    }
    for (const auto &Argument : Node.arguments())
    {
      using frontend::parser::ArgumentKind;
      if (Argument.form() != ArgumentKind::Named && Argument.form() != ArgumentKind::Positional && Argument.form() != ArgumentKind::SpreadPositional)
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Argument.range(), "call argument kind must be Named, Positional or SpreadPositional");
        return nullptr;
      }
      if (!Argument.value())
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Argument.range(), "call argument requires a value expression");
        return nullptr;
      }
      if (Argument.form() == ArgumentKind::Named)
      {
        if (!Argument.name())
        {
          Context.report<core::DiagnosticKind::SemanticInvalidAST>(Argument.range(), "named call argument requires a name");
          return nullptr;
        }
        if (Argument.name()->Text.empty())
        {
          Context.report<core::DiagnosticKind::SemanticInvalidAST>(Argument.range(), "named call argument requires a nonempty name");
          return nullptr;
        }
      }
      else if (Argument.name())
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Argument.range(), "positional or spread call argument must not have a name");
        return nullptr;
      }
    }
    if (Node.optional())
    {
      // TODO: Lower optional calls with conditional evaluation and the appropriate result type.
      reportUnsupported(State, Node.getSourceRange(), "optional calls");
      return nullptr;
    }
    for (const auto &Argument : Node.arguments())
    {
      if (Argument.form() == frontend::parser::ArgumentKind::Named)
      {
        // TODO: Map named arguments to parameter slots without changing source evaluation order.
        reportUnsupported(State, Argument.range(), "named call arguments");
        return nullptr;
      }
      if (Argument.form() == frontend::parser::ArgumentKind::SpreadPositional)
      {
        // TODO: Expand spread operands when variadic argument lowering is available.
        reportUnsupported(State, Argument.range(), "spread call arguments");
        return nullptr;
      }
    }

    auto &Builder = State.irBuilder();
    const ir::BasicBlock *Block = Builder.insertBlock();
    if (!Block)
    {
      // TODO: Lower module-level runtime calls into the module initialization function.
      reportUnsupported(State, Node.getSourceRange(), "module-level calls");
      return nullptr;
    }
    if (!Builder.insertFunction() || Block->hasTerminator())
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    const ir::Value *Callee = analyzeExpr(State, *Node.callee(), {.IsCallCallee = true});
    if (!Callee)
    {
      return nullptr;
    }
    if (ir::Type::classof(Callee))
    {
      // TODO: Analyze type construction and explicit conversion calls separately from runtime function calls.
      reportUnsupported(State, Node.callee()->getSourceRange(), "type construction and conversion calls");
      return nullptr;
    }
    if (ir::GlobalVariable::classof(Callee))
    {
      // TODO: Materialize a function value or function pointer from variable storage before calling it.
      reportUnsupported(State, Node.callee()->getSourceRange(), "loading a callable from variable storage");
      return nullptr;
    }
    if (ir::Module::classof(Callee))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.callee()->getSourceRange(), "a callable function or function pointer", ir::valueKindName(Callee->kind()));
      return nullptr;
    }
    if (!requireRuntimeValue(Context, Node.callee()->getSourceRange(), Callee))
    {
      return nullptr;
    }
    const ir::Type *CalleeType = Callee->type();
    const ir::Type *SignatureType = ir::PointerType::classof(CalleeType) ? static_cast<const ir::PointerType *>(CalleeType)->pointeeType() : CalleeType;
    if (!ir::FunctionType::classof(SignatureType))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.callee()->getSourceRange(), "a callable function or function pointer", CalleeType->toString());
      return nullptr;
    }
    const auto &Signature = static_cast<const ir::FunctionType &>(*SignatureType);
    for (const auto &Parameter : Signature.parameters())
    {
      if (Parameter.Variadic)
      {
        // TODO: Match variadic argument packs once the signature and operand lowering support them.
        reportUnsupported(State, Node.getSourceRange(), "variadic calls");
        return nullptr;
      }
    }
    if (Node.arguments().size() != Signature.parameterCount())
    {
      Context.report<core::DiagnosticKind::SemanticArgumentCount>(Node.getSourceRange(), Signature.parameterCount(), Node.arguments().size());
      return nullptr;
    }

    std::vector<ir::Argument> Arguments;
    Arguments.reserve(Node.arguments().size());
    // Evaluate the callee first and each positional argument once, in source order.
    for (std::size_t Index = 0; Index < Node.arguments().size(); ++Index)
    {
      const auto &Argument = Node.arguments()[Index];
      const ir::Type *ExpectedType = Signature.parameters()[Index].ParameterType;
      const ir::Value *Value = requireRuntimeValue(Context, Argument.range(), analyzeExpr(State, *Argument.value(), {.ExpectedType = ExpectedType}));
      if (!Value)
      {
        return nullptr;
      }
      if (ir::GlobalVariable::classof(Value))
      {
        // TODO: Materialize argument storage reads before constructing the call operands.
        reportUnsupported(State, Argument.range(), "loading call arguments from variable storage");
        return nullptr;
      }
      Arguments.emplace_back(*Value);
    }
    const ir::CallInstruction *Call = Builder.createCall(Signature, *Callee, std::move(Arguments));
    if (!Call)
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
    }
    return Call;
  }
} // namespace ink::semantic
