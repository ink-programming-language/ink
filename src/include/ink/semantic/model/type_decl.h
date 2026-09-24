#ifndef INK_SEMANTIC_MODEL_TYPE_DECL_H
#define INK_SEMANTIC_MODEL_TYPE_DECL_H

#include "ink/semantic/model/decl.h"

namespace ink::semantic
{
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
