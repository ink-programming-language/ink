#include "tokenizer_test_support.h"

#include "utf8_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::SourceRange;

    struct KeywordCase
    {
        const char *Spelling;
        KeywordKind Value;
    };

    struct BuiltinTypeCase
    {
        const char *Spelling;
        BuiltinTypeKind Value;
    };

    // Verifies that every reserved keyword is recognized only by its complete spelling.
    TEST(KeywordsAndBuiltinTypesTest, ClassifiesEveryHardKeywordByItsCompleteSpelling)
    {
      const std::vector<KeywordCase> Cases = {
          {"as", KeywordKind::As},
          {"async", KeywordKind::Async},
          {"await", KeywordKind::Await},
          {"break", KeywordKind::Break},
          {"class", KeywordKind::Class},
          {"comptime", KeywordKind::Comptime},
          {"const", KeywordKind::Const},
          {"continue", KeywordKind::Continue},
          {"decorator", KeywordKind::Decorator},
          {"defer", KeywordKind::Defer},
          {"else", KeywordKind::Else},
          {"enum", KeywordKind::Enum},
          {"extern", KeywordKind::Extern},
          {"final", KeywordKind::Final},
          {"for", KeywordKind::For},
          {"from", KeywordKind::From},
          {"func", KeywordKind::Func},
          {"if", KeywordKind::If},
          {"implicit", KeywordKind::Implicit},
          {"import", KeywordKind::Import},
          {"in", KeywordKind::In},
          {"interface", KeywordKind::Interface},
          {"let", KeywordKind::Let},
          {"override", KeywordKind::Override},
          {"private", KeywordKind::Private},
          {"protected", KeywordKind::Protected},
          {"public", KeywordKind::Public},
          {"return", KeywordKind::Return},
          {"static", KeywordKind::Static},
          {"this", KeywordKind::This},
          {"var", KeywordKind::Var},
          {"virtual", KeywordKind::Virtual},
          {"while", KeywordKind::While},
      };

      for (const KeywordCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::Keyword);
        EXPECT_EQ(Token.Span, (SourceRange{0, std::string(Test.Spelling).size()}));
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<KeywordKind>(Token.Payload));
        EXPECT_EQ(std::get<KeywordKind>(Token.Payload), Test.Value);
        EXPECT_EQ(Result.tokens().back().Kind, TokenKind::EndOfFile);
      }
    }

    // Verifies classification and payloads for the true, false, and null literal spellings.
    TEST(KeywordsAndBuiltinTypesTest, ClassifiesBooleanAndNullLiteralSpellings)
    {
      struct LiteralCase
      {
          const char *Spelling;
          TokenKind Kind;
          bool BooleanValue;
      };

      const std::vector<LiteralCase> Cases = {
          {"true", TokenKind::BoolLiteral, true},
          {"false", TokenKind::BoolLiteral, false},
          {"null", TokenKind::NullLiteral, false},
      };

      for (const LiteralCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, Test.Kind);
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        if (Test.Kind == TokenKind::BoolLiteral)
        {
          ASSERT_TRUE(std::holds_alternative<bool>(Token.Payload));
          EXPECT_EQ(std::get<bool>(Token.Payload), Test.BooleanValue);
        }
        else
        {
          EXPECT_TRUE(std::holds_alternative<std::monostate>(Token.Payload));
        }
      }
    }

    // Verifies that every core built-in type spelling maps to its corresponding type kind.
    TEST(KeywordsAndBuiltinTypesTest, ClassifiesEveryCoreBuiltinType)
    {
      const std::vector<BuiltinTypeCase> Cases = {
          {"i8", BuiltinTypeKind::I8},
          {"i16", BuiltinTypeKind::I16},
          {"i32", BuiltinTypeKind::I32},
          {"i64", BuiltinTypeKind::I64},
          {"i128", BuiltinTypeKind::I128},
          {"u8", BuiltinTypeKind::U8},
          {"u16", BuiltinTypeKind::U16},
          {"u32", BuiltinTypeKind::U32},
          {"u64", BuiltinTypeKind::U64},
          {"u128", BuiltinTypeKind::U128},
          {"int", BuiltinTypeKind::Int},
          {"uint", BuiltinTypeKind::UInt},
          {"ptrsize", BuiltinTypeKind::PtrSize},
          {"f16", BuiltinTypeKind::F16},
          {"f32", BuiltinTypeKind::F32},
          {"f64", BuiltinTypeKind::F64},
          {"bool", BuiltinTypeKind::Bool},
          {"byte", BuiltinTypeKind::Byte},
          {"void", BuiltinTypeKind::Void},
          {"never", BuiltinTypeKind::Never},
          {"type", BuiltinTypeKind::Type},
      };

      for (const BuiltinTypeCase &Test : Cases)
      {
        SCOPED_TRACE(Test.Spelling);
        const TokenizedBuffer Result = tokenize(Test.Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::BuiltinType);
        EXPECT_EQ(Result.raw(Token), Test.Spelling);
        ASSERT_TRUE(std::holds_alternative<BuiltinTypeKind>(Token.Payload));
        EXPECT_EQ(std::get<BuiltinTypeKind>(Token.Payload), Test.Value);
      }
    }

    // Verifies that removed language spellings, case variants, near misses, and function-style built-ins remain identifiers.
    TEST(KeywordsAndBuiltinTypesTest, KeepsRemovedSpellingsCaseVariantsNearMissesAndFunctionStyleBuiltinsAsIdentifiers)
    {
      const std::vector<std::string> Spellings = {
          "catch",
          "match",
          "throw",
          "try",
          "Func",
          "TRUE",
          "Null",
          "functional",
          "i32value",
          "nullable",
          "constructor",
          "constructorValue",
          "destructor",
          "cast",
          "bitcast",
          "ptrcast",
          "try_cast",
          "reflect",
          "function",
          "operator",
          "String",
          "UnicodeScalar",
          "f128",
          "u256",
          utf8(u8"func\u7528\u6237"),
          utf8(u8"i32\u53D8\u91CF"),
      };

      for (const std::string &Spelling : Spellings)
      {
        SCOPED_TRACE(Spelling);
        const TokenizedBuffer Result = tokenize(Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        const Token &Token = Result.tokens().front();
        EXPECT_EQ(Token.Kind, TokenKind::Identifier);
        EXPECT_EQ(Token.Span, (SourceRange{0, Spelling.size()}));
        EXPECT_EQ(Result.raw(Token), Spelling);
        EXPECT_TRUE(std::holds_alternative<std::monostate>(Token.Payload));
      }
    }

    // Verifies that scanning the complete Unicode identifier precedes keyword, Boolean, null, and built-in classification.
    TEST(KeywordsAndBuiltinTypesTest, UnicodeContinuationPreventsReservedSpellingClassification)
    {
      const std::vector<std::string> Spellings = {
          utf8(u8"func\u7528\u6237"),
          utf8(u8"true\u503C"),
          utf8(u8"false\u503C"),
          utf8(u8"null\u503C"),
          utf8(u8"i32\u53D8\u91CF"),
      };

      for (const std::string &Spelling : Spellings)
      {
        SCOPED_TRACE(Spelling);
        const TokenizedBuffer Result = tokenize(Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Result.tokens()[0].Span, (SourceRange{0, Spelling.size()}));
        EXPECT_EQ(Result.raw(Result.tokens()[0]), Spelling);
        EXPECT_TRUE(std::holds_alternative<std::monostate>(Result.tokens()[0].Payload));
        EXPECT_TRUE(testDiagnostics(Result).empty());
        EXPECT_EQ(Result.tokens()[1].Kind, TokenKind::EndOfFile);
      }
    }

    // Verifies that NFC-stable spellings with only compatibility equivalents remain identifiers rather than reserved ASCII spellings.
    TEST(KeywordsAndBuiltinTypesTest, CompatibilityEquivalentUnicodeSpellingsRemainIdentifiers)
    {
      const std::vector<std::string> Spellings = {
          utf8(u8"\uFF46\uFF55\uFF4E\uFF43"),
          utf8(u8"\uFF54\uFF52\uFF55\uFF45"),
          utf8(u8"\uFF46\uFF41\uFF4C\uFF53\uFF45"),
          utf8(u8"\uFF4E\uFF55\uFF4C\uFF4C"),
          utf8(u8"\uFF49\uFF13\uFF12"),
      };

      for (const std::string &Spelling : Spellings)
      {
        SCOPED_TRACE(Spelling);
        const TokenizedBuffer Result = tokenize(Spelling);
        ASSERT_TRUE(Result.succeeded());
        ASSERT_EQ(Result.tokens().size(), 2U);
        EXPECT_EQ(Result.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_EQ(Result.tokens()[0].Span, (SourceRange{0, Spelling.size()}));
        EXPECT_EQ(Result.raw(Result.tokens()[0]), Spelling);
        EXPECT_TRUE(std::holds_alternative<std::monostate>(Result.tokens()[0].Payload));
        EXPECT_TRUE(testDiagnostics(Result).empty());
        EXPECT_EQ(Result.tokens()[1].Kind, TokenKind::EndOfFile);
      }
    }

    // Verifies that surrounding punctuation and trivia do not alter keyword classification.
    TEST(KeywordsAndBuiltinTypesTest, ClassificationDoesNotDependOnSurroundingSyntax)
    {
      const TokenizedBuffer Result = tokenize("func func: func");
      ASSERT_TRUE(Result.succeeded());
      ASSERT_EQ(Result.tokens().size(), 7U);
      EXPECT_EQ(Result.tokens()[0].Kind, TokenKind::Keyword);
      EXPECT_EQ(std::get<KeywordKind>(Result.tokens()[0].Payload), KeywordKind::Func);
      EXPECT_EQ(Result.tokens()[1].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Result.tokens()[2].Kind, TokenKind::Keyword);
      EXPECT_EQ(std::get<KeywordKind>(Result.tokens()[2].Payload), KeywordKind::Func);
      EXPECT_EQ(Result.tokens()[3].Kind, TokenKind::Symbol);
      EXPECT_EQ(std::get<char>(Result.tokens()[3].Payload), ':');
      EXPECT_EQ(Result.tokens()[4].Kind, TokenKind::SpacesAndTabs);
      EXPECT_EQ(Result.tokens()[5].Kind, TokenKind::Keyword);
      EXPECT_EQ(Result.tokens()[6].Kind, TokenKind::EndOfFile);
    }
  } // namespace
} // namespace ink::tokenizer
