#ifndef INK_AST_CST_LOWERING_H
#define INK_AST_CST_LOWERING_H

#include "ink/ast/ast_context.h"
#include "ink/core/source_file_id.h"
#include "ink/core/string_interner.h"
#include "ink/parser/parser.h"

namespace ink::ast
{
  class CstLowering
  {
  public:
    CstLowering(AstContext &Context, core::StringInterner &Strings) noexcept;
    AstFile lower(const parser::ParsedFile &ParsedFile, core::SourceFileId SourceFile);

  private:
    AstContext &Context;
    core::StringInterner &Strings;
  };

  AstFile lowerCst(const parser::ParsedFile &ParsedFile, core::SourceFileId SourceFile, AstContext &Context, core::StringInterner &Strings);
} // namespace ink::ast

#endif
