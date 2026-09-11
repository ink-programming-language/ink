#include "grammar_test_support.h"

namespace ink::tokenizer
{
  namespace
  {
    using namespace grammar_test;

    // Verifies every keyword in lexer.bnf has its exact typed keyword payload, including built-in types and constants.
    TEST(KeywordsAndBuiltinTypesTest, ClassifiesEveryGrammarKeyword)
    {
      const std::vector<std::pair<std::string, KeywordKind>> Cases = {
          {"as", KeywordKind::As},
          {"bool", KeywordKind::Bool},
          {"break", KeywordKind::Break},
          {"class", KeywordKind::Class},
          {"class_field", KeywordKind::ClassField},
          {"class_method", KeywordKind::ClassMethod},
          {"comptime", KeywordKind::Comptime},
          {"const", KeywordKind::Const},
          {"continue", KeywordKind::Continue},
          {"defer", KeywordKind::Defer},
          {"double", KeywordKind::Double},
          {"else", KeywordKind::Else},
          {"enum", KeywordKind::Enum},
          {"enum_field", KeywordKind::EnumField},
          {"extends", KeywordKind::Extends},
          {"extern", KeywordKind::Extern},
          {"false", KeywordKind::False},
          {"float", KeywordKind::Float},
          {"for", KeywordKind::For},
          {"from", KeywordKind::From},
          {"func", KeywordKind::Func},
          {"if", KeywordKind::If},
          {"implements", KeywordKind::Implements},
          {"import", KeywordKind::Import},
          {"in", KeywordKind::In},
          {"int8", KeywordKind::Int8},
          {"int16", KeywordKind::Int16},
          {"int32", KeywordKind::Int32},
          {"int64", KeywordKind::Int64},
          {"interface", KeywordKind::Interface},
          {"let", KeywordKind::Let},
          {"null", KeywordKind::Null},
          {"private", KeywordKind::Private},
          {"ptr", KeywordKind::Ptr},
          {"public", KeywordKind::Public},
          {"ref", KeywordKind::Ref},
          {"return", KeywordKind::Return},
          {"self", KeywordKind::Self},
          {"super", KeywordKind::Super},
          {"true", KeywordKind::True},
          {"type", KeywordKind::Type},
          {"uint8", KeywordKind::UInt8},
          {"uint16", KeywordKind::UInt16},
          {"uint32", KeywordKind::UInt32},
          {"uint64", KeywordKind::UInt64},
          {"var", KeywordKind::Var},
          {"void", KeywordKind::Void},
          {"while", KeywordKind::While},
      };
      for (const auto &Entry : Cases)
      {
        SCOPED_TRACE(Entry.first);
        const TokenizedBuffer File = tokenize(Entry.first);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Entry.first});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Keyword);
        const KeywordKind *Payload = std::get_if<KeywordKind>(&File.tokens()[0].Payload);
        ASSERT_NE(Payload, nullptr);
        EXPECT_EQ(*Payload, Entry.second);
        EXPECT_EQ(keywordSpelling(*Payload), Entry.first);
        EXPECT_EQ(lookupKeyword(Entry.first), Entry.second);
      }
    }

    // Verifies former keywords and type aliases no longer reserved by lexer.bnf remain ordinary identifiers.
    TEST(KeywordsAndBuiltinTypesTest, RemovedSpellingsAndCaseVariantsRemainIdentifiers)
    {
      const std::vector<std::string> Cases = {
          "async",
          "await",
          "decorator",
          "final",
          "implicit",
          "override",
          "protected",
          "static",
          "this",
          "virtual",
          "i8",
          "i32",
          "u32",
          "i128",
          "int",
          "uint",
          "usize",
          "ptrsize",
          "f16",
          "f32",
          "f64",
          "byte",
          "never",
          "Bool",
          "TRUE",
          "Null",
          "Class",
          "Class_field",
          "INT32",
      };
      for (const std::string &Source : Cases)
      {
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Source});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Identifier);
        EXPECT_FALSE(lookupKeyword(Source).has_value());
      }
    }

    // Verifies maximal identifier matching wins over a keyword prefix even when the suffix is Unicode.
    TEST(KeywordsAndBuiltinTypesTest, KeywordPrefixesDoNotSplitIdentifiers)
    {
      const std::vector<std::string> Cases = {
          "class_field_extra",
          "class_method2",
          "int320",
          "true_",
          "nullptr",
          "superman",
          "if" + utf8(u8"变量"),
      };
      for (const std::string &Source : Cases)
      {
        const TokenizedBuffer File = tokenize(Source);
        ASSERT_TRUE(File.succeeded());
        expectRaws(File, {Source});
        EXPECT_EQ(File.tokens()[0].Kind, TokenKind::Identifier);
      }
    }

    // Verifies keyword classification remains lexical after member access and inside otherwise invalid syntax.
    TEST(KeywordsAndBuiltinTypesTest, ClassificationDoesNotDependOnSurroundingSyntax)
    {
      const TokenizedBuffer File = tokenize("x.class self.true (int32) {null}");
      ASSERT_TRUE(File.succeeded());
      expectRaws(File, {"x", ".", "class", "self", ".", "true", "(", "int32", ")", "{", "null", "}"});
      for (std::size_t Index : {2U, 3U, 5U, 7U, 10U})
      {
        EXPECT_EQ(File.tokens()[Index].Kind, TokenKind::Keyword);
      }
    }
  } // namespace
} // namespace ink::tokenizer