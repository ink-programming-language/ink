#include "ink/tokenizer/unicode.h"

#include <cstdint>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include <iterator>

#include <UnicodeCharSets.h>
#include <utf8proc.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Unicode.h>

namespace ink::tokenizer::unicode
{
  namespace
  {
    static_assert(UTF8PROC_VERSION_MAJOR == 2 && UTF8PROC_VERSION_MINOR == 9 && UTF8PROC_VERSION_PATCH == 0, "Ink requires utf8proc 2.9.0 with Unicode 15.1 data");

    const llvm::sys::UnicodeCharSet XidStartCharacters(XIDStartRanges);
    const llvm::sys::UnicodeCharSet XidContinueCharacters(XIDContinueRanges);

    struct NameAlias
    {
        std::string_view Name;
        char32_t Value;
    };

    constexpr NameAlias NameAliases[] = {
#include "unicode_name_aliases.inc"
    };

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

  bool normalizeNfc(std::string_view Source, std::string &Output)
  {
    if (Source.empty())
    {
      Output.assign(Source);
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
      Output.assign(Source);
      return true;
    }
    if (Source.size() > static_cast<std::size_t>(std::numeric_limits<utf8proc_ssize_t>::max()))
    {
      return false;
    }

    utf8proc_uint8_t *NormalizedData = nullptr;
    const auto Options = static_cast<utf8proc_option_t>(UTF8PROC_STABLE | UTF8PROC_COMPOSE);
    const utf8proc_ssize_t NormalizedLength = utf8proc_map(reinterpret_cast<const utf8proc_uint8_t *>(Source.data()), static_cast<utf8proc_ssize_t>(Source.size()), &NormalizedData, Options);
    if (NormalizedLength < 0)
    {
      std::free(NormalizedData);
      return false;
    }

    const std::string_view Normalized(reinterpret_cast<const char *>(NormalizedData), static_cast<std::size_t>(NormalizedLength));
    Output.assign(Normalized);
    std::free(NormalizedData);
    return true;
  }

  std::optional<char32_t> lookupName(std::string_view Name)
  {
    std::string Uppercase;
    Uppercase.reserve(Name.size());
    for (const unsigned char Character : Name)
    {
      if (Character >= 0x80)
      {
        return std::nullopt;
      }
      Uppercase.push_back(static_cast<char>(Character >= 'a' && Character <= 'z' ? Character - 'a' + 'A' : Character));
    }
    // LLVM omits figment and abbreviation aliases for C++ compatibility.
    const auto LessThanName = [](const NameAlias &Entry, std::string_view Query)
    {
      return Entry.Name < Query;
    };
    const auto Alias = std::lower_bound(std::begin(NameAliases), std::end(NameAliases), std::string_view(Uppercase), LessThanName);
    if (Alias != std::end(NameAliases) && Alias->Name == Uppercase)
    {
      return Alias->Value;
    }
    const auto Value = llvm::sys::unicode::nameToCodepointStrict(llvm::StringRef(Uppercase));
    if (!Value)
    {
      return std::nullopt;
    }
    // LLVM's algorithmic-name parser also accepts extra leading zeroes and
    // numeric prefixes. Require the actual Unicode spelling of its suffix.
    constexpr std::string_view Prefixes[] = {
        "CJK UNIFIED IDEOGRAPH-",
        "CJK COMPATIBILITY IDEOGRAPH-",
        "TANGUT IDEOGRAPH-",
        "KHITAN SMALL SCRIPT CHARACTER-",
        "NUSHU CHARACTER-",
    };
    for (const std::string_view Prefix : Prefixes)
    {
      if (Uppercase.compare(0, Prefix.size(), Prefix) != 0)
      {
        continue;
      }
      constexpr char Digits[] = "0123456789ABCDEF";
      std::string Suffix;
      for (char32_t Remaining = *Value; Remaining != 0; Remaining >>= 4U)
      {
        Suffix.push_back(Digits[Remaining & 0xFU]);
      }
      std::reverse(Suffix.begin(), Suffix.end());
      if (std::string_view(Uppercase).substr(Prefix.size()) != Suffix)
      {
        return std::nullopt;
      }
      break;
    }
    return Value;
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
