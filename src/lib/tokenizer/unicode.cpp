#include "unicode.h"

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <new>
#include <stdexcept>

#include <UnicodeCharSets.h>
#include <utf8proc.h>

namespace ink::tokenizer::unicode
{
  namespace
  {
    static_assert(UTF8PROC_VERSION_MAJOR == 2 && UTF8PROC_VERSION_MINOR == 9 && UTF8PROC_VERSION_PATCH == 0, "Ink requires utf8proc 2.9.0 with Unicode 15.1 data");

    const llvm::sys::UnicodeCharSet XidStartCharacters(XIDStartRanges);
    const llvm::sys::UnicodeCharSet XidContinueCharacters(XIDContinueRanges);

    struct CodePointRange
    {
      char32_t Lower;
      char32_t Upper;
    };

    template <typename Range, std::size_t Size>
    bool contains(const Range (&Ranges)[Size], char32_t Value) noexcept
    {
      std::size_t First = 0;
      std::size_t Count = Size;
      while (Count != 0)
      {
        const std::size_t Step = Count / 2;
        const std::size_t Index = First + Step;
        if (Ranges[Index].Upper < Value)
        {
          First = Index + 1;
          Count -= Step + 1;
        }
        else
        {
          Count = Step;
        }
      }
      return First != Size && Ranges[First].Lower <= Value;
    }

    bool isContinuation(unsigned char Value) noexcept
    {
      return Value >= 0x80 && Value <= 0xBF;
    }

    std::size_t invalidSequenceLength(std::string_view Source, std::size_t Offset, std::size_t Expected) noexcept
    {
      std::size_t Length = 1;
      while (Length < Expected && Offset + Length < Source.size() && isContinuation(static_cast<unsigned char>(Source[Offset + Length])))
      {
        ++Length;
      }
      return Length;
    }
  } // namespace

  DecodeResult decode(std::string_view Source, std::size_t Offset) noexcept
  {
    if (Offset >= Source.size())
    {
      return {};
    }
    const auto First = static_cast<unsigned char>(Source[Offset]);
    if (First <= 0x7F)
    {
      return {First, 1, true};
    }

    std::size_t Length = 0;
    char32_t Value = 0;
    if (First >= 0xC2 && First <= 0xDF)
    {
      Length = 2;
      Value = First & 0x1F;
    }
    else if (First >= 0xE0 && First <= 0xEF)
    {
      Length = 3;
      Value = First & 0x0F;
    }
    else if (First >= 0xF0 && First <= 0xF4)
    {
      Length = 4;
      Value = First & 0x07;
    }
    else if (First == 0xC0 || First == 0xC1)
    {
      return {0, invalidSequenceLength(Source, Offset, 2), false};
    }
    else if (First >= 0xF5 && First <= 0xF7)
    {
      return {0, invalidSequenceLength(Source, Offset, 4), false};
    }
    else
    {
      return {0, 1, false};
    }

    if (Offset + Length > Source.size())
    {
      return {0, invalidSequenceLength(Source, Offset, Length), false};
    }
    for (std::size_t Index = 1; Index < Length; ++Index)
    {
      const auto Byte = static_cast<unsigned char>(Source[Offset + Index]);
      if (!isContinuation(Byte))
      {
        return {0, Index, false};
      }
      Value = static_cast<char32_t>((Value << 6U) | (Byte & 0x3F));
    }

    const auto Second = static_cast<unsigned char>(Source[Offset + 1]);
    if ((First == 0xE0 && Second < 0xA0) || (First == 0xED && Second > 0x9F) || (First == 0xF0 && Second < 0x90) || (First == 0xF4 && Second > 0x8F))
    {
      return {0, Length, false};
    }
    return {Value, Length, true};
  }

  bool isXidStart(char32_t Value) noexcept
  {
    return XidStartCharacters.contains(static_cast<std::uint32_t>(Value));
  }

  bool isXidContinue(char32_t Value) noexcept
  {
    return isXidStart(Value) || XidContinueCharacters.contains(static_cast<std::uint32_t>(Value));
  }

  bool isNfc(std::string_view Source)
  {
    if (Source.empty())
    {
      return true;
    }
    bool IsAscii = true;
    for (const char Character : Source)
    {
      if (static_cast<unsigned char>(Character) > 0x7F)
      {
        IsAscii = false;
        break;
      }
    }
    if (IsAscii)
    {
      return true;
    }
    if (Source.size() > static_cast<std::size_t>(std::numeric_limits<utf8proc_ssize_t>::max()))
    {
      throw std::length_error("UTF-8 input is too large for utf8proc NFC normalization");
    }

    utf8proc_uint8_t *NormalizedData = nullptr;
    const auto Options = static_cast<utf8proc_option_t>(UTF8PROC_STABLE | UTF8PROC_COMPOSE);
    const utf8proc_ssize_t NormalizedLength = utf8proc_map(reinterpret_cast<const utf8proc_uint8_t *>(Source.data()), static_cast<utf8proc_ssize_t>(Source.size()), &NormalizedData, Options);
    if (NormalizedLength < 0)
    {
      std::free(NormalizedData);
      if (NormalizedLength == UTF8PROC_ERROR_NOMEM)
      {
        throw std::bad_alloc();
      }
      if (NormalizedLength == UTF8PROC_ERROR_OVERFLOW)
      {
        throw std::length_error("utf8proc NFC normalization overflowed its supported output length");
      }
      throw std::runtime_error(utf8proc_errmsg(NormalizedLength));
    }

    const std::string_view Normalized(reinterpret_cast<const char *>(NormalizedData), static_cast<std::size_t>(NormalizedLength));
    const bool IsNormalized = Source == Normalized;
    std::free(NormalizedData);
    return IsNormalized;
  }

  bool isDefaultIgnorable(char32_t Value) noexcept
  {
    return utf8proc_get_property(static_cast<utf8proc_int32_t>(Value))->ignorable != 0;
  }

  bool isUnicodeWhitespace(char32_t Value) noexcept
  {
    static constexpr CodePointRange Ranges[] = {
        {0x0085, 0x0085},
        {0x00A0, 0x00A0},
        {0x1680, 0x1680},
        {0x2000, 0x200A},
        {0x2028, 0x2029},
        {0x202F, 0x202F},
        {0x205F, 0x205F},
        {0x3000, 0x3000},
        {0xFEFF, 0xFEFF},
    };
    return contains(Ranges, Value);
  }

  void appendUtf8(std::string &Output, char32_t Value)
  {
    if (Value <= 0x7F)
    {
      Output.push_back(static_cast<char>(Value));
    }
    else if (Value <= 0x7FF)
    {
      Output.push_back(static_cast<char>(0xC0 | (Value >> 6U)));
      Output.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
    }
    else if (Value <= 0xFFFF)
    {
      Output.push_back(static_cast<char>(0xE0 | (Value >> 12U)));
      Output.push_back(static_cast<char>(0x80 | ((Value >> 6U) & 0x3F)));
      Output.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
    }
    else
    {
      Output.push_back(static_cast<char>(0xF0 | (Value >> 18U)));
      Output.push_back(static_cast<char>(0x80 | ((Value >> 12U) & 0x3F)));
      Output.push_back(static_cast<char>(0x80 | ((Value >> 6U) & 0x3F)));
      Output.push_back(static_cast<char>(0x80 | (Value & 0x3F)));
    }
  }
} // namespace ink::tokenizer::unicode
