#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;

    // Verifies prefix matching prefers the longest complete symbol and handles empty or unrelated source suffixes.
    TEST(SymbolTokenTest, PrefixLookupUsesCompleteLongestSpelling)
    {
      EXPECT_EQ(matchSymbolPrefix("<<=rest"), SymbolKind::ShiftLeftAssign);
      EXPECT_EQ(matchSymbolPrefix(">>>="), SymbolKind::ShiftRight);
      EXPECT_EQ(matchSymbolPrefix("...rest"), SymbolKind::Ellipsis);
      EXPECT_EQ(matchSymbolPrefix("..rest"), SymbolKind::Dot);
      EXPECT_EQ(matchSymbolPrefix(":"), SymbolKind::Colon);
      EXPECT_FALSE(matchSymbolPrefix("").has_value());
      EXPECT_FALSE(matchSymbolPrefix("identifier").has_value());
      EXPECT_FALSE(matchSymbolPrefix("@=").has_value());
      EXPECT_FALSE(lookupSymbol("<<=rest").has_value());
    }

    // Verifies every lexical symbol is one complete typed token with a reversible spelling.
    TEST(SymbolTokenTest, EveryGrammarSymbolFormsOneTypedToken)
    {
      const std::vector<std::string> Symbols = {
          "<<=",
          ">>=",
          "...",
          "+=",
          "-=",
          "*=",
          "/=",
          "%=",
          "&=",
          "|=",
          "^=",
          "||",
          "&&",
          "==",
          "!=",
          "<=",
          ">=",
          "<<",
          ">>",
          "::",
          "->",
          "=",
          "?",
          "|",
          "^",
          "&",
          "<",
          ">",
          "+",
          "-",
          "*",
          "/",
          "%",
          "!",
          "~",
          "(",
          ")",
          "{",
          "}",
          "[",
          "]",
          ",",
          ";",
          ":",
          ".",
          "_",
      };
      for (const std::string &Source : Symbols)
      {
        SCOPED_TRACE(Source);
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Source});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Symbol);
        const SymbolKind *Payload = std::get_if<SymbolKind>(&File.tokens()[0].Payload);
        ASSERT_NE(Payload, nullptr);
        EXPECT_EQ(symbolSpelling(*Payload), Source);
        EXPECT_EQ(symbolSpelling(File.tokens()[0]), Source);
        EXPECT_EQ(lookupSymbol(Source), *Payload);
      }
    }

    // Verifies competing operator prefixes always use the longest available spelling.
    TEST(SymbolTokenTest, OperatorsUseMaximalMunch)
    {
      const std::vector<std::pair<std::string, std::vector<std::string>>> Cases = {
          {"<<==", {"<<=", "="}},
          {">>>=", {">>", ">="}},
          {"....", {"...", "."}},
          {"..", {".", "."}},
          {":::[]", {"::", ":", "[", "]"}},
          {"->>", {"->", ">"}},
          {"&&&=", {"&&", "&="}},
          {"|||=", {"||", "|="}},
          {"====", {"==", "=="}},
          {"++--", {"+", "+", "-", "-"}},
          {"=>", {"=", ">"}},
          {"?.", {"?", "."}},
      };
      for (const auto &Entry : Cases)
      {
        const TokenizedBuffer File = tokenize(Entry.first);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, Entry.second);
      }
    }

    // Verifies whitespace breaks compound operators without generating lexical errors.
    TEST(SymbolTokenTest, TriviaPreservesOperatorBoundaries)
    {
      const TokenizedBuffer File = tokenize("< < = > > = : : . . . - >");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"<", "<", "=", ">", ">", "=", ":", ":", ".", ".", ".", "-", ">"});
    }

    // Verifies comment recognition takes precedence over the slash token.
    TEST(SymbolTokenTest, SlashOnlyFormsASymbolOutsideCommentPrefixes)
    {
      const TokenizedBuffer File = tokenize("/ /= //line\n /* block */ */ /");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"/", "/=", "*", "/", "/"});
    }

    // Verifies punctuation not listed by lexer.bnf is rejected individually and scanning resumes.
    TEST(SymbolTokenTest, UnlistedPunctuationIsInvalid)
    {
      const TokenizedBuffer File = tokenize("@#$`\\after");
      ASSERT_FALSE(File.succeeded());
      expectRaws(File, {"@", "#", "$", "`", "\\", "after"});
      for (std::size_t Index = 0; Index < 5; ++Index)
      {
        EXPECT_EQ(File.tokens()[Index].Kind, TokenKind::InvalidCharacter);
      }
    }

    // Verifies generic declarations and explicit applications require no contextual angle-bracket tokenization.
    TEST(SymbolTokenTest, GenericApplicationKeepsDoubleColonAndBrackets)
    {
      const TokenizedBuffer File = tokenize("func F[T:type](x:T)->T; F::[int32](1);");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"func", "F", "[", "T", ":", "type", "]", "(", "x", ":", "T", ")", "->", "T", ";", "F", "::", "[", "int32", "]", "(", "1", ")", ";"});
    }

    // Verifies symbol spelling queries fail safely for nonsymbol tokens and unknown values.
    TEST(SymbolTokenTest, SymbolSpellingRejectsNonsymbolPayloads)
    {
      const TokenizedBuffer File = tokenize("name");
      EXPECT_TRUE(symbolSpelling(File.tokens()[0]).empty());
      EXPECT_TRUE(symbolSpelling(static_cast<SymbolKind>(9999)).empty());
      EXPECT_FALSE(lookupSymbol("@").has_value());
      EXPECT_FALSE(lookupSymbol("").has_value());
    }
  } // namespace
} // namespace ink::tokenizer
