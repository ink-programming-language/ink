#include "parser_internal.h"
namespace ink::parser
{
  TypeSyntax *Parser::parseTypeSyntax()
  {
    const std::size_t Start = start();
    Expr *Value = parseUnary(true);
    return make<TypeSyntax>(range(Start), Value);
  }
  ASTArray<Parameter> Parser::parseGenericParameters()
  {
    if (!take(TokenKind::LBracket))
    {
      return {};
    }
    return parseParameters(TokenKind::RBracket, false);
  }
  ASTArray<Parameter> Parser::parseParameters(TokenKind End, bool AllowEmpty, bool Payload)
  {
    RecoveryScope Scope(*this, End, true);
    std::vector<Parameter> Parameters;
    while (!listEnd(End) && active())
    {
      const TokenId Before = Cursor.position();
      const std::size_t Start = start();
      const NameToken Name = name();
      expect(TokenKind::Colon);
      TypeSyntax *Type = parseTypeSyntax();
      Expr *Default = nullptr;
      bool Variadic = false;
      if (take(TokenKind::Assign))
      {
        Default = parseExpr();
        if (Payload)
        {
          invalid(range(Start));
          Default = nullptr;
        }
      }
      if (take(TokenKind::Ellipsis))
      {
        Variadic = true;
        if (Default || Payload)
        {
          invalid(range(Start));
          Variadic = false;
        }
      }
      Parameters.emplace_back(Name, Type, Default, Variadic, range(Start));
      if (Cursor.position() == Before && !at(TokenKind::Comma) && !listEnd(End))
      {
        ensureProgress(Before);
      }
      if (!nextElement(End))
      {
        break;
      }
    }
    if (!AllowEmpty && Parameters.empty())
    {
      invalid(point());
      auto *Missing = make<MissingExpr>(point());
      auto *Type = make<TypeSyntax>(point(), Missing);
      Parameters.emplace_back(NameToken{InvalidTokenId, {}, point()}, Type, nullptr, false, point());
    }
    expect(End);
    return array(Parameters);
  }
  Expr *Parser::parseFunction(bool TypeOnly)
  {
    const std::size_t Start = start();
    bump();
    const bool HasGenerics = at(TokenKind::LBracket);
    const auto Generics = parseGenericParameters();
    if (TypeOnly && HasGenerics)
    {
      invalid(range(Start));
    }
    expect(TokenKind::LParen);
    struct HeaderParameter
    {
        std::optional<NameToken> Name;
        TypeSyntax *Type;
        Expr *Default;
        bool Variadic;
        SourceRange Range;
    };
    std::vector<HeaderParameter> Parameters;
    bool HasDefault = false;
    {
      RecoveryScope Scope(*this, TokenKind::RParen, true);
      while (!listEnd(TokenKind::RParen) && active())
      {
        const TokenId Before = Cursor.position();
        const std::size_t ParameterStart = start();
        std::optional<NameToken> Name;
        if (at(TokenKind::Identifier) && kind(1) == TokenKind::Colon)
        {
          Name = name();
          bump();
        }
        TypeSyntax *Type = parseTypeSyntax();
        Expr *Default = nullptr;
        if (take(TokenKind::Assign))
        {
          Default = parseExpr();
          HasDefault = true;
        }
        bool Variadic = take(TokenKind::Ellipsis);
        if (Default && Variadic)
        {
          invalid(range(ParameterStart));
          Variadic = false;
        }
        Parameters.push_back({Name, Type, Default, Variadic, range(ParameterStart)});
        if (Cursor.position() == Before && !at(TokenKind::Comma) && !listEnd(TokenKind::RParen))
        {
          ensureProgress(Before);
        }
        if (!nextElement(TokenKind::RParen))
        {
          break;
        }
      }
      expect(TokenKind::RParen);
    }
    TypeSyntax *ReturnType = nullptr;
    if (take(TokenKind::Colon))
    {
      ReturnType = parseTypeSyntax();
    }
    const bool Lambda = !TypeOnly && (HasGenerics || HasDefault || ReturnType == nullptr || at(TokenKind::LBrace));
    if (Lambda)
    {
      std::vector<Parameter> Ordinary;
      for (const auto &Parameter : Parameters)
      {
        if (!Parameter.Name)
        {
          invalid(Parameter.Range);
        }
        Ordinary.emplace_back(Parameter.Name.value_or(NameToken{InvalidTokenId, {}, SourceRange(Parameter.Range.getBegin())}), Parameter.Type, Parameter.Default, Parameter.Variadic, Parameter.Range);
      }
      BlockStmt *Body = parseBlock();
      return parsePostfix(make<LambdaExpr>(range(Start), Generics, array(Ordinary), ReturnType, Body));
    }
    std::vector<FunctionTypeParameter> TypeParameters;
    for (const auto &Parameter : Parameters)
    {
      if (Parameter.Default)
      {
        invalid(Parameter.Range);
      }
      TypeParameters.emplace_back(Parameter.Name, Parameter.Type, Parameter.Variadic, Parameter.Range);
    }
    if (!ReturnType)
    {
      expect(TokenKind::Colon);
      ReturnType = parseTypeSyntax();
    }
    return make<FunctionTypeExpr>(range(Start), array(TypeParameters), ReturnType);
  }
} // namespace ink::parser
