#include "ink/ast/cst_lowering.h"

#include "ink/parser/cst.h"
#include "ink/tokenizer/token.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace ink::ast
{
  namespace
  {
    enum class CstDisposition : std::uint8_t
    {
      Fold,
      SourceFile,
      Error,
      Import,
      Binding,
      Parameter,
      Function,
      UnsupportedDeclaration,
      ContextualComptime,
      Block,
      Assignment,
      ExpressionStatement,
      IfStatement,
      WhileStatement,
      ReturnStatement,
      BreakStatement,
      ContinueStatement,
      UnsupportedStatement,
      BindingPattern,
      WildcardPattern,
      TuplePattern,
      VariantPattern,
      Literal,
      Name,
      TypeName,
      BuiltinType,
      ThisExpression,
      GroupExpression,
      TypeGroupExpression,
      UnaryExpression,
      BinaryExpression,
      CallExpression,
      IfExpression,
      ComptimeExpression,
      FunctionTypeExpression,
      TypeSyntax,
      UnsupportedExpression,
    };

    CstDisposition disposition(parser::CstKind Kind)
    {
      switch (Kind)
      {
      case parser::CstKind::Unknown:
      case parser::CstKind::Error:
        return CstDisposition::Error;
      case parser::CstKind::SourceFile:
        return CstDisposition::SourceFile;
      case parser::CstKind::ModuleImportDeclaration:
        return CstDisposition::Import;
      case parser::CstKind::TopLevelBindingDeclaration:
      case parser::CstKind::LocalBindingDeclaration:
        return CstDisposition::Binding;
      case parser::CstKind::GenericParameter:
      case parser::CstKind::FunctionParameter:
        return CstDisposition::Parameter;
      case parser::CstKind::FunctionDeclaration:
      case parser::CstKind::DecoratorDeclaration:
        return CstDisposition::Function;
      case parser::CstKind::ClassDeclaration:
      case parser::CstKind::InterfaceDeclaration:
      case parser::CstKind::EnumDeclaration:
      case parser::CstKind::FieldDeclaration:
      case parser::CstKind::EnumBranch:
      case parser::CstKind::MemberImportDeclaration:
      case parser::CstKind::AttributeList:
      case parser::CstKind::DecoratorApplication:
      case parser::CstKind::FunctionModifier:
      case parser::CstKind::ExternModifier:
      case parser::CstKind::AccessModifier:
      case parser::CstKind::TypeModifier:
      case parser::CstKind::ReceiverQualifier:
      case parser::CstKind::ConstructorInitializerClause:
        return CstDisposition::UnsupportedDeclaration;
      case parser::CstKind::ComptimeBlockControl:
      case parser::CstKind::ComptimeIfControl:
      case parser::CstKind::ComptimeMatchControl:
      case parser::CstKind::ComptimeForControl:
      case parser::CstKind::ComptimeWhileControl:
        return CstDisposition::ContextualComptime;
      case parser::CstKind::StatementBlock:
        return CstDisposition::Block;
      case parser::CstKind::AssignmentStatement:
        return CstDisposition::Assignment;
      case parser::CstKind::ExpressionStatement:
        return CstDisposition::ExpressionStatement;
      case parser::CstKind::IfStatement:
        return CstDisposition::IfStatement;
      case parser::CstKind::WhileStatement:
        return CstDisposition::WhileStatement;
      case parser::CstKind::ReturnStatement:
        return CstDisposition::ReturnStatement;
      case parser::CstKind::BreakStatement:
        return CstDisposition::BreakStatement;
      case parser::CstKind::ContinueStatement:
        return CstDisposition::ContinueStatement;
      case parser::CstKind::MatchStatement:
      case parser::CstKind::ForStatement:
      case parser::CstKind::DeferStatement:
      case parser::CstKind::ThrowStatement:
      case parser::CstKind::TryStatement:
        return CstDisposition::UnsupportedStatement;
      case parser::CstKind::BindingPattern:
      case parser::CstKind::ForBindingPattern:
        return CstDisposition::BindingPattern;
      case parser::CstKind::WildcardPattern:
      case parser::CstKind::ForWildcardPattern:
        return CstDisposition::WildcardPattern;
      case parser::CstKind::TuplePattern:
        return CstDisposition::TuplePattern;
      case parser::CstKind::VariantPattern:
        return CstDisposition::VariantPattern;
      case parser::CstKind::LiteralExpression:
        return CstDisposition::Literal;
      case parser::CstKind::NameExpression:
        return CstDisposition::Name;
      case parser::CstKind::TypeName:
        return CstDisposition::TypeName;
      case parser::CstKind::BuiltinTypeExpression:
        return CstDisposition::BuiltinType;
      case parser::CstKind::ThisExpression:
        return CstDisposition::ThisExpression;
      case parser::CstKind::ParenthesizedExpression:
        return CstDisposition::GroupExpression;
      case parser::CstKind::ParenthesizedTypeExpression:
        return CstDisposition::TypeGroupExpression;
      case parser::CstKind::UnaryExpression:
        return CstDisposition::UnaryExpression;
      case parser::CstKind::BinaryExpression:
        return CstDisposition::BinaryExpression;
      case parser::CstKind::CallExpression:
        return CstDisposition::CallExpression;
      case parser::CstKind::IfExpression:
        return CstDisposition::IfExpression;
      case parser::CstKind::ComptimeExpression:
        return CstDisposition::ComptimeExpression;
      case parser::CstKind::FunctionType:
        return CstDisposition::FunctionTypeExpression;
      case parser::CstKind::ClassTypeExpression:
      case parser::CstKind::MatchCondition:
      case parser::CstKind::WhileMatchCondition:
      case parser::CstKind::ForRangeSource:
      case parser::CstKind::ListExpansion:
      case parser::CstKind::IndexExpression:
      case parser::CstKind::SliceExpression:
      case parser::CstKind::MemberExpression:
      case parser::CstKind::PointerMemberExpression:
      case parser::CstKind::GenericArgumentClause:
      case parser::CstKind::AggregateInitializationExpression:
      case parser::CstKind::ArrayExpression:
      case parser::CstKind::MatchExpression:
      case parser::CstKind::ConstTypeValueExpression:
      case parser::CstKind::TypeConstructorExpression:
      case parser::CstKind::TypePostfixSuffix:
      case parser::CstKind::BracketPostfixSuffix:
      case parser::CstKind::NamedArgument:
      case parser::CstKind::ForwardAllArguments:
      case parser::CstKind::ParenthesizedCommaList:
        return CstDisposition::UnsupportedExpression;
      case parser::CstKind::TypeSyntax:
        return CstDisposition::TypeSyntax;
      case parser::CstKind::ModulePath:
      case parser::CstKind::ImportAlias:
      case parser::CstKind::ImportedMember:
      case parser::CstKind::NamedBindingDeclaration:
      case parser::CstKind::TupleDestructuringDeclaration:
      case parser::CstKind::AttributeApplication:
      case parser::CstKind::ApplicationArgumentClause:
      case parser::CstKind::TypeDeclarationPrefix:
      case parser::CstKind::IdentifierFunctionName:
      case parser::CstKind::DestructorFormFunctionName:
      case parser::CstKind::GenericParameterClause:
      case parser::CstKind::GenericParameterList:
      case parser::CstKind::ParameterPackSuffix:
      case parser::CstKind::DefaultArgument:
      case parser::CstKind::FunctionParameterClause:
      case parser::CstKind::FunctionParameterList:
      case parser::CstKind::ReturnClause:
      case parser::CstKind::FunctionDefinition:
      case parser::CstKind::ConstructorInitializer:
      case parser::CstKind::ConstructorInitializerTarget:
      case parser::CstKind::ClassDefinitionTail:
      case parser::CstKind::InheritanceClause:
      case parser::CstKind::ClassMemberBlock:
      case parser::CstKind::InterfaceMemberBlock:
      case parser::CstKind::EnumMemberBlock:
      case parser::CstKind::FieldAnnotationSequence:
      case parser::CstKind::FieldModifierSequence:
      case parser::CstKind::FieldInitializer:
      case parser::CstKind::EnumPayloadClause:
      case parser::CstKind::EnumDiscriminantClause:
      case parser::CstKind::RegionArm:
      case parser::CstKind::TopLevelBlock:
      case parser::CstKind::MatchStatementArm:
      case parser::CstKind::ForBindingMode:
      case parser::CstKind::ThrowCauseClause:
      case parser::CstKind::CatchClause:
      case parser::CstKind::TypedCatchClause:
      case parser::CstKind::CatchAllClause:
      case parser::CstKind::CatchBinding:
      case parser::CstKind::ArgumentList:
      case parser::CstKind::PositionalArgument:
      case parser::CstKind::GenericArgumentList:
      case parser::CstKind::GenericArgument:
      case parser::CstKind::AggregateFieldInitializer:
      case parser::CstKind::AggregateFieldShorthand:
      case parser::CstKind::MatchExpressionArm:
      case parser::CstKind::Operator:
      case parser::CstKind::ConstTypeQualifier:
      case parser::CstKind::FunctionTypeParameter:
      case parser::CstKind::FunctionTypeResult:
      case parser::CstKind::FunctionTypeExpression:
      case parser::CstKind::EmptyBracketTypeSuffix:
      case parser::CstKind::PointerTypeSuffix:
      case parser::CstKind::ReferenceTypeSuffix:
        return CstDisposition::Fold;
      }
      throw std::logic_error("CST kind has no explicit lowering disposition");
    }

    AstNodeCategory categoryFor(CstDisposition Disposition, AstNodeCategory ParentCategory)
    {
      switch (Disposition)
      {
      case CstDisposition::Fold:
        return ParentCategory;
      case CstDisposition::SourceFile:
      case CstDisposition::Import:
      case CstDisposition::Binding:
      case CstDisposition::Parameter:
      case CstDisposition::Function:
      case CstDisposition::UnsupportedDeclaration:
        return AstNodeCategory::Declaration;
      case CstDisposition::ContextualComptime:
        return ParentCategory == AstNodeCategory::Statement ? AstNodeCategory::Statement : AstNodeCategory::Declaration;
      case CstDisposition::Block:
      case CstDisposition::Assignment:
      case CstDisposition::ExpressionStatement:
      case CstDisposition::IfStatement:
      case CstDisposition::WhileStatement:
      case CstDisposition::ReturnStatement:
      case CstDisposition::BreakStatement:
      case CstDisposition::ContinueStatement:
      case CstDisposition::UnsupportedStatement:
        return AstNodeCategory::Statement;
      case CstDisposition::BindingPattern:
      case CstDisposition::WildcardPattern:
      case CstDisposition::TuplePattern:
      case CstDisposition::VariantPattern:
        return AstNodeCategory::Pattern;
      case CstDisposition::Literal:
      case CstDisposition::Name:
      case CstDisposition::TypeName:
      case CstDisposition::BuiltinType:
      case CstDisposition::ThisExpression:
      case CstDisposition::GroupExpression:
      case CstDisposition::TypeGroupExpression:
      case CstDisposition::UnaryExpression:
      case CstDisposition::BinaryExpression:
      case CstDisposition::CallExpression:
      case CstDisposition::IfExpression:
      case CstDisposition::ComptimeExpression:
      case CstDisposition::FunctionTypeExpression:
      case CstDisposition::TypeSyntax:
      case CstDisposition::UnsupportedExpression:
        return AstNodeCategory::Expression;
      case CstDisposition::Error:
        return ParentCategory == AstNodeCategory::Unknown ? AstNodeCategory::Declaration : ParentCategory;
      }
      throw std::logic_error("CST lowering disposition has no AST category");
    }

    AstExpectedKind expectedKind(tokenizer::TokenKind Kind) noexcept
    {
      switch (Kind)
      {
      case tokenizer::TokenKind::Identifier:
        return AstExpectedKind::Identifier;
      case tokenizer::TokenKind::Keyword:
        return AstExpectedKind::Keyword;
      case tokenizer::TokenKind::BuiltinType:
        return AstExpectedKind::BuiltinType;
      case tokenizer::TokenKind::BoolLiteral:
      case tokenizer::TokenKind::NullLiteral:
      case tokenizer::TokenKind::IntegerLiteral:
      case tokenizer::TokenKind::FloatLiteral:
      case tokenizer::TokenKind::ScalarLiteral:
      case tokenizer::TokenKind::StringLiteral:
        return AstExpectedKind::Literal;
      case tokenizer::TokenKind::Symbol:
        return AstExpectedKind::Symbol;
      case tokenizer::TokenKind::EndOfFile:
        return AstExpectedKind::EndOfFile;
      case tokenizer::TokenKind::Utf8Bom:
      case tokenizer::TokenKind::SpacesAndTabs:
      case tokenizer::TokenKind::LineBreak:
      case tokenizer::TokenKind::LineComment:
      case tokenizer::TokenKind::BlockComment:
      case tokenizer::TokenKind::InvalidEncoding:
      case tokenizer::TokenKind::InvalidCharacter:
      case tokenizer::TokenKind::InvalidIdentifier:
      case tokenizer::TokenKind::InvalidNumber:
      case tokenizer::TokenKind::InvalidScalarLiteral:
      case tokenizer::TokenKind::InvalidStringLiteral:
      case tokenizer::TokenKind::UnterminatedBlockComment:
        return AstExpectedKind::Unknown;
      }
      return AstExpectedKind::Unknown;
    }

    struct SyntaxToken
    {
      tokenizer::TokenKind Kind = tokenizer::TokenKind::InvalidCharacter;
      core::SourceRange Range;
      std::string_view Text;
    };

    struct Fragment
    {
      std::vector<AstNodeRef> Nodes;
      std::vector<SyntaxToken> Tokens;
      std::vector<AstRecovery> Recoveries;
      std::optional<std::size_t> Start;
      std::optional<std::size_t> End;
    };

    struct Frame
    {
      parser::CstNodeId Id = 0;
      std::size_t NodeStartToken = 0;
      AstNodeCategory Category = AstNodeCategory::Unknown;
      std::size_t NextChild = 0;
      std::size_t ConsumedTokens = 0;
      Fragment Content;
    };

    void includeRange(Fragment &Target, core::SourceRange Range)
    {
      Target.Start = Target.Start ? (std::min)(*Target.Start, Range.Start) : Range.Start;
      Target.End = Target.End ? (std::max)(*Target.End, Range.End) : Range.End;
    }

    void appendFragment(Fragment &Target, Fragment Source)
    {
      if (Source.Start)
      {
        includeRange(Target, {*Source.Start, Source.End.value_or(*Source.Start)});
      }
      Target.Nodes.insert(Target.Nodes.end(), std::make_move_iterator(Source.Nodes.begin()), std::make_move_iterator(Source.Nodes.end()));
      Target.Tokens.insert(Target.Tokens.end(), std::make_move_iterator(Source.Tokens.begin()), std::make_move_iterator(Source.Tokens.end()));
      Target.Recoveries.insert(Target.Recoveries.end(), std::make_move_iterator(Source.Recoveries.begin()), std::make_move_iterator(Source.Recoveries.end()));
    }

    core::SourceRange fragmentRange(const Fragment &Value, std::size_t Anchor) noexcept
    {
      return Value.Start ? core::SourceRange{*Value.Start, Value.End.value_or(*Value.Start)} : core::SourceRange{Anchor, Anchor};
    }

    std::size_t frameAnchor(const Frame &Value, const tokenizer::TokenizedBuffer &LexedFile) noexcept
    {
      return Value.NodeStartToken < LexedFile.tokens().size() ? LexedFile.tokens()[Value.NodeStartToken].Span.Start : LexedFile.source().size();
    }

    CstOrigin nodeOrigin(parser::CstNodeId Id)
    {
      if (Id >= CstOrigin::InvalidValue)
      {
        throw std::length_error("CST node ID exceeds the AST origin range");
      }
      return CstOrigin::node(static_cast<CstOrigin::ValueType>(Id));
    }

    CstOrigin elementOrigin(parser::CstNodeId Id, std::size_t Element)
    {
      if (Id >= CstOrigin::InvalidValue || Element >= CstOrigin::InvalidValue)
      {
        throw std::length_error("CST element origin exceeds the AST origin range");
      }
      return CstOrigin::element(static_cast<CstOrigin::ValueType>(Id), static_cast<CstOrigin::ValueType>(Element));
    }

    std::vector<AstNodeRef> nodesOfCategory(const Fragment &Value, AstNodeCategory Category)
    {
      std::vector<AstNodeRef> Result;
      for (const AstNodeRef Node : Value.Nodes)
      {
        if (Node.Category == Category)
        {
          Result.push_back(Node);
        }
      }
      return Result;
    }

    std::uint64_t nodeKey(AstNodeRef Ref) noexcept
    {
      return (static_cast<std::uint64_t>(Ref.Category) << 32U) | Ref.Index;
    }

    std::vector<AstNodeRef> supplementalNodes(const std::vector<AstNodeRef> &All, const std::vector<AstNodeRef> &Used)
    {
      std::unordered_set<std::uint64_t> UsedKeys;
      UsedKeys.reserve(Used.size());
      for (const AstNodeRef Node : Used)
      {
        UsedKeys.insert(nodeKey(Node));
      }
      std::vector<AstNodeRef> Result;
      Result.reserve(All.size());
      for (const AstNodeRef Node : All)
      {
        if (UsedKeys.find(nodeKey(Node)) == UsedKeys.end())
        {
          Result.push_back(Node);
        }
      }
      return Result;
    }

    AstNodeHeader makeHeader(AstContext &Context, core::SourceRange Range, CstOrigin Origin, std::vector<AstRecovery> Recoveries, std::vector<AstNodeRef> Supplemental)
    {
      return {Range, Origin, Context.addRecoveries(std::move(Recoveries)), Context.addList(std::move(Supplemental))};
    }

    AstNodeRef addDeclaration(AstContext &Context, AstNodeHeader Header, AstKind Kind, DeclarationPayload Payload)
    {
      return AstNodeRef::declaration(Context.addDeclaration({std::move(Header), Kind, std::move(Payload)}));
    }

    AstNodeRef addExpression(AstContext &Context, AstNodeHeader Header, AstKind Kind, ExpressionPayload Payload)
    {
      return AstNodeRef::expression(Context.addExpression({std::move(Header), Kind, std::move(Payload)}));
    }

    AstNodeRef addStatement(AstContext &Context, AstNodeHeader Header, AstKind Kind, StatementPayload Payload)
    {
      return AstNodeRef::statement(Context.addStatement({std::move(Header), Kind, std::move(Payload)}));
    }

    AstNodeRef addPattern(AstContext &Context, AstNodeHeader Header, AstKind Kind, PatternPayload Payload)
    {
      return AstNodeRef::pattern(Context.addPattern({std::move(Header), Kind, std::move(Payload)}));
    }

    AstNodeRef synthesizeError(AstContext &Context, AstNodeCategory Category, core::SourceRange Range, CstOrigin Origin)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, {}, {});
      const ErrorPayload Payload{Context.addList({})};
      switch (Category)
      {
      case AstNodeCategory::Declaration:
        return addDeclaration(Context, Header, AstKind::ErrorDeclaration, Payload);
      case AstNodeCategory::Expression:
        return addExpression(Context, Header, AstKind::ErrorExpression, Payload);
      case AstNodeCategory::Statement:
        return addStatement(Context, Header, AstKind::ErrorStatement, Payload);
      case AstNodeCategory::Pattern:
        return addPattern(Context, Header, AstKind::ErrorPattern, Payload);
      case AstNodeCategory::Unknown:
        break;
      }
      throw std::logic_error("cannot synthesize an AST error without a category");
    }

    struct PlaceholderSite
    {
      core::SourceRange Range;
      CstOrigin Origin;
    };

    PlaceholderSite placeholderSite(const Fragment &Content, std::size_t PreferredMissing, core::SourceRange FallbackRange, CstOrigin FallbackOrigin) noexcept
    {
      const AstRecovery *LastMissing = nullptr;
      std::size_t MissingIndex = 0;
      for (const AstRecovery &Recovery : Content.Recoveries)
      {
        if (Recovery.Kind != AstRecoveryKind::MissingToken)
        {
          continue;
        }
        LastMissing = &Recovery;
        if (MissingIndex++ == PreferredMissing)
        {
          return {Recovery.Range, Recovery.Origin};
        }
      }
      return LastMissing ? PlaceholderSite{LastMissing->Range, LastMissing->Origin} : PlaceholderSite{FallbackRange, FallbackOrigin};
    }

    AstExprId requiredExpression(AstContext &Context, const Fragment &Content, const std::vector<AstNodeRef> &Expressions, std::size_t Index, core::SourceRange Range, CstOrigin Origin, std::vector<AstNodeRef> &Used, std::optional<std::size_t> PreferredMissing = std::nullopt)
    {
      const PlaceholderSite Site = placeholderSite(Content, PreferredMissing.value_or(Index), Range, Origin);
      AstNodeRef Result = Index < Expressions.size() ? Expressions[Index] : synthesizeError(Context, AstNodeCategory::Expression, Site.Range, Site.Origin);
      if (Index < Expressions.size())
      {
        Used.push_back(Result);
      }
      return AstExprId::fromValue(Result.Index);
    }

    AstStmtId requiredStatement(AstContext &Context, const Fragment &Content, const std::vector<AstNodeRef> &Statements, std::size_t Index, core::SourceRange Range, CstOrigin Origin, std::vector<AstNodeRef> &Used)
    {
      const PlaceholderSite Site = placeholderSite(Content, Index, Range, Origin);
      AstNodeRef Result = Index < Statements.size() ? Statements[Index] : synthesizeError(Context, AstNodeCategory::Statement, Site.Range, Site.Origin);
      if (Index < Statements.size())
      {
        Used.push_back(Result);
      }
      return AstStmtId::fromValue(Result.Index);
    }

    AstPatternId requiredPattern(AstContext &Context, const Fragment &Content, const std::vector<AstNodeRef> &Patterns, std::size_t Index, core::SourceRange Range, CstOrigin Origin, std::vector<AstNodeRef> &Used)
    {
      const PlaceholderSite Site = placeholderSite(Content, Index, Range, Origin);
      AstNodeRef Result = Index < Patterns.size() ? Patterns[Index] : synthesizeError(Context, AstNodeCategory::Pattern, Site.Range, Site.Origin);
      if (Index < Patterns.size())
      {
        Used.push_back(Result);
      }
      return AstPatternId::fromValue(Result.Index);
    }

    std::optional<std::size_t> tokenIndex(const Fragment &Value, std::string_view Text, std::size_t Start = 0) noexcept
    {
      for (std::size_t Index = Start; Index < Value.Tokens.size(); ++Index)
      {
        if (Value.Tokens[Index].Text == Text)
        {
          return Index;
        }
      }
      return std::nullopt;
    }

    std::optional<std::size_t> firstTokenOfKind(const Fragment &Value, tokenizer::TokenKind Kind, std::size_t Start = 0) noexcept
    {
      for (std::size_t Index = Start; Index < Value.Tokens.size(); ++Index)
      {
        if (Value.Tokens[Index].Kind == Kind)
        {
          return Index;
        }
      }
      return std::nullopt;
    }

    std::string joinedTokenText(const Fragment &Value, std::size_t Begin, std::size_t End)
    {
      std::string Result;
      for (std::size_t Index = Begin; Index < End && Index < Value.Tokens.size(); ++Index)
      {
        Result.append(Value.Tokens[Index].Text);
      }
      return Result;
    }

    bool isOperatorToken(const SyntaxToken &Token) noexcept
    {
      return (Token.Kind == tokenizer::TokenKind::Symbol && Token.Text.size() == 1 && std::string_view("+-*/%&|^!~=<>?").find(Token.Text.front()) != std::string_view::npos) || Token.Text == "await";
    }

    core::InternedStringId internOperator(core::StringInterner &Strings, const Fragment &Value, std::size_t Begin, std::size_t End)
    {
      std::string Result;
      for (const SyntaxToken &Token : Value.Tokens)
      {
        if (Begin <= Token.Range.Start && Token.Range.End <= End && isOperatorToken(Token))
        {
          Result.append(Token.Text);
        }
      }
      if (Result.empty())
      {
        for (const SyntaxToken &Token : Value.Tokens)
        {
          if (isOperatorToken(Token))
          {
            Result.append(Token.Text);
          }
        }
      }
      return Strings.intern(Result);
    }

    AstBindingMode bindingModeAt(const Fragment &Value, const std::optional<std::size_t> &Introducer) noexcept
    {
      if (!Introducer)
      {
        return AstBindingMode::Unknown;
      }
      if (Value.Tokens[*Introducer].Text == "let")
      {
        return AstBindingMode::Let;
      }
      if (Value.Tokens[*Introducer].Text == "var")
      {
        return AstBindingMode::Var;
      }
      return AstBindingMode::Const;
    }

    std::optional<std::size_t> bindingIntroducer(const Fragment &Value, const AstContext &Context)
    {
      for (std::size_t Index = 0; Index < Value.Tokens.size(); ++Index)
      {
        if (Value.Tokens[Index].Text == "let" || Value.Tokens[Index].Text == "var" || Value.Tokens[Index].Text == "const")
        {
          bool CoveredByChild = false;
          for (const AstNodeRef Node : Value.Nodes)
          {
            const core::SourceRange ChildRange = Context.node(Node).Header->Range;
            if (ChildRange.Start <= Value.Tokens[Index].Range.Start && Value.Tokens[Index].Range.End <= ChildRange.End)
            {
              CoveredByChild = true;
              break;
            }
          }
          if (!CoveredByChild)
          {
            return Index;
          }
        }
      }
      return std::nullopt;
    }

    UnsupportedFeature declarationFeature(parser::CstKind Kind) noexcept
    {
      switch (Kind)
      {
      case parser::CstKind::AttributeList:
        return UnsupportedFeature::Attribute;
      case parser::CstKind::DecoratorApplication:
        return UnsupportedFeature::Decorator;
      case parser::CstKind::FunctionModifier:
      case parser::CstKind::ExternModifier:
      case parser::CstKind::AccessModifier:
      case parser::CstKind::TypeModifier:
      case parser::CstKind::ReceiverQualifier:
        return UnsupportedFeature::DeclarationModifier;
      case parser::CstKind::ConstructorInitializerClause:
        return UnsupportedFeature::ConstructorInitializer;
      case parser::CstKind::MemberImportDeclaration:
        return UnsupportedFeature::Import;
      case parser::CstKind::ClassDeclaration:
      case parser::CstKind::FieldDeclaration:
        return UnsupportedFeature::Class;
      case parser::CstKind::InterfaceDeclaration:
        return UnsupportedFeature::Interface;
      case parser::CstKind::EnumDeclaration:
      case parser::CstKind::EnumBranch:
        return UnsupportedFeature::Enum;
      default:
        return UnsupportedFeature::Unknown;
      }
    }

    UnsupportedFeature statementFeature(parser::CstKind Kind) noexcept
    {
      switch (Kind)
      {
      case parser::CstKind::MatchStatement:
        return UnsupportedFeature::Match;
      case parser::CstKind::ForStatement:
        return UnsupportedFeature::For;
      case parser::CstKind::DeferStatement:
        return UnsupportedFeature::Defer;
      case parser::CstKind::ThrowStatement:
        return UnsupportedFeature::Throw;
      case parser::CstKind::TryStatement:
        return UnsupportedFeature::Try;
      default:
        return UnsupportedFeature::Unknown;
      }
    }

    UnsupportedFeature expressionFeature(parser::CstKind Kind) noexcept
    {
      switch (Kind)
      {
      case parser::CstKind::ClassTypeExpression:
        return UnsupportedFeature::Class;
      case parser::CstKind::MatchCondition:
      case parser::CstKind::WhileMatchCondition:
      case parser::CstKind::MatchExpression:
        return UnsupportedFeature::Match;
      case parser::CstKind::ForRangeSource:
        return UnsupportedFeature::For;
      case parser::CstKind::ListExpansion:
      case parser::CstKind::ArrayExpression:
        return UnsupportedFeature::Array;
      case parser::CstKind::IndexExpression:
        return UnsupportedFeature::Index;
      case parser::CstKind::SliceExpression:
        return UnsupportedFeature::Slice;
      case parser::CstKind::MemberExpression:
        return UnsupportedFeature::MemberAccess;
      case parser::CstKind::PointerMemberExpression:
        return UnsupportedFeature::PointerMemberAccess;
      case parser::CstKind::GenericArgumentClause:
        return UnsupportedFeature::Generic;
      case parser::CstKind::AggregateInitializationExpression:
        return UnsupportedFeature::Aggregate;
      case parser::CstKind::TypeConstructorExpression:
        return UnsupportedFeature::TypeConstructor;
      case parser::CstKind::ConstTypeValueExpression:
      case parser::CstKind::TypePostfixSuffix:
      case parser::CstKind::BracketPostfixSuffix:
        return UnsupportedFeature::ComplexType;
      case parser::CstKind::NamedArgument:
      case parser::CstKind::ForwardAllArguments:
        return UnsupportedFeature::CallArgument;
      case parser::CstKind::ParenthesizedCommaList:
        return UnsupportedFeature::Tuple;
      default:
        return UnsupportedFeature::Unknown;
      }
    }

    bool isTypeUnsupportedFeature(UnsupportedFeature Feature) noexcept
    {
      switch (Feature)
      {
      case UnsupportedFeature::Generic:
      case UnsupportedFeature::Class:
      case UnsupportedFeature::Array:
      case UnsupportedFeature::TypeConstructor:
      case UnsupportedFeature::ComplexType:
      case UnsupportedFeature::Tuple:
        return true;
      default:
        return false;
      }
    }

    bool isLoweredTypeExpression(const AstContext &Context, AstNodeRef Ref)
    {
      if (Ref.Category != AstNodeCategory::Expression)
      {
        return false;
      }
      const Expression &Node = Context.expression(AstExprId::fromValue(Ref.Index));
      switch (Node.Kind)
      {
      case AstKind::ErrorExpression:
      case AstKind::TypeNameExpression:
      case AstKind::BuiltinTypeExpression:
      case AstKind::TypeGroupExpression:
      case AstKind::FunctionTypeExpression:
        return true;
      case AstKind::UnsupportedExpression:
      {
        const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload);
        return Payload && isTypeUnsupportedFeature(Payload->Feature);
      }
      default:
        return false;
      }
    }

    Fragment nodeFragment(AstNodeRef Node, core::SourceRange Range)
    {
      Fragment Result;
      Result.Nodes.push_back(Node);
      includeRange(Result, Range);
      return Result;
    }

    Fragment lowerError(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      Value.Content.Recoveries.push_back({AstRecoveryKind::UnexpectedSyntax, Range, nodeOrigin(Value.Id), AstExpectedKind::Unknown, Strings.intern("unexpected syntax")});
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), {});
      const ErrorPayload Payload{Context.addList(std::move(Value.Content.Nodes))};
      switch (Value.Category)
      {
      case AstNodeCategory::Declaration:
        return nodeFragment(addDeclaration(Context, Header, AstKind::ErrorDeclaration, Payload), Range);
      case AstNodeCategory::Expression:
        return nodeFragment(addExpression(Context, Header, AstKind::ErrorExpression, Payload), Range);
      case AstNodeCategory::Statement:
        return nodeFragment(addStatement(Context, Header, AstKind::ErrorStatement, Payload), Range);
      case AstNodeCategory::Pattern:
        return nodeFragment(addPattern(Context, Header, AstKind::ErrorPattern, Payload), Range);
      case AstNodeCategory::Unknown:
        break;
      }
      throw std::logic_error("cannot lower an error CST node without an AST category");
    }

    Fragment lowerPlaceholder(Frame Value, AstContext &Context, core::SourceRange Range, CstOrigin Origin)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), {});
      const ErrorPayload Payload{Context.addList(std::move(Value.Content.Nodes))};
      switch (Value.Category)
      {
      case AstNodeCategory::Declaration:
        return nodeFragment(addDeclaration(Context, Header, AstKind::ErrorDeclaration, Payload), Range);
      case AstNodeCategory::Expression:
        return nodeFragment(addExpression(Context, Header, AstKind::ErrorExpression, Payload), Range);
      case AstNodeCategory::Statement:
        return nodeFragment(addStatement(Context, Header, AstKind::ErrorStatement, Payload), Range);
      case AstNodeCategory::Pattern:
        return nodeFragment(addPattern(Context, Header, AstKind::ErrorPattern, Payload), Range);
      case AstNodeCategory::Unknown:
        break;
      }
      throw std::logic_error("cannot lower an AST placeholder without a category");
    }

    Fragment lowerSourceFile(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const std::vector<AstNodeRef> Items = nodesOfCategory(Value.Content, AstNodeCategory::Declaration);
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Items));
      const SourceFilePayload Payload{Context.addList(Items)};
      return nodeFragment(addDeclaration(Context, Header, AstKind::SourceFile, Payload), Range);
    }

    Fragment lowerImport(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      const std::optional<std::size_t> Import = tokenIndex(Value.Content, "import");
      const std::size_t PathBegin = Import ? *Import + 1 : 0;
      std::size_t PathEnd = Value.Content.Tokens.size();
      const std::optional<std::size_t> As = tokenIndex(Value.Content, "as", PathBegin);
      if (As)
      {
        PathEnd = *As;
      }
      else if (const std::optional<std::size_t> Semicolon = tokenIndex(Value.Content, ";", PathBegin))
      {
        PathEnd = *Semicolon;
      }
      const std::string Path = joinedTokenText(Value.Content, PathBegin, PathEnd);
      if (Path.empty())
      {
        return lowerPlaceholder(std::move(Value), Context, Range, Origin);
      }
      std::optional<core::InternedStringId> Alias;
      if (As)
      {
        if (const std::optional<std::size_t> AliasIndex = firstTokenOfKind(Value.Content, tokenizer::TokenKind::Identifier, *As + 1))
        {
          Alias = Strings.intern(Value.Content.Tokens[*AliasIndex].Text);
        }
        else
        {
          return lowerPlaceholder(std::move(Value), Context, Range, Origin);
        }
      }
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), std::move(Value.Content.Nodes));
      const ImportPayload Payload{Strings.intern(Path), Alias};
      return nodeFragment(addDeclaration(Context, Header, AstKind::ImportDeclaration, Payload), Range);
    }

    Fragment lowerBinding(Frame Value, parser::CstKind Kind, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const std::optional<std::size_t> Introducer = bindingIntroducer(Value.Content, Context);
      const std::size_t BindingStart = Introducer ? *Introducer + 1 : 0;
      const std::vector<AstNodeRef> Patterns = nodesOfCategory(Value.Content, AstNodeCategory::Pattern);
      AstPatternId Pattern;
      if (!Patterns.empty())
      {
        Pattern = requiredPattern(Context, Value.Content, Patterns, 0, Range, Origin, Used);
      }
      else if (const std::optional<std::size_t> Name = firstTokenOfKind(Value.Content, tokenizer::TokenKind::Identifier, BindingStart))
      {
        const core::SourceRange PatternRange = Value.Content.Tokens[*Name].Range;
        const AstNodeHeader PatternHeader = makeHeader(Context, PatternRange, Origin, {}, {});
        Pattern = AstPatternId::fromValue(addPattern(Context, PatternHeader, AstKind::BindingPattern, BindingPatternPayload{Strings.intern(Value.Content.Tokens[*Name].Text)}).Index);
      }
      else
      {
        const PlaceholderSite Site = placeholderSite(Value.Content, 0, Range, Origin);
        Pattern = AstPatternId::fromValue(synthesizeError(Context, AstNodeCategory::Pattern, Site.Range, Site.Origin).Index);
      }

      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      std::optional<AstExprId> Type;
      std::optional<AstExprId> Initializer;
      const std::optional<std::size_t> Colon = tokenIndex(Value.Content, ":", BindingStart);
      const std::optional<std::size_t> Equal = tokenIndex(Value.Content, "=", BindingStart);
      for (const AstNodeRef Expression : Expressions)
      {
        const core::SourceRange ExpressionRange = Context.node(Expression).Header->Range;
        if (!Type && Colon && ExpressionRange.Start >= Value.Content.Tokens[*Colon].Range.End && (!Equal || ExpressionRange.End <= Value.Content.Tokens[*Equal].Range.Start))
        {
          Type = AstExprId::fromValue(Expression.Index);
          Used.push_back(Expression);
        }
        else if (!Initializer && Equal && ExpressionRange.Start >= Value.Content.Tokens[*Equal].Range.End)
        {
          Initializer = AstExprId::fromValue(Expression.Index);
          Used.push_back(Expression);
        }
      }
      if (!Initializer && !Colon && !Expressions.empty())
      {
        const std::size_t BindingByteStart = Introducer ? Value.Content.Tokens[*Introducer].Range.End : Range.Start;
        for (const AstNodeRef Expression : Expressions)
        {
          if (Context.node(Expression).Header->Range.Start >= BindingByteStart)
          {
            Initializer = AstExprId::fromValue(Expression.Index);
            Used.push_back(Expression);
            break;
          }
        }
      }
      if (Colon && !Type)
      {
        const PlaceholderSite Site = placeholderSite(Value.Content, 0, Range, Origin);
        Type = AstExprId::fromValue(synthesizeError(Context, AstNodeCategory::Expression, Site.Range, Site.Origin).Index);
      }
      if (Equal && !Initializer)
      {
        const PlaceholderSite Site = placeholderSite(Value.Content, Value.Content.Recoveries.size(), Range, Origin);
        Initializer = AstExprId::fromValue(synthesizeError(Context, AstNodeCategory::Expression, Site.Range, Site.Origin).Index);
      }
      const AstBindingMode Mode = bindingModeAt(Value.Content, Introducer);
      if (!Initializer && (Mode == AstBindingMode::Const || !Type))
      {
        const PlaceholderSite Site = placeholderSite(Value.Content, Value.Content.Recoveries.size(), Range, Origin);
        Initializer = AstExprId::fromValue(synthesizeError(Context, AstNodeCategory::Expression, Site.Range, Site.Origin).Index);
      }
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      const BindingPayload Payload{Mode, Kind == parser::CstKind::TopLevelBindingDeclaration, Pattern, Type, Initializer};
      return nodeFragment(addDeclaration(Context, Header, AstKind::BindingDeclaration, Payload), Range);
    }

    Fragment lowerParameter(Frame Value, parser::CstKind Kind, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      const std::optional<std::size_t> NameIndex = firstTokenOfKind(Value.Content, tokenizer::TokenKind::Identifier);
      if (!NameIndex)
      {
        return lowerPlaceholder(std::move(Value), Context, Range, Origin);
      }
      std::vector<AstNodeRef> Used;
      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      std::vector<AstNodeRef> TypeExpressions;
      std::vector<AstNodeRef> DefaultExpressions;
      std::optional<AstExprId> DefaultValue;
      const std::optional<std::size_t> Equal = tokenIndex(Value.Content, "=");
      for (const AstNodeRef Expression : Expressions)
      {
        const core::SourceRange ExpressionRange = Context.node(Expression).Header->Range;
        if (Equal && ExpressionRange.Start >= Value.Content.Tokens[*Equal].Range.End)
        {
          DefaultExpressions.push_back(Expression);
        }
        else
        {
          TypeExpressions.push_back(Expression);
        }
      }
      const AstExprId Type = requiredExpression(Context, Value.Content, TypeExpressions, 0, Range, Origin, Used);
      if (Equal)
      {
        DefaultValue = requiredExpression(Context, Value.Content, DefaultExpressions, 0, Range, Origin, Used);
      }
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      const ParameterPayload Payload{Kind == parser::CstKind::GenericParameter ? AstParameterFlavor::Generic : AstParameterFlavor::Function, Strings.intern(Value.Content.Tokens[*NameIndex].Text), Type, DefaultValue, tokenIndex(Value.Content, "...").has_value()};
      return nodeFragment(addDeclaration(Context, Header, AstKind::ParameterDeclaration, Payload), Range);
    }

    Fragment lowerFunction(Frame Value, parser::CstKind Kind, const parser::CstTree &Tree, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::size_t NameSearch = 0;
      if (const std::optional<std::size_t> Introducer = tokenIndex(Value.Content, Kind == parser::CstKind::DecoratorDeclaration ? "decorator" : "func"))
      {
        NameSearch = *Introducer + 1;
      }
      const std::optional<std::size_t> NameIndex = firstTokenOfKind(Value.Content, tokenizer::TokenKind::Identifier, NameSearch);
      if (!NameIndex)
      {
        return lowerPlaceholder(std::move(Value), Context, Range, Origin);
      }
      std::vector<AstNodeRef> Used;
      std::vector<AstNodeRef> Parameters;
      for (const AstNodeRef Node : Value.Content.Nodes)
      {
        if (Node.Category != AstNodeCategory::Declaration)
        {
          continue;
        }
        const Declaration &DeclarationNode = Context.declaration(AstDeclId::fromValue(Node.Index));
        bool IsParameter = DeclarationNode.Kind == AstKind::ParameterDeclaration;
        if (!IsParameter && DeclarationNode.Kind == AstKind::ErrorDeclaration && DeclarationNode.Header.Origin && DeclarationNode.Header.Origin->node() < Tree.nodes().size())
        {
          const parser::CstKind OriginKind = Tree.node(DeclarationNode.Header.Origin->node()).Kind;
          IsParameter = OriginKind == parser::CstKind::GenericParameter || OriginKind == parser::CstKind::FunctionParameter;
        }
        if (IsParameter)
        {
          Parameters.push_back(Node);
          Used.push_back(Node);
        }
      }
      std::optional<AstExprId> ResultType;
      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      std::optional<std::size_t> ArrowEnd;
      for (std::size_t Index = 1; Index < Value.Content.Tokens.size(); ++Index)
      {
        if (Value.Content.Tokens[Index - 1].Text == "-" && Value.Content.Tokens[Index].Text == ">")
        {
          ArrowEnd = Value.Content.Tokens[Index].Range.End;
          break;
        }
      }
      if (ArrowEnd)
      {
        std::vector<AstNodeRef> ResultExpressions;
        for (const AstNodeRef Expression : Expressions)
        {
          if (Context.node(Expression).Header->Range.Start >= *ArrowEnd)
          {
            ResultExpressions.push_back(Expression);
          }
        }
        ResultType = requiredExpression(Context, Value.Content, ResultExpressions, 0, Range, Origin, Used);
      }
      std::optional<AstStmtId> Body;
      const std::vector<AstNodeRef> Statements = nodesOfCategory(Value.Content, AstNodeCategory::Statement);
      if (!Statements.empty())
      {
        Body = AstStmtId::fromValue(Statements.front().Index);
        Used.push_back(Statements.front());
      }
      else if (!tokenIndex(Value.Content, ";"))
      {
        Body = requiredStatement(Context, Value.Content, Statements, 0, Range, Origin, Used);
      }
      AstFunctionFlavor Flavor = Kind == parser::CstKind::DecoratorDeclaration ? AstFunctionFlavor::Decorator : AstFunctionFlavor::Function;
      if (tokenIndex(Value.Content, "~", NameSearch))
      {
        Flavor = AstFunctionFlavor::Destructor;
      }
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      const FunctionPayload Payload{Flavor, Strings.intern(Value.Content.Tokens[*NameIndex].Text), Context.addList(std::move(Parameters)), ResultType, Body};
      return nodeFragment(addDeclaration(Context, Header, AstKind::FunctionDeclaration, Payload), Range);
    }

    Fragment lowerUnsupportedDeclaration(Frame Value, parser::CstKind Kind, AstContext &Context, core::SourceRange Range)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), {});
      const UnsupportedPayload Payload{declarationFeature(Kind), Context.addList(std::move(Value.Content.Nodes))};
      return nodeFragment(addDeclaration(Context, Header, AstKind::UnsupportedDeclaration, Payload), Range);
    }

    Fragment lowerContextualComptime(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), {});
      const UnsupportedPayload Payload{UnsupportedFeature::Comptime, Context.addList(std::move(Value.Content.Nodes))};
      if (Value.Category == AstNodeCategory::Statement)
      {
        return nodeFragment(addStatement(Context, Header, AstKind::UnsupportedStatement, Payload), Range);
      }
      return nodeFragment(addDeclaration(Context, Header, AstKind::UnsupportedDeclaration, Payload), Range);
    }

    Fragment lowerBlock(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      std::vector<AstNodeRef> Items;
      for (const AstNodeRef Node : Value.Content.Nodes)
      {
        if (Node.Category == AstNodeCategory::Declaration || Node.Category == AstNodeCategory::Statement)
        {
          Items.push_back(Node);
        }
      }
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Items));
      const BlockPayload Payload{Context.addList(std::move(Items))};
      return nodeFragment(addStatement(Context, Header, AstKind::BlockStatement, Payload), Range);
    }

    Fragment lowerAssignment(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      const AstExprId Left = requiredExpression(Context, Value.Content, Expressions, 0, Range, Origin, Used);
      const AstExprId Right = requiredExpression(Context, Value.Content, Expressions, 1, Range, Origin, Used);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      const AssignmentPayload Payload{Left, internOperator(Strings, Value.Content, Context.expression(Left).Header.Range.End, Context.expression(Right).Header.Range.Start), Right};
      return nodeFragment(addStatement(Context, Header, AstKind::AssignmentStatement, Payload), Range);
    }

    Fragment lowerExpressionStatement(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const AstExprId Expression = requiredExpression(Context, Value.Content, nodesOfCategory(Value.Content, AstNodeCategory::Expression), 0, Range, Origin, Used);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addStatement(Context, Header, AstKind::ExpressionStatement, ExpressionStatementPayload{Expression}), Range);
    }

    Fragment lowerIfStatement(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      AstNodeRef Condition;
      for (const AstNodeRef Node : Value.Content.Nodes)
      {
        if (Node.Category == AstNodeCategory::Expression || Node.Category == AstNodeCategory::Pattern)
        {
          Condition = Node;
          Used.push_back(Node);
          break;
        }
      }
      if (!Condition.isValid())
      {
        const PlaceholderSite Site = placeholderSite(Value.Content, 0, Range, Origin);
        Condition = synthesizeError(Context, AstNodeCategory::Expression, Site.Range, Site.Origin);
      }
      const std::vector<AstNodeRef> Statements = nodesOfCategory(Value.Content, AstNodeCategory::Statement);
      const AstStmtId ThenBlock = requiredStatement(Context, Value.Content, Statements, 0, Range, Origin, Used);
      std::optional<AstStmtId> ElseBlock;
      if (Statements.size() > 1)
      {
        ElseBlock = AstStmtId::fromValue(Statements[1].Index);
        Used.push_back(Statements[1]);
      }
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addStatement(Context, Header, AstKind::IfStatement, IfStatementPayload{Condition, ThenBlock, ElseBlock}), Range);
    }

    Fragment lowerWhileStatement(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      AstNodeRef Condition;
      for (const AstNodeRef Node : Value.Content.Nodes)
      {
        if (Node.Category == AstNodeCategory::Expression || Node.Category == AstNodeCategory::Pattern)
        {
          Condition = Node;
          Used.push_back(Node);
          break;
        }
      }
      if (!Condition.isValid())
      {
        const PlaceholderSite Site = placeholderSite(Value.Content, 0, Range, Origin);
        Condition = synthesizeError(Context, AstNodeCategory::Expression, Site.Range, Site.Origin);
      }
      const AstStmtId Body = requiredStatement(Context, Value.Content, nodesOfCategory(Value.Content, AstNodeCategory::Statement), 0, Range, Origin, Used);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addStatement(Context, Header, AstKind::WhileStatement, WhileStatementPayload{Condition, Body}), Range);
    }

    Fragment lowerReturnStatement(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      std::vector<AstNodeRef> Used;
      std::optional<AstExprId> Returned;
      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      if (!Expressions.empty())
      {
        Returned = AstExprId::fromValue(Expressions.front().Index);
        Used.push_back(Expressions.front());
      }
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addStatement(Context, Header, AstKind::ReturnStatement, ReturnPayload{Returned}), Range);
    }

    Fragment lowerControlStatement(Frame Value, AstKind Kind, AstContext &Context, core::SourceRange Range)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), std::move(Value.Content.Nodes));
      return nodeFragment(addStatement(Context, Header, Kind, ControlStatementPayload{}), Range);
    }

    Fragment lowerUnsupportedStatement(Frame Value, parser::CstKind Kind, AstContext &Context, core::SourceRange Range)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), {});
      const UnsupportedPayload Payload{statementFeature(Kind), Context.addList(std::move(Value.Content.Nodes))};
      return nodeFragment(addStatement(Context, Header, AstKind::UnsupportedStatement, Payload), Range);
    }

    Fragment lowerBindingPattern(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      const std::optional<std::size_t> Name = firstTokenOfKind(Value.Content, tokenizer::TokenKind::Identifier);
      if (!Name)
      {
        return lowerPlaceholder(std::move(Value), Context, Range, Origin);
      }
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), std::move(Value.Content.Nodes));
      return nodeFragment(addPattern(Context, Header, AstKind::BindingPattern, BindingPatternPayload{Strings.intern(Value.Content.Tokens[*Name].Text)}), Range);
    }

    Fragment lowerWildcardPattern(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), std::move(Value.Content.Nodes));
      return nodeFragment(addPattern(Context, Header, AstKind::WildcardPattern, WildcardPatternPayload{}), Range);
    }

    Fragment lowerTuplePattern(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const std::vector<AstNodeRef> Patterns = nodesOfCategory(Value.Content, AstNodeCategory::Pattern);
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Patterns));
      return nodeFragment(addPattern(Context, Header, AstKind::TuplePattern, TuplePatternPayload{Context.addList(Patterns)}), Range);
    }

    Fragment lowerVariantPattern(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      const std::optional<std::size_t> Name = firstTokenOfKind(Value.Content, tokenizer::TokenKind::Identifier);
      if (!Name)
      {
        return lowerPlaceholder(std::move(Value), Context, Range, Origin);
      }
      const std::vector<AstNodeRef> Patterns = nodesOfCategory(Value.Content, AstNodeCategory::Pattern);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Patterns));
      return nodeFragment(addPattern(Context, Header, AstKind::VariantPattern, VariantPatternPayload{Strings.intern(Value.Content.Tokens[*Name].Text), Context.addList(Patterns)}), Range);
    }

    Fragment lowerLiteral(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      AstLiteralKind LiteralKind = AstLiteralKind::Integer;
      std::string_view Spelling;
      for (const SyntaxToken &Token : Value.Content.Tokens)
      {
        switch (Token.Kind)
        {
        case tokenizer::TokenKind::BoolLiteral:
          LiteralKind = AstLiteralKind::Bool;
          Spelling = Token.Text;
          break;
        case tokenizer::TokenKind::NullLiteral:
          LiteralKind = AstLiteralKind::Null;
          Spelling = Token.Text;
          break;
        case tokenizer::TokenKind::IntegerLiteral:
          LiteralKind = AstLiteralKind::Integer;
          Spelling = Token.Text;
          break;
        case tokenizer::TokenKind::FloatLiteral:
          LiteralKind = AstLiteralKind::Float;
          Spelling = Token.Text;
          break;
        case tokenizer::TokenKind::ScalarLiteral:
          LiteralKind = AstLiteralKind::Scalar;
          Spelling = Token.Text;
          break;
        case tokenizer::TokenKind::StringLiteral:
          LiteralKind = AstLiteralKind::String;
          Spelling = Token.Text;
          break;
        default:
          continue;
        }
        break;
      }
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), std::move(Value.Content.Nodes));
      return nodeFragment(addExpression(Context, Header, AstKind::LiteralExpression, LiteralPayload{LiteralKind, Strings.intern(Spelling)}), Range);
    }

    Fragment lowerName(Frame Value, AstKind Kind, tokenizer::TokenKind TokenKind, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      const std::optional<std::size_t> Name = firstTokenOfKind(Value.Content, TokenKind);
      if (!Name)
      {
        return lowerPlaceholder(std::move(Value), Context, Range, Origin);
      }
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), std::move(Value.Content.Nodes));
      return nodeFragment(addExpression(Context, Header, Kind, NamePayload{Strings.intern(Value.Content.Tokens[*Name].Text)}), Range);
    }

    Fragment lowerThisExpression(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), std::move(Value.Content.Nodes));
      return nodeFragment(addExpression(Context, Header, AstKind::ThisExpression, ThisPayload{}), Range);
    }

    Fragment lowerGroupExpression(Frame Value, AstKind Kind, AstContext &Context, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      if (Kind == AstKind::TypeGroupExpression && !Expressions.empty() && !std::all_of(Expressions.begin(), Expressions.end(), [&](AstNodeRef Expression) { return isLoweredTypeExpression(Context, Expression); }))
      {
        const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), {});
        const UnsupportedPayload Payload{UnsupportedFeature::ComplexType, Context.addList(std::move(Value.Content.Nodes))};
        return nodeFragment(addExpression(Context, Header, AstKind::UnsupportedExpression, Payload), Range);
      }
      const AstExprId Expression = requiredExpression(Context, Value.Content, Expressions, 0, Range, Origin, Used);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addExpression(Context, Header, Kind, GroupPayload{Expression}), Range);
    }

    Fragment lowerUnaryExpression(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const AstExprId Operand = requiredExpression(Context, Value.Content, nodesOfCategory(Value.Content, AstNodeCategory::Expression), 0, Range, Origin, Used);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addExpression(Context, Header, AstKind::UnaryExpression, UnaryPayload{internOperator(Strings, Value.Content, Range.Start, Context.expression(Operand).Header.Range.Start), Operand}), Range);
    }

    Fragment lowerComptimeExpression(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const AstExprId Operand = requiredExpression(Context, Value.Content, nodesOfCategory(Value.Content, AstNodeCategory::Expression), 0, Range, Origin, Used);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addExpression(Context, Header, AstKind::ComptimeExpression, UnaryPayload{Strings.intern("comptime"), Operand}), Range);
    }

    Fragment lowerBinaryExpression(Frame Value, AstContext &Context, core::StringInterner &Strings, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      const AstExprId Left = requiredExpression(Context, Value.Content, Expressions, 0, Range, Origin, Used);
      const AstExprId Right = requiredExpression(Context, Value.Content, Expressions, 1, Range, Origin, Used);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addExpression(Context, Header, AstKind::BinaryExpression, BinaryPayload{Left, internOperator(Strings, Value.Content, Context.expression(Left).Header.Range.End, Context.expression(Right).Header.Range.Start), Right}), Range);
    }

    Fragment lowerCallExpression(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      const AstExprId Callee = requiredExpression(Context, Value.Content, Expressions, 0, Range, Origin, Used);
      std::vector<AstNodeRef> Arguments;
      for (std::size_t Index = 1; Index < Expressions.size(); ++Index)
      {
        Arguments.push_back(Expressions[Index]);
        Used.push_back(Expressions[Index]);
      }
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addExpression(Context, Header, AstKind::CallExpression, CallPayload{Callee, Context.addList(std::move(Arguments))}), Range);
    }

    Fragment lowerIfExpression(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      const CstOrigin Origin = nodeOrigin(Value.Id);
      std::vector<AstNodeRef> Used;
      const std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      std::vector<AstNodeRef> Conditions;
      std::vector<AstNodeRef> ThenValues;
      std::vector<AstNodeRef> ElseValues;
      const std::optional<std::size_t> OpenParenthesis = tokenIndex(Value.Content, "(");
      const std::optional<std::size_t> CloseParenthesis = OpenParenthesis ? tokenIndex(Value.Content, ")", *OpenParenthesis + 1) : std::nullopt;
      const std::optional<std::size_t> Else = CloseParenthesis ? tokenIndex(Value.Content, "else", *CloseParenthesis + 1) : std::nullopt;
      if (OpenParenthesis && CloseParenthesis && Else)
      {
        for (const AstNodeRef Expression : Expressions)
        {
          const core::SourceRange ExpressionRange = Context.node(Expression).Header->Range;
          if (Value.Content.Tokens[*OpenParenthesis].Range.End <= ExpressionRange.Start && ExpressionRange.End <= Value.Content.Tokens[*CloseParenthesis].Range.Start)
          {
            Conditions.push_back(Expression);
          }
          else if (Value.Content.Tokens[*CloseParenthesis].Range.End <= ExpressionRange.Start && ExpressionRange.End <= Value.Content.Tokens[*Else].Range.Start)
          {
            ThenValues.push_back(Expression);
          }
          else if (Value.Content.Tokens[*Else].Range.End <= ExpressionRange.Start)
          {
            ElseValues.push_back(Expression);
          }
        }
      }
      else
      {
        if (!Expressions.empty())
        {
          Conditions.push_back(Expressions[0]);
        }
        if (Expressions.size() > 1)
        {
          ThenValues.push_back(Expressions[1]);
        }
        if (Expressions.size() > 2)
        {
          ElseValues.push_back(Expressions[2]);
        }
      }
      const AstExprId Condition = requiredExpression(Context, Value.Content, Conditions, 0, Range, Origin, Used, 0);
      const AstExprId ThenValue = requiredExpression(Context, Value.Content, ThenValues, 0, Range, Origin, Used, 1);
      const AstExprId ElseValue = requiredExpression(Context, Value.Content, ElseValues, 0, Range, Origin, Used, 2);
      const AstNodeHeader Header = makeHeader(Context, Range, Origin, std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Used));
      return nodeFragment(addExpression(Context, Header, AstKind::IfExpression, IfExpressionPayload{Condition, ThenValue, ElseValue}), Range);
    }

    Fragment lowerFunctionTypeExpression(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      if (tokenIndex(Value.Content, "async"))
      {
        const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), {});
        const UnsupportedPayload Payload{UnsupportedFeature::ComplexType, Context.addList(std::move(Value.Content.Nodes))};
        return nodeFragment(addExpression(Context, Header, AstKind::UnsupportedExpression, Payload), Range);
      }
      std::vector<AstNodeRef> Expressions = nodesOfCategory(Value.Content, AstNodeCategory::Expression);
      std::vector<AstNodeRef> Parameters;
      std::optional<AstNodeRef> Result;
      std::optional<std::size_t> ArrowEnd;
      for (std::size_t Index = 1; Index < Value.Content.Tokens.size(); ++Index)
      {
        if (Value.Content.Tokens[Index - 1].Text == "-" && Value.Content.Tokens[Index].Text == ">")
        {
          ArrowEnd = Value.Content.Tokens[Index].Range.End;
          break;
        }
      }
      for (const AstNodeRef Expression : Expressions)
      {
        if (!Result && ArrowEnd && Context.node(Expression).Header->Range.Start >= *ArrowEnd)
        {
          Result = Expression;
        }
        else
        {
          Parameters.push_back(Expression);
        }
      }
      std::optional<AstExprId> ResultId;
      if (Result)
      {
        ResultId = AstExprId::fromValue(Result->Index);
      }
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), supplementalNodes(Value.Content.Nodes, Expressions));
      return nodeFragment(addExpression(Context, Header, AstKind::FunctionTypeExpression, FunctionTypePayload{Context.addList(std::move(Parameters)), ResultId}), Range);
    }

    Fragment lowerTypeSyntax(Frame Value, AstContext &Context, core::SourceRange Range)
    {
      if (Value.Content.Nodes.empty())
      {
        const PlaceholderSite Site = placeholderSite(Value.Content, 0, Range, nodeOrigin(Value.Id));
        return lowerPlaceholder(std::move(Value), Context, Site.Range, Site.Origin);
      }
      if (!tokenIndex(Value.Content, "const"))
      {
        return std::move(Value.Content);
      }
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), {});
      const UnsupportedPayload Payload{UnsupportedFeature::ComplexType, Context.addList(std::move(Value.Content.Nodes))};
      return nodeFragment(addExpression(Context, Header, AstKind::UnsupportedExpression, Payload), Range);
    }

    Fragment lowerUnsupportedExpression(Frame Value, parser::CstKind Kind, AstContext &Context, core::SourceRange Range)
    {
      const AstNodeHeader Header = makeHeader(Context, Range, nodeOrigin(Value.Id), std::move(Value.Content.Recoveries), {});
      const UnsupportedPayload Payload{expressionFeature(Kind), Context.addList(std::move(Value.Content.Nodes))};
      return nodeFragment(addExpression(Context, Header, AstKind::UnsupportedExpression, Payload), Range);
    }

    Fragment lowerFrame(Frame Value, parser::CstKind Kind, const parser::CstTree &Tree, const tokenizer::TokenizedBuffer &LexedFile, AstContext &Context, core::StringInterner &Strings)
    {
      const CstDisposition Disposition = disposition(Kind);
      if (Disposition == CstDisposition::Fold)
      {
        return std::move(Value.Content);
      }
      core::SourceRange Range = fragmentRange(Value.Content, frameAnchor(Value, LexedFile));
      if (Disposition == CstDisposition::SourceFile)
      {
        Range = {0, LexedFile.source().size()};
      }
      switch (Disposition)
      {
      case CstDisposition::Fold:
        break;
      case CstDisposition::SourceFile:
        return lowerSourceFile(std::move(Value), Context, Range);
      case CstDisposition::Error:
        return lowerError(std::move(Value), Context, Strings, Range);
      case CstDisposition::Import:
        return lowerImport(std::move(Value), Context, Strings, Range);
      case CstDisposition::Binding:
        return lowerBinding(std::move(Value), Kind, Context, Strings, Range);
      case CstDisposition::Parameter:
        return lowerParameter(std::move(Value), Kind, Context, Strings, Range);
      case CstDisposition::Function:
        return lowerFunction(std::move(Value), Kind, Tree, Context, Strings, Range);
      case CstDisposition::UnsupportedDeclaration:
        return lowerUnsupportedDeclaration(std::move(Value), Kind, Context, Range);
      case CstDisposition::ContextualComptime:
        return lowerContextualComptime(std::move(Value), Context, Range);
      case CstDisposition::Block:
        return lowerBlock(std::move(Value), Context, Range);
      case CstDisposition::Assignment:
        return lowerAssignment(std::move(Value), Context, Strings, Range);
      case CstDisposition::ExpressionStatement:
        return lowerExpressionStatement(std::move(Value), Context, Range);
      case CstDisposition::IfStatement:
        return lowerIfStatement(std::move(Value), Context, Range);
      case CstDisposition::WhileStatement:
        return lowerWhileStatement(std::move(Value), Context, Range);
      case CstDisposition::ReturnStatement:
        return lowerReturnStatement(std::move(Value), Context, Range);
      case CstDisposition::BreakStatement:
        return lowerControlStatement(std::move(Value), AstKind::BreakStatement, Context, Range);
      case CstDisposition::ContinueStatement:
        return lowerControlStatement(std::move(Value), AstKind::ContinueStatement, Context, Range);
      case CstDisposition::UnsupportedStatement:
        return lowerUnsupportedStatement(std::move(Value), Kind, Context, Range);
      case CstDisposition::BindingPattern:
        return lowerBindingPattern(std::move(Value), Context, Strings, Range);
      case CstDisposition::WildcardPattern:
        return lowerWildcardPattern(std::move(Value), Context, Range);
      case CstDisposition::TuplePattern:
        return lowerTuplePattern(std::move(Value), Context, Range);
      case CstDisposition::VariantPattern:
        return lowerVariantPattern(std::move(Value), Context, Strings, Range);
      case CstDisposition::Literal:
        return lowerLiteral(std::move(Value), Context, Strings, Range);
      case CstDisposition::Name:
        return lowerName(std::move(Value), AstKind::NameExpression, tokenizer::TokenKind::Identifier, Context, Strings, Range);
      case CstDisposition::TypeName:
      {
        const tokenizer::TokenKind NameKind = firstTokenOfKind(Value.Content, tokenizer::TokenKind::BuiltinType) ? tokenizer::TokenKind::BuiltinType : tokenizer::TokenKind::Identifier;
        const AstKind AstNameKind = NameKind == tokenizer::TokenKind::BuiltinType ? AstKind::BuiltinTypeExpression : AstKind::TypeNameExpression;
        return lowerName(std::move(Value), AstNameKind, NameKind, Context, Strings, Range);
      }
      case CstDisposition::BuiltinType:
        return lowerName(std::move(Value), AstKind::BuiltinTypeExpression, tokenizer::TokenKind::BuiltinType, Context, Strings, Range);
      case CstDisposition::ThisExpression:
        return lowerThisExpression(std::move(Value), Context, Range);
      case CstDisposition::GroupExpression:
        return lowerGroupExpression(std::move(Value), AstKind::GroupExpression, Context, Range);
      case CstDisposition::TypeGroupExpression:
        return lowerGroupExpression(std::move(Value), AstKind::TypeGroupExpression, Context, Range);
      case CstDisposition::UnaryExpression:
        return lowerUnaryExpression(std::move(Value), Context, Strings, Range);
      case CstDisposition::BinaryExpression:
        return lowerBinaryExpression(std::move(Value), Context, Strings, Range);
      case CstDisposition::CallExpression:
        return lowerCallExpression(std::move(Value), Context, Range);
      case CstDisposition::IfExpression:
        return lowerIfExpression(std::move(Value), Context, Range);
      case CstDisposition::ComptimeExpression:
        return lowerComptimeExpression(std::move(Value), Context, Strings, Range);
      case CstDisposition::FunctionTypeExpression:
        return lowerFunctionTypeExpression(std::move(Value), Context, Range);
      case CstDisposition::TypeSyntax:
        return lowerTypeSyntax(std::move(Value), Context, Range);
      case CstDisposition::UnsupportedExpression:
        return lowerUnsupportedExpression(std::move(Value), Kind, Context, Range);
      }
      throw std::logic_error("CST lowering disposition was not handled");
    }
  } // namespace

  CstLowering::CstLowering(AstContext &Context, core::StringInterner &Strings) noexcept : Context(Context), Strings(Strings)
  {
  }

  AstFile CstLowering::lower(const parser::ParsedFile &ParsedFile, core::SourceFileId SourceFile)
  {
    if (!SourceFile.isValid() || ParsedFile.sourceFileId() != SourceFile)
    {
      throw std::invalid_argument("CST lowering requires the ParsedFile's valid SourceFileId");
    }

    const parser::CstTree &Tree = ParsedFile.cst();
    const tokenizer::TokenizedBuffer &LexedFile = ParsedFile.lexedFile();
    if (Tree.nodes().empty() || Tree.root() >= Tree.nodes().size())
    {
      throw std::logic_error("CST has no valid root node");
    }
    if (Tree.nodes().size() > CstOrigin::InvalidValue)
    {
      throw std::length_error("CST node table exceeds the origin ID range");
    }

    std::vector<std::uint8_t> States(Tree.nodes().size(), 0);
    std::vector<Frame> Work;
    const parser::CstNode &RootNode = Tree.node(Tree.root());
    States[Tree.root()] = 1;
    Work.push_back({Tree.root(), 0, categoryFor(disposition(RootNode.Kind), AstNodeCategory::Declaration)});
    AstNodeRef Root;

    while (!Work.empty())
    {
      Frame &Current = Work.back();
      const parser::CstNode &CstNode = Tree.node(Current.Id);
      if (CstNode.FirstChild > Tree.children().size() || CstNode.ChildCount > Tree.children().size() - CstNode.FirstChild)
      {
        throw std::logic_error("CST node child range is out of bounds");
      }

      if (Current.NextChild < CstNode.ChildCount)
      {
        const std::size_t ElementOrdinal = Current.NextChild++;
        const parser::CstElement &Element = Tree.children()[CstNode.FirstChild + ElementOrdinal];
        if (const parser::CstNodeRef *Child = std::get_if<parser::CstNodeRef>(&Element))
        {
          if (Child->Id >= Tree.nodes().size())
          {
            throw std::logic_error("CST node reference is out of bounds");
          }
          if (States[Child->Id] != 0)
          {
            throw std::logic_error(States[Child->Id] == 1 ? "CST contains a node-reference cycle" : "CST node is reachable more than once");
          }
          const parser::CstNode &ChildNode = Tree.node(Child->Id);
          if (ChildNode.TokenCount > std::numeric_limits<std::size_t>::max() - Current.ConsumedTokens)
          {
            throw std::overflow_error("CST token count overflows its owner");
          }
          const std::size_t ChildStartToken = Current.NodeStartToken + Current.ConsumedTokens;
          Current.ConsumedTokens += ChildNode.TokenCount;
          States[Child->Id] = 1;
          Work.push_back({Child->Id, ChildStartToken, categoryFor(disposition(ChildNode.Kind), Current.Category)});
          continue;
        }

        if (const parser::CstTokenRef *TokenReference = std::get_if<parser::CstTokenRef>(&Element))
        {
          if (TokenReference->TokenOffset != Current.ConsumedTokens)
          {
            throw std::logic_error("CST token offset does not match child order");
          }
          if (Current.NodeStartToken > std::numeric_limits<std::size_t>::max() - TokenReference->TokenOffset)
          {
            throw std::overflow_error("CST token index overflows");
          }
          const std::size_t TokenIndex = Current.NodeStartToken + TokenReference->TokenOffset;
          if (TokenIndex >= LexedFile.tokens().size())
          {
            throw std::logic_error("CST token reference is out of bounds");
          }
          const tokenizer::Token &Token = LexedFile.tokens()[TokenIndex];
          ++Current.ConsumedTokens;
          includeRange(Current.Content, Token.Span);
          if (!Token.isTrivia() && Token.Kind != tokenizer::TokenKind::Utf8Bom && Token.Kind != tokenizer::TokenKind::EndOfFile)
          {
            Current.Content.Tokens.push_back({Token.Kind, Token.Span, LexedFile.raw(Token)});
          }
          continue;
        }

        const parser::MissingToken &Missing = std::get<parser::MissingToken>(Element);
        if (Missing.AnchorByteOffset > LexedFile.source().size())
        {
          throw std::logic_error("CST missing-token anchor is outside the source file");
        }
        const AstExpectedKind ExpectedKind = expectedKind(Missing.ExpectedKind);
        if (ExpectedKind == AstExpectedKind::Unknown)
        {
          throw std::logic_error("CST missing token has no representable expected kind");
        }
        const core::SourceRange MissingRange{Missing.AnchorByteOffset, Missing.AnchorByteOffset};
        includeRange(Current.Content, MissingRange);
        const std::string ExpectedSpelling = Missing.ExpectedSpelling.empty() ? tokenizer::tokenKindName(Missing.ExpectedKind) : Missing.ExpectedSpelling;
        Current.Content.Recoveries.push_back({AstRecoveryKind::MissingToken, MissingRange, elementOrigin(Current.Id, ElementOrdinal), ExpectedKind, Strings.intern(ExpectedSpelling)});
        continue;
      }

      if (Current.ConsumedTokens != CstNode.TokenCount)
      {
        throw std::logic_error("CST node token count does not match its children");
      }
      Frame CompletedFrame = std::move(Current);
      States[CompletedFrame.Id] = 2;
      Work.pop_back();
      Fragment Completed = lowerFrame(std::move(CompletedFrame), CstNode.Kind, Tree, LexedFile, Context, Strings);
      if (Work.empty())
      {
        if (Completed.Nodes.size() != 1)
        {
          throw std::logic_error("CST root did not lower to exactly one AST node");
        }
        Root = Completed.Nodes.front();
      }
      else
      {
        appendFragment(Work.back().Content, std::move(Completed));
      }
    }

    if (Root.Category != AstNodeCategory::Declaration)
    {
      throw std::logic_error("CST source root did not lower to an AST declaration");
    }
    if (std::any_of(States.begin(), States.end(), [](std::uint8_t State)
    {
      return State != 2;
    }))
    {
      throw std::logic_error("CST contains a node that is unreachable from the root");
    }

    std::vector<std::uint32_t> CstChildCounts;
    CstChildCounts.reserve(Tree.nodes().size());
    for (const parser::CstNode &Node : Tree.nodes())
    {
      if (Node.ChildCount > CstOrigin::InvalidValue)
      {
        throw std::length_error("CST child count exceeds the AST origin range");
      }
      CstChildCounts.push_back(static_cast<std::uint32_t>(Node.ChildCount));
    }
    return Context.createFile(SourceFile, AstDeclId::fromValue(Root.Index), LexedFile.source().size(), Strings, std::move(CstChildCounts));
  }

  AstFile lowerCst(const parser::ParsedFile &ParsedFile, core::SourceFileId SourceFile, AstContext &Context, core::StringInterner &Strings)
  {
    return CstLowering(Context, Strings).lower(ParsedFile, SourceFile);
  }
} // namespace ink::ast
