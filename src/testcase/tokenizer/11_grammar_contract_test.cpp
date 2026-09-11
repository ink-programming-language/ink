#include "grammar_test_support.h"

#include <fstream>
#include <iterator>
#include <set>

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;

    std::string readGrammar(const char *Name)
    {
      std::ifstream Stream(std::string(INK_GRAMMAR_SOURCE_DIRECTORY) + "/" + Name, std::ios::binary);
      return {std::istreambuf_iterator<char>(Stream), std::istreambuf_iterator<char>()};
    }

    std::string withoutComments(std::string_view Source)
    {
      std::string Result;
      std::size_t Position = 0;
      while (Position < Source.size())
      {
        if (Source.substr(Position, 2) == "(*")
        {
          const std::size_t End = Source.find("*)", Position + 2);
          if (End == std::string_view::npos)
          {
            return {};
          }
          Position = End + 2;
          Result += ' ';
        }
        else
        {
          Result += Source[Position++];
        }
      }
      return Result;
    }

    std::set<std::string> quotedTerminals(std::string_view Source)
    {
      std::set<std::string> Result;
      for (std::size_t Position = 0; Position < Source.size(); ++Position)
      {
        const char Quote = Source[Position];
        if (Quote != '"' && Quote != '\'')
        {
          continue;
        }
        const std::size_t End = Source.find(Quote, Position + 1);
        if (End == std::string_view::npos)
        {
          return {};
        }
        Result.emplace(Source.substr(Position + 1, End - Position - 1));
        Position = End;
      }
      return Result;
    }

    std::string_view production(std::string_view Source, std::string_view Name)
    {
      const std::string Marker = "\n" + std::string(Name) + " =";
      const std::size_t Start = Source.find(Marker);
      if (Start == std::string_view::npos)
      {
        return {};
      }
      char Quote = 0;
      for (std::size_t Position = Start + Marker.size(); Position < Source.size(); ++Position)
      {
        const char Current = Source[Position];
        if (Quote != 0)
        {
          if (Current == Quote)
          {
            Quote = 0;
          }
        }
        else if (Current == '"' || Current == '\'')
        {
          Quote = Current;
        }
        else if (Current == ';')
        {
          return Source.substr(Start + Marker.size(), Position - Start - Marker.size());
        }
      }
      return {};
    }

    // Verifies the keyword registry exactly matches the authoritative lexer production, detecting missing and extra reserved words.
    TEST(GrammarLexicalContractTest, KeywordRegistryExactlyMatchesLexerBnf)
    {
      const std::string Lexer = withoutComments(readGrammar("lexer.bnf"));
      ASSERT_FALSE(Lexer.empty());
      const std::set<std::string> Expected = quotedTerminals(production(Lexer, "keyword"));
      ASSERT_FALSE(Expected.empty());
      const std::set<std::string> Actual = {
#define INK_KEYWORD(Name, Spelling) Spelling,
#include "ink/tokenizer/token.def"
#undef INK_KEYWORD
      };
      EXPECT_EQ(Actual, Expected);
      for (const std::string &Spelling : Expected)
      {
        const TokenizedBuffer File = tokenize(Spelling);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Spelling});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Keyword);
      }
    }

    // Verifies the symbol registry exactly matches lexer.bnf, including the separately guarded slash production.
    TEST(GrammarLexicalContractTest, SymbolRegistryExactlyMatchesLexerBnf)
    {
      const std::string Lexer = withoutComments(readGrammar("lexer.bnf"));
      ASSERT_FALSE(Lexer.empty());
      std::set<std::string> Expected = quotedTerminals(production(Lexer, "symbol"));
      ASSERT_FALSE(Expected.empty());
      Expected.insert("/");
      const std::set<std::string> Actual = {
#define INK_SYMBOL(Name, Spelling) Spelling,
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
      };
      EXPECT_EQ(Actual, Expected);
      for (const std::string &Spelling : Expected)
      {
        const TokenizedBuffer File = tokenize(Spelling);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Spelling});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Symbol);
      }
    }

    // Verifies every fixed terminal in grammar.bnf is expressible as exactly one keyword or symbol token.
    TEST(GrammarLexicalContractTest, EveryGrammarTerminalHasOneLexicalToken)
    {
      const std::string Grammar = withoutComments(readGrammar("grammar.bnf"));
      ASSERT_FALSE(Grammar.empty());
      const std::set<std::string> Terminals = quotedTerminals(Grammar);
      ASSERT_FALSE(Terminals.empty());
      for (const std::string &Spelling : Terminals)
      {
        SCOPED_TRACE(Spelling);
        EXPECT_TRUE(lookupKeyword(Spelling).has_value() || lookupSymbol(Spelling).has_value());
        const TokenizedBuffer File = tokenize(Spelling);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Spelling});
      }
    }

    // Verifies the unified call/construction form, generic suffixes and comptime statements produce the intended atomic operators.
    TEST(GrammarLexicalContractTest, RepresentativeNewGrammarSourceTokenizes)
    {
      const std::string Source = "class C { class D { comptime if (Enabled) { class_field Value: int32; } } }\nlet x: makeType() = Factory::[int32](1, 2);\nclass_method run[T:type](v:T)->T { return v; }";
      const TokenizedBuffer File = tokenize(Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(testDiagnostics(File).empty());
      std::size_t GenericCount = 0;
      std::size_t ArrowCount = 0;
      for (const Token &Entry : File.tokens())
      {
        GenericCount += File.raw(Entry) == "::";
        ArrowCount += File.raw(Entry) == "->";
      }
      EXPECT_EQ(GenericCount, 1U);
      EXPECT_EQ(ArrowCount, 1U);
      expectStream(File);
    }
  } // namespace
} // namespace ink::tokenizer
