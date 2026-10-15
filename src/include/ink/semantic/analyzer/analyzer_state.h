#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_STATE_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_STATE_H

#include "ink/semantic/name_resolve/name_resolver.h"

#include <cstddef>

namespace ink::frontend::parser
{
  struct ParseResult;
} // namespace ink::frontend::parser

namespace ink::ir
{
  class Module;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;

  // Owns traversal state for one analyze call; Context, Input and Output must outlive it.
  class AnalyzerState final
  {
    public:
      enum class Mode
      {
        CollectDecls,
        FullAnalysis,
      };

      // Temporarily selects a mode; State must outlive the guard.
      class Guard final
      {
        public:
          Guard(AnalyzerState &State, Mode AnalysisMode) noexcept;
          ~Guard() noexcept;

          Guard(const Guard &) = delete;
          Guard &operator=(const Guard &) = delete;
          Guard(Guard &&) = delete;
          Guard &operator=(Guard &&) = delete;

        private:
          AnalyzerState &State;
          Mode SavedMode;
      };

      AnalyzerState(SemanticContext &Context, const frontend::parser::ParseResult &Input, ir::Module &Output, Mode AnalysisMode = Mode::FullAnalysis);

      AnalyzerState(const AnalyzerState &) = delete;
      AnalyzerState &operator=(const AnalyzerState &) = delete;
      AnalyzerState(AnalyzerState &&) = delete;
      AnalyzerState &operator=(AnalyzerState &&) = delete;

      auto &nameResolver(this auto &&Self) noexcept
      {
        return Self.Resolver;
      }

    private:
      friend class Analyzer;

      SemanticContext &Context;
      const frontend::parser::ParseResult &Input;
      ir::Module &Output;
      // Each call has an independent cursor; node handlers select the required module/member/block scopes.
      NameResolver Resolver;
      Mode AnalysisMode = Mode::FullAnalysis;
      std::size_t RecursionDepthLimit;
      std::size_t RecursionDepth = 0;
  };
} // namespace ink::semantic

#endif
