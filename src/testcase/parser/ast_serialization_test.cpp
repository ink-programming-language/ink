#include "parser_test_support.h"
#include "ink/parser/ast_serialization.h"
#include <llvm/ADT/SmallVector.h>
#include <llvm/Bitstream/BitstreamReader.h>
#include <llvm/Bitstream/BitstreamWriter.h>
#include <gmock/gmock.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <array>
#include <csignal>
#include <cstdlib>
#include <limits>
#include <set>

namespace ink::parser::test
{
  namespace
  {
    struct GrammarCase
    {
        const char *Name;
        const char *Rule;
        const char *Derivation;
        const char *Source;
    };
#include "corpus/grammar_cases.inc"

    struct Record
    {
        unsigned Code;
        std::vector<std::uint64_t> Values;
    };

    // Use LLVM's general reader as an independent check of the emitted container.
    std::vector<Record> records(std::string_view Bytes)
    {
      llvm::BitstreamCursor Cursor(llvm::StringRef(Bytes.data(), Bytes.size()));
      if (auto Error = Cursor.JumpToBit(32))
      {
        ADD_FAILURE() << llvm::toString(std::move(Error));
        return {};
      }
      auto Block = Cursor.advance();
      if (!Block)
      {
        ADD_FAILURE() << llvm::toString(Block.takeError());
        return {};
      }
      EXPECT_EQ(Block->Kind, llvm::BitstreamEntry::SubBlock);
      EXPECT_EQ(Block->ID, 8U);
      if (auto Error = Cursor.EnterSubBlock(Block->ID))
      {
        ADD_FAILURE() << llvm::toString(std::move(Error));
        return {};
      }
      std::vector<Record> Result;
      while (!Cursor.AtEndOfStream())
      {
        auto Entry = Cursor.advance();
        if (!Entry)
        {
          ADD_FAILURE() << llvm::toString(Entry.takeError());
          return {};
        }
        if (Entry->Kind == llvm::BitstreamEntry::EndBlock)
        {
          EXPECT_TRUE(Cursor.AtEndOfStream());
          return Result;
        }
        if (Entry->Kind != llvm::BitstreamEntry::Record)
        {
          ADD_FAILURE() << "unexpected entry";
          return {};
        }
        llvm::SmallVector<std::uint64_t, 16> Values;
        auto Code = Cursor.readRecord(Entry->ID, Values);
        if (!Code)
        {
          ADD_FAILURE() << llvm::toString(Code.takeError());
          return {};
        }
        Result.push_back({*Code, {Values.begin(), Values.end()}});
      }
      ADD_FAILURE() << "missing end block";
      return {};
    }

    std::string archive(const std::vector<Record> &Records)
    {
      llvm::SmallVector<char, 0> Bytes;
      llvm::BitstreamWriter Writer(Bytes);
      for (unsigned char Byte : std::string_view("IAST"))
      {
        Writer.Emit(Byte, 8);
      }
      Writer.EnterSubblock(8, 3);
      for (const auto &Entry : Records)
      {
        Writer.EmitRecord(Entry.Code, Entry.Values);
      }
      Writer.ExitBlock();
      return {Bytes.begin(), Bytes.end()};
    }

    Record &nodeRecord(std::vector<Record> &Records, ASTKind Kind)
    {
      for (auto &Entry : Records)
      {
        if (Entry.Code == 5 && Entry.Values.front() == static_cast<std::uint64_t>(Kind))
        {
          return Entry;
        }
      }
      ADD_FAILURE() << "node not present: " << astKindName(Kind);
      return Records.front();
    }

    void expectTokensEqual(const tokenizer::TokenizedBuffer &Left, const tokenizer::TokenizedBuffer &Right)
    {
      EXPECT_EQ(Left.source(), Right.source());
      EXPECT_EQ(Left.sourceName(), Right.sourceName());
      EXPECT_EQ(Left.lineStarts(), Right.lineStarts());
      EXPECT_EQ(Left.succeeded(), Right.succeeded());
      ASSERT_EQ(Left.tokens().size(), Right.tokens().size());
      for (std::size_t Index = 0; Index < Left.tokens().size(); ++Index)
      {
        const auto &A = Left.tokens()[Index];
        const auto &B = Right.tokens()[Index];
        SCOPED_TRACE(Index);
        EXPECT_EQ(A.Kind, B.Kind);
        EXPECT_EQ(A.Span, B.Span);
        EXPECT_EQ(Left.raw(A), Right.raw(B));
        ASSERT_EQ(A.Payload.index(), B.Payload.index());
        std::visit([&](const auto &Value)
                   {
                     using T = std::decay_t<decltype(Value)>;
                     const auto &Other = std::get<T>(B.Payload);
                     if constexpr (std::is_same_v<T, tokenizer::IdentifierInfo>)
                     {
                       EXPECT_EQ(Value.Name, Other.Name);
                     }
                     else if constexpr (std::is_same_v<T, tokenizer::NumericInfo>)
                     {
                       EXPECT_EQ(Value.Base, Other.Base);
                     }
                     else if constexpr (std::is_same_v<T, tokenizer::StringInfo>)
                     {
                       EXPECT_EQ(Value.Mode, Other.Mode);
                       EXPECT_EQ(Value.Decoded, Other.Decoded);
                     }
                     else if constexpr (std::is_same_v<T, tokenizer::CharInfo>)
                     {
                       EXPECT_EQ(Value.Value, Other.Value);
                       EXPECT_EQ(Value.Raw, Other.Raw);
                     }
                   },
                   A.Payload);
      }
    }

