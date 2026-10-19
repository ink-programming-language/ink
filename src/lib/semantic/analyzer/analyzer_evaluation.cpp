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
    auto &Context = State.semanticContext();
    // TODO: Lower explicit global storage reads through the module's address representation.
    UNSUPPORT_IF(State, ir::GlobalVariable::classof(&Storage), nullptr, Range, "variable storage reads");
    const ir::Type *Type = Storage.type();
    RETURN_REPORT_IF(Context, !Storage.isStorage() || !Context.irContext().containsType(Type) || !Type->hasLayout(), core::DiagnosticKind::SemanticConstructionFailed, nullptr, Range);
    const core::Name Name = ir::AllocaInstruction::classof(&Storage) ? static_cast<const ir::AllocaInstruction &>(Storage).name() : core::NAME_None;
    RETURN_REPORT_IF_FALSE(Context, Storage.isInitialized(), core::DiagnosticKind::SemanticUninitializedRead, nullptr, Range, core::NamePool::instance().text(Name));
    auto &Builder = State.irBuilder();
    RETURN_REPORT_IF(Context, !Builder.insertBlock() || Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed, nullptr, Range);
    const ir::Value *Loaded = Builder.createLoad(*Type, Storage);
    RETURN_REPORT_IF_FALSE(Context, Loaded, core::DiagnosticKind::SemanticConstructionFailed, nullptr, Range);
    return Loaded;
  }
} // namespace ink::semantic
