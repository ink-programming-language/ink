#include "ink/parser/ast_walker.h"
namespace ink::parser
{
  namespace
  {
    template <typename Emit>
    void children(const Parameter &Record, Emit &&Visit)
    {
      if (Record.type())
      {
        Visit(Record.type());
      }
      if (Record.defaultValue())
      {
        Visit(Record.defaultValue());
      }
    }
    template <typename Emit>
    void children(const FunctionTypeParameter &Record, Emit &&Visit)
    {
      if (Record.type())
      {
        Visit(Record.type());
      }
    }
    template <typename Emit>
    void children(const Argument &Record, Emit &&Visit)
    {
      if (Record.value())
      {
        Visit(Record.value());
      }
    }
    template <typename Emit>
    void children(const PathSegment &Record, Emit &&Visit)
    {
      for (const auto &Entry : Record.arguments())
      {
        children(Entry, Visit);
      }
    }
    template <typename Emit>
    void children(const Attribute &Record, Emit &&Visit)
    {
      for (const auto &Entry : Record.arguments())
      {
        children(Entry, Visit);
      }
      if (Record.value())
      {
        Visit(Record.value());
      }
    }
    template <typename Emit>
    void children(const BaseSpec &Record, Emit &&Visit)
    {
      if (Record.type())
      {
        Visit(Record.type());
      }
    }
    template <typename Emit>
    void children(const MatchArm &Record, Emit &&Visit)
    {
      if (Record.pattern())
      {
        Visit(Record.pattern());
      }
      if (Record.guard())
      {
        Visit(Record.guard());
      }
      if (Record.value())
      {
        Visit(Record.value());
      }
    }
    template <typename Emit>
    void children(const SwitchClause &Record, Emit &&Visit)
    {
      if (Record.value())
      {
        Visit(Record.value());
      }
      for (auto *Child : Record.statements())
      {
        if (Child)
        {
          Visit(Child);
        }
      }
    }
    template <typename Emit>
    void children(const ImportEntry &Record, Emit &&Visit)
    {
      (void)Record;
      (void)Visit;
    }
    template <typename NodeType, typename Emit>
    void enumerate(NodeType *Node, Emit &&Visit)
    {
      switch (Node->getKind())
      {
      case ASTKind::ModuleAST:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ModuleAST, ModuleAST>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->statements())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::TypeSyntax:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const TypeSyntax, TypeSyntax>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->expression())
        {
          Visit(Value->expression());
        }
        return;
      }
      case ASTKind::MissingExpr:
      {
        return;
      }
      case ASTKind::ErrorExpr:
      {
        return;
      }
      case ASTKind::MissingStmt:
      {
        return;
      }
      case ASTKind::ErrorStmt:
      {
        return;
      }
      case ASTKind::MissingDecl:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const MissingDecl, MissingDecl>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->attributes())
        {
          children(Record, Visit);
        }
        return;
      }
      case ASTKind::ErrorDecl:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ErrorDecl, ErrorDecl>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->attributes())
        {
          children(Record, Visit);
        }
        return;
      }
      case ASTKind::MissingBindingPattern:
      {
        return;
      }
      case ASTKind::ErrorBindingPattern:
      {
        return;
      }
      case ASTKind::MissingMatchPattern:
      {
        return;
      }
      case ASTKind::ErrorMatchPattern:
      {
        return;
      }
      case ASTKind::NameExpr:
      {
        return;
      }
      case ASTKind::LiteralExpr:
      {
        return;
      }
      case ASTKind::UnaryExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const UnaryExpr, UnaryExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->operand())
        {
          Visit(Value->operand());
        }
        return;
      }
      case ASTKind::ComptimeExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ComptimeExpr, ComptimeExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->operand())
        {
          Visit(Value->operand());
        }
        return;
      }
      case ASTKind::BinaryExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const BinaryExpr, BinaryExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->left())
        {
          Visit(Value->left());
        }
        if (Value->right())
        {
          Visit(Value->right());
        }
        return;
      }
      case ASTKind::ConditionalExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ConditionalExpr, ConditionalExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->condition())
        {
          Visit(Value->condition());
        }
        if (Value->then())
        {
          Visit(Value->then());
        }
        if (Value->elseValue())
        {
          Visit(Value->elseValue());
        }
        return;
      }
      case ASTKind::ParenExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ParenExpr, ParenExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->expression())
        {
          Visit(Value->expression());
        }
        return;
      }
      case ASTKind::TupleExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const TupleExpr, TupleExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->elements())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::ArrayExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ArrayExpr, ArrayExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->elements())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::ArrayRepeatExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ArrayRepeatExpr, ArrayRepeatExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->value())
        {
          Visit(Value->value());
        }
        if (Value->count())
        {
          Visit(Value->count());
        }
        return;
      }
      case ASTKind::CallExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const CallExpr, CallExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->callee())
        {
          Visit(Value->callee());
        }
        for (auto &Record : Value->arguments())
        {
          children(Record, Visit);
        }
        return;
      }
      case ASTKind::IndexExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const IndexExpr, IndexExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->object())
        {
          Visit(Value->object());
        }
        if (Value->index())
        {
          Visit(Value->index());
        }
        return;
      }
      case ASTKind::MemberExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const MemberExpr, MemberExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->object())
        {
          Visit(Value->object());
        }
        return;
      }
      case ASTKind::GenericApplyExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const GenericApplyExpr, GenericApplyExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->object())
        {
          Visit(Value->object());
        }
        for (auto &Record : Value->arguments())
        {
          children(Record, Visit);
        }
        return;
      }
      case ASTKind::PostfixUpdateExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const PostfixUpdateExpr, PostfixUpdateExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->operand())
        {
          Visit(Value->operand());
        }
        return;
      }
      case ASTKind::FunctionTypeExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const FunctionTypeExpr, FunctionTypeExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->parameters())
        {
          children(Record, Visit);
        }
        if (Value->returnType())
        {
          Visit(Value->returnType());
        }
        return;
      }
      case ASTKind::LambdaExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const LambdaExpr, LambdaExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->genericParameters())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->parameters())
        {
          children(Record, Visit);
        }
        if (Value->returnType())
        {
          Visit(Value->returnType());
        }
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::MatchExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const MatchExpr, MatchExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->value())
        {
          Visit(Value->value());
        }
        for (auto &Record : Value->arms())
        {
          children(Record, Visit);
        }
        return;
      }
      case ASTKind::BlockExpr:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const BlockExpr, BlockExpr>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::ExprItem:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ExprItem, ExprItem>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->expression())
        {
          Visit(Value->expression());
        }
        return;
      }
      case ASTKind::AssignmentItem:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const AssignmentItem, AssignmentItem>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->left())
        {
          Visit(Value->left());
        }
        if (Value->right())
        {
          Visit(Value->right());
        }
        return;
      }
      case ASTKind::SimpleStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const SimpleStmt, SimpleStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->items())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::BlockStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const BlockStmt, BlockStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->statements())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::DeclStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const DeclStmt, DeclStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->declaration())
        {
          Visit(Value->declaration());
        }
        return;
      }
      case ASTKind::IfStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const IfStmt, IfStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->condition())
        {
          Visit(Value->condition());
        }
        if (Value->thenBranch())
        {
          Visit(Value->thenBranch());
        }
        if (Value->elseBranch())
        {
          Visit(Value->elseBranch());
        }
        return;
      }
      case ASTKind::WhileStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const WhileStmt, WhileStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->condition())
        {
          Visit(Value->condition());
        }
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::ClassicForStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ClassicForStmt, ClassicForStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->initializer())
        {
          Visit(Value->initializer());
        }
        if (Value->condition())
        {
          Visit(Value->condition());
        }
        for (auto *Child : Value->step())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::ForInStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ForInStmt, ForInStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->binding())
        {
          Visit(Value->binding());
        }
        if (Value->iterable())
        {
          Visit(Value->iterable());
        }
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::SwitchStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const SwitchStmt, SwitchStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->value())
        {
          Visit(Value->value());
        }
        for (auto &Record : Value->clauses())
        {
          children(Record, Visit);
        }
        return;
      }
      case ASTKind::ReturnStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ReturnStmt, ReturnStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->value())
        {
          Visit(Value->value());
        }
        return;
      }
      case ASTKind::BreakStmt:
      {
        return;
      }
      case ASTKind::ContinueStmt:
      {
        return;
      }
      case ASTKind::YieldStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const YieldStmt, YieldStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->value())
        {
          Visit(Value->value());
        }
        return;
      }
      case ASTKind::DeferStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const DeferStmt, DeferStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::ComptimeStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ComptimeStmt, ComptimeStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::DirectImportStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const DirectImportStmt, DirectImportStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->imports())
        {
          children(Record, Visit);
        }
        return;
      }
      case ASTKind::FromImportStmt:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const FromImportStmt, FromImportStmt>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->imports())
        {
          children(Record, Visit);
        }
        return;
      }
      case ASTKind::VarDecl:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const VarDecl, VarDecl>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->attributes())
        {
          children(Record, Visit);
        }
        if (Value->binding())
        {
          Visit(Value->binding());
        }
        if (Value->type())
        {
          Visit(Value->type());
        }
        if (Value->initializer())
        {
          Visit(Value->initializer());
        }
        return;
      }
      case ASTKind::FieldDecl:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const FieldDecl, FieldDecl>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->attributes())
        {
          children(Record, Visit);
        }
        if (Value->type())
        {
          Visit(Value->type());
        }
        for (auto &Record : Value->payload())
        {
          children(Record, Visit);
        }
        if (Value->initializer())
        {
          Visit(Value->initializer());
        }
        return;
      }
      case ASTKind::FunctionDecl:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const FunctionDecl, FunctionDecl>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->attributes())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->genericParameters())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->parameters())
        {
          children(Record, Visit);
        }
        if (Value->returnType())
        {
          Visit(Value->returnType());
        }
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::ClassDecl:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ClassDecl, ClassDecl>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->attributes())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->genericParameters())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->bases())
        {
          children(Record, Visit);
        }
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::EnumDecl:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const EnumDecl, EnumDecl>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->attributes())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->genericParameters())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->bases())
        {
          children(Record, Visit);
        }
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::InterfaceDecl:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const InterfaceDecl, InterfaceDecl>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->attributes())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->genericParameters())
        {
          children(Record, Visit);
        }
        for (auto &Record : Value->bases())
        {
          children(Record, Visit);
        }
        if (Value->body())
        {
          Visit(Value->body());
        }
        return;
      }
      case ASTKind::WildcardBindingPattern:
      {
        return;
      }
      case ASTKind::NameBindingPattern:
      {
        return;
      }
      case ASTKind::TupleBindingPattern:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const TupleBindingPattern, TupleBindingPattern>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->elements())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::ArrayBindingPattern:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ArrayBindingPattern, ArrayBindingPattern>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->elements())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::WildcardMatchPattern:
      {
        return;
      }
      case ASTKind::NameMatchPattern:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const NameMatchPattern, NameMatchPattern>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto &Record : Value->path())
        {
          children(Record, Visit);
        }
        for (auto *Child : Value->arguments())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::TupleMatchPattern:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const TupleMatchPattern, TupleMatchPattern>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->elements())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::ArrayMatchPattern:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const ArrayMatchPattern, ArrayMatchPattern>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->elements())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      case ASTKind::LiteralMatchPattern:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const LiteralMatchPattern, LiteralMatchPattern>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->value())
        {
          Visit(Value->value());
        }
        return;
      }
      case ASTKind::GroupedMatchPattern:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const GroupedMatchPattern, GroupedMatchPattern>;
        auto *Value = static_cast<Leaf *>(Node);
        if (Value->pattern())
        {
          Visit(Value->pattern());
        }
        return;
      }
      case ASTKind::OrMatchPattern:
      {
        using Leaf = std::conditional_t<std::is_const_v<NodeType>, const OrMatchPattern, OrMatchPattern>;
        auto *Value = static_cast<Leaf *>(Node);
        for (auto *Child : Value->alternatives())
        {
          if (Child)
          {
            Visit(Child);
          }
        }
        return;
      }
      }
    }
  } // namespace
  void forEachChild(const ASTNodeBase *Node, const std::function<void(const ASTNodeBase *)> &Visit)
  {
    enumerate(Node, Visit);
  }
} // namespace ink::parser
