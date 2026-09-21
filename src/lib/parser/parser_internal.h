#pragma once
#include "ink/parser/parser.h"
#include "ink/parser/parser_diagnostic_emitter.h"
#include <algorithm>
namespace ink::parser
{
  struct RecoveryPolicy
  {
      TokenKind End;
      bool Comma = false;
  };
  struct RecoveryContext
  {
      std::vector<RecoveryPolicy> Scopes;
  };
  class Parser
  {
    public:
      Parser(core::FrontendContext &Context, ParsedUnit &Unit, ParseLimits Limits);
      ParseResult run(std::unique_ptr<ParsedUnit> Unit);
      ModuleAST *parseModule();

    private:
      class RecoveryScope
      {
        public:
          RecoveryScope(Parser &Owner, TokenKind End, bool Comma = false)
              : Owner(Owner),
                Size(Owner.Recovery.Scopes.size())
          {
            Owner.Recovery.Scopes.push_back({End, Comma});
          }
          ~RecoveryScope()
          {
            Owner.Recovery.Scopes.resize(Size);
          }

        private:
          Parser &Owner;
          std::size_t Size;
      };
      class DepthScope
      {
        public:
          explicit DepthScope(Parser &Owner)
              : Owner(Owner),
                Entered(Owner.active() && Owner.Depth < std::min<std::size_t>(Owner.Limits.MaxNestingDepth, 512))
          {
            if (Entered)
            {
              ++Owner.Depth;
            }
            else if (Owner.Status == ParseStatus::Completed && !(Owner.Strict && Owner.TrialFailed))
            {
              Owner.exceedLimit();
            }
          }
          ~DepthScope()
          {
            if (Entered)
            {
              --Owner.Depth;
            }
          }
          explicit operator bool() const
          {
            return Entered;
          }

        private:
          Parser &Owner;
          bool Entered;
      };
      class ParseTransaction
      {
        public:
          explicit ParseTransaction(Parser &Owner);
          ~ParseTransaction();
          void commit();

        private:
          Parser &Owner;
          TokenId Position;
          ASTCheckpoint Allocation;
          RecoveryContext Recovery;
          std::size_t RecoverySize;
          std::size_t End;
          bool Strict;
          bool Failed;
          bool Committed = false;
      };
      const TokenBuffer &Input;
      ASTContext &AST;
      TokenCursor Cursor;
      ParserDiagnosticEmitter Diagnostics;
      SyntaxRecoveryInfo &RecoveryInfo;
      RecoveryContext Recovery;
      ParseLimits Limits;
      ParseStatus Status = ParseStatus::Completed;
      std::size_t Work = 0;
      std::size_t Depth = 0;
      std::size_t LastEnd = 0;
      bool Strict = false;
      bool TrialFailed = false;

      TokenKind kind(std::size_t Ahead = 0) const
      {
        return Cursor.peek(Ahead).Kind;
      }
      bool at(TokenKind Kind) const
      {
        return kind() == Kind;
      }
      std::size_t start() const
      {
        return Cursor.peek().Span.getBegin().getByteOffset();
      }
      SourceRange point() const
      {
        return SourceRange(Cursor.peek().getLocation());
      }
      SourceRange range(std::size_t Start) const
      {
        return SourceRange::fromByteOffsets(Start, std::max(Start, LastEnd));
      }
      TokenId bump();
      bool take(TokenKind Kind);
      bool active();
      void exceedLimit();
      bool declarationStart(std::size_t Ahead = 0) const;
      bool strongStart(TokenKind Kind) const;
      bool boundary(TokenKind Kind) const;
      bool listEnd(TokenKind End) const;
      bool expressionStart(TokenKind Kind) const;
      void error(core::DiagnosticKind Kind, SourceRange Range);
      void invalid(SourceRange Range);
      ExpectResult expect(TokenKind Kind);
      SourceRange recover(TokenKind Expected);
      NameToken name();
      bool nextElement(TokenKind End, bool AllowSingleTrailing = false);
      void ensureProgress(TokenId Before);
      template <typename T, typename... Args>
      T *make(SourceRange Range, Args &&...Values)
      {
        // Missing children may sit beyond the last consumed token after trivia.
        // Enclosing ranges must include those insertion points as well.
        LastEnd = std::max(LastEnd, Range.getEnd().getByteOffset());
        return AST.make<T>(Range, std::forward<Args>(Values)...);
      }
      template <typename T>
      ASTArray<T> array(const std::vector<T> &Values)
      {
        return AST.copyArray(Values);
      }
      Expr *missingExpr();
      Stmt *parseStmt();
      BlockStmt *parseBlock();
      Stmt *parseFor();
      Stmt *parseSwitch();
      Stmt *parseImport();
      Stmt *parseSimpleStmt(bool Semicolon = true);
      ASTArray<SimpleItem *> parseSimpleList(TokenKind End);
      SimpleItem *parseSimpleItem();
      Expr *parseExpr(unsigned MinBP = 0);
      Expr *parseUnary(bool TypeOnly = false);
      Expr *parsePostfix(Expr *Value);
      Expr *parsePrimary(bool TypeOnly = false);
      Expr *parseFunction(bool TypeOnly);
      Expr *parseMatch();
      TypeSyntax *parseTypeSyntax();
      ASTArray<Argument> parseArguments(TokenKind End);
      ASTArray<Parameter> parseParameters(TokenKind End, bool AllowEmpty, bool Payload = false);
      ASTArray<Parameter> parseGenericParameters();
      ASTArray<NameToken> parseQualifiedName();
      ASTArray<Attribute> parseAttributes();
      bool attributesAhead();
      Decl *parseDecl(ASTArray<Attribute> Attributes);
      VarDecl *parseVar(ASTArray<Attribute> Attributes);
      BindingPattern *parseBindingPattern();
      MatchPattern *parseMatchPattern();
      MatchPattern *parseMatchAtom();
  };
} // namespace ink::parser