    void expectRoundTrip(const ParseResult &Original, core::FrontendContext &Frontend, std::set<ASTKind> *Kinds = nullptr)
    {
      std::vector<bool> OriginalComptime;
      ASTWalker{}.walk(Original.Unit->root(), [&](const ASTNodeBase *Node)
      {
        OriginalComptime.push_back(Node->isComptime());
        return WalkAction::Continue;
      });
      for (bool Text : {false, true})
      {
        SCOPED_TRACE(Text ? "text" : "binary");
        const auto Saved = Text ? trySerializeASTText(Original) : serializeAST(Frontend, Original);
        ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
        auto Loaded = Text ? tryDeserializeASTText(Frontend, Saved.Bytes) : deserializeAST(Frontend, Saved.Bytes);
        ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
        EXPECT_EQ(Loaded.Parsed.Status, Original.Status);
        EXPECT_EQ(Loaded.Parsed.HasSyntaxErrors, Original.HasSyntaxErrors);
        EXPECT_EQ(Loaded.Parsed.succeeded(), Original.succeeded());
        EXPECT_NE(Loaded.Parsed.Unit->root(), Original.Unit->root());
        EXPECT_TRUE(Loaded.Parsed.Unit->input().lexedFile().isRegisteredWith(Frontend.sourceManager()));
        EXPECT_NE(Loaded.Parsed.Unit->input().lexedFile().sourceId(), Original.Unit->input().lexedFile().sourceId());
        EXPECT_EQ(dumpAST(*Loaded.Parsed.Unit), dumpAST(*Original.Unit));
        std::vector<bool> LoadedComptime;
        ASTWalker{}.walk(Loaded.Parsed.Unit->root(), [&](const ASTNodeBase *Node)
        {
          LoadedComptime.push_back(Node->isComptime());
          return WalkAction::Continue;
        });
        EXPECT_EQ(LoadedComptime, OriginalComptime);
        EXPECT_TRUE(verifyAST(Loaded.Parsed.Unit->root(), Loaded.Parsed.Unit->input().lexedFile().source().size()));
        expectTokensEqual(Original.Unit->input().lexedFile(), Loaded.Parsed.Unit->input().lexedFile());
        const auto &Before = Original.Unit->recoveryInfo().Entries;
        const auto &After = Loaded.Parsed.Unit->recoveryInfo().Entries;
        ASSERT_EQ(Before.size(), After.size());
        for (std::size_t Index = 0; Index < Before.size(); ++Index)
        {
          EXPECT_NE(Before[Index].Node, After[Index].Node);
          EXPECT_EQ(Before[Index].Node->getKind(), After[Index].Node->getKind());
          EXPECT_EQ(Before[Index].Token.Expected, After[Index].Token.Expected);
          EXPECT_EQ(Before[Index].Token.Actual, After[Index].Token.Actual);
          EXPECT_EQ(Before[Index].Token.Range, After[Index].Token.Range);
          EXPECT_EQ(Before[Index].Token.Status, After[Index].Token.Status);
          EXPECT_EQ(Before[Index].Skipped, After[Index].Skipped);
        }
        const auto Repeated = Text ? trySerializeASTText(Loaded.Parsed) : serializeAST(Frontend, Loaded.Parsed);
        ASSERT_TRUE(Repeated.succeeded()) << Repeated.Message;
        EXPECT_EQ(Repeated.Bytes, Saved.Bytes);
        if (Kinds)
        {
          ASTWalker{}.walk(Loaded.Parsed.Unit->root(), [&](const ASTNodeBase *Node)
                           {
                             Kinds->insert(Node->getKind());
                             return WalkAction::Continue;
                           });
        }
      }
    }

    void expectRejected(core::FrontendContext &Frontend, std::string_view Bytes, ASTArchiveStatus Status = ASTArchiveStatus::InvalidArchive, ASTArchiveLimits Limits = {})
    {
      const char *Pattern = Status == ASTArchiveStatus::UnsupportedVersion ? "internal compiler error\\[INK-P0023\\]" : "internal compiler error\\[INK-P";
      EXPECT_DEATH(deserializeAST(Frontend, Bytes, Limits), Pattern);
    }

    void expectWriteRejected(core::FrontendContext &Frontend, const ParseResult &Parsed, core::DiagnosticKind Kind, ASTArchiveLimits Limits = {})
    {
      const std::string Pattern = std::string("internal compiler error\\[") + core::diagnosticCode(Kind) + "\\]";
      EXPECT_DEATH(serializeAST(Frontend, Parsed, Limits), Pattern);
    }

    bool completedOrPanicked(int Status)
    {
#ifdef _WIN32
      return ::testing::ExitedWithCode(0)(Status) || ::testing::ExitedWithCode(3)(Status);
#else
      return ::testing::ExitedWithCode(0)(Status) || ::testing::KilledBySignal(SIGABRT)(Status);
#endif
    }
  } // namespace

  // Text snapshots reject truncation, unknown enums/fields and bounded mutations through explicit status.
  TEST_F(ParserTest, ASTTextSerializationRejectsMalformedInput)
  {
    const auto Original = read("func Identity[T: type](Value: T): T { return Value; }");
    const auto Saved = trySerializeASTText(Original);
    ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
    EXPECT_NE(Saved.Bytes.find("FunctionDecl { range = range("), std::string::npos);
    EXPECT_NE(Saved.Bytes.find("genericParameters = ["), std::string::npos);
    EXPECT_EQ(Saved.Bytes.find("IAST"), std::string::npos);
    for (std::size_t Length = 0; Length < Saved.Bytes.size(); ++Length)
    {
      const auto Result = tryDeserializeASTText(Frontend, std::string_view(Saved.Bytes).substr(0, Length));
      EXPECT_FALSE(Result.succeeded()) << Length;
      EXPECT_EQ(Result.Parsed.Unit, nullptr);
    }
    const std::pair<std::string_view, std::string_view> Changes[] = {
        {"Completed", "InvalidState"},
        {"genericParameters", "unknownField"},
        {"FunctionDecl", "UnknownNode"},
        {"source_name", "invalid_name"},
    };
    for (const auto &[From, To] : Changes)
    {
      auto Text = Saved.Bytes;
      ASSERT_NE(Text.find(From), std::string::npos);
      Text.replace(Text.find(From), From.size(), To);
      const auto Result = tryDeserializeASTText(Frontend, Text);
      EXPECT_EQ(Result.Status, ASTArchiveStatus::InvalidArchive) << Result.Message;
    }
    for (std::size_t Index = 0; Index < Saved.Bytes.size(); Index += 7)
    {
      auto Text = Saved.Bytes;
      Text[Index] = static_cast<char>((Index * 37) & 255);
      const auto Result = tryDeserializeASTText(Frontend, Text);
      if (Result.succeeded())
      {
        EXPECT_TRUE(verifyAST(Result.Parsed.Unit->root(), Result.Parsed.Unit->input().lexedFile().source().size()));
      }
      else
      {
        EXPECT_EQ(Result.Parsed.Unit, nullptr);
      }
    }
    ASTArchiveLimits Limits;
    Limits.MaxAllocationBytes = 64;
    EXPECT_EQ(tryDeserializeASTText(Frontend, Saved.Bytes, Limits).Status, ASTArchiveStatus::LimitExceeded);
  }

