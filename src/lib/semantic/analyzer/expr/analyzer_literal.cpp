#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/config/numeric_literal.h"
#include "ink/core/support/unicode.h"
#include "ink/frontend/parser/parser.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
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
    std::optional<std::uint64_t> parseIntegerLiteral(SemanticContext &Context, const frontend::parser::TokenBuffer &Input, const frontend::parser::LiteralExpr &Node, const frontend::tokenizer::Token &Token)
    {
      RETURN_IF(Node.literalKind() != frontend::parser::TokenKind::IntegerLiteral, std::nullopt);
      const auto *Numeric = std::get_if<frontend::tokenizer::NumericInfo>(&Token.Payload);
      RETURN_REPORT_IF(Context, !Numeric || !Numeric->hasValidBase(), core::DiagnosticKind::TokenizerInvalidNumericToken, std::nullopt, Node.getSourceRange(), "invalid integer metadata");
      std::string_view Text = Input.spelling(Token);
      if (Numeric->Base != core::NumericBase::Decimal)
      {
        RETURN_REPORT_IF(Context, Text.size() <= core::NumericBasePrefixLength, core::DiagnosticKind::TokenizerInvalidNumericToken, std::nullopt, Node.getSourceRange(), "non-decimal integer is missing its prefix or digits");
        Text.remove_prefix(core::NumericBasePrefixLength);
      }
      std::uint64_t Value = 0;
      const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value, static_cast<int>(Numeric->Base));
      RETURN_REPORT_IF(Context, Parsed.ptr != Text.data() + Text.size() || (Parsed.ec != std::errc{} && Parsed.ec != std::errc::result_out_of_range), core::DiagnosticKind::TokenizerInvalidNumericToken, std::nullopt, Node.getSourceRange(), "invalid integer digits");
      RETURN_IF(Parsed.ec == std::errc::result_out_of_range, std::nullopt);
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
      RETURN_REPORT_IF_FALSE(Context, Constant, core::DiagnosticKind::SemanticIntegerOutOfRange, nullptr, Range, Type.toString());
      return Constant;
    }

    template <typename FloatT>
    const ir::FloatConstant *parseFloat(ir::ConstantPool &Constants, std::string_view Text)
    {
      FloatT Value = 0;
      const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value, std::chars_format::general);
      RETURN_IF(Parsed.ec != std::errc{} || Parsed.ptr != Text.data() + Text.size(), nullptr);
      return Constants.getFloatConstant(Value);
    }

    const ir::Value *materializeFloat(AnalyzerState &State, const frontend::parser::LiteralExpr &Node, std::string_view Text, const ir::FloatType &Type)
    {
      auto &Context = State.semanticContext();
      auto &Constants = Context.irContext().constantPool();
      const auto *Constant = Type.width() == 32 ? parseFloat<float>(Constants, Text) : parseFloat<double>(Constants, Text);
      RETURN_REPORT_IF_FALSE(Context, Constant, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.getSourceRange(), Type.toString(), "an invalid or out-of-range floating-point literal");
      return Constant;
    }
  } // namespace

  const ir::Value *Analyzer::analyzeLiteralExpr(AnalyzerState &State, const frontend::parser::LiteralExpr &Node, ExprHints Hints)
  {
    auto &Context = State.semanticContext();
    const ir::Type *ExpectedType = Hints.ExpectedType;
    const auto &Input = State.input();
    const auto &Token = Input.requireToken(Node.token(), Context.diagnosticEngine(), Node.getSourceRange());
    RETURN_REPORT_IF(Context, Token.Kind != Node.literalKind(), core::DiagnosticKind::SemanticInvalidAST, nullptr, Node.getSourceRange(), "LiteralExpr kind must match its referenced token kind");
    using frontend::parser::TokenKind;
    switch (Node.literalKind())
    {
      case TokenKind::IntegerLiteral:
      {
        EXPECTED_TYPE(Context, Node.getSourceRange(), ExpectedType, ir::IntegerType, "an integer type");
        const auto *DefaultType = Hints.DefaultIntegerLiteralType ? Hints.DefaultIntegerLiteralType : Context.typePool().getInt32Type();
        const auto *Type = ExpectedType ? static_cast<const ir::IntegerType *>(ExpectedType) : DefaultType;
        const auto Value = parseIntegerLiteral(Context, Input, Node, Token);
        RETURN_REPORT_IF_FALSE(Context, Value, core::DiagnosticKind::SemanticIntegerOutOfRange, nullptr, Node.getSourceRange(), Type->toString());
        return materializeInteger(State, Node.getSourceRange(), *Value, *Type);
      }
      case TokenKind::FloatLiteral:
      {
        EXPECTED_TYPE(Context, Node.getSourceRange(), ExpectedType, ir::FloatType, "a floating-point type");
        const auto *Numeric = std::get_if<frontend::tokenizer::NumericInfo>(&Token.Payload);
        RETURN_REPORT_IF(Context, !Numeric || Numeric->Base != core::NumericBase::Decimal, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.getSourceRange(), "a decimal floating-point literal", "invalid numeric metadata");
        const auto *Type = ExpectedType ? static_cast<const ir::FloatType *>(ExpectedType) : Context.typePool().getFloat64Type();
        const std::string_view Text = Input.spelling(Token);
        return materializeFloat(State, Node, Text, *Type);
      }
      case TokenKind::CharLiteral:
      {
        EXPECTED_TYPE(Context, Node.getSourceRange(), ExpectedType, ir::IntegerType, "an integer type");
        const auto *Character = std::get_if<frontend::tokenizer::CharInfo>(&Token.Payload);
        RETURN_REPORT_IF_FALSE(Context, Character, core::DiagnosticKind::SemanticInvalidAST, nullptr, Node.getSourceRange(), "character LiteralExpr must reference a token with CharInfo metadata");
        RETURN_REPORT_IF_FALSE(Context, core::isUnicodeScalarValue(Character->Value), core::DiagnosticKind::SemanticInvalidAST, nullptr, Node.getSourceRange(), "character LiteralExpr metadata must contain a Unicode scalar value");
        const auto *Type = ExpectedType ? static_cast<const ir::IntegerType *>(ExpectedType) : Context.typePool().getUInt32Type();
        return materializeInteger(State, Node.getSourceRange(), static_cast<std::uint64_t>(Character->Value), *Type);
      }
      case TokenKind::StringLiteral:
      {
        const auto *String = std::get_if<frontend::tokenizer::StringInfo>(&Token.Payload);
        RETURN_REPORT_IF_FALSE(Context, String, core::DiagnosticKind::SemanticInvalidAST, nullptr, Node.getSourceRange(), "string LiteralExpr must reference a token with StringInfo metadata");
        const auto *StringType = Context.typePool().getArrayType(Context.typePool().getUInt8Type(), String->Decoded.size());
        RETURN_REPORT_IF_FALSE(Context, StringType, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.getSourceRange(), "a string with representable byte length", "an invalid string layout");
        CHECK_TYPE_MATCH(Context, Node.getSourceRange(), ExpectedType, StringType, nullptr);
        const auto *Constant = Context.irContext().constantPool().getStringConstant(String->Decoded);
        RETURN_REPORT_IF_FALSE(Context, Constant, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.getSourceRange(), "a valid UTF-8 string with representable byte length", "invalid string bytes or length");
        return Constant;
      }
      default:
        reportUnsupported(State, Node);
        return nullptr;
    }
  }
} // namespace ink::semantic
