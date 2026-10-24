#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/module/global_variable.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::loadLocation(AnalyzerState &State, const AnalysisSite &Site, ExprResult Location)
  {
    auto &Context = State.semanticContext();
    const ir::Type *Type = Location.type();
    CHECK_ICE(Site, Location.isLocation() && Context.irContext().containsType(Type) && Type->hasLayout(), core::DiagnosticKind::SemanticConstructionFailed);
    auto &Builder = State.irBuilder();
    CHECK_ICE(Site, Builder.insertBlock() && !Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed);
    const ir::Value *Loaded = Builder.createLoad(*Type, *Location.value());
    CHECK_ICE(Site, Loaded, core::DiagnosticKind::SemanticConstructionFailed);
    // Only direct local declarations carry this advisory state; an indirect location does not prove initialization.
    const auto *Local = ir::AllocaInstruction::classof(Location.declaration()) ? static_cast<const ir::AllocaInstruction *>(Location.declaration()) : nullptr;
    WARNING_IF(Site, Local && !Local->isInitialized(), core::DiagnosticKind::SemanticUninitializedRead, core::NamePool::instance().text(Local->name()));
    return Loaded;
  }
} // namespace ink::semantic
