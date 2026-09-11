#include "ink/parser/parser.h"

#include <algorithm>
#include <cassert>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace ink::parser
{
  using core::Diagnostic;
  using core::DiagnosticKind;
  using core::makeDiagnostic;
  using core::SourceRange;
  using tokenizer::KeywordKind;
  using tokenizer::SymbolKind;
  using tokenizer::Token;
  using tokenizer::TokenizedBuffer;
  using tokenizer::TokenKind;

  class ParserImpl
  {
    public:
      ParserImpl(const TokenizedBuffer &LexedFile, ParserOptions Options)
          : LexedFile(LexedFile),
            Options(Options)
      {
      }

      AstTree run()
      {
        SourceFile File;
        while (!atEnd())
        {
          const std::size_t Before = Index;
          File.Statements.push_back(parseStatement());
          if (Index == Before && !atEnd())
          {
            File.Statements.push_back(unexpected());
          }
        }
        Tree.setRoot(Tree.addNode(std::move(File), {0, LexedFile.source().size()}));
        return std::move(Tree);
      }

      std::vector<Diagnostic> takeDiagnostics()
      {
        return std::move(Diagnostics);
      }

      ParseCompleteness completeness() const noexcept
      {
        return Incomplete && !DefinitiveError ? ParseCompleteness::Incomplete : ParseCompleteness::Complete;
      }

    private:
      enum class BinaryPrecedence
      {
        None,
        LogicalOr,
        LogicalAnd,
        BitwiseOr,
        BitwiseXor,
        BitwiseAnd,
        Equality,
        Relational,
        Shift,
        Additive,
        Multiplicative,
      };

      struct ParseMark
      {
          std::size_t Start = 0;
          std::size_t DiagnosticCount = 0;
          std::size_t MissingCount = 0;
      };

      struct NestingScope
      {
          explicit NestingScope(std::size_t &Depth)
              : Depth(Depth)
          {
            ++Depth;
          }

          ~NestingScope()
          {
            --Depth;
          }

          std::size_t &Depth;
      };

      const Token &peek(std::size_t Offset = 0) const
      {
        std::size_t Position = Index;
        while (Position < LexedFile.tokens().size())
        {
          if (!LexedFile.tokens()[Position].isTrivia())
          {
            if (Offset == 0)
            {
              return LexedFile.tokens()[Position];
            }
            --Offset;
          }
          ++Position;
        }
        return LexedFile.tokens().back();
      }

      bool atEnd() const
      {
        return at(TokenKind::EndOfFile);
      }

      template <typename Kind>
      bool at(Kind Expected, std::size_t Offset = 0) const
      {
        return peek(Offset).is(Expected);
      }

      template <typename... Kinds>
      bool atAny(Kinds... Expected) const
      {
        return peek().isOneOf(Expected...);
      }

      ParseMark mark() const
      {
        return {peek().Span.Start, Diagnostics.size(), MissingCount};
      }

      ParseMark markFrom(AstNodeId Id) const
      {
        return {Tree.node(Id).Span.Start, Diagnostics.size(), MissingCount};
      }

      SourceRange range(const ParseMark &Mark) const
      {
        std::size_t End = std::max(Mark.Start, LastEnd);
        if (Mark.MissingCount != MissingCount)
        {
          End = std::max(End, LastMissingAnchor);
        }
        return {Mark.Start, End};
      }

      template <typename NodeType>
      AstNodeId makeNode(NodeType Node, const ParseMark &Mark)
      {
        AstNodeFlags Flags = AstNodeFlags::None;
        if (Mark.DiagnosticCount != Diagnostics.size())
        {
          Flags |= AstNodeFlags::HasError;
        }
        if (Mark.MissingCount != MissingCount)
        {
          Flags |= AstNodeFlags::HasMissing;
        }
        return Tree.addNode(std::move(Node), range(Mark), Flags);
      }

      AstTokenId consume()
      {
        while (Index < LexedFile.tokens().size() && LexedFile.tokens()[Index].isTrivia())
        {
          ++Index;
        }
        if (Index == LexedFile.tokens().size())
        {
          return InvalidAstTokenId;
        }
        const AstTokenId Result = Index++;
        LastEnd = LexedFile.tokens()[Result].Span.End;
        return Result;
      }

      template <typename Kind>
      bool accept(Kind Expected)
      {
        if (!at(Expected))
        {
          return false;
        }
        consume();
        return true;
      }

      void missing(std::string_view Expected)
      {
        LastMissingAnchor = peek().Span.Start;
        ++MissingCount;
        Diagnostics.push_back(makeDiagnostic<DiagnosticKind::ExpectedToken>({LastMissingAnchor, LastMissingAnchor}, Expected));
        if (atEnd())
        {
          Incomplete = Options.Mode == ParseMode::Interactive;
        }
        else
        {
          DefinitiveError = true;
        }
      }

      void missing(SymbolKind Expected)
      {
        missing(tokenizer::symbolSpelling(Expected));
      }

      void missing(KeywordKind Expected)
      {
        missing(tokenizer::keywordSpelling(Expected));
      }

      template <typename Kind>
      void expect(Kind Expected)
      {
        if (!accept(Expected))
        {
          missing(Expected);
        }
      }

      AstTokenId expectKind(TokenKind Kind, std::string_view Expected)
      {
        if (at(Kind))
        {
          return consume();
        }
        missing(Expected);
        return InvalidAstTokenId;
      }

      void reportUnexpected()
      {
        Diagnostics.push_back(makeDiagnostic<DiagnosticKind::UnexpectedToken>(peek().Span, LexedFile.raw(peek())));
        DefinitiveError = true;
      }

      AstNodeId unexpected()
      {
        const ParseMark Mark = mark();
        if (atEnd())
        {
          missing("statement");
        }
        else
        {
          reportUnexpected();
          consume();
        }
        return makeNode(Error{"statement", {}}, Mark);
      }

      std::optional<AstNodeId> nestingError()
      {
        if (Depth < Options.MaxSyntaxNestingDepth)
        {
          return std::nullopt;
        }
        const ParseMark Mark = mark();
        Diagnostics.push_back(makeDiagnostic<DiagnosticKind::SyntaxNestingLimit>(peek().Span));
        DefinitiveError = true;
        std::vector<SymbolKind> Closers;
        do
        {
          if (atEnd())
          {
            break;
          }
          if (at(SymbolKind::LeftParen))
          {
            Closers.push_back(SymbolKind::RightParen);
          }
          else if (at(SymbolKind::LeftBracket))
          {
            Closers.push_back(SymbolKind::RightBracket);
          }
          else if (at(SymbolKind::LeftBrace))
          {
            Closers.push_back(SymbolKind::RightBrace);
          }
          else if (!Closers.empty() && at(Closers.back()))
          {
            Closers.pop_back();
          }
          consume();
        } while (!Closers.empty());
        return makeNode(Error{"syntax nesting within the configured limit", {}}, Mark);
      }

      AstNodeId parseStatement()
      {
        if (const std::optional<AstNodeId> ErrorId = nestingError())
        {
          return *ErrorId;
        }
        NestingScope Scope(Depth);
        std::vector<ParseMark> ComptimeMarks;
        while (at(KeywordKind::Comptime))
        {
          ComptimeMarks.push_back(mark());
          consume();
        }
        AstNodeId Statement = parseStatementBody();
        while (!ComptimeMarks.empty())
        {
          Statement = makeNode(ComptimeStatement{Statement}, ComptimeMarks.back());
          ComptimeMarks.pop_back();
        }
        return Statement;
      }

      AstNodeId parseStatementBody()
      {
        if (at(SymbolKind::LeftBrace))
        {
          return parseBlock();
        }
        if (atAny(KeywordKind::Import, KeywordKind::From))
        {
          return parseImport();
        }
        if (at(KeywordKind::If))
        {
          return parseIf();
        }
        if (at(KeywordKind::While))
        {
          const ParseMark Mark = mark();
          consume();
          WhileStatement Statement;
          Statement.Condition = parseCondition();
          Statement.Body = parseBlock();
          return makeNode(Statement, Mark);
        }
        if (at(KeywordKind::For))
        {
          return parseFor();
        }
        if (atAny(KeywordKind::Return, KeywordKind::Break, KeywordKind::Continue, KeywordKind::Defer))
        {
          const ParseMark Mark = mark();
          const bool Return = at(KeywordKind::Return);
          const bool Defer = at(KeywordKind::Defer);
          const bool Break = at(KeywordKind::Break);
          consume();
          if (Defer)
          {
            const AstNodeId Action = at(SymbolKind::LeftBrace) ? parseBlock() : parseExpressionOrAssignment(true);
            return makeNode(DeferStatement{Action}, Mark);
          }
          AstNodeId Value = InvalidAstNodeId;
          if (Return && !at(SymbolKind::Semicolon))
          {
            Value = parseExpression();
          }
          expect(SymbolKind::Semicolon);
          if (Return)
          {
            return makeNode(ReturnStatement{Value}, Mark);
          }
          return Break ? makeNode(BreakStatement{}, Mark) : makeNode(ContinueStatement{}, Mark);
        }
        if (atAny(KeywordKind::Public, KeywordKind::Private, KeywordKind::Class, KeywordKind::Interface, KeywordKind::Enum, KeywordKind::ClassMethod, KeywordKind::ClassField, KeywordKind::EnumField, KeywordKind::Let, KeywordKind::Var, KeywordKind::Const, KeywordKind::Extern, KeywordKind::Func))
        {
          return parseDeclarationOrExpression();
        }
        return parseExpressionOrAssignment(true);
      }

      AstNodeId parseBlock()
      {
        const ParseMark Mark = mark();
        BlockStatement Block;
        if (!accept(SymbolKind::LeftBrace))
        {
          missing(SymbolKind::LeftBrace);
          return makeNode(std::move(Block), Mark);
        }
        while (!atEnd() && !at(SymbolKind::RightBrace))
        {
          const std::size_t Before = Index;
          Block.Statements.push_back(parseStatement());
          if (Index == Before && !atEnd())
          {
            Block.Statements.push_back(unexpected());
          }
        }
        expect(SymbolKind::RightBrace);
        return makeNode(std::move(Block), Mark);
      }

      AstNodeId parseImport()
      {
        const ParseMark Mark = mark();
        ImportDeclaration Declaration;
        Declaration.IsMemberImport = at(KeywordKind::From);
        consume();
        Declaration.Package.push_back(expectKind(TokenKind::Identifier, "identifier"));
        while (accept(SymbolKind::Dot))
        {
          Declaration.Package.push_back(expectKind(TokenKind::Identifier, "identifier"));
        }
        if (Declaration.IsMemberImport)
        {
          expect(KeywordKind::Import);
          Declaration.Member = expectKind(TokenKind::Identifier, "identifier");
        }
        if (accept(KeywordKind::As))
        {
          Declaration.Alias = expectKind(TokenKind::Identifier, "identifier");
        }
        expect(SymbolKind::Semicolon);
        return makeNode(std::move(Declaration), Mark);
      }

      AstNodeId parseDeclarationOrExpression()
      {
        const ParseMark Mark = mark();
        const bool HasAccess = atAny(KeywordKind::Public, KeywordKind::Private);
        AccessKind Access = AccessKind::Unspecified;
        if (HasAccess)
        {
          Access = at(KeywordKind::Public) ? AccessKind::Public : AccessKind::Private;
          consume();
        }
        const bool HasLinkage = accept(KeywordKind::Extern);
        const AstTokenId Linkage = HasLinkage ? expectKind(TokenKind::StringLiteral, "string literal") : InvalidAstTokenId;
        const bool Constant = accept(KeywordKind::Const);
        if (atAny(KeywordKind::Func, KeywordKind::ClassMethod))
        {
          const bool Method = at(KeywordKind::ClassMethod);
          consume();
          if (at(TokenKind::Identifier) || HasAccess || Method || Constant)
          {
            if (Constant && Method)
            {
              missing(KeywordKind::Func);
            }
            FunctionDeclaration Declaration;
            Declaration.Access = Access;
            Declaration.Linkage = Linkage;
            Declaration.IsConst = Constant;
            Declaration.IsClassMethod = Method;
            Declaration.Name = expectKind(TokenKind::Identifier, "identifier");
            if (at(SymbolKind::LeftBracket))
            {
              Declaration.GenericParameters = parseParameters(true);
            }
            Declaration.Parameters = parseParameters(false);
            expect(SymbolKind::Arrow);
            Declaration.ReturnType = parseExpression();
            if (at(SymbolKind::LeftBrace))
            {
              Declaration.Body = parseBlock();
            }
            else
            {
              expect(SymbolKind::Semicolon);
            }
            return makeNode(std::move(Declaration), Mark);
          }
          FunctionTypeExpression Function;
          Function.Linkage = Linkage;
          Function.Parameters = parseFunctionTypeParameters();
          expect(SymbolKind::Arrow);
          Function.ReturnType = parseUnary();
          const AstNodeId Expression = makeNode(std::move(Function), Mark);
          return parseExpressionOrAssignmentTail(Expression, true, Mark);
        }
        if (HasLinkage)
        {
          missing("func or class_method");
          return makeNode(Error{"func or class_method", {}}, Mark);
        }
        if (!Constant && atAny(KeywordKind::Class, KeywordKind::Interface, KeywordKind::Enum))
        {
          const bool Class = at(KeywordKind::Class);
          const bool Interface = at(KeywordKind::Interface);
          consume();
          const AstTokenId Name = expectKind(TokenKind::Identifier, "identifier");
          std::vector<AstNodeId> GenericParameters;
          if (at(SymbolKind::LeftBracket))
          {
            GenericParameters = parseParameters(true);
          }
          AstNodeId BaseType = InvalidAstNodeId;
          std::vector<AstNodeId> BaseTypes;
          std::vector<AstNodeId> Interfaces;
          if (Class && accept(KeywordKind::Extends))
          {
            BaseType = parseExpression();
          }
          else if (Interface && accept(KeywordKind::Extends))
          {
            BaseTypes = parseTypeList();
          }
          else if (!Class && !Interface && accept(SymbolKind::Colon))
          {
            BaseType = parseExpression();
          }
          if (!Interface && accept(KeywordKind::Implements))
          {
            Interfaces = parseTypeList();
          }
          const AstNodeId Body = parseBlock();
          if (Class)
          {
            return makeNode(ClassDeclaration{Access, Name, std::move(GenericParameters), BaseType, std::move(Interfaces), Body}, Mark);
          }
          if (Interface)
          {
            return makeNode(InterfaceDeclaration{Access, Name, std::move(GenericParameters), std::move(BaseTypes), Body}, Mark);
          }
          return makeNode(EnumDeclaration{Access, Name, std::move(GenericParameters), BaseType, std::move(Interfaces), Body}, Mark);
        }
        if (at(KeywordKind::ClassField) || (!HasAccess && !Constant && at(KeywordKind::EnumField)))
        {
          const bool ClassField = at(KeywordKind::ClassField);
          consume();
          const AstTokenId Name = expectKind(TokenKind::Identifier, "identifier");
          AstNodeId TypeExpression = InvalidAstNodeId;
          if (ClassField)
          {
            expect(SymbolKind::Colon);
            TypeExpression = parseExpression();
          }
          const AstNodeId Initializer = accept(SymbolKind::Assign) ? parseExpression() : InvalidAstNodeId;
          expect(SymbolKind::Semicolon);
          if (ClassField)
          {
            return makeNode(ClassFieldDeclaration{Access, Constant, Name, TypeExpression, Initializer}, Mark);
          }
          return makeNode(EnumFieldDeclaration{Name, Initializer}, Mark);
        }
        if (Constant && !HasAccess && atAny(KeywordKind::Ref, KeywordKind::Ptr))
        {
          const bool Reference = at(KeywordKind::Ref);
          consume();
          const AstNodeId Operand = parseUnary();
          const AstNodeId Expression = Reference ? makeNode(ReferenceTypeExpression{true, Operand}, Mark) : makeNode(PointerTypeExpression{true, Operand}, Mark);
          return parseExpressionOrAssignmentTail(Expression, true, Mark);
        }
        if (Constant || atAny(KeywordKind::Let, KeywordKind::Var))
        {
          BindingDeclaration Declaration;
          Declaration.Access = Access;
          Declaration.Binding = Constant ? BindingKind::Const : at(KeywordKind::Var) ? BindingKind::Var
                                                                                     : BindingKind::Let;
          if (!Constant)
          {
            consume();
          }
          Declaration.Pattern = parsePattern();
          expect(SymbolKind::Colon);
          Declaration.TypeExpression = parseExpression();
          expect(SymbolKind::Assign);
          Declaration.Initializer = parseExpression();
          expect(SymbolKind::Semicolon);
          return makeNode(Declaration, Mark);
        }
        missing("declaration");
        return makeNode(Error{"declaration", {}}, Mark);
      }

      std::vector<AstNodeId> parseTypeList()
      {
        std::vector<AstNodeId> Types;
        Types.push_back(parseExpression());
        while (accept(SymbolKind::Comma))
        {
          Types.push_back(parseExpression());
        }
        return Types;
      }

      AstNodeId parsePattern()
      {
        if (const std::optional<AstNodeId> ErrorId = nestingError())
        {
          return *ErrorId;
        }
        NestingScope Scope(Depth);
        const ParseMark Mark = mark();
        if (accept(SymbolKind::LeftParen))
        {
          TuplePattern Pattern;
          Pattern.Elements.push_back(parsePattern());
          expect(SymbolKind::Comma);
          Pattern.Elements.push_back(parsePattern());
          while (accept(SymbolKind::Comma))
          {
            Pattern.Elements.push_back(parsePattern());
          }
          expect(SymbolKind::RightParen);
          return makeNode(std::move(Pattern), Mark);
        }
        if (accept(SymbolKind::Underscore))
        {
          return makeNode(WildcardPattern{}, Mark);
        }
        const AstTokenId Name = expectKind(TokenKind::Identifier, "binding pattern");
        return Name == InvalidAstTokenId ? makeNode(Error{"binding pattern", {}}, Mark) : makeNode(NamePattern{Name}, Mark);
      }

      std::vector<AstNodeId> parseParameters(bool Generic)
      {
        std::vector<AstNodeId> Parameters;
        expect(Generic ? SymbolKind::LeftBracket : SymbolKind::LeftParen);
        const SymbolKind Close = Generic ? SymbolKind::RightBracket : SymbolKind::RightParen;
        bool Defaults = false;
        if (Generic || !at(Close))
        {
          while (!atEnd())
          {
            const std::size_t Before = Index;
            const ParseMark Mark = mark();
            if (!Generic && accept(SymbolKind::Ellipsis))
            {
              FunctionParameter Parameter;
              Parameter.IsVariadic = true;
              Parameters.push_back(makeNode(Parameter, Mark));
              break;
            }
            const AstTokenId Name = Generic ? expectKind(TokenKind::Identifier, "generic parameter name") : InvalidAstTokenId;
            const AstNodeId Pattern = Generic ? InvalidAstNodeId : parsePattern();
            expect(SymbolKind::Colon);
            const AstNodeId TypeExpression = parseExpression();
            const bool Pack = accept(SymbolKind::Ellipsis);
            AstNodeId DefaultValue = InvalidAstNodeId;
            if (!Pack && accept(SymbolKind::Assign))
            {
              Defaults = true;
              DefaultValue = parseExpression();
            }
            else if (!Pack && Defaults)
            {
              missing("default value or parameter pack");
            }
            Parameters.push_back(Generic ? makeNode(GenericParameter{Name, TypeExpression, DefaultValue, Pack}, Mark) : makeNode(FunctionParameter{Pattern, TypeExpression, DefaultValue, Pack}, Mark));
            if (Pack || !accept(SymbolKind::Comma))
            {
              break;
            }
            if (at(Close))
            {
              missing("parameter");
              break;
            }
            if (Before == Index)
            {
              Parameters.push_back(unexpected());
              break;
            }
          }
        }
        if (Generic && atEnd())
        {
          missing("generic parameter");
        }
        expect(Close);
        return Parameters;
      }

      AstNodeId parseCondition()
      {
        expect(SymbolKind::LeftParen);
        const AstNodeId Condition = parseExpression();
        expect(SymbolKind::RightParen);
        return Condition;
      }

      AstNodeId parseIf()
      {
        struct Branch
        {
            ParseMark Mark;
            AstNodeId Condition = InvalidAstNodeId;
            AstNodeId Body = InvalidAstNodeId;
        };
        std::vector<Branch> Branches;
        AstNodeId Else = InvalidAstNodeId;
        do
        {
          Branch Current;
          Current.Mark = mark();
          expect(KeywordKind::If);
          Current.Condition = parseCondition();
          Current.Body = parseBlock();
          Branches.push_back(Current);
          if (!accept(KeywordKind::Else))
          {
            break;
          }
          if (!at(KeywordKind::If))
          {
            Else = parseBlock();
            break;
          }
        } while (true);
        while (!Branches.empty())
        {
          const Branch &Current = Branches.back();
          Else = makeNode(IfStatement{Current.Condition, Current.Body, Else}, Current.Mark);
          Branches.pop_back();
        }
        return Else;
      }

      AstNodeId parseFor()
      {
        const ParseMark Mark = mark();
        consume();
        expect(SymbolKind::LeftParen);
        ForStatement Statement;
        const bool Constant = at(KeywordKind::Const);
        const bool Binding = atAny(KeywordKind::Let, KeywordKind::Var) || (Constant && !at(KeywordKind::Ref, 1) && !at(KeywordKind::Ptr, 1));
        if (Binding)
        {
          const ParseMark BindingMark = mark();
          BindingDeclaration Declaration;
          Declaration.Binding = Constant ? BindingKind::Const : at(KeywordKind::Var) ? BindingKind::Var
                                                                                     : BindingKind::Let;
          consume();
          Declaration.Pattern = parsePattern();
          expect(SymbolKind::Colon);
          Declaration.TypeExpression = parseExpression();
          if (!Constant && accept(KeywordKind::In))
          {
            ForInStatement Iteration;
            Iteration.Binding = Declaration.Binding;
            Iteration.Pattern = Declaration.Pattern;
            Iteration.TypeExpression = Declaration.TypeExpression;
            Iteration.Iterable = parseExpression();
            expect(SymbolKind::RightParen);
            Iteration.Body = parseBlock();
            return makeNode(Iteration, Mark);
          }
          expect(SymbolKind::Assign);
          Declaration.Initializer = parseExpression();
          Statement.Initializers.push_back(makeNode(Declaration, BindingMark));
        }
        else if (!at(SymbolKind::Semicolon))
        {
          Statement.Initializers = parseExpressionOrAssignmentList();
        }
        expect(SymbolKind::Semicolon);
        if (!at(SymbolKind::Semicolon))
        {
          Statement.Condition = parseExpression();
        }
        expect(SymbolKind::Semicolon);
        if (!at(SymbolKind::RightParen))
        {
          Statement.Updates = parseExpressionOrAssignmentList();
        }
        expect(SymbolKind::RightParen);
        Statement.Body = parseBlock();
        return makeNode(std::move(Statement), Mark);
      }

      std::vector<AstNodeId> parseExpressionOrAssignmentList()
      {
        std::vector<AstNodeId> Operations;
        Operations.push_back(parseExpressionOrAssignment(false));
        while (accept(SymbolKind::Comma))
        {
          Operations.push_back(parseExpressionOrAssignment(false));
        }
        return Operations;
      }

      AstNodeId parseExpressionOrAssignment(bool Semicolon)
      {
        const ParseMark Mark = mark();
        const AstNodeId Expression = parseUnary();
        return parseExpressionOrAssignmentTail(Expression, Semicolon, Mark);
      }

      AstNodeId parseExpressionOrAssignmentTail(AstNodeId Left, bool Semicolon, const ParseMark &Mark)
      {
        if (atAny(SymbolKind::Assign, SymbolKind::PlusAssign, SymbolKind::MinusAssign, SymbolKind::StarAssign, SymbolKind::SlashAssign, SymbolKind::PercentAssign, SymbolKind::AmpersandAssign, SymbolKind::PipeAssign, SymbolKind::CaretAssign, SymbolKind::ShiftLeftAssign, SymbolKind::ShiftRightAssign))
        {
          const tokenizer::SymbolKind Operator = peek().symbol();
          consume();
          const AstNodeId Value = parseExpression();
          if (Semicolon)
          {
            expect(SymbolKind::Semicolon);
          }
          return makeNode(AssignmentStatement{Operator, Left, Value}, Mark);
        }
        Left = parseBinaryTail(Left, BinaryPrecedence::LogicalOr);
        Left = parseConditionalTail(Left);
        if (Semicolon)
        {
          expect(SymbolKind::Semicolon);
        }
        return makeNode(ExpressionStatement{Left}, Mark);
      }

      AstNodeId parseExpression()
      {
        if (const std::optional<AstNodeId> ErrorId = nestingError())
        {
          return *ErrorId;
        }
        NestingScope Scope(Depth);
        return parseConditionalTail(parseBinary(BinaryPrecedence::LogicalOr));
      }

      AstNodeId parseConditionalTail(AstNodeId Condition)
      {
        if (!at(SymbolKind::Question))
        {
          return Condition;
        }
        const ParseMark Mark = markFrom(Condition);
        consume();
        const AstNodeId Then = parseExpression();
        expect(SymbolKind::Colon);
        const AstNodeId Else = parseExpression();
        return makeNode(ConditionalExpression{Condition, Then, Else}, Mark);
      }

      BinaryPrecedence precedence() const
      {
        const Token &Current = peek();
        if (!Current.is(TokenKind::Symbol))
        {
          return BinaryPrecedence::None;
        }
        switch (Current.symbol())
        {
        case SymbolKind::LogicalOr:
          return BinaryPrecedence::LogicalOr;
        case SymbolKind::LogicalAnd:
          return BinaryPrecedence::LogicalAnd;
        case SymbolKind::Pipe:
          return BinaryPrecedence::BitwiseOr;
        case SymbolKind::Caret:
          return BinaryPrecedence::BitwiseXor;
        case SymbolKind::Ampersand:
          return BinaryPrecedence::BitwiseAnd;
        case SymbolKind::Equal:
        case SymbolKind::NotEqual:
          return BinaryPrecedence::Equality;
        case SymbolKind::Less:
        case SymbolKind::LessEqual:
        case SymbolKind::Greater:
        case SymbolKind::GreaterEqual:
          return BinaryPrecedence::Relational;
        case SymbolKind::ShiftLeft:
        case SymbolKind::ShiftRight:
          return BinaryPrecedence::Shift;
        case SymbolKind::Plus:
        case SymbolKind::Minus:
          return BinaryPrecedence::Additive;
        case SymbolKind::Star:
        case SymbolKind::Slash:
        case SymbolKind::Percent:
          return BinaryPrecedence::Multiplicative;
        default:
          return BinaryPrecedence::None;
        }
      }

      AstNodeId parseBinary(BinaryPrecedence Minimum)
      {
        return parseBinaryTail(parseUnary(), Minimum);
      }

      AstNodeId parseBinaryTail(AstNodeId Left, BinaryPrecedence Minimum)
      {
        bool Equality = false;
        bool Relational = false;
        while (true)
        {
          const BinaryPrecedence Precedence = precedence();
          if (Precedence < Minimum)
          {
            break;
          }
          const ParseMark Mark = markFrom(Left);
          if ((Precedence == BinaryPrecedence::Equality && Equality) || (Precedence == BinaryPrecedence::Relational && Relational))
          {
            reportUnexpected();
            return makeNode(Error{"non-chained comparison", {Left}}, Mark);
          }
          Equality = Equality || Precedence == BinaryPrecedence::Equality;
          Relational = Relational || Precedence == BinaryPrecedence::Relational;
          if (Precedence < BinaryPrecedence::Equality)
          {
            Equality = false;
          }
          if (Precedence < BinaryPrecedence::Relational)
          {
            Relational = false;
          }
          const tokenizer::SymbolKind Operator = peek().symbol();
          consume();
          const AstNodeId Right = parseBinary(static_cast<BinaryPrecedence>(static_cast<unsigned>(Precedence) + 1));
          Left = makeNode(BinaryExpression{Operator, Left, Right}, Mark);
        }
        return Left;
      }

      AstNodeId parseUnary()
      {
        enum class PrefixKind
        {
          Unary,
          Comptime,
          Reference,
          Pointer,
          Function,
        };
        struct Prefix
        {
            PrefixKind Kind = PrefixKind::Unary;
            ParseMark Mark;
            tokenizer::SymbolKind Operator = tokenizer::SymbolKind::Plus;
            bool IsConst = false;
            AstTokenId Linkage = InvalidAstTokenId;
            std::vector<FunctionTypeParameter> Parameters;
        };
        std::vector<Prefix> Prefixes;
        while (true)
        {
          if (atAny(SymbolKind::Plus, SymbolKind::Minus, SymbolKind::Exclamation, SymbolKind::Tilde, SymbolKind::Ampersand, SymbolKind::Star, KeywordKind::Comptime, KeywordKind::Ref, KeywordKind::Ptr, KeywordKind::Const))
          {
            Prefix Current;
            Current.Mark = mark();
            Current.IsConst = at(KeywordKind::Const);
            Current.Kind = at(KeywordKind::Comptime) ? PrefixKind::Comptime : at(KeywordKind::Ref) || (Current.IsConst && at(KeywordKind::Ref, 1)) ? PrefixKind::Reference
                                                                          : at(KeywordKind::Ptr) || Current.IsConst                                ? PrefixKind::Pointer
                                                                                                                                                   : PrefixKind::Unary;
            if (Current.Kind == PrefixKind::Unary)
            {
              Current.Operator = peek().symbol();
            }
            consume();
            if (Current.IsConst)
            {
              if (atAny(KeywordKind::Ref, KeywordKind::Ptr))
              {
                consume();
              }
              else
              {
                missing("ref or ptr");
              }
            }
            Prefixes.push_back(std::move(Current));
            continue;
          }
          if (atAny(KeywordKind::Func, KeywordKind::Extern))
          {
            Prefix Current;
            Current.Kind = PrefixKind::Function;
            Current.Mark = mark();
            if (accept(KeywordKind::Extern))
            {
              Current.Linkage = expectKind(TokenKind::StringLiteral, "string literal");
            }
            expect(KeywordKind::Func);
            Current.Parameters = parseFunctionTypeParameters();
            expect(SymbolKind::Arrow);
            Prefixes.push_back(std::move(Current));
            continue;
          }
          break;
        }
        AstNodeId Operand = parsePostfix();
        while (!Prefixes.empty())
        {
          Prefix &Current = Prefixes.back();
          switch (Current.Kind)
          {
          case PrefixKind::Unary:
            Operand = makeNode(UnaryExpression{Current.Operator, Operand}, Current.Mark);
            break;
          case PrefixKind::Comptime:
            Operand = makeNode(ComptimeExpression{Operand}, Current.Mark);
            break;
          case PrefixKind::Reference:
            Operand = makeNode(ReferenceTypeExpression{Current.IsConst, Operand}, Current.Mark);
            break;
          case PrefixKind::Pointer:
            Operand = makeNode(PointerTypeExpression{Current.IsConst, Operand}, Current.Mark);
            break;
          case PrefixKind::Function:
            Operand = makeNode(FunctionTypeExpression{Current.Linkage, std::move(Current.Parameters), Operand}, Current.Mark);
            break;
          }
          Prefixes.pop_back();
        }
        return Operand;
      }

      std::vector<FunctionTypeParameter> parseFunctionTypeParameters()
      {
        std::vector<FunctionTypeParameter> Parameters;
        expect(SymbolKind::LeftParen);
        if (!at(SymbolKind::RightParen))
        {
          while (!atEnd())
          {
            const ParseMark Mark = mark();
            if (accept(SymbolKind::Ellipsis))
            {
              Parameters.push_back({InvalidAstNodeId, true, range(Mark)});
              break;
            }
            const AstNodeId TypeExpression = parseExpression();
            const bool Variadic = accept(SymbolKind::Ellipsis);
            Parameters.push_back({TypeExpression, Variadic, range(Mark)});
            if (Variadic || !accept(SymbolKind::Comma))
            {
              break;
            }
            if (at(SymbolKind::RightParen))
            {
              missing("function type parameter");
              break;
            }
          }
        }
        expect(SymbolKind::RightParen);
        return Parameters;
      }

      AstNodeId parsePostfix()
      {
        AstNodeId Expression = parsePrimary();
        while (true)
        {
          const ParseMark Mark = markFrom(Expression);
          if (accept(SymbolKind::LeftParen))
          {
            std::vector<AstArgument> Arguments = parseArguments(SymbolKind::RightParen, true);
            Expression = makeNode(CallExpression{Expression, std::move(Arguments)}, Mark);
          }
          else if (accept(SymbolKind::DoubleColon))
          {
            expect(SymbolKind::LeftBracket);
            std::vector<AstArgument> Arguments = parseArguments(SymbolKind::RightBracket, false);
            Expression = makeNode(GenericInstantiationExpression{Expression, std::move(Arguments)}, Mark);
          }
          else if (accept(SymbolKind::LeftBracket))
          {
            const AstNodeId IndexExpressionId = parseExpression();
            expect(SymbolKind::RightBracket);
            Expression = makeNode(IndexExpression{Expression, IndexExpressionId}, Mark);
          }
          else if (atAny(SymbolKind::Dot, SymbolKind::Arrow))
          {
            const bool Pointer = at(SymbolKind::Arrow);
            consume();
            const AstTokenId Member = expectKind(TokenKind::Identifier, "member name");
            Expression = makeNode(MemberExpression{Expression, Member, Pointer}, Mark);
          }
          else
          {
            break;
          }
        }
        return Expression;
      }

      std::vector<AstArgument> parseArguments(SymbolKind Close, bool EmptyAllowed)
      {
        std::vector<AstArgument> Arguments;
        if (!EmptyAllowed || !at(Close))
        {
          while (true)
          {
            const ParseMark Mark = mark();
            const AstNodeId Expression = parseExpression();
            const bool Pack = accept(SymbolKind::Ellipsis);
            Arguments.push_back({Expression, Pack, range(Mark)});
            if (Pack || !accept(SymbolKind::Comma))
            {
              break;
            }
          }
        }
        expect(Close);
        return Arguments;
      }

      AstNodeId parsePrimary()
      {
        const ParseMark Mark = mark();
        if (at(TokenKind::Identifier))
        {
          const AstTokenId Name = consume();
          return makeNode(NameExpression{Name}, Mark);
        }
        if (atAny(TokenKind::IntegerLiteral, TokenKind::FloatLiteral, TokenKind::StringLiteral, KeywordKind::True, KeywordKind::False, KeywordKind::Null))
        {
          const AstTokenId Value = consume();
          return makeNode(LiteralExpression{Value}, Mark);
        }
        if (atAny(KeywordKind::Void, KeywordKind::Bool, KeywordKind::Int8, KeywordKind::Int16, KeywordKind::Int32, KeywordKind::Int64, KeywordKind::UInt8, KeywordKind::UInt16, KeywordKind::UInt32, KeywordKind::UInt64, KeywordKind::Float, KeywordKind::Double, KeywordKind::Type))
        {
          const tokenizer::KeywordKind Type = peek().keyword();
          consume();
          return makeNode(BuiltinTypeExpression{Type}, Mark);
        }
        if (atAny(KeywordKind::Self, KeywordKind::Super))
        {
          const tokenizer::KeywordKind Receiver = peek().keyword();
          consume();
          return makeNode(ReceiverExpression{Receiver}, Mark);
        }
        if (atAny(SymbolKind::LeftParen, SymbolKind::LeftBracket))
        {
          const bool Parenthesis = at(SymbolKind::LeftParen);
          consume();
          const SymbolKind Close = Parenthesis ? SymbolKind::RightParen : SymbolKind::RightBracket;
          std::vector<AstNodeId> Elements;
          if (Parenthesis || !at(Close))
          {
            Elements.push_back(parseExpression());
            while (accept(SymbolKind::Comma))
            {
              Elements.push_back(parseExpression());
            }
          }
          expect(Close);
          if (!Parenthesis)
          {
            return makeNode(ArrayExpression{std::move(Elements)}, Mark);
          }
          if (Elements.size() == 1)
          {
            return makeNode(ParenthesizedExpression{Elements.front()}, Mark);
          }
          return makeNode(TupleExpression{std::move(Elements)}, Mark);
        }
        missing("expression");
        if (!atEnd() && !atAny(SymbolKind::Semicolon, SymbolKind::RightParen, SymbolKind::RightBracket, SymbolKind::RightBrace, SymbolKind::Comma, SymbolKind::Colon, SymbolKind::Assign, SymbolKind::LeftBrace))
        {
          reportUnexpected();
          consume();
        }
        return makeNode(Error{"expression", {}}, Mark);
      }

      const TokenizedBuffer &LexedFile;
      ParserOptions Options;
      AstTree Tree;
      std::vector<Diagnostic> Diagnostics;
      std::size_t Index = 0;
      std::size_t LastEnd = 0;
      std::size_t Depth = 0;
      std::size_t MissingCount = 0;
      std::size_t LastMissingAnchor = 0;
      bool Incomplete = false;
      bool DefinitiveError = false;
  };

  ParsedFile::ParsedFile(TokenizedBuffer LexedFile, AstTree Tree, bool Succeeded, ParseCompleteness Completeness)
      : LexedFile(std::move(LexedFile)),
        Tree(std::move(Tree)),
        Succeeded(Succeeded),
        Completeness(Completeness)
  {
  }

  bool ParsedFile::succeeded() const noexcept
  {
    return Succeeded && !Tree.empty();
  }

  SourceRange ParsedFile::span(AstNodeId Id) const
  {
    return Tree.node(Id).Span;
  }

  Parser::Parser(core::FrontendContext &Context, ParserOptions Options)
      : Context(Context),
        Options(Options)
  {
  }

  ParsedFile Parser::parse(TokenizedBuffer LexedFile) const
  {
    if (!LexedFile.succeeded() || !LexedFile.isRegisteredWith(Context.sourceManager()))
    {
      return ParsedFile(std::move(LexedFile), {}, false, ParseCompleteness::Complete);
    }
    ParserImpl Implementation(LexedFile, Options);
    AstTree Tree = Implementation.run();
    std::vector<Diagnostic> ParserDiagnostics = Implementation.takeDiagnostics();
    const bool Succeeded = !Tree.empty() && ParserDiagnostics.empty();
    for (Diagnostic &DiagnosticEntry : ParserDiagnostics)
    {
      DiagnosticEntry.Source = LexedFile.sourceId();
      Context.diagnosticEngine().report(DiagnosticEntry);
    }
    return ParsedFile(std::move(LexedFile), std::move(Tree), Succeeded, Implementation.completeness());
  }

  ParsedFile parse(core::FrontendContext &Context, TokenizedBuffer LexedFile, ParserOptions Options)
  {
    return Parser(Context, Options).parse(std::move(LexedFile));
  }
} // namespace ink::parser
