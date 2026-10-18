#include "ink/semantic/analyzer/analyzer_state.h"

#include "ink/core/config/config_manager.h"
#include "ink/frontend/parser/parser.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  AnalyzerState::Guard::Guard(AnalyzerState &State, Mode AnalysisMode) noexcept
      : State(State),
        SavedMode(State.AnalysisMode)
  {
    State.AnalysisMode = AnalysisMode;
  }

  AnalyzerState::Guard::~Guard() noexcept
  {
    State.AnalysisMode = SavedMode;
  }

  AnalyzerState::AnalyzerState(SemanticContext &Context, const frontend::parser::ParseResult &Input, ir::Module &Output, Mode AnalysisMode)
      : Context(Context),
        Input(Input),
        Output(Output),
        Resolver(Context),
        Builder(Context.irContext()),
        AnalysisMode(AnalysisMode),
        RecursionDepthLimit(core::ConfigManager::getSize<core::ConfigKind::SemanticRecursionDepthLimit>())
  {
  }

  const frontend::parser::TokenBuffer &AnalyzerState::input() const noexcept
  {
    if (!Input.Unit)
    {
      Context.diagnosticEngine().report<core::DiagnosticKind::SemanticInvalidAST>({}, "analysis input requires a parsed unit");
    }
    return Input.Unit->input();
  }
} // namespace ink::semantic
