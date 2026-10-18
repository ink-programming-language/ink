#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/module/global_variable.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  ExprResult Analyzer::toRValue(AnalyzerState &State, core::SourceRange Range, ExprResult Result)
  {
    auto &Context = State.semanticContext();
    RETURN_IF(!Result || !requireRuntimeValue(Context, Range, Result), ExprResult{});
    RETURN_IF(Result.Kind == ExprResultKind::RValue, Result);
    RETURN_REPORT_IF_FALSE(Context, Result.Kind == ExprResultKind::LValue, core::DiagnosticKind::SemanticConstructionFailed, ExprResult{}, Range);
    // TODO: Lower explicit global storage reads through the module's address representation.
    UNSUPPORT_IF(State, ir::GlobalVariable::classof(Result.Value), ExprResult{}, Range, "variable storage reads");
    if (Result.Variable)
    {
      const auto Initialized = State.InitializationStates.find(Result.Variable);
      RETURN_REPORT_IF(Context, Initialized == State.InitializationStates.end() || Initialized->second != InitializationState::Initialized, core::DiagnosticKind::SemanticUninitializedRead, ExprResult{}, Range, core::NamePool::instance().text(Result.Variable->name()));
    }
    auto &Builder = State.irBuilder();
    RETURN_REPORT_IF(Context, !Context.irContext().containsType(Result.Type) || !Builder.insertBlock() || Builder.insertBlock()->hasTerminator(), core::DiagnosticKind::SemanticConstructionFailed, ExprResult{}, Range);
    const ir::Value *Loaded = Builder.createLoad(*Result.Type, *Result.Value);
    RETURN_REPORT_IF_FALSE(Context, Loaded, core::DiagnosticKind::SemanticConstructionFailed, ExprResult{}, Range);
    return ExprResult::fromValue(Loaded);
  }
} // namespace ink::semantic
