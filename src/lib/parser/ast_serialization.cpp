#include "ink/parser/ast_serialization.h"
#include "ink/parser/ast_visitor.h"
#include "ink/parser/ast_walker.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/Bitstream/BitstreamReader.h>
#include <llvm/Bitstream/BitstreamWriter.h>
#include <algorithm>
#include <limits>
#include <tuple>
#include <unordered_map>

namespace ink::parser
{
  namespace
  {
    constexpr unsigned ArchiveBlock = 8;
    constexpr unsigned CodeWidth = 3;
    enum RecordCode : unsigned
    {
      HeaderRecord = 1,
      SourceNameRecord = 2,
      SourceTextRecord = 3,
      TokenRecord = 4,
      NodeRecord = 5,
      RecoveryRecord = 6
    };

    template <typename T>
    struct Fields;
#define AST_FIELDS(Type, ...)                \
  template <>                                \
  struct Fields<Type>                        \
  {                                          \
      static auto get(const Type &Value)     \
      {                                      \
        return std::make_tuple(__VA_ARGS__); \
      }                                      \
  };
#include "ast_serialization_fields.def"
#undef AST_FIELDS

    template <typename T>
    struct EnumEntry
    {
        T Value;
        std::uint64_t Id;
    };
    template <typename T>
    struct EnumValues;
#define AST_ENUM_BEGIN(Type) \
  template <>                \
  struct EnumValues<Type>    \
  {                          \
      using Enum = Type;     \
      static constexpr EnumEntry<Enum> Values[] = {
#define AST_ENUM_VALUE(Name, Id) {Enum::Name, Id},
#define AST_ENUM_END() \
  }                    \
  ;                    \
  }                    \
  ;
#include "ast_serialization_enums.def"
#undef AST_ENUM_END
#undef AST_ENUM_VALUE
#undef AST_ENUM_BEGIN

    constexpr std::size_t TokenKindCount = 0
#define INK_TOKEN(Name, DisplayName) +1
#define INK_KEYWORD(Name, DisplayName, Spelling) +1
#define INK_SYMBOL(Name, DisplayName, Spelling) +1
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
#undef INK_KEYWORD
#undef INK_TOKEN
        ;
    static_assert(std::size(EnumValues<TokenKind>::Values) == TokenKindCount, "Add new token kinds to the stable archive enum mapping");

    template <typename T>
    struct ReadType
    {
        using Type = T;
    };
    template <typename T>
    struct ReadType<const T *>
    {
        using Type = T *;
    };
    template <typename T>
    struct ReadType<ConstNodeArray<T>>
    {
        using Type = ASTArray<T *>;
    };
    template <typename... T>
    std::tuple<typename ReadType<T>::Type...> mutableFields(std::tuple<T...>);
    template <typename T>
    using FieldTuple = decltype(mutableFields(Fields<T>::get(std::declval<const T &>())));
    template <typename T>
    struct OptionalType : std::false_type
    {
    };
    template <typename T>
    struct OptionalType<std::optional<T>> : std::true_type
    {
        using Element = T;
    };
    template <typename T>
    struct ArrayType : std::false_type
    {
    };
    template <typename T>
    struct ArrayType<ASTArray<T>> : std::true_type
    {
        using Element = T;
    };

    bool validRange(SourceRange Range, std::size_t SourceSize)
    {
      return Range.isValid() && Range.getEnd().getByteOffset() <= SourceSize;
    }

    bool validName(const NameToken &Name, const std::vector<tokenizer::Token> &Tokens, std::string_view Source)
    {
      if (!validRange(Name.Range, Source.size()))
      {
        return false;
      }
      if (Name.Id == InvalidTokenId)
      {
        return Name.Text.empty();
      }
      return Name.Id < Tokens.size() && Name.Range == Tokens[Name.Id].Span && Name.Text == Source.substr(Name.Range.getBegin().getByteOffset(), Name.Range.size());
    }

    bool validNodeMetadata(const ASTNodeBase *Node, const std::vector<tokenizer::Token> &Tokens)
    {
      if (const auto *Literal = dyn_cast<LiteralExpr>(Node))
      {
        const TokenId Id = Literal->token();
        const TokenKind Kind = Literal->literalKind();
        return Id < Tokens.size() && Tokens[Id].Kind == Kind && Tokens[Id].Span == Literal->getSourceRange() && (Kind == TokenKind::IntegerLiteral || Kind == TokenKind::FloatLiteral || Kind == TokenKind::CharLiteral || Kind == TokenKind::StringLiteral);
      }
      if (const auto *Import = dyn_cast<FromImportStmt>(Node))
      {
        std::size_t Level = 0;
        for (TokenId Id : Import->relativeTokens())
        {
          if (Id >= Tokens.size() || !Tokens[Id].isOneOf(TokenKind::Dot, TokenKind::Ellipsis))
          {
            return false;
          }
          Level += Tokens[Id].Kind == TokenKind::Dot ? 1 : 3;
        }
        return Level == Import->relativeLevel();
      }
      return true;
    }

