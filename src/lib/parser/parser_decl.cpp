#include "parser_internal.h"
namespace ink::parser
{
  ASTArray<NameToken> Parser::parseQualifiedName()
  {
    std::vector<NameToken> Parts;
    Parts.push_back(name());
    while (take(TokenKind::Dot) && active())
    {
      Parts.push_back(name());
    }
    return array(Parts);
  }
  bool Parser::attributesAhead()
  {
    std::size_t Ahead = at(TokenKind::KwComptime) ? 1 : 0;
    if (kind(Ahead) != TokenKind::LBracket)
    {
      return false;
    }
    std::vector<TokenKind> Ends{TokenKind::RBracket};
    for (++Ahead; active(); ++Ahead)
    {
      const TokenKind Kind = kind(Ahead);
      if (Kind == TokenKind::EndOfFile)
      {
        return false;
      }
      if (Ends.size() == 1 && declarationStart(Ahead))
      {
        return true;
      }
      if (Kind == TokenKind::LParen)
      {
        Ends.push_back(TokenKind::RParen);
      }
      else if (Kind == TokenKind::LBracket)
      {
        Ends.push_back(TokenKind::RBracket);
      }
      else if (Kind == TokenKind::LBrace)
      {
        Ends.push_back(TokenKind::RBrace);
      }
      else if (Kind == TokenKind::RParen || Kind == TokenKind::RBracket || Kind == TokenKind::RBrace)
      {
        if (Ends.back() != Kind)
        {
          return false;
        }
        Ends.pop_back();
        if (Ends.empty())
        {
          return declarationStart(Ahead + 1);
        }
      }
      else if (Ends.size() == 1 && Kind == TokenKind::Semicolon)
      {
        return false;
      }
    }
    return false;
  }
  ASTArray<Attribute> Parser::parseAttributes()
  {
    if (!take(TokenKind::LBracket))
    {
      return {};
    }
    RecoveryScope Scope(*this, TokenKind::RBracket, true);
    std::vector<Attribute> Attributes;
    if (at(TokenKind::RBracket))
    {
      invalid(point());
    }
    while (!listEnd(TokenKind::RBracket) && active())
    {
      const std::size_t Start = start();
      const TokenId Before = Cursor.position();
      const auto Path = parseQualifiedName();
      AttributeSuffix Suffix = AttributeSuffix::None;
      ASTArray<Argument> Arguments;
      Expr *Value = nullptr;
      if (take(TokenKind::LParen))
      {
        Suffix = AttributeSuffix::Arguments;
        Arguments = parseArguments(TokenKind::RParen);
      }
      else if (take(TokenKind::Assign))
      {
        Suffix = AttributeSuffix::Value;
        Value = parseExpr();
      }
      Attributes.emplace_back(Path, Suffix, Arguments, Value, range(Start));
      if (Before == Cursor.position() && !at(TokenKind::Comma) && !listEnd(TokenKind::RBracket))
      {
        ensureProgress(Before);
      }
      if (!nextElement(TokenKind::RBracket))
      {
        break;
      }
    }
    expect(TokenKind::RBracket);
    return array(Attributes);
  }
  VarDecl *Parser::parseVar(ASTArray<Attribute> Attributes)
  {
    const std::size_t Start = Attributes.empty() ? start() : Attributes.front().range().getBegin().getByteOffset();
    const bool Constant = take(TokenKind::KwConst);
    if (!Constant)
    {
      expect(TokenKind::KwVar);
    }
    BindingPattern *Binding = parseBindingPattern();
    TypeSyntax *Type = nullptr;
    if (take(TokenKind::Colon))
    {
      Type = parseTypeSyntax();
    }
    Expr *Initializer = nullptr;
    if (take(TokenKind::Assign))
    {
      Initializer = parseExpr();
    }
    else if (Constant || !isa<NameBindingPattern>(Binding))
    {
      expect(TokenKind::Assign);
      Initializer = missingExpr();
    }
    return make<VarDecl>(range(Start), Attributes, Constant, Binding, Type, Initializer, Initializer ? VarDeclForm::Initialized : VarDeclForm::Uninitialized);
  }
  Decl *Parser::parseDecl(ASTArray<Attribute> Attributes)
  {
    const std::size_t Start = Attributes.empty() ? start() : Attributes.front().range().getBegin().getByteOffset();
    if (at(TokenKind::KwVar) || at(TokenKind::KwConst))
    {
      VarDecl *Result = parseVar(Attributes);
      expect(TokenKind::Semicolon);
      return Result;
    }
    if (take(TokenKind::KwFunc))
    {
      const NameToken Name = name();
      const auto Generics = parseGenericParameters();
      expect(TokenKind::LParen);
      const auto Parameters = parseParameters(TokenKind::RParen, true);
      expect(TokenKind::Colon);
      TypeSyntax *ReturnType = parseTypeSyntax();
      const bool Forward = take(TokenKind::Semicolon);
      BlockStmt *Body = Forward ? nullptr : parseBlock();
      return make<FunctionDecl>(range(Start), Attributes, Name, Generics, Parameters, ReturnType, Forward ? FunctionBodyKind::DeclarationOnly : FunctionBodyKind::Definition, Body);
    }
    if (take(TokenKind::KwField))
    {
      const NameToken Name = name();
      FieldTailKind Tail = FieldTailKind::None;
      TypeSyntax *Type = nullptr;
      Expr *Initializer = nullptr;
      ASTArray<Parameter> Payload;
      if (take(TokenKind::Colon))
      {
        Tail = FieldTailKind::Typed;
        Type = parseTypeSyntax();
      }
      else if (take(TokenKind::LParen))
      {
        Tail = FieldTailKind::Payload;
        Payload = parseParameters(TokenKind::RParen, false, true);
      }
      if (take(TokenKind::Assign))
      {
        if (Tail == FieldTailKind::None)
        {
          Tail = FieldTailKind::InitializerOnly;
        }
        Initializer = parseExpr();
      }
      expect(TokenKind::Semicolon);
      return make<FieldDecl>(range(Start), Attributes, Name, Tail, Type, Payload, Initializer);
    }
    if (at(TokenKind::KwClass) || at(TokenKind::KwEnum) || at(TokenKind::KwInterface))
    {
      const TokenKind Kind = Input.token(bump()).Kind;
      const NameToken Name = name();
      const auto Generics = parseGenericParameters();
      std::vector<BaseSpec> Bases;
      if (take(TokenKind::Colon))
      {
        do
        {
          const std::size_t BaseStart = start();
          const bool Implements = take(TokenKind::KwImplements);
          TypeSyntax *Type = parseTypeSyntax();
          Bases.emplace_back(Implements, Type, range(BaseStart));
        } while (take(TokenKind::Comma) && active());
      }
      const bool Forward = Kind == TokenKind::KwClass && Bases.empty() && at(TokenKind::Semicolon);
      BlockStmt *Body = Forward ? nullptr : parseBlock();
      const ExpectResult Semicolon = expect(TokenKind::Semicolon);
      const AggregateForm Form = Forward ? AggregateForm::Forward : AggregateForm::Definition;
      if (Kind == TokenKind::KwClass)
      {
        return make<ClassDecl>(range(Start), Attributes, Name, Generics, array(Bases), Form, Body, Semicolon.Range);
      }
      if (Kind == TokenKind::KwEnum)
      {
        return make<EnumDecl>(range(Start), Attributes, Name, Generics, array(Bases), Form, Body, Semicolon.Range);
      }
      return make<InterfaceDecl>(range(Start), Attributes, Name, Generics, array(Bases), Form, Body, Semicolon.Range);
    }
    invalid(point());
    if (boundary(kind()) || declarationStart())
    {
      return make<MissingDecl>(range(Start), Attributes);
    }
    bump();
    return make<ErrorDecl>(range(Start), Attributes);
  }
  Stmt *Parser::parseImport()
  {
    const std::size_t Start = start();
    const bool From = take(TokenKind::KwFrom);
    ASTArray<NameToken> Path;
    std::vector<TokenId> RelativeTokens;
    std::size_t RelativeLevel = 0;
    if (From)
    {
      while ((at(TokenKind::Dot) || at(TokenKind::Ellipsis)) && active())
      {
        RelativeLevel += at(TokenKind::Dot) ? 1 : 3;
        RelativeTokens.push_back(bump());
      }
      if (at(TokenKind::Identifier) || RelativeLevel == 0)
      {
        Path = parseQualifiedName();
      }
    }
    expect(TokenKind::KwImport);
    RecoveryScope Scope(*this, TokenKind::Semicolon, true);
    std::vector<ImportEntry> Entries;
    do
    {
      const std::size_t EntryStart = start();
      ASTArray<NameToken> EntryPath;
      if (From)
      {
        const std::vector<NameToken> Single{name()};
        EntryPath = array(Single);
      }
      else
      {
        EntryPath = parseQualifiedName();
      }
      std::optional<NameToken> Alias;
      if (take(TokenKind::KwAs))
      {
        Alias = name();
      }
      Entries.emplace_back(EntryPath, Alias, range(EntryStart));
    } while (take(TokenKind::Comma) && active());
    expect(TokenKind::Semicolon);
    if (From)
    {
      return make<FromImportStmt>(range(Start), RelativeLevel, array(RelativeTokens), Path, array(Entries));
    }
    return make<DirectImportStmt>(range(Start), array(Entries));
  }
} // namespace ink::parser
