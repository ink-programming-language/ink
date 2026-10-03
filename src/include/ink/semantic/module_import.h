#ifndef INK_SEMANTIC_MODULE_IMPORT_H
#define INK_SEMANTIC_MODULE_IMPORT_H

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace ink::parser
{
  struct NameToken;
} // namespace ink::parser

namespace ink::semantic
{
  // Canonical module identities use dots. Relative level one names the importer's package.
  // Empty means an invalid path or a relative import that escapes the module root.
  std::string resolveImportModuleName(std::string_view Importer, std::span<const parser::NameToken> Path, std::size_t RelativeLevel = 0);
} // namespace ink::semantic

#endif
