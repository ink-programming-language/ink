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
    const auto Site = State.at(Node);
    auto &Builder = State.irBuilder();
    auto &Resolver = State.nameResolver();
    CHECK_ICE(Site, Node.binding(), core::DiagnosticKind::SemanticInvalidAST, "VarDecl requires a binding pattern");
    // TODO: Validate and lower variable attributes.
    CHECK_ICE(Site, Node.attributes().empty(), core::DiagnosticKind::SemanticUnsupported, "variable attributes");
    // TODO: Lower discard, tuple, array and rest bindings with single evaluation of the initializer.
    CHECK_ICE(Site, frontend::parser::NameBindingPattern::classof(Node.binding()), core::DiagnosticKind::SemanticUnsupported, "variable binding patterns");
    // TODO: Lower top-level variables into module storage and its initialization function.
    CHECK_ICE(Site, Builder.insertFunction(), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    RETURN_IF(State.AnalysisMode == AnalyzerState::Mode::CollectDecls, true);
    CHECK_ICE(Site, Builder.insertBlock() && !Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed);
    const auto NameToken = static_cast<const frontend::parser::NameBindingPattern *>(Node.binding())->name();
    const auto NameSite = State.at(NameToken.Range);
    CHECK_ICE(NameSite, !NameToken.Text.empty(), core::DiagnosticKind::SemanticInvalidAST, "variable name must be nonempty");
    const auto Name = core::NamePool::instance().intern(NameToken.Text);
    CHECK_ICE(NameSite, Name.valid(), core::DiagnosticKind::SemanticConstructionFailed);
    RETURN_REPORT_IF(NameSite, Resolver.lookupLocal(Name), core::DiagnosticKind::SemanticDuplicateName, false, NameToken.Text);
    CHECK_ICE(Site, !Node.constant() || Node.initializer(), core::DiagnosticKind::SemanticInvalidAST, "const declaration requires an initializer");

    const ir::Type *Type = nullptr;
    if (Node.type())
    {
      Type = analyzeTypeWithLayout(State, *Node.type(), "a variable type with runtime layout");
      RETURN_IF_NULL(Type, false);
    }
    RETURN_REPORT_IF(Site, !Type && !Node.initializer(), core::DiagnosticKind::SemanticMissingVariableType, false);
    const ir::Value *Initializer = nullptr;
    if (Node.initializer())
    {
      // The new name becomes visible only after its initializer succeeds; outer bindings remain visible here.
      Initializer = emitRuntimeValue(State, *Node.initializer(), {.ExpectedType = Type});
      RETURN_IF(!Initializer, false);
      if (!Type)
      {
        Type = Initializer->type()->unqualifiedType();
      }
    }
    // TODO: Represent zero-sized unit bindings without allocating runtime storage.
    CHECK_ICE(Site, !ir::VoidType::classof(Type), core::DiagnosticKind::SemanticUnsupported, "unit variable storage");
    ir::AllocaInstruction *Storage = Builder.createAlloca(*Type, !Node.constant());
    CHECK_ICE(Site, Storage, core::DiagnosticKind::SemanticConstructionFailed);
    Storage->setName(Name);
    CHECK_ICE(Site, !Initializer || Builder.createInitialize(*Initializer, *Storage), core::DiagnosticKind::SemanticConstructionFailed);
    CHECK_ICE(NameSite, Resolver.bind(Name, *Storage) == BindResult::Inserted, core::DiagnosticKind::SemanticConstructionFailed);
    return true;
  }
} // namespace ink::semantic
