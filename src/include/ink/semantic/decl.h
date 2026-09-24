#ifndef INK_SEMANTIC_DECL_H
#define INK_SEMANTIC_DECL_H

#include "ink/core/source_id.h"
#include "ink/core/source_range.h"
#include "ink/semantic/name.h"

namespace ink::parser
{
  class Decl;
} // namespace ink::parser

namespace ink::semantic
{
  class SemanticContext;
  class Type;
  class Value;

  enum class DeclKind : std::uint8_t
  {
    Variable,
    Class,
    Enum,
    Interface,
  };

  // Whether a binding can be assigned after initialization. This is independent
  // of access through its value and whether its initializer is known at compile time.
  enum class BindingMutability : std::uint8_t
  {
    Mutable,
    Immutable,
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

  class VarDecl final : public Decl
  {
    public:
      // Null until declaration analysis resolves the type.
      const Type *type() const noexcept
      {
        return ValueType;
      }

      // Publishes the type once. An existing initializer must have this type
      // after any required conversion; foreign types or conflicting updates fail.
      bool setType(const Type &ResolvedType) noexcept;

      BindingMutability mutability() const noexcept
      {
        return Mutability;
      }

      bool isMutable() const noexcept
      {
        return Mutability == BindingMutability::Mutable;
      }

      // Null when absent or not yet analyzed. The value belongs to context().
      const Value *initializer() const noexcept
      {
        return Initializer;
      }

      // Publishes a checked initializer once, matching the type if already known.
      bool setInitializer(const Value &InitializerValue) noexcept;

      static bool classof(const Decl *Declaration) noexcept
      {
        return Declaration && Declaration->kind() == DeclKind::Variable;
      }

    private:
      VarDecl(SemanticContext &Context, Name DeclName, BindingMutability Mutability, DeclSource Source, const Value *Initializer) noexcept
          : Decl(Context, DeclKind::Variable, DeclName, Source),
            Mutability(Mutability),
            Initializer(Initializer)
      {
      }

      const Type *ValueType = nullptr;
      BindingMutability Mutability;
      const Value *Initializer;

      friend class SemanticContext;
  };

  // A nominal type declaration can be registered before its members are checked.
  class TypeDecl final : public Decl
  {
    public:
      static bool classof(const Decl *Declaration) noexcept
      {
        if (!Declaration)
        {
          return false;
        }
        switch (Declaration->kind())
        {
        case DeclKind::Class:
        case DeclKind::Enum:
        case DeclKind::Interface:
          return true;
        default:
          return false;
        }
      }

    private:
      TypeDecl(SemanticContext &Context, DeclKind Kind, Name DeclName, DeclSource Source) noexcept
          : Decl(Context, Kind, DeclName, Source)
      {
      }

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
