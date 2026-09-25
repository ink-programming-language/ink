#ifndef INK_SEMANTIC_ANALYZE_ANALYZER_H
#define INK_SEMANTIC_ANALYZE_ANALYZER_H

#include <string_view>

namespace ink::parser
{
  struct ParseResult;
}

namespace ink::semantic
{
  class SemanticContext;
  class Module;

  class Analyzer
  {
    public:
      // Placeholder entry point; returns null without analyzing input or reporting diagnostics.
      Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName = "main");
  };
} // namespace ink::semantic

#endif
