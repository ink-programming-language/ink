#pragma once
#include "ink/parser/ast_context.h"
#include "ink/parser/ast_kind.h"
#include "ink/core/source_range.h"
#include "ink/tokenizer/token.h"
#include <optional>
#include <string_view>
#include <type_traits>
namespace ink::parser
{
  using core::SourceRange;
  using tokenizer::TokenKind;
  using TokenId = std::size_t;
  inline constexpr TokenId InvalidTokenId = static_cast<TokenId>(-1);
  struct NameToken
  {
      TokenId Id = InvalidTokenId;
      std::string_view Text;
      SourceRange Range;
  };
  struct RestBinding
  {
      NameToken Name;
      bool Wildcard;
      SourceRange EllipsisRange;
  };
  enum class ArgumentKind
  {
    Named,
    Positional,
    SpreadPositional
  };
  enum class AttributeSuffix
  {
    None,
    Arguments,
    Value
  };
  enum class VarDeclForm
  {
    Uninitialized,
    Initialized
  };
  enum class FieldTailKind
  {
    None,
    Typed,
    InitializerOnly,
    Payload
  };
  enum class FunctionBodyKind
  {
    DeclarationOnly,
    Definition
  };
  enum class AggregateForm
  {
    Forward,
    Definition
  };
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
      SourceRange Range;
  };
  class Expr : public ASTNodeBase
  {
    public:
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && categoryOf(Node->getKind()) == ASTCategory::Expr;
      }

    protected:
      using ASTNodeBase::ASTNodeBase;
  };
  class Stmt : public ASTNodeBase
  {
    public:
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && categoryOf(Node->getKind()) == ASTCategory::Stmt;
      }

    protected:
      using ASTNodeBase::ASTNodeBase;
  };
  class Decl : public ASTNodeBase
  {
    public:
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && categoryOf(Node->getKind()) == ASTCategory::Decl;
      }

    protected:
      using ASTNodeBase::ASTNodeBase;
  };
  class SimpleItem : public ASTNodeBase
  {
    public:
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && categoryOf(Node->getKind()) == ASTCategory::SimpleItem;
      }

    protected:
      using ASTNodeBase::ASTNodeBase;
  };
  class BindingPattern : public ASTNodeBase
  {
    public:
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && categoryOf(Node->getKind()) == ASTCategory::BindingPattern;
      }

    protected:
      using ASTNodeBase::ASTNodeBase;
  };
  class MatchPattern : public ASTNodeBase
  {
    public:
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && categoryOf(Node->getKind()) == ASTCategory::MatchPattern;
      }

    protected:
      using ASTNodeBase::ASTNodeBase;
  };
  class ModuleAST;
  class TypeSyntax;
  class MissingExpr;
  class ErrorExpr;
  class MissingStmt;
  class ErrorStmt;
  class MissingDecl;
  class ErrorDecl;
  class MissingBindingPattern;
  class ErrorBindingPattern;
  class MissingMatchPattern;
  class ErrorMatchPattern;
  class NameExpr;
  class LiteralExpr;
  class UnaryExpr;
  class ComptimeExpr;
  class BinaryExpr;
  class ConditionalExpr;
  class ParenExpr;
  class TupleExpr;
  class ArrayExpr;
  class ArrayRepeatExpr;
  class CallExpr;
  class IndexExpr;
  class MemberExpr;
  class GenericApplyExpr;
  class PostfixUpdateExpr;
  class FunctionTypeExpr;
  class LambdaExpr;
  class MatchExpr;
  class BlockExpr;
  class ExprItem;
  class AssignmentItem;
  class SimpleStmt;
  class BlockStmt;
  class DeclStmt;
  class IfStmt;
  class WhileStmt;
  class ClassicForStmt;
  class ForInStmt;
  class SwitchStmt;
  class ReturnStmt;
  class BreakStmt;
  class ContinueStmt;
  class YieldStmt;
  class DeferStmt;
  class ComptimeStmt;
  class DirectImportStmt;
  class FromImportStmt;
  class VarDecl;
  class FieldDecl;
  class FunctionDecl;
  class ClassDecl;
  class EnumDecl;
  class InterfaceDecl;
  class WildcardBindingPattern;
  class NameBindingPattern;
  class TupleBindingPattern;
  class ArrayBindingPattern;
  class WildcardMatchPattern;
  class NameMatchPattern;
  class TupleMatchPattern;
  class ArrayMatchPattern;
  class LiteralMatchPattern;
  class GroupedMatchPattern;
  class OrMatchPattern;
  class Parameter;
  class FunctionTypeParameter;
  class Argument;
  class PathSegment;
  class Attribute;
  class BaseSpec;
  class MatchArm;
  class SwitchClause;
  class ImportEntry;
  class Parameter
  {
    public:
      Parameter(NameToken Name, TypeSyntax *Type, Expr *DefaultValue, bool Variadic, SourceRange Range)
          : Name(Name),
            Type(Type),
            DefaultValue(DefaultValue),
            Variadic(Variadic),
            Range(Range)
      {
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
      }
      Expr *defaultValue() noexcept
      {
        return DefaultValue;
      }
      const Expr *defaultValue() const noexcept
      {
        return DefaultValue;
      }
      bool variadic() const noexcept
      {
        return Variadic;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      NameToken Name;
      TypeSyntax *Type;
      Expr *DefaultValue;
      bool Variadic;
      SourceRange Range;
  };
  class FunctionTypeParameter
  {
    public:
      FunctionTypeParameter(std::optional<NameToken> Name, TypeSyntax *Type, bool Variadic, SourceRange Range)
          : Name(Name),
            Type(Type),
            Variadic(Variadic),
            Range(Range)
      {
      }
      std::optional<NameToken> name() const noexcept
      {
        return Name;
      }
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
      }
      bool variadic() const noexcept
      {
        return Variadic;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      std::optional<NameToken> Name;
      TypeSyntax *Type;
      bool Variadic;
      SourceRange Range;
  };
  class Argument
  {
    public:
      Argument(ArgumentKind Form, std::optional<NameToken> Name, Expr *Value, SourceRange Range)
          : Form(Form),
            Name(Name),
            Value(Value),
            Range(Range)
      {
      }
      ArgumentKind form() const noexcept
      {
        return Form;
      }
      std::optional<NameToken> name() const noexcept
      {
        return Name;
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      ArgumentKind Form;
      std::optional<NameToken> Name;
      Expr *Value;
      SourceRange Range;
  };
  class PathSegment
  {
    public:
      PathSegment(NameToken Name, bool HasGenericArguments, ASTArray<Argument> Arguments, SourceRange Range)
          : Name(Name),
            HasGenericArguments(HasGenericArguments),
            Arguments(Arguments),
            Range(Range)
      {
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      bool hasGenericArguments() const noexcept
      {
        return HasGenericArguments;
      }
      ASTArray<Argument> arguments() const noexcept
      {
        return Arguments;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      NameToken Name;
      bool HasGenericArguments;
      ASTArray<Argument> Arguments;
      SourceRange Range;
  };
  class Attribute
  {
    public:
      Attribute(ASTArray<NameToken> Path, AttributeSuffix Suffix, ASTArray<Argument> Arguments, Expr *Value, SourceRange Range)
          : Path(Path),
            Suffix(Suffix),
            Arguments(Arguments),
            Value(Value),
            Range(Range)
      {
      }
      ASTArray<NameToken> path() const noexcept
      {
        return Path;
      }
      AttributeSuffix suffix() const noexcept
      {
        return Suffix;
      }
      ASTArray<Argument> arguments() const noexcept
      {
        return Arguments;
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      ASTArray<NameToken> Path;
      AttributeSuffix Suffix;
      ASTArray<Argument> Arguments;
      Expr *Value;
      SourceRange Range;
  };
  class BaseSpec
  {
    public:
      BaseSpec(bool Implements, TypeSyntax *Type, SourceRange Range)
          : Implements(Implements),
            Type(Type),
            Range(Range)
      {
      }
      bool implements() const noexcept
      {
        return Implements;
      }
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      bool Implements;
      TypeSyntax *Type;
      SourceRange Range;
  };
  class MatchArm
  {
    public:
      MatchArm(MatchPattern *Pattern, Expr *Guard, Expr *Value, SourceRange Range)
          : Pattern(Pattern),
            Guard(Guard),
            Value(Value),
            Range(Range)
      {
      }
      MatchPattern *pattern() noexcept
      {
        return Pattern;
      }
      const MatchPattern *pattern() const noexcept
      {
        return Pattern;
      }
      Expr *guard() noexcept
      {
        return Guard;
      }
      const Expr *guard() const noexcept
      {
        return Guard;
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      MatchPattern *Pattern;
      Expr *Guard;
      Expr *Value;
      SourceRange Range;
  };
  class SwitchClause
  {
    public:
      SwitchClause(bool Default, Expr *Value, ASTArray<Stmt *> Statements, SourceRange Range)
          : Default(Default),
            Value(Value),
            Statements(Statements),
            Range(Range)
      {
      }
      bool isDefault() const noexcept
      {
        return Default;
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      ASTArray<Stmt *> statements() noexcept
      {
        return Statements;
      }
      ConstNodeArray<Stmt> statements() const noexcept
      {
        return ConstNodeArray<Stmt>(Statements);
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      bool Default;
      Expr *Value;
      ASTArray<Stmt *> Statements;
      SourceRange Range;
  };
  class ImportEntry
  {
    public:
      ImportEntry(ASTArray<NameToken> Path, std::optional<NameToken> Alias, SourceRange Range)
          : Path(Path),
            Alias(Alias),
            Range(Range)
      {
      }
      ASTArray<NameToken> path() const noexcept
      {
        return Path;
      }
      std::optional<NameToken> alias() const noexcept
      {
        return Alias;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      ASTArray<NameToken> Path;
      std::optional<NameToken> Alias;
      SourceRange Range;
  };
  class ModuleAST final : public ASTNodeBase
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ModuleAST;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ModuleAST(SourceRange Range, ASTArray<Stmt *> Statements)
          : ASTNodeBase(Kind, Range),
            Statements(Statements)
      {
      }
      ASTArray<Stmt *> statements() noexcept
      {
        return Statements;
      }
      ConstNodeArray<Stmt> statements() const noexcept
      {
        return ConstNodeArray<Stmt>(Statements);
      }

    private:
      ASTArray<Stmt *> Statements;
  };
  class TypeSyntax final : public ASTNodeBase
  {
    public:
      static constexpr ASTKind Kind = ASTKind::TypeSyntax;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      TypeSyntax(SourceRange Range, Expr *Expression)
          : ASTNodeBase(Kind, Range),
            Expression(Expression)
      {
      }
      Expr *expression() noexcept
      {
        return Expression;
      }
      const Expr *expression() const noexcept
      {
        return Expression;
      }

    private:
      Expr *Expression;
  };
  class MissingExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingExpr(SourceRange Range)
          : Expr(Kind, Range)
      {
      }

    private:
  };
  class ErrorExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ErrorExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ErrorExpr(SourceRange Range)
          : Expr(Kind, Range)
      {
      }

    private:
  };
  class MissingStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingStmt(SourceRange Range)
          : Stmt(Kind, Range)
      {
      }

    private:
  };
  class ErrorStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ErrorStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ErrorStmt(SourceRange Range)
          : Stmt(Kind, Range)
      {
      }

    private:
  };
  class MissingDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingDecl(SourceRange Range, ASTArray<Attribute> Attributes)
          : Decl(Kind, Range),
            Attributes(Attributes)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }

    private:
      ASTArray<Attribute> Attributes;
  };
  class ErrorDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ErrorDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ErrorDecl(SourceRange Range, ASTArray<Attribute> Attributes)
          : Decl(Kind, Range),
            Attributes(Attributes)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }

    private:
      ASTArray<Attribute> Attributes;
  };
  class MissingBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingBindingPattern(SourceRange Range)
          : BindingPattern(Kind, Range)
      {
      }

    private:
  };
  class ErrorBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ErrorBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ErrorBindingPattern(SourceRange Range)
          : BindingPattern(Kind, Range)
      {
      }

    private:
  };
  class MissingMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingMatchPattern(SourceRange Range)
          : MatchPattern(Kind, Range)
      {
      }

    private:
  };
  class ErrorMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ErrorMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ErrorMatchPattern(SourceRange Range)
          : MatchPattern(Kind, Range)
      {
      }

    private:
  };
  class NameExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::NameExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      NameExpr(SourceRange Range, NameToken Name)
          : Expr(Kind, Range),
            Name(Name)
      {
      }
      NameToken name() const noexcept
      {
        return Name;
      }

    private:
      NameToken Name;
  };
  class LiteralExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::LiteralExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      LiteralExpr(SourceRange Range, TokenId Token, TokenKind LiteralKind)
          : Expr(Kind, Range),
            Token(Token),
            LiteralKind(LiteralKind)
      {
      }
      TokenId token() const noexcept
      {
        return Token;
      }
      TokenKind literalKind() const noexcept
      {
        return LiteralKind;
      }

    private:
      TokenId Token;
      TokenKind LiteralKind;
  };
  class UnaryExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::UnaryExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      UnaryExpr(SourceRange Range, TokenKind Op, SourceRange OperatorRange, Expr *Operand)
          : Expr(Kind, Range),
            Op(Op),
            OperatorRange(OperatorRange),
            Operand(Operand)
      {
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }
      Expr *operand() noexcept
      {
        return Operand;
      }
      const Expr *operand() const noexcept
      {
        return Operand;
      }

    private:
      TokenKind Op;
      SourceRange OperatorRange;
      Expr *Operand;
  };
  class ComptimeExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ComptimeExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ComptimeExpr(SourceRange Range, Expr *Operand)
          : Expr(Kind, Range),
            Operand(Operand)
      {
      }
      Expr *operand() noexcept
      {
        return Operand;
      }
      const Expr *operand() const noexcept
      {
        return Operand;
      }

    private:
      Expr *Operand;
  };
  class BinaryExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::BinaryExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      BinaryExpr(SourceRange Range, Expr *Left, TokenKind Op, SourceRange OperatorRange, Expr *Right)
          : Expr(Kind, Range),
            Left(Left),
            Op(Op),
            OperatorRange(OperatorRange),
            Right(Right)
      {
      }
      Expr *left() noexcept
      {
        return Left;
      }
      const Expr *left() const noexcept
      {
        return Left;
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }
      Expr *right() noexcept
      {
        return Right;
      }
      const Expr *right() const noexcept
      {
        return Right;
      }

    private:
      Expr *Left;
      TokenKind Op;
      SourceRange OperatorRange;
      Expr *Right;
  };
  class ConditionalExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ConditionalExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ConditionalExpr(SourceRange Range, Expr *Condition, Expr *Then, Expr *Else)
          : Expr(Kind, Range),
            Condition(Condition),
            Then(Then),
            Else(Else)
      {
      }
      Expr *condition() noexcept
      {
        return Condition;
      }
      const Expr *condition() const noexcept
      {
        return Condition;
      }
      Expr *then() noexcept
      {
        return Then;
      }
      const Expr *then() const noexcept
      {
        return Then;
      }
      Expr *elseValue() noexcept
      {
        return Else;
      }
      const Expr *elseValue() const noexcept
      {
        return Else;
      }

    private:
      Expr *Condition;
      Expr *Then;
      Expr *Else;
  };
  class ParenExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ParenExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ParenExpr(SourceRange Range, Expr *Expression)
          : Expr(Kind, Range),
            Expression(Expression)
      {
      }
      Expr *expression() noexcept
      {
        return Expression;
      }
      const Expr *expression() const noexcept
      {
        return Expression;
      }

    private:
      Expr *Expression;
  };
  class TupleExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::TupleExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      TupleExpr(SourceRange Range, ASTArray<Expr *> Elements)
          : Expr(Kind, Range),
            Elements(Elements)
      {
      }
      ASTArray<Expr *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<Expr> elements() const noexcept
      {
        return ConstNodeArray<Expr>(Elements);
      }

    private:
      ASTArray<Expr *> Elements;
  };
  class ArrayExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ArrayExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ArrayExpr(SourceRange Range, ASTArray<Expr *> Elements)
          : Expr(Kind, Range),
            Elements(Elements)
      {
      }
      ASTArray<Expr *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<Expr> elements() const noexcept
      {
        return ConstNodeArray<Expr>(Elements);
      }

    private:
      ASTArray<Expr *> Elements;
  };
  class ArrayRepeatExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ArrayRepeatExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ArrayRepeatExpr(SourceRange Range, Expr *Value, Expr *Count)
          : Expr(Kind, Range),
            Value(Value),
            Count(Count)
      {
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      Expr *count() noexcept
      {
        return Count;
      }
      const Expr *count() const noexcept
      {
        return Count;
      }

    private:
      Expr *Value;
      Expr *Count;
  };
  class CallExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::CallExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      CallExpr(SourceRange Range, Expr *Callee, ASTArray<Argument> Arguments, bool Optional)
          : Expr(Kind, Range),
            Callee(Callee),
            Arguments(Arguments),
            Optional(Optional)
      {
      }
      Expr *callee() noexcept
      {
        return Callee;
      }
      const Expr *callee() const noexcept
      {
        return Callee;
      }
      ASTArray<Argument> arguments() const noexcept
      {
        return Arguments;
      }
      bool optional() const noexcept
      {
        return Optional;
      }

    private:
      Expr *Callee;
      ASTArray<Argument> Arguments;
      bool Optional;
  };
  class IndexExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::IndexExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      IndexExpr(SourceRange Range, Expr *Object, Expr *Index, bool Optional)
          : Expr(Kind, Range),
            Object(Object),
            Index(Index),
            Optional(Optional)
      {
      }
      Expr *object() noexcept
      {
        return Object;
      }
      const Expr *object() const noexcept
      {
        return Object;
      }
      Expr *index() noexcept
      {
        return Index;
      }
      const Expr *index() const noexcept
      {
        return Index;
      }
      bool optional() const noexcept
      {
        return Optional;
      }

    private:
      Expr *Object;
      Expr *Index;
      bool Optional;
  };
  class MemberExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MemberExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MemberExpr(SourceRange Range, Expr *Object, TokenKind Access, NameToken Member)
          : Expr(Kind, Range),
            Object(Object),
            Access(Access),
            Member(Member)
      {
      }
      Expr *object() noexcept
      {
        return Object;
      }
      const Expr *object() const noexcept
      {
        return Object;
      }
      TokenKind access() const noexcept
      {
        return Access;
      }
      NameToken member() const noexcept
      {
        return Member;
      }

    private:
      Expr *Object;
      TokenKind Access;
      NameToken Member;
  };
  class GenericApplyExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::GenericApplyExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      GenericApplyExpr(SourceRange Range, Expr *Object, ASTArray<Argument> Arguments)
          : Expr(Kind, Range),
            Object(Object),
            Arguments(Arguments)
      {
      }
      Expr *object() noexcept
      {
        return Object;
      }
      const Expr *object() const noexcept
      {
        return Object;
      }
      ASTArray<Argument> arguments() const noexcept
      {
        return Arguments;
      }

    private:
      Expr *Object;
      ASTArray<Argument> Arguments;
  };
  class PostfixUpdateExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::PostfixUpdateExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      PostfixUpdateExpr(SourceRange Range, Expr *Operand, TokenKind Op, SourceRange OperatorRange)
          : Expr(Kind, Range),
            Operand(Operand),
            Op(Op),
            OperatorRange(OperatorRange)
      {
      }
      Expr *operand() noexcept
      {
        return Operand;
      }
      const Expr *operand() const noexcept
      {
        return Operand;
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }

    private:
      Expr *Operand;
      TokenKind Op;
      SourceRange OperatorRange;
  };
  class FunctionTypeExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::FunctionTypeExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      FunctionTypeExpr(SourceRange Range, ASTArray<FunctionTypeParameter> Parameters, TypeSyntax *ReturnType)
          : Expr(Kind, Range),
            Parameters(Parameters),
            ReturnType(ReturnType)
      {
      }
      ASTArray<FunctionTypeParameter> parameters() const noexcept
      {
        return Parameters;
      }
      TypeSyntax *returnType() noexcept
      {
        return ReturnType;
      }
      const TypeSyntax *returnType() const noexcept
      {
        return ReturnType;
      }

    private:
      ASTArray<FunctionTypeParameter> Parameters;
      TypeSyntax *ReturnType;
  };
  class LambdaExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::LambdaExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      LambdaExpr(SourceRange Range, ASTArray<Parameter> GenericParameters, ASTArray<Parameter> Parameters, TypeSyntax *ReturnType, BlockStmt *Body)
          : Expr(Kind, Range),
            GenericParameters(GenericParameters),
            Parameters(Parameters),
            ReturnType(ReturnType),
            Body(Body)
      {
      }
      ASTArray<Parameter> genericParameters() const noexcept
      {
        return GenericParameters;
      }
      ASTArray<Parameter> parameters() const noexcept
      {
        return Parameters;
      }
      TypeSyntax *returnType() noexcept
      {
        return ReturnType;
      }
      const TypeSyntax *returnType() const noexcept
      {
        return ReturnType;
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }

    private:
      ASTArray<Parameter> GenericParameters;
      ASTArray<Parameter> Parameters;
      TypeSyntax *ReturnType;
      BlockStmt *Body;
  };
  class MatchExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MatchExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MatchExpr(SourceRange Range, Expr *Value, ASTArray<MatchArm> Arms)
          : Expr(Kind, Range),
            Value(Value),
            Arms(Arms)
      {
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      ASTArray<MatchArm> arms() const noexcept
      {
        return Arms;
      }

    private:
      Expr *Value;
      ASTArray<MatchArm> Arms;
  };
  class BlockExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::BlockExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      BlockExpr(SourceRange Range, BlockStmt *Body)
          : Expr(Kind, Range),
            Body(Body)
      {
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }

    private:
      BlockStmt *Body;
  };
  class ExprItem final : public SimpleItem
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ExprItem;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ExprItem(SourceRange Range, Expr *Expression)
          : SimpleItem(Kind, Range),
            Expression(Expression)
      {
      }
      Expr *expression() noexcept
      {
        return Expression;
      }
      const Expr *expression() const noexcept
      {
        return Expression;
      }

    private:
      Expr *Expression;
  };
  class AssignmentItem final : public SimpleItem
  {
    public:
      static constexpr ASTKind Kind = ASTKind::AssignmentItem;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      AssignmentItem(SourceRange Range, Expr *Left, TokenKind Op, SourceRange OperatorRange, SimpleItem *Right)
          : SimpleItem(Kind, Range),
            Left(Left),
            Op(Op),
            OperatorRange(OperatorRange),
            Right(Right)
      {
      }
      Expr *left() noexcept
      {
        return Left;
      }
      const Expr *left() const noexcept
      {
        return Left;
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }
      SimpleItem *right() noexcept
      {
        return Right;
      }
      const SimpleItem *right() const noexcept
      {
        return Right;
      }

    private:
      Expr *Left;
      TokenKind Op;
      SourceRange OperatorRange;
      SimpleItem *Right;
  };
  class SimpleStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::SimpleStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      SimpleStmt(SourceRange Range, ASTArray<SimpleItem *> Items)
          : Stmt(Kind, Range),
            Items(Items)
      {
      }
      ASTArray<SimpleItem *> items() noexcept
      {
        return Items;
      }
      ConstNodeArray<SimpleItem> items() const noexcept
      {
        return ConstNodeArray<SimpleItem>(Items);
      }

    private:
      ASTArray<SimpleItem *> Items;
  };
  class BlockStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::BlockStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      BlockStmt(SourceRange Range, ASTArray<Stmt *> Statements, bool Synthetic)
          : Stmt(Kind, Range),
            Statements(Statements),
            Synthetic(Synthetic)
      {
      }
      ASTArray<Stmt *> statements() noexcept
      {
        return Statements;
      }
      ConstNodeArray<Stmt> statements() const noexcept
      {
        return ConstNodeArray<Stmt>(Statements);
      }
      bool synthetic() const noexcept
      {
        return Synthetic;
      }

    private:
      ASTArray<Stmt *> Statements;
      bool Synthetic;
  };
  class DeclStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::DeclStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      DeclStmt(SourceRange Range, Decl *Declaration)
          : Stmt(Kind, Range),
            Declaration(Declaration)
      {
      }
      Decl *declaration() noexcept
      {
        return Declaration;
      }
      const Decl *declaration() const noexcept
      {
        return Declaration;
      }

    private:
      Decl *Declaration;
  };
  class IfStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::IfStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      IfStmt(SourceRange Range, Expr *Condition, Stmt *ThenBranch, Stmt *ElseBranch)
          : Stmt(Kind, Range),
            Condition(Condition),
            ThenBranch(ThenBranch),
            ElseBranch(ElseBranch)
      {
      }
      Expr *condition() noexcept
      {
        return Condition;
      }
      const Expr *condition() const noexcept
      {
        return Condition;
      }
      Stmt *thenBranch() noexcept
      {
        return ThenBranch;
      }
      const Stmt *thenBranch() const noexcept
      {
        return ThenBranch;
      }
      Stmt *elseBranch() noexcept
      {
        return ElseBranch;
      }
      const Stmt *elseBranch() const noexcept
      {
        return ElseBranch;
      }

    private:
      Expr *Condition;
      Stmt *ThenBranch;
      Stmt *ElseBranch;
  };
  class WhileStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::WhileStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      WhileStmt(SourceRange Range, Expr *Condition, Stmt *Body)
          : Stmt(Kind, Range),
            Condition(Condition),
            Body(Body)
      {
      }
      Expr *condition() noexcept
      {
        return Condition;
      }
      const Expr *condition() const noexcept
      {
        return Condition;
      }
      Stmt *body() noexcept
      {
        return Body;
      }
      const Stmt *body() const noexcept
      {
        return Body;
      }

    private:
      Expr *Condition;
      Stmt *Body;
  };
  class ClassicForStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ClassicForStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ClassicForStmt(SourceRange Range, Stmt *Initializer, Expr *Condition, ASTArray<SimpleItem *> Step, Stmt *Body)
          : Stmt(Kind, Range),
            Initializer(Initializer),
            Condition(Condition),
            Step(Step),
            Body(Body)
      {
      }
      Stmt *initializer() noexcept
      {
        return Initializer;
      }
      const Stmt *initializer() const noexcept
      {
        return Initializer;
      }
      Expr *condition() noexcept
      {
        return Condition;
      }
      const Expr *condition() const noexcept
      {
        return Condition;
      }
      ASTArray<SimpleItem *> step() noexcept
      {
        return Step;
      }
      ConstNodeArray<SimpleItem> step() const noexcept
      {
        return ConstNodeArray<SimpleItem>(Step);
      }
      Stmt *body() noexcept
      {
        return Body;
      }
      const Stmt *body() const noexcept
      {
        return Body;
      }

    private:
      Stmt *Initializer;
      Expr *Condition;
      ASTArray<SimpleItem *> Step;
      Stmt *Body;
  };
  class ForInStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ForInStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ForInStmt(SourceRange Range, BindingPattern *Binding, Expr *Iterable, Stmt *Body)
          : Stmt(Kind, Range),
            Binding(Binding),
            Iterable(Iterable),
            Body(Body)
      {
      }
      BindingPattern *binding() noexcept
      {
        return Binding;
      }
      const BindingPattern *binding() const noexcept
      {
        return Binding;
      }
      Expr *iterable() noexcept
      {
        return Iterable;
      }
      const Expr *iterable() const noexcept
      {
        return Iterable;
      }
      Stmt *body() noexcept
      {
        return Body;
      }
      const Stmt *body() const noexcept
      {
        return Body;
      }

    private:
      BindingPattern *Binding;
      Expr *Iterable;
      Stmt *Body;
  };
  class SwitchStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::SwitchStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      SwitchStmt(SourceRange Range, Expr *Value, ASTArray<SwitchClause> Clauses)
          : Stmt(Kind, Range),
            Value(Value),
            Clauses(Clauses)
      {
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      ASTArray<SwitchClause> clauses() const noexcept
      {
        return Clauses;
      }

    private:
      Expr *Value;
      ASTArray<SwitchClause> Clauses;
  };
  class ReturnStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ReturnStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ReturnStmt(SourceRange Range, Expr *Value)
          : Stmt(Kind, Range),
            Value(Value)
      {
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }

    private:
      Expr *Value;
  };
  class BreakStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::BreakStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      BreakStmt(SourceRange Range)
          : Stmt(Kind, Range)
      {
      }

    private:
  };
  class ContinueStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ContinueStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ContinueStmt(SourceRange Range)
          : Stmt(Kind, Range)
      {
      }

    private:
  };
  class YieldStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::YieldStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      YieldStmt(SourceRange Range, bool Return, Expr *Value)
          : Stmt(Kind, Range),
            Return(Return),
            Value(Value)
      {
      }
      bool isReturn() const noexcept
      {
        return Return;
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }

    private:
      bool Return;
      Expr *Value;
  };
  class DeferStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::DeferStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      DeferStmt(SourceRange Range, Stmt *Body)
          : Stmt(Kind, Range),
            Body(Body)
      {
      }
      Stmt *body() noexcept
      {
        return Body;
      }
      const Stmt *body() const noexcept
      {
        return Body;
      }

    private:
      Stmt *Body;
  };
  class ComptimeStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ComptimeStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ComptimeStmt(SourceRange Range, SourceRange KeywordRange, Stmt *Body)
          : Stmt(Kind, Range),
            KeywordRange(KeywordRange),
            Body(Body)
      {
      }
      SourceRange keywordRange() const noexcept
      {
        return KeywordRange;
      }
      Stmt *body() noexcept
      {
        return Body;
      }
      const Stmt *body() const noexcept
      {
        return Body;
      }

    private:
      SourceRange KeywordRange;
      Stmt *Body;
  };
  class DirectImportStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::DirectImportStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      DirectImportStmt(SourceRange Range, ASTArray<ImportEntry> Imports)
          : Stmt(Kind, Range),
            Imports(Imports)
      {
      }
      ASTArray<ImportEntry> imports() const noexcept
      {
        return Imports;
      }

    private:
      ASTArray<ImportEntry> Imports;
  };
  class FromImportStmt final : public Stmt
  {
    public:
      static constexpr ASTKind Kind = ASTKind::FromImportStmt;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      FromImportStmt(SourceRange Range, std::size_t RelativeLevel, ASTArray<TokenId> RelativeTokens, ASTArray<NameToken> Path, ASTArray<ImportEntry> Imports)
          : Stmt(Kind, Range),
            RelativeLevel(RelativeLevel),
            RelativeTokens(RelativeTokens),
            Path(Path),
            Imports(Imports)
      {
      }
      std::size_t relativeLevel() const noexcept
      {
        return RelativeLevel;
      }
      ASTArray<TokenId> relativeTokens() const noexcept
      {
        return RelativeTokens;
      }
      ASTArray<NameToken> path() const noexcept
      {
        return Path;
      }
      ASTArray<ImportEntry> imports() const noexcept
      {
        return Imports;
      }

    private:
      std::size_t RelativeLevel;
      ASTArray<TokenId> RelativeTokens;
      ASTArray<NameToken> Path;
      ASTArray<ImportEntry> Imports;
  };
  class VarDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::VarDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      VarDecl(SourceRange Range, ASTArray<Attribute> Attributes, bool Constant, BindingPattern *Binding, TypeSyntax *Type, Expr *Initializer, VarDeclForm Form)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Constant(Constant),
            Binding(Binding),
            Type(Type),
            Initializer(Initializer),
            Form(Form)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      bool constant() const noexcept
      {
        return Constant;
      }
      BindingPattern *binding() noexcept
      {
        return Binding;
      }
      const BindingPattern *binding() const noexcept
      {
        return Binding;
      }
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
      }
      Expr *initializer() noexcept
      {
        return Initializer;
      }
      const Expr *initializer() const noexcept
      {
        return Initializer;
      }
      VarDeclForm form() const noexcept
      {
        return Form;
      }

    private:
      ASTArray<Attribute> Attributes;
      bool Constant;
      BindingPattern *Binding;
      TypeSyntax *Type;
      Expr *Initializer;
      VarDeclForm Form;
  };
  class FieldDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::FieldDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      FieldDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, FieldTailKind Tail, TypeSyntax *Type, ASTArray<Parameter> Payload, Expr *Initializer)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Name(Name),
            Tail(Tail),
            Type(Type),
            Payload(Payload),
            Initializer(Initializer)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      FieldTailKind tail() const noexcept
      {
        return Tail;
      }
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
      }
      ASTArray<Parameter> payload() const noexcept
      {
        return Payload;
      }
      Expr *initializer() noexcept
      {
        return Initializer;
      }
      const Expr *initializer() const noexcept
      {
        return Initializer;
      }

    private:
      ASTArray<Attribute> Attributes;
      NameToken Name;
      FieldTailKind Tail;
      TypeSyntax *Type;
      ASTArray<Parameter> Payload;
      Expr *Initializer;
  };
  class FunctionDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::FunctionDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      FunctionDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, ASTArray<Parameter> GenericParameters, ASTArray<Parameter> Parameters, TypeSyntax *ReturnType, FunctionBodyKind BodyKind, BlockStmt *Body)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Name(Name),
            GenericParameters(GenericParameters),
            Parameters(Parameters),
            ReturnType(ReturnType),
            BodyKind(BodyKind),
            Body(Body)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      ASTArray<Parameter> genericParameters() const noexcept
      {
        return GenericParameters;
      }
      ASTArray<Parameter> parameters() const noexcept
      {
        return Parameters;
      }
      TypeSyntax *returnType() noexcept
      {
        return ReturnType;
      }
      const TypeSyntax *returnType() const noexcept
      {
        return ReturnType;
      }
      FunctionBodyKind bodyKind() const noexcept
      {
        return BodyKind;
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }

    private:
      ASTArray<Attribute> Attributes;
      NameToken Name;
      ASTArray<Parameter> GenericParameters;
      ASTArray<Parameter> Parameters;
      TypeSyntax *ReturnType;
      FunctionBodyKind BodyKind;
      BlockStmt *Body;
  };
  class ClassDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ClassDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ClassDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, ASTArray<Parameter> GenericParameters, ASTArray<BaseSpec> Bases, AggregateForm Form, BlockStmt *Body, SourceRange SemicolonRange)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Name(Name),
            GenericParameters(GenericParameters),
            Bases(Bases),
            Form(Form),
            Body(Body),
            SemicolonRange(SemicolonRange)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      ASTArray<Parameter> genericParameters() const noexcept
      {
        return GenericParameters;
      }
      ASTArray<BaseSpec> bases() const noexcept
      {
        return Bases;
      }
      AggregateForm form() const noexcept
      {
        return Form;
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }
      SourceRange semicolonRange() const noexcept
      {
        return SemicolonRange;
      }

    private:
      ASTArray<Attribute> Attributes;
      NameToken Name;
      ASTArray<Parameter> GenericParameters;
      ASTArray<BaseSpec> Bases;
      AggregateForm Form;
      BlockStmt *Body;
      SourceRange SemicolonRange;
  };
  class EnumDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::EnumDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      EnumDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, ASTArray<Parameter> GenericParameters, ASTArray<BaseSpec> Bases, AggregateForm Form, BlockStmt *Body, SourceRange SemicolonRange)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Name(Name),
            GenericParameters(GenericParameters),
            Bases(Bases),
            Form(Form),
            Body(Body),
            SemicolonRange(SemicolonRange)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      ASTArray<Parameter> genericParameters() const noexcept
      {
        return GenericParameters;
      }
      ASTArray<BaseSpec> bases() const noexcept
      {
        return Bases;
      }
      AggregateForm form() const noexcept
      {
        return Form;
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }
      SourceRange semicolonRange() const noexcept
      {
        return SemicolonRange;
      }

    private:
      ASTArray<Attribute> Attributes;
      NameToken Name;
      ASTArray<Parameter> GenericParameters;
      ASTArray<BaseSpec> Bases;
      AggregateForm Form;
      BlockStmt *Body;
      SourceRange SemicolonRange;
  };
  class InterfaceDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::InterfaceDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      InterfaceDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, ASTArray<Parameter> GenericParameters, ASTArray<BaseSpec> Bases, AggregateForm Form, BlockStmt *Body, SourceRange SemicolonRange)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Name(Name),
            GenericParameters(GenericParameters),
            Bases(Bases),
            Form(Form),
            Body(Body),
            SemicolonRange(SemicolonRange)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      ASTArray<Parameter> genericParameters() const noexcept
      {
        return GenericParameters;
      }
      ASTArray<BaseSpec> bases() const noexcept
      {
        return Bases;
      }
      AggregateForm form() const noexcept
      {
        return Form;
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }
      SourceRange semicolonRange() const noexcept
      {
        return SemicolonRange;
      }

    private:
      ASTArray<Attribute> Attributes;
      NameToken Name;
      ASTArray<Parameter> GenericParameters;
      ASTArray<BaseSpec> Bases;
      AggregateForm Form;
      BlockStmt *Body;
      SourceRange SemicolonRange;
  };
  class WildcardBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::WildcardBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      WildcardBindingPattern(SourceRange Range)
          : BindingPattern(Kind, Range)
      {
      }

    private:
  };
  class NameBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::NameBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      NameBindingPattern(SourceRange Range, NameToken Name)
          : BindingPattern(Kind, Range),
            Name(Name)
      {
      }
      NameToken name() const noexcept
      {
        return Name;
      }

    private:
      NameToken Name;
  };
  class TupleBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::TupleBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      TupleBindingPattern(SourceRange Range, ASTArray<BindingPattern *> Elements)
          : BindingPattern(Kind, Range),
            Elements(Elements)
      {
      }
      ASTArray<BindingPattern *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<BindingPattern> elements() const noexcept
      {
        return ConstNodeArray<BindingPattern>(Elements);
      }

    private:
      ASTArray<BindingPattern *> Elements;
  };
  class ArrayBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ArrayBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ArrayBindingPattern(SourceRange Range, ASTArray<BindingPattern *> Elements, std::optional<RestBinding> Rest)
          : BindingPattern(Kind, Range),
            Elements(Elements),
            Rest(Rest)
      {
      }
      ASTArray<BindingPattern *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<BindingPattern> elements() const noexcept
      {
        return ConstNodeArray<BindingPattern>(Elements);
      }
      std::optional<RestBinding> rest() const noexcept
      {
        return Rest;
      }

    private:
      ASTArray<BindingPattern *> Elements;
      std::optional<RestBinding> Rest;
  };
  class WildcardMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::WildcardMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      WildcardMatchPattern(SourceRange Range)
          : MatchPattern(Kind, Range)
      {
      }

    private:
  };
  class NameMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::NameMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      NameMatchPattern(SourceRange Range, ASTArray<PathSegment> Path, bool HasArguments, ASTArray<MatchPattern *> Arguments)
          : MatchPattern(Kind, Range),
            Path(Path),
            HasArguments(HasArguments),
            Arguments(Arguments)
      {
      }
      ASTArray<PathSegment> path() const noexcept
      {
        return Path;
      }
      bool hasArguments() const noexcept
      {
        return HasArguments;
      }
      ASTArray<MatchPattern *> arguments() noexcept
      {
        return Arguments;
      }
      ConstNodeArray<MatchPattern> arguments() const noexcept
      {
        return ConstNodeArray<MatchPattern>(Arguments);
      }

    private:
      ASTArray<PathSegment> Path;
      bool HasArguments;
      ASTArray<MatchPattern *> Arguments;
  };
  class TupleMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::TupleMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      TupleMatchPattern(SourceRange Range, ASTArray<MatchPattern *> Elements)
          : MatchPattern(Kind, Range),
            Elements(Elements)
      {
      }
      ASTArray<MatchPattern *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<MatchPattern> elements() const noexcept
      {
        return ConstNodeArray<MatchPattern>(Elements);
      }

    private:
      ASTArray<MatchPattern *> Elements;
  };
  class ArrayMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ArrayMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ArrayMatchPattern(SourceRange Range, ASTArray<MatchPattern *> Elements, std::optional<RestBinding> Rest)
          : MatchPattern(Kind, Range),
            Elements(Elements),
            Rest(Rest)
      {
      }
      ASTArray<MatchPattern *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<MatchPattern> elements() const noexcept
      {
        return ConstNodeArray<MatchPattern>(Elements);
      }
      std::optional<RestBinding> rest() const noexcept
      {
        return Rest;
      }

    private:
      ASTArray<MatchPattern *> Elements;
      std::optional<RestBinding> Rest;
  };
  class LiteralMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::LiteralMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      LiteralMatchPattern(SourceRange Range, LiteralExpr *Value, bool Negative)
          : MatchPattern(Kind, Range),
            Value(Value),
            Negative(Negative)
      {
      }
      LiteralExpr *value() noexcept
      {
        return Value;
      }
      const LiteralExpr *value() const noexcept
      {
        return Value;
      }
      bool negative() const noexcept
      {
        return Negative;
      }

    private:
      LiteralExpr *Value;
      bool Negative;
  };
  class GroupedMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::GroupedMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      GroupedMatchPattern(SourceRange Range, MatchPattern *Pattern)
          : MatchPattern(Kind, Range),
            Pattern(Pattern)
      {
      }
      MatchPattern *pattern() noexcept
      {
        return Pattern;
      }
      const MatchPattern *pattern() const noexcept
      {
        return Pattern;
      }

    private:
      MatchPattern *Pattern;
  };
  class OrMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::OrMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      OrMatchPattern(SourceRange Range, ASTArray<MatchPattern *> Alternatives)
          : MatchPattern(Kind, Range),
            Alternatives(Alternatives)
      {
      }
      ASTArray<MatchPattern *> alternatives() noexcept
      {
        return Alternatives;
      }
      ConstNodeArray<MatchPattern> alternatives() const noexcept
      {
        return ConstNodeArray<MatchPattern>(Alternatives);
      }

    private:
      ASTArray<MatchPattern *> Alternatives;
  };

  template <typename T>
  bool isa(const ASTNodeBase *Node)
  {
    return T::classof(Node);
  }
  template <typename T>
  T *dyn_cast(ASTNodeBase *Node)
  {
    return isa<T>(Node) ? static_cast<T *>(Node) : nullptr;
  }
  template <typename T>
  const T *dyn_cast(const ASTNodeBase *Node)
  {
    return isa<T>(Node) ? static_cast<const T *>(Node) : nullptr;
  }
  template <typename T>
  T *cast(ASTNodeBase *Node)
  {
    assert(isa<T>(Node));
    return static_cast<T *>(Node);
  }
  template <typename T>
  const T *cast(const ASTNodeBase *Node)
  {
    assert(isa<T>(Node));
    return static_cast<const T *>(Node);
  }
} // namespace ink::parser
