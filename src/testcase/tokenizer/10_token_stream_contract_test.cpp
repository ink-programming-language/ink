#include "grammar_test_support.h"

#include <limits>
#include <random>

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;
    using core::DiagnosticKind;

    // Verifies enum predicates distinguish token categories while accepting mixed keyword, symbol, and token-kind sets.
    TEST(TokenStreamContractTest, EnumPredicatesMatchTypedTokens)
    {
      const TokenizedBuffer File = tokenize("if + If 42");
      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(File.tokens().size(), 5u);
      const Token &Keyword = File.tokens()[0];
      EXPECT_TRUE(Keyword.is(TokenKind::Keyword));
      EXPECT_TRUE(Keyword.is(KeywordKind::If));
      EXPECT_EQ(Keyword.keyword(), KeywordKind::If);
      EXPECT_FALSE(Keyword.is(KeywordKind::While));
      EXPECT_FALSE(Keyword.is(SymbolKind::Plus));
      EXPECT_TRUE(Keyword.isOneOf(SymbolKind::Plus, KeywordKind::If, TokenKind::Identifier));
      const Token &Symbol = File.tokens()[1];
      EXPECT_TRUE(Symbol.is(SymbolKind::Plus));
      EXPECT_EQ(Symbol.symbol(), SymbolKind::Plus);
      EXPECT_FALSE(Symbol.is(KeywordKind::If));
      EXPECT_FALSE(Symbol.is(SymbolKind::PlusAssign));
      EXPECT_TRUE(Symbol.isOneOf(TokenKind::Identifier, SymbolKind::Plus));
      EXPECT_TRUE(File.tokens()[2].is(TokenKind::Identifier));
      EXPECT_FALSE(File.tokens()[2].is(KeywordKind::If));
      EXPECT_TRUE(File.tokens()[3].is(TokenKind::IntegerLiteral));
      EXPECT_TRUE(File.tokens()[4].is(TokenKind::EndOfFile));
    }

    // Verifies typed predicates safely reject absent or mismatched payloads rather than interpreting unrelated enum values.
    TEST(TokenStreamContractTest, EnumPredicatesRejectMismatchedPayloads)
    {
      const Token MissingKeyword{TokenKind::Keyword, {}, {}};
      const Token WrongKeyword{TokenKind::Keyword, {}, SymbolKind::Plus};
      const Token WrongSymbol{TokenKind::Symbol, {}, KeywordKind::If};
      EXPECT_FALSE(MissingKeyword.is(KeywordKind::If));
      EXPECT_FALSE(WrongKeyword.is(KeywordKind::If));
      EXPECT_FALSE(WrongKeyword.is(SymbolKind::Plus));
      EXPECT_FALSE(WrongSymbol.is(SymbolKind::Plus));
      EXPECT_FALSE(WrongSymbol.is(KeywordKind::If));
      EXPECT_FALSE(WrongSymbol.isOneOf(SymbolKind::Plus, KeywordKind::If));
    }

    // Verifies successful token streams retain original source text and byte-based raw slices.
    TEST(TokenStreamContractTest, SourceTextAndRawSlicesRemainStable)
    {
      const std::string Source = "let " + utf8(u8"变量") + ": int32 = 1; /* text */";
      const TokenizedBuffer File = tokenize(Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.source(), Source);
      expectRaws(File, {"let", utf8(u8"变量"), ":", "int32", "=", "1", ";"});
    }

    // Verifies trivia filtering preserves syntax tokens, payloads, offsets and diagnostics.
    TEST(TokenStreamContractTest, DefaultAndFullFidelityStreamsHaveIdenticalSyntax)
    {
      const std::string Source = " \tlet a:int32=1; // line\r\n/* c */ @ \"text\"";
      const TokenizedBuffer Filtered = tokenize(Source);
      const TokenizedBuffer Full = tokenize(Source, preservingTrivia());
      std::vector<Token> Expected;
      for (const Token &Entry : Full.tokens())
      {
        if (!Entry.isTrivia())
        {
          Expected.push_back(Entry);
        }
      }
      EXPECT_EQ(Filtered.tokens(), Expected);
      ASSERT_EQ(testDiagnostics(Filtered).size(), testDiagnostics(Full).size());
      for (std::size_t Index = 0; Index < testDiagnostics(Filtered).size(); ++Index)
      {
        EXPECT_EQ(testDiagnostics(Filtered)[Index].Kind, testDiagnostics(Full)[Index].Kind);
        EXPECT_EQ(testDiagnostics(Filtered)[Index].Span, testDiagnostics(Full)[Index].Span);
      }
      expectStream(Filtered);
      expectStream(Full, true);
    }

    // Verifies line lookup clamps offsets and retains trailing empty lines for CR, LF and CRLF.
    TEST(TokenStreamContractTest, LineLookupClampsAndRetainsTrailingEmptyLine)
    {
      const TokenizedBuffer Empty = tokenize("");
      const TokenizedBuffer File = tokenize("a\r\nb\nc\r");
      EXPECT_EQ(Empty.lineNumber(std::numeric_limits<std::size_t>::max()), 1U);
      EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0, 3, 5, 7}));
      EXPECT_EQ(File.lineNumber(1), 1U);
      EXPECT_EQ(File.lineNumber(2), 1U);
      EXPECT_EQ(File.lineNumber(3), 2U);
      EXPECT_EQ(File.lineNumber(7), 4U);
      EXPECT_EQ(File.lineNumber(std::numeric_limits<std::size_t>::max()), 4U);
      expectStream(File);
    }

    // Verifies line maps include line endings in discarded comments and opaque string tokens.
    TEST(TokenStreamContractTest, LineLookupIncludesOpaqueTokenContent)
    {
      const TokenizedBuffer File = tokenize("/*a\r\nb*/\r\"\"\"c\nd\"\"\"");
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.lineStarts(), (std::vector<std::size_t>{0, 5, 9, 14}));
      EXPECT_EQ(File.lineNumber(File.tokens()[0].Span.Start), 3U);
      expectStream(File);
    }

    // Verifies a named SourceManager entry retains its identity and shared source storage.
    TEST(TokenStreamContractTest, TokenizesNamedSourceManagerEntries)
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      const core::SourceId Source = Compilation.sourceManager().addSource("named.ink", "let value: int32 = 1;");
      const TokenizedBuffer File = tokenizeSource(Context, Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.sourceId(), Source);
      EXPECT_EQ(File.sourceName(), "named.ink");
      EXPECT_EQ(File.source(), "let value: int32 = 1;");
      EXPECT_TRUE(File.isRegisteredWith(Compilation.sourceManager()));
      ASSERT_NE(Compilation.sourceManager().findSource(Source), nullptr);
      expectStream(File);
    }

    // Verifies source text outlives the compilation context that originally registered it.
    TEST(TokenStreamContractTest, KeepsSourceAliveBeyondCompilationContext)
    {
      const TokenizedBuffer File = []()
      {
        core::CompilationContext Compilation;
        core::FrontendContext Context(Compilation);
        const core::SourceId Source = Compilation.sourceManager().addSource("temporary.ink", "persistent");
        return tokenizeSource(Context, Source);
      }();
      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.sourceName(), "temporary.ink");
      EXPECT_EQ(File.source(), "persistent");
      EXPECT_EQ(File.raw(File.tokens()[0]), "persistent");
      expectStream(File);
    }

    // Verifies unknown SourceId inputs return a safe unsuccessful buffer and explicit diagnostic.
    TEST(TokenStreamContractTest, RejectsUnknownSourceManagerEntries)
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      core::CollectingDiagnosticConsumer Diagnostics;
      Compilation.diagnosticEngine().addConsumer(Diagnostics);
      const TokenizedBuffer File = tokenizeSource(Context, core::SourceId(1));
      EXPECT_FALSE(File.succeeded());
      EXPECT_FALSE(File.sourceId().valid());
      EXPECT_TRUE(File.source().empty());
      EXPECT_TRUE(File.sourceName().empty());
      EXPECT_TRUE(File.tokens().empty());
      EXPECT_TRUE(File.lineStarts().empty());
      EXPECT_EQ(File.lineNumber(0), 0U);
      ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
      EXPECT_EQ(Diagnostics.diagnostics()[0].Kind, DiagnosticKind::TokenizerSourceNotFound);
    }

    // Verifies equal numeric IDs from different source managers do not share registered identity.
    TEST(TokenStreamContractTest, SourceIdentityBelongsToItsOriginalManager)
    {
      core::CompilationContext First;
      core::CompilationContext Second;
      core::FrontendContext Context(First);
      const core::SourceId Source = First.sourceManager().addSource("first.ink", "a");
      Second.sourceManager().addSource("second.ink", "b");
      const TokenizedBuffer File = tokenizeSource(Context, Source);
      EXPECT_TRUE(File.isRegisteredWith(First.sourceManager()));
      EXPECT_FALSE(File.isRegisteredWith(Second.sourceManager()));
    }

    // Verifies a reusable scanner retains options while a previous failure does not poison later input.
    TEST(TokenStreamContractTest, ScannerReusePreservesOptionsAndResetsFailure)
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      TokenizerOptions Options;
      Options.MaxBlockCommentDepth = 1;
      const Tokenizer Scanner(Context, Options);
      const TokenizedBuffer First = Scanner.tokenize("let");
      const TokenizedBuffer Failed = Scanner.tokenize("/* /* */ */");
      const TokenizedBuffer Last = Scanner.tokenize("let");
      EXPECT_TRUE(First.succeeded());
      EXPECT_FALSE(Failed.succeeded());
      EXPECT_TRUE(Last.succeeded());
      EXPECT_EQ(First.tokens(), Last.tokens());
    }

    // Verifies public token names and orthogonal trivia/error categories cover the complete new token enum.
    TEST(TokenStreamContractTest, TokenKindNamesAndClassificationsAreExhaustive)
    {
      const std::vector<std::pair<TokenKind, const char *>> Cases = {
          {TokenKind::SpacesAndTabs, "SpacesAndTabs"},
          {TokenKind::LineBreak, "LineBreak"},
          {TokenKind::LineComment, "LineComment"},
          {TokenKind::BlockComment, "BlockComment"},
          {TokenKind::Identifier, "Identifier"},
          {TokenKind::Keyword, "Keyword"},
          {TokenKind::IntegerLiteral, "IntegerLiteral"},
          {TokenKind::FloatLiteral, "FloatLiteral"},
          {TokenKind::StringLiteral, "StringLiteral"},
          {TokenKind::Symbol, "Symbol"},
          {TokenKind::InvalidEncoding, "InvalidEncoding"},
          {TokenKind::InvalidCharacter, "InvalidCharacter"},
          {TokenKind::InvalidIdentifier, "InvalidIdentifier"},
          {TokenKind::InvalidStringLiteral, "InvalidStringLiteral"},
          {TokenKind::UnterminatedBlockComment, "UnterminatedBlockComment"},
          {TokenKind::EndOfFile, "EndOfFile"},
      };
      const std::vector<TokenKind> Trivia = {
          TokenKind::SpacesAndTabs,
          TokenKind::LineBreak,
          TokenKind::LineComment,
          TokenKind::BlockComment,
      };
      const std::vector<TokenKind> Errors = {
          TokenKind::InvalidEncoding,
          TokenKind::InvalidCharacter,
          TokenKind::InvalidIdentifier,
          TokenKind::InvalidStringLiteral,
          TokenKind::UnterminatedBlockComment,
      };
      for (const auto &Entry : Cases)
      {
        EXPECT_STREQ(tokenKindName(Entry.first), Entry.second);
        EXPECT_EQ(isTrivia(Entry.first), std::find(Trivia.begin(), Trivia.end(), Entry.first) != Trivia.end());
        EXPECT_EQ(isError(Entry.first), std::find(Errors.begin(), Errors.end(), Entry.first) != Errors.end());
        EXPECT_FALSE(isTrivia(Entry.first) && isError(Entry.first));
      }
      EXPECT_STREQ(tokenKindName(static_cast<TokenKind>(9999)), "Unknown");
    }

    // Verifies each lexical error category marks failure, emits a diagnostic and retains an ordered token stream.
    TEST(TokenStreamContractTest, EveryLexicalErrorKindMakesTheResultFail)
    {
      const std::vector<std::pair<std::string, TokenKind>> Cases = {
          {bytes({0x80}), TokenKind::InvalidEncoding},
          {"@", TokenKind::InvalidCharacter},
          {utf8(u8"cafe\u0301"), TokenKind::InvalidIdentifier},
          {"\"", TokenKind::InvalidStringLiteral},
          {"/*", TokenKind::UnterminatedBlockComment},
      };
      for (const auto &Entry : Cases)
      {
        const TokenizedBuffer File = tokenize(Entry.first);
        EXPECT_FALSE(File.succeeded());
        ASSERT_GE(File.tokens().size(), 2U);
        EXPECT_EQ(File.tokens()[0].Kind, Entry.second);
        EXPECT_FALSE(testDiagnostics(File).empty());
        expectStream(File);
      }
    }

    // Verifies deterministic arbitrary bytes always advance with full byte partitioning and bounded diagnostics.
    TEST(TokenStreamContractTest, ArbitraryBytesAlwaysAdvanceAndPreserveTheExactPartition)
    {
      std::mt19937 Generator(0x1A2B3C4D);
      std::uniform_int_distribution<int> Length(0, 128);
      std::uniform_int_distribution<int> Byte(0, 255);
      for (std::size_t Index = 0; Index < 1000; ++Index)
      {
        SCOPED_TRACE(Index);
        std::string Source(static_cast<std::size_t>(Length(Generator)), '\0');
        for (char &Value : Source)
        {
          Value = static_cast<char>(Byte(Generator));
        }
        const TokenizedBuffer Full = tokenize(Source, preservingTrivia());
        const TokenizedBuffer Filtered = tokenize(Source);
        ASSERT_LE(Full.tokens().size(), Source.size() + 1);
        ASSERT_LE(Filtered.tokens().size(), Full.tokens().size());
        expectStream(Full, true);
        expectStream(Filtered);
      }
    }

    // Verifies repeated tokenization is deterministic for both accepted and erroneous input.
    TEST(TokenStreamContractTest, RepeatedTokenizationIsDeterministic)
    {
      for (const std::string &Source : {"let x:int32=0xFF; // comment\r\n\"a\\n\"", "@ /* unclosed"})
      {
        const TokenizedBuffer First = tokenize(Source, preservingTrivia());
        const TokenizedBuffer Second = tokenize(Source, preservingTrivia());
        EXPECT_EQ(First.succeeded(), Second.succeeded());
        EXPECT_EQ(First.tokens(), Second.tokens());
        ASSERT_EQ(testDiagnostics(First).size(), testDiagnostics(Second).size());
        for (std::size_t Index = 0; Index < testDiagnostics(First).size(); ++Index)
        {
          EXPECT_EQ(testDiagnostics(First)[Index].Kind, testDiagnostics(Second)[Index].Kind);
          EXPECT_EQ(testDiagnostics(First)[Index].Span, testDiagnostics(Second)[Index].Span);
        }
        expectStream(First, true);
        expectStream(Second, true);
      }
    }

    // Verifies syntactically invalid text remains lexically accepted when every token belongs to lexer.bnf.
    TEST(TokenStreamContractTest, LexicalSuccessDoesNotRequireValidGrammar)
    {
      for (const std::string &Source : {")(", "{[(", "let", "1e+", "Point{1,2}", "import \"missing.ink\""})
      {
        const TokenizedBuffer File = tokenize(Source);
        EXPECT_TRUE(File.succeeded());
        EXPECT_TRUE(testDiagnostics(File).empty());
        expectStream(File);
      }
    }
  } // namespace
} // namespace ink::tokenizer
