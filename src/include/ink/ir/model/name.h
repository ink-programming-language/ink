#ifndef INK_IR_NAME_H
#define INK_IR_NAME_H

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <string>
#include <string_view>
#include <utility>

namespace ink::ir
{
  class Name
  {
    public:
      Name() = default;

      Name(const char *Text)
          : Text(Text)
      {
      }

      Name(std::string Text)
          : Text(std::move(Text))
      {
      }

      Name(std::string_view Text)
          : Text(Text)
      {
      }

      bool empty() const noexcept
      {
        return Text.empty();
      }

      bool valid() const noexcept;

      const std::string &str() const noexcept
      {
        return Text;
      }

      std::string_view text() const noexcept
      {
        return Text;
      }

      operator std::string_view() const noexcept
      {
        return Text;
      }

      static bool isStartCharacter(char32_t Character) noexcept;
      static bool isContinueCharacter(char32_t Character) noexcept;

    private:
      std::string Text;
  };

  bool operator==(const Name &Left, const Name &Right) noexcept;
  bool operator!=(const Name &Left, const Name &Right) noexcept;
  std::ostream &operator<<(std::ostream &Output, const Name &Value);
} // namespace ink::ir

namespace std
{
  template <>
  struct hash<ink::ir::Name>
  {
      std::size_t operator()(const ink::ir::Name &Value) const noexcept
      {
        return hash<std::string_view>{}(Value.text());
      }
  };
} // namespace std

#endif
