#ifndef INK_PARSER_AST_H
#define INK_PARSER_AST_H

#include "ink/core/source_range.h"
#include "ink/tokenizer/token.h"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::parser
{
  using AstNodeId = std::size_t;
  // Absolute token indices in ParsedFile::lexedFile(), including any retained
  // trivia. Token storage and source ownership stay with the parsed file.
  using AstTokenId = std::size_t;
  inline constexpr AstNodeId InvalidAstNodeId = std::numeric_limits<AstNodeId>::max();
  inline constexpr AstTokenId InvalidAstTokenId = std::numeric_limits<AstTokenId>::max();

  enum class AstNodeFlags : std::uint8_t
  {
    None = 0,
    HasError = 1U << 0U,
    HasMissing = 1U << 1U,
  };

  constexpr AstNodeFlags operator|(AstNodeFlags Left, AstNodeFlags Right) noexcept
  {
    return static_cast<AstNodeFlags>(static_cast<std::uint8_t>(Left) | static_cast<std::uint8_t>(Right));
  }

  constexpr AstNodeFlags &operator|=(AstNodeFlags &Left, AstNodeFlags Right) noexcept
  {
    Left = Left | Right;
    return Left;
  }

  constexpr bool hasFlag(AstNodeFlags Value, AstNodeFlags Flag) noexcept
  {
    return (static_cast<std::uint8_t>(Value) & static_cast<std::uint8_t>(Flag)) != 0;
  }

  enum class AccessKind
  {
    Unspecified,
    Public,
    Private,
  };

  enum class BindingKind
  {
    Let,
    Var,
    Const,
  };

  struct AstArgument
  {
      AstNodeId Expression = InvalidAstNodeId;
      bool IsPackExpansion = false;
      core::SourceRange Span;
  };

  struct FunctionTypeParameter
  {
      AstNodeId TypeExpression = InvalidAstNodeId;
      bool IsVariadic = false;
      core::SourceRange Span;
  };

  enum class AstKind
  {
#define INK_AST_KIND(Name, Base) Name,
#include "ink/parser/ast_kind.def"
#undef INK_AST_KIND
  };

  const char *astKindName(AstKind Kind) noexcept;

  // Nodes use a kind tag for checked casts, as in Clang. Only the tree owns
  // nodes; child IDs do not recursively own their descendants.
  class AstNode
  {
    public:
      core::SourceRange Span;
      AstNodeFlags Flags = AstNodeFlags::None;

      AstKind kind() const noexcept
      {
        return Kind;
      }

      static bool classof(const AstNode *) noexcept
      {
        return true;
      }

      template <typename NodeType>
      const NodeType *as() const noexcept
      {
        static_assert(std::is_base_of_v<AstNode, NodeType>, "Expected an AST node type");
        return NodeType::classof(this) ? static_cast<const NodeType *>(this) : nullptr;
      }

      template <typename NodeType>
      const NodeType &get() const noexcept
      {
        const NodeType *Value = as<NodeType>();
        assert(Value != nullptr);
        return *Value;
      }

    protected:
      explicit AstNode(AstKind Kind) noexcept
          : Kind(Kind)
      {
      }

      AstNode(const AstNode &) = default;
      AstNode(AstNode &&) = default;
      ~AstNode() = default;

    private:
      const AstKind Kind;
  };

  class AstDeclaration : public AstNode
  {
    public:
      static bool classof(const AstNode *Node) noexcept;

    protected:
      explicit AstDeclaration(AstKind Kind) noexcept
          : AstNode(Kind)
      {
      }

      AstDeclaration(const AstDeclaration &) = default;
      AstDeclaration(AstDeclaration &&) = default;
      ~AstDeclaration() = default;
  };

  class AstStatement : public AstNode
  {
    public:
      static bool classof(const AstNode *Node) noexcept;

    protected:
      explicit AstStatement(AstKind Kind) noexcept
          : AstNode(Kind)
      {
      }

      AstStatement(const AstStatement &) = default;
      AstStatement(AstStatement &&) = default;
      ~AstStatement() = default;
  };

  class AstExpression : public AstNode
  {
    public:
      static bool classof(const AstNode *Node) noexcept;

    protected:
      explicit AstExpression(AstKind Kind) noexcept
          : AstNode(Kind)
      {
      }

      AstExpression(const AstExpression &) = default;
      AstExpression(AstExpression &&) = default;
      ~AstExpression() = default;
  };

  class AstPattern : public AstNode
  {
    public:
      static bool classof(const AstNode *Node) noexcept;

    protected:
      explicit AstPattern(AstKind Kind) noexcept
          : AstNode(Kind)
      {
      }

      AstPattern(const AstPattern &) = default;
      AstPattern(AstPattern &&) = default;
      ~AstPattern() = default;
  };

  class SourceFile final : public AstNode
  {
    public:
      explicit SourceFile(std::vector<AstNodeId> Statements = {})
          : AstNode(AstKind::SourceFile),
            Statements(std::move(Statements))
      {
      }

      std::vector<AstNodeId> Statements;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::SourceFile;
      }
  };

  class Error final : public AstNode
  {
    public:
      explicit Error(std::string Expected = {}, std::vector<AstNodeId> Recovered = {})
          : AstNode(AstKind::Error),
            Expected(std::move(Expected)),
            Recovered(std::move(Recovered))
      {
      }

      std::string Expected;
      std::vector<AstNodeId> Recovered;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::Error;
      }
  };

  class ImportDeclaration final : public AstDeclaration
  {
    public:
      explicit ImportDeclaration(std::vector<AstTokenId> Package = {}, AstTokenId Member = InvalidAstTokenId, AstTokenId Alias = InvalidAstTokenId, bool IsMemberImport = false)
          : AstDeclaration(AstKind::ImportDeclaration),
            Package(std::move(Package)),
            Member(Member),
            Alias(Alias),
            IsMemberImport(IsMemberImport)
      {
      }

      std::vector<AstTokenId> Package;
      AstTokenId Member;
      AstTokenId Alias;
      bool IsMemberImport;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ImportDeclaration;
      }
  };

  class FunctionDeclaration final : public AstDeclaration
  {
    public:
      explicit FunctionDeclaration(AccessKind Access = AccessKind::Unspecified, AstTokenId Linkage = InvalidAstTokenId, bool IsConst = false, bool IsClassMethod = false, AstTokenId Name = InvalidAstTokenId, std::vector<AstNodeId> GenericParameters = {}, std::vector<AstNodeId> Parameters = {}, AstNodeId ReturnType = InvalidAstNodeId, AstNodeId Body = InvalidAstNodeId)
          : AstDeclaration(AstKind::FunctionDeclaration),
            Access(Access),
            Linkage(Linkage),
            IsConst(IsConst),
            IsClassMethod(IsClassMethod),
            Name(Name),
            GenericParameters(std::move(GenericParameters)),
            Parameters(std::move(Parameters)),
            ReturnType(ReturnType),
            Body(Body)
      {
      }

      AccessKind Access;
      AstTokenId Linkage;
      bool IsConst;
      bool IsClassMethod;
      AstTokenId Name;
      std::vector<AstNodeId> GenericParameters;
      std::vector<AstNodeId> Parameters;
      AstNodeId ReturnType;
      AstNodeId Body;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::FunctionDeclaration;
      }
  };

  class ClassDeclaration final : public AstDeclaration
  {
    public:
      explicit ClassDeclaration(AccessKind Access = AccessKind::Unspecified, AstTokenId Name = InvalidAstTokenId, std::vector<AstNodeId> GenericParameters = {}, AstNodeId BaseType = InvalidAstNodeId, std::vector<AstNodeId> Interfaces = {}, AstNodeId Body = InvalidAstNodeId)
          : AstDeclaration(AstKind::ClassDeclaration),
            Access(Access),
            Name(Name),
            GenericParameters(std::move(GenericParameters)),
            BaseType(BaseType),
            Interfaces(std::move(Interfaces)),
            Body(Body)
      {
      }

      AccessKind Access;
      AstTokenId Name;
      std::vector<AstNodeId> GenericParameters;
      AstNodeId BaseType;
      std::vector<AstNodeId> Interfaces;
      AstNodeId Body;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ClassDeclaration;
      }
  };

  class InterfaceDeclaration final : public AstDeclaration
  {
    public:
      explicit InterfaceDeclaration(AccessKind Access = AccessKind::Unspecified, AstTokenId Name = InvalidAstTokenId, std::vector<AstNodeId> GenericParameters = {}, std::vector<AstNodeId> BaseTypes = {}, AstNodeId Body = InvalidAstNodeId)
          : AstDeclaration(AstKind::InterfaceDeclaration),
            Access(Access),
            Name(Name),
            GenericParameters(std::move(GenericParameters)),
            BaseTypes(std::move(BaseTypes)),
            Body(Body)
      {
      }

      AccessKind Access;
      AstTokenId Name;
      std::vector<AstNodeId> GenericParameters;
      std::vector<AstNodeId> BaseTypes;
      AstNodeId Body;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::InterfaceDeclaration;
      }
  };

  class EnumDeclaration final : public AstDeclaration
  {
    public:
      explicit EnumDeclaration(AccessKind Access = AccessKind::Unspecified, AstTokenId Name = InvalidAstTokenId, std::vector<AstNodeId> GenericParameters = {}, AstNodeId BaseType = InvalidAstNodeId, std::vector<AstNodeId> Interfaces = {}, AstNodeId Body = InvalidAstNodeId)
          : AstDeclaration(AstKind::EnumDeclaration),
            Access(Access),
            Name(Name),
            GenericParameters(std::move(GenericParameters)),
            BaseType(BaseType),
            Interfaces(std::move(Interfaces)),
            Body(Body)
      {
      }

      AccessKind Access;
      AstTokenId Name;
      std::vector<AstNodeId> GenericParameters;
      AstNodeId BaseType;
      std::vector<AstNodeId> Interfaces;
      AstNodeId Body;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::EnumDeclaration;
      }
  };

  class ClassFieldDeclaration final : public AstDeclaration
  {
    public:
      explicit ClassFieldDeclaration(AccessKind Access = AccessKind::Unspecified, bool IsConst = false, AstTokenId Name = InvalidAstTokenId, AstNodeId TypeExpression = InvalidAstNodeId, AstNodeId Initializer = InvalidAstNodeId)
          : AstDeclaration(AstKind::ClassFieldDeclaration),
            Access(Access),
            IsConst(IsConst),
            Name(Name),
            TypeExpression(TypeExpression),
            Initializer(Initializer)
      {
      }

      AccessKind Access;
      bool IsConst;
      AstTokenId Name;
      AstNodeId TypeExpression;
      AstNodeId Initializer;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ClassFieldDeclaration;
      }
  };

  class EnumFieldDeclaration final : public AstDeclaration
  {
    public:
      explicit EnumFieldDeclaration(AstTokenId Name = InvalidAstTokenId, AstNodeId Initializer = InvalidAstNodeId)
          : AstDeclaration(AstKind::EnumFieldDeclaration),
            Name(Name),
            Initializer(Initializer)
      {
      }

      AstTokenId Name;
      AstNodeId Initializer;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::EnumFieldDeclaration;
      }
  };

  class BindingDeclaration final : public AstDeclaration
  {
    public:
      explicit BindingDeclaration(AccessKind Access = AccessKind::Unspecified, BindingKind Binding = BindingKind::Let, AstNodeId Pattern = InvalidAstNodeId, AstNodeId TypeExpression = InvalidAstNodeId, AstNodeId Initializer = InvalidAstNodeId)
          : AstDeclaration(AstKind::BindingDeclaration),
            Access(Access),
            Binding(Binding),
            Pattern(Pattern),
            TypeExpression(TypeExpression),
            Initializer(Initializer)
      {
      }

      AccessKind Access;
      BindingKind Binding;
      AstNodeId Pattern;
      // Type annotations remain expressions; compile-time evaluation and the
      // requirement to produce a type value belong to semantic analysis.
      AstNodeId TypeExpression;
      AstNodeId Initializer;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::BindingDeclaration;
      }
  };

  class FunctionParameter final : public AstNode
  {
    public:
      explicit FunctionParameter(AstNodeId Pattern = InvalidAstNodeId, AstNodeId TypeExpression = InvalidAstNodeId, AstNodeId DefaultValue = InvalidAstNodeId, bool IsVariadic = false)
          : AstNode(AstKind::FunctionParameter),
            Pattern(Pattern),
            TypeExpression(TypeExpression),
            DefaultValue(DefaultValue),
            IsVariadic(IsVariadic)
      {
      }

      AstNodeId Pattern;
      AstNodeId TypeExpression;
      AstNodeId DefaultValue;
      bool IsVariadic;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::FunctionParameter;
      }
  };

  class GenericParameter final : public AstNode
  {
    public:
      explicit GenericParameter(AstTokenId Name = InvalidAstTokenId, AstNodeId TypeExpression = InvalidAstNodeId, AstNodeId DefaultValue = InvalidAstNodeId, bool IsVariadic = false)
          : AstNode(AstKind::GenericParameter),
            Name(Name),
            TypeExpression(TypeExpression),
            DefaultValue(DefaultValue),
            IsVariadic(IsVariadic)
      {
      }

      AstTokenId Name;
      AstNodeId TypeExpression;
      AstNodeId DefaultValue;
      bool IsVariadic;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::GenericParameter;
      }
  };

  class NamePattern final : public AstPattern
  {
    public:
      explicit NamePattern(AstTokenId Name = InvalidAstTokenId)
          : AstPattern(AstKind::NamePattern),
            Name(Name)
      {
      }

      AstTokenId Name;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::NamePattern;
      }
  };

  class WildcardPattern final : public AstPattern
  {
    public:
      WildcardPattern()
          : AstPattern(AstKind::WildcardPattern)
      {
      }

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::WildcardPattern;
      }
  };

  class TuplePattern final : public AstPattern
  {
    public:
      explicit TuplePattern(std::vector<AstNodeId> Elements = {})
          : AstPattern(AstKind::TuplePattern),
            Elements(std::move(Elements))
      {
      }

      std::vector<AstNodeId> Elements;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::TuplePattern;
      }
  };

  class BlockStatement final : public AstStatement
  {
    public:
      explicit BlockStatement(std::vector<AstNodeId> Statements = {})
          : AstStatement(AstKind::BlockStatement),
            Statements(std::move(Statements))
      {
      }

      std::vector<AstNodeId> Statements;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::BlockStatement;
      }
  };

  class ExpressionStatement final : public AstStatement
  {
    public:
      explicit ExpressionStatement(AstNodeId Expression = InvalidAstNodeId)
          : AstStatement(AstKind::ExpressionStatement),
            Expression(Expression)
      {
      }

      AstNodeId Expression;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ExpressionStatement;
      }
  };

  class AssignmentStatement final : public AstStatement
  {
    public:
      explicit AssignmentStatement(tokenizer::SymbolKind Operator = tokenizer::SymbolKind::Assign, AstNodeId Target = InvalidAstNodeId, AstNodeId Value = InvalidAstNodeId)
          : AstStatement(AstKind::AssignmentStatement),
            Operator(Operator),
            Target(Target),
            Value(Value)
      {
      }

      tokenizer::SymbolKind Operator;
      AstNodeId Target;
      AstNodeId Value;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::AssignmentStatement;
      }
  };

  class IfStatement final : public AstStatement
  {
    public:
      explicit IfStatement(AstNodeId Condition = InvalidAstNodeId, AstNodeId Then = InvalidAstNodeId, AstNodeId Else = InvalidAstNodeId)
          : AstStatement(AstKind::IfStatement),
            Condition(Condition),
            Then(Then),
            Else(Else)
      {
      }

      AstNodeId Condition;
      AstNodeId Then;
      AstNodeId Else;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::IfStatement;
      }
  };

  class WhileStatement final : public AstStatement
  {
    public:
      explicit WhileStatement(AstNodeId Condition = InvalidAstNodeId, AstNodeId Body = InvalidAstNodeId)
          : AstStatement(AstKind::WhileStatement),
            Condition(Condition),
            Body(Body)
      {
      }

      AstNodeId Condition;
      AstNodeId Body;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::WhileStatement;
      }
  };

  class ForStatement final : public AstStatement
  {
    public:
      explicit ForStatement(std::vector<AstNodeId> Initializers = {}, AstNodeId Condition = InvalidAstNodeId, std::vector<AstNodeId> Updates = {}, AstNodeId Body = InvalidAstNodeId)
          : AstStatement(AstKind::ForStatement),
            Initializers(std::move(Initializers)),
            Condition(Condition),
            Updates(std::move(Updates)),
            Body(Body)
      {
      }

      std::vector<AstNodeId> Initializers;
      AstNodeId Condition;
      std::vector<AstNodeId> Updates;
      AstNodeId Body;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ForStatement;
      }
  };

  class ForInStatement final : public AstStatement
  {
    public:
      explicit ForInStatement(BindingKind Binding = BindingKind::Let, AstNodeId Pattern = InvalidAstNodeId, AstNodeId TypeExpression = InvalidAstNodeId, AstNodeId Iterable = InvalidAstNodeId, AstNodeId Body = InvalidAstNodeId)
          : AstStatement(AstKind::ForInStatement),
            Binding(Binding),
            Pattern(Pattern),
            TypeExpression(TypeExpression),
            Iterable(Iterable),
            Body(Body)
      {
      }

      BindingKind Binding;
      AstNodeId Pattern;
      AstNodeId TypeExpression;
      AstNodeId Iterable;
      AstNodeId Body;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ForInStatement;
      }
  };

  class BreakStatement final : public AstStatement
  {
    public:
      BreakStatement()
          : AstStatement(AstKind::BreakStatement)
      {
      }

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::BreakStatement;
      }
  };

  class ContinueStatement final : public AstStatement
  {
    public:
      ContinueStatement()
          : AstStatement(AstKind::ContinueStatement)
      {
      }

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ContinueStatement;
      }
  };

  class ReturnStatement final : public AstStatement
  {
    public:
      explicit ReturnStatement(AstNodeId Value = InvalidAstNodeId)
          : AstStatement(AstKind::ReturnStatement),
            Value(Value)
      {
      }

      AstNodeId Value;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ReturnStatement;
      }
  };

  class DeferStatement final : public AstStatement
  {
    public:
      explicit DeferStatement(AstNodeId Action = InvalidAstNodeId)
          : AstStatement(AstKind::DeferStatement),
            Action(Action)
      {
      }

      AstNodeId Action;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::DeferStatement;
      }
  };

  class ComptimeStatement final : public AstStatement
  {
    public:
      explicit ComptimeStatement(AstNodeId Statement = InvalidAstNodeId)
          : AstStatement(AstKind::ComptimeStatement),
            Statement(Statement)
      {
      }

      AstNodeId Statement;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ComptimeStatement;
      }
  };

  class ConditionalExpression final : public AstExpression
  {
    public:
      explicit ConditionalExpression(AstNodeId Condition = InvalidAstNodeId, AstNodeId Then = InvalidAstNodeId, AstNodeId Else = InvalidAstNodeId)
          : AstExpression(AstKind::ConditionalExpression),
            Condition(Condition),
            Then(Then),
            Else(Else)
      {
      }

      AstNodeId Condition;
      AstNodeId Then;
      AstNodeId Else;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ConditionalExpression;
      }
  };

  class BinaryExpression final : public AstExpression
  {
    public:
      explicit BinaryExpression(tokenizer::SymbolKind Operator = tokenizer::SymbolKind::Plus, AstNodeId Left = InvalidAstNodeId, AstNodeId Right = InvalidAstNodeId)
          : AstExpression(AstKind::BinaryExpression),
            Operator(Operator),
            Left(Left),
            Right(Right)
      {
      }

      tokenizer::SymbolKind Operator;
      AstNodeId Left;
      AstNodeId Right;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::BinaryExpression;
      }
  };

  class UnaryExpression final : public AstExpression
  {
    public:
      explicit UnaryExpression(tokenizer::SymbolKind Operator = tokenizer::SymbolKind::Plus, AstNodeId Operand = InvalidAstNodeId)
          : AstExpression(AstKind::UnaryExpression),
            Operator(Operator),
            Operand(Operand)
      {
      }

      tokenizer::SymbolKind Operator;
      AstNodeId Operand;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::UnaryExpression;
      }
  };

  class ComptimeExpression final : public AstExpression
  {
    public:
      explicit ComptimeExpression(AstNodeId Operand = InvalidAstNodeId)
          : AstExpression(AstKind::ComptimeExpression),
            Operand(Operand)
      {
      }

      AstNodeId Operand;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ComptimeExpression;
      }
  };

  class ReferenceTypeExpression final : public AstExpression
  {
    public:
      explicit ReferenceTypeExpression(bool IsConst = false, AstNodeId Operand = InvalidAstNodeId)
          : AstExpression(AstKind::ReferenceTypeExpression),
            IsConst(IsConst),
            Operand(Operand)
      {
      }

      bool IsConst;
      AstNodeId Operand;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ReferenceTypeExpression;
      }
  };

  class PointerTypeExpression final : public AstExpression
  {
    public:
      explicit PointerTypeExpression(bool IsConst = false, AstNodeId Operand = InvalidAstNodeId)
          : AstExpression(AstKind::PointerTypeExpression),
            IsConst(IsConst),
            Operand(Operand)
      {
      }

      bool IsConst;
      AstNodeId Operand;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::PointerTypeExpression;
      }
  };

  class FunctionTypeExpression final : public AstExpression
  {
    public:
      explicit FunctionTypeExpression(AstTokenId Linkage = InvalidAstTokenId, std::vector<FunctionTypeParameter> Parameters = {}, AstNodeId ReturnType = InvalidAstNodeId)
          : AstExpression(AstKind::FunctionTypeExpression),
            Linkage(Linkage),
            Parameters(std::move(Parameters)),
            ReturnType(ReturnType)
      {
      }

      AstTokenId Linkage;
      std::vector<FunctionTypeParameter> Parameters;
      AstNodeId ReturnType;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::FunctionTypeExpression;
      }
  };

  class CallExpression final : public AstExpression
  {
    public:
      explicit CallExpression(AstNodeId Callee = InvalidAstNodeId, std::vector<AstArgument> Arguments = {})
          : AstExpression(AstKind::CallExpression),
            Callee(Callee),
            Arguments(std::move(Arguments))
      {
      }

      // The semantic stage decides whether the callee invokes a function or
      // constructs an object. Both have the same syntax and argument layout.
      AstNodeId Callee;
      std::vector<AstArgument> Arguments;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::CallExpression;
      }
  };

  class GenericInstantiationExpression final : public AstExpression
  {
    public:
      explicit GenericInstantiationExpression(AstNodeId Target = InvalidAstNodeId, std::vector<AstArgument> Arguments = {})
          : AstExpression(AstKind::GenericInstantiationExpression),
            Target(Target),
            Arguments(std::move(Arguments))
      {
      }

      AstNodeId Target;
      std::vector<AstArgument> Arguments;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::GenericInstantiationExpression;
      }
  };

  class IndexExpression final : public AstExpression
  {
    public:
      explicit IndexExpression(AstNodeId Target = InvalidAstNodeId, AstNodeId Index = InvalidAstNodeId)
          : AstExpression(AstKind::IndexExpression),
            Target(Target),
            Index(Index)
      {
      }

      AstNodeId Target;
      AstNodeId Index;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::IndexExpression;
      }
  };

  class MemberExpression final : public AstExpression
  {
    public:
      explicit MemberExpression(AstNodeId Target = InvalidAstNodeId, AstTokenId Member = InvalidAstTokenId, bool IsPointer = false)
          : AstExpression(AstKind::MemberExpression),
            Target(Target),
            Member(Member),
            IsPointer(IsPointer)
      {
      }

      AstNodeId Target;
      AstTokenId Member;
      bool IsPointer;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::MemberExpression;
      }
  };

  class LiteralExpression final : public AstExpression
  {
    public:
      explicit LiteralExpression(AstTokenId Token = InvalidAstTokenId)
          : AstExpression(AstKind::LiteralExpression),
            Token(Token)
      {
      }

      AstTokenId Token;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::LiteralExpression;
      }
  };

  class NameExpression final : public AstExpression
  {
    public:
      explicit NameExpression(AstTokenId Name = InvalidAstTokenId)
          : AstExpression(AstKind::NameExpression),
            Name(Name)
      {
      }

      AstTokenId Name;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::NameExpression;
      }
  };

  class BuiltinTypeExpression final : public AstExpression
  {
    public:
      explicit BuiltinTypeExpression(tokenizer::KeywordKind Type = tokenizer::KeywordKind::Type)
          : AstExpression(AstKind::BuiltinTypeExpression),
            Type(Type)
      {
      }

      tokenizer::KeywordKind Type;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::BuiltinTypeExpression;
      }
  };

  class ReceiverExpression final : public AstExpression
  {
    public:
      explicit ReceiverExpression(tokenizer::KeywordKind Receiver = tokenizer::KeywordKind::Self)
          : AstExpression(AstKind::ReceiverExpression),
            Receiver(Receiver)
      {
      }

      tokenizer::KeywordKind Receiver;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ReceiverExpression;
      }
  };

  class ParenthesizedExpression final : public AstExpression
  {
    public:
      explicit ParenthesizedExpression(AstNodeId Expression = InvalidAstNodeId)
          : AstExpression(AstKind::ParenthesizedExpression),
            Expression(Expression)
      {
      }

      AstNodeId Expression;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ParenthesizedExpression;
      }
  };

  class TupleExpression final : public AstExpression
  {
    public:
      explicit TupleExpression(std::vector<AstNodeId> Elements = {})
          : AstExpression(AstKind::TupleExpression),
            Elements(std::move(Elements))
      {
      }

      std::vector<AstNodeId> Elements;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::TupleExpression;
      }
  };

  class ArrayExpression final : public AstExpression
  {
    public:
      explicit ArrayExpression(std::vector<AstNodeId> Elements = {})
          : AstExpression(AstKind::ArrayExpression),
            Elements(std::move(Elements))
      {
      }

      std::vector<AstNodeId> Elements;

      static bool classof(const AstNode *Node) noexcept
      {
        return Node->kind() == AstKind::ArrayExpression;
      }
  };

  // Exhaustive dispatch over concrete subclasses; adding a node updates all
  // visitors at compile time without requiring C++ RTTI or virtual functions.
  template <typename Visitor>
  decltype(auto) visitAstNode(const AstNode &Node, Visitor &&Visit)
  {
    switch (Node.kind())
    {
#define INK_AST_KIND(Name, Base) \
  case AstKind::Name:            \
    return std::forward<Visitor>(Visit)(Node.get<Name>());
#include "ink/parser/ast_kind.def"
#undef INK_AST_KIND
    }
    std::abort();
  }

  struct AstChildEdge
  {
      const char *Role;
      AstNodeId Id;
      std::size_t Index = InvalidAstNodeId;
  };

  // This is a derived traversal view. Concrete nodes own their named fields;
  // neither punctuation tokens nor grammar-only list wrappers are tree nodes.
  std::vector<AstChildEdge> astChildren(const AstNode &Node);

  struct AstNodeDeleter
  {
      void operator()(AstNode *Node) const noexcept;
  };

  class AstTree
  {
    public:
      AstTree() = default;
      AstTree(const AstTree &) = delete;
      AstTree &operator=(const AstTree &) = delete;
      AstTree(AstTree &&Other) noexcept;
      AstTree &operator=(AstTree &&Other) noexcept;

      std::size_t size() const noexcept
      {
        return Nodes.size();
      }

      bool empty() const noexcept
      {
        return Nodes.empty();
      }

      // An empty result (for example after lexical failure) has an invalid root.
      AstNodeId root() const noexcept
      {
        return Root;
      }

      const AstNode &node(AstNodeId Id) const noexcept
      {
        assert(Id < Nodes.size());
        return *Nodes[Id];
      }

      std::vector<AstChildEdge> children(AstNodeId Id) const
      {
        return astChildren(node(Id));
      }

    private:
      using NodeOwner = std::unique_ptr<AstNode, AstNodeDeleter>;

      template <typename NodeType>
      AstNodeId addNode(NodeType Value, core::SourceRange Span, AstNodeFlags Flags = AstNodeFlags::None)
      {
        static_assert(std::is_base_of_v<AstNode, NodeType> && std::is_final_v<NodeType>, "Only concrete AST nodes can be stored");
        return addNode(NodeOwner(new NodeType(std::move(Value))), Span, Flags);
      }

      AstNodeId addNode(NodeOwner Node, core::SourceRange Span, AstNodeFlags Flags);

      void setRoot(AstNodeId Id) noexcept
      {
        assert(Id < Nodes.size());
        Root = Id;
      }

      // Individual allocations preserve node addresses when the index grows.
      // Typed deletion releases vector/string fields without following child IDs.
      std::vector<NodeOwner> Nodes;
      AstNodeId Root = InvalidAstNodeId;

      friend class ParserImpl;
  };
} // namespace ink::parser

#endif
