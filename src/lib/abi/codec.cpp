#include "codec.h"

#include <charconv>
#include <cstdlib>
#include <limits>

#include <utf8proc.h>

namespace ink::abi::detail
{
  namespace
  {
    static_assert(UTF8PROC_VERSION_MAJOR == 2 && UTF8PROC_VERSION_MINOR == 9 && UTF8PROC_VERSION_PATCH == 0, "Ink requires utf8proc 2.9.0 with Unicode 15.1 data");

    bool isValidUtf8(std::string_view Text) noexcept
    {
      if (Text.size() > static_cast<std::size_t>(std::numeric_limits<utf8proc_ssize_t>::max()))
      {
        return false;
      }
      std::size_t Position = 0;
      while (Position < Text.size())
      {
        utf8proc_int32_t CodePoint = 0;
        const utf8proc_ssize_t Length = utf8proc_iterate(reinterpret_cast<const utf8proc_uint8_t *>(Text.data() + Position), static_cast<utf8proc_ssize_t>(Text.size() - Position), &CodePoint);
        if (Length <= 0)
        {
          return false;
        }
        Position += static_cast<std::size_t>(Length);
      }
      return true;
    }
  } // namespace

  NameValidationResult validateName(std::string_view Name) noexcept
  {
    if (Name.empty())
    {
      return NameValidationResult::Empty;
    }
    if (!isValidUtf8(Name))
    {
      return NameValidationResult::InvalidUtf8;
    }

    bool IsAscii = true;
    for (char Character : Name)
    {
      if (static_cast<unsigned char>(Character) > 0x7F)
      {
        IsAscii = false;
        break;
      }
    }
    if (IsAscii)
    {
      return NameValidationResult::Valid;
    }

    utf8proc_uint8_t *NormalizedData = nullptr;
    const auto Options = static_cast<utf8proc_option_t>(UTF8PROC_STABLE | UTF8PROC_COMPOSE);
    const utf8proc_ssize_t NormalizedLength = utf8proc_map(reinterpret_cast<const utf8proc_uint8_t *>(Name.data()), static_cast<utf8proc_ssize_t>(Name.size()), &NormalizedData, Options);
    if (NormalizedLength == UTF8PROC_ERROR_NOMEM)
    {
      std::free(NormalizedData);
      std::abort();
    }
    if (NormalizedLength < 0)
    {
      std::free(NormalizedData);
      return NameValidationResult::NormalizationFailed;
    }
    const std::string_view Normalized(reinterpret_cast<const char *>(NormalizedData), static_cast<std::size_t>(NormalizedLength));
    const bool Matches = Name == Normalized;
    std::free(NormalizedData);
    return Matches ? NameValidationResult::Valid : NameValidationResult::NotNormalized;
  }

  void appendUnsigned(std::string &Output, std::uint64_t Value)
  {
    char Buffer[32];
    const auto Result = std::to_chars(Buffer, Buffer + sizeof(Buffer), Value);
    Output.append(Buffer, static_cast<std::size_t>(Result.ptr - Buffer));
  }

  void appendUpperHexadecimal(std::string &Output, std::string_view Bytes)
  {
    static constexpr char Digits[] = "0123456789ABCDEF";
    for (char ByteValue : Bytes)
    {
      const auto Byte = static_cast<unsigned char>(ByteValue);
      Output.push_back(Digits[Byte >> 4U]);
      Output.push_back(Digits[Byte & 0x0FU]);
    }
  }

  std::uint8_t uppercaseHexadecimalValue(char Character) noexcept
  {
    if (Character >= '0' && Character <= '9')
    {
      return static_cast<std::uint8_t>(Character - '0');
    }
    if (Character >= 'A' && Character <= 'F')
    {
      return static_cast<std::uint8_t>(Character - 'A' + 10);
    }
    return 0xFF;
  }

  std::size_t canonicalBitHexDigitCount(const Type &ValueType) noexcept
  {
    if (ValueType.kind() == TypeKind::Byte)
    {
      return 2;
    }
    return ValueType.kind() == TypeKind::I32 ? 8 : 0;
  }

  bool validateCanonicalBits(const Type &ValueType, std::string_view Bits) noexcept
  {
    const std::size_t ExpectedDigits = canonicalBitHexDigitCount(ValueType);
    if (ExpectedDigits == 0 || Bits.size() != ExpectedDigits)
    {
      return false;
    }
    for (char Character : Bits)
    {
      if (uppercaseHexadecimalValue(Character) == 0xFF)
      {
        return false;
      }
    }
    return true;
  }
} // namespace ink::abi::detail
