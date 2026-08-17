#ifndef INK_PARSER_PARSER_H
#define INK_PARSER_PARSER_H

#include "ink/core/context.h"
#include "ink/core/source_range.h"
#include "ink/parser/cst.h"
#include "ink/tokenizer/tokenizer.h"

#include <cstddef>
#include <vector>

namespace ink::parser
{
  enum class ParseMode
  {
    Batch,
    Interactive,
  };

  enum class ParseCompleteness
  {
    Complete,
    Incomplete,
  };

  struct ParserOptions
  {
      ParseMode Mode = ParseMode::Batch;
      std::size_t MaxSyntaxNestingDepth = 128;
  };

  class ParsedFile
  {
    public:
      const tokenizer::TokenizedBuffer &lexedFile() const noexcept
      {
        return LexedFile;
      }

      const CstTree &cst() const noexcept
      {
        return Tree;
      }

      const std::vector<core::Diagnostic> &diagnostics() const noexcept
      {
        return Diagnostics;
      }

      bool succeeded() const noexcept;
      ParseCompleteness completeness() const noexcept
      {
        return Completeness;
      }

      core::SourceRange span(CstNodeId Id) const;

    private:
      ParsedFile(tokenizer::TokenizedBuffer LexedFile, CstTree Tree, std::vector<core::Diagnostic> Diagnostics, ParseCompleteness Completeness);

      tokenizer::TokenizedBuffer LexedFile;
      CstTree Tree;
      std::vector<core::Diagnostic> Diagnostics;
      ParseCompleteness Completeness = ParseCompleteness::Complete;

      friend class Parser;
  };

  class Parser
  {
    public:
      explicit Parser(core::FrontendContext &Context, ParserOptions Options = {});
      ParsedFile parse(tokenizer::TokenizedBuffer LexedFile) const;

    private:
      core::FrontendContext &Context;
      ParserOptions Options;
  };

  ParsedFile parse(core::FrontendContext &Context, tokenizer::TokenizedBuffer LexedFile, ParserOptions Options = {});
  ParsedFile parse(tokenizer::TokenizedBuffer LexedFile, ParserOptions Options = {});
} // namespace ink::parser

#endif