  // Ordinary expression and statement nodes retain their comptime flags and prefix-inclusive ranges in both formats.
  TEST_F(ParserTest, ASTSerializationComptimeFlags)
  {
    const auto Parsed = read("comptime 42; comptime comptime 'x'; comptime(a + b); comptime if (x) {} comptime { var value = comptime 7; } var plain = 8;");
    ASSERT_TRUE(Parsed.succeeded());
    const auto *Literal = cast<LiteralExpr>(expression(Parsed));
    EXPECT_TRUE(Literal->isComptime());
    EXPECT_EQ(Literal->getSourceRange(), SourceRange::fromByteOffsets(0, 11));
    EXPECT_TRUE(isa<LiteralExpr>(expression(Parsed, 1)));
    EXPECT_TRUE(expression(Parsed, 1)->isComptime());
    EXPECT_TRUE(isa<ParenExpr>(expression(Parsed, 2)));
    EXPECT_TRUE(expression(Parsed, 2)->isComptime());
    const auto *If = Parsed.Unit->root()->statements()[3];
    EXPECT_TRUE(isa<IfStmt>(If));
    EXPECT_TRUE(If->isComptime());
    const auto *Block = Parsed.Unit->root()->statements()[4];
    EXPECT_TRUE(isa<BlockStmt>(Block));
    EXPECT_TRUE(Block->isComptime());
    EXPECT_FALSE(cast<VarDecl>(declaration(Parsed, 5))->initializer()->isComptime());
    expectRoundTrip(Parsed, Frontend);
  }

  // The common node flag is mandatory, boolean-valued, and incompatible with the previous wrapper-node versions.
  TEST_F(ParserTest, ASTSerializationRejectsMalformedComptimeFlagsAndLegacyVersions)
  {
    const auto Parsed = read("comptime 42;");
    ASSERT_TRUE(Parsed.succeeded());
    const auto Binary = trySerializeAST(Parsed);
    ASSERT_TRUE(Binary.succeeded()) << Binary.Message;
    const auto OriginalRecords = records(Binary.Bytes);
    auto Records = OriginalRecords;
    nodeRecord(Records, ASTKind::LiteralExpr).Values.back() = 2;
    EXPECT_EQ(tryDeserializeAST(Frontend, archive(Records)).Status, ASTArchiveStatus::InvalidArchive);
    Records = OriginalRecords;
    nodeRecord(Records, ASTKind::LiteralExpr).Values.pop_back();
    EXPECT_EQ(tryDeserializeAST(Frontend, archive(Records)).Status, ASTArchiveStatus::InvalidArchive);
    for (std::uint64_t Version : {1ULL, 2ULL, 3ULL})
    {
      Records = OriginalRecords;
      Records.front().Values.front() = Version;
      EXPECT_EQ(tryDeserializeAST(Frontend, archive(Records)).Status, ASTArchiveStatus::UnsupportedVersion);
    }
    const auto Text = trySerializeASTText(Parsed);
    ASSERT_TRUE(Text.succeeded()) << Text.Message;
    const std::string_view Flag = ", comptime = true";
    const auto Position = Text.Bytes.find(Flag);
    ASSERT_NE(Position, std::string::npos);
    for (const std::string_view Replacement : {"", ", comptime = 2"})
    {
      auto Malformed = Text.Bytes;
      Malformed.replace(Position, Flag.size(), Replacement);
      EXPECT_EQ(tryDeserializeASTText(Frontend, Malformed).Status, ASTArchiveStatus::InvalidArchive);
    }
    auto Legacy = Text.Bytes;
    Legacy.replace(0, ("ast " + std::to_string(ASTTextArchiveVersion)).size(), "ast 1");
    EXPECT_EQ(tryDeserializeASTText(Frontend, Legacy).Status, ASTArchiveStatus::UnsupportedVersion);
  }

  // Visibility is preserved for every declaration category and nested declarations in both binary and textual snapshots.
  TEST_F(ParserTest, ASTSerializationDeclarationVisibility)
  {
    const auto Parsed = read("func plain(): i32 { return 0; } public func exposed(): i32 { private func hidden(): i32 { return 1; } return hidden(); } private var value = 2; public const fixed = 3; private field X: T; public class C; private enum E {}; public interface I {}; public import \"C\" func native(): i32;");
    ASSERT_TRUE(Parsed.succeeded());
    expectRoundTrip(Parsed, Frontend);
    const auto Text = trySerializeASTText(Parsed);
    ASSERT_TRUE(Text.succeeded()) << Text.Message;
    EXPECT_NE(Text.Bytes.find("visibility = Default"), std::string::npos);
    EXPECT_NE(Text.Bytes.find("visibility = Public"), std::string::npos);
    EXPECT_NE(Text.Bytes.find("visibility = Private"), std::string::npos);
    for (bool IsText : {false, true})
    {
      const auto Saved = IsText ? trySerializeASTText(Parsed) : trySerializeAST(Parsed);
      ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
      auto Loaded = IsText ? tryDeserializeASTText(Frontend, Saved.Bytes) : tryDeserializeAST(Frontend, Saved.Bytes);
      ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
      EXPECT_EQ(declaration(Loaded.Parsed, 0)->visibility(), DeclarationVisibility::Default);
      EXPECT_EQ(declaration(Loaded.Parsed, 1)->visibility(), DeclarationVisibility::Public);
      EXPECT_EQ(declaration(Loaded.Parsed, 2)->visibility(), DeclarationVisibility::Private);
      const auto *Nested = cast<DeclStmt>(cast<FunctionDecl>(declaration(Loaded.Parsed, 1))->body()->statements()[0])->declaration();
      EXPECT_EQ(Nested->visibility(), DeclarationVisibility::Private);
    }
  }

  // Declaration visibility is mandatory and uses a closed enum; missing or invalid tags and the preceding text format are rejected.
  TEST_F(ParserTest, ASTSerializationRejectsMalformedVisibility)
  {
    const auto Parsed = read("public func f(): i32 { return 0; }");
    ASSERT_TRUE(Parsed.succeeded());
    const auto Saved = trySerializeAST(Parsed);
    ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
    const auto Original = records(Saved.Bytes);
    auto Records = Original;
    nodeRecord(Records, ASTKind::FunctionDecl).Values.back() = 99;
    EXPECT_EQ(tryDeserializeAST(Frontend, archive(Records)).Status, ASTArchiveStatus::InvalidArchive);
    Records = Original;
    nodeRecord(Records, ASTKind::FunctionDecl).Values.pop_back();
    EXPECT_EQ(tryDeserializeAST(Frontend, archive(Records)).Status, ASTArchiveStatus::InvalidArchive);
    const auto Text = trySerializeASTText(Parsed);
    ASSERT_TRUE(Text.succeeded()) << Text.Message;
    const std::string_view Field = ", visibility = Public";
    const auto Position = Text.Bytes.find(Field);
    ASSERT_NE(Position, std::string::npos);
    for (std::string_view Replacement : {"", ", visibility = Unknown", ", visibility = 1"})
    {
      std::string Invalid = Text.Bytes;
      Invalid.replace(Position, Field.size(), Replacement);
      EXPECT_EQ(tryDeserializeASTText(Frontend, Invalid).Status, ASTArchiveStatus::InvalidArchive);
    }
    std::string Legacy = Text.Bytes;
    Legacy.replace(0, ("ast " + std::to_string(ASTTextArchiveVersion)).size(), "ast 2");
    EXPECT_EQ(tryDeserializeASTText(Frontend, Legacy).Status, ASTArchiveStatus::UnsupportedVersion);
  }