    bool validRecovery(const RecoveryEntry &Entry, const std::vector<tokenizer::Token> &Tokens, std::size_t SourceSize)
    {
      if (!Entry.Node || !validRange(Entry.Token.Range, SourceSize) || (Entry.Skipped && !validRange(*Entry.Skipped, SourceSize)))
      {
        return false;
      }
      if (Entry.Token.Status == ExpectStatus::Inserted)
      {
        return !Entry.Token.Actual && Entry.Token.Range.empty();
      }
      return Entry.Token.Status == ExpectStatus::Recovered && Entry.Token.Actual && *Entry.Token.Actual < Tokens.size() && Tokens[*Entry.Token.Actual].Kind == Entry.Token.Expected && Tokens[*Entry.Token.Actual].Span == Entry.Token.Range && (!Entry.Skipped || Entry.Skipped->getEnd() <= Entry.Token.Range.getBegin());
    }

    class ASTWriter
    {
      public:
        ASTWriter(core::FrontendContext &Context, const ParseResult &Parsed, ASTArchiveLimits Limits)
            : Context(Context),
              Parsed(Parsed),
              Limits(Limits),
              Stream(Output)
        {
        }

        ASTSerializeResult run()
        {
          if (!Parsed.Unit || !Parsed.Unit->root())
          {
            failICE<core::DiagnosticKind::ASTArchiveMissingInput>(ASTArchiveStatus::InvalidInput);
            return result();
          }
          const auto &Input = Parsed.Unit->input().lexedFile();
          Source = Input.source();
          Tokens = &Input.tokens();
          if (Source.size() > Limits.MaxSourceBytes)
          {
            failICE<core::DiagnosticKind::ASTArchiveSourceLimitExceeded>(ASTArchiveStatus::LimitExceeded, Source.size(), Limits.MaxSourceBytes);
            return result();
          }
          if (Tokens->size() > Limits.MaxTokens)
          {
            failICE<core::DiagnosticKind::ASTArchiveTokenLimitExceeded>(ASTArchiveStatus::LimitExceeded, Tokens->size(), Limits.MaxTokens);
            return result();
          }
          if (Parsed.Unit->recoveryInfo().Entries.size() > Limits.MaxArrayElements)
          {
            failICE<core::DiagnosticKind::ASTArchiveRecoveryLimitExceeded>(ASTArchiveStatus::LimitExceeded, Parsed.Unit->recoveryInfo().Entries.size(), Limits.MaxArrayElements);
            return result();
          }
          std::string Reason;
          if (!verifyAST(Parsed.Unit->root(), Source.size(), &Reason))
          {
            failICE<core::DiagnosticKind::ASTArchiveInvalidTree>(ASTArchiveStatus::InvalidInput, std::move(Reason));
            return result();
          }
          ASTWalker{}.walk(Parsed.Unit->root(), [&](const ASTNodeBase *)
                           {
                             return Status == ASTArchiveStatus::Success ? WalkAction::Continue : WalkAction::Stop;
                           },
                           [&](const ASTNodeBase *Node)
                           {
                             if (Nodes.size() >= Limits.MaxNodes)
                             {
                               failICE<core::DiagnosticKind::ASTArchiveNodeLimitExceeded>(ASTArchiveStatus::LimitExceeded, Nodes.size() + 1, Limits.MaxNodes);
                               return;
                             }
                             Ids.emplace(Node, Nodes.size() + 1);
                             Nodes.push_back(Node);
                           });
          for (unsigned char Byte : std::string_view("IAST"))
          {
            Stream.Emit(Byte, 8);
          }
          Stream.EnterSubblock(ArchiveBlock, CodeWidth);
          write(ASTArchiveVersion);
          write(Parsed.Status);
          write(Parsed.HasSyntaxErrors);
          write(Input.succeeded());
          write(Tokens->size());
          write(Nodes.size());
          write(Parsed.Unit->recoveryInfo().Entries.size());
          emit(HeaderRecord);
          emitText(SourceNameRecord, Input.sourceName());
          emitText(SourceTextRecord, Source);
          for (const auto &Token : *Tokens)
          {
            if (Status != ASTArchiveStatus::Success)
            {
              break;
            }
            write(Token.Kind);
            write(Token.Span);
            // Payload tags are explicit and independent of std::variant ordering.
            std::visit([&](const auto &Payload)
                       {
                         using T = std::decay_t<decltype(Payload)>;
                         if constexpr (std::is_same_v<T, std::monostate>)
                         {
                           write(0U);
                         }
                         else if constexpr (std::is_same_v<T, tokenizer::IdentifierInfo>)
                         {
                           write(1U);
                           write(std::string_view(Payload.Name));
                         }
                         else if constexpr (std::is_same_v<T, tokenizer::NumericInfo>)
                         {
                           write(2U);
                           write(Payload.Base);
                         }
                         else if constexpr (std::is_same_v<T, tokenizer::StringInfo>)
                         {
                           write(3U);
                           write(Payload.Mode);
                           write(std::string_view(Payload.Decoded));
                         }
                         else if constexpr (std::is_same_v<T, tokenizer::CharInfo>)
                         {
                           write(4U);
                           write(static_cast<std::uint32_t>(Payload.Value));
                           write(Payload.Raw);
                         }
                       },
                       Token.Payload);
            emit(TokenRecord);
          }
          for (const ASTNodeBase *Node : Nodes)
          {
            if (Status != ASTArchiveStatus::Success)
            {
              break;
            }
            if (!validNodeMetadata(Node, *Tokens))
            {
              failICE<core::DiagnosticKind::ASTArchiveInvalidNodeToken>(ASTArchiveStatus::InvalidInput);
              break;
            }
            write(static_cast<std::uint64_t>(Node->getKind()));
            switch (Node->getKind())
            {
#define AST_NODE(Name, Base, Category, Id)         \
  case ASTKind::Name:                              \
    writeFields(*static_cast<const Name *>(Node)); \
    break;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
            }
            emit(NodeRecord);
          }
          for (const auto &Entry : Parsed.Unit->recoveryInfo().Entries)
          {
            if (!validRecovery(Entry, *Tokens, Source.size()))
            {
              failICE<core::DiagnosticKind::ASTArchiveInvalidRecovery>(ASTArchiveStatus::InvalidInput);
              break;
            }
            write(Entry.Node);
            write(Entry.Token.Expected);
            write(Entry.Token.Actual);
            write(Entry.Token.Range);
            write(Entry.Token.Status);
            write(Entry.Skipped);
            emit(RecoveryRecord);
          }
          Stream.ExitBlock();
          if (Output.size() > Limits.MaxArchiveBytes)
          {
            failICE<core::DiagnosticKind::ASTArchiveSizeLimitExceeded>(ASTArchiveStatus::LimitExceeded, Output.size(), Limits.MaxArchiveBytes);
          }
          if (Status != ASTArchiveStatus::Success)
          {
            return result();
          }
          return {std::string(Output.begin(), Output.end()), Status, {}};
        }

