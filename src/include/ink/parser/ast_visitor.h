#pragma once
#include "ink/parser/ast.h"
#include <cstdlib>
namespace ink::parser
{
  template <typename Derived>
  class ASTVisitor
  {
    public:
      void visit(ASTNodeBase *Node)
      {
        assert(Node);
        switch (Node->getKind())
        {
#define AST_NODE(Name, Base, Category) \
  case ASTKind::Name:                  \
    return impl().visit##Name(static_cast<Name *>(Node));
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
        }
        std::abort();
      }
#define AST_NODE(Name, Base, Category) \
  void visit##Name(Name *Node)         \
  {                                    \
    impl().visit##Base(Node);          \
  }
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
      void visitExpr(Expr *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitStmt(Stmt *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitDecl(Decl *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitSimpleItem(SimpleItem *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitBindingPattern(BindingPattern *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitMatchPattern(MatchPattern *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitASTNodeBase(ASTNodeBase *)
      {
      }

    private:
      Derived &impl()
      {
        return static_cast<Derived &>(*this);
      }
  };
  template <typename Derived>
  class ConstASTVisitor
  {
    public:
      void visit(const ASTNodeBase *Node)
      {
        assert(Node);
        switch (Node->getKind())
        {
#define AST_NODE(Name, Base, Category) \
  case ASTKind::Name:                  \
    return impl().visit##Name(static_cast<const Name *>(Node));
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
        }
        std::abort();
      }
#define AST_NODE(Name, Base, Category) \
  void visit##Name(const Name *Node)   \
  {                                    \
    impl().visit##Base(Node);          \
  }
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
      void visitExpr(const Expr *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitStmt(const Stmt *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitDecl(const Decl *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitSimpleItem(const SimpleItem *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitBindingPattern(const BindingPattern *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitMatchPattern(const MatchPattern *Node)
      {
        impl().visitASTNodeBase(Node);
      }
      void visitASTNodeBase(const ASTNodeBase *)
      {
      }

    private:
      Derived &impl()
      {
        return static_cast<Derived &>(*this);
      }
  };
  // There are deliberately no expression leaf fallbacks.
  template <typename Derived, typename Result = void>
  class StrictExprVisitor
  {
    public:
      Result visit(const Expr *Node)
      {
        assert(Node);
        switch (Node->getKind())
        {
#define INK_STRICT_Expr(Name) \
  case ASTKind::Name:         \
    return static_cast<Derived &>(*this).visit##Name(static_cast<const Name *>(Node));
#define INK_STRICT_Root(Name)
#define INK_STRICT_Stmt(Name)
#define INK_STRICT_Decl(Name)
#define INK_STRICT_SimpleItem(Name)
#define INK_STRICT_BindingPattern(Name)
#define INK_STRICT_MatchPattern(Name)
#define AST_NODE(Name, Base, Category) INK_STRICT_##Category(Name)
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
#undef INK_STRICT_Expr
#undef INK_STRICT_Root
#undef INK_STRICT_Stmt
#undef INK_STRICT_Decl
#undef INK_STRICT_SimpleItem
#undef INK_STRICT_BindingPattern
#undef INK_STRICT_MatchPattern
        default:
          std::abort();
        }
      }
  };
} // namespace ink::parser
