#include "parser_internal.h"
namespace ink::parser
{
  Parser::Parser(core::FrontendContext &Context, ParsedUnit &Unit, ParseLimits Limits)
      : Input(Unit.input()),
        AST(Unit.context()),
        Cursor(Input),
        Diagnostics(Context.diagnosticEngine(), Input.lexedFile().sourceId(), Limits.MaxDiagnostics),
        RecoveryInfo(Unit.Recovery),
        Limits(std::move(Limits))
  {
  }
  ParseResult Parser::run(std::unique_ptr<ParsedUnit> Unit)
  {
    Unit->Root = parseModule();
    if (Status == ParseStatus::Completed && AST.allocatedBytes() > Limits.MaxAllocationBytes)
    {
      exceedLimit();
    }
    // The module owns recovery records; their exact ranges identify insertion
    // and deletion sites without retaining a token list on every node.
    for (auto &Entry : RecoveryInfo.Entries)
    {
      Entry.Node = Unit->Root;
    }
    return {std::move(Unit), Status, Diagnostics.errorCount() != 0};
  }
  ParseResult parse(core::FrontendContext &Context, tokenizer::TokenizedBuffer Input, ParseLimits Limits)
  {
    auto Unit = std::make_unique<ParsedUnit>(std::make_shared<const TokenBuffer>(std::move(Input)));
    Parser Reader(Context, *Unit, std::move(Limits));
    return Reader.run(std::move(Unit));
  }
  ModuleAST *Parser::parseModule()
  {
    std::vector<Stmt *> Statements;
    while (!at(TokenKind::EndOfFile) && active())
    {
      const TokenId Before = Cursor.position();
      Statements.push_back(parseStmt());
      ensureProgress(Before);
    }
    return make<ModuleAST>(SourceRange::fromByteOffsets(0, Input.lexedFile().source().size()), array(Statements));
  }
  bool Parser::active()
  {
    if (Status != ParseStatus::Completed || (Strict && TrialFailed))
    {
      return false;
    }
    if (Limits.IsCancelled && Limits.IsCancelled())
    {
      Status = ParseStatus::Cancelled;
      return false;
    }
    if (++Work > Limits.MaxWork || AST.allocatedBytes() >= Limits.MaxAllocationBytes)
    {
      exceedLimit();
      return false;
    }
    return true;
  }
  void Parser::exceedLimit()
  {
    if (Status != ParseStatus::Completed)
    {
      return;
    }
    Status = ParseStatus::LimitExceeded;
    Diagnostics.report(core::makeDiagnostic<core::DiagnosticKind::ParserLimitExceeded>(point()));
  }
  TokenId Parser::bump()
  {
    const TokenId Id = Cursor.bump();
    LastEnd = std::max(LastEnd, Input.token(Id).Span.getEnd().getByteOffset());
    return Id;
  }
  bool Parser::take(TokenKind Kind)
  {
    if (!at(Kind))
    {
      return false;
    }
    bump();
    return true;
  }
  void Parser::error(core::DiagnosticKind Kind, SourceRange Range)
  {
    if (Strict)
    {
      TrialFailed = true;
      return;
    }
    if (Status != ParseStatus::Completed || (at(TokenKind::EndOfFile) && !Input.lexedFile().succeeded()))
    {
      return;
    }
    core::Diagnostic Diagnostic;
    Diagnostic.Kind = Kind;
    Diagnostic.Span = Range;
    Diagnostics.report(std::move(Diagnostic));
  }
  void Parser::invalid(SourceRange Range)
  {
    error(core::DiagnosticKind::ParserInvalidSyntax, Range);
  }
  Expr *Parser::missingExpr()
  {
    error(core::DiagnosticKind::ParserExpectedExpression, point());
    LastEnd = std::max(LastEnd, start());
    return make<MissingExpr>(point());
  }
  NameToken Parser::name()
  {
    const ExpectResult Result = expect(TokenKind::Identifier);
    return {Result.Actual.value_or(InvalidTokenId), Result.Actual ? Input.spelling(*Result.Actual) : std::string_view{}, Result.Range};
  }
  void Parser::ensureProgress(TokenId Before)
  {
    if (Cursor.position() == Before && !at(TokenKind::EndOfFile) && active())
    {
      invalid(Cursor.peek().Span);
      bump();
    }
  }
} // namespace ink::parser
