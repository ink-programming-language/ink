#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/module/global_variable.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::loadStorage(AnalyzerState &State, core::SourceRange Range, const ir::Value &Storage)
  {
    const auto Site = State.at(Range);
    auto &Context = State.semanticContext();
    // TODO: Lower explicit global storage reads through the module's address representation.
    CHECK_ICE(Site, !ir::GlobalVariable::classof(&Storage), core::DiagnosticKind::SemanticUnsupported, "variable storage reads");
    const ir::Type *Type = Storage.type();
    CHECK_ICE(Site, Storage.isStorage() && Context.irContext().containsType(Type) && Type->hasLayout(), core::DiagnosticKind::SemanticConstructionFailed);
    auto &Builder = State.irBuilder();
    CHECK_ICE(Site, Builder.insertBlock() && !Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed);
    const ir::Value *Loaded = Builder.createLoad(*Type, Storage);
    CHECK_ICE(Site, Loaded, core::DiagnosticKind::SemanticConstructionFailed);
    // Declaration initialization is advisory; indirect storage has no declaration flag to check.
    if (ir::AllocaInstruction::classof(&Storage) && !Storage.isInitialized())
    {
      const core::Name Name = static_cast<const ir::AllocaInstruction &>(Storage).name();
      Site.report<core::DiagnosticKind::SemanticUninitializedRead>(core::NamePool::instance().text(Name));
    }
    return Loaded;
  }
} // namespace ink::semantic
