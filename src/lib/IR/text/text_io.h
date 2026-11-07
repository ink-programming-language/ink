#pragma once

#include "text_internal.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace ink::ir::text
{
  template <typename Enum>
  struct EnumEntry
  {
      Enum Value;
      std::string_view Spelling;
  };

  inline constexpr EnumEntry<ExpressionKind> ExpressionKinds[] = {
      {ExpressionKind::MetaType, "type"},
      {ExpressionKind::BoolType, "bool"},
      {ExpressionKind::VoidType, "void"},
      {ExpressionKind::PointerType, "ptr"},
      {ExpressionKind::ArrayType, "["},
      {ExpressionKind::FunctionType, "fn"},
      {ExpressionKind::IntegerConstant, "int"},
      {ExpressionKind::FloatConstant, "float"},
      {ExpressionKind::StringConstant, "string"},
  };

  class TextReader final
  {
    public:
      static constexpr bool Reading = true;

      explicit TextReader(std::string_view Input);
      bool token(std::string_view Expected);
      bool reference(std::string &Value);
      bool quoted(std::string &Value);
      bool number(std::uint64_t &Value);
      bool expressionKind(Expression &Node);
      bool resultId(std::string &Value);
      bool beginRegion();
      bool endRegion();
      bool indent() noexcept;
      bool unindent() noexcept;
      bool newline() noexcept;
      bool peek(std::string_view Value) const noexcept;
      bool atBlock() const noexcept;
      bool done();
      bool fail(std::string_view Description);
      bool enterExpression();
      void leaveExpression() noexcept;
      void offset(std::size_t &Value) const noexcept;
      std::size_t position() const noexcept;
      const std::optional<core::Diagnostic> &error() const noexcept;

      template <typename Enum, std::size_t Count>
      bool enumValue(Enum &Value, const EnumEntry<Enum> (&Entries)[Count])
      {
        for (const auto &Entry : Entries)
        {
          if (peek(Entry.Spelling))
          {
            Value = Entry.Value;
            return token(Entry.Spelling);
          }
        }
        return fail("expected a recognized keyword");
      }

    private:
      enum class TokenKind
      {
        Word,
        Reference,
        String,
        Punctuation,
        End,
      };

      struct Token
      {
          TokenKind Kind = TokenKind::End;
          std::string Text;
          std::size_t Offset = 0;
      };

      void lex(std::string_view Input);
      bool lexError(std::size_t Offset, std::string_view Description);
      const Token &current() const noexcept;

      std::vector<Token> Tokens;
      std::size_t Index = 0;
      std::size_t ExpressionDepth = 0;
      std::optional<core::Diagnostic> Failure;
  };

  class TextWriter final
  {
    public:
      static constexpr bool Reading = false;

      bool token(std::string_view Value);
      bool reference(std::string &Value);
      bool quoted(std::string &Value);
      bool number(std::uint64_t &Value);
      bool expressionKind(Expression &Node);
      bool resultId(std::string &Value);
      bool beginRegion();
      bool endRegion();
      bool indent() noexcept;
      bool unindent();
      bool newline();
      bool fail(std::string_view Description);
      bool enterExpression();
      void leaveExpression() noexcept;
      void offset(std::size_t &) const noexcept;
      const std::optional<core::Diagnostic> &error() const noexcept;
      std::string take();

      template <typename Enum, std::size_t Count>
      bool enumValue(Enum &Value, const EnumEntry<Enum> (&Entries)[Count])
      {
        for (const auto &Entry : Entries)
        {
          if (Value == Entry.Value)
          {
            return token(Entry.Spelling);
          }
        }
        return fail("cannot print an unknown enumeration value");
      }

    private:
      std::string Output;
      std::string Previous;
      std::size_t Indent = 0;
      std::size_t ExpressionDepth = 0;
      bool LineStart = true;
      std::optional<core::Diagnostic> Failure;
  };
} // namespace ink::ir::text