  // Every accepted grammar sample round-trips with identical fields, tokens, recovery state and canonical bytes.
  TEST_F(ParserTest, ASTSerializationGrammarCorpus)
  {
    std::set<ASTKind> Kinds;
    const auto Check = [&](const auto &Cases)
    {
      for (const auto &Case : Cases)
      {
        SCOPED_TRACE(Case.Name);
        SCOPED_TRACE(Case.Source);
        core::CompilationContext LocalCompilation;
        core::FrontendContext LocalFrontend(LocalCompilation);
        const auto Parsed = parse(LocalFrontend, tokenizer::tokenize(LocalFrontend, Case.Source));
        ASSERT_TRUE(Parsed.succeeded());
        expectRoundTrip(Parsed, LocalFrontend, &Kinds);
      }
    };
    Check(GrammarBranchCases);
    Check(GrammarCombinationCases);
    constexpr ASTKind RecoveryKinds[] = {
        ASTKind::MissingExpr,
        ASTKind::ErrorExpr,
        ASTKind::MissingStmt,
        ASTKind::ErrorStmt,
        ASTKind::MissingDecl,
        ASTKind::ErrorDecl,
        ASTKind::MissingBindingPattern,
        ASTKind::ErrorBindingPattern,
        ASTKind::MissingMatchPattern,
        ASTKind::ErrorMatchPattern,
    };
    for (ASTKind Kind : RecoveryKinds)
    {
      Kinds.insert(Kind);
    }
#define AST_NODE(Name, Base, Category, Id) EXPECT_TRUE(Kinds.contains(ASTKind::Name)) << #Name;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
  }

  // All ten error/missing node kinds restore through their typed parents, even when normal parsing rarely produces them.
  TEST_F(ParserTest, ASTSerializationEveryRecoveryKind)
  {
    const auto Empty = serializeAST(Frontend, read(""));
    ASSERT_TRUE(Empty.succeeded());
    for (unsigned Kind = 2; Kind <= 11; ++Kind)
    {
      SCOPED_TRACE(Kind);
      auto Records = records(Empty.Bytes);
      Records.pop_back();
      Records.front().Values[2] = 1;
      const auto Add = [&](ASTKind Type, std::initializer_list<std::uint64_t> Fields)
      {
        Record Entry{5, {static_cast<std::uint64_t>(Type), 1, 1}};
        Entry.Values.insert(Entry.Values.end(), Fields.begin(), Fields.end());
        Entry.Values.push_back(0);
        if (categoryOf(Type) == ASTCategory::Decl)
        {
          Entry.Values.push_back(0);
        }
        Records.push_back(std::move(Entry));
      };
      const auto Type = static_cast<ASTKind>(Kind);
      if (Type == ASTKind::MissingDecl || Type == ASTKind::ErrorDecl)
      {
        Add(Type, {0});
        Add(ASTKind::DeclStmt, {1});
        Add(ASTKind::ModuleAST, {1, 2});
      }
      else if (categoryOf(Type) == ASTCategory::Stmt)
      {
        Add(Type, {});
        Add(ASTKind::ModuleAST, {1, 1});
      }
      else if (categoryOf(Type) == ASTCategory::BindingPattern)
      {
        Add(Type, {});
        Add(ASTKind::MissingExpr, {});
        Add(ASTKind::VarDecl, {0, 0, 1, 0, 2, 1});
        Add(ASTKind::DeclStmt, {3});
        Add(ASTKind::ModuleAST, {1, 4});
      }
      else
      {
        Add(Type, {});
        std::uint64_t Expression = 1;
        if (categoryOf(Type) == ASTCategory::MatchPattern)
        {
          Add(ASTKind::MissingExpr, {});
          Add(ASTKind::MissingExpr, {});
          Add(ASTKind::MatchExpr, {2, 1, 1, 0, 3, 1, 1});
          Expression = 4;
        }
        Add(ASTKind::ExprItem, {Expression});
        Add(ASTKind::SimpleStmt, {1, Expression + 1});
        Add(ASTKind::ModuleAST, {1, Expression + 2});
      }
      std::size_t NodeCount = 0;
      for (const auto &Entry : Records)
      {
        NodeCount += Entry.Code == 5;
      }
      Records.front().Values[5] = NodeCount;
      auto Loaded = deserializeAST(Frontend, archive(Records));
      ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
      EXPECT_FALSE(Loaded.Parsed.succeeded());
      std::set<ASTKind> Kinds;
      expectRoundTrip(Loaded.Parsed, Frontend, &Kinds);
      EXPECT_TRUE(Kinds.contains(Type));
    }
  }

  // Parse errors, missing delimiters and malformed lexical prefixes preserve partial trees and recovery records.
  TEST_F(ParserTest, ASTSerializationRecoveryAndLexicalFailure)
  {
    const char *Sources[] = {
        "var value = ; var kept = 2;",
        "func f(x: T { return x; }",
        "f(1,,2); var kept = 3;",
        "if (x) else {}",
        "var 42 = x;",
        "match(x) { => 1, @ => 2 };",
        "var valid = 1; @",
        "\xC0\xAF",
    };
    for (const char *Source : Sources)
    {
      SCOPED_TRACE(Source);
      const auto Parsed = read(Source);
      EXPECT_FALSE(Parsed.succeeded());
      expectRoundTrip(Parsed, Frontend);
    }
  }

  // Cancellation retains its status in a round trip; parser resource exhaustion terminates before producing an archive.
  TEST_F(ParserTest, ASTSerializationInterruptedParses)
  {
    ParseLimits Limits;
    Limits.IsCancelled = []()
    {
      return true;
    };
    const auto Cancelled = read("var x = 1;", Limits);
    ASSERT_EQ(Cancelled.Status, ParseStatus::Cancelled);
    expectRoundTrip(Cancelled, Frontend);
    Limits = {};
    Limits.MaxWork = 3;
    EXPECT_DEATH(read("func f(x: T): T { return x; }", Limits), "internal compiler error\\[INK-P0004\\]");
  }

