namespace ink::parser
{
  // Included in ast_serialization.cpp to share its constructor and stable enum schemas.
  namespace
  {
    std::string_view fieldLabel(std::string_view &Labels)
    {
      const auto Dot = Labels.find('.');
      const auto End = Labels.find('(', Dot);
      const auto Name = Labels.substr(Dot + 1, End - Dot - 1);
      const auto Comma = Labels.find(',', End);
      Labels = Comma == std::string_view::npos ? std::string_view{} : Labels.substr(Comma + 1);
      return Name == "getSourceRange" ? "range" : Name;
    }

    class ASTTextWriter
    {
      public:
        ASTTextWriter(const ParseResult &Parsed, ASTArchiveLimits Limits)
            : Parsed(Parsed),
              Limits(Limits)
        {
        }

        ASTSerializeResult run()
        {
          // Reuse all syntax and metadata checks of the binary codec before printing.
          auto Validated = ASTWriter(nullptr, Parsed, Limits).run();
          if (!Validated.succeeded())
          {
            return Validated;
          }
          ASTWalker{}.walk(Parsed.Unit->root(), [](const ASTNodeBase *)
                           {
                             return WalkAction::Continue;
                           },
                           [&](const ASTNodeBase *Node)
                           {
                             Ids.emplace(Node, Nodes.size() + 1);
                             Nodes.push_back(Node);
                           });
          const auto &Input = Parsed.Unit->input().lexedFile();
          put("ast " + std::to_string(ASTTextArchiveVersion) + " {\n  source_name = ");
          value(Input.sourceName());
          put("\n  source = ");
          value(Input.source());
          put("\n  status = ");
          value(Parsed.Status);
          put("\n  syntax_errors = ");
          value(Parsed.HasSyntaxErrors);
          put("\n  lexical_success = ");
          value(Input.succeeded());
          put("\n  tokens [\n");
          for (const auto &Token : Input.tokens())
          {
            put("    ");
            value(Token.Kind);
            put(" ");
            value(Token.Span);
            put(" ");
            std::visit([&](const auto &Payload)
                       {
                         using T = std::decay_t<decltype(Payload)>;
                         if constexpr (std::is_same_v<T, std::monostate>)
                         {
                           put("none");
                         }
                         else if constexpr (std::is_same_v<T, tokenizer::IdentifierInfo>)
                         {
                           put("identifier ");
                           value(std::string_view(Payload.Name));
                         }
                         else if constexpr (std::is_same_v<T, tokenizer::NumericInfo>)
                         {
                           put("numeric ");
                           value(Payload.Base);
                         }
                         else if constexpr (std::is_same_v<T, tokenizer::StringInfo>)
                         {
                           put("string ");
                           value(Payload.Mode);
                           put(" ");
                           value(std::string_view(Payload.Decoded));
                         }
                         else if constexpr (std::is_same_v<T, tokenizer::CharInfo>)
                         {
                           put("character ");
                           value(static_cast<std::uint32_t>(Payload.Value));
                           put(" ");
                           value(Payload.Raw);
                         }
                       },
                       Token.Payload);
            put("\n");
          }
          put("  ]\n  nodes [\n");
          for (const auto *Node : Nodes)
          {
            put("    %" + std::to_string(Ids.at(Node)) + " = ");
            switch (Node->getKind())
            {
#define AST_NODE(Name, Base, Category, Id)    \
  case ASTKind::Name:                         \
    put(#Name " ");                           \
    fields(*static_cast<const Name *>(Node)); \
    break;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
            }
            put("\n");
          }
          put("  ]\n  recovery [\n");
          for (const auto &Entry : Parsed.Unit->recoveryInfo().Entries)
          {
            put("    { node = ");
            value(Entry.Node);
            put(", expected = ");
            value(Entry.Token.Expected);
            put(", actual = ");
            value(Entry.Token.Actual);
            put(", range = ");
            value(Entry.Token.Range);
            put(", status = ");
            value(Entry.Token.Status);
            put(", skipped = ");
            value(Entry.Skipped);
            put(" }\n");
          }
          put("  ]\n}");
          if (!Good)
          {
            return {{}, core::ArchiveStatus::LimitExceeded, "AST text size or allocation limit exceeded"};
          }
          return {std::move(Output), core::ArchiveStatus::Success, {}};
        }

      private:
        void put(std::string_view Text)
        {
          const auto Maximum = std::min(Limits.MaxArchiveBytes, Limits.MaxAllocationBytes / 2);
          if (Text.size() > Maximum - Output.size())
          {
            Good = false;
          }
          if (Good)
          {
            Output += Text;
          }
        }

        template <typename T>
        void fields(const T &Object)
        {
          auto Labels = Fields<T>::Labels;
          put("{ ");
          bool First = true;
          std::apply([&](const auto &...Field)
                     {
                       const auto One = [&](const auto &Item)
                       {
                         if (!First)
                         {
                           put(", ");
                         }
                         First = false;
                         put(fieldLabel(Labels));
                         put(" = ");
                         value(Item);
                       };
                       (One(Field), ...);
                     },
                     Fields<T>::get(Object));
          if constexpr (std::is_base_of_v<ASTNodeBase, T>)
          {
            put(", comptime = ");
            value(Object.isComptime());
          }
          if constexpr (std::is_base_of_v<Decl, T>)
          {
            put(", visibility = ");
            value(Object.visibility());
          }
          put(" }");
        }

        template <typename T>
        void value(const T &Value)
        {
          if constexpr (std::is_same_v<T, bool>)
          {
            put(Value ? "true" : "false");
          }
          else if constexpr (std::is_integral_v<T>)
          {
            put(std::to_string(Value));
          }
          else if constexpr (std::is_enum_v<T>)
          {
            for (const auto &Entry : EnumValues<T>::Values)
            {
              if (Entry.Value == Value)
              {
                put(Entry.Spelling);
                return;
              }
            }
          }
          else if constexpr (std::is_pointer_v<T>)
          {
            put(Value ? "%" + std::to_string(Ids.at(Value)) : "null");
          }
          else if constexpr (std::is_same_v<T, SourceRange>)
          {
            put("range(" + std::to_string(Value.getBegin().getByteOffset()) + ", " + std::to_string(Value.getEnd().getByteOffset()) + ")");
          }
          else if constexpr (std::is_same_v<T, std::string_view> || std::is_same_v<T, std::string>)
          {
            put(core::archive::quote(Value));
          }
          else if constexpr (std::is_same_v<T, NameToken>)
          {
            put("name(");
            value(Value.Id == InvalidTokenId ? 0 : Value.Id + 1);
            put(", ");
            value(Value.Text);
            put(", ");
            value(Value.Range);
            put(")");
          }
          else if constexpr (std::is_same_v<T, RestBinding>)
          {
            put("rest(");
            value(Value.Name);
            put(", ");
            value(Value.Wildcard);
            put(", ");
            value(Value.EllipsisRange);
            put(")");
          }
          else if constexpr (OptionalType<T>::value)
          {
            if (Value)
            {
              put("some(");
              value(*Value);
              put(")");
            }
            else
            {
              put("none");
            }
          }
          else if constexpr (requires { Value.begin(); Value.size(); })
          {
            put("[");
            bool First = true;
            for (const auto &Element : Value)
            {
              if (!First)
              {
                put(", ");
              }
              First = false;
              value(Element);
            }
            put("]");
          }
          else
          {
            fields(Value);
          }
        }

        const ParseResult &Parsed;
        ASTArchiveLimits Limits;
        std::string Output;
        bool Good = true;
        std::vector<const ASTNodeBase *> Nodes;
        std::unordered_map<const ASTNodeBase *, std::size_t> Ids;
    };

