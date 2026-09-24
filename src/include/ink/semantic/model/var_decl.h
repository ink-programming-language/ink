#ifndef INK_SEMANTIC_MODEL_VAR_DECL_H
#define INK_SEMANTIC_MODEL_VAR_DECL_H

#include "ink/semantic/model/decl.h"

namespace ink::semantic
{
  class Type;
  class Value;

  // Whether a binding can be assigned after initialization. This is independent
  // of access through its value and whether its initializer is known at compile time.
  enum class BindingMutability : std::uint8_t
  {
    Mutable,
    Immutable,
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
} // namespace ink::semantic

#endif