      private:
        template <core::DiagnosticKind Kind, typename... ArgumentTypes>
        void failICE(ASTArchiveStatus NewStatus, ArgumentTypes &&...Arguments)
        {
          static_assert(core::DiagnosticTraits<Kind>::Classification == core::DiagnosticClass::InternalCompilerError);
          if (Status == ASTArchiveStatus::Success)
          {
            Status = NewStatus;
            const auto Diagnostic = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
            Message = core::DiagnosticFormatter{}.format(Diagnostic).Message;
            Context.diagnosticEngine().report(Diagnostic);
          }
        }
        ASTSerializeResult result()
        {
          return {{}, Status, std::move(Message)};
        }
        void emit(unsigned Code)
        {
          if (Status == ASTArchiveStatus::Success)
          {
            Stream.EmitRecord(Code, Values);
            if (Output.size() > Limits.MaxArchiveBytes)
            {
              failICE<core::DiagnosticKind::ASTArchiveSizeLimitExceeded>(ASTArchiveStatus::LimitExceeded, Output.size(), Limits.MaxArchiveBytes);
            }
          }
          Values.clear();
        }
        void emitText(unsigned Code, std::string_view Text)
        {
          if (Status == ASTArchiveStatus::Success)
          {
            Stream.EmitRecord(Code, llvm::ArrayRef<unsigned char>(reinterpret_cast<const unsigned char *>(Text.data()), Text.size()));
          }
        }
        template <typename T>
        void writeFields(const T &Value)
        {
          std::apply([&](const auto &...Field)
                     {
                       (write(Field), ...);
                     },
                     Fields<T>::get(Value));
        }
        template <typename T>
        void write(const T &Value)
        {
          if (Status != ASTArchiveStatus::Success)
          {
            return;
          }
          if constexpr (std::is_integral_v<T>)
          {
            if (Values.size() >= Limits.MaxAllocationBytes / sizeof(std::uint64_t))
            {
              failICE<core::DiagnosticKind::ASTArchiveRecordStorageLimitExceeded>(ASTArchiveStatus::LimitExceeded, Values.size() + 1, Limits.MaxAllocationBytes / sizeof(std::uint64_t));
              return;
            }
            Values.push_back(static_cast<std::uint64_t>(Value));
          }
          else if constexpr (std::is_enum_v<T>)
          {
            for (const auto &Entry : EnumValues<T>::Values)
            {
              if (Entry.Value == Value)
              {
                write(Entry.Id);
                return;
              }
            }
            failICE<core::DiagnosticKind::ASTArchiveInvalidEnum>(ASTArchiveStatus::InvalidInput, static_cast<std::uint64_t>(Value));
          }
          else if constexpr (std::is_pointer_v<T>)
          {
            if (!Value)
            {
              write(0U);
            }
            else if (const auto Found = Ids.find(Value); Found != Ids.end())
            {
              write(Found->second);
            }
            else
            {
              failICE<core::DiagnosticKind::ASTArchiveExternalNodeReference>(ASTArchiveStatus::InvalidInput);
            }
          }
          else if constexpr (std::is_same_v<T, SourceRange>)
          {
            if (!validRange(Value, Source.size()))
            {
              failICE<core::DiagnosticKind::ASTArchiveInvalidSourceRange>(ASTArchiveStatus::InvalidInput);
              return;
            }
            write(Value.getBegin().getRawEncoding());
            write(Value.getEnd().getRawEncoding());
          }
          else if constexpr (std::is_same_v<T, std::string_view>)
          {
            write(Value.size());
            for (unsigned char Byte : Value)
            {
              write(Byte);
            }
          }
          else if constexpr (std::is_same_v<T, NameToken>)
          {
            if (!validName(Value, *Tokens, Source))
            {
              failICE<core::DiagnosticKind::ASTArchiveInvalidNameToken>(ASTArchiveStatus::InvalidInput);
              return;
            }
            write(Value.Id == InvalidTokenId ? 0 : Value.Id + 1);
            write(Value.Text);
            write(Value.Range);
          }
          else if constexpr (std::is_same_v<T, RestBinding>)
          {
            write(Value.Name);
            write(Value.Wildcard);
            write(Value.EllipsisRange);
          }
          else if constexpr (OptionalType<T>::value)
          {
            write(Value.has_value());
            if (Value)
            {
              write(*Value);
            }
          }
          else if constexpr (requires { Value.begin(); Value.size(); })
          {
            if (Value.size() > Limits.MaxArrayElements)
            {
              failICE<core::DiagnosticKind::ASTArchiveArrayLimitExceeded>(ASTArchiveStatus::LimitExceeded, Value.size(), Limits.MaxArrayElements);
              return;
            }
            write(Value.size());
            for (const auto &Element : Value)
            {
              write(Element);
            }
          }
          else
          {
            writeFields(Value);
          }
        }

