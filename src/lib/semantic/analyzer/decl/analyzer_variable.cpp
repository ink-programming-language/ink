#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  bool Analyzer::analyzeVarDecl(AnalyzerState &State, const frontend::parser::VarDecl &Node)
  {
    auto &Context = State.semanticContext();
    auto &Builder = State.irBuilder();
    auto &Resolver = State.nameResolver();
    RETURN_REPORT_IF_FALSE(Context, Node.binding(), core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "VarDecl requires a binding pattern");
    // TODO: Validate and lower variable attributes.
    UNSUPPORT_IF(State, !Node.attributes().empty(), false, Node.getSourceRange(), "variable attributes");
    // TODO: Lower discard, tuple, array and rest bindings with single evaluation of the initializer.
    UNSUPPORT_IF(State, !frontend::parser::NameBindingPattern::classof(Node.binding()), false, Node.getSourceRange(), "variable binding patterns");
    // TODO: Lower top-level variables into module storage and its initialization function.
    UNSUPPORT_IF(State, !Builder.insertFunction(), false, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    RETURN_IF(State.AnalysisMode == AnalyzerState::Mode::CollectDecls, true);
    RETURN_REPORT_IF(Context, !Builder.insertBlock() || Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    const auto NameToken = static_cast<const frontend::parser::NameBindingPattern *>(Node.binding())->name();
    RETURN_REPORT_IF(Context, NameToken.Text.empty(), core::DiagnosticKind::SemanticInvalidAST, false, NameToken.Range, "variable name must be nonempty");
    const auto Name = core::NamePool::instance().intern(NameToken.Text);
    CHECK_VALID_NAME(Context, NameToken.Range, Name, false);
    RETURN_REPORT_IF(Context, Resolver.lookupLocal(Name), core::DiagnosticKind::SemanticDuplicateName, false, NameToken.Range, NameToken.Text);
    RETURN_REPORT_IF(Context, Node.constant() && !Node.initializer(), core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "const declaration requires an initializer");

    const ir::Type *Type = nullptr;
    if (Node.type())
    {
      Type = analyzeType(State, *Node.type());
      RETURN_IF_NULL(Type, false);
      CHECK_HAS_LAYOUT(Context, Node.type()->getSourceRange(), Type, "a variable type with runtime layout", false);
    }
    RETURN_REPORT_IF(Context, !Type && !Node.initializer(), core::DiagnosticKind::SemanticMissingVariableType, false, Node.getSourceRange());
    const ir::Value *Initializer = nullptr;
    if (Node.initializer())
    {
      // The new name becomes visible only after its initializer succeeds; outer bindings remain visible here.
      Initializer = requireRuntimeValue(Context, Node.initializer()->getSourceRange(), emitValue(State, *Node.initializer(), {.ExpectedType = Type}));
      RETURN_IF(!Initializer, false);
      if (!Type)
      {
        Type = Initializer->type()->unqualifiedType();
      }
    }
    // TODO: Represent zero-sized unit bindings without allocating runtime storage.
    UNSUPPORT_IF(State, ir::VoidType::classof(Type), false, Node.getSourceRange(), "unit variable storage");
    ir::AllocaInstruction *Storage = Builder.createAlloca(*Type, !Node.constant());
    RETURN_REPORT_IF_FALSE(Context, Storage, core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    Storage->setName(Name);
    RETURN_REPORT_IF(Context, Initializer && !Builder.createInitialize(*Initializer, *Storage), core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    RETURN_REPORT_IF(Context, Resolver.bind(Name, *Storage) != BindResult::Inserted, core::DiagnosticKind::SemanticConstructionFailed, false, NameToken.Range);
    return true;
  }
} // namespace ink::semantic
