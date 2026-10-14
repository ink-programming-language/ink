#include "ink/semantic/analyzer/analyzer_state.h"

#include "ink/core/config/config_manager.h"

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
        AnalysisMode(AnalysisMode),
        BlockDepthLimit(core::ConfigManager::getSize<core::ConfigKind::SemanticBlockDepthLimit>())
  {
  }
} // namespace ink::semantic