        core::FrontendContext &Context;
        const ParseResult &Parsed;
        ASTArchiveLimits Limits;
        llvm::SmallVector<char, 0> Output;
        llvm::BitstreamWriter Stream;
        std::vector<std::uint64_t> Values;
        std::vector<const ASTNodeBase *> Nodes;
        std::unordered_map<const ASTNodeBase *, std::size_t> Ids;
        std::string_view Source;
        const std::vector<tokenizer::Token> *Tokens = nullptr;
        ASTArchiveStatus Status = ASTArchiveStatus::Success;
        std::string Message;
    };
  } // namespace

  class ASTReader
  {
    public:
      ASTReader(core::FrontendContext &Context, std::string_view Bytes, ASTArchiveLimits Limits)
          : Context(Context),
            Bytes(Bytes),
            Limits(Limits),
            Cursor(llvm::StringRef(Bytes.data(), Bytes.size())),
            Arena(std::make_unique<ASTContext>())
      {
      }

      ASTDeserializeResult run()
      {
        if (Bytes.size() > Limits.MaxArchiveBytes)
        {
          failICE<core::DiagnosticKind::ASTArchiveSizeLimitExceeded>(ASTArchiveStatus::LimitExceeded, Bytes.size(), Limits.MaxArchiveBytes);
          return result();
        }
        if (Bytes.size() < 12 || Bytes.size() % 4 != 0 || Bytes.substr(0, 4) != "IAST")
        {
          failICE<core::DiagnosticKind::ASTArchiveInvalidSignature>(ASTArchiveStatus::InvalidArchive);
          return result();
        }
        bits(32);
        if (bits(2) != llvm::bitc::ENTER_SUBBLOCK || vbr(8) != ArchiveBlock || vbr(4) != CodeWidth || !align())
        {
          failICE<core::DiagnosticKind::ASTArchiveInvalidBlock>(ASTArchiveStatus::InvalidArchive);
          return result();
        }
        const auto Words = bits(32);
        if (Words != (Bytes.size() - Cursor.GetCurrentBitNo() / 8) / 4)
        {
          failICE<core::DiagnosticKind::ASTArchiveInvalidBlockSize>(ASTArchiveStatus::InvalidArchive);
          return result();
        }
        begin(HeaderRecord);
        const auto Version = number();
        if (Good && Version != ASTArchiveVersion)
        {
          failICE<core::DiagnosticKind::ASTArchiveUnsupportedVersion>(ASTArchiveStatus::UnsupportedVersion, Version, ASTArchiveVersion);
          return result();
        }
        const auto ParseState = read<ParseStatus>();
        const auto SyntaxErrors = read<bool>();
        const auto LexSucceeded = read<bool>();
        const auto TokenCount = count(Limits.MaxTokens);
        const auto NodeCount = count(Limits.MaxNodes);
        const auto RecoveryCount = count(Limits.MaxArrayElements);
        end();
        if (!Good || NodeCount == 0)
        {
          failICE<core::DiagnosticKind::ASTArchiveMissingNodes>(ASTArchiveStatus::InvalidArchive);
          return result();
        }
        begin(SourceNameRecord);
        auto SourceName = text(Remaining);
        end();
        begin(SourceTextRecord);
        Source = text(Remaining, Limits.MaxSourceBytes);
        end();
        if (!Good || !charge(TokenCount, sizeof(tokenizer::Token)) || !charge(NodeCount, sizeof(ASTNodeBase *) + sizeof(unsigned)))
        {
          return result();
        }
        Tokens.reserve(TokenCount);
        Nodes.reserve(NodeCount);
        Parents.reserve(NodeCount);
        for (std::size_t Index = 0; Good && Index < TokenCount; ++Index)
        {
          begin(TokenRecord);
          const auto Kind = read<TokenKind>();
          const auto Range = read<SourceRange>();
          const auto Tag = number();
          tokenizer::TokenPayload Payload;
          switch (Tag)
          {
          case 0:
            break;
          case 1:
            Payload = tokenizer::IdentifierInfo{string()};
            break;
          case 2:
          {
            const auto Base = read<unsigned>();
            if (Base)
            {
              Payload = tokenizer::NumericInfo{*Base};
            }
            break;
          }
          case 3:
          {
            const auto Mode = read<tokenizer::StringMode>();
            auto Decoded = string();
            if (Mode)
            {
              Payload = tokenizer::StringInfo{*Mode, std::move(Decoded)};
            }
            break;
          }
          case 4:
          {
            const auto Value = read<std::uint32_t>();
            const auto Raw = read<bool>();
            if (Value && Raw)
            {
              Payload = tokenizer::CharInfo{static_cast<char32_t>(*Value), *Raw};
            }
            break;
          }
          default:
            failICE<core::DiagnosticKind::ASTArchiveUnknownTokenPayload>(ASTArchiveStatus::InvalidArchive, Tag);
          }
          end();
          if (Good)
          {
            Tokens.push_back({*Kind, *Range, std::move(Payload)});
          }
        }
        for (std::size_t Index = 0; Good && Index < NodeCount; ++Index)
        {
          begin(NodeRecord);
          const auto Kind = number();
          ASTNodeBase *Node = nullptr;
          switch (Kind)
          {
#define AST_NODE(Name, Base, Category, Id) \
  case Id:                                 \
    Node = node<Name>();                   \
    break;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
          default:
            failICE<core::DiagnosticKind::ASTArchiveUnknownNodeKind>(ASTArchiveStatus::InvalidArchive, Kind);
          }
          end();
          if (Good && !validNodeMetadata(Node, Tokens))
          {
            failICE<core::DiagnosticKind::ASTArchiveInvalidNodeToken>(ASTArchiveStatus::InvalidArchive);
          }
          if (Good)
          {
            Nodes.push_back(Node);
            Parents.push_back(0);
          }
        }
        if (!Good)
        {
          return result();
        }
        ModuleAST *Root = dyn_cast<ModuleAST>(Nodes.back());
        if (!Root || Parents.back() != 0 || !std::all_of(Parents.begin(), Parents.end() - 1, [](unsigned Count)
                                                         {
                                                           return Count == 1;
                                                         }))
        {
          failICE<core::DiagnosticKind::ASTArchiveInvalidRoot>(ASTArchiveStatus::InvalidArchive);
          return result();
        }
        if (!charge(RecoveryCount, sizeof(RecoveryEntry)))
        {
          return result();
        }
        SyntaxRecoveryInfo Recovery;
        Recovery.Entries.reserve(RecoveryCount);
        for (std::size_t Index = 0; Good && Index < RecoveryCount; ++Index)
        {
          begin(RecoveryRecord);
          const auto NodeId = number();
          const auto Expected = read<TokenKind>();
          const auto Actual = read<std::optional<TokenId>>();
          const auto Range = read<SourceRange>();
          const auto State = read<ExpectStatus>();
          const auto Skipped = read<std::optional<SourceRange>>();
          end();
          if (!Good)
          {
            break;
          }
          RecoveryEntry Entry{NodeId && NodeId <= Nodes.size() ? Nodes[NodeId - 1] : nullptr, {*Expected, *Actual, *Range, *State}, *Skipped};
          if (!validRecovery(Entry, Tokens, Source.size()))
          {
            failICE<core::DiagnosticKind::ASTArchiveInvalidRecovery>(ASTArchiveStatus::InvalidArchive);
            break;
          }
          Recovery.Entries.push_back(Entry);
        }
        if (!Good)
        {
          return result();
        }
        if (bits(CodeWidth) != llvm::bitc::END_BLOCK || !align() || Cursor.GetCurrentBitNo() != Bytes.size() * 8)
        {
          failICE<core::DiagnosticKind::ASTArchiveInvalidBlockEnd>(ASTArchiveStatus::InvalidArchive);
        }
        std::string Reason;
        if (Good && !verifyAST(Root, Source.size(), &Reason))
        {
          failICE<core::DiagnosticKind::ASTArchiveInvalidTree>(ASTArchiveStatus::InvalidArchive, std::move(Reason));
        }
        // Counting CR and LF separately conservatively covers CRLF normalization.
        const auto LineCount = 1 + static_cast<std::size_t>(std::count(Source.begin(), Source.end(), '\r')) + static_cast<std::size_t>(std::count(Source.begin(), Source.end(), '\n'));
        if (!Good || !charge(LineCount, sizeof(std::size_t) * 2))
        {
          return result();
        }
        auto Input = tokenizer::TokenizedBuffer::fromSnapshot(Context.sourceManager(), std::move(SourceName), std::move(Source), std::move(Tokens), *LexSucceeded);
        if (!Input)
        {
          failICE<core::DiagnosticKind::ASTArchiveInvalidLexicalSnapshot>(ASTArchiveStatus::InvalidArchive);
          return result();
        }
        auto Unit = std::make_unique<ParsedUnit>(std::make_shared<const TokenBuffer>(std::move(*Input)));
        Unit->Context = std::move(Arena);
        Unit->Root = Root;
        Unit->Recovery = std::move(Recovery);
        return {{std::move(Unit), *ParseState, *SyntaxErrors}, ASTArchiveStatus::Success, {}};
      }

