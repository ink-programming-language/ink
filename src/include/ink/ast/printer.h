#ifndef INK_AST_PRINTER_H
#define INK_AST_PRINTER_H

#include "ink/ast/ast_context.h"
#include "ink/core/string_interner.h"

#include <iosfwd>
#include <string>

namespace ink::ast
{
  class AstPrinter
  {
  public:
    void print(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings, std::ostream &Output) const;
    std::string print(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings) const;
  };

  void printAst(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings, std::ostream &Output);
  std::string printAst(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings);
} // namespace ink::ast

#endif
