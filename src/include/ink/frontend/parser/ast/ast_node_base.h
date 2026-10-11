#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast_kind.h"

namespace ink::frontend::parser
{
  class ASTNodeBase
  {
    public:
      ASTKind getKind() const noexcept
      {
        return NodeKind;
      }
      SourceRange getSourceRange() const noexcept
      {
        return Range;
      }
      void setSourceRange(SourceRange Value) noexcept
      {
        Range = Value;
      }
      bool isComptime() const noexcept
      {
        return Comptime;
      }
      void setComptime(bool Value = true) noexcept
      {
        Comptime = Value;
      }
      core::SourceLocation getLocation() const noexcept
      {
        return Range.getBegin();
      }
      ASTNodeBase(const ASTNodeBase &) = delete;
      ASTNodeBase &operator=(const ASTNodeBase &) = delete;

    protected:
      ASTNodeBase(ASTKind Kind, SourceRange Range)
          : NodeKind(Kind),
            Range(Range)
      {
      }
      ~ASTNodeBase() = default;

    private:
      ASTKind NodeKind;
      bool Comptime = false;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