    private:
      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void failICE(ASTArchiveStatus Status, ArgumentTypes &&...Arguments)
      {
        static_assert(core::DiagnosticTraits<Kind>::Classification == core::DiagnosticClass::InternalCompilerError);
        if (Good)
        {
          Good = false;
          Failure = Status;
          const auto Diagnostic = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
          Message = core::DiagnosticFormatter{}.format(Diagnostic).Message;
          Context.diagnosticEngine().report(Diagnostic);
        }
      }
      ASTDeserializeResult result()
      {
        return {{}, Failure, std::move(Message)};
      }
      bool charge(std::size_t Count, std::size_t Size)
      {
        if (!Good)
        {
          return false;
        }
        if (Size && Count > (Limits.MaxAllocationBytes - Allocated) / Size)
        {
          failICE<core::DiagnosticKind::ASTArchiveAllocationLimitExceeded>(ASTArchiveStatus::LimitExceeded, Count, Size, Limits.MaxAllocationBytes - Allocated);
          return false;
        }
        Allocated += Count * Size;
        return true;
      }
      std::uint64_t bits(unsigned Width)
      {
        if (!Good)
        {
          return 0;
        }
        auto Value = Cursor.Read(Width);
        if (!Value)
        {
          llvm::consumeError(Value.takeError());
          failICE<core::DiagnosticKind::ASTArchiveTruncatedBitstream>(ASTArchiveStatus::InvalidArchive);
          return 0;
        }
        return *Value;
      }
      std::uint64_t vbr(unsigned Width)
      {
        std::uint64_t Value = 0;
        const unsigned DataWidth = Width - 1;
        for (unsigned Shift = 0; Good && Shift < 64; Shift += DataWidth)
        {
          const auto Chunk = bits(Width);
          const auto Payload = Chunk & ((std::uint64_t{1} << DataWidth) - 1);
          if (Payload > (std::numeric_limits<std::uint64_t>::max() >> Shift))
          {
            break;
          }
          Value |= Payload << Shift;
          if ((Chunk >> DataWidth) == 0)
          {
            return Value;
          }
        }
        failICE<core::DiagnosticKind::ASTArchiveIntegerOverflow>(ASTArchiveStatus::InvalidArchive);
        return 0;
      }
      bool align()
      {
        const unsigned Padding = static_cast<unsigned>((32 - Cursor.GetCurrentBitNo() % 32) % 32);
        if (Padding && bits(Padding) != 0)
        {
          failICE<core::DiagnosticKind::ASTArchiveNonzeroPadding>(ASTArchiveStatus::InvalidArchive);
        }
        return Good;
      }
      void begin(unsigned Expected)
      {
        if (!Good)
        {
          return;
        }
        if (bits(CodeWidth) != llvm::bitc::UNABBREV_RECORD || vbr(6) != Expected)
        {
          failICE<core::DiagnosticKind::ASTArchiveUnexpectedRecord>(ASTArchiveStatus::InvalidArchive);
          return;
        }
        Remaining = vbr(6);
        if (Good && Remaining > (Bytes.size() * 8 - Cursor.GetCurrentBitNo()) / 6)
        {
          failICE<core::DiagnosticKind::ASTArchiveRecordLengthExceeded>(ASTArchiveStatus::InvalidArchive);
        }
      }
      void end()
      {
        if (Good && Remaining != 0)
        {
          failICE<core::DiagnosticKind::ASTArchiveExtraRecordFields>(ASTArchiveStatus::InvalidArchive);
        }
      }
      std::uint64_t number()
      {
        if (!Good)
        {
          return 0;
        }
        if (Remaining == 0)
        {
          failICE<core::DiagnosticKind::ASTArchiveMissingRecordField>(ASTArchiveStatus::InvalidArchive);
          return 0;
        }
        --Remaining;
        return vbr(6);
      }
      std::size_t count(std::size_t Maximum)
      {
        const auto Count = number();
        if (Good && Count > Maximum)
        {
          failICE<core::DiagnosticKind::ASTArchiveElementLimitExceeded>(ASTArchiveStatus::LimitExceeded, Count, Maximum);
          return 0;
        }
        return static_cast<std::size_t>(Count);
      }
      std::string text(std::uint64_t Length, std::size_t Maximum = std::numeric_limits<std::size_t>::max())
      {
        if (!Good)
        {
          return {};
        }
        if (Length > Maximum)
        {
          failICE<core::DiagnosticKind::ASTArchiveTextLimitExceeded>(ASTArchiveStatus::LimitExceeded, Length, Maximum);
          return {};
        }
        if (Length > Remaining)
        {
          failICE<core::DiagnosticKind::ASTArchiveStringLengthExceeded>(ASTArchiveStatus::InvalidArchive);
          return {};
        }
        if (!charge(static_cast<std::size_t>(Length), 2))
        {
          return {};
        }
        std::string Result(static_cast<std::size_t>(Length), '\0');
        for (char &Byte : Result)
        {
          const auto Value = number();
          if (!Good || Value > 255)
          {
            failICE<core::DiagnosticKind::ASTArchiveInvalidByte>(ASTArchiveStatus::InvalidArchive, Value);
            return {};
          }
          Byte = static_cast<char>(Value);
        }
        return Result;
      }
      std::string string()
      {
        const auto Length = number();
        return text(Length);
      }
      template <typename Tuple, std::size_t... Index>
      std::optional<Tuple> tuple(std::index_sequence<Index...>)
      {
        Tuple Values{};
        const auto Read = [&]<std::size_t I>()
        {
          auto Value = read<std::tuple_element_t<I, Tuple>>();
          if (Value)
          {
            std::get<I>(Values) = std::move(*Value);
          }
        };
        (Read.template operator()<Index>(), ...);
        return Good ? std::optional<Tuple>(std::move(Values)) : std::nullopt;
      }
      template <typename T>
      auto fields()
      {
        return tuple<FieldTuple<T>>(std::make_index_sequence<std::tuple_size_v<FieldTuple<T>>>{});
      }
      template <typename T>
      T *node()
      {
        auto Values = fields<T>();
        if (!Values || !charge(1, sizeof(T) + alignof(T)))
        {
          return nullptr;
        }
        return std::apply([&](auto &&...Field)
                          {
                            return Arena->make<T>(std::forward<decltype(Field)>(Field)...);
                          },
                          std::move(*Values));
      }
      template <typename T>
      std::optional<T> read()
      {
        if (!Good)
        {
          return std::nullopt;
        }
        if constexpr (std::is_integral_v<T>)
        {
          const auto Value = number();
          if (Good && Value > static_cast<std::uint64_t>(std::numeric_limits<T>::max()))
          {
            failICE<core::DiagnosticKind::ASTArchiveIntegerOutOfRange>(ASTArchiveStatus::InvalidArchive, Value, static_cast<std::uint64_t>(std::numeric_limits<T>::max()));
          }
          if (Good)
          {
            return static_cast<T>(Value);
          }
        }
        else if constexpr (std::is_enum_v<T>)
        {
          const auto Value = number();
          for (const auto &Entry : EnumValues<T>::Values)
          {
            if (Good && Entry.Id == Value)
            {
              return Entry.Value;
            }
          }
          failICE<core::DiagnosticKind::ASTArchiveUnknownEnum>(ASTArchiveStatus::InvalidArchive, Value);
        }
        else if constexpr (std::is_pointer_v<T>)
        {
          const auto Id = number();
          if (Good && Id == 0)
          {
            return T(nullptr);
          }
          if (Good && Id <= Nodes.size() && std::remove_pointer_t<T>::classof(Nodes[Id - 1]) && Parents[Id - 1] == 0)
          {
            ++Parents[Id - 1];
            return static_cast<T>(Nodes[Id - 1]);
          }
          failICE<core::DiagnosticKind::ASTArchiveInvalidNodeReference>(ASTArchiveStatus::InvalidArchive, Id);
        }
        else if constexpr (std::is_same_v<T, SourceRange>)
        {
          const auto Start = read<std::uint32_t>();
          const auto End = read<std::uint32_t>();
          if (Good)
          {
            const SourceRange Range(core::SourceLocation::getFromRawEncoding(*Start), core::SourceLocation::getFromRawEncoding(*End));
            if (validRange(Range, Source.size()))
            {
              return Range;
            }
            failICE<core::DiagnosticKind::ASTArchiveInvalidSourceRange>(ASTArchiveStatus::InvalidArchive);
          }
        }
        else if constexpr (std::is_same_v<T, NameToken>)
        {
          const auto Id = number();
          auto Text = string();
          const auto Range = read<SourceRange>();
          if (Good)
          {
            NameToken Name{Id ? static_cast<TokenId>(Id - 1) : InvalidTokenId, Text, *Range};
            if (Id > Tokens.size() || !validName(Name, Tokens, Source))
            {
              failICE<core::DiagnosticKind::ASTArchiveInvalidNameToken>(ASTArchiveStatus::InvalidArchive);
            }
            else if (charge(Text.size(), 1))
            {
              const auto Stored = Arena->copyArray<char>(std::span<const char>(Text.data(), Text.size()));
              Name.Text = Stored.empty() ? std::string_view{} : std::string_view(Stored.data(), Stored.size());
              return Name;
            }
          }
        }
        else if constexpr (std::is_same_v<T, RestBinding>)
        {
          const auto Name = read<NameToken>();
          const auto Wildcard = read<bool>();
          const auto Range = read<SourceRange>();
          if (Good)
          {
            return RestBinding{*Name, *Wildcard, *Range};
          }
        }
        else if constexpr (OptionalType<T>::value)
        {
          const auto Present = read<bool>();
          if (Good && !*Present)
          {
            return T{};
          }
          auto Value = read<typename OptionalType<T>::Element>();
          if (Value)
          {
            return T(std::move(*Value));
          }
        }
        else if constexpr (ArrayType<T>::value)
        {
          using Element = typename ArrayType<T>::Element;
          const auto Count = count(Limits.MaxArrayElements);
          if (Good && Count > Remaining)
          {
            failICE<core::DiagnosticKind::ASTArchiveArrayLengthExceeded>(ASTArchiveStatus::InvalidArchive);
          }
          if (charge(Count, sizeof(Element) * 2))
          {
            std::vector<Element> Values;
            Values.reserve(Count);
            for (std::size_t Index = 0; Good && Index < Count; ++Index)
            {
              auto Value = read<Element>();
              if (Value)
              {
                Values.push_back(std::move(*Value));
              }
            }
            if (Good)
            {
              return Arena->copyArray(Values);
            }
          }
        }
        else
        {
          auto Values = fields<T>();
          if (Values)
          {
            return std::apply([](auto &&...Field)
                              {
                                return T(std::forward<decltype(Field)>(Field)...);
                              },
                              std::move(*Values));
          }
        }
        return std::nullopt;
      }

      core::FrontendContext &Context;
      std::string_view Bytes;
      ASTArchiveLimits Limits;
      llvm::SimpleBitstreamCursor Cursor;
      std::unique_ptr<ASTContext> Arena;
      std::string Source;
      std::vector<tokenizer::Token> Tokens;
      std::vector<ASTNodeBase *> Nodes;
      std::vector<unsigned> Parents;
      std::uint64_t Remaining = 0;
      std::size_t Allocated = 0;
      bool Good = true;
      ASTArchiveStatus Failure = ASTArchiveStatus::InvalidArchive;
      std::string Message;
  };

  ASTSerializeResult serializeAST(core::FrontendContext &Context, const ParseResult &Parsed, ASTArchiveLimits Limits)
  {
    return ASTWriter(Context, Parsed, Limits).run();
  }

  ASTDeserializeResult deserializeAST(core::FrontendContext &Context, std::string_view Bytes, ASTArchiveLimits Limits)
  {
    return ASTReader(Context, Bytes, Limits).run();
  }
} // namespace ink::parser