  // Names, nondecimal numbers, raw/multiline strings, escaped NUL and Unicode scalars preserve their decoded payloads.
  TEST_F(ParserTest, ASTSerializationLiteralPayloads)
  {
    const auto Parsed = read("var 中文 = (0b101, 0o17, 0xFF, 12.5, '界', r'字', \"a\\0b\", r\"raw\\n\", \"\"\"line\nnext\"\"\", r\"\"\"raw\nnext\"\"\");\r\n中文;");
    ASSERT_TRUE(Parsed.succeeded());
    const auto Saved = serializeAST(Frontend, Parsed);
    ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
    auto Loaded = deserializeAST(Frontend, Saved.Bytes);
    ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
    bool SawNul = false;
    std::set<tokenizer::StringMode> Modes;
    for (const auto &Token : Loaded.Parsed.Unit->input().lexedFile().tokens())
    {
      if (const auto *Info = std::get_if<tokenizer::StringInfo>(&Token.Payload))
      {
        Modes.insert(Info->Mode);
        SawNul |= Info->Decoded == std::string("a\0b", 3);
      }
    }
    EXPECT_TRUE(SawNul);
    EXPECT_EQ(Modes.size(), 4U);
    expectRoundTrip(Parsed, Frontend);
  }

  // Linkage literals and recovered missing linkage survive snapshots without restricting vendor names or decoded bytes.
  TEST_F(ParserTest, ASTSerializationNativeLinkage)
  {
    const char *Sources[] = {
        "import \"C\" func f(msg: *u8): i32;",
        "[tag] export \"vendor.custom\" func f(): void {}",
        "import \"\\x43++\" func f(): void;",
        "import \"C\\0other\" func f(): void;",
        "import func f(): void;",
        "private export \"C\" func f(): void {}",
        "[abi(\"C\")] private func f(): void {}",
        "export func f(): void {}",
    };
    for (const char *Source : Sources)
    {
      SCOPED_TRACE(Source);
      const auto Parsed = read(Source);
      expectRoundTrip(Parsed, Frontend);
    }
  }

  // Binary and text snapshots reject unknown native directions and direction/linkage mismatches.
  TEST_F(ParserTest, ASTSerializationRejectsInvalidNativeSymbolKind)
  {
    const auto Parsed = read("private export \"C\" func f(): void {}");
    ASSERT_TRUE(Parsed.succeeded());
    const auto Saved = serializeAST(Frontend, Parsed);
    ASSERT_TRUE(Saved.succeeded());
    for (std::uint64_t Invalid : {0U, 99U})
    {
      auto Records = records(Saved.Bytes);
      auto &Values = nodeRecord(Records, ASTKind::FunctionDecl).Values;
      Values[Values.size() - 3] = Invalid;
      expectRejected(Frontend, archive(Records));
    }
    const auto Text = trySerializeASTText(Parsed);
    ASSERT_TRUE(Text.succeeded());
    ASSERT_NE(Text.Bytes.find("nativeSymbolKind = Export"), std::string::npos);
    auto Invalid = Text.Bytes;
    Invalid.replace(Invalid.find("nativeSymbolKind = Export"), std::string_view("nativeSymbolKind = Export").size(), "nativeSymbolKind = None");
    EXPECT_FALSE(tryDeserializeASTText(Frontend, Invalid).succeeded());
  }

  // A function linkage field cannot smuggle a non-string expression through the archive schema.
  TEST_F(ParserTest, ASTSerializationRejectsNonStringLinkage)
  {
    const auto Parsed = read("import \"C\" func f(): void;");
    ASSERT_TRUE(Parsed.succeeded());
    const auto Saved = serializeAST(Frontend, Parsed);
    ASSERT_TRUE(Saved.succeeded());
    auto Records = records(Saved.Bytes);
    bool Changed = false;
    for (auto &Entry : Records)
    {
      if (Entry.Code == 5 && Entry.Values[0] == static_cast<std::uint64_t>(ASTKind::FunctionDecl))
      {
        // The immediately preceding node is the return TypeSyntax, whose child is a valid NameExpr.
        for (const auto &Candidate : Records)
        {
          if (Candidate.Code == 5 && Candidate.Values[0] == static_cast<std::uint64_t>(ASTKind::TypeSyntax))
          {
            Entry.Values[Entry.Values.size() - 4] = Candidate.Values[Candidate.Values.size() - 2];
            Changed = true;
            break;
          }
        }
      }
    }
    ASSERT_TRUE(Changed);
    expectRejected(Frontend, archive(Records));
  }

  // Loaded source, names and token payloads outlive the producer, input bytes and receiving compilation context.
  TEST(ASTSerializationTest, IndependentOwnership)
  {
    ASTDeserializeResult Loaded;
    std::string Expected;
    {
      std::string Bytes;
      {
        core::CompilationContext Producer;
        core::FrontendContext Frontend(Producer);
        const auto Id = Producer.sourceManager().addSource("directory/源文件.ink", "export \"vendor.custom\" func retained[T: type](x: T): T { return x; }");
        const auto Parsed = parse(Frontend, tokenizer::tokenizeSource(Frontend, Id));
        ASSERT_TRUE(Parsed.succeeded());
        Expected = dumpAST(*Parsed.Unit);
        const auto Saved = serializeAST(Frontend, Parsed);
        ASSERT_TRUE(Saved.succeeded());
        Bytes = Saved.Bytes;
      }
      core::CompilationContext Receiver;
      core::FrontendContext Frontend(Receiver);
      Loaded = deserializeAST(Frontend, Bytes);
      ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
      std::fill(Bytes.begin(), Bytes.end(), '\xFF');
    }
    ASSERT_TRUE(Loaded.Parsed.succeeded());
    EXPECT_EQ(dumpAST(*Loaded.Parsed.Unit), Expected);
    EXPECT_EQ(Loaded.Parsed.Unit->input().lexedFile().sourceName(), "directory/源文件.ink");
    const auto *Function = cast<FunctionDecl>(cast<DeclStmt>(Loaded.Parsed.Unit->root()->statements()[0])->declaration());
    EXPECT_EQ(Function->name().Text, "retained");
    EXPECT_EQ(Function->genericParameters()[0].name().Text, "T");
    EXPECT_EQ(Loaded.Parsed.Unit->input().spelling(Function->name().Id), "retained");
    ASSERT_TRUE(isa<LiteralExpr>(Function->linkage()));
    const auto *Linkage = cast<LiteralExpr>(Function->linkage());
    EXPECT_EQ(std::get<tokenizer::StringInfo>(Loaded.Parsed.Unit->input().token(Linkage->token()).Payload).Decoded, "vendor.custom");
    core::CompilationContext Verification;
    core::FrontendContext Frontend(Verification);
    EXPECT_TRUE(serializeAST(Frontend, Loaded.Parsed).succeeded());
  }

