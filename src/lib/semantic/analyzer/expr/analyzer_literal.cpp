#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/config/numeric_literal.h"
#include "ink/frontend/parser/parser.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>

namespace ink::semantic
{
  namespace
  {
    std::optional<std::uint64_t> parseIntegerLiteral(SemanticContext &Context, const frontend::parser::ParseResult &Input, const frontend::parser::LiteralExpr &Node)
    {
      if (Node.literalKind() != frontend::parser::TokenKind::IntegerLiteral)
      {
        return std::nullopt;
      }
      const auto &Tokens = Input.Unit->input();
      const auto *Numeric = std::get_if<frontend::tokenizer::NumericInfo>(&Tokens.token(Node.token()).Payload);
      if (!Numeric || !Numeric->hasValidBase())
      {
        Context.report<core::DiagnosticKind::TokenizerInvalidNumericToken>(Node.getSourceRange(), "invalid integer metadata");
        return std::nullopt;
      }
      std::string_view Text = Tokens.spelling(Node.token());
      if (Numeric->Base != core::NumericBase::Decimal)
      {
        if (Text.size() <= core::NumericBasePrefixLength)
        {
          Context.report<core::DiagnosticKind::TokenizerInvalidNumericToken>(Node.getSourceRange(), "non-decimal integer is missing its prefix or digits");
          return std::nullopt;
        }
        Text.remove_prefix(core::NumericBasePrefixLength);
      }
      std::uint64_t Value = 0;
      const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value, static_cast<int>(Numeric->Base));
      if (Parsed.ptr != Text.data() + Text.size() || (Parsed.ec != std::errc{} && Parsed.ec != std::errc::result_out_of_range))
      {
        Context.report<core::DiagnosticKind::TokenizerInvalidNumericToken>(Node.getSourceRange(), "invalid integer digits");
        return std::nullopt;
      }
      if (Parsed.ec == std::errc::result_out_of_range)
      {
        return std::nullopt;
      }
      return Value;
    }

    const ir::Value *materializeInteger(AnalyzerState &State, core::SourceRange Range, std::uint64_t Value, const ir::IntegerType &Type)
    {
      auto &Context = State.semanticContext();
      const ir::IntegerConstant *Constant = nullptr;
      if (Type.isSigned())
      {
        const auto Maximum = Type.maximumValue();
        if (Value <= Maximum)
        {
          Constant = Context.irContext().constantPool().getSignedIntegerConstant(&Type, static_cast<std::int64_t>(Value));
        }
      }
      else
      {
        Constant = Context.irContext().constantPool().getUnsignedIntegerConstant(&Type, Value);
      }
      if (!Constant)
      {
        Context.report<core::DiagnosticKind::SemanticIntegerOutOfRange>(Range, Type.toString());
        return nullptr;
      }
      return Constant;
    }

