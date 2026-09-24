#ifndef INK_SEMANTIC_MODEL_DECL_H
#define INK_SEMANTIC_MODEL_DECL_H

#include "ink/core/source_id.h"
#include "ink/core/source_range.h"
#include "ink/semantic/model/name.h"

namespace ink::parser
{
  class Decl;
} // namespace ink::parser

namespace ink::semantic
{
  class SemanticContext;

  enum class DeclKind : std::uint8_t
  {
    Variable,
    Class,
    Enum,
    Interface,
  };

  // AST pointers are borrowed. Their ParsedUnit must outlive these declarations.
  // Generated declarations may omit Syntax and/or their source range.
  struct DeclSource
  {
      core::SourceId Source;
      core::SourceRange Range;
      const parser::Decl *Syntax = nullptr;
  };

  // The stable address of a Decl is its identity within a SemanticContext.
  // Identical names, types and initializers never merge declarations.
  class Decl
  {
    public:
      virtual ~Decl() = default;
      Decl(const Decl &) = delete;
      Decl &operator=(const Decl &) = delete;
      Decl(Decl &&) = delete;
      Decl &operator=(Decl &&) = delete;

      DeclKind kind() const noexcept
      {
        return Kind;
      }

      Name name() const noexcept
      {
        return DeclName;
      }

      const SemanticContext &context() const noexcept
      {
        return Context;
      }

      const DeclSource &source() const noexcept
      {
        return Source;
      }

    protected:
      Decl(SemanticContext &Context, DeclKind Kind, Name DeclName, DeclSource Source) noexcept
          : Context(Context),
            Kind(Kind),
            DeclName(DeclName),
            Source(Source)
      {
      }

    private:
      const SemanticContext &Context;
      DeclKind Kind;
      Name DeclName;
      DeclSource Source;
  };
} // namespace ink::semantic

#endif
