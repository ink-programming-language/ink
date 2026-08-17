#include "ink/ir/model/name.h"

#include "ink/tokenizer/unicode.h"

#include <ostream>

namespace ink::ir
{
  bool Name::valid() const noexcept
  {
    if (Text.empty())
    {
      return false;
    }
    std::size_t Position = 0;
    tokenizer::unicode::DecodeResult Decoded = tokenizer::unicode::decode(Text, Position);
    if (!Decoded.Valid || !isStartCharacter(Decoded.Value))
    {
      return false;
    }
    Position += Decoded.Length;
    while (Position < Text.size())
    {
      Decoded = tokenizer::unicode::decode(Text, Position);
      if (!Decoded.Valid || !isContinueCharacter(Decoded.Value))
      {
        return false;
      }
      Position += Decoded.Length;
    }
    return true;
  }

  bool Name::isStartCharacter(char32_t Character) noexcept
  {
    return tokenizer::unicode::isXidStart(Character) || Character == U'_' || Character == U'.' || Character == U'$';
  }

  bool Name::isContinueCharacter(char32_t Character) noexcept
  {
    return tokenizer::unicode::isXidContinue(Character) || Character == U'_' || Character == U'.' || Character == U'$';
  }

  bool operator==(const Name &Left, const Name &Right) noexcept
  {
    return Left.text() == Right.text();
  }

  bool operator!=(const Name &Left, const Name &Right) noexcept
  {
    return !(Left == Right);
  }

  std::ostream &operator<<(std::ostream &Output, const Name &Value)
  {
    return Output << Value.text();
  }
} // namespace ink::ir