  // Iterative node numbering and reconstruction handle long left-associative trees without recursive AST decoding.
  TEST_F(ParserTest, ASTSerializationDeepTree)
  {
    std::string Source = "x";
    for (unsigned Index = 0; Index < 10000; ++Index)
    {
      Source += "+x";
    }
    Source += ';';
    const auto Parsed = parse(Frontend, tokenizer::tokenize(Frontend, Source));
    ASSERT_TRUE(Parsed.succeeded());
    const auto Saved = serializeAST(Frontend, Parsed);
    ASSERT_TRUE(Saved.succeeded()) << Saved.Message;
    auto Loaded = deserializeAST(Frontend, Saved.Bytes);
    ASSERT_TRUE(Loaded.succeeded()) << Loaded.Message;
    EXPECT_TRUE(verifyAST(Loaded.Parsed.Unit->root(), Source.size()));
    std::size_t Count = 0;
    ASTWalker{}.walk(Loaded.Parsed.Unit->root(), [&](const ASTNodeBase *)
                     {
                       ++Count;
                       return WalkAction::Continue;
                     });
    EXPECT_EQ(Count, 20004U);
    EXPECT_EQ(serializeAST(Frontend, Loaded.Parsed).Bytes, Saved.Bytes);
  }

  // Separate parses and general LLVM bitstream decoding agree on deterministic archives, including empty modules.
  TEST_F(ParserTest, ASTSerializationDeterministicContainer)
  {
    for (const char *Source : {"", "f(a, b = 2);", "class A { field b: T; };"})
    {
      const auto First = serializeAST(Frontend, read(Source));
      const auto Second = serializeAST(Frontend, read(Source));
      ASSERT_TRUE(First.succeeded()) << First.Message;
      ASSERT_TRUE(Second.succeeded());
      EXPECT_EQ(First.Bytes, Second.Bytes);
      EXPECT_EQ(archive(records(First.Bytes)), First.Bytes);
      expectRoundTrip(read(Source), Frontend);
    }
  }

  // Every byte truncation, bad magic, unsupported version and trailing bytes is rejected without registering a source.
  TEST_F(ParserTest, ASTSerializationFramingFailures)
  {
    const auto Saved = serializeAST(Frontend, read("f(a, b = 2);"));
    ASSERT_TRUE(Saved.succeeded());
    for (std::size_t Length = 0; Length < Saved.Bytes.size(); ++Length)
    {
      SCOPED_TRACE(Length);
      expectRejected(Frontend, std::string_view(Saved.Bytes).substr(0, Length));
    }
    auto BadMagic = Saved.Bytes;
    BadMagic[0] = 'X';
    expectRejected(Frontend, BadMagic);
    expectRejected(Frontend, Saved.Bytes + std::string(4, '\0'));
    auto Records = records(Saved.Bytes);
    Records[0].Values[0] = ASTArchiveVersion + 1;
    expectRejected(Frontend, archive(Records), ASTArchiveStatus::UnsupportedVersion);
    Records[0].Values[0] = 1;
    expectRejected(Frontend, archive(Records), ASTArchiveStatus::UnsupportedVersion);
    Records[0].Values[0] = 4;
    expectRejected(Frontend, archive(Records), ASTArchiveStatus::UnsupportedVersion);
    Records = records(Saved.Bytes);
    Records[0].Values.push_back(0);
    expectRejected(Frontend, archive(Records));
    Records = records(Saved.Bytes);
    Records[1].Code = 99;
    expectRejected(Frontend, archive(Records));
    Records = records(Saved.Bytes);
    Records.erase(Records.begin() + 1);
    expectRejected(Frontend, archive(Records));
  }

