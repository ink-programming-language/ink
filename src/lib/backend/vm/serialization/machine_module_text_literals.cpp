#include "machine_module_text_literals.h"

#include "ink/core/name/name_pool.h"

#include <bit>
#include <charconv>
#include <limits>

#define ERROR_IF(Condition, Span, Description) \
    do                                         \
    {                                          \
        if (Condition)                         \
        {                                      \
            return failAt(Span, Description);  \
        }                                      \
    } while (false)

namespace ink::vm::detail
{
    ParseResult<core::Name> internName(std::string_view Text, TextSpan Span)
    {
        const core::Name Interned = core::NamePool::instance().intern(Text);
        ERROR_IF(!Text.empty() && !Interned.valid(), Span, "name pool capacity exhausted");
        return Interned;
    }

    ParseResult<std::uint64_t> decodeUnsignedInteger(std::string_view Digits, TextSpan Span, std::string_view ExpectedMessage)
    {
        int Base = 10;
        if (Digits.starts_with("0x") || Digits.starts_with("0X"))
        {
            Digits.remove_prefix(2);
            Base = 16;
        }
        ERROR_IF(Digits.empty(), Span, ExpectedMessage);
        std::uint64_t Value = 0;
        const auto Result = std::from_chars(Digits.data(), Digits.data() + Digits.size(), Value, Base);
        ERROR_IF(Result.ec == std::errc::result_out_of_range, Span, "unsigned integer overflows 64-bit range");
        ERROR_IF(Result.ec != std::errc{} || Result.ptr != Digits.data() + Digits.size(), Span, ExpectedMessage);
        return Value;
    }

    ParseResult<std::int64_t> applyDisplacementSign(bool Negative, std::uint64_t Magnitude, TextSpan Span)
    {
        constexpr std::uint64_t SignBit = std::uint64_t{1} << 63;
        ERROR_IF(Magnitude > SignBit || (!Negative && Magnitude == SignBit), Span, "signed displacement overflows 64-bit two's complement range");
        if (!Negative)
        {
            return static_cast<std::int64_t>(Magnitude);
        }
        return Magnitude == SignBit ? std::numeric_limits<std::int64_t>::min() : -static_cast<std::int64_t>(Magnitude);
    }

    ParseResult<std::uint64_t> applyIntegerWidthAndSign(std::uint32_t BitWidth, bool Negative, std::uint64_t Magnitude, TextSpan Span)
    {
        const std::uint64_t MaxUnsigned = BitWidth == 64 ? std::numeric_limits<std::uint64_t>::max() : ((std::uint64_t{1} << BitWidth) - 1);
        const std::uint64_t MaxNegativeMagnitude = std::uint64_t{1} << (BitWidth - 1);
        if (Negative)
        {
            ERROR_IF(Magnitude > MaxNegativeMagnitude, Span, "negative integer overflows target bit width");
            return (std::uint64_t{0} - Magnitude) & MaxUnsigned;
        }
        ERROR_IF(Magnitude > MaxUnsigned, Span, "integer literal overflows target bit width");
        return Magnitude;
    }

    ParseResult<std::uint64_t> decodeFloat64Bits(bool Negative, const Token& ValueToken, TextSpan FullSpan)
    {
        if (ValueToken.Kind == TokenKind::Identifier)
        {
            if (ValueToken.text() == "inf")
            {
                const double Value = Negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
                return std::bit_cast<std::uint64_t>(Value);
            }
            if (ValueToken.text() == "nan")
            {
                return Negative ? (CanonicalQuietNaNBits | (std::uint64_t{1} << 63)) : CanonicalQuietNaNBits;
            }
            return failAt(FullSpan, "expected an IEEE 754 binary64 floating-point value");
        }
        ERROR_IF(ValueToken.Kind != TokenKind::Number, FullSpan, "expected an IEEE 754 binary64 floating-point value");
        std::string_view Digits = ValueToken.text();
        double Value = 0.0;
        std::from_chars_result Result{};
        if (Digits.starts_with("0x") || Digits.starts_with("0X"))
        {
            ERROR_IF(Digits.find('p') == std::string_view::npos && Digits.find('P') == std::string_view::npos, FullSpan, "hexadecimal .f64 literals require a binary exponent 'p'; use .i64 for raw 64-bit bit patterns");
            Digits.remove_prefix(2);
            Result = std::from_chars(Digits.data(), Digits.data() + Digits.size(), Value, std::chars_format::hex);
        }
        else
        {
            Result = std::from_chars(Digits.data(), Digits.data() + Digits.size(), Value, std::chars_format::general);
        }
        ERROR_IF(Digits.empty() || Result.ptr != Digits.data() + Digits.size(), FullSpan, "expected an IEEE 754 binary64 floating-point value");
        ERROR_IF(Result.ec == std::errc::result_out_of_range, FullSpan, "floating-point literal is out of IEEE 754 binary64 range");
        ERROR_IF(Result.ec != std::errc{}, FullSpan, "expected an IEEE 754 binary64 floating-point value");
        if (Negative)
        {
            Value = -Value;
        }
        return std::bit_cast<std::uint64_t>(Value);
    }
} // namespace ink::vm::detail

#undef ERROR_IF
