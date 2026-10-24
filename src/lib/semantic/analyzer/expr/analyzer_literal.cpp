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
    std::optional<std::uint64_t> parseIntegerLiteral(const AnalysisSite &Site, const frontend::parser::TokenBuffer &Input, const frontend::tokenizer::Token &Token)
    {
      const auto *Numeric = std::get_if<frontend::tokenizer::NumericInfo>(&Token.Payload);
      CHECK_ICE(Site, Numeric && Numeric->hasValidBase(), core::DiagnosticKind::TokenizerInvalidNumericToken, "invalid integer metadata");
      std::string_view Text = Input.spelling(Token);
      if (Numeric->Base != core::NumericBase::Decimal)
      {
        CHECK_ICE(Site, Text.size() > core::NumericBasePrefixLength, core::DiagnosticKind::TokenizerInvalidNumericToken, "non-decimal integer is missing its prefix or digits");
        Text.remove_prefix(core::NumericBasePrefixLength);
      }
      std::uint64_t Value = 0;
      const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value, static_cast<int>(Numeric->Base));
      CHECK_ICE(Site, Parsed.ptr == Text.data() + Text.size() && (Parsed.ec == std::errc{} || Parsed.ec == std::errc::result_out_of_range), core::DiagnosticKind::TokenizerInvalidNumericToken, "invalid integer digits");
      RETURN_IF(Parsed.ec == std::errc::result_out_of_range, std::nullopt);
      return Value;
    }

    const ir::Value *materializeInteger(const AnalysisSite &Site, std::uint64_t Value, const ir::IntegerType &Type)
    {
      auto &Context = Site.semanticContext();
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
      RETURN_ERROR_IF_NOT(Site, Constant, core::DiagnosticKind::SemanticIntegerOutOfRange, nullptr, Type.toString());
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

    const ir::Value *materializeFloat(const AnalysisSite &Site, std::string_view Text, const ir::FloatType &Type)
    {
      auto &Context = Site.semanticContext();
      auto &Constants = Context.irContext().constantPool();
      const auto *Constant = Type.width() == 32 ? parseFloat<float>(Constants, Text) : parseFloat<double>(Constants, Text);
      RETURN_ERROR_IF_NOT(Site, Constant, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Type.toString(), "an invalid or out-of-range floating-point literal");
      return Constant;
    }
  } // namespace

  const ir::Value *Analyzer::analyzeLiteralExpr(AnalyzerState &State, const frontend::parser::LiteralExpr &Node, ExprHints Hints)
  {
    auto &Context = State.semanticContext();
    const auto Site = State.at(Node);
    const ir::Type *ExpectedType = Hints.ExpectedType ? Hints.ExpectedType->unqualifiedType() : nullptr;
    const auto &Input = State.input();
    const auto &Token = Input.requireToken(Node.token(), Context.diagnosticEngine(), Node.getSourceRange());
    CHECK_ICE(Site, Token.Kind == Node.literalKind(), core::DiagnosticKind::SemanticInvalidAST, "LiteralExpr kind must match its referenced token kind");
    using frontend::parser::TokenKind;
    switch (Node.literalKind())
    {
      case TokenKind::IntegerLiteral:
      {
        EXPECTED_TYPE(Site, ExpectedType, ir::IntegerType, "an integer type");
        const auto *DefaultType = Hints.DefaultIntegerLiteralType ? Hints.DefaultIntegerLiteralType : Context.typePool().getInt32Type();
        const auto *Type = ExpectedType ? static_cast<const ir::IntegerType *>(ExpectedType) : DefaultType;
        const auto Value = parseIntegerLiteral(Site, Input, Token);
        RETURN_ERROR_IF_NOT(Site, Value, core::DiagnosticKind::SemanticIntegerOutOfRange, nullptr, Type->toString());
        return materializeInteger(Site, *Value, *Type);
      }
      case TokenKind::FloatLiteral:
      {
        EXPECTED_TYPE(Site, ExpectedType, ir::FloatType, "a floating-point type");
        const auto *Numeric = std::get_if<frontend::tokenizer::NumericInfo>(&Token.Payload);
        RETURN_ERROR_IF(Site, !Numeric || Numeric->Base != core::NumericBase::Decimal, core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a decimal floating-point literal", "invalid numeric metadata");
        const auto *Type = ExpectedType ? static_cast<const ir::FloatType *>(ExpectedType) : Context.typePool().getFloat64Type();
        const std::string_view Text = Input.spelling(Token);
        return materializeFloat(Site, Text, *Type);
      }
      case TokenKind::CharLiteral:
      {
        EXPECTED_TYPE(Site, ExpectedType, ir::IntegerType, "an integer type");
        const auto *Character = std::get_if<frontend::tokenizer::CharInfo>(&Token.Payload);
        CHECK_ICE(Site, Character, core::DiagnosticKind::SemanticInvalidAST, "character LiteralExpr must reference a token with CharInfo metadata");
        CHECK_ICE(Site, core::isUnicodeScalarValue(Character->Value), core::DiagnosticKind::SemanticInvalidAST, "character LiteralExpr metadata must contain a Unicode scalar value");
        const auto *Type = ExpectedType ? static_cast<const ir::IntegerType *>(ExpectedType) : Context.typePool().getUInt32Type();
        return materializeInteger(Site, static_cast<std::uint64_t>(Character->Value), *Type);
      }
      case TokenKind::StringLiteral:
      {
        const auto *String = std::get_if<frontend::tokenizer::StringInfo>(&Token.Payload);
        CHECK_ICE(Site, String, core::DiagnosticKind::SemanticInvalidAST, "string LiteralExpr must reference a token with StringInfo metadata");
        const auto *StringType = Context.typePool().getArrayType(Context.typePool().getUInt8Type(), String->Decoded.size());
        RETURN_ERROR_IF_NOT(Site, StringType, core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a string with representable byte length", "an invalid string layout");
        if (!ir::PointerType::classof(ExpectedType))
        {
          CHECK_TYPE_MATCH(Site, ExpectedType, StringType, nullptr);
        }
        // Pointer targets are checked by the shared conversion path, which can materialize read-only byte storage.
        const auto *Constant = Context.irContext().constantPool().getStringConstant(String->Decoded);
        RETURN_ERROR_IF_NOT(Site, Constant, core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a valid UTF-8 string with representable byte length", "invalid string bytes or length");
        return Constant;
      }
      default:
        Site.reportICE<core::DiagnosticKind::SemanticUnsupported>(frontend::parser::astKindName(Node.getKind()));
    }
  }
} // namespace ink::semantic
