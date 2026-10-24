#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/call_expr.h"
#include "ink/frontend/parser/ast/expr.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/semantic/analyzer/analysis_site.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

#include <utility>
#include <vector>

namespace ink::semantic
{
  namespace
  {
    void validateCallShape(const AnalysisSite &CallSite, const frontend::parser::CallExpr &Node)
    {
      CHECK_ICE(CallSite, Node.callee(), core::DiagnosticKind::SemanticInvalidAST, "CallExpr requires a callee expression");
      for (const auto &Argument : Node.arguments())
      {
        using frontend::parser::ArgumentKind;
        const auto ArgumentSite = CallSite.at(Argument.range());
        CHECK_ICE(ArgumentSite, Argument.form() == ArgumentKind::Named || Argument.form() == ArgumentKind::Positional || Argument.form() == ArgumentKind::SpreadPositional, core::DiagnosticKind::SemanticInvalidAST, "call argument kind must be Named, Positional or SpreadPositional");
        CHECK_ICE(ArgumentSite, Argument.value(), core::DiagnosticKind::SemanticInvalidAST, "call argument requires a value expression");
        if (Argument.form() == ArgumentKind::Named)
        {
          CHECK_ICE(ArgumentSite, Argument.name(), core::DiagnosticKind::SemanticInvalidAST, "named call argument requires a name");
          CHECK_ICE(ArgumentSite, !Argument.name()->Text.empty(), core::DiagnosticKind::SemanticInvalidAST, "named call argument requires a nonempty name");
        }
        else
        {
          CHECK_ICE(ArgumentSite, !Argument.name(), core::DiagnosticKind::SemanticInvalidAST, "positional or spread call argument must not have a name");
        }
      }
    }

    void validateCallSupport(AnalyzerState &State, const frontend::parser::CallExpr &Node)
    {
      const auto CallSite = State.at(Node);
      // TODO: Lower optional calls with conditional evaluation and the appropriate result type.
      CHECK_ICE(CallSite, !Node.optional(), core::DiagnosticKind::SemanticUnsupported, "optional calls");
      for (const auto &Argument : Node.arguments())
      {
        const auto ArgumentSite = State.at(Argument.range());
        // TODO: Map named arguments to parameter slots without changing source evaluation order.
        CHECK_ICE(ArgumentSite, Argument.form() != frontend::parser::ArgumentKind::Named, core::DiagnosticKind::SemanticUnsupported, "named call arguments");
        // TODO: Expand spread operands when variadic argument lowering is available.
        CHECK_ICE(ArgumentSite, Argument.form() != frontend::parser::ArgumentKind::SpreadPositional, core::DiagnosticKind::SemanticUnsupported, "spread call arguments");
      }

      auto &Builder = State.irBuilder();
      const ir::BasicBlock *Block = Builder.insertBlock();
      // TODO: Lower module-level runtime calls into the module initialization function.
      CHECK_ICE(CallSite, Block, core::DiagnosticKind::SemanticUnsupported, "module-level calls");
      CHECK_ICE(CallSite, Builder.insertFunction() && !Block->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed);
    }

    const ir::FunctionType *requireCallableSignature(const AnalysisSite &Site, const ir::Value &Callee)
    {
      // TODO: Analyze type construction and explicit conversion calls separately from runtime function calls.
      CHECK_ICE(Site, !ir::Type::classof(&Callee), core::DiagnosticKind::SemanticUnsupported, "type construction and conversion calls");
      RETURN_ERROR_IF(Site, ir::Module::classof(&Callee), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a callable function or function pointer", ir::valueKindName(Callee.kind()));
      RETURN_IF_NOT(requireRuntimeValue(Site, &Callee), nullptr);
      const ir::Type *CalleeType = Callee.type();
      const ir::Type *SignatureType = ir::PointerType::classof(CalleeType) ? static_cast<const ir::PointerType *>(CalleeType)->pointeeType() : CalleeType;
      RETURN_ERROR_IF_NOT(Site, ir::FunctionType::classof(SignatureType), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a callable function or function pointer", CalleeType->toString());
      return static_cast<const ir::FunctionType *>(SignatureType);
    }
  } // namespace

  const ir::Value *Analyzer::analyzeCallExpr(AnalyzerState &State, const frontend::parser::CallExpr &Node)
  {
    const auto CallSite = State.at(Node);
    // Validate the complete AST shape before checking feature support or emitting any IR.
    validateCallShape(CallSite, Node);
    validateCallSupport(State, Node);

    auto &Builder = State.irBuilder();
    const auto CalleeSite = State.at(*Node.callee());
    const ir::Value *Callee = emitValue(State, *Node.callee(), {.IsCallCallee = true});
    RETURN_IF_NOT(Callee, nullptr);
    const ir::FunctionType *Signature = requireCallableSignature(CalleeSite, *Callee);
    RETURN_IF_NOT(Signature, nullptr);
    for (const auto &Parameter : Signature->parameters())
    {
      // TODO: Match variadic argument packs once the signature and operand lowering support them.
      CHECK_ICE(CallSite, !Parameter.Variadic, core::DiagnosticKind::SemanticUnsupported, "variadic calls");
    }
    RETURN_ERROR_IF(CallSite, Node.arguments().size() != Signature->parameterCount(), core::DiagnosticKind::SemanticArgumentCount, nullptr, Signature->parameterCount(), Node.arguments().size());

    std::vector<ir::Argument> Arguments;
    Arguments.reserve(Node.arguments().size());
    // Evaluate the callee first and each positional argument once, in source order.
    for (std::size_t Index = 0; Index < Node.arguments().size(); ++Index)
    {
      const auto &Argument = Node.arguments()[Index];
      const auto ArgumentSite = State.at(Argument.range());
      const ir::Type *ExpectedType = Signature->parameters()[Index].ParameterType;
      const ir::Value *Value = emitRuntimeValue(State, ArgumentSite, *Argument.value(), {.ExpectedType = ExpectedType});
      RETURN_IF_NOT(Value, nullptr);
      Arguments.emplace_back(*Value);
    }
    const ir::CallInstruction *Call = Builder.createCall(*Signature, *Callee, std::move(Arguments));
    CHECK_ICE(CallSite, Call, core::DiagnosticKind::SemanticConstructionFailed);
    return Call;
  }
} // namespace ink::semantic
