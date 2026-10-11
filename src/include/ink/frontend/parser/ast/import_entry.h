#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast_context.h"
#include <optional>

namespace ink::frontend::parser
{
  class ImportEntry
  {
    public:
      ImportEntry(ASTArray<NameToken> Path, std::optional<NameToken> Alias, SourceRange Range)
          : Path(Path),
            Alias(Alias),
            Range(Range)
      {
      }
      ASTArray<NameToken> path() const noexcept
      {
        return Path;
      }
      std::optional<NameToken> alias() const noexcept
      {
        return Alias;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      ASTArray<NameToken> Path;
      std::optional<NameToken> Alias;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
