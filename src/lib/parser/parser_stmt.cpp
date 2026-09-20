#include "parser_internal.h"
namespace ink::parser
{
  namespace
  {
    bool assignment(TokenKind Kind)
    {
      switch (Kind)
      {
      case TokenKind::Assign:
      case TokenKind::PlusAssign:
      case TokenKind::MinusAssign:
      case TokenKind::StarAssign:
      case TokenKind::SlashAssign:
      case TokenKind::PercentAssign:
      case TokenKind::ShiftLeftAssign:
      case TokenKind::ShiftRightAssign:
      case TokenKind::AmpAssign:
      case TokenKind::CaretAssign:
      case TokenKind::PipeAssign:
        return true;
      default:
        return false;
      }
    }
  } // namespace
  SimpleItem *Parser::parseSimpleItem()
  {
    struct Segment
    {
        Expr *Left;
        TokenKind Op;
        SourceRange OperatorRange;
    };
    std::vector<Segment> Segments;
    Expr *Value = parseExpr();
    while (assignment(kind()) && active())
    {
      const TokenId Operator = bump();
      Segments.push_back({Value, Input.token(Operator).Kind, Input.token(Operator).Span});
      Value = parseExpr();
    }
    SimpleItem *Result = make<ExprItem>(Value->getSourceRange(), Value);
    for (auto It = Segments.rbegin(); It != Segments.rend(); ++It)
    {
      Result = make<AssignmentItem>(SourceRange(It->Left->getLocation(), Result->getSourceRange().getEnd()), It->Left, It->Op, It->OperatorRange, Result);
    }
    return Result;
  }
  ASTArray<SimpleItem *> Parser::parseSimpleList(TokenKind End)
  {
    RecoveryScope Scope(*this, End, true);
    std::vector<SimpleItem *> Items;
    Items.push_back(parseSimpleItem());
    while (take(TokenKind::Comma) && active())
    {
      Items.push_back(parseSimpleItem());
    }
    return array(Items);
  }
  Stmt *Parser::parseSimpleStmt(bool Semicolon)
  {
    const std::size_t Start = start();
    const auto Items = parseSimpleList(TokenKind::Semicolon);
    if (Semicolon)
    {
      expect(TokenKind::Semicolon);
    }
    return make<SimpleStmt>(range(Start), Items);
  }
  BlockStmt *Parser::parseBlock()
  {
    const std::size_t Start = start();
    DepthScope Nesting(*this);
    if (!Nesting)
    {
      return make<BlockStmt>(point(), ASTArray<Stmt *>{}, true);
    }
    const bool Synthetic = expect(TokenKind::LBrace).Status != ExpectStatus::Matched;
    std::vector<Stmt *> Statements;
    RecoveryScope Scope(*this, TokenKind::RBrace);
    if (Synthetic)
    {
      // Preserve a recognizable body statement, but leave a following declaration
      // to its owner when a callable's entire body is absent.
      if (!boundary(kind()) && !declarationStart() && !at(TokenKind::EndOfFile))
      {
        Statements.push_back(parseStmt());
      }
      return make<BlockStmt>(range(Start), array(Statements), true);
    }
    while (!at(TokenKind::RBrace) && !at(TokenKind::EndOfFile) && !at(TokenKind::KwElse) && !at(TokenKind::KwCase) && !at(TokenKind::KwDefault) && active())
    {
      const TokenId Before = Cursor.position();
      Statements.push_back(parseStmt());
      ensureProgress(Before);
    }
    expect(TokenKind::RBrace);
    return make<BlockStmt>(range(Start), array(Statements), false);
  }
  Stmt *Parser::parseStmt()
  {
    const std::size_t Start = start();
    DepthScope Nesting(*this);
    if (!Nesting)
    {
      return make<MissingStmt>(point());
    }
    if (at(TokenKind::KwComptime))
    {
      const TokenKind Next = kind(1);
      const bool Modified = declarationStart(1) || Next == TokenKind::LBrace || Next == TokenKind::KwImport || Next == TokenKind::KwFrom || Next == TokenKind::KwIf || Next == TokenKind::KwWhile || Next == TokenKind::KwFor || Next == TokenKind::KwSwitch || (Next == TokenKind::LBracket && attributesAhead());
      if (Modified)
      {
        const SourceRange Keyword = Input.token(bump()).Span;
        Stmt *Body = parseStmt();
        return make<ComptimeStmt>(range(Start), Keyword, Body);
      }
    }
    if (declarationStart() || (at(TokenKind::LBracket) && attributesAhead()))
    {
      const auto Attributes = parseAttributes();
      Decl *Declaration = parseDecl(Attributes);
      return make<DeclStmt>(range(Start), Declaration);
    }
    if (at(TokenKind::LBrace))
    {
      return parseBlock();
    }
    if (at(TokenKind::KwIf) || at(TokenKind::KwWhile))
    {
      const bool If = take(TokenKind::KwIf);
      if (!If)
      {
        bump();
      }
      expect(TokenKind::LParen);
      Expr *Condition;
      {
        RecoveryScope Scope(*this, TokenKind::RParen);
        Condition = parseExpr();
        expect(TokenKind::RParen);
      }
      Stmt *Body;
      {
        RecoveryScope Scope(*this, TokenKind::KwElse);
        Body = parseStmt();
      }
      if (!If)
      {
        return make<WhileStmt>(range(Start), Condition, Body);
      }
      Stmt *Else = nullptr;
      if (take(TokenKind::KwElse))
      {
        Else = parseStmt();
      }
      return make<IfStmt>(range(Start), Condition, Body, Else);
    }
    if (at(TokenKind::KwFor))
    {
      return parseFor();
    }
    if (at(TokenKind::KwSwitch))
    {
      return parseSwitch();
    }
    if (at(TokenKind::KwImport) || at(TokenKind::KwFrom))
    {
      return parseImport();
    }
    if (take(TokenKind::KwReturn))
    {
      Expr *Value = nullptr;
      if (!at(TokenKind::Semicolon) && !boundary(kind()) && !strongStart(kind()))
      {
        Value = parseExpr();
      }
      expect(TokenKind::Semicolon);
      return make<ReturnStmt>(range(Start), Value);
    }
    if (take(TokenKind::KwYield))
    {
      const bool Return = take(TokenKind::KwReturn);
      Expr *Value = parseExpr();
      expect(TokenKind::Semicolon);
      return make<YieldStmt>(range(Start), Return, Value);
    }
    if (at(TokenKind::KwBreak) || at(TokenKind::KwContinue))
    {
      const bool Break = take(TokenKind::KwBreak);
      if (!Break)
      {
        bump();
      }
      expect(TokenKind::Semicolon);
      if (Break)
      {
        return make<BreakStmt>(range(Start));
      }
      return make<ContinueStmt>(range(Start));
    }
    if (take(TokenKind::KwDefer))
    {
      Stmt *Body = at(TokenKind::LBrace) ? parseBlock() : parseSimpleStmt();
      return make<DeferStmt>(range(Start), Body);
    }
    if (at(TokenKind::EndOfFile) || (!Recovery.Scopes.empty() && boundary(kind()) && !at(TokenKind::Semicolon)))
    {
      invalid(point());
      LastEnd = std::max(LastEnd, start());
      return make<MissingStmt>(point());
    }
    if (at(TokenKind::Semicolon) || boundary(kind()))
    {
      invalid(Cursor.peek().Span);
      bump();
      return make<ErrorStmt>(range(Start));
    }
    return parseSimpleStmt();
  }
  Stmt *Parser::parseFor()
  {
    const std::size_t Start = start();
    bump();
    expect(TokenKind::LParen);
    BindingPattern *Binding = nullptr;
    Expr *Iterable = nullptr;
    Stmt *Initializer = nullptr;
    Expr *Condition = nullptr;
    ASTArray<SimpleItem *> Step;
    {
      RecoveryScope Scope(*this, TokenKind::RParen);
      {
        ParseTransaction Trial(*this);
        BindingPattern *Candidate = parseBindingPattern();
        if (!TrialFailed && take(TokenKind::KwIn))
        {
          Binding = Candidate;
          Trial.commit();
        }
      }
      if (Binding)
      {
        Iterable = parseExpr();
      }
      else
      {
        if (!at(TokenKind::Semicolon))
        {
          if (at(TokenKind::KwVar) || at(TokenKind::KwConst))
          {
            VarDecl *Declaration = parseVar({});
            Initializer = make<DeclStmt>(Declaration->getSourceRange(), Declaration);
          }
          else
          {
            Initializer = parseSimpleStmt(false);
          }
        }
        expect(TokenKind::Semicolon);
        if (!at(TokenKind::Semicolon) && !at(TokenKind::RParen))
        {
          Condition = parseExpr();
        }
        expect(TokenKind::Semicolon);
        if (!at(TokenKind::RParen) && !boundary(kind()))
        {
          Step = parseSimpleList(TokenKind::RParen);
        }
      }
      expect(TokenKind::RParen);
    }
    Stmt *Body = parseStmt();
    if (Binding)
    {
      return make<ForInStmt>(range(Start), Binding, Iterable, Body);
    }
    return make<ClassicForStmt>(range(Start), Initializer, Condition, Step, Body);
  }
  Stmt *Parser::parseSwitch()
  {
    const std::size_t Start = start();
    bump();
    expect(TokenKind::LParen);
    Expr *Value;
    {
      RecoveryScope Scope(*this, TokenKind::RParen);
      Value = parseExpr();
      expect(TokenKind::RParen);
    }
    expect(TokenKind::LBrace);
    RecoveryScope Scope(*this, TokenKind::RBrace);
    std::vector<SwitchClause> Clauses;
    while (!at(TokenKind::RBrace) && !at(TokenKind::EndOfFile) && !at(TokenKind::KwElse) && active())
    {
      const std::size_t ClauseStart = start();
      const bool Default = take(TokenKind::KwDefault);
      if (!Default && !take(TokenKind::KwCase))
      {
        invalid(Cursor.peek().Span);
        bump();
        continue;
      }
      Expr *Label = nullptr;
      {
        RecoveryScope LabelScope(*this, TokenKind::Colon);
        if (!Default)
        {
          Label = parseExpr();
        }
        expect(TokenKind::Colon);
      }
      std::vector<Stmt *> Statements;
      while (!at(TokenKind::KwCase) && !at(TokenKind::KwDefault) && !at(TokenKind::RBrace) && !at(TokenKind::EndOfFile) && !at(TokenKind::KwElse) && active())
      {
        const TokenId Before = Cursor.position();
        Statements.push_back(parseStmt());
        ensureProgress(Before);
      }
      Clauses.emplace_back(Default, Label, array(Statements), range(ClauseStart));
    }
    expect(TokenKind::RBrace);
    return make<SwitchStmt>(range(Start), Value, array(Clauses));
  }
} // namespace ink::parser
