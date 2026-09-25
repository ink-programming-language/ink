#include "ink/semantic/analyze/analyzer.h"

namespace ink::semantic
{
  Module *Analyzer::analyze(SemanticContext &, const parser::ParseResult &, std::string_view)
  {
    return nullptr;
  }
} // namespace ink::semantic
