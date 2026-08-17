#include "ink/ir/compilation/module_name.h"

#include "ink/tokenizer/unicode.h"

namespace ink::ir
{
  namespace
  {
    bool isSegmentStart(char32_t Character) noexcept
    {
      return tokenizer::unicode::isXidStart(Character) || Character == U'_';
    }

    bool isSegmentContinue(char32_t Character) noexcept
    {
      return tokenizer::unicode::isXidContinue(Character) || Character == U'_';
    }
  } // namespace

  bool isValidModuleName(const Name &NameValue) noexcept
  {
    if (!NameValue.valid())
    {
      return false;
    }
    const std::string_view Text = NameValue.text();
    bool AtSegmentStart = true;
    bool HasPackageSeparator = false;
    std::size_t Position = 0;
    while (Position < Text.size())
    {
      const tokenizer::unicode::DecodeResult Decoded = tokenizer::unicode::decode(Text, Position);
      if (!Decoded.Valid)
      {
        return false;
      }
      if (Decoded.Value == U'.')
      {
        if (AtSegmentStart)
        {
          return false;
        }
        AtSegmentStart = true;
        HasPackageSeparator = true;
        Position += Decoded.Length;
        continue;
      }
      if (AtSegmentStart ? !isSegmentStart(Decoded.Value) : !isSegmentContinue(Decoded.Value))
      {
        return false;
      }
      AtSegmentStart = false;
      Position += Decoded.Length;
    }
    return !AtSegmentStart && HasPackageSeparator;
  }
} // namespace ink::ir
