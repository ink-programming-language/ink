#include "ink/tokenizer/tokenizer.h"

#include <gtest/gtest.h>

#include <string>
#include <variant>
#include <vector>

namespace ink::tokenizer {
namespace {

struct KeywordCase {
  const char* spelling;
  KeywordKind value;
};

struct BuiltinTypeCase {
  const char* spelling;
  BuiltinTypeKind value;
};

TEST(KeywordsAndBuiltinTypesTest, ClassifiesEveryHardKeywordByItsCompleteSpelling) {
  const std::vector<KeywordCase> cases = {{"as", KeywordKind::As}, {"async", KeywordKind::Async}, {"await", KeywordKind::Await}, {"break", KeywordKind::Break}, {"catch", KeywordKind::Catch}, {"class", KeywordKind::Class}, {"comptime", KeywordKind::Comptime}, {"const", KeywordKind::Const}, {"continue", KeywordKind::Continue}, {"constructor", KeywordKind::Constructor}, {"decorator", KeywordKind::Decorator}, {"defer", KeywordKind::Defer}, {"destructor", KeywordKind::Destructor}, {"else", KeywordKind::Else}, {"enum", KeywordKind::Enum}, {"extern", KeywordKind::Extern}, {"for", KeywordKind::For}, {"from", KeywordKind::From}, {"func", KeywordKind::Func}, {"if", KeywordKind::If}, {"implicit", KeywordKind::Implicit}, {"import", KeywordKind::Import}, {"in", KeywordKind::In}, {"interface", KeywordKind::Interface}, {"let", KeywordKind::Let}, {"match", KeywordKind::Match}, {"override", KeywordKind::Override}, {"private", KeywordKind::Private}, {"return", KeywordKind::Return}, {"this", KeywordKind::This}, {"throw", KeywordKind::Throw}, {"try", KeywordKind::Try}, {"var", KeywordKind::Var}, {"virtual", KeywordKind::Virtual}, {"while", KeywordKind::While}};

  for (const KeywordCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::Keyword);
    EXPECT_EQ(token.span, (ByteSpan{0, std::string(test.spelling).size()}));
    EXPECT_EQ(result.raw(token), test.spelling);
    ASSERT_TRUE(std::holds_alternative<KeywordKind>(token.payload));
    EXPECT_EQ(std::get<KeywordKind>(token.payload), test.value);
    EXPECT_EQ(result.tokens().back().kind, TokenKind::EndOfFile);
  }
}

TEST(KeywordsAndBuiltinTypesTest, ClassifiesBooleanAndNullLiteralSpellings) {
  struct LiteralCase {
    const char* spelling;
    TokenKind kind;
    bool boolean_value;
  };
  const std::vector<LiteralCase> cases = {{"true", TokenKind::BoolLiteral, true}, {"false", TokenKind::BoolLiteral, false}, {"null", TokenKind::NullLiteral, false}};

  for (const LiteralCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, test.kind);
    EXPECT_EQ(result.raw(token), test.spelling);
    if (test.kind == TokenKind::BoolLiteral) {
      ASSERT_TRUE(std::holds_alternative<bool>(token.payload));
      EXPECT_EQ(std::get<bool>(token.payload), test.boolean_value);
    } else {
      EXPECT_TRUE(std::holds_alternative<std::monostate>(token.payload));
    }
  }
}

TEST(KeywordsAndBuiltinTypesTest, ClassifiesEveryCoreBuiltinType) {
  const std::vector<BuiltinTypeCase> cases = {{"i8", BuiltinTypeKind::I8}, {"i16", BuiltinTypeKind::I16}, {"i32", BuiltinTypeKind::I32}, {"i64", BuiltinTypeKind::I64}, {"i128", BuiltinTypeKind::I128}, {"u8", BuiltinTypeKind::U8}, {"u16", BuiltinTypeKind::U16}, {"u32", BuiltinTypeKind::U32}, {"u64", BuiltinTypeKind::U64}, {"u128", BuiltinTypeKind::U128}, {"int", BuiltinTypeKind::Int}, {"uint", BuiltinTypeKind::UInt}, {"ptrsize", BuiltinTypeKind::PtrSize}, {"f16", BuiltinTypeKind::F16}, {"f32", BuiltinTypeKind::F32}, {"f64", BuiltinTypeKind::F64}, {"bool", BuiltinTypeKind::Bool}, {"byte", BuiltinTypeKind::Byte}, {"void", BuiltinTypeKind::Void}, {"never", BuiltinTypeKind::Never}, {"type", BuiltinTypeKind::Type}};

  for (const BuiltinTypeCase& test : cases) {
    SCOPED_TRACE(test.spelling);
    const LexedFile result = tokenize(test.spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::BuiltinType);
    EXPECT_EQ(result.raw(token), test.spelling);
    ASSERT_TRUE(std::holds_alternative<BuiltinTypeKind>(token.payload));
    EXPECT_EQ(std::get<BuiltinTypeKind>(token.payload), test.value);
  }
}

TEST(KeywordsAndBuiltinTypesTest, KeepsCaseVariantsNearMissesAndFunctionStyleBuiltinsAsIdentifiers) {
  const std::vector<std::string> spellings = {"Func", "TRUE", "Null", "functional", "i32value", "nullable", "constructorValue", "cast", "bitcast", "ptrcast", "try_cast", "reflect", "function", "String", "UnicodeScalar", "f128", "u256", "public", "static"};

  for (const std::string& spelling : spellings) {
    SCOPED_TRACE(spelling);
    const LexedFile result = tokenize(spelling);
    ASSERT_TRUE(result.succeeded());
    ASSERT_EQ(result.tokens().size(), 2U);
    const Token& token = result.tokens().front();
    EXPECT_EQ(token.kind, TokenKind::Identifier);
    EXPECT_EQ(result.raw(token), spelling);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(token.payload));
  }
}

TEST(KeywordsAndBuiltinTypesTest, ClassificationDoesNotDependOnSurroundingSyntax) {
  const LexedFile result = tokenize("func func: func");
  ASSERT_TRUE(result.succeeded());
  ASSERT_EQ(result.tokens().size(), 7U);
  EXPECT_EQ(result.tokens()[0].kind, TokenKind::Keyword);
  EXPECT_EQ(std::get<KeywordKind>(result.tokens()[0].payload), KeywordKind::Func);
  EXPECT_EQ(result.tokens()[1].kind, TokenKind::SpacesAndTabs);
  EXPECT_EQ(result.tokens()[2].kind, TokenKind::Keyword);
  EXPECT_EQ(std::get<KeywordKind>(result.tokens()[2].payload), KeywordKind::Func);
  EXPECT_EQ(result.tokens()[3].kind, TokenKind::Symbol);
  EXPECT_EQ(std::get<char>(result.tokens()[3].payload), ':');
  EXPECT_EQ(result.tokens()[4].kind, TokenKind::SpacesAndTabs);
  EXPECT_EQ(result.tokens()[5].kind, TokenKind::Keyword);
  EXPECT_EQ(result.tokens()[6].kind, TokenKind::EndOfFile);
}

}  // namespace
}  // namespace ink::tokenizer