    // Translate typed textual fields into the existing validated AST constructor stream.
    // This preserves recovery nodes and token payloads without reparsing source code.
    class ASTTextReader
    {
      public:
        ASTTextReader(core::FrontendContext &Context, std::string_view Text, ASTArchiveLimits Limits)
            : Context(Context),
              Text(Text),
              Limits(Limits),
              Lex(Text)
        {
        }

        ASTDeserializeResult run()
        {
          if (Text.size() > Limits.MaxArchiveBytes)
          {
            return {{}, core::ArchiveStatus::LimitExceeded, "AST text byte limit exceeded"};
          }
          Lex.expect("ast");
          if (Lex.number() != ASTTextArchiveVersion && Lex.good())
          {
            return {{}, core::ArchiveStatus::UnsupportedVersion, "Unsupported AST text version"};
          }
          Lex.expect("{");
          label("source_name");
          rawString(SourceNameRecord, Limits.MaxArchiveBytes);
          label("source");
          rawString(SourceTextRecord, Limits.MaxSourceBytes);
          label("status");
          read<ParseStatus>();
          label("syntax_errors");
          read<bool>();
          label("lexical_success");
          read<bool>();
          auto Header = std::move(Values);
          std::size_t TokenCount = 0;
          Lex.expect("tokens");
          Lex.expect("[");
          while (good() && !Lex.take("]"))
          {
            if (++TokenCount > Limits.MaxTokens)
            {
              limit();
              break;
            }
            read<TokenKind>();
            read<SourceRange>();
            if (Lex.take("none"))
            {
              push(0);
            }
            else if (Lex.take("identifier"))
            {
              push(1);
              read<std::string_view>();
            }
            else if (Lex.take("numeric"))
            {
              push(2);
              read<unsigned>();
            }
            else if (Lex.take("string"))
            {
              push(3);
              read<tokenizer::StringMode>();
              read<std::string_view>();
            }
            else if (Lex.take("character"))
            {
              push(4);
              read<std::uint32_t>();
              read<bool>();
            }
            else
            {
              Lex.fail("Unknown token payload");
            }
            emit(TokenRecord);
          }
          std::size_t NodeCount = 0;
          Lex.expect("nodes");
          Lex.expect("[");
          while (good() && !Lex.take("]"))
          {
            if (++NodeCount > Limits.MaxNodes)
            {
              limit();
              break;
            }
            Lex.expect("%");
            if (Lex.number() != NodeCount)
            {
              Lex.fail("AST nodes must use consecutive postorder IDs");
            }
            Lex.expect("=");
            const auto Kind = Lex.token();
            Lex.next();
            if (false)
            {
            }
#define AST_NODE(Name, Base, Category, Id) \
  else if (Kind == #Name)                  \
  {                                        \
    push(Id);                              \
    fields<Name>();                        \
  }
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
            else
            {
              Lex.fail("Unknown AST node kind");
            }
            emit(NodeRecord);
          }
          std::size_t RecoveryCount = 0;
          Lex.expect("recovery");
          Lex.expect("[");
          while (good() && !Lex.take("]"))
          {
            if (++RecoveryCount > Limits.MaxArrayElements)
            {
              limit();
              break;
            }
            Lex.expect("{");
            label("node");
            read<const ASTNodeBase *>();
            Lex.expect(",");
            label("expected");
            read<TokenKind>();
            Lex.expect(",");
            label("actual");
            read<std::optional<TokenId>>();
            Lex.expect(",");
            label("range");
            read<SourceRange>();
            Lex.expect(",");
            label("status");
            read<ExpectStatus>();
            Lex.expect(",");
            label("skipped");
            read<std::optional<SourceRange>>();
            Lex.expect("}");
            emit(RecoveryRecord);
          }
          Lex.expect("}");
          if (!Lex.token().empty())
          {
            Lex.fail("Trailing AST text");
          }
          if (!good())
          {
            return {{}, Status, Lex.message()};
          }
          llvm::SmallVector<char, 0> Bytes;
          llvm::BitstreamWriter Stream(Bytes);
          for (unsigned char C : std::string_view("IAST"))
          {
            Stream.Emit(C, 8);
          }
          Stream.EnterSubblock(ArchiveBlock, CodeWidth);
          Header.insert(Header.begin(), ASTArchiveVersion);
          Header.insert(Header.end(), {TokenCount, NodeCount, RecoveryCount});
          Stream.EmitRecord(HeaderRecord, Header);
          for (const auto &Record : Records)
          {
            Stream.EmitRecord(Record.first, Record.second);
          }
          Stream.ExitBlock();
          if (!charge(Bytes.size(), 2))
          {
            return {{}, Status, Lex.message()};
          }
          auto DecodeLimits = Limits;
          DecodeLimits.MaxAllocationBytes -= Allocated;
          auto Result = ASTReader(Context, std::string_view(Bytes.data(), Bytes.size()), DecodeLimits, false).run();
          Result.AllocationBytes += Allocated;
          return Result;
        }

      private:
        bool good() const
        {
          return Lex.good() && Status == core::ArchiveStatus::InvalidArchive;
        }
        void limit()
        {
          Status = core::ArchiveStatus::LimitExceeded;
          Lex.fail("AST text allocation or element limit exceeded");
        }
        bool charge(std::size_t Count, std::size_t Size)
        {
          if (Size && Count > (Limits.MaxAllocationBytes - Allocated) / Size)
          {
            limit();
            return false;
          }
          Allocated += Count * Size;
          return true;
        }
        void push(std::uint64_t Value)
        {
          if (good() && charge(1, 32))
          {
            Values.push_back(Value);
          }
        }
        void label(std::string_view Name)
        {
          Lex.expect(Name);
          Lex.expect("=");
        }
        void emit(unsigned Code)
        {
          if (good() && charge(1, 64))
          {
            Records.emplace_back(Code, std::move(Values));
            Values.clear();
          }
        }
        void rawString(unsigned Code, std::size_t Maximum)
        {
          if (Lex.token().size() > Limits.MaxAllocationBytes - Allocated)
          {
            limit();
            return;
          }
          auto Value = Lex.string();
          if (Value.size() > Maximum)
          {
            limit();
            return;
          }
          for (unsigned char C : Value)
          {
            push(C);
          }
          emit(Code);
        }
        template <typename T, std::size_t... I>
        void fieldTuple(std::index_sequence<I...>)
        {
          auto Labels = Fields<T>::Labels;
          Lex.expect("{");
          const auto One = [&]<std::size_t Index>()
          {
            if constexpr (Index != 0)
            {
              Lex.expect(",");
            }
            label(fieldLabel(Labels));
            read<std::tuple_element_t<Index, FieldTuple<T>>>();
          };
          (One.template operator()<I>(), ...);
          if constexpr (std::is_base_of_v<ASTNodeBase, T>)
          {
            Lex.expect(",");
            label("comptime");
            read<bool>();
          }
          if constexpr (std::is_base_of_v<Decl, T>)
          {
            Lex.expect(",");
            label("visibility");
            read<DeclarationVisibility>();
          }
          Lex.expect("}");
        }
        template <typename T>
        void fields()
        {
          fieldTuple<T>(std::make_index_sequence<std::tuple_size_v<FieldTuple<T>>>{});
        }

        template <typename T>
        void read()
        {
          if (!good())
          {
            return;
          }
          if constexpr (std::is_same_v<T, bool>)
          {
            if (Lex.take("true"))
            {
              push(1);
            }
            else if (Lex.take("false"))
            {
              push(0);
            }
            else
            {
              Lex.fail("Expected boolean");
            }
          }
          else if constexpr (std::is_integral_v<T>)
          {
            push(Lex.number());
          }
          else if constexpr (std::is_enum_v<T>)
          {
            for (const auto &Entry : EnumValues<T>::Values)
            {
              if (Lex.take(Entry.Spelling))
              {
                push(Entry.Id);
                return;
              }
            }
            Lex.fail("Unknown AST enum spelling");
          }
          else if constexpr (std::is_pointer_v<T>)
          {
            if (Lex.take("null"))
            {
              push(0);
            }
            else
            {
              Lex.expect("%");
              push(Lex.number());
            }
          }
          else if constexpr (std::is_same_v<T, SourceRange>)
          {
            Lex.expect("range");
            Lex.expect("(");
            const auto Begin = Lex.number();
            Lex.expect(",");
            const auto End = Lex.number();
            Lex.expect(")");
            if (Begin == UINT64_MAX || End == UINT64_MAX)
            {
              Lex.fail("Source offset overflow");
            }
            push(Begin + 1);
            push(End + 1);
          }
          else if constexpr (std::is_same_v<T, std::string_view>)
          {
            if (Lex.token().size() > Limits.MaxAllocationBytes - Allocated)
            {
              limit();
              return;
            }
            const auto String = Lex.string();
            push(String.size());
            for (unsigned char C : String)
            {
              push(C);
            }
          }
          else if constexpr (std::is_same_v<T, NameToken>)
          {
            Lex.expect("name");
            Lex.expect("(");
            read<std::uint64_t>();
            Lex.expect(",");
            read<std::string_view>();
            Lex.expect(",");
            read<SourceRange>();
            Lex.expect(")");
          }
          else if constexpr (std::is_same_v<T, RestBinding>)
          {
            Lex.expect("rest");
            Lex.expect("(");
            read<decltype(RestBinding::Name)>();
            Lex.expect(",");
            read<bool>();
            Lex.expect(",");
            read<SourceRange>();
            Lex.expect(")");
          }
          else if constexpr (OptionalType<T>::value)
          {
            if (Lex.take("none"))
            {
              push(0);
            }
            else
            {
              Lex.expect("some");
              Lex.expect("(");
              push(1);
              read<typename OptionalType<T>::Element>();
              Lex.expect(")");
            }
          }
          else if constexpr (ArrayType<T>::value)
          {
            Lex.expect("[");
            const auto Index = Values.size();
            push(0);
            std::size_t Count = 0;
            while (good() && !Lex.take("]"))
            {
              if (Count && !Lex.expect(","))
              {
                break;
              }
              if (++Count > Limits.MaxArrayElements)
              {
                limit();
                break;
              }
              read<typename ArrayType<T>::Element>();
            }
            if (good())
            {
              Values[Index] = Count;
            }
          }
          else
          {
            fields<T>();
          }
        }

        core::FrontendContext &Context;
        std::string_view Text;
        ASTArchiveLimits Limits;
        core::archive::Lexer Lex;
        core::ArchiveStatus Status = core::ArchiveStatus::InvalidArchive;
        std::size_t Allocated = 0;
        std::vector<std::uint64_t> Values;
        std::vector<std::pair<unsigned, std::vector<std::uint64_t>>> Records;
    };
  } // namespace

  ASTSerializeResult trySerializeASTText(const ParseResult &Parsed, ASTArchiveLimits Limits)
  {
    return ASTTextWriter(Parsed, Limits).run();
  }

  ASTDeserializeResult tryDeserializeASTText(core::FrontendContext &Context, std::string_view Text, ASTArchiveLimits Limits)
  {
    return ASTTextReader(Context, Text, Limits).run();
  }
} // namespace ink::parser