  // Unknown kinds, missing fields, forward/self/shared pointers and category mismatches never construct a published AST.
  TEST_F(ParserTest, ASTSerializationNodeFailures)
  {
    const auto Saved = serializeAST(Frontend, read("a + b;"));
    ASSERT_TRUE(Saved.succeeded());
    const auto Original = records(Saved.Bytes);
    auto Records = Original;
    nodeRecord(Records, ASTKind::BinaryExpr).Values[0] = 255;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::BinaryExpr).Values.pop_back();
    expectRejected(Frontend, archive(Records));
    for (std::uint64_t Reference : {0ULL, 3ULL, 999ULL, 1ULL})
    {
      Records = Original;
      auto &Values = nodeRecord(Records, ASTKind::BinaryExpr).Values;
      Values[Values.size() - 2] = Reference;
      expectRejected(Frontend, archive(Records));
    }
    Records = Original;
    auto &Name = nodeRecord(Records, ASTKind::NameExpr);
    Name.Values = {static_cast<unsigned>(ASTKind::BreakStmt), Name.Values[1], Name.Values[2], 0};
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::ModuleAST).Values[3] = 0;
    nodeRecord(Records, ASTKind::ModuleAST).Values.pop_back();
    expectRejected(Frontend, archive(Records));
  }

  // Invalid source offsets, name/token links, boolean tags, array lengths and wire enums are rejected before publication.
  TEST_F(ParserTest, ASTSerializationFieldFailures)
  {
    const auto Parsed = read("f(a, key = 2);");
    ASSERT_TRUE(Parsed.succeeded());
    const auto Saved = serializeAST(Frontend, Parsed);
    ASSERT_TRUE(Saved.succeeded());
    const auto Original = records(Saved.Bytes);
    auto Records = Original;
    nodeRecord(Records, ASTKind::NameExpr).Values[2] = 10000;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::NameExpr).Values[1] = 0;
    nodeRecord(Records, ASTKind::NameExpr).Values[2] = 0;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::NameExpr).Values[3] = 999;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::NameExpr).Values[5] = 'z';
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::LiteralExpr).Values[3] = 999;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::LiteralExpr).Values[4] = 999;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    auto &CallValues = nodeRecord(Records, ASTKind::CallExpr).Values;
    CallValues[CallValues.size() - 2] = 2;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::CallExpr).Values[4] = 100;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    nodeRecord(Records, ASTKind::CallExpr).Values[5] = 99;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[0].Values[1] = 99;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[2].Values[0] = 256;
    expectRejected(Frontend, archive(Records));
  }

  // Token payload tags, token order, EOF contracts and invalid Unicode scalars cannot bypass lexical snapshot validation.
  TEST_F(ParserTest, ASTSerializationTokenFailures)
  {
    const auto Saved = serializeAST(Frontend, read("'x';"));
    ASSERT_TRUE(Saved.succeeded());
    const auto Original = records(Saved.Bytes);
    auto Records = Original;
    ASSERT_EQ(Records[3].Code, 4U);
    Records[3].Values[3] = 99;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[3].Values[4] = 0xD800;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[3].Values[4] = 0x110000;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[3].Values[0] = 84;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[0].Values[3] = 0;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[4].Values[1] = 1;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[5].Values[2] -= 1;
    expectRejected(Frontend, archive(Records));
  }

  // Successful lexical snapshots reject invalid source encoding and invalid decoded strings while allowing decoded NUL.
  TEST_F(ParserTest, ASTSerializationEncodingFailures)
  {
    const auto Saved = serializeAST(Frontend, read("\"text\"; // gap"));
    ASSERT_TRUE(Saved.succeeded());
    const auto Original = records(Saved.Bytes);
    auto Records = Original;
    Records[2].Values.back() = 0xFF;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[2].Values.back() = 0;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    ASSERT_EQ(Records[3].Code, 4U);
    Records[3].Values[4] = 99;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[3].Values.back() = 0xFF;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records[3].Values[3] = 0;
    Records[3].Values.resize(4);
    expectRejected(Frontend, archive(Records));
    const auto Numeric = serializeAST(Frontend, read("42;"));
    ASSERT_TRUE(Numeric.succeeded());
    Records = records(Numeric.Bytes);
    Records[3].Values[4] = 3;
    expectRejected(Frontend, archive(Records));
  }

  // Overlong VBR integers and undeclared abbreviation IDs fail without shifts past 64 bits or allocation from forged counts.
  TEST_F(ParserTest, ASTSerializationMalformedBitstream)
  {
    for (unsigned Mode = 0; Mode < 3; ++Mode)
    {
      llvm::SmallVector<char, 0> Bytes;
      llvm::BitstreamWriter Writer(Bytes);
      for (unsigned char Byte : std::string_view("IAST"))
      {
        Writer.Emit(Byte, 8);
      }
      Writer.EnterSubblock(8, 3);
      if (Mode == 2)
      {
        Writer.Emit(7, 3);
      }
      else
      {
        Writer.Emit(llvm::bitc::UNABBREV_RECORD, 3);
        Writer.EmitVBR(1, 6);
        if (Mode == 1)
        {
          Writer.EmitVBR(1, 6);
        }
        for (unsigned Index = 0; Index < 20; ++Index)
        {
          Writer.Emit(63, 6);
        }
      }
      Writer.ExitBlock();
      expectRejected(Frontend, std::string_view(Bytes.data(), Bytes.size()));
    }
  }

  // Recovery records require an existing node, matching token references and valid insertion/recovery states.
  TEST_F(ParserTest, ASTSerializationRecoveryFailures)
  {
    const auto Saved = serializeAST(Frontend, read("var x = 1"));
    ASSERT_TRUE(Saved.succeeded());
    const auto Original = records(Saved.Bytes);
    ASSERT_EQ(Original.back().Code, 6U);
    auto Records = Original;
    Records.back().Values[0] = 999;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records.back().Values[1] = 999;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records.back().Values[5] = 0;
    expectRejected(Frontend, archive(Records));
    Records = Original;
    Records.back().Values[4] = 999;
    expectRejected(Frontend, archive(Records));
  }

  // Each public budget rejects oversized input before publishing a unit; exact byte/source/count limits remain usable.
  TEST_F(ParserTest, ASTSerializationResourceLimits)
  {
    const auto Parsed = read("a + b;");
    const auto Saved = serializeAST(Frontend, Parsed);
    ASSERT_TRUE(Saved.succeeded());
    ASTArchiveLimits Limits;
    Limits.MaxArchiveBytes = Saved.Bytes.size() - 1;
    expectRejected(Frontend, Saved.Bytes, ASTArchiveStatus::LimitExceeded, Limits);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveSizeLimitExceeded, Limits);
    Limits = {};
    Limits.MaxSourceBytes = 2;
    expectRejected(Frontend, Saved.Bytes, ASTArchiveStatus::LimitExceeded, Limits);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveSourceLimitExceeded, Limits);
    Limits = {};
    Limits.MaxNodes = 1;
    expectRejected(Frontend, Saved.Bytes, ASTArchiveStatus::LimitExceeded, Limits);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveNodeLimitExceeded, Limits);
    Limits = {};
    Limits.MaxTokens = 1;
    expectRejected(Frontend, Saved.Bytes, ASTArchiveStatus::LimitExceeded, Limits);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveTokenLimitExceeded, Limits);
    Limits = {};
    Limits.MaxArrayElements = 0;
    expectRejected(Frontend, Saved.Bytes, ASTArchiveStatus::LimitExceeded, Limits);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveArrayLimitExceeded, Limits);
    Limits = {};
    Limits.MaxAllocationBytes = 1;
    expectRejected(Frontend, Saved.Bytes, ASTArchiveStatus::LimitExceeded, Limits);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveRecordStorageLimitExceeded, Limits);
    Limits = {};
    Limits.MaxArchiveBytes = Saved.Bytes.size();
    Limits.MaxSourceBytes = 6;
    Limits.MaxNodes = 6;
    Limits.MaxTokens = 5;
    ASSERT_TRUE(deserializeAST(Frontend, Saved.Bytes, Limits).succeeded());
    auto Records = records(Saved.Bytes);
    Records[0].Values[5] = std::numeric_limits<std::uint64_t>::max();
    expectRejected(Frontend, archive(Records), ASTArchiveStatus::LimitExceeded);
  }

  // Each random byte mutation runs in a subprocess and either prints an ICE or completes a verified round trip.
  TEST_F(ParserTest, ASTSerializationMutationCorpus)
  {
    const auto Saved = serializeAST(Frontend, read("func f[T: type](x: T = 1): T { return x + 2; }"));
    ASSERT_TRUE(Saved.succeeded());
    ASTArchiveLimits Limits;
    Limits.MaxSourceBytes = 4096;
    Limits.MaxNodes = 1024;
    Limits.MaxTokens = 1024;
    Limits.MaxArrayElements = 1024;
    Limits.MaxAllocationBytes = 1024 * 1024;
    std::uint32_t State = 0xA57A1234;
    for (unsigned Iteration = 0; Iteration < 2000; ++Iteration)
    {
      State = State * 1664525U + 1013904223U;
      auto Bytes = Saved.Bytes;
      const auto Offset = State % Bytes.size();
      State = State * 1664525U + 1013904223U;
      Bytes[Offset] ^= static_cast<char>((State >> 24) | 1);
      SCOPED_TRACE(Iteration);
      const auto CheckMutation = [&]()
      {
        core::CompilationContext LocalCompilation;
        core::FrontendContext LocalFrontend(LocalCompilation);
        core::CollectingDiagnosticConsumer MutationDiagnostics;
        LocalFrontend.diagnosticEngine().addConsumer(MutationDiagnostics);
        auto Loaded = deserializeAST(LocalFrontend, Bytes, Limits);
        ASSERT_TRUE(Loaded.succeeded());
        ASSERT_TRUE(verifyAST(Loaded.Parsed.Unit->root(), Loaded.Parsed.Unit->input().lexedFile().source().size()));
        const auto Again = serializeAST(LocalFrontend, Loaded.Parsed, Limits);
        ASSERT_TRUE(Again.succeeded()) << Iteration << ' ' << Again.Message;
        ASSERT_TRUE(deserializeAST(LocalFrontend, Again.Bytes, Limits).succeeded());
        ASSERT_TRUE(MutationDiagnostics.diagnostics().empty());
        const auto Sink = std::make_shared<spdlog::sinks::stderr_sink_mt>();
        spdlog::logger Logger("archive-mutation-test", Sink);
        Logger.info("archive mutation round trip succeeded");
        Logger.flush();
        std::_Exit(0);
      };
      EXPECT_EXIT(CheckMutation(), completedOrPanicked, ::testing::AnyOf(::testing::HasSubstr("internal compiler error[INK-P"), ::testing::HasSubstr("archive mutation round trip succeeded")));
    }
  }

  // Serializing an absent parse unit prints its ICE before terminating.
  TEST(ASTSerializationTest, MissingInput)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    expectWriteRejected(Frontend, ParseResult{}, core::DiagnosticKind::ASTArchiveMissingInput);
  }

  // Source syntax diagnostics are not replayed or converted to ICE when a recovered snapshot successfully round-trips.
  TEST_F(ParserTest, ASTSerializationSuccessDoesNotReportICE)
  {
    const auto Parsed = read("var value = ;");
    ASSERT_FALSE(Parsed.succeeded());
    ASSERT_FALSE(Diagnostics.diagnostics().empty());
    for (const auto &Diagnostic : Diagnostics.diagnostics())
    {
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
    }
    Diagnostics.clear();
    expectRoundTrip(Parsed, Frontend);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    expectRejected(Frontend, "not an archive");
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    Diagnostics.clear();
    expectRoundTrip(Parsed, Frontend);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Writer early exits and subsequent framing failures preserve the first ICE instead of emitting a cascade.
  TEST_F(ParserTest, ASTSerializationWriterFirstFailure)
  {
    auto Parsed = read("var value = 1");
    ASSERT_FALSE(Parsed.Unit->recoveryInfo().Entries.empty());
    ASTArchiveLimits Limits;
    Limits.MaxArrayElements = 0;
    Limits.MaxArchiveBytes = 0;
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveRecoveryLimitExceeded, Limits);
    Limits = {};
    Limits.MaxNodes = 0;
    Limits.MaxAllocationBytes = 0;
    Limits.MaxArchiveBytes = 0;
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveNodeLimitExceeded, Limits);
    Parsed.Status = static_cast<ParseStatus>(99);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveInvalidEnum);
  }

  // Invalid ASTs and token metadata terminate in child processes; repaired parent state still serializes successfully.
  TEST_F(ParserTest, ASTSerializationWriterInvalidAST)
  {
    auto Parsed = read("x;");
    auto *Statement = cast<SimpleStmt>(Parsed.Unit->root()->statements()[0]);
    // The arena stores mutable pointer objects; bypass the const view only to inject invalid compiler state.
    auto *Items = const_cast<SimpleItem **>(Statement->items().data());
    auto *Item = Items[0];
    Items[0] = nullptr;
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveInvalidTree);
    const auto Range = SourceRange::fromByteOffsets(0, 1);
    auto &Arena = Parsed.Unit->context();
    auto *Literal = Arena.make<LiteralExpr>(Range, 999, TokenKind::IntegerLiteral);
    Items[0] = Arena.make<ExprItem>(Range, Literal);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveInvalidNodeToken);
    auto *Name = Arena.make<NameExpr>(Range, NameToken{0, "wrong", Range});
    Items[0] = Arena.make<ExprItem>(Range, Name);
    expectWriteRejected(Frontend, Parsed, core::DiagnosticKind::ASTArchiveInvalidNameToken);
    Items[0] = Item;
    Diagnostics.clear();
    expectRoundTrip(Parsed, Frontend);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // Real archive failures print the actual version and budget values before aborting.
  TEST_F(ParserTest, ASTSerializationICEDiagnosticArguments)
  {
    const auto Saved = serializeAST(Frontend, read("var value = 1;"));
    ASSERT_TRUE(Saved.succeeded());
    ASTArchiveLimits Limits;
    Limits.MaxArchiveBytes = Saved.Bytes.size() - 1;
    const std::string SizeMessage = "AST archive size " + std::to_string(Saved.Bytes.size()) + " bytes exceeds limit " + std::to_string(Limits.MaxArchiveBytes) + " bytes";
    EXPECT_DEATH(deserializeAST(Frontend, Saved.Bytes, Limits), SizeMessage);
    auto Records = records(Saved.Bytes);
    Records[0].Values[0] = ASTArchiveVersion + 7;
    const std::string VersionMessage = "unsupported AST archive version " + std::to_string(ASTArchiveVersion + 7) + "; supported version is " + std::to_string(ASTArchiveVersion);
    EXPECT_DEATH(deserializeAST(Frontend, archive(Records)), VersionMessage);
    Records = records(Saved.Bytes);
    Records[0].Values[5] = std::numeric_limits<std::uint64_t>::max();
    EXPECT_DEATH(deserializeAST(Frontend, archive(Records)), std::to_string(std::numeric_limits<std::uint64_t>::max()));
  }
} // namespace ink::parser::test
