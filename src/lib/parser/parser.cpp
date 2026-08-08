#include "ink/parser/parser.h"

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace ink::parser
{
  using core::Diagnostic;
  using core::DiagnosticArgumentName;
  using core::DiagnosticBuilder;
  using core::DiagnosticKind;
  using core::SourceRange;
  using tokenizer::KeywordKind;
  using tokenizer::Token;
  using tokenizer::TokenizedBuffer;
  using tokenizer::TokenKind;

  class CstBuilder
  {
  public:
    struct TemporaryNode
    {
      CstKind Kind = CstKind::Unknown;
      std::vector<CstElement> Children;
    };

    struct Checkpoint
    {
      std::size_t NodesSize = 0;
      std::vector<CstNodeId> Stack;
      CstNodeId Parent = 0;
      std::size_t ParentChildCount = 0;
      std::optional<CstElement> LastChild;
    };

    CstNodeId start(CstKind Kind)
    {
      const CstNodeId Id = Nodes.size();
      Nodes.push_back({Kind, {}});
      if (!Stack.empty())
      {
        Nodes[Stack.back()].Children.push_back(CstNodeRef{Id});
      }
      Stack.push_back(Id);
      return Id;
    }

    void finish()
    {
      assert(!Stack.empty());
      Stack.pop_back();
    }

    CstNodeId wrapLast(CstKind Kind)
    {
      assert(!Stack.empty());
      const CstNodeId Parent = Stack.back();
      std::vector<CstElement> &ParentChildren = Nodes[Parent].Children;
      assert(!ParentChildren.empty());
      CstElement Previous = std::move(ParentChildren.back());
      ParentChildren.pop_back();
      const CstNodeId Id = Nodes.size();
      Nodes.push_back({Kind, {std::move(Previous)}});
      Nodes[Parent].Children.push_back(CstNodeRef{Id});
      Stack.push_back(Id);
      return Id;
    }

    void setKind(CstNodeId Id, CstKind Kind)
    {
      Nodes.at(Id).Kind = Kind;
    }

    void token(std::size_t TokenIndex)
    {
      assert(!Stack.empty());
      Nodes[Stack.back()].Children.push_back(CstTokenRef{TokenIndex});
    }

    void missing(MissingToken Token)
    {
      assert(!Stack.empty());
      Nodes[Stack.back()].Children.push_back(std::move(Token));
    }

    Checkpoint checkpoint() const
    {
      assert(!Stack.empty());
      const CstNodeId Parent = Stack.back();
      const std::vector<CstElement> &ParentChildren = Nodes[Parent].Children;
      return {Nodes.size(), Stack, Parent, ParentChildren.size(), ParentChildren.empty() ? std::optional<CstElement>() : std::optional<CstElement>(ParentChildren.back())};
    }

    void restore(Checkpoint State)
    {
      Nodes.resize(State.NodesSize);
      assert(State.Parent < Nodes.size());
      std::vector<CstElement> &ParentChildren = Nodes[State.Parent].Children;
      ParentChildren.resize(State.ParentChildCount);
      if (State.LastChild)
      {
        assert(!ParentChildren.empty());
        ParentChildren.back() = std::move(*State.LastChild);
      }
      Stack = std::move(State.Stack);
    }

    CstTree build(const TokenizedBuffer &LexedFile) const
    {
      CstTree Result;
      Result.Nodes.resize(Nodes.size());
      Result.Root = 0;
      for (std::size_t Index = 0; Index < Nodes.size(); ++Index)
      {
        const TemporaryNode &Temporary = Nodes[Index];
        CstNode &Node = Result.Nodes[Index];
        Node.Kind = Temporary.Kind;
        Node.FirstChild = Result.Children.size();
        Node.ChildCount = Temporary.Children.size();
        Result.Children.insert(Result.Children.end(), Temporary.Children.begin(), Temporary.Children.end());
      }
      struct Frame
      {
        std::size_t NodeIndex = 0;
        std::size_t NextChild = 0;
      };
      std::vector<std::uint8_t> State(Nodes.size(), 0);
      std::vector<Frame> Work;
      for (std::size_t Start = 0; Start < Nodes.size(); ++Start)
      {
        if (State[Start] != 0)
        {
          continue;
        }
        State[Start] = 1;
        Work.push_back({Start, 0});
        while (!Work.empty())
        {
          Frame &Current = Work.back();
          const TemporaryNode &Temporary = Nodes[Current.NodeIndex];
          bool Descended = false;
          while (Current.NextChild < Temporary.Children.size())
          {
            const CstElement &Element = Temporary.Children[Current.NextChild++];
            const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element);
            if (Child == nullptr || State[Child->Id] == 2)
            {
              continue;
            }
            if (State[Child->Id] == 1)
            {
              throw std::logic_error("CST contains a node-reference cycle");
            }
            State[Child->Id] = 1;
            Work.push_back({Child->Id, 0});
            Descended = true;
            break;
          }
          if (Descended)
          {
            continue;
          }

          CstNode &Node = Result.Nodes[Current.NodeIndex];
          if (Node.Kind == CstKind::Error)
          {
            Node.Flags |= CstNodeFlags::HasError;
          }
          for (const CstElement &Element : Temporary.Children)
          {
            if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
            {
              const CstNode &ChildNode = Result.Nodes.at(Child->Id);
              Node.TokenCount += ChildNode.TokenCount;
              Node.TextLength += ChildNode.TextLength;
              Node.Flags |= ChildNode.Flags;
            }
            else if (const CstTokenRef *TokenReference = std::get_if<CstTokenRef>(&Element))
            {
              const Token &ReferencedToken = LexedFile.tokens().at(TokenReference->TokenOffset);
              ++Node.TokenCount;
              Node.TextLength += ReferencedToken.Span.size();
            }
            else
            {
              Node.Flags |= CstNodeFlags::HasMissing;
            }
          }
          State[Current.NodeIndex] = 2;
          Work.pop_back();
        }
      }

      struct OffsetFrame
      {
        CstNodeId NodeId = 0;
        std::size_t NodeStartToken = 0;
        std::size_t NextChild = 0;
        std::size_t ConsumedTokens = 0;
      };
      assert(!Result.Nodes.empty());
      std::vector<std::uint8_t> OffsetState(Result.Nodes.size(), 0);
      std::vector<OffsetFrame> OffsetWork;
      OffsetState[Result.Root] = 1;
      OffsetWork.push_back({Result.Root, 0, 0, 0});
      while (!OffsetWork.empty())
      {
        OffsetFrame &Current = OffsetWork.back();
        const CstNode &Node = Result.Nodes.at(Current.NodeId);
        if (Current.NextChild == Node.ChildCount)
        {
          assert(Current.ConsumedTokens == Node.TokenCount);
          OffsetState[Current.NodeId] = 2;
          OffsetWork.pop_back();
          continue;
        }

        CstElement &Element = Result.Children.at(Node.FirstChild + Current.NextChild++);
        if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
        {
          const CstNode &ChildNode = Result.Nodes.at(Child->Id);
          const std::size_t ChildStartToken = Current.NodeStartToken + Current.ConsumedTokens;
          Current.ConsumedTokens += ChildNode.TokenCount;
          if (OffsetState[Child->Id] == 1)
          {
            throw std::logic_error("CST contains a node-reference cycle");
          }
          if (OffsetState[Child->Id] == 0)
          {
            OffsetState[Child->Id] = 1;
            OffsetWork.push_back({Child->Id, ChildStartToken, 0, 0});
          }
        }
        else if (CstTokenRef *TokenReference = std::get_if<CstTokenRef>(&Element))
        {
          assert(TokenReference->TokenOffset == Current.NodeStartToken + Current.ConsumedTokens);
          TokenReference->TokenOffset = Current.ConsumedTokens;
          ++Current.ConsumedTokens;
        }
      }
      for (const std::uint8_t NodeState : OffsetState)
      {
        if (NodeState != 2)
        {
          throw std::logic_error("CST contains a node that is not reachable from the root");
        }
      }
      return Result;
    }

  private:
    std::vector<TemporaryNode> Nodes;
    std::vector<CstNodeId> Stack;
  };

  namespace
  {
    enum class RegionKind
    {
      Statement,
      TopLevel,
      ClassMember,
      InterfaceMember,
      EnumMember,
    };

    enum class DeclarationKind
    {
      Unknown,
      Binding,
      Function,
      Decorator,
      Class,
      Interface,
      Enum,
    };

    struct StopSet
    {
      std::vector<std::string_view> Symbols;
      std::vector<KeywordKind> Keywords;
      bool EndOfFile = false;
      bool MatchArm = false;
    };

    StopSet withKeyword(const StopSet &Original, KeywordKind Keyword)
    {
      StopSet Result = Original;
      Result.Keywords.push_back(Keyword);
      return Result;
    }

    constexpr std::string_view SymbolSequences[] = {
        "...",
        "::<",
        "<<=",
        ">>=",
        "++",
        "--",
        "..",
        "->",
        "=>",
        "<=",
        ">=",
        "==",
        "!=",
        "<<",
        ">>",
        "&&",
        "||",
        "+=",
        "-=",
        "*=",
        "/=",
        "%=",
        "&=",
        "|=",
        "^=",
    };

    constexpr std::string_view AssignmentOperators[] = {
        "=",
        "+=",
        "-=",
        "*=",
        "/=",
        "%=",
        "&=",
        "|=",
        "^=",
        "<<=",
        ">>=",
    };

    constexpr std::string_view ComparisonOperators[] = {
        "<",
        "<=",
        ">",
        ">=",
        "==",
        "!=",
    };

    bool contains(std::string_view Value, const std::string_view *Begin, const std::string_view *End)
    {
      return std::find(Begin, End, Value) != End;
    }

    char closingDelimiter(char Opening)
    {
      if (Opening == '(')
      {
        return ')';
      }
      if (Opening == '[')
      {
        return ']';
      }
      return '}';
    }

    class SyntaxNestingGuard
    {
    public:
      explicit SyntaxNestingGuard(std::size_t &Depth)
          : Depth(Depth)
      {
        ++Depth;
      }

      ~SyntaxNestingGuard()
      {
        --Depth;
      }

      SyntaxNestingGuard(const SyntaxNestingGuard &) = delete;
      SyntaxNestingGuard &operator=(const SyntaxNestingGuard &) = delete;

    private:
      std::size_t &Depth;
    };

    struct MatchArmBoundaryState
    {
      std::size_t DelimiterDepth = 0;
      std::size_t BoundaryDepth = 0;
    };

    class MatchArmBoundaryGuard
    {
    public:
      MatchArmBoundaryGuard(std::vector<MatchArmBoundaryState> &States, bool Active, std::size_t BoundaryDepth)
          : States(States), Active(Active)
      {
        if (Active)
        {
          States.push_back({0, BoundaryDepth});
        }
      }

      ~MatchArmBoundaryGuard()
      {
        if (Active)
        {
          assert(!States.empty());
          States.pop_back();
        }
      }

      MatchArmBoundaryGuard(const MatchArmBoundaryGuard &) = delete;
      MatchArmBoundaryGuard &operator=(const MatchArmBoundaryGuard &) = delete;

    private:
      std::vector<MatchArmBoundaryState> &States;
      bool Active;
    };
  } // namespace

  class ParserImpl
  {
  public:
    ParserImpl(const TokenizedBuffer &LexedFile, ParserOptions Options)
        : LexedFile(LexedFile), Options(Options)
    {
    }

    CstTree run();

    std::vector<Diagnostic> takeDiagnostics()
    {
      return std::move(Diagnostics);
    }

    ParseCompleteness completeness() const noexcept
    {
      return Completeness;
    }

  private:
    struct Checkpoint
    {
      std::size_t RawIndex = 0;
      CstBuilder::Checkpoint BuilderState;
      std::size_t DiagnosticCount = 0;
      ParseCompleteness Completeness = ParseCompleteness::Complete;
      bool SawDefinitiveError = false;
      std::vector<MatchArmBoundaryState> MatchArmBoundaries;
    };

    const Token &peekRaw(std::size_t Offset = 0) const;
    std::size_t significantIndex(std::size_t Offset = 0) const;
    const Token &peekSignificant(std::size_t Offset = 0) const;
    std::string_view raw(const Token &TokenValue) const;
    bool atMatchArmBoundary() const;
    void trackMatchArmDelimiter(const Token &TokenValue);
    bool atEnd() const;
    bool atToken(TokenKind Kind) const;
    bool atKeyword(KeywordKind Kind) const;
    bool atIdentifier() const;
    bool atIdentifierSpelling(std::string_view Spelling) const;
    bool atBuiltinType() const;
    bool rawSymbolAt(std::size_t Index, char Symbol) const;
    bool symbolRunMatches(std::size_t Index, std::string_view Sequence) const;
    std::string longestSymbolSequenceAt(std::size_t Index) const;
    std::string longestSymbolSequence() const;
    bool atSymbols(std::string_view Sequence) const;
    bool atSingleSymbol(char Symbol) const;
    bool atTypeSymbol(char Symbol) const;
    bool atStop(const StopSet &Stop) const;
    bool isExpressionStart(std::size_t Offset = 0) const;
    bool isTypeStart(std::size_t Offset = 0) const;
    bool isPatternStart() const;
    bool isStatementStart() const;
    bool isTopLevelStart() const;
    bool isMemberStart() const;
    bool isClassTypeExpressionStart(std::size_t Offset = 0) const;
    bool shouldCommitClassTypeExpression();
    bool isNamedArgumentStart() const;
    bool isMatchArmStart() const;
    bool isIfStatementStart() const;
    bool isUnsuffixedDecimalInteger(const Token &TokenValue) const;
    DeclarationKind classifyDeclaration(std::size_t Offset = 0) const;
    bool hasIncompleteDeclarationPrefix() const;

    CstNodeId startNode(CstKind Kind);
    CstNodeId wrapLast(CstKind Kind);
    void finishNode();
    void flushTrivia();
    void consumeCurrent();
    bool consumeToken(TokenKind Kind);
    bool consumeKeyword(KeywordKind Kind);
    bool consumeSymbols(std::string_view Sequence);
    bool consumeSingleSymbol(char Symbol);
    void consumeOperator(std::string_view Sequence);
    void expectToken(TokenKind Kind, std::string_view Expected);
    void expectKeyword(KeywordKind Kind, std::string_view Expected);
    void expectSymbols(std::string_view Sequence);
    void expectSingleSymbol(char Symbol);
    void addMissing(TokenKind Kind, std::string_view Expected, bool MarksIncompleteAtEof = true);
    void addExpectedSyntax(std::string_view Expected);
    void addUnexpected(DiagnosticKind Kind = DiagnosticKind::UnexpectedToken);
    void consumeUnexpected(DiagnosticKind Kind = DiagnosticKind::UnexpectedToken);
    void consumeUnexpectedSymbols(DiagnosticKind Kind = DiagnosticKind::UnexpectedToken);
    bool consumeUnexpectedCommas();
    void recoverSyntaxNesting();
    std::size_t anchorOffset() const;
    Checkpoint checkpoint() const;
    void restore(Checkpoint State);

    void parseSourceFile();
    void parseTopLevelItem(bool MatchArmBody = false);
    void parseImportDeclaration();
    void parseModulePath();
    void parseImportAlias();
    void parseImportedMember();
    void parseIncompleteDeclarationPrefix();
    void parseTopLevelDeclaration(DeclarationKind Kind);
    void parseBindingDeclaration(bool TopLevel, const StopSet &Stop = {});
    void parseBindingCore(const StopSet &Stop);
    void parseAttributeList();
    void parseAttributeApplication();
    void parseDecoratorApplication();
    void parseApplicationArgumentClause();
    void parseFunctionDeclaration(bool Decorator);
    void parseFunctionModifier();
    void parseFunctionName();
    void parseGenericParameterClause();
    void parseGenericParameter();
    void parseFunctionParameterClause();
    void parseFunctionParameter();
    void parseReturnClause(const StopSet &Stop);
    void parseFunctionBody();
    void parseConstructorInitializerClause();
    void parseConstructorInitializer();
    void parseConstructorInitializerTarget();
    void parseTypeDeclaration(DeclarationKind Kind, bool ExpressionClass = false);
    void parseInvalidDeclarationPrefixElement();
    void parseTypeDeclarationPrefix();
    void parseInheritanceClause();
    void parseClassMemberBlock(bool Interface, bool MatchArmBody = false);
    void parseClassMemberItem(bool Interface, bool MatchArmBody = false);
    void parseFieldDeclaration(const StopSet &Stop = {});
    void parseEnumMemberBlock(bool MatchArmBody = false);
    void parseEnumMemberItem(bool MatchArmBody = false);
    void parseEnumBranch(const StopSet &Stop = {});

    void parseComptimeRegion(RegionKind Region);
    void parseRegionBlock(RegionKind Region, bool MatchArmBody = false);
    void parseRegionIfTail(RegionKind Region);
    void parseRegionMatchTail(RegionKind Region);
    void parseRegionForTail(RegionKind Region);
    void parseRegionWhileTail(RegionKind Region);

    void parseStatementBlock(bool MatchArmBody = false);
    void parseBlockItem(bool MatchArmBody = false);
    void parseStatement(bool MatchArmBody = false);
    void parseExpressionOrAssignmentStatement(bool MatchArmBody = false);
    void parseIfStatement();
    void parseIfCondition(CstKind MatchConditionKind = CstKind::MatchCondition);
    void parseMatchStatement();
    void parseMatchStatementArm();
    void parseWhileStatement();
    void parseWhileCondition();
    void parseForStatement();
    void parseForHeader();
    void parseBreakStatement();
    void parseContinueStatement();
    void parseReturnStatement(bool MatchArmBody = false);
    void parseDeferStatement(bool MatchArmBody = false);
    void parseThrowStatement(bool MatchArmBody = false);
    void parseTryStatement();
    void parseCatchClause(bool CatchAll);

    void parsePayloadPattern();
    void parseTuplePattern();
    void parseVariantPattern();
    void parseConditionalMatchPattern();
    void parseMatchArmPattern();
    void parseForPattern();

    void parseExpression(const StopSet &Stop);
    void parseIfExpression(const StopSet &Stop, bool Generic);
    void parseGenericArgumentExpression(const StopSet &Stop);
    void parseLogicalOrExpression(const StopSet &Stop, bool Generic);
    void parseLogicalAndExpression(const StopSet &Stop, bool Generic);
    void parseComparisonExpression(const StopSet &Stop, bool Generic);
    void parseBitwiseOrExpression(const StopSet &Stop, bool Generic);
    void parseBitwiseXorExpression(const StopSet &Stop, bool Generic);
    void parseBitwiseAndExpression(const StopSet &Stop, bool Generic);
    void parseShiftExpression(const StopSet &Stop, bool Generic);
    void parseAdditiveExpression(const StopSet &Stop);
    void parseMultiplicativeExpression(const StopSet &Stop);
    void parseUnaryExpression(const StopSet &Stop);
    void parsePostfixExpression(const StopSet &Stop);
    void parsePrimaryExpression(const StopSet &Stop);
    void parseParenthesizedExpression();
    void parseArrayExpression();
    void parseMatchExpression();
    void parseMatchExpressionArm();
    bool parsePostfixSuffix(const StopSet &Stop, bool TypeContext);
    void parseCallSuffix(bool AllowForwardAll = true);
    void parseArgumentList();
    void parsePositionalArgument();
    void parseNamedArgument();
    void parseListExpansion(const StopSet &Stop, bool Generic);
    bool parseIndexOrSliceSuffix();
    void parseMemberSuffix(bool Pointer);
    void parseGenericArgumentClause();
    void parseGenericArgument();
    void parseAggregateInitializationSuffix(const StopSet &Stop);
    void parseTypeConstructorTail(const StopSet &Stop);

    void parseType(const StopSet &Stop);
    void parsePostfixableTypePrimary(const StopSet &Stop);
    void parseTupleType();
    void parseFunctionType(const StopSet &Stop);
    void parseFunctionTypeParameter();
    void parseTypeSymbolSuffix();
    void parseEmptyBracketSuffix();
    bool parenthesisContainsTopLevelCommaOrExpansion() const;

    const TokenizedBuffer &LexedFile;
    ParserOptions Options;
    std::size_t RawIndex = 0;
    CstBuilder Builder;
    std::vector<Diagnostic> Diagnostics;
    ParseCompleteness Completeness = ParseCompleteness::Complete;
    bool SawDefinitiveError = false;
    std::size_t SyntaxNestingDepth = 0;
    std::vector<MatchArmBoundaryState> MatchArmBoundaries;
  };
  const Token &ParserImpl::peekRaw(std::size_t Offset) const
  {
    const std::size_t Index = std::min(RawIndex + Offset, LexedFile.tokens().size() - 1);
    return LexedFile.tokens()[Index];
  }

  std::size_t ParserImpl::significantIndex(std::size_t Offset) const
  {
    std::size_t Index = RawIndex;
    while (Index < LexedFile.tokens().size())
    {
      if (!LexedFile.tokens()[Index].isTrivia())
      {
        if (Offset == 0)
        {
          return Index;
        }
        --Offset;
      }
      ++Index;
    }
    return LexedFile.tokens().size() - 1;
  }

  const Token &ParserImpl::peekSignificant(std::size_t Offset) const
  {
    return LexedFile.tokens()[significantIndex(Offset)];
  }

  std::string_view ParserImpl::raw(const Token &TokenValue) const
  {
    return LexedFile.raw(TokenValue);
  }

  bool ParserImpl::atMatchArmBoundary() const
  {
    return !MatchArmBoundaries.empty() && MatchArmBoundaries.back().DelimiterDepth == MatchArmBoundaries.back().BoundaryDepth && isMatchArmStart();
  }

  void ParserImpl::trackMatchArmDelimiter(const Token &TokenValue)
  {
    if (TokenValue.Kind != TokenKind::Symbol)
    {
      return;
    }
    const char *Symbol = std::get_if<char>(&TokenValue.Payload);
    if (Symbol == nullptr)
    {
      return;
    }
    const bool Opening = *Symbol == '(' || *Symbol == '[' || *Symbol == '{';
    const bool Closing = *Symbol == ')' || *Symbol == ']' || *Symbol == '}';
    if (!Opening && !Closing)
    {
      return;
    }
    for (MatchArmBoundaryState &State : MatchArmBoundaries)
    {
      if (Opening)
      {
        ++State.DelimiterDepth;
      }
      else if (State.DelimiterDepth != 0)
      {
        --State.DelimiterDepth;
      }
    }
  }

  bool ParserImpl::atEnd() const
  {
    return peekSignificant().Kind == TokenKind::EndOfFile;
  }

  bool ParserImpl::atToken(TokenKind Kind) const
  {
    return !atMatchArmBoundary() && peekSignificant().Kind == Kind;
  }

  bool ParserImpl::atKeyword(KeywordKind Kind) const
  {
    const Token &Current = peekSignificant();
    const KeywordKind *Keyword = std::get_if<KeywordKind>(&Current.Payload);
    return Current.Kind == TokenKind::Keyword && Keyword != nullptr && *Keyword == Kind;
  }

  bool ParserImpl::atIdentifier() const
  {
    return atToken(TokenKind::Identifier);
  }

  bool ParserImpl::atIdentifierSpelling(std::string_view Spelling) const
  {
    return atIdentifier() && raw(peekSignificant()) == Spelling;
  }

  bool ParserImpl::atBuiltinType() const
  {
    return atToken(TokenKind::BuiltinType);
  }

  bool ParserImpl::rawSymbolAt(std::size_t Index, char Symbol) const
  {
    if (Index >= LexedFile.tokens().size())
    {
      return false;
    }
    const Token &Current = LexedFile.tokens()[Index];
    const char *Value = std::get_if<char>(&Current.Payload);
    return Current.Kind == TokenKind::Symbol && Value != nullptr && *Value == Symbol;
  }

  bool ParserImpl::symbolRunMatches(std::size_t Index, std::string_view Sequence) const
  {
    if (Sequence.empty() || Index + Sequence.size() > LexedFile.tokens().size())
    {
      return false;
    }
    for (std::size_t Offset = 0; Offset < Sequence.size(); ++Offset)
    {
      if (!rawSymbolAt(Index + Offset, Sequence[Offset]))
      {
        return false;
      }
      if (Offset > 0 && LexedFile.tokens()[Index + Offset - 1].Span.End != LexedFile.tokens()[Index + Offset].Span.Start)
      {
        return false;
      }
    }
    return true;
  }

  std::string ParserImpl::longestSymbolSequenceAt(std::size_t Index) const
  {
    if (Index >= LexedFile.tokens().size() || LexedFile.tokens()[Index].Kind != TokenKind::Symbol)
    {
      return {};
    }
    for (std::string_view Sequence : SymbolSequences)
    {
      if (symbolRunMatches(Index, Sequence))
      {
        return std::string(Sequence);
      }
    }
    const char *Value = std::get_if<char>(&LexedFile.tokens()[Index].Payload);
    return Value == nullptr ? std::string() : std::string(1, *Value);
  }

  std::string ParserImpl::longestSymbolSequence() const
  {
    return longestSymbolSequenceAt(significantIndex());
  }

  bool ParserImpl::atSymbols(std::string_view Sequence) const
  {
    return !atMatchArmBoundary() && longestSymbolSequence() == Sequence;
  }

  bool ParserImpl::atSingleSymbol(char Symbol) const
  {
    return !atMatchArmBoundary() && rawSymbolAt(significantIndex(), Symbol);
  }

  bool ParserImpl::atTypeSymbol(char Symbol) const
  {
    if (atMatchArmBoundary())
    {
      return false;
    }
    const std::size_t Index = significantIndex();
    if (!rawSymbolAt(Index, Symbol))
    {
      return false;
    }
    if ((Symbol == '*' || Symbol == '&') && Index + 1 < LexedFile.tokens().size() && rawSymbolAt(Index + 1, '=') && LexedFile.tokens()[Index].Span.End == LexedFile.tokens()[Index + 1].Span.Start)
    {
      return false;
    }
    return true;
  }

  bool ParserImpl::atStop(const StopSet &Stop) const
  {
    if (atMatchArmBoundary())
    {
      return true;
    }
    if (Stop.EndOfFile && atEnd())
    {
      return true;
    }
    for (KeywordKind Keyword : Stop.Keywords)
    {
      if (atKeyword(Keyword))
      {
        return true;
      }
    }
    for (std::string_view Symbol : Stop.Symbols)
    {
      if ((Symbol == ">" && atSingleSymbol('>')) || atSymbols(Symbol))
      {
        return true;
      }
    }
    if (Stop.MatchArm && isMatchArmStart())
    {
      return true;
    }
    return false;
  }

  bool ParserImpl::isExpressionStart(std::size_t Offset) const
  {
    if (Offset == 0 && atMatchArmBoundary())
    {
      return false;
    }
    if (isClassTypeExpressionStart(Offset))
    {
      return true;
    }
    const Token &Current = peekSignificant(Offset);
    switch (Current.Kind)
    {
    case TokenKind::Identifier:
    case TokenKind::BuiltinType:
    case TokenKind::BoolLiteral:
    case TokenKind::NullLiteral:
    case TokenKind::IntegerLiteral:
    case TokenKind::FloatLiteral:
    case TokenKind::ScalarLiteral:
    case TokenKind::StringLiteral:
      return true;
    default:
      break;
    }
    if (Current.Kind == TokenKind::Keyword)
    {
      const KeywordKind Kind = std::get<KeywordKind>(Current.Payload);
      return Kind == KeywordKind::This || Kind == KeywordKind::If || Kind == KeywordKind::Match || Kind == KeywordKind::Const || Kind == KeywordKind::Func || Kind == KeywordKind::Async || Kind == KeywordKind::Class || Kind == KeywordKind::Comptime || Kind == KeywordKind::Await;
    }
    if (Current.Kind != TokenKind::Symbol)
    {
      return false;
    }
    const std::size_t Index = significantIndex(Offset);
    const std::string Sequence = longestSymbolSequenceAt(Index);
    return Sequence == "(" || Sequence == "[" || Sequence == "+" || Sequence == "-" || Sequence == "!" || Sequence == "~" || Sequence == "*" || Sequence == "&" || Sequence == "++" || Sequence == "--";
  }

  bool ParserImpl::isTypeStart(std::size_t Offset) const
  {
    if (Offset == 0 && atMatchArmBoundary())
    {
      return false;
    }
    const Token &Current = peekSignificant(Offset);
    if (Current.Kind == TokenKind::Identifier || Current.Kind == TokenKind::BuiltinType)
    {
      return true;
    }
    if (Current.Kind == TokenKind::Keyword)
    {
      const KeywordKind Kind = std::get<KeywordKind>(Current.Payload);
      return Kind == KeywordKind::Const || Kind == KeywordKind::Async || Kind == KeywordKind::Func;
    }
    return Current.Kind == TokenKind::Symbol && longestSymbolSequenceAt(significantIndex(Offset)) == "(";
  }

  bool ParserImpl::isPatternStart() const
  {
    return atIdentifier() || atSymbols("(") || atSymbols(".");
  }

  bool ParserImpl::isStatementStart() const
  {
    if (atSymbols("{"))
    {
      return true;
    }
    if (peekSignificant().Kind == TokenKind::Keyword)
    {
      const KeywordKind Kind = std::get<KeywordKind>(peekSignificant().Payload);
      switch (Kind)
      {
      case KeywordKind::Var:
      case KeywordKind::Const:
      case KeywordKind::If:
      case KeywordKind::Match:
      case KeywordKind::While:
      case KeywordKind::For:
      case KeywordKind::Break:
      case KeywordKind::Continue:
      case KeywordKind::Return:
      case KeywordKind::Defer:
      case KeywordKind::Throw:
      case KeywordKind::Try:
      case KeywordKind::Comptime:
        return true;
      default:
        break;
      }
    }
    return isExpressionStart();
  }

  bool ParserImpl::isTopLevelStart() const
  {
    return atKeyword(KeywordKind::Import) || atKeyword(KeywordKind::From) || atKeyword(KeywordKind::Comptime) || classifyDeclaration() != DeclarationKind::Unknown;
  }

  bool ParserImpl::isMemberStart() const
  {
    return atKeyword(KeywordKind::Comptime) || classifyDeclaration() != DeclarationKind::Unknown;
  }

  bool ParserImpl::isClassTypeExpressionStart(std::size_t Offset) const
  {
    while (true)
    {
      const Token &Current = peekSignificant(Offset);
      if (Current.Kind != TokenKind::Keyword)
      {
        return false;
      }
      const KeywordKind Kind = std::get<KeywordKind>(Current.Payload);
      if (Kind == KeywordKind::Class)
      {
        return true;
      }
      if (Kind != KeywordKind::Public && Kind != KeywordKind::Protected && Kind != KeywordKind::Private && Kind != KeywordKind::Final)
      {
        return false;
      }
      ++Offset;
    }
  }

  bool ParserImpl::shouldCommitClassTypeExpression()
  {
    if (isClassTypeExpressionStart())
    {
      return true;
    }
    if (!atSymbols("["))
    {
      return false;
    }
    Checkpoint Saved = checkpoint();
    const std::size_t OriginalDiagnosticCount = Diagnostics.size();
    while (atSymbols("["))
    {
      parseAttributeList();
    }
    while (atKeyword(KeywordKind::Public) || atKeyword(KeywordKind::Protected) || atKeyword(KeywordKind::Private) || atKeyword(KeywordKind::Final))
    {
      consumeCurrent();
    }
    const bool Commit = Diagnostics.size() == OriginalDiagnosticCount && atKeyword(KeywordKind::Class);
    restore(std::move(Saved));
    return Commit;
  }

  bool ParserImpl::isNamedArgumentStart() const
  {
    if (peekSignificant().Kind != TokenKind::Identifier)
    {
      return false;
    }
    return longestSymbolSequenceAt(significantIndex(1)) == "=";
  }

  bool ParserImpl::isMatchArmStart() const
  {
    std::size_t Offset = 0;
    const Token &First = peekSignificant();
    if (First.Kind == TokenKind::Identifier && raw(First) == "_")
    {
      ++Offset;
    }
    else if (longestSymbolSequenceAt(significantIndex()) == ".")
    {
      ++Offset;
      if (peekSignificant(Offset).Kind != TokenKind::Identifier)
      {
        return false;
      }
      ++Offset;
      if (longestSymbolSequenceAt(significantIndex(Offset)) == "(")
      {
        std::size_t Depth = 0;
        do
        {
          const Token &Current = peekSignificant(Offset);
          if (Current.Kind == TokenKind::EndOfFile)
          {
            return false;
          }
          const std::string Sequence = longestSymbolSequenceAt(significantIndex(Offset));
          if (Sequence == "(")
          {
            ++Depth;
          }
          else if (Sequence == ")")
          {
            if (Depth == 0)
            {
              return false;
            }
            --Depth;
          }
          else if (Sequence == "]" || Sequence == "}")
          {
            return false;
          }
          ++Offset;
        } while (Depth != 0);
      }
    }
    else
    {
      return false;
    }
    return longestSymbolSequenceAt(significantIndex(Offset)) == "=>";
  }

  bool ParserImpl::isIfStatementStart() const
  {
    if (!atKeyword(KeywordKind::If) || longestSymbolSequenceAt(significantIndex(1)) != "(")
    {
      return true;
    }
    std::vector<char> Closers;
    std::size_t Offset = 1;
    while (true)
    {
      const Token &Current = peekSignificant(Offset);
      if (Current.Kind == TokenKind::EndOfFile)
      {
        return true;
      }
      const std::string Sequence = longestSymbolSequenceAt(significantIndex(Offset));
      if (Sequence == "(" || Sequence == "[" || Sequence == "{")
      {
        Closers.push_back(closingDelimiter(Sequence.front()));
      }
      else if (Sequence == ")" || Sequence == "]" || Sequence == "}")
      {
        if (Closers.empty() || Closers.back() != Sequence.front())
        {
          return true;
        }
        Closers.pop_back();
        if (Closers.empty())
        {
          return longestSymbolSequenceAt(significantIndex(Offset + 1)) == "{";
        }
      }
      ++Offset;
    }
  }

  bool ParserImpl::isUnsuffixedDecimalInteger(const Token &TokenValue) const
  {
    if (TokenValue.Kind != TokenKind::IntegerLiteral)
    {
      return false;
    }
    const tokenizer::NumericInfo *Info = std::get_if<tokenizer::NumericInfo>(&TokenValue.Payload);
    return Info != nullptr && Info->Base == 10 && Info->Suffix == tokenizer::NumericSuffix::None;
  }

  DeclarationKind ParserImpl::classifyDeclaration(std::size_t Offset) const
  {
    std::size_t CurrentOffset = Offset;
    while (true)
    {
      const Token &Current = peekSignificant(CurrentOffset);
      if (Current.Kind == TokenKind::EndOfFile)
      {
        return DeclarationKind::Unknown;
      }
      if (Current.Kind == TokenKind::Keyword)
      {
        const KeywordKind Kind = std::get<KeywordKind>(Current.Payload);
        switch (Kind)
        {
        case KeywordKind::Var:
        case KeywordKind::Const:
          return DeclarationKind::Binding;
        case KeywordKind::Func:
          return DeclarationKind::Function;
        case KeywordKind::Decorator:
          return DeclarationKind::Decorator;
        case KeywordKind::Class:
          return DeclarationKind::Class;
        case KeywordKind::Interface:
          return DeclarationKind::Interface;
        case KeywordKind::Enum:
          return DeclarationKind::Enum;
        case KeywordKind::Public:
        case KeywordKind::Protected:
        case KeywordKind::Private:
        case KeywordKind::Static:
        case KeywordKind::Virtual:
        case KeywordKind::Override:
        case KeywordKind::Final:
        case KeywordKind::Async:
        case KeywordKind::Implicit:
          ++CurrentOffset;
          continue;
        case KeywordKind::Extern:
          ++CurrentOffset;
          if (peekSignificant(CurrentOffset).Kind == TokenKind::StringLiteral)
          {
            ++CurrentOffset;
          }
          continue;
        default:
          return DeclarationKind::Unknown;
        }
      }
      const std::string Sequence = longestSymbolSequenceAt(significantIndex(CurrentOffset));
      if (Sequence == "[")
      {
        std::size_t Depth = 0;
        do
        {
          const std::string Nested = longestSymbolSequenceAt(significantIndex(CurrentOffset));
          if (Nested == "[")
          {
            ++Depth;
          }
          else if (Nested == "]" && Depth > 0)
          {
            --Depth;
          }
          ++CurrentOffset;
          if (peekSignificant(CurrentOffset).Kind == TokenKind::EndOfFile)
          {
            return DeclarationKind::Unknown;
          }
        } while (Depth != 0);
        continue;
      }
      if (Sequence == "@")
      {
        ++CurrentOffset;
        if (peekSignificant(CurrentOffset).Kind == TokenKind::Identifier)
        {
          ++CurrentOffset;
        }
        if (longestSymbolSequenceAt(significantIndex(CurrentOffset)) == "(")
        {
          std::size_t Depth = 0;
          do
          {
            const std::string Nested = longestSymbolSequenceAt(significantIndex(CurrentOffset));
            if (Nested == "(")
            {
              ++Depth;
            }
            else if (Nested == ")" && Depth > 0)
            {
              --Depth;
            }
            ++CurrentOffset;
            if (peekSignificant(CurrentOffset).Kind == TokenKind::EndOfFile)
            {
              return DeclarationKind::Unknown;
            }
          } while (Depth != 0);
        }
        continue;
      }
      return DeclarationKind::Unknown;
    }
  }

  bool ParserImpl::hasIncompleteDeclarationPrefix() const
  {
    enum class DelimiterScan
    {
      Closed,
      Incomplete,
      Invalid,
    };

    std::size_t CurrentOffset = 0;
    bool SawPrefix = false;
    auto scanDelimited = [&](char Opening) -> DelimiterScan
    {
      std::vector<char> Closers;
      Closers.push_back(closingDelimiter(Opening));
      ++CurrentOffset;
      while (!Closers.empty())
      {
        if (peekSignificant(CurrentOffset).Kind == TokenKind::EndOfFile)
        {
          return DelimiterScan::Incomplete;
        }
        const std::string Sequence = longestSymbolSequenceAt(significantIndex(CurrentOffset));
        if (Sequence == "(" || Sequence == "[" || Sequence == "{")
        {
          Closers.push_back(closingDelimiter(Sequence.front()));
        }
        else if (Sequence == ")" || Sequence == "]" || Sequence == "}")
        {
          if (Sequence.front() != Closers.back())
          {
            return DelimiterScan::Invalid;
          }
          Closers.pop_back();
        }
        ++CurrentOffset;
      }
      return DelimiterScan::Closed;
    };

    while (true)
    {
      const Token &Current = peekSignificant(CurrentOffset);
      if (Current.Kind == TokenKind::EndOfFile)
      {
        return SawPrefix;
      }
      const std::string Sequence = longestSymbolSequenceAt(significantIndex(CurrentOffset));
      if (Sequence == "[")
      {
        SawPrefix = true;
        const DelimiterScan Result = scanDelimited('[');
        if (Result != DelimiterScan::Closed)
        {
          return Result == DelimiterScan::Incomplete;
        }
        continue;
      }
      if (Sequence == "@")
      {
        SawPrefix = true;
        ++CurrentOffset;
        if (peekSignificant(CurrentOffset).Kind == TokenKind::EndOfFile)
        {
          return true;
        }
        if (peekSignificant(CurrentOffset).Kind != TokenKind::Identifier)
        {
          return false;
        }
        ++CurrentOffset;
        if (longestSymbolSequenceAt(significantIndex(CurrentOffset)) == "(")
        {
          const DelimiterScan Result = scanDelimited('(');
          if (Result != DelimiterScan::Closed)
          {
            return Result == DelimiterScan::Incomplete;
          }
        }
        continue;
      }
      if (Current.Kind != TokenKind::Keyword)
      {
        return false;
      }
      const KeywordKind Kind = std::get<KeywordKind>(Current.Payload);
      const bool Modifier = Kind == KeywordKind::Public || Kind == KeywordKind::Protected || Kind == KeywordKind::Private || Kind == KeywordKind::Extern || Kind == KeywordKind::Static || Kind == KeywordKind::Virtual || Kind == KeywordKind::Override || Kind == KeywordKind::Final || Kind == KeywordKind::Async || Kind == KeywordKind::Implicit;
      if (!Modifier)
      {
        return false;
      }
      SawPrefix = true;
      ++CurrentOffset;
      if (Kind == KeywordKind::Extern && peekSignificant(CurrentOffset).Kind == TokenKind::StringLiteral)
      {
        ++CurrentOffset;
      }
    }
  }

  CstNodeId ParserImpl::startNode(CstKind Kind)
  {
    flushTrivia();
    return Builder.start(Kind);
  }

  CstNodeId ParserImpl::wrapLast(CstKind Kind)
  {
    return Builder.wrapLast(Kind);
  }

  void ParserImpl::finishNode()
  {
    Builder.finish();
  }

  void ParserImpl::flushTrivia()
  {
    while (RawIndex < LexedFile.tokens().size() && LexedFile.tokens()[RawIndex].isTrivia())
    {
      Builder.token(RawIndex);
      ++RawIndex;
    }
  }

  void ParserImpl::consumeCurrent()
  {
    if (atMatchArmBoundary())
    {
      return;
    }
    flushTrivia();
    if (RawIndex < LexedFile.tokens().size())
    {
      trackMatchArmDelimiter(LexedFile.tokens()[RawIndex]);
      Builder.token(RawIndex);
      ++RawIndex;
    }
  }

  bool ParserImpl::consumeToken(TokenKind Kind)
  {
    if (!atToken(Kind))
    {
      return false;
    }
    consumeCurrent();
    return true;
  }

  bool ParserImpl::consumeKeyword(KeywordKind Kind)
  {
    if (!atKeyword(Kind))
    {
      return false;
    }
    consumeCurrent();
    return true;
  }

  bool ParserImpl::consumeSymbols(std::string_view Sequence)
  {
    if (!atSymbols(Sequence))
    {
      return false;
    }
    flushTrivia();
    for (std::size_t Index = 0; Index < Sequence.size(); ++Index)
    {
      trackMatchArmDelimiter(LexedFile.tokens()[RawIndex]);
      Builder.token(RawIndex);
      ++RawIndex;
    }
    return true;
  }

  bool ParserImpl::consumeSingleSymbol(char Symbol)
  {
    if (!atSingleSymbol(Symbol))
    {
      return false;
    }
    consumeCurrent();
    return true;
  }

  void ParserImpl::consumeOperator(std::string_view Sequence)
  {
    startNode(CstKind::Operator);
    consumeSymbols(Sequence);
    finishNode();
  }

  void ParserImpl::expectToken(TokenKind Kind, std::string_view Expected)
  {
    if (!consumeToken(Kind))
    {
      addMissing(Kind, Expected);
    }
  }

  void ParserImpl::expectKeyword(KeywordKind Kind, std::string_view Expected)
  {
    if (!consumeKeyword(Kind))
    {
      addMissing(TokenKind::Keyword, Expected);
    }
  }

  void ParserImpl::expectSymbols(std::string_view Sequence)
  {
    if (!consumeSymbols(Sequence))
    {
      addMissing(TokenKind::Symbol, Sequence);
    }
  }

  void ParserImpl::expectSingleSymbol(char Symbol)
  {
    if (!consumeSingleSymbol(Symbol))
    {
      addMissing(TokenKind::Symbol, std::string(1, Symbol));
    }
  }

  void ParserImpl::addMissing(TokenKind Kind, std::string_view Expected, bool MarksIncompleteAtEof)
  {
    const std::size_t Anchor = anchorOffset();
    Builder.missing({Kind, std::string(Expected), Anchor});
    Diagnostics.push_back(DiagnosticBuilder(DiagnosticKind::ExpectedToken, {Anchor, Anchor}).argument(DiagnosticArgumentName::Expected, std::string(Expected)).build());
    if (!atEnd())
    {
      SawDefinitiveError = true;
    }
    else if (MarksIncompleteAtEof && Options.Mode == ParseMode::Interactive && !SawDefinitiveError)
    {
      Completeness = ParseCompleteness::Incomplete;
    }
  }

  void ParserImpl::addExpectedSyntax(std::string_view Expected)
  {
    const std::size_t Anchor = anchorOffset();
    Builder.missing({TokenKind::Identifier, std::string(Expected), Anchor});
    Diagnostics.push_back(DiagnosticBuilder(DiagnosticKind::ExpectedSyntax, {Anchor, Anchor}).argument(DiagnosticArgumentName::Expected, std::string(Expected)).build());
    if (!atEnd())
    {
      SawDefinitiveError = true;
    }
    else if (Options.Mode == ParseMode::Interactive && !SawDefinitiveError)
    {
      Completeness = ParseCompleteness::Incomplete;
    }
  }

  void ParserImpl::addUnexpected(DiagnosticKind Kind)
  {
    const Token &Current = peekSignificant();
    const std::string Actual = Current.Kind == TokenKind::EndOfFile ? "end of file" : std::string(raw(Current));
    Diagnostics.push_back(DiagnosticBuilder(Kind, Current.Span).argument(DiagnosticArgumentName::Actual, Actual).build());
    if (Current.Kind != TokenKind::EndOfFile)
    {
      SawDefinitiveError = true;
    }
  }

  void ParserImpl::consumeUnexpected(DiagnosticKind Kind)
  {
    if (atMatchArmBoundary())
    {
      return;
    }
    if (atEnd())
    {
      addUnexpected(Kind);
      return;
    }
    startNode(CstKind::Error);
    addUnexpected(Kind);
    consumeCurrent();
    finishNode();
  }

  void ParserImpl::consumeUnexpectedSymbols(DiagnosticKind Kind)
  {
    if (atMatchArmBoundary())
    {
      return;
    }
    if (atEnd() || peekSignificant().Kind != TokenKind::Symbol)
    {
      consumeUnexpected(Kind);
      return;
    }
    const std::string Sequence = longestSymbolSequence();
    startNode(CstKind::Error);
    const std::size_t FirstToken = significantIndex();
    const SourceRange Span = {LexedFile.tokens()[FirstToken].Span.Start, LexedFile.tokens()[FirstToken + Sequence.size() - 1].Span.End};
    Diagnostics.push_back(DiagnosticBuilder(Kind, Span).argument(DiagnosticArgumentName::Actual, Sequence).build());
    SawDefinitiveError = true;
    consumeSymbols(Sequence);
    finishNode();
  }

  bool ParserImpl::consumeUnexpectedCommas()
  {
    bool Consumed = false;
    while (atSymbols(","))
    {
      consumeUnexpected();
      Consumed = true;
    }
    return Consumed;
  }

  void ParserImpl::recoverSyntaxNesting()
  {
    const Token &Current = peekSignificant();
    Diagnostics.push_back(DiagnosticBuilder(DiagnosticKind::SyntaxNestingLimit, Current.Span).build());
    SawDefinitiveError = true;
    startNode(CstKind::Error);
    if (!atEnd())
    {
      const std::size_t FirstIndex = significantIndex();
      char Opening = '\0';
      for (const char Candidate : {'(', '[', '{'})
      {
        if (rawSymbolAt(FirstIndex, Candidate))
        {
          Opening = Candidate;
          break;
        }
      }
      if (Opening == '\0')
      {
        consumeCurrent();
      }
      else
      {
        std::vector<char> Delimiters = {closingDelimiter(Opening)};
        consumeCurrent();
        while (!Delimiters.empty() && !atEnd())
        {
          if (!MatchArmBoundaries.empty() && isMatchArmStart())
          {
            MatchArmBoundaryState &Boundary = MatchArmBoundaries.back();
            Boundary.DelimiterDepth = Boundary.BoundaryDepth;
            break;
          }
          const std::size_t CurrentIndex = significantIndex();
          char NestedOpening = '\0';
          for (const char Candidate : {'(', '[', '{'})
          {
            if (rawSymbolAt(CurrentIndex, Candidate))
            {
              NestedOpening = Candidate;
              break;
            }
          }
          if (NestedOpening != '\0')
          {
            Delimiters.push_back(closingDelimiter(NestedOpening));
            consumeCurrent();
          }
          else if (rawSymbolAt(CurrentIndex, Delimiters.back()))
          {
            consumeCurrent();
            Delimiters.pop_back();
          }
          else
          {
            consumeCurrent();
          }
        }
      }
    }
    finishNode();
  }

  std::size_t ParserImpl::anchorOffset() const
  {
    return peekSignificant().Span.Start;
  }

  ParserImpl::Checkpoint ParserImpl::checkpoint() const
  {
    return {RawIndex, Builder.checkpoint(), Diagnostics.size(), Completeness, SawDefinitiveError, MatchArmBoundaries};
  }

  void ParserImpl::restore(Checkpoint State)
  {
    RawIndex = State.RawIndex;
    Builder.restore(std::move(State.BuilderState));
    Diagnostics.resize(State.DiagnosticCount);
    Completeness = State.Completeness;
    SawDefinitiveError = State.SawDefinitiveError;
    MatchArmBoundaries = std::move(State.MatchArmBoundaries);
  }
  CstTree ParserImpl::run()
  {
    Builder.start(CstKind::SourceFile);
    parseSourceFile();
    Builder.finish();
    return Builder.build(LexedFile);
  }

  void ParserImpl::parseSourceFile()
  {
    while (!atEnd())
    {
      const std::size_t Before = RawIndex;
      parseTopLevelItem();
      if (RawIndex == Before)
      {
        consumeUnexpected();
      }
    }
    expectToken(TokenKind::EndOfFile, "end of file");
  }

  void ParserImpl::parseTopLevelItem(bool MatchArmBody)
  {
    if (atKeyword(KeywordKind::Import) || atKeyword(KeywordKind::From))
    {
      parseImportDeclaration();
      return;
    }
    if (atKeyword(KeywordKind::Comptime))
    {
      parseComptimeRegion(RegionKind::TopLevel);
      return;
    }
    const DeclarationKind Kind = classifyDeclaration();
    if (Kind != DeclarationKind::Unknown)
    {
      if (Kind == DeclarationKind::Binding && MatchArmBody)
      {
        parseBindingDeclaration(true, {{}, {}, false, true});
        return;
      }
      parseTopLevelDeclaration(Kind);
      return;
    }
    if (hasIncompleteDeclarationPrefix())
    {
      parseIncompleteDeclarationPrefix();
      return;
    }
    consumeUnexpected();
  }

  void ParserImpl::parseImportDeclaration()
  {
    const bool MemberImport = atKeyword(KeywordKind::From);
    startNode(MemberImport ? CstKind::MemberImportDeclaration : CstKind::ModuleImportDeclaration);
    if (MemberImport)
    {
      consumeKeyword(KeywordKind::From);
      parseModulePath();
      expectKeyword(KeywordKind::Import, "import");
      consumeUnexpectedCommas();
      parseImportedMember();
      while (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == ";")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        parseImportedMember();
      }
    }
    else
    {
      consumeKeyword(KeywordKind::Import);
      parseModulePath();
      if (atKeyword(KeywordKind::As))
      {
        parseImportAlias();
      }
    }
    expectSymbols(";");
    finishNode();
  }

  void ParserImpl::parseModulePath()
  {
    startNode(CstKind::ModulePath);
    if (atSingleSymbol('.'))
    {
      flushTrivia();
      bool First = true;
      std::size_t PreviousEnd = 0;
      while (!atMatchArmBoundary() && RawIndex < LexedFile.tokens().size() && rawSymbolAt(RawIndex, '.') && (First || LexedFile.tokens()[RawIndex].Span.Start == PreviousEnd))
      {
        First = false;
        PreviousEnd = LexedFile.tokens()[RawIndex].Span.End;
        Builder.token(RawIndex);
        ++RawIndex;
      }
      expectToken(TokenKind::Identifier, "identifier");
    }
    else
    {
      expectToken(TokenKind::Identifier, "identifier");
      expectSymbols(".");
      expectToken(TokenKind::Identifier, "identifier");
    }
    while (atSymbols("."))
    {
      consumeSymbols(".");
      expectToken(TokenKind::Identifier, "identifier");
    }
    finishNode();
  }

  void ParserImpl::parseImportAlias()
  {
    startNode(CstKind::ImportAlias);
    consumeKeyword(KeywordKind::As);
    expectToken(TokenKind::Identifier, "identifier");
    finishNode();
  }

  void ParserImpl::parseImportedMember()
  {
    startNode(CstKind::ImportedMember);
    expectToken(TokenKind::Identifier, "identifier");
    if (atKeyword(KeywordKind::As))
    {
      parseImportAlias();
    }
    finishNode();
  }

  void ParserImpl::parseIncompleteDeclarationPrefix()
  {
    startNode(CstKind::Error);
    while (!atMatchArmBoundary() && !atEnd())
    {
      if (atSymbols("["))
      {
        parseAttributeList();
      }
      else if (atSymbols("@"))
      {
        parseDecoratorApplication();
      }
      else if (atKeyword(KeywordKind::Extern))
      {
        parseFunctionModifier();
      }
      else
      {
        consumeCurrent();
      }
    }
    addExpectedSyntax("declaration");
    finishNode();
  }

  void ParserImpl::parseTopLevelDeclaration(DeclarationKind Kind)
  {
    switch (Kind)
    {
    case DeclarationKind::Binding:
      parseBindingDeclaration(true);
      return;
    case DeclarationKind::Function:
      parseFunctionDeclaration(false);
      return;
    case DeclarationKind::Decorator:
      parseFunctionDeclaration(true);
      return;
    case DeclarationKind::Class:
    case DeclarationKind::Interface:
    case DeclarationKind::Enum:
      parseTypeDeclaration(Kind);
      return;
    case DeclarationKind::Unknown:
      break;
    }
    consumeUnexpected();
  }

  void ParserImpl::parseBindingDeclaration(bool TopLevel, const StopSet &Stop)
  {
    startNode(TopLevel ? CstKind::TopLevelBindingDeclaration : CstKind::LocalBindingDeclaration);
    if (TopLevel)
    {
      while (true)
      {
        if (atKeyword(KeywordKind::Public) || atKeyword(KeywordKind::Protected) || atKeyword(KeywordKind::Private))
        {
          startNode(CstKind::AccessModifier);
          consumeCurrent();
          finishNode();
        }
        else if (atSymbols("[") || atSymbols("@") || atKeyword(KeywordKind::Extern) || atKeyword(KeywordKind::Static) || atKeyword(KeywordKind::Virtual) || atKeyword(KeywordKind::Override) || atKeyword(KeywordKind::Final) || atKeyword(KeywordKind::Async) || atKeyword(KeywordKind::Implicit))
        {
          parseInvalidDeclarationPrefixElement();
        }
        else
        {
          break;
        }
      }
    }
    parseBindingCore(Stop);
    finishNode();
  }

  void ParserImpl::parseBindingCore(const StopSet &Stop)
  {
    StopSet ExpressionStop = Stop;
    ExpressionStop.Symbols.push_back(";");
    StopSet TypeStop = ExpressionStop;
    TypeStop.Symbols.push_back("=");
    const bool IsConst = atKeyword(KeywordKind::Const);
    const bool Tuple = longestSymbolSequenceAt(significantIndex(1)) == "(";
    startNode(Tuple ? CstKind::TupleDestructuringDeclaration : CstKind::NamedBindingDeclaration);
    if (atKeyword(KeywordKind::Var))
    {
      consumeKeyword(KeywordKind::Var);
    }
    else
    {
      expectKeyword(KeywordKind::Const, "const");
    }
    if (atSymbols("("))
    {
      parseTuplePattern();
      expectSymbols("=");
      if (!atStop(ExpressionStop) && isExpressionStart())
      {
        parseExpression(ExpressionStop);
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression");
      }
    }
    else
    {
      expectToken(TokenKind::Identifier, "identifier");
      if (consumeSymbols(":"))
      {
        if (!atStop(TypeStop) && isTypeStart())
        {
          parseType(TypeStop);
        }
        else
        {
          addMissing(TokenKind::Identifier, "type");
        }
        if (consumeSymbols("="))
        {
          if (!atStop(ExpressionStop) && isExpressionStart())
          {
            parseExpression(ExpressionStop);
          }
          else
          {
            addMissing(TokenKind::Identifier, "expression");
          }
        }
        else if (IsConst)
        {
          addMissing(TokenKind::Symbol, "=");
          addMissing(TokenKind::Identifier, "expression");
        }
      }
      else if (consumeSymbols("="))
      {
        if (!atStop(ExpressionStop) && isExpressionStart())
        {
          parseExpression(ExpressionStop);
        }
        else
        {
          addMissing(TokenKind::Identifier, "expression");
        }
      }
      else
      {
        addMissing(TokenKind::Symbol, IsConst ? "=" : ": or =");
        if (IsConst)
        {
          addMissing(TokenKind::Identifier, "expression");
        }
        else
        {
          addMissing(TokenKind::Identifier, "type or expression");
        }
      }
    }
    expectSymbols(";");
    finishNode();
  }

  void ParserImpl::parseAttributeList()
  {
    startNode(CstKind::AttributeList);
    expectSymbols("[");
    consumeUnexpectedCommas();
    if (atSymbols("]"))
    {
      addMissing(TokenKind::Identifier, "attribute");
    }
    else
    {
      parseAttributeApplication();
      while (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == "]")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        parseAttributeApplication();
      }
    }
    expectSymbols("]");
    finishNode();
  }

  void ParserImpl::parseAttributeApplication()
  {
    startNode(CstKind::AttributeApplication);
    expectToken(TokenKind::Identifier, "identifier");
    if (atSymbols("("))
    {
      parseApplicationArgumentClause();
    }
    finishNode();
  }

  void ParserImpl::parseDecoratorApplication()
  {
    startNode(CstKind::DecoratorApplication);
    expectSymbols("@");
    expectToken(TokenKind::Identifier, "identifier");
    if (atSymbols("("))
    {
      parseApplicationArgumentClause();
    }
    finishNode();
  }

  void ParserImpl::parseInvalidDeclarationPrefixElement()
  {
    startNode(CstKind::Error);
    addUnexpected();
    if (atSymbols("["))
    {
      parseAttributeList();
    }
    else if (atSymbols("@"))
    {
      parseDecoratorApplication();
    }
    else
    {
      parseFunctionModifier();
    }
    finishNode();
  }

  void ParserImpl::parseApplicationArgumentClause()
  {
    startNode(CstKind::ApplicationArgumentClause);
    expectSymbols("(");
    if (!atSymbols(")"))
    {
      parseArgumentList();
    }
    expectSymbols(")");
    finishNode();
  }

  void ParserImpl::parseFunctionModifier()
  {
    const bool Extern = atKeyword(KeywordKind::Extern);
    startNode(Extern ? CstKind::ExternModifier : CstKind::FunctionModifier);
    consumeCurrent();
    if (Extern)
    {
      expectToken(TokenKind::StringLiteral, "string literal");
    }
    finishNode();
  }

  void ParserImpl::parseFunctionName()
  {
    const bool Destructor = atSymbols("~");
    startNode(Destructor ? CstKind::DestructorFormFunctionName : CstKind::IdentifierFunctionName);
    if (Destructor)
    {
      consumeSymbols("~");
      expectToken(TokenKind::Identifier, "identifier");
    }
    else
    {
      expectToken(TokenKind::Identifier, "identifier");
    }
    finishNode();
  }

  void ParserImpl::parseFunctionDeclaration(bool Decorator)
  {
    startNode(Decorator ? CstKind::DecoratorDeclaration : CstKind::FunctionDeclaration);
    while (atSymbols("[") || (!Decorator && atSymbols("@")))
    {
      if (atSymbols("["))
      {
        parseAttributeList();
      }
      else
      {
        parseDecoratorApplication();
      }
    }
    while (atKeyword(KeywordKind::Public) || atKeyword(KeywordKind::Protected) || atKeyword(KeywordKind::Private) || atKeyword(KeywordKind::Extern) || atKeyword(KeywordKind::Static) || atKeyword(KeywordKind::Virtual) || atKeyword(KeywordKind::Override) || atKeyword(KeywordKind::Final) || atKeyword(KeywordKind::Async) || atKeyword(KeywordKind::Implicit))
    {
      parseFunctionModifier();
    }
    while (atSymbols("[") || atSymbols("@") || atKeyword(KeywordKind::Public) || atKeyword(KeywordKind::Protected) || atKeyword(KeywordKind::Private) || atKeyword(KeywordKind::Extern) || atKeyword(KeywordKind::Static) || atKeyword(KeywordKind::Virtual) || atKeyword(KeywordKind::Override) || atKeyword(KeywordKind::Final) || atKeyword(KeywordKind::Async) || atKeyword(KeywordKind::Implicit))
    {
      if (atSymbols("[") || atSymbols("@"))
      {
        startNode(CstKind::Error);
        addUnexpected();
        if (atSymbols("["))
        {
          parseAttributeList();
        }
        else
        {
          parseDecoratorApplication();
        }
        finishNode();
      }
      else
      {
        parseFunctionModifier();
      }
    }
    expectKeyword(Decorator ? KeywordKind::Decorator : KeywordKind::Func, Decorator ? "decorator" : "func");
    parseFunctionName();
    if (atSymbols("<"))
    {
      parseGenericParameterClause();
    }
    parseFunctionParameterClause();
    if (atKeyword(KeywordKind::Const))
    {
      startNode(CstKind::ReceiverQualifier);
      consumeKeyword(KeywordKind::Const);
      finishNode();
    }
    while (atKeyword(KeywordKind::Const))
    {
      consumeUnexpected();
    }
    if (atSymbols("->"))
    {
      parseReturnClause({{":", "{", ";"}, {}, false});
    }
    parseFunctionBody();
    finishNode();
  }

  void ParserImpl::parseGenericParameterClause()
  {
    startNode(CstKind::GenericParameterClause);
    expectSymbols("<");
    startNode(CstKind::GenericParameterList);
    consumeUnexpectedCommas();
    if (atSingleSymbol('>'))
    {
      addExpectedSyntax("generic parameter");
    }
    else
    {
      parseGenericParameter();
      while (atSymbols(","))
      {
        if (rawSymbolAt(significantIndex(1), '>'))
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        parseGenericParameter();
      }
    }
    finishNode();
    expectSingleSymbol('>');
    finishNode();
  }

  void ParserImpl::parseGenericParameter()
  {
    startNode(CstKind::GenericParameter);
    expectToken(TokenKind::Identifier, "identifier");
    expectSymbols(":");
    parseType({{",", ">", "=", "..."}, {}, false});
    if (atSymbols("..."))
    {
      startNode(CstKind::ParameterPackSuffix);
      consumeSymbols("...");
      finishNode();
    }
    else if (consumeSymbols("="))
    {
      startNode(CstKind::DefaultArgument);
      parseGenericArgumentExpression({{",", ">"}, {}, false});
      finishNode();
    }
    finishNode();
  }

  void ParserImpl::parseFunctionParameterClause()
  {
    startNode(CstKind::FunctionParameterClause);
    expectSymbols("(");
    consumeUnexpectedCommas();
    if (!atSymbols(")"))
    {
      startNode(CstKind::FunctionParameterList);
      parseFunctionParameter();
      while (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == ")")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        parseFunctionParameter();
      }
      finishNode();
    }
    expectSymbols(")");
    finishNode();
  }

  void ParserImpl::parseFunctionParameter()
  {
    startNode(CstKind::FunctionParameter);
    expectToken(TokenKind::Identifier, "identifier");
    expectSymbols(":");
    parseType({{",", ")", "=", "..."}, {}, false});
    if (atSymbols("..."))
    {
      startNode(CstKind::ParameterPackSuffix);
      consumeSymbols("...");
      finishNode();
    }
    else if (consumeSymbols("="))
    {
      startNode(CstKind::DefaultArgument);
      parseExpression({{",", ")"}, {}, false});
      finishNode();
    }
    finishNode();
  }

  void ParserImpl::parseReturnClause(const StopSet &Stop)
  {
    startNode(CstKind::ReturnClause);
    expectSymbols("->");
    parseType(Stop);
    finishNode();
  }

  void ParserImpl::parseFunctionBody()
  {
    if (consumeSymbols(";"))
    {
      return;
    }
    startNode(CstKind::FunctionDefinition);
    if (atSymbols(":"))
    {
      parseConstructorInitializerClause();
    }
    if (atSymbols("{"))
    {
      parseStatementBlock();
    }
    else
    {
      addMissing(TokenKind::Symbol, "{");
      addMissing(TokenKind::Symbol, "}");
    }
    finishNode();
  }

  void ParserImpl::parseConstructorInitializerClause()
  {
    startNode(CstKind::ConstructorInitializerClause);
    expectSymbols(":");
    consumeUnexpectedCommas();
    parseConstructorInitializer();
    while (atSymbols(","))
    {
      if (longestSymbolSequenceAt(significantIndex(1)) == "{")
      {
        consumeUnexpected(DiagnosticKind::TrailingComma);
        break;
      }
      consumeSymbols(",");
      consumeUnexpectedCommas();
      parseConstructorInitializer();
    }
    finishNode();
  }

  void ParserImpl::parseConstructorInitializer()
  {
    startNode(CstKind::ConstructorInitializer);
    parseConstructorInitializerTarget();
    parseCallSuffix();
    finishNode();
  }

  void ParserImpl::parseConstructorInitializerTarget()
  {
    startNode(CstKind::ConstructorInitializerTarget);
    expectToken(TokenKind::Identifier, "identifier");
    while (atSymbols(".") || atSymbols("::<"))
    {
      if (consumeSymbols("."))
      {
        expectToken(TokenKind::Identifier, "identifier");
      }
      else
      {
        parseGenericArgumentClause();
      }
    }
    finishNode();
  }
  void ParserImpl::parseTypeDeclarationPrefix()
  {
    startNode(CstKind::TypeDeclarationPrefix);
    while (atSymbols("["))
    {
      parseAttributeList();
    }
    while (true)
    {
      if (atKeyword(KeywordKind::Public) || atKeyword(KeywordKind::Protected) || atKeyword(KeywordKind::Private) || atKeyword(KeywordKind::Final))
      {
        const bool Access = atKeyword(KeywordKind::Public) || atKeyword(KeywordKind::Protected) || atKeyword(KeywordKind::Private);
        startNode(Access ? CstKind::AccessModifier : CstKind::TypeModifier);
        consumeCurrent();
        finishNode();
      }
      else if (atSymbols("[") || atSymbols("@") || atKeyword(KeywordKind::Extern) || atKeyword(KeywordKind::Static) || atKeyword(KeywordKind::Virtual) || atKeyword(KeywordKind::Override) || atKeyword(KeywordKind::Async) || atKeyword(KeywordKind::Implicit))
      {
        parseInvalidDeclarationPrefixElement();
      }
      else
      {
        break;
      }
    }
    finishNode();
  }

  void ParserImpl::parseTypeDeclaration(DeclarationKind Kind, bool ExpressionClass)
  {
    if (SyntaxNestingDepth >= Options.MaxSyntaxNestingDepth)
    {
      recoverSyntaxNesting();
      return;
    }
    SyntaxNestingGuard Guard(SyntaxNestingDepth);
    CstKind NodeKind = CstKind::Unknown;
    KeywordKind Introducer = KeywordKind::Class;
    std::string_view Expected = "class";
    if (Kind == DeclarationKind::Class)
    {
      NodeKind = ExpressionClass ? CstKind::ClassTypeExpression : CstKind::ClassDeclaration;
    }
    else if (Kind == DeclarationKind::Interface)
    {
      NodeKind = CstKind::InterfaceDeclaration;
      Introducer = KeywordKind::Interface;
      Expected = "interface";
    }
    else
    {
      NodeKind = CstKind::EnumDeclaration;
      Introducer = KeywordKind::Enum;
      Expected = "enum";
    }
    startNode(NodeKind);
    parseTypeDeclarationPrefix();
    expectKeyword(Introducer, Expected);
    if (!ExpressionClass || atIdentifier())
    {
      expectToken(TokenKind::Identifier, "identifier");
    }
    if (Kind == DeclarationKind::Class)
    {
      startNode(CstKind::ClassDefinitionTail);
    }
    if (atSymbols("<"))
    {
      parseGenericParameterClause();
    }
    if (Kind == DeclarationKind::Enum)
    {
      parseEnumMemberBlock();
    }
    else
    {
      if (atSymbols(":"))
      {
        parseInheritanceClause();
      }
      parseClassMemberBlock(Kind == DeclarationKind::Interface);
    }
    if (Kind == DeclarationKind::Class)
    {
      finishNode();
    }
    finishNode();
  }

  void ParserImpl::parseInheritanceClause()
  {
    startNode(CstKind::InheritanceClause);
    expectSymbols(":");
    consumeUnexpectedCommas();
    parseType({{",", "{"}, {}, false});
    while (true)
    {
      if (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == "{")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        parseType({{",", "{"}, {}, false});
        continue;
      }
      if (!isTypeStart())
      {
        break;
      }
      addMissing(TokenKind::Symbol, ",");
      parseType({{",", "{"}, {}, false});
    }
    finishNode();
  }

  void ParserImpl::parseClassMemberBlock(bool Interface, bool MatchArmBody)
  {
    startNode(Interface ? CstKind::InterfaceMemberBlock : CstKind::ClassMemberBlock);
    expectSymbols("{");
    if (atSymbols(";"))
    {
      consumeUnexpected();
      addMissing(TokenKind::Symbol, "}", false);
      finishNode();
      return;
    }
    while (!atMatchArmBoundary() && !atSymbols("}") && !(MatchArmBody && isMatchArmStart()) && !atEnd())
    {
      const std::size_t Before = RawIndex;
      parseClassMemberItem(Interface, MatchArmBody);
      if (RawIndex == Before)
      {
        consumeUnexpected();
      }
    }
    expectSymbols("}");
    finishNode();
  }

  void ParserImpl::parseClassMemberItem(bool Interface, bool MatchArmBody)
  {
    if (atKeyword(KeywordKind::Comptime))
    {
      parseComptimeRegion(Interface ? RegionKind::InterfaceMember : RegionKind::ClassMember);
      return;
    }
    const DeclarationKind Kind = classifyDeclaration();
    switch (Kind)
    {
    case DeclarationKind::Binding:
      parseFieldDeclaration(MatchArmBody ? StopSet{{}, {}, false, true} : StopSet{});
      return;
    case DeclarationKind::Function:
      parseFunctionDeclaration(false);
      return;
    case DeclarationKind::Class:
    case DeclarationKind::Interface:
    case DeclarationKind::Enum:
      parseTypeDeclaration(Kind);
      return;
    default:
      if (hasIncompleteDeclarationPrefix())
      {
        parseIncompleteDeclarationPrefix();
        return;
      }
      consumeUnexpected();
      return;
    }
  }

  void ParserImpl::parseFieldDeclaration(const StopSet &Stop)
  {
    StopSet ExpressionStop = Stop;
    ExpressionStop.Symbols.push_back(";");
    StopSet TypeStop = ExpressionStop;
    TypeStop.Symbols.push_back("=");
    startNode(CstKind::FieldDeclaration);
    startNode(CstKind::FieldAnnotationSequence);
    while (atSymbols("["))
    {
      parseAttributeList();
    }
    finishNode();
    startNode(CstKind::FieldModifierSequence);
    while (true)
    {
      if (atKeyword(KeywordKind::Public) || atKeyword(KeywordKind::Protected) || atKeyword(KeywordKind::Private))
      {
        startNode(CstKind::AccessModifier);
        consumeCurrent();
        finishNode();
      }
      else if (atSymbols("[") || atSymbols("@") || atKeyword(KeywordKind::Extern) || atKeyword(KeywordKind::Static) || atKeyword(KeywordKind::Virtual) || atKeyword(KeywordKind::Override) || atKeyword(KeywordKind::Final) || atKeyword(KeywordKind::Async) || atKeyword(KeywordKind::Implicit))
      {
        parseInvalidDeclarationPrefixElement();
      }
      else
      {
        break;
      }
    }
    finishNode();
    if (atKeyword(KeywordKind::Var) || atKeyword(KeywordKind::Const))
    {
      consumeCurrent();
    }
    else
    {
      addMissing(TokenKind::Keyword, "var or const");
    }
    expectToken(TokenKind::Identifier, "identifier");
    expectSymbols(":");
    if (!atStop(TypeStop) && isTypeStart())
    {
      parseType(TypeStop);
    }
    else
    {
      addMissing(TokenKind::Identifier, "type");
    }
    if (atSymbols("="))
    {
      startNode(CstKind::FieldInitializer);
      consumeSymbols("=");
      if (!atStop(ExpressionStop) && isExpressionStart())
      {
        parseExpression(ExpressionStop);
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression");
      }
      finishNode();
    }
    expectSymbols(";");
    finishNode();
  }

  void ParserImpl::parseEnumMemberBlock(bool MatchArmBody)
  {
    startNode(CstKind::EnumMemberBlock);
    expectSymbols("{");
    if (atSymbols(";"))
    {
      consumeUnexpected();
      addMissing(TokenKind::Symbol, "}", false);
      finishNode();
      return;
    }
    consumeUnexpectedCommas();
    if (!atMatchArmBoundary() && !atSymbols("}") && !(MatchArmBody && isMatchArmStart()) && !atEnd())
    {
      parseEnumMemberItem(MatchArmBody);
      while (!atMatchArmBoundary() && !atSymbols("}") && !(MatchArmBody && isMatchArmStart()) && !atEnd())
      {
        if (atSymbols(","))
        {
          if (longestSymbolSequenceAt(significantIndex(1)) == "}")
          {
            consumeUnexpected(DiagnosticKind::TrailingComma);
            break;
          }
          consumeSymbols(",");
          consumeUnexpectedCommas();
          if (MatchArmBody && isMatchArmStart())
          {
            break;
          }
          parseEnumMemberItem(MatchArmBody);
        }
        else if (atIdentifier() || atSymbols("[") || atKeyword(KeywordKind::Comptime))
        {
          addMissing(TokenKind::Symbol, ",");
          parseEnumMemberItem(MatchArmBody);
        }
        else
        {
          consumeUnexpected();
        }
      }
    }
    expectSymbols("}");
    finishNode();
  }

  void ParserImpl::parseEnumMemberItem(bool MatchArmBody)
  {
    if (atKeyword(KeywordKind::Comptime))
    {
      parseComptimeRegion(RegionKind::EnumMember);
      return;
    }
    parseEnumBranch(MatchArmBody ? StopSet{{}, {}, false, true} : StopSet{});
  }

  void ParserImpl::parseEnumBranch(const StopSet &Stop)
  {
    startNode(CstKind::EnumBranch);
    while (atSymbols("["))
    {
      parseAttributeList();
    }
    expectToken(TokenKind::Identifier, "identifier");
    if (atSymbols("("))
    {
      startNode(CstKind::EnumPayloadClause);
      consumeSymbols("(");
      consumeUnexpectedCommas();
      if (atSymbols(")"))
      {
        addExpectedSyntax("payload type");
      }
      else
      {
        parseType({{",", ")"}, {}, false});
        while (atSymbols(","))
        {
          if (longestSymbolSequenceAt(significantIndex(1)) == ")")
          {
            consumeUnexpected(DiagnosticKind::TrailingComma);
            break;
          }
          consumeSymbols(",");
          consumeUnexpectedCommas();
          parseType({{",", ")"}, {}, false});
        }
      }
      expectSymbols(")");
      finishNode();
    }
    if (consumeSymbols("="))
    {
      startNode(CstKind::EnumDiscriminantClause);
      StopSet ExpressionStop = Stop;
      ExpressionStop.Symbols.push_back(",");
      ExpressionStop.Symbols.push_back("}");
      if (!atStop(ExpressionStop) && isExpressionStart())
      {
        parseExpression(ExpressionStop);
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression");
      }
      finishNode();
    }
    finishNode();
  }

  void ParserImpl::parseComptimeRegion(RegionKind Region)
  {
    if (SyntaxNestingDepth >= Options.MaxSyntaxNestingDepth)
    {
      recoverSyntaxNesting();
      return;
    }
    SyntaxNestingGuard Guard(SyntaxNestingDepth);
    CstKind Kind = CstKind::ComptimeBlockControl;
    if (atKeyword(KeywordKind::Comptime))
    {
      if (const Token &Operand = peekSignificant(1); Operand.Kind == TokenKind::Keyword)
      {
        switch (std::get<KeywordKind>(Operand.Payload))
        {
        case KeywordKind::If:
          Kind = CstKind::ComptimeIfControl;
          break;
        case KeywordKind::Match:
          Kind = CstKind::ComptimeMatchControl;
          break;
        case KeywordKind::For:
          Kind = CstKind::ComptimeForControl;
          break;
        case KeywordKind::While:
          Kind = CstKind::ComptimeWhileControl;
          break;
        default:
          break;
        }
      }
    }
    startNode(Kind);
    expectKeyword(KeywordKind::Comptime, "comptime");
    if (atSymbols("{"))
    {
      parseRegionBlock(Region);
    }
    else if (atKeyword(KeywordKind::If))
    {
      parseRegionIfTail(Region);
    }
    else if (atKeyword(KeywordKind::Match))
    {
      parseRegionMatchTail(Region);
    }
    else if (atKeyword(KeywordKind::For))
    {
      parseRegionForTail(Region);
    }
    else if (atKeyword(KeywordKind::While))
    {
      parseRegionWhileTail(Region);
    }
    else
    {
      addExpectedSyntax("comptime region operand");
      if (!atEnd())
      {
        consumeUnexpected();
      }
    }
    finishNode();
  }

  void ParserImpl::parseRegionBlock(RegionKind Region, bool MatchArmBody)
  {
    if (MatchArmBody && !atSymbols("{"))
    {
      switch (Region)
      {
      case RegionKind::Statement:
        startNode(CstKind::StatementBlock);
        break;
      case RegionKind::TopLevel:
        startNode(CstKind::TopLevelBlock);
        break;
      case RegionKind::ClassMember:
        startNode(CstKind::ClassMemberBlock);
        break;
      case RegionKind::InterfaceMember:
        startNode(CstKind::InterfaceMemberBlock);
        break;
      case RegionKind::EnumMember:
        startNode(CstKind::EnumMemberBlock);
        break;
      }
      addMissing(TokenKind::Symbol, "{");
      while (!isMatchArmStart() && !atSymbols(",") && !atSymbols(";") && !atSymbols("}") && !atEnd())
      {
        consumeUnexpected();
      }
      addMissing(TokenKind::Symbol, "}");
      finishNode();
      return;
    }
    MatchArmBoundaryGuard Boundary(MatchArmBoundaries, MatchArmBody, 1);
    switch (Region)
    {
    case RegionKind::Statement:
      parseStatementBlock(MatchArmBody);
      return;
    case RegionKind::TopLevel:
      startNode(CstKind::TopLevelBlock);
      expectSymbols("{");
      while (!atMatchArmBoundary() && !atSymbols("}") && !(MatchArmBody && isMatchArmStart()) && !atEnd())
      {
        const std::size_t Before = RawIndex;
        parseTopLevelItem(MatchArmBody);
        if (RawIndex == Before)
        {
          consumeUnexpected();
        }
      }
      expectSymbols("}");
      finishNode();
      return;
    case RegionKind::ClassMember:
      parseClassMemberBlock(false, MatchArmBody);
      return;
    case RegionKind::InterfaceMember:
      parseClassMemberBlock(true, MatchArmBody);
      return;
    case RegionKind::EnumMember:
      parseEnumMemberBlock(MatchArmBody);
      return;
    }
  }

  void ParserImpl::parseRegionIfTail(RegionKind Region)
  {
    std::size_t NestedIfDepth = 0;
    while (true)
    {
      expectKeyword(KeywordKind::If, "if");
      expectSymbols("(");
      parseIfCondition();
      expectSymbols(")");
      parseRegionBlock(Region);
      if (!consumeKeyword(KeywordKind::Else))
      {
        break;
      }
      if (atKeyword(KeywordKind::If))
      {
        if (SyntaxNestingDepth + NestedIfDepth >= Options.MaxSyntaxNestingDepth)
        {
          recoverSyntaxNesting();
          break;
        }
        ++NestedIfDepth;
        continue;
      }
      parseRegionBlock(Region);
      break;
    }
  }

  void ParserImpl::parseRegionMatchTail(RegionKind Region)
  {
    expectKeyword(KeywordKind::Match, "match");
    expectSymbols("(");
    parseExpression({{")"}, {}, false});
    expectSymbols(")");
    expectSymbols("{");
    if (atSymbols("}"))
    {
      addExpectedSyntax("match arm");
    }
    while (!atMatchArmBoundary() && !atSymbols("}") && !atEnd())
    {
      if (atSymbols(",") || atSymbols(";"))
      {
        consumeUnexpected();
        continue;
      }
      const std::size_t Before = RawIndex;
      startNode(CstKind::RegionArm);
      parseMatchArmPattern();
      expectSymbols("=>");
      parseRegionBlock(Region, true);
      while (!isMatchArmStart() && !atSymbols("}") && !atEnd())
      {
        consumeUnexpected();
      }
      finishNode();
      if (RawIndex == Before)
      {
        consumeUnexpected();
      }
    }
    expectSymbols("}");
  }

  void ParserImpl::parseRegionForTail(RegionKind Region)
  {
    expectKeyword(KeywordKind::For, "for");
    parseForHeader();
    parseRegionBlock(Region);
  }

  void ParserImpl::parseRegionWhileTail(RegionKind Region)
  {
    expectKeyword(KeywordKind::While, "while");
    expectSymbols("(");
    parseWhileCondition();
    expectSymbols(")");
    parseRegionBlock(Region);
  }

  void ParserImpl::parsePayloadPattern()
  {
    if (atSymbols("("))
    {
      parseTuplePattern();
      return;
    }
    if (atIdentifier())
    {
      startNode(atIdentifierSpelling("_") ? CstKind::WildcardPattern : CstKind::BindingPattern);
      consumeCurrent();
      finishNode();
      return;
    }
    addMissing(TokenKind::Identifier, "pattern");
  }

  void ParserImpl::parseTuplePattern()
  {
    if (SyntaxNestingDepth >= Options.MaxSyntaxNestingDepth)
    {
      recoverSyntaxNesting();
      return;
    }
    SyntaxNestingGuard Guard(SyntaxNestingDepth);
    startNode(CstKind::TuplePattern);
    expectSymbols("(");
    consumeUnexpectedCommas();
    if (consumeSymbols(")"))
    {
      finishNode();
      return;
    }
    parsePayloadPattern();
    if (!consumeSymbols(","))
    {
      addMissing(TokenKind::Symbol, ",");
      expectSymbols(")");
      finishNode();
      return;
    }
    if (consumeSymbols(")"))
    {
      finishNode();
      return;
    }
    parsePayloadPattern();
    while (atSymbols(","))
    {
      if (longestSymbolSequenceAt(significantIndex(1)) == ")")
      {
        consumeUnexpected(DiagnosticKind::TrailingComma);
        break;
      }
      consumeSymbols(",");
      consumeUnexpectedCommas();
      parsePayloadPattern();
    }
    expectSymbols(")");
    finishNode();
  }

  void ParserImpl::parseVariantPattern()
  {
    startNode(CstKind::VariantPattern);
    expectSymbols(".");
    expectToken(TokenKind::Identifier, "identifier");
    if (consumeSymbols("("))
    {
      consumeUnexpectedCommas();
      if (atSymbols(")"))
      {
        addExpectedSyntax("payload pattern");
      }
      else
      {
        parsePayloadPattern();
        while (atSymbols(","))
        {
          if (longestSymbolSequenceAt(significantIndex(1)) == ")")
          {
            consumeUnexpected(DiagnosticKind::TrailingComma);
            break;
          }
          consumeSymbols(",");
          consumeUnexpectedCommas();
          parsePayloadPattern();
        }
      }
      expectSymbols(")");
    }
    finishNode();
  }

  void ParserImpl::parseConditionalMatchPattern()
  {
    if (atSymbols("."))
    {
      parseVariantPattern();
    }
    else
    {
      addExpectedSyntax("variant pattern");
    }
  }

  void ParserImpl::parseMatchArmPattern()
  {
    if (atSymbols("."))
    {
      parseVariantPattern();
    }
    else if (atIdentifierSpelling("_"))
    {
      startNode(CstKind::WildcardPattern);
      consumeCurrent();
      finishNode();
    }
    else
    {
      addExpectedSyntax("match arm pattern");
      if (!atEnd() && !atSymbols("=>"))
      {
        consumeUnexpected();
      }
    }
  }

  void ParserImpl::parseForPattern()
  {
    if (atIdentifier())
    {
      startNode(atIdentifierSpelling("_") ? CstKind::ForWildcardPattern : CstKind::ForBindingPattern);
      consumeCurrent();
      finishNode();
    }
    else
    {
      addMissing(TokenKind::Identifier, "for pattern");
    }
  }
  void ParserImpl::parseStatementBlock(bool MatchArmBody)
  {
    startNode(CstKind::StatementBlock);
    expectSymbols("{");
    while (!atMatchArmBoundary() && !atSymbols("}") && !(MatchArmBody && isMatchArmStart()) && !atEnd())
    {
      const std::size_t Before = RawIndex;
      parseBlockItem(MatchArmBody);
      if (RawIndex == Before)
      {
        consumeUnexpected();
      }
    }
    expectSymbols("}");
    finishNode();
  }

  void ParserImpl::parseBlockItem(bool MatchArmBody)
  {
    if (atKeyword(KeywordKind::Var) || atKeyword(KeywordKind::Const))
    {
      StopSet DeclarationStop;
      DeclarationStop.MatchArm = MatchArmBody;
      if (MatchArmBody)
      {
        DeclarationStop.Symbols = {",", "}"};
      }
      parseBindingDeclaration(false, DeclarationStop);
      return;
    }
    parseStatement(MatchArmBody);
  }

  void ParserImpl::parseStatement(bool MatchArmBody)
  {
    if (MatchArmBody && isMatchArmStart())
    {
      addExpectedSyntax("statement");
      return;
    }
    if (SyntaxNestingDepth >= Options.MaxSyntaxNestingDepth)
    {
      recoverSyntaxNesting();
      return;
    }
    SyntaxNestingGuard Guard(SyntaxNestingDepth);
    if (atSymbols("{"))
    {
      parseStatementBlock();
      return;
    }
    if (atKeyword(KeywordKind::Comptime))
    {
      const Token &Operand = peekSignificant(1);
      const bool StructuredKeyword = Operand.Kind == TokenKind::Keyword && (std::get<KeywordKind>(Operand.Payload) == KeywordKind::If || std::get<KeywordKind>(Operand.Payload) == KeywordKind::Match || std::get<KeywordKind>(Operand.Payload) == KeywordKind::For || std::get<KeywordKind>(Operand.Payload) == KeywordKind::While);
      const bool StructuredBlock = Operand.Kind == TokenKind::Symbol && longestSymbolSequenceAt(significantIndex(1)) == "{";
      if (StructuredKeyword || StructuredBlock)
      {
        parseComptimeRegion(RegionKind::Statement);
        return;
      }
    }
    if (atKeyword(KeywordKind::If) && isIfStatementStart())
    {
      parseIfStatement();
      return;
    }
    if (atKeyword(KeywordKind::Match))
    {
      parseMatchStatement();
      return;
    }
    if (atKeyword(KeywordKind::While))
    {
      parseWhileStatement();
      return;
    }
    if (atKeyword(KeywordKind::For))
    {
      parseForStatement();
      return;
    }
    if (atKeyword(KeywordKind::Break))
    {
      parseBreakStatement();
      return;
    }
    if (atKeyword(KeywordKind::Continue))
    {
      parseContinueStatement();
      return;
    }
    if (atKeyword(KeywordKind::Return))
    {
      parseReturnStatement(MatchArmBody);
      return;
    }
    if (atKeyword(KeywordKind::Defer))
    {
      parseDeferStatement(MatchArmBody);
      return;
    }
    if (atKeyword(KeywordKind::Throw))
    {
      parseThrowStatement(MatchArmBody);
      return;
    }
    if (atKeyword(KeywordKind::Try))
    {
      parseTryStatement();
      return;
    }
    if (atKeyword(KeywordKind::Var) || atKeyword(KeywordKind::Const))
    {
      const SourceRange Span = peekSignificant().Span;
      Diagnostics.push_back(DiagnosticBuilder(DiagnosticKind::DeclarationRequiresBlock, Span).build());
      SawDefinitiveError = true;
      startNode(CstKind::Error);
      StopSet DeclarationStop;
      DeclarationStop.MatchArm = MatchArmBody;
      if (MatchArmBody)
      {
        DeclarationStop.Symbols = {",", "}"};
      }
      parseBindingDeclaration(false, DeclarationStop);
      finishNode();
      return;
    }
    if (MatchArmBody && (isMatchArmStart() || atSymbols(",") || atSymbols(";") || atSymbols("}")))
    {
      addExpectedSyntax("statement");
      return;
    }
    if (isExpressionStart())
    {
      parseExpressionOrAssignmentStatement(MatchArmBody);
      return;
    }
    addExpectedSyntax("statement");
    if (!atEnd() && !atSymbols("}"))
    {
      consumeUnexpected();
    }
  }

  void ParserImpl::parseExpressionOrAssignmentStatement(bool MatchArmBody)
  {
    const CstNodeId Statement = startNode(CstKind::ExpressionStatement);
    StopSet LeftStop;
    LeftStop.Symbols.assign(std::begin(AssignmentOperators), std::end(AssignmentOperators));
    LeftStop.Symbols.push_back(";");
    LeftStop.MatchArm = MatchArmBody;
    parseExpression(LeftStop);
    const std::string Sequence = longestSymbolSequence();
    if (contains(Sequence, std::begin(AssignmentOperators), std::end(AssignmentOperators)))
    {
      Builder.setKind(Statement, CstKind::AssignmentStatement);
      consumeOperator(Sequence);
      if (isExpressionStart())
      {
        parseExpression({{";"}, {}, false, MatchArmBody});
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression");
      }
    }
    else if (Sequence == "++" || Sequence == "--")
    {
      consumeUnexpectedSymbols(DiagnosticKind::ReservedSymbolSequence);
    }
    expectSymbols(";");
    finishNode();
  }

  void ParserImpl::parseIfStatement()
  {
    std::size_t OpenNodes = 0;
    while (true)
    {
      startNode(CstKind::IfStatement);
      ++OpenNodes;
      expectKeyword(KeywordKind::If, "if");
      expectSymbols("(");
      parseIfCondition();
      expectSymbols(")");
      if (atSymbols("{"))
      {
        parseStatementBlock();
      }
      else
      {
        addMissing(TokenKind::Symbol, "{");
        addMissing(TokenKind::Symbol, "}");
      }
      if (!consumeKeyword(KeywordKind::Else))
      {
        break;
      }
      if (atKeyword(KeywordKind::If))
      {
        if (SyntaxNestingDepth + OpenNodes - 1 >= Options.MaxSyntaxNestingDepth)
        {
          recoverSyntaxNesting();
          break;
        }
        continue;
      }
      if (atSymbols("{"))
      {
        parseStatementBlock();
      }
      else
      {
        addMissing(TokenKind::Symbol, "{");
        addMissing(TokenKind::Symbol, "}");
      }
      break;
    }
    while (OpenNodes != 0)
    {
      --OpenNodes;
      finishNode();
    }
  }

  void ParserImpl::parseIfCondition(CstKind MatchConditionKind)
  {
    if (atKeyword(KeywordKind::Match) && longestSymbolSequenceAt(significantIndex(1)) == ".")
    {
      startNode(MatchConditionKind);
      consumeKeyword(KeywordKind::Match);
      parseConditionalMatchPattern();
      expectSymbols("=");
      if (isExpressionStart())
      {
        parseExpression({{")"}, {}, false});
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression");
      }
      finishNode();
    }
    else if (isExpressionStart())
    {
      parseExpression({{")"}, {}, false});
    }
    else
    {
      addMissing(TokenKind::Identifier, "condition");
    }
  }

  void ParserImpl::parseMatchStatement()
  {
    startNode(CstKind::MatchStatement);
    expectKeyword(KeywordKind::Match, "match");
    expectSymbols("(");
    if (isExpressionStart())
    {
      parseExpression({{")"}, {}, false});
    }
    else
    {
      addMissing(TokenKind::Identifier, "expression");
    }
    expectSymbols(")");
    expectSymbols("{");
    if (atSymbols("}"))
    {
      addExpectedSyntax("match arm");
    }
    while (!atMatchArmBoundary() && !atSymbols("}") && !atEnd())
    {
      if (atSymbols(",") || atSymbols(";"))
      {
        consumeUnexpected();
        continue;
      }
      const std::size_t Before = RawIndex;
      parseMatchStatementArm();
      if (RawIndex == Before)
      {
        consumeUnexpected();
      }
    }
    expectSymbols("}");
    finishNode();
  }

  void ParserImpl::parseMatchStatementArm()
  {
    startNode(CstKind::MatchStatementArm);
    parseMatchArmPattern();
    expectSymbols("=>");
    MatchArmBoundaryGuard Boundary(MatchArmBoundaries, true, 0);
    if (atKeyword(KeywordKind::Var) || atKeyword(KeywordKind::Const))
    {
      const SourceRange Span = peekSignificant().Span;
      Diagnostics.push_back(DiagnosticBuilder(DiagnosticKind::DeclarationRequiresBlock, Span).build());
      SawDefinitiveError = true;
      startNode(CstKind::Error);
      parseBindingCore({{",", "}"}, {}, false, true});
      finishNode();
    }
    else
    {
      parseStatement(true);
    }
    while (!isMatchArmStart() && !atSymbols("}") && !atEnd())
    {
      consumeUnexpected();
    }
    finishNode();
  }

  void ParserImpl::parseWhileStatement()
  {
    startNode(CstKind::WhileStatement);
    expectKeyword(KeywordKind::While, "while");
    expectSymbols("(");
    parseWhileCondition();
    expectSymbols(")");
    if (atSymbols("{"))
    {
      parseStatementBlock();
    }
    else
    {
      addMissing(TokenKind::Symbol, "{");
      addMissing(TokenKind::Symbol, "}");
    }
    finishNode();
  }

  void ParserImpl::parseWhileCondition()
  {
    parseIfCondition(CstKind::WhileMatchCondition);
  }

  void ParserImpl::parseForStatement()
  {
    startNode(CstKind::ForStatement);
    expectKeyword(KeywordKind::For, "for");
    parseForHeader();
    if (atSymbols("{"))
    {
      parseStatementBlock();
    }
    else
    {
      addMissing(TokenKind::Symbol, "{");
      addMissing(TokenKind::Symbol, "}");
    }
    finishNode();
  }

  void ParserImpl::parseForHeader()
  {
    expectSymbols("(");
    startNode(CstKind::ForBindingMode);
    if (atKeyword(KeywordKind::Var) || atKeyword(KeywordKind::Const))
    {
      consumeCurrent();
    }
    else
    {
      addMissing(TokenKind::Keyword, "var or const");
    }
    finishNode();
    parseForPattern();
    expectKeyword(KeywordKind::In, "in");
    if (isExpressionStart())
    {
      parseExpression({{"..", ")"}, {}, false});
    }
    else
    {
      addMissing(TokenKind::Identifier, "expression");
    }
    if (atSymbols(".."))
    {
      wrapLast(CstKind::ForRangeSource);
      consumeSymbols("..");
      if (isExpressionStart())
      {
        parseExpression({{")"}, {}, false});
      }
      else
      {
        addMissing(TokenKind::Identifier, "range end expression");
      }
      finishNode();
    }
    expectSymbols(")");
  }

  void ParserImpl::parseBreakStatement()
  {
    startNode(CstKind::BreakStatement);
    expectKeyword(KeywordKind::Break, "break");
    expectSymbols(";");
    finishNode();
  }

  void ParserImpl::parseContinueStatement()
  {
    startNode(CstKind::ContinueStatement);
    expectKeyword(KeywordKind::Continue, "continue");
    expectSymbols(";");
    finishNode();
  }

  void ParserImpl::parseReturnStatement(bool MatchArmBody)
  {
    startNode(CstKind::ReturnStatement);
    expectKeyword(KeywordKind::Return, "return");
    const bool AtArmBoundary = MatchArmBody && (isMatchArmStart() || atSymbols(",") || atSymbols("}"));
    if (!atSymbols(";") && !atEnd() && !AtArmBoundary)
    {
      if (isExpressionStart())
      {
        parseExpression({{";"}, {}, false, MatchArmBody});
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression");
      }
    }
    expectSymbols(";");
    finishNode();
  }

  void ParserImpl::parseDeferStatement(bool MatchArmBody)
  {
    startNode(CstKind::DeferStatement);
    expectKeyword(KeywordKind::Defer, "defer");
    if (atSymbols("{"))
    {
      parseStatementBlock();
    }
    else
    {
      if (MatchArmBody && isMatchArmStart())
      {
        addMissing(TokenKind::Identifier, "expression or statement block");
      }
      else if (isExpressionStart())
      {
        parseExpression({{";"}, {}, false, MatchArmBody});
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression or statement block");
      }
      expectSymbols(";");
    }
    finishNode();
  }

  void ParserImpl::parseThrowStatement(bool MatchArmBody)
  {
    startNode(CstKind::ThrowStatement);
    expectKeyword(KeywordKind::Throw, "throw");
    const bool AtArmBoundary = MatchArmBody && (isMatchArmStart() || atSymbols(",") || atSymbols("}"));
    bool ParsedExpression = false;
    if (!atSymbols(";") && !atEnd() && !AtArmBoundary)
    {
      if (isExpressionStart())
      {
        parseExpression({{";"}, {KeywordKind::From}, false, MatchArmBody});
        ParsedExpression = true;
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression");
      }
      if (ParsedExpression && atKeyword(KeywordKind::From))
      {
        startNode(CstKind::ThrowCauseClause);
        consumeKeyword(KeywordKind::From);
        if (MatchArmBody && isMatchArmStart())
        {
          addMissing(TokenKind::Identifier, "identifier");
        }
        else
        {
          expectToken(TokenKind::Identifier, "identifier");
        }
        finishNode();
        while (!atSymbols(";") && !atSymbols("}") && !(MatchArmBody && (isMatchArmStart() || atSymbols(","))) && !atEnd())
        {
          consumeUnexpected();
        }
      }
    }
    expectSymbols(";");
    finishNode();
  }

  void ParserImpl::parseTryStatement()
  {
    startNode(CstKind::TryStatement);
    expectKeyword(KeywordKind::Try, "try");
    if (atSymbols("{"))
    {
      parseStatementBlock();
    }
    else
    {
      addMissing(TokenKind::Symbol, "{");
      addMissing(TokenKind::Symbol, "}");
    }
    if (!atKeyword(KeywordKind::Catch))
    {
      startNode(CstKind::CatchClause);
      addMissing(TokenKind::Keyword, "catch");
      addMissing(TokenKind::Symbol, "{");
      addMissing(TokenKind::Symbol, "}");
      finishNode();
      finishNode();
      return;
    }
    bool SawCatchAll = false;
    while (atKeyword(KeywordKind::Catch))
    {
      const bool CatchAll = longestSymbolSequenceAt(significantIndex(1)) == "{" || (peekSignificant(1).Kind == TokenKind::Keyword && std::get<KeywordKind>(peekSignificant(1).Payload) == KeywordKind::As);
      if (SawCatchAll)
      {
        startNode(CstKind::Error);
        addUnexpected();
        parseCatchClause(CatchAll);
        finishNode();
      }
      else
      {
        parseCatchClause(CatchAll);
      }
      SawCatchAll = SawCatchAll || CatchAll;
    }
    finishNode();
  }

  void ParserImpl::parseCatchClause(bool CatchAll)
  {
    startNode(CatchAll ? CstKind::CatchAllClause : CstKind::TypedCatchClause);
    expectKeyword(KeywordKind::Catch, "catch");
    if (!CatchAll)
    {
      parseType({{"{"}, {KeywordKind::As}, false});
    }
    if (atKeyword(KeywordKind::As))
    {
      startNode(CstKind::CatchBinding);
      consumeKeyword(KeywordKind::As);
      expectToken(TokenKind::Identifier, "identifier");
      finishNode();
    }
    if (atSymbols("{"))
    {
      parseStatementBlock();
    }
    else
    {
      addMissing(TokenKind::Symbol, "{");
      addMissing(TokenKind::Symbol, "}");
    }
    finishNode();
  }
  void ParserImpl::parseExpression(const StopSet &Stop)
  {
    if (atStop(Stop))
    {
      addMissing(TokenKind::Identifier, "expression");
      return;
    }
    if (SyntaxNestingDepth >= Options.MaxSyntaxNestingDepth)
    {
      recoverSyntaxNesting();
      return;
    }
    SyntaxNestingGuard Guard(SyntaxNestingDepth);
    if (atKeyword(KeywordKind::If))
    {
      parseIfExpression(Stop, false);
    }
    else
    {
      parseLogicalOrExpression(Stop, false);
    }
  }

  void ParserImpl::parseIfExpression(const StopSet &Stop, bool Generic)
  {
    startNode(CstKind::IfExpression);
    expectKeyword(KeywordKind::If, "if");
    expectSymbols("(");
    parseLogicalOrExpression({{")"}, {}, false}, false);
    expectSymbols(")");
    const StopSet TrueStop = withKeyword(Stop, KeywordKind::Else);
    if (Generic)
    {
      parseGenericArgumentExpression(TrueStop);
    }
    else
    {
      parseExpression(TrueStop);
    }
    expectKeyword(KeywordKind::Else, "else");
    if (Generic)
    {
      parseGenericArgumentExpression(Stop);
    }
    else
    {
      parseExpression(Stop);
    }
    finishNode();
  }

  void ParserImpl::parseGenericArgumentExpression(const StopSet &Stop)
  {
    if (atStop(Stop))
    {
      addMissing(TokenKind::Identifier, "generic argument expression");
      return;
    }
    if (SyntaxNestingDepth >= Options.MaxSyntaxNestingDepth)
    {
      recoverSyntaxNesting();
      return;
    }
    SyntaxNestingGuard Guard(SyntaxNestingDepth);
    if (atKeyword(KeywordKind::If))
    {
      parseIfExpression(Stop, true);
    }
    else
    {
      parseLogicalOrExpression(Stop, true);
    }
  }

  void ParserImpl::parseLogicalOrExpression(const StopSet &Stop, bool Generic)
  {
    parseLogicalAndExpression(Stop, Generic);
    while (atSymbols("||"))
    {
      wrapLast(CstKind::BinaryExpression);
      consumeOperator("||");
      parseLogicalAndExpression(Stop, Generic);
      finishNode();
    }
  }

  void ParserImpl::parseLogicalAndExpression(const StopSet &Stop, bool Generic)
  {
    parseComparisonExpression(Stop, Generic);
    while (atSymbols("&&"))
    {
      wrapLast(CstKind::BinaryExpression);
      consumeOperator("&&");
      parseComparisonExpression(Stop, Generic);
      finishNode();
    }
  }

  void ParserImpl::parseComparisonExpression(const StopSet &Stop, bool Generic)
  {
    parseBitwiseOrExpression(Stop, Generic);
    std::string Sequence = longestSymbolSequence();
    const bool Allowed = Generic ? (Sequence == "<" || Sequence == "<=" || Sequence == "==" || Sequence == "!=") : contains(Sequence, std::begin(ComparisonOperators), std::end(ComparisonOperators));
    if (Allowed)
    {
      wrapLast(CstKind::BinaryExpression);
      consumeOperator(Sequence);
      parseBitwiseOrExpression(Stop, Generic);
      finishNode();
      Sequence = longestSymbolSequence();
      const bool Additional = Generic ? (Sequence == "<" || Sequence == "<=" || Sequence == "==" || Sequence == "!=") : contains(Sequence, std::begin(ComparisonOperators), std::end(ComparisonOperators));
      if (Additional)
      {
        startNode(CstKind::Error);
        addUnexpected();
        consumeOperator(Sequence);
        parseBitwiseOrExpression(Stop, Generic);
        finishNode();
      }
    }
  }

  void ParserImpl::parseBitwiseOrExpression(const StopSet &Stop, bool Generic)
  {
    parseBitwiseXorExpression(Stop, Generic);
    while (atSymbols("|"))
    {
      wrapLast(CstKind::BinaryExpression);
      consumeOperator("|");
      parseBitwiseXorExpression(Stop, Generic);
      finishNode();
    }
  }

  void ParserImpl::parseBitwiseXorExpression(const StopSet &Stop, bool Generic)
  {
    parseBitwiseAndExpression(Stop, Generic);
    while (atSymbols("^"))
    {
      wrapLast(CstKind::BinaryExpression);
      consumeOperator("^");
      parseBitwiseAndExpression(Stop, Generic);
      finishNode();
    }
  }

  void ParserImpl::parseBitwiseAndExpression(const StopSet &Stop, bool Generic)
  {
    parseShiftExpression(Stop, Generic);
    while (atSymbols("&"))
    {
      wrapLast(CstKind::BinaryExpression);
      consumeOperator("&");
      parseShiftExpression(Stop, Generic);
      finishNode();
    }
  }

  void ParserImpl::parseShiftExpression(const StopSet &Stop, bool Generic)
  {
    parseAdditiveExpression(Stop);
    while (atSymbols("<<") || (!Generic && atSymbols(">>")))
    {
      const std::string Sequence = longestSymbolSequence();
      wrapLast(CstKind::BinaryExpression);
      consumeOperator(Sequence);
      parseAdditiveExpression(Stop);
      finishNode();
    }
  }

  void ParserImpl::parseAdditiveExpression(const StopSet &Stop)
  {
    parseMultiplicativeExpression(Stop);
    while (atSymbols("+") || atSymbols("-"))
    {
      const std::string Sequence = longestSymbolSequence();
      wrapLast(CstKind::BinaryExpression);
      consumeOperator(Sequence);
      parseMultiplicativeExpression(Stop);
      finishNode();
    }
  }

  void ParserImpl::parseMultiplicativeExpression(const StopSet &Stop)
  {
    parseUnaryExpression(Stop);
    while (atSymbols("*") || atSymbols("/") || atSymbols("%"))
    {
      const std::string Sequence = longestSymbolSequence();
      wrapLast(CstKind::BinaryExpression);
      consumeOperator(Sequence);
      parseUnaryExpression(Stop);
      finishNode();
    }
  }

  void ParserImpl::parseUnaryExpression(const StopSet &Stop)
  {
    std::size_t OpenNodes = 0;
    while (atSymbols("++") || atSymbols("--") || atKeyword(KeywordKind::Comptime) || atKeyword(KeywordKind::Await) || atSymbols("+") || atSymbols("-") || atSymbols("!") || atSymbols("~") || atSymbols("*") || atSymbols("&"))
    {
      const bool Comptime = atKeyword(KeywordKind::Comptime);
      startNode(Comptime ? CstKind::ComptimeExpression : CstKind::UnaryExpression);
      ++OpenNodes;
      if (atSymbols("++") || atSymbols("--"))
      {
        consumeUnexpectedSymbols(DiagnosticKind::ReservedSymbolSequence);
      }
      else if (Comptime)
      {
        consumeKeyword(KeywordKind::Comptime);
      }
      else if (atKeyword(KeywordKind::Await))
      {
        consumeKeyword(KeywordKind::Await);
      }
      else
      {
        consumeOperator(longestSymbolSequence());
      }
      if (!isExpressionStart())
      {
        addMissing(TokenKind::Identifier, "expression");
        while (OpenNodes != 0)
        {
          --OpenNodes;
          finishNode();
        }
        return;
      }
    }
    parsePostfixExpression(Stop);
    while (OpenNodes != 0)
    {
      --OpenNodes;
      finishNode();
    }
  }

  void ParserImpl::parsePostfixExpression(const StopSet &Stop)
  {
    const bool StartsWithAsyncFunction = atKeyword(KeywordKind::Async) && peekSignificant(1).Kind == TokenKind::Keyword && std::get<KeywordKind>(peekSignificant(1).Payload) == KeywordKind::Func;
    const bool StartsWithConstFunction = atKeyword(KeywordKind::Const) && peekSignificant(1).Kind == TokenKind::Keyword && std::get<KeywordKind>(peekSignificant(1).Payload) == KeywordKind::Func;
    const bool StartsWithConstAsyncFunction = atKeyword(KeywordKind::Const) && peekSignificant(1).Kind == TokenKind::Keyword && std::get<KeywordKind>(peekSignificant(1).Payload) == KeywordKind::Async && peekSignificant(2).Kind == TokenKind::Keyword && std::get<KeywordKind>(peekSignificant(2).Payload) == KeywordKind::Func;
    const bool DirectFunctionType = atKeyword(KeywordKind::Func) || StartsWithAsyncFunction || StartsWithConstFunction || StartsWithConstAsyncFunction;
    if (DirectFunctionType)
    {
      startNode(CstKind::FunctionTypeExpression);
      parseType(Stop);
      finishNode();
      return;
    }
    parsePrimaryExpression(Stop);
    while (parsePostfixSuffix(Stop, false))
    {
    }
    parseTypeConstructorTail(Stop);
  }

  void ParserImpl::parsePrimaryExpression(const StopSet &Stop)
  {
    if (atStop(Stop))
    {
      addMissing(TokenKind::Identifier, "expression");
      return;
    }
    const TokenKind Kind = peekSignificant().Kind;
    if (Kind == TokenKind::IntegerLiteral || Kind == TokenKind::FloatLiteral || Kind == TokenKind::ScalarLiteral || Kind == TokenKind::StringLiteral || Kind == TokenKind::BoolLiteral || Kind == TokenKind::NullLiteral)
    {
      startNode(CstKind::LiteralExpression);
      consumeCurrent();
      finishNode();
      return;
    }
    if (Kind == TokenKind::Identifier)
    {
      startNode(CstKind::NameExpression);
      consumeCurrent();
      finishNode();
      return;
    }
    if (Kind == TokenKind::BuiltinType)
    {
      startNode(CstKind::BuiltinTypeExpression);
      consumeCurrent();
      finishNode();
      return;
    }
    if (atKeyword(KeywordKind::This))
    {
      startNode(CstKind::ThisExpression);
      consumeKeyword(KeywordKind::This);
      finishNode();
      return;
    }
    if (atSymbols("("))
    {
      parseParenthesizedExpression();
      return;
    }
    if (atSymbols("["))
    {
      if (shouldCommitClassTypeExpression())
      {
        parseTypeDeclaration(DeclarationKind::Class, true);
      }
      else
      {
        parseArrayExpression();
      }
      return;
    }
    if (atKeyword(KeywordKind::Match))
    {
      parseMatchExpression();
      return;
    }
    if (shouldCommitClassTypeExpression())
    {
      parseTypeDeclaration(DeclarationKind::Class, true);
      return;
    }
    if (atKeyword(KeywordKind::Const))
    {
      startNode(CstKind::ConstTypeValueExpression);
      consumeKeyword(KeywordKind::Const);
      parsePostfixableTypePrimary(Stop);
      finishNode();
      return;
    }
    addMissing(TokenKind::Identifier, "expression");
  }

  void ParserImpl::parseParenthesizedExpression()
  {
    const CstNodeId Parenthesized = startNode(CstKind::ParenthesizedExpression);
    expectSymbols("(");
    if (consumeSymbols(")"))
    {
      Builder.setKind(Parenthesized, CstKind::ParenthesizedCommaList);
      finishNode();
      return;
    }
    if (consumeUnexpectedCommas() && atSymbols(")"))
    {
      expectSymbols(")");
      finishNode();
      return;
    }
    if (atSymbols("..."))
    {
      Builder.setKind(Parenthesized, CstKind::ParenthesizedCommaList);
      parseListExpansion({{")", ","}, {}, false}, false);
      if (!atSymbols(","))
      {
        expectSymbols(")");
        finishNode();
        return;
      }
      if (longestSymbolSequenceAt(significantIndex(1)) == ")")
      {
        consumeUnexpected(DiagnosticKind::TrailingComma);
        expectSymbols(")");
        finishNode();
        return;
      }
      consumeSymbols(",");
      consumeUnexpectedCommas();
      if (atSymbols("..."))
      {
        parseListExpansion({{",", ")"}, {}, false}, false);
      }
      else
      {
        parseExpression({{",", ")"}, {}, false});
      }
      while (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == ")")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        if (atSymbols("..."))
        {
          parseListExpansion({{",", ")"}, {}, false}, false);
        }
        else
        {
          parseExpression({{",", ")"}, {}, false});
        }
      }
      expectSymbols(")");
      finishNode();
      return;
    }
    if (isExpressionStart())
    {
      parseExpression({{",", ")"}, {}, false});
    }
    else
    {
      addMissing(TokenKind::Identifier, "expression");
    }
    if (!consumeSymbols(","))
    {
      expectSymbols(")");
      finishNode();
      return;
    }
    Builder.setKind(Parenthesized, CstKind::ParenthesizedCommaList);
    if (consumeSymbols(")"))
    {
      finishNode();
      return;
    }
    if (consumeUnexpectedCommas() && atSymbols(")"))
    {
      expectSymbols(")");
      finishNode();
      return;
    }
    if (atSymbols("..."))
    {
      parseListExpansion({{",", ")"}, {}, false}, false);
    }
    else
    {
      parseExpression({{",", ")"}, {}, false});
    }
    while (atSymbols(","))
    {
      if (longestSymbolSequenceAt(significantIndex(1)) == ")")
      {
        consumeUnexpected(DiagnosticKind::TrailingComma);
        break;
      }
      consumeSymbols(",");
      consumeUnexpectedCommas();
      if (atSymbols("..."))
      {
        parseListExpansion({{",", ")"}, {}, false}, false);
      }
      else
      {
        parseExpression({{",", ")"}, {}, false});
      }
    }
    expectSymbols(")");
    finishNode();
  }

  void ParserImpl::parseArrayExpression()
  {
    startNode(CstKind::ArrayExpression);
    expectSymbols("[");
    consumeUnexpectedCommas();
    if (!atSymbols("]"))
    {
      parseExpression({{",", "]"}, {}, false});
      while (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == "]")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        parseExpression({{",", "]"}, {}, false});
      }
    }
    expectSymbols("]");
    finishNode();
  }

  void ParserImpl::parseMatchExpression()
  {
    startNode(CstKind::MatchExpression);
    expectKeyword(KeywordKind::Match, "match");
    expectSymbols("(");
    parseExpression({{")"}, {}, false});
    expectSymbols(")");
    expectSymbols("{");
    if (atSymbols("}"))
    {
      addExpectedSyntax("match arm");
    }
    while (!atMatchArmBoundary() && !atSymbols("}") && !atEnd())
    {
      if (atSymbols(",") || atSymbols(";"))
      {
        consumeUnexpected();
        continue;
      }
      const std::size_t Before = RawIndex;
      parseMatchExpressionArm();
      if (RawIndex == Before)
      {
        consumeUnexpected();
      }
    }
    expectSymbols("}");
    finishNode();
  }

  void ParserImpl::parseMatchExpressionArm()
  {
    startNode(CstKind::MatchExpressionArm);
    parseMatchArmPattern();
    expectSymbols("=>");
    MatchArmBoundaryGuard Boundary(MatchArmBoundaries, true, 0);
    if (atSymbols("{"))
    {
      parseStatementBlock();
    }
    else if (isMatchArmStart())
    {
      addMissing(TokenKind::Identifier, "expression or statement block");
    }
    else if (isExpressionStart())
    {
      parseExpression({{","}, {}, false, true});
    }
    else
    {
      addMissing(TokenKind::Identifier, "expression or statement block");
    }
    while (atSymbols(";"))
    {
      consumeUnexpected();
    }
    expectSymbols(",");
    while (!isMatchArmStart() && !atSymbols("}") && !atEnd())
    {
      consumeUnexpected();
    }
    finishNode();
  }

  bool ParserImpl::parsePostfixSuffix(const StopSet &Stop, bool TypeContext)
  {
    if (atStop(Stop))
    {
      return false;
    }
    if (atSymbols("("))
    {
      wrapLast(CstKind::CallExpression);
      parseCallSuffix();
      finishNode();
      return true;
    }
    if (atSymbols("["))
    {
      if (longestSymbolSequenceAt(significantIndex(1)) == "]")
      {
        if (!TypeContext)
        {
          return false;
        }
        wrapLast(CstKind::TypePostfixSuffix);
        parseEmptyBracketSuffix();
        finishNode();
        return true;
      }
      const CstNodeId Suffix = wrapLast(CstKind::BracketPostfixSuffix);
      if (parseIndexOrSliceSuffix())
      {
        Builder.setKind(Suffix, CstKind::SliceExpression);
      }
      finishNode();
      return true;
    }
    if (atSymbols("."))
    {
      wrapLast(CstKind::MemberExpression);
      parseMemberSuffix(false);
      finishNode();
      return true;
    }
    if (atSymbols("->"))
    {
      wrapLast(CstKind::PointerMemberExpression);
      parseMemberSuffix(true);
      finishNode();
      return true;
    }
    if (atSymbols("::<"))
    {
      wrapLast(CstKind::GenericArgumentClause);
      parseGenericArgumentClause();
      finishNode();
      return true;
    }
    if (!TypeContext && atSymbols("{"))
    {
      wrapLast(CstKind::AggregateInitializationExpression);
      parseAggregateInitializationSuffix(Stop);
      finishNode();
      return true;
    }
    return false;
  }

  void ParserImpl::parseCallSuffix(bool AllowForwardAll)
  {
    expectSymbols("(");
    if (!atSymbols(")"))
    {
      std::size_t AfterEllipsis = significantIndex() + 3;
      while (AfterEllipsis < LexedFile.tokens().size() && LexedFile.tokens()[AfterEllipsis].isTrivia())
      {
        ++AfterEllipsis;
      }
      if (AllowForwardAll && atSymbols("...") && longestSymbolSequenceAt(AfterEllipsis) == ")")
      {
        startNode(CstKind::ForwardAllArguments);
        consumeSymbols("...");
        finishNode();
      }
      else
      {
        parseArgumentList();
      }
    }
    expectSymbols(")");
  }

  void ParserImpl::parseArgumentList()
  {
    startNode(CstKind::ArgumentList);
    consumeUnexpectedCommas();
    bool NamedPhase = isNamedArgumentStart();
    if (NamedPhase)
    {
      parseNamedArgument();
    }
    else
    {
      parsePositionalArgument();
    }
    while (atSymbols(","))
    {
      if (longestSymbolSequenceAt(significantIndex(1)) == ")")
      {
        consumeUnexpected(DiagnosticKind::TrailingComma);
        break;
      }
      consumeSymbols(",");
      consumeUnexpectedCommas();
      if (isNamedArgumentStart())
      {
        NamedPhase = true;
        parseNamedArgument();
      }
      else if (!NamedPhase)
      {
        parsePositionalArgument();
      }
      else
      {
        addExpectedSyntax("named argument");
        startNode(CstKind::Error);
        if (atSymbols("..."))
        {
          parseListExpansion({{",", ")"}, {}, false}, false);
        }
        else if (isExpressionStart())
        {
          parseExpression({{",", ")"}, {}, false});
        }
        else if (!atEnd())
        {
          consumeCurrent();
        }
        finishNode();
      }
    }
    finishNode();
  }

  void ParserImpl::parsePositionalArgument()
  {
    startNode(CstKind::PositionalArgument);
    if (atSymbols("..."))
    {
      parseListExpansion({{",", ")"}, {}, false}, false);
    }
    else if (isExpressionStart())
    {
      parseExpression({{",", ")"}, {}, false});
    }
    else
    {
      addMissing(TokenKind::Identifier, "argument");
    }
    finishNode();
  }

  void ParserImpl::parseNamedArgument()
  {
    startNode(CstKind::NamedArgument);
    expectToken(TokenKind::Identifier, "identifier");
    expectSymbols("=");
    if (isExpressionStart())
    {
      parseExpression({{",", ")"}, {}, false});
    }
    else
    {
      addMissing(TokenKind::Identifier, "expression");
    }
    finishNode();
  }

  void ParserImpl::parseListExpansion(const StopSet &Stop, bool Generic)
  {
    startNode(CstKind::ListExpansion);
    expectSymbols("...");
    if (isExpressionStart())
    {
      if (Generic)
      {
        parseGenericArgumentExpression(Stop);
      }
      else
      {
        parseExpression(Stop);
      }
    }
    else
    {
      addMissing(TokenKind::Identifier, "expression");
    }
    finishNode();
  }

  bool ParserImpl::parseIndexOrSliceSuffix()
  {
    expectSymbols("[");
    bool Slice = false;
    if (consumeSymbols(":"))
    {
      Slice = true;
      if (!atSymbols("]"))
      {
        parseExpression({{"]"}, {}, false});
      }
    }
    else
    {
      if (isExpressionStart())
      {
        parseExpression({{":", "]"}, {}, false});
      }
      else
      {
        addMissing(TokenKind::Identifier, "expression");
      }
      if (consumeSymbols(":"))
      {
        Slice = true;
        if (!atSymbols("]"))
        {
          parseExpression({{"]"}, {}, false});
        }
      }
    }
    expectSymbols("]");
    return Slice;
  }

  void ParserImpl::parseMemberSuffix(bool Pointer)
  {
    expectSymbols(Pointer ? "->" : ".");
    if (atToken(TokenKind::Identifier) || isUnsuffixedDecimalInteger(peekSignificant()))
    {
      consumeCurrent();
    }
    else
    {
      addMissing(TokenKind::Identifier, "member selector");
    }
  }

  void ParserImpl::parseGenericArgumentClause()
  {
    expectSymbols("::<");
    consumeUnexpectedCommas();
    if (!atSingleSymbol('>'))
    {
      startNode(CstKind::GenericArgumentList);
      parseGenericArgument();
      while (atSymbols(","))
      {
        if (rawSymbolAt(significantIndex(1), '>'))
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        parseGenericArgument();
      }
      finishNode();
    }
    expectSingleSymbol('>');
  }

  void ParserImpl::parseGenericArgument()
  {
    startNode(CstKind::GenericArgument);
    if (atSymbols("..."))
    {
      parseListExpansion({{",", ">"}, {}, false}, true);
    }
    else if (isExpressionStart())
    {
      parseGenericArgumentExpression({{",", ">"}, {}, false});
    }
    else
    {
      addMissing(TokenKind::Identifier, "generic argument");
    }
    finishNode();
  }

  void ParserImpl::parseAggregateInitializationSuffix(const StopSet &Stop)
  {
    StopSet FieldStop = Stop;
    FieldStop.Symbols.push_back(",");
    FieldStop.Symbols.push_back("}");
    expectSymbols("{");
    consumeUnexpectedCommas();

    auto AtOuterStop = [&]()
    {
      return !atSymbols(",") && !atSymbols("}") && atStop(Stop);
    };
    auto RecoverToItemBoundary = [&]()
    {
      if (atSymbols(",") || atSymbols("}") || AtOuterStop() || atEnd())
      {
        return;
      }
      startNode(CstKind::Error);
      std::vector<char> NestedClosers;
      do
      {
        addUnexpected();
        const std::string Sequence = longestSymbolSequence();
        if (Sequence.size() == 1 && (Sequence.front() == '(' || Sequence.front() == '[' || Sequence.front() == '{'))
        {
          NestedClosers.push_back(closingDelimiter(Sequence.front()));
        }
        else if (Sequence.size() == 1 && !NestedClosers.empty() && (Sequence.front() == ')' || Sequence.front() == ']' || Sequence.front() == '}'))
        {
          NestedClosers.pop_back();
        }
        consumeCurrent();
        if (NestedClosers.empty() && (atSymbols(",") || atSymbols("}") || AtOuterStop()))
        {
          break;
        }
      } while (!atMatchArmBoundary() && !atEnd());
      finishNode();
    };

    while (!atSymbols("}") && !AtOuterStop() && !atEnd())
    {
      if (atToken(TokenKind::Identifier))
      {
        const bool Explicit = longestSymbolSequenceAt(significantIndex(1)) == ":";
        startNode(Explicit ? CstKind::AggregateFieldInitializer : CstKind::AggregateFieldShorthand);
        consumeCurrent();
        if (Explicit)
        {
          consumeSymbols(":");
          parseExpression(FieldStop);
        }
        finishNode();
      }
      else
      {
        addExpectedSyntax("aggregate field initializer");
        RecoverToItemBoundary();
      }

      if (atSymbols("}") || AtOuterStop() || atEnd())
      {
        break;
      }
      if (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == "}")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        continue;
      }
      addMissing(TokenKind::Symbol, ",");
      if (atToken(TokenKind::Identifier))
      {
        continue;
      }
      RecoverToItemBoundary();
      if (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == "}")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
      }
    }
    expectSymbols("}");
  }

  void ParserImpl::parseTypeConstructorTail(const StopSet &Stop)
  {
    const bool StartsWithSymbol = atTypeSymbol('*') || atTypeSymbol('&');
    const bool StartsWithEmptyBrackets = atSymbols("[") && longestSymbolSequenceAt(significantIndex(1)) == "]";
    if (!StartsWithSymbol && !StartsWithEmptyBrackets)
    {
      return;
    }
    Checkpoint Saved = checkpoint();
    wrapLast(CstKind::TypeConstructorExpression);
    bool Consumed = false;
    while (true)
    {
      if (atTypeSymbol('*') || atTypeSymbol('&'))
      {
        parseTypeSymbolSuffix();
        Consumed = true;
        continue;
      }
      if (atSymbols("[") && longestSymbolSequenceAt(significantIndex(1)) == "]")
      {
        parseEmptyBracketSuffix();
        Consumed = true;
        continue;
      }
      if (atSymbols("[") && longestSymbolSequenceAt(significantIndex(1)) != ":")
      {
        startNode(CstKind::BracketPostfixSuffix);
        consumeSymbols("[");
        if (isExpressionStart())
        {
          parseExpression({{"]"}, {}, false});
        }
        else
        {
          addMissing(TokenKind::Identifier, "expression");
        }
        expectSymbols("]");
        finishNode();
        Consumed = true;
        continue;
      }
      break;
    }
    if (Consumed && atStop(Stop))
    {
      finishNode();
      return;
    }
    restore(std::move(Saved));
  }
  void ParserImpl::parseType(const StopSet &Stop)
  {
    startNode(CstKind::TypeSyntax);
    if (atStop(Stop))
    {
      addMissing(TokenKind::Identifier, "type");
      finishNode();
      return;
    }
    if (SyntaxNestingDepth >= Options.MaxSyntaxNestingDepth)
    {
      recoverSyntaxNesting();
      finishNode();
      return;
    }
    SyntaxNestingGuard Guard(SyntaxNestingDepth);
    if (atKeyword(KeywordKind::Const))
    {
      startNode(CstKind::ConstTypeQualifier);
      consumeKeyword(KeywordKind::Const);
      finishNode();
    }
    if (atKeyword(KeywordKind::Func) || (atKeyword(KeywordKind::Async) && peekSignificant(1).Kind == TokenKind::Keyword && std::get<KeywordKind>(peekSignificant(1).Payload) == KeywordKind::Func))
    {
      parseFunctionType(Stop);
      finishNode();
      return;
    }
    parsePostfixableTypePrimary(Stop);
    while (true)
    {
      if (parsePostfixSuffix(Stop, true))
      {
        continue;
      }
      if (atTypeSymbol('*') || atTypeSymbol('&'))
      {
        wrapLast(CstKind::TypePostfixSuffix);
        parseTypeSymbolSuffix();
        finishNode();
        continue;
      }
      break;
    }
    finishNode();
  }

  void ParserImpl::parsePostfixableTypePrimary(const StopSet &Stop)
  {
    if (atIdentifier() || atBuiltinType())
    {
      startNode(CstKind::TypeName);
      consumeCurrent();
      finishNode();
      return;
    }
    if (atSymbols("("))
    {
      if (parenthesisContainsTopLevelCommaOrExpansion())
      {
        parseTupleType();
      }
      else
      {
        startNode(CstKind::ParenthesizedTypeExpression);
        consumeSymbols("(");
        if (isExpressionStart())
        {
          parseExpression({{")"}, {}, false});
        }
        else
        {
          addMissing(TokenKind::Identifier, "expression");
        }
        expectSymbols(")");
        finishNode();
      }
      return;
    }
    (void)Stop;
    addMissing(TokenKind::Identifier, "type");
  }

  void ParserImpl::parseTupleType()
  {
    startNode(CstKind::ParenthesizedCommaList);
    expectSymbols("(");
    if (consumeSymbols(")"))
    {
      finishNode();
      return;
    }
    if (atSymbols("..."))
    {
      parseListExpansion({{")", ","}, {}, false}, false);
      if (!atSymbols(","))
      {
        expectSymbols(")");
        finishNode();
        return;
      }
      if (longestSymbolSequenceAt(significantIndex(1)) == ")")
      {
        consumeUnexpected(DiagnosticKind::TrailingComma);
        expectSymbols(")");
        finishNode();
        return;
      }
      consumeSymbols(",");
      consumeUnexpectedCommas();
      if (atSymbols("..."))
      {
        parseListExpansion({{",", ")"}, {}, false}, false);
      }
      else
      {
        parseType({{",", ")"}, {}, false});
      }
      while (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == ")")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        if (atSymbols("..."))
        {
          parseListExpansion({{",", ")"}, {}, false}, false);
        }
        else
        {
          parseType({{",", ")"}, {}, false});
        }
      }
      expectSymbols(")");
      finishNode();
      return;
    }
    parseType({{",", ")"}, {}, false});
    if (!consumeSymbols(","))
    {
      expectSymbols(")");
      finishNode();
      return;
    }
    if (consumeSymbols(")"))
    {
      finishNode();
      return;
    }
    if (atSymbols("..."))
    {
      parseListExpansion({{",", ")"}, {}, false}, false);
    }
    else
    {
      parseType({{",", ")"}, {}, false});
    }
    while (atSymbols(","))
    {
      if (longestSymbolSequenceAt(significantIndex(1)) == ")")
      {
        consumeUnexpected(DiagnosticKind::TrailingComma);
        break;
      }
      consumeSymbols(",");
      consumeUnexpectedCommas();
      if (atSymbols("..."))
      {
        parseListExpansion({{",", ")"}, {}, false}, false);
      }
      else
      {
        parseType({{",", ")"}, {}, false});
      }
    }
    expectSymbols(")");
    finishNode();
  }

  void ParserImpl::parseFunctionType(const StopSet &Stop)
  {
    startNode(CstKind::FunctionType);
    if (atKeyword(KeywordKind::Async))
    {
      consumeKeyword(KeywordKind::Async);
    }
    expectKeyword(KeywordKind::Func, "func");
    expectSymbols("(");
    consumeUnexpectedCommas();
    if (!atSymbols(")"))
    {
      parseFunctionTypeParameter();
      while (atSymbols(","))
      {
        if (longestSymbolSequenceAt(significantIndex(1)) == ")")
        {
          consumeUnexpected(DiagnosticKind::TrailingComma);
          break;
        }
        consumeSymbols(",");
        consumeUnexpectedCommas();
        parseFunctionTypeParameter();
      }
    }
    expectSymbols(")");
    if (atSymbols("->"))
    {
      startNode(CstKind::FunctionTypeResult);
      consumeSymbols("->");
      parseType(Stop);
      finishNode();
    }
    finishNode();
  }

  void ParserImpl::parseFunctionTypeParameter()
  {
    startNode(CstKind::FunctionTypeParameter);
    if (atSymbols("..."))
    {
      parseListExpansion({{",", ")"}, {}, false}, false);
    }
    else if (isTypeStart())
    {
      parseType({{",", ")"}, {}, false});
    }
    else
    {
      addMissing(TokenKind::Identifier, "type");
    }
    finishNode();
  }

  void ParserImpl::parseTypeSymbolSuffix()
  {
    const bool Pointer = atTypeSymbol('*');
    startNode(Pointer ? CstKind::PointerTypeSuffix : CstKind::ReferenceTypeSuffix);
    if (Pointer)
    {
      consumeSingleSymbol('*');
    }
    else if (atTypeSymbol('&'))
    {
      consumeSingleSymbol('&');
    }
    else
    {
      addMissing(TokenKind::Symbol, "* or &");
    }
    finishNode();
  }

  void ParserImpl::parseEmptyBracketSuffix()
  {
    startNode(CstKind::EmptyBracketTypeSuffix);
    expectSymbols("[");
    expectSymbols("]");
    finishNode();
  }

  bool ParserImpl::parenthesisContainsTopLevelCommaOrExpansion() const
  {
    if (!atSymbols("("))
    {
      return false;
    }
    std::size_t ParenthesisDepth = 0;
    std::size_t BracketDepth = 0;
    std::size_t BraceDepth = 0;
    struct GenericDepth
    {
      std::size_t Parenthesis = 0;
      std::size_t Bracket = 0;
      std::size_t Brace = 0;
    };
    std::vector<GenericDepth> GenericDepths;
    for (std::size_t Offset = 0;; ++Offset)
    {
      const Token &Current = peekSignificant(Offset);
      if (Current.Kind == TokenKind::EndOfFile)
      {
        return false;
      }
      const std::size_t TokenIndex = significantIndex(Offset);
      const std::string Sequence = longestSymbolSequenceAt(TokenIndex);
      if (Sequence == "(" && BracketDepth == 0 && BraceDepth == 0)
      {
        ++ParenthesisDepth;
      }
      else if (Sequence == ")" && BracketDepth == 0 && BraceDepth == 0)
      {
        if (ParenthesisDepth == 1)
        {
          return GenericDepths.empty() && Offset == 1;
        }
        if (ParenthesisDepth > 0)
        {
          --ParenthesisDepth;
        }
      }
      else if (Sequence == "[")
      {
        ++BracketDepth;
      }
      else if (Sequence == "]" && BracketDepth > 0)
      {
        --BracketDepth;
      }
      else if (Sequence == "{")
      {
        ++BraceDepth;
      }
      else if (Sequence == "}" && BraceDepth > 0)
      {
        --BraceDepth;
      }
      else if (Sequence == "::<")
      {
        GenericDepths.push_back({ParenthesisDepth, BracketDepth, BraceDepth});
      }
      const bool GreaterBelongsToArrow = TokenIndex > 0 && (symbolRunMatches(TokenIndex - 1, "->") || symbolRunMatches(TokenIndex - 1, "=>"));
      if (!GenericDepths.empty() && rawSymbolAt(TokenIndex, '>') && !GreaterBelongsToArrow && GenericDepths.back().Parenthesis == ParenthesisDepth && GenericDepths.back().Bracket == BracketDepth && GenericDepths.back().Brace == BraceDepth)
      {
        GenericDepths.pop_back();
      }
      else if (GenericDepths.empty() && ParenthesisDepth == 1 && BracketDepth == 0 && BraceDepth == 0 && (Sequence == "," || (Offset == 1 && Sequence == "...")))
      {
        return true;
      }
    }
  }

  ParsedFile::ParsedFile(TokenizedBuffer LexedFile, CstTree Tree, std::vector<Diagnostic> Diagnostics, ParseCompleteness Completeness)
      : LexedFile(std::move(LexedFile)), Tree(std::move(Tree)), Diagnostics(std::move(Diagnostics)), Completeness(Completeness)
  {
  }

  bool ParsedFile::succeeded() const noexcept
  {
    return LexedFile.succeeded() && Diagnostics.empty();
  }

  SourceRange ParsedFile::span(CstNodeId Id) const
  {
    Tree.node(Id);
    struct TraversalFrame
    {
      CstNodeId NodeId = 0;
      std::size_t NodeStartToken = 0;
      std::size_t NextChild = 0;
      std::size_t ConsumedTokens = 0;
    };

    std::size_t RequestedNodeStartToken = 0;
    bool FoundRequestedNode = Id == Tree.root();
    std::vector<TraversalFrame> Work;
    Work.push_back({Tree.root(), 0, 0, 0});
    while (!FoundRequestedNode && !Work.empty())
    {
      TraversalFrame &Current = Work.back();
      const CstNode &Node = Tree.node(Current.NodeId);
      if (Current.NextChild == Node.ChildCount)
      {
        assert(Current.ConsumedTokens == Node.TokenCount);
        Work.pop_back();
        continue;
      }

      const CstElement &Element = Tree.children().at(Node.FirstChild + Current.NextChild++);
      if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
      {
        const CstNode &ChildNode = Tree.node(Child->Id);
        const std::size_t ChildStartToken = Current.NodeStartToken + Current.ConsumedTokens;
        Current.ConsumedTokens += ChildNode.TokenCount;
        if (Child->Id == Id)
        {
          RequestedNodeStartToken = ChildStartToken;
          FoundRequestedNode = true;
          break;
        }
        Work.push_back({Child->Id, ChildStartToken, 0, 0});
      }
      else if (std::holds_alternative<CstTokenRef>(Element))
      {
        assert(std::get<CstTokenRef>(Element).TokenOffset == Current.ConsumedTokens);
        ++Current.ConsumedTokens;
      }
    }
    if (!FoundRequestedNode)
    {
      throw std::logic_error("CST node is not reachable from the root");
    }

    std::optional<std::size_t> Start;
    std::optional<std::size_t> End;
    Work.clear();
    Work.push_back({Id, RequestedNodeStartToken, 0, 0});
    while (!Work.empty())
    {
      TraversalFrame &Current = Work.back();
      const CstNode &Node = Tree.node(Current.NodeId);
      if (Current.NextChild == Node.ChildCount)
      {
        assert(Current.ConsumedTokens == Node.TokenCount);
        Work.pop_back();
        continue;
      }

      const CstElement &Element = Tree.children().at(Node.FirstChild + Current.NextChild++);
      if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
      {
        const CstNode &ChildNode = Tree.node(Child->Id);
        const std::size_t ChildStartToken = Current.NodeStartToken + Current.ConsumedTokens;
        Current.ConsumedTokens += ChildNode.TokenCount;
        Work.push_back({Child->Id, ChildStartToken, 0, 0});
      }
      else if (const CstTokenRef *TokenReference = std::get_if<CstTokenRef>(&Element))
      {
        assert(TokenReference->TokenOffset == Current.ConsumedTokens);
        const Token &TokenValue = LexedFile.tokens().at(Current.NodeStartToken + TokenReference->TokenOffset);
        ++Current.ConsumedTokens;
        if (!Start)
        {
          Start = TokenValue.Span.Start;
        }
        End = TokenValue.Span.End;
      }
      else if (const MissingToken *Missing = std::get_if<MissingToken>(&Element))
      {
        if (!Start)
        {
          Start = Missing->AnchorByteOffset;
        }
        if (!End)
        {
          End = Missing->AnchorByteOffset;
        }
      }
    }
    const std::size_t Anchor = Start.value_or(0);
    return {Anchor, End.value_or(Anchor)};
  }

  Parser::Parser(ParserOptions Options)
      : Options(Options)
  {
  }

  ParsedFile Parser::parse(TokenizedBuffer LexedFile) const
  {
    if (!LexedFile.succeeded())
    {
      throw std::invalid_argument("ink parser requires a successful tokenized buffer");
    }
    ParserImpl Implementation(LexedFile, Options);
    CstTree Tree = Implementation.run();
    std::vector<Diagnostic> ParserDiagnostics = Implementation.takeDiagnostics();
    return ParsedFile(std::move(LexedFile), std::move(Tree), std::move(ParserDiagnostics), Implementation.completeness());
  }

  ParsedFile parse(TokenizedBuffer LexedFile, ParserOptions Options)
  {
    return Parser(Options).parse(std::move(LexedFile));
  }
} // namespace ink::parser
