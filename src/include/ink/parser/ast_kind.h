#pragma once
#include <cstdint>
namespace ink::parser
{
  enum class ASTCategory
  {
    Root,
    Expr,
    Stmt,
    Decl,
    SimpleItem,
    BindingPattern,
    MatchPattern
  };
  enum class ASTKind : std::uint8_t
  {
#define AST_NODE(Name, Base, Category, Id) Name = Id,
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
  };
  constexpr ASTCategory categoryOf(ASTKind Kind)
  {
    switch (Kind)
    {
#define AST_NODE(Name, Base, Category, Id) \
  case ASTKind::Name:                      \
    return ASTCategory::Category;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
    }
    return ASTCategory::Root;
  }
  constexpr const char *astKindName(ASTKind Kind)
  {
    switch (Kind)
    {
#define AST_NODE(Name, Base, Category, Id) \
  case ASTKind::Name:                      \
    return #Name;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
    }
    return "InvalidASTKind";
  }
} // namespace ink::parser
