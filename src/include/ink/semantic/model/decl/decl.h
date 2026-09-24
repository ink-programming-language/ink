#ifndef INK_SEMANTIC_MODEL_DECL_H
#define INK_SEMANTIC_MODEL_DECL_H

#include "ink/semantic/model/name.h"

namespace ink::parser
{
  class Decl;
} // namespace ink::parser

namespace ink::semantic
{
  // An uninstantiated generic definition. Its AST is borrowed and must outlive the context.
  // Resolved types, values, bindings and instance state are stored separately.
  class Decl
  {
    public:
      virtual ~Decl() = default;
      Decl(const Decl &) = delete;
      Decl &operator=(const Decl &) = delete;
      Decl(Decl &&) = delete;
      Decl &operator=(Decl &&) = delete;

      Name name() const noexcept
      {
        return DeclName;
      }

      const parser::Decl &ast() const noexcept
      {
        return AST;
      }

    private:
      Decl(Name DeclName, const parser::Decl &AST) noexcept
          : DeclName(DeclName),
            AST(AST)
      {
      }

      Name DeclName;
      const parser::Decl &AST;

      friend class FunctionDecl;
      friend class ClassDecl;
  };
} // namespace ink::semantic

#endif
