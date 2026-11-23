#pragma once

#include "machine_module_text_format.h"
#include "machine_module_text_lexer.h"

#include "ink/core/name/name.h"

#include <cstdint>
#include <string_view>

namespace ink::vm::detail
{
    ParseResult<core::Name> internName(std::string_view Text, TextSpan Span);
    ParseResult<std::uint64_t> decodeUnsignedInteger(std::string_view Digits, TextSpan Span, std::string_view ExpectedMessage);
    ParseResult<std::int64_t> applyDisplacementSign(bool Negative, std::uint64_t Magnitude, TextSpan Span);
    ParseResult<std::uint64_t> applyIntegerWidthAndSign(std::uint32_t BitWidth, bool Negative, std::uint64_t Magnitude, TextSpan Span);
    ParseResult<std::uint64_t> decodeFloat64Bits(bool Negative, const Token& ValueToken, TextSpan FullSpan);
} // namespace ink::vm::detail
