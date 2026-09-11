#include "ink/parser/ast.h"

#include <algorithm>
#include <type_traits>

namespace ink::parser
{
  const char *astKindName(AstKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_AST_KIND(Name, Base) \
  case AstKind::Name:            \
    return #Name;
#include "ink/parser/ast_kind.def"
#undef INK_AST_KIND
    }
    return "Unknown";
  }

  bool AstDeclaration::classof(const AstNode *Node) noexcept
  {
    switch (Node->kind())
    {
#define INK_AST_KIND(Name, Base) \
  case AstKind::Name:            \
    return std::is_base_of_v<AstDeclaration, Name>;
#include "ink/parser/ast_kind.def"
#undef INK_AST_KIND
    }
    return false;
  }

  bool AstStatement::classof(const AstNode *Node) noexcept
  {
    switch (Node->kind())
    {
#define INK_AST_KIND(Name, Base) \
  case AstKind::Name:            \
    return std::is_base_of_v<AstStatement, Name>;
#include "ink/parser/ast_kind.def"
#undef INK_AST_KIND
    }
    return false;
  }

  bool AstExpression::classof(const AstNode *Node) noexcept
  {
    switch (Node->kind())
    {
#define INK_AST_KIND(Name, Base) \
  case AstKind::Name:            \
    return std::is_base_of_v<AstExpression, Name>;
#include "ink/parser/ast_kind.def"
#undef INK_AST_KIND
    }
    return false;
  }

  bool AstPattern::classof(const AstNode *Node) noexcept
  {
    switch (Node->kind())
    {
#define INK_AST_KIND(Name, Base) \
  case AstKind::Name:            \
    return std::is_base_of_v<AstPattern, Name>;
#include "ink/parser/ast_kind.def"
#undef INK_AST_KIND
    }
    return false;
  }

  std::vector<AstChildEdge> astChildren(const AstNode &Node)
  {
    std::vector<AstChildEdge> Result;
    const auto Add = [&Result](const char *Role, AstNodeId Id, std::size_t Index = InvalidAstNodeId)
    {
      if (Id != InvalidAstNodeId)
      {
        Result.push_back({Role, Id, Index});
      }
    };
    const auto AddList = [&Add](const char *Role, const std::vector<AstNodeId> &Ids)
    {
      for (std::size_t Index = 0; Index < Ids.size(); ++Index)
      {
        Add(Role, Ids[Index], Index);
      }
    };
    const auto Visit = [&](const auto &Value)
    {
      using NodeType = std::decay_t<decltype(Value)>;
      if constexpr (std::is_same_v<NodeType, SourceFile> || std::is_same_v<NodeType, BlockStatement>)
      {
        AddList("Statements", Value.Statements);
      }
      else if constexpr (std::is_same_v<NodeType, Error>)
      {
        AddList("Recovered", Value.Recovered);
      }
      else if constexpr (std::is_same_v<NodeType, FunctionDeclaration>)
      {
        AddList("GenericParameters", Value.GenericParameters);
        AddList("Parameters", Value.Parameters);
        Add("ReturnType", Value.ReturnType);
        Add("Body", Value.Body);
      }
      else if constexpr (std::is_same_v<NodeType, ClassDeclaration> || std::is_same_v<NodeType, EnumDeclaration>)
      {
        AddList("GenericParameters", Value.GenericParameters);
        Add("BaseType", Value.BaseType);
        AddList("Interfaces", Value.Interfaces);
        Add("Body", Value.Body);
      }
      else if constexpr (std::is_same_v<NodeType, InterfaceDeclaration>)
      {
        AddList("GenericParameters", Value.GenericParameters);
        AddList("BaseTypes", Value.BaseTypes);
        Add("Body", Value.Body);
      }
      else if constexpr (std::is_same_v<NodeType, ClassFieldDeclaration>)
      {
        Add("TypeExpression", Value.TypeExpression);
        Add("Initializer", Value.Initializer);
      }
      else if constexpr (std::is_same_v<NodeType, EnumFieldDeclaration>)
      {
        Add("Initializer", Value.Initializer);
      }
      else if constexpr (std::is_same_v<NodeType, BindingDeclaration>)
      {
        Add("Pattern", Value.Pattern);
        Add("TypeExpression", Value.TypeExpression);
        Add("Initializer", Value.Initializer);
      }
      else if constexpr (std::is_same_v<NodeType, FunctionParameter>)
      {
        Add("Pattern", Value.Pattern);
        Add("TypeExpression", Value.TypeExpression);
        Add("DefaultValue", Value.DefaultValue);
      }
      else if constexpr (std::is_same_v<NodeType, GenericParameter>)
      {
        Add("TypeExpression", Value.TypeExpression);
        Add("DefaultValue", Value.DefaultValue);
      }
      else if constexpr (std::is_same_v<NodeType, TuplePattern> || std::is_same_v<NodeType, TupleExpression> || std::is_same_v<NodeType, ArrayExpression>)
      {
        AddList("Elements", Value.Elements);
      }
      else if constexpr (std::is_same_v<NodeType, ExpressionStatement> || std::is_same_v<NodeType, ParenthesizedExpression>)
      {
        Add("Expression", Value.Expression);
      }
      else if constexpr (std::is_same_v<NodeType, AssignmentStatement>)
      {
        Add("Target", Value.Target);
        Add("Value", Value.Value);
      }
      else if constexpr (std::is_same_v<NodeType, IfStatement> || std::is_same_v<NodeType, ConditionalExpression>)
      {
        Add("Condition", Value.Condition);
        Add("Then", Value.Then);
        Add("Else", Value.Else);
      }
      else if constexpr (std::is_same_v<NodeType, WhileStatement>)
      {
        Add("Condition", Value.Condition);
        Add("Body", Value.Body);
      }
      else if constexpr (std::is_same_v<NodeType, ForStatement>)
      {
        AddList("Initializers", Value.Initializers);
        Add("Condition", Value.Condition);
        AddList("Updates", Value.Updates);
        Add("Body", Value.Body);
      }
      else if constexpr (std::is_same_v<NodeType, ForInStatement>)
      {
        Add("Pattern", Value.Pattern);
        Add("TypeExpression", Value.TypeExpression);
        Add("Iterable", Value.Iterable);
        Add("Body", Value.Body);
      }
      else if constexpr (std::is_same_v<NodeType, ReturnStatement>)
      {
        Add("Value", Value.Value);
      }
      else if constexpr (std::is_same_v<NodeType, DeferStatement>)
      {
        Add("Action", Value.Action);
      }
      else if constexpr (std::is_same_v<NodeType, ComptimeStatement>)
      {
        Add("Statement", Value.Statement);
      }
      else if constexpr (std::is_same_v<NodeType, BinaryExpression>)
      {
        Add("Left", Value.Left);
        Add("Right", Value.Right);
      }
      else if constexpr (std::is_same_v<NodeType, UnaryExpression> || std::is_same_v<NodeType, ComptimeExpression> || std::is_same_v<NodeType, ReferenceTypeExpression> || std::is_same_v<NodeType, PointerTypeExpression>)
      {
        Add("Operand", Value.Operand);
      }
      else if constexpr (std::is_same_v<NodeType, FunctionTypeExpression>)
      {
        for (std::size_t Index = 0; Index < Value.Parameters.size(); ++Index)
        {
          Add("Parameters.TypeExpression", Value.Parameters[Index].TypeExpression, Index);
        }
        Add("ReturnType", Value.ReturnType);
      }
      else if constexpr (std::is_same_v<NodeType, CallExpression> || std::is_same_v<NodeType, GenericInstantiationExpression>)
      {
        if constexpr (std::is_same_v<NodeType, CallExpression>)
        {
          Add("Callee", Value.Callee);
        }
        else
        {
          Add("Target", Value.Target);
        }
        for (std::size_t Index = 0; Index < Value.Arguments.size(); ++Index)
        {
          Add("Arguments", Value.Arguments[Index].Expression, Index);
        }
      }
      else if constexpr (std::is_same_v<NodeType, IndexExpression>)
      {
        Add("Target", Value.Target);
        Add("Index", Value.Index);
      }
      else if constexpr (std::is_same_v<NodeType, MemberExpression>)
      {
        Add("Target", Value.Target);
      }
      else
      {
        static_assert(std::is_same_v<NodeType, ImportDeclaration> || std::is_same_v<NodeType, NamePattern> || std::is_same_v<NodeType, WildcardPattern> || std::is_same_v<NodeType, BreakStatement> || std::is_same_v<NodeType, ContinueStatement> || std::is_same_v<NodeType, LiteralExpression> || std::is_same_v<NodeType, NameExpression> || std::is_same_v<NodeType, BuiltinTypeExpression> || std::is_same_v<NodeType, ReceiverExpression>, "New AST nodes must explicitly define their child traversal");
      }
    };
    visitAstNode(Node, Visit);
    return Result;
  }

  void AstNodeDeleter::operator()(AstNode *Node) const noexcept
  {
    const auto Destroy = [](const auto &Concrete)
    {
      delete &Concrete;
    };
    visitAstNode(*Node, Destroy);
  }

  AstTree::AstTree(AstTree &&Other) noexcept
      : Nodes(std::move(Other.Nodes)),
        Root(std::exchange(Other.Root, InvalidAstNodeId))
  {
  }

  AstTree &AstTree::operator=(AstTree &&Other) noexcept
  {
    if (this != &Other)
    {
      Nodes = std::move(Other.Nodes);
      Root = std::exchange(Other.Root, InvalidAstNodeId);
    }
    return *this;
  }

  AstNodeId AstTree::addNode(NodeOwner Node, core::SourceRange Span, AstNodeFlags Flags)
  {
    assert(Span.Start <= Span.End);
    Node->Span = Span;
    Node->Flags = Flags;
    if (Node->as<Error>() != nullptr)
    {
      Node->Flags |= AstNodeFlags::HasError;
    }
    for (const AstChildEdge &Child : astChildren(*Node))
    {
      assert(Child.Id < Nodes.size());
      const AstNode &ChildNode = node(Child.Id);
      Node->Flags |= ChildNode.Flags;
      Node->Span.Start = std::min(Node->Span.Start, ChildNode.Span.Start);
      Node->Span.End = std::max(Node->Span.End, ChildNode.Span.End);
    }
    const AstNodeId Id = Nodes.size();
    Nodes.push_back(std::move(Node));
    return Id;
  }
} // namespace ink::parser
