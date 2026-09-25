#ifndef INK_SEMANTIC_ANALYZER_H
#define INK_SEMANTIC_ANALYZER_H

#include <string_view>

namespace ink::parser
{
  struct ParseResult;
}

namespace ink::semantic
{
  class SemanticContext;
  class Module;

  // Input and Context must use the same CompilationContext. Returns null for
  // failed parsing or analysis; semantic errors go to its DiagnosticEngine.
  // The returned graph is owned by Context and does not borrow the input AST.
  // Failed analysis can retain unreachable allocations until Context is destroyed.
  Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName = "main");
} // namespace ink::semantic

#endif
