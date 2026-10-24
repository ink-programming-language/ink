#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_STATE_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_STATE_H

#include "ink/ir/builder/ir_builder.h"
#include "ink/semantic/analyzer/analysis_site.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <cstddef>
#include <utility>

namespace ink::frontend::parser
{
  class ASTNodeBase;
  struct ParseResult;
  class TokenBuffer;
} // namespace ink::frontend::parser

namespace ink::ir
{
  class Module;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;

  // Owns traversal state and a builder for one analyze call; Context, Input and Output must outlive it.
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

      auto &semanticContext(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Context);
      }

      // Exposes the token input without exposing the parsed unit's storage.
      const frontend::parser::TokenBuffer &input() const noexcept;

      // Captures a diagnostic range belonging to this state's input without changing context source selection.
      AnalysisSite at(core::SourceRange Range) const;
      AnalysisSite at(const frontend::parser::ASTNodeBase &Node) const;

      auto &nameResolver(this auto &&Self) noexcept
      {
        return Self.Resolver;
      }

      auto &irBuilder(this auto &&Self) noexcept
      {
        return Self.Builder;
      }

      auto &recursionDepth(this auto &&Self) noexcept
      {
        return Self.RecursionDepth;
      }

      std::size_t recursionDepthLimit(this auto &&Self) noexcept
      {
        return Self.RecursionDepthLimit;
      }

    private:
      friend class Analyzer;

      SemanticContext &Context;
      const frontend::parser::ParseResult &Input;
      ir::Module &Output;
      // Each call has an independent cursor; node handlers select the required module/member/block scopes.
      NameResolver Resolver;
      // Borrows Context and any selected block in Output; owns no IR.
      ir::IRBuilder Builder;
      Mode AnalysisMode = Mode::FullAnalysis;
      std::size_t RecursionDepthLimit;
      std::size_t RecursionDepth = 0;
  };
} // namespace ink::semantic

#endif