    template <typename FloatT>
    const ir::FloatConstant *parseFloat(ir::ConstantPool &Constants, std::string_view Text)
    {
      FloatT Value = 0;
      const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value, std::chars_format::general);
      if (Parsed.ec != std::errc{} || Parsed.ptr != Text.data() + Text.size())
      {
        return nullptr;
      }
      return Constants.getFloatConstant(Value);
    }

    const ir::Value *materializeFloat(AnalyzerState &State, const frontend::parser::LiteralExpr &Node, std::string_view Text, const ir::FloatType &Type)
    {
      auto &Context = State.semanticContext();
      auto &Constants = Context.irContext().constantPool();
      const auto *Constant = Type.width() == 32 ? parseFloat<float>(Constants, Text) : parseFloat<double>(Constants, Text);
      if (!Constant)
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), Type.toString(), "an invalid or out-of-range floating-point literal");
      }
      return Constant;
    }
  } // namespace

  const ir::Value *Analyzer::analyzeLiteralExpr(AnalyzerState &State, const frontend::parser::LiteralExpr &Node, ExprHints Hints)
  {
    auto &Context = State.semanticContext();
    const ir::Type *ExpectedType = Hints.ExpectedType;
    const auto &Input = State.input().Unit->input();
    if (Node.token() >= Input.size())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "LiteralExpr token index must reference a token in its input");
      return nullptr;
    }
    const auto &Token = Input.token(Node.token());
    if (Token.Kind != Node.literalKind())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "LiteralExpr kind must match its referenced token kind");
      return nullptr;
    }
    using frontend::parser::TokenKind;
    switch (Node.literalKind())
    {
      case TokenKind::IntegerLiteral:
      {
        if (ExpectedType && !ir::IntegerType::classof(ExpectedType))
        {
          Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "an integer type", ExpectedType->toString());
          return nullptr;
        }
        const auto *DefaultType = Hints.DefaultIntegerLiteralType ? Hints.DefaultIntegerLiteralType : Context.typePool().getInt32Type();
        const auto *Type = ExpectedType ? static_cast<const ir::IntegerType *>(ExpectedType) : DefaultType;
        const auto Value = parseIntegerLiteral(Context, State.input(), Node);
        if (!Value)
        {
          Context.report<core::DiagnosticKind::SemanticIntegerOutOfRange>(Node.getSourceRange(), Type->toString());
          return nullptr;
        }
        return materializeInteger(State, Node.getSourceRange(), *Value, *Type);
      }
      case TokenKind::FloatLiteral:
      {
        if (ExpectedType && !ir::FloatType::classof(ExpectedType))
        {
          Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a floating-point type", ExpectedType->toString());
          return nullptr;
        }
        const auto *Numeric = std::get_if<frontend::tokenizer::NumericInfo>(&Token.Payload);
        if (!Numeric || Numeric->Base != core::NumericBase::Decimal)
        {
          Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a decimal floating-point literal", "invalid numeric metadata");
          return nullptr;
        }
        const auto *Type = ExpectedType ? static_cast<const ir::FloatType *>(ExpectedType) : Context.typePool().getFloat64Type();
        const std::string_view Text = Input.spelling(Node.token());
        return materializeFloat(State, Node, Text, *Type);
      }
      case TokenKind::CharLiteral:
      {
        if (ExpectedType && !ir::IntegerType::classof(ExpectedType))
        {
          Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "an integer type", ExpectedType->toString());
          return nullptr;
        }
        const auto *Character = std::get_if<frontend::tokenizer::CharInfo>(&Token.Payload);
        if (!Character)
        {
          Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "character LiteralExpr must reference a token with CharInfo metadata");
          return nullptr;
        }
        if (Character->Value > 0x10ffff || (Character->Value >= 0xd800 && Character->Value <= 0xdfff))
        {
          Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "character LiteralExpr metadata must contain a Unicode scalar value");
          return nullptr;
        }
        const auto *Type = ExpectedType ? static_cast<const ir::IntegerType *>(ExpectedType) : Context.typePool().getUInt32Type();
        return materializeInteger(State, Node.getSourceRange(), static_cast<std::uint64_t>(Character->Value), *Type);
      }
      case TokenKind::StringLiteral:
      {
        const auto *String = std::get_if<frontend::tokenizer::StringInfo>(&Token.Payload);
        if (!String)
        {
          Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "string LiteralExpr must reference a token with StringInfo metadata");
          return nullptr;
        }
        const auto *StringType = Context.typePool().getArrayType(Context.typePool().getUInt8Type(), String->Decoded.size());
        if (!StringType)
        {
          Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a string with representable byte length", "an invalid string layout");
          return nullptr;
        }
        if (ExpectedType && ExpectedType != StringType)
        {
          Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), ExpectedType->toString(), StringType->toString());
          return nullptr;
        }
        const auto *Constant = Context.irContext().constantPool().getStringConstant(String->Decoded);
        if (!Constant)
        {
          Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a valid UTF-8 string with representable byte length", "invalid string bytes or length");
          return nullptr;
        }
        return Constant;
      }
      default:
        reportUnsupported(State, Node);
        return nullptr;
    }
  }
} // namespace ink::semantic
