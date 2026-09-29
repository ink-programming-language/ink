#ifndef INK_CORE_ARCHIVE_TEXT_H
#define INK_CORE_ARCHIVE_TEXT_H

#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::core::archive
{
  inline int hexDigit(char C)
  {
    if (C >= '0' && C <= '9')
    {
      return C - '0';
    }
    if (C >= 'a' && C <= 'f')
    {
      return C - 'a' + 10;
    }
    if (C >= 'A' && C <= 'F')
    {
      return C - 'A' + 10;
    }
    return -1;
  }

  inline std::string quote(std::string_view Value)
  {
    constexpr char Hex[] = "0123456789abcdef";
    std::string Result = "\"";
    for (unsigned char C : Value)
    {
      switch (C)
      {
      case '\\':
        Result += "\\\\";
        break;
      case '"':
        Result += "\\\"";
        break;
      case '\n':
        Result += "\\n";
        break;
      case '\r':
        Result += "\\r";
        break;
      case '\t':
        Result += "\\t";
        break;
      default:
        if (C < 32 || C == 127)
        {
          Result += "\\x";
          Result += Hex[C >> 4];
          Result += Hex[C & 15];
        }
        else
        {
          Result += static_cast<char>(C);
        }
      }
    }
    return Result + '"';
  }

  // Streaming lexer: tokens borrow the input, and only decoded strings allocate.
  class Lexer
  {
    public:
      explicit Lexer(std::string_view Input)
          : Input(Input)
      {
        next();
      }

      bool good() const
      {
        return Error.empty();
      }
      std::string message() const
      {
        return Error + " at byte " + std::to_string(Start);
      }
      std::string_view token() const
      {
        return Token;
      }
      bool quoted() const
      {
        return !Token.empty() && Token.front() == '"';
      }
      std::size_t offset() const
      {
        return Start;
      }

      bool fail(std::string_view Reason)
      {
        if (good())
        {
          Error = Reason;
        }
        return false;
      }

      void next()
      {
        while (Position < Input.size())
        {
          const char C = Input[Position];
          if (C == ' ' || C == '\n' || C == '\r' || C == '\t')
          {
            ++Position;
          }
          else if (C == ';')
          {
            while (Position < Input.size() && Input[Position] != '\n')
            {
              ++Position;
            }
          }
          else
          {
            break;
          }
        }
        Start = Position;
        if (Position == Input.size())
        {
          Token = {};
          return;
        }
        const char C = Input[Position++];
        if (C == '"')
        {
          bool Closed = false;
          while (Position < Input.size())
          {
            const char Byte = Input[Position++];
            if (Byte == '"')
            {
              Closed = true;
              break;
            }
            if (Byte == '\\' && Position < Input.size())
            {
              ++Position;
            }
          }
          if (!Closed)
          {
            fail("Unterminated string");
          }
        }
        else if (wordChar(C))
        {
          while (Position < Input.size() && wordChar(Input[Position]))
          {
            ++Position;
          }
        }
        Token = Input.substr(Start, Position - Start);
      }

      bool take(std::string_view Expected)
      {
        if (good() && Token == Expected)
        {
          next();
          return true;
        }
        return false;
      }

      bool expect(std::string_view Expected)
      {
        return take(Expected) || fail("Expected '" + std::string(Expected) + "'");
      }

      std::uint64_t number()
      {
        std::uint64_t Value = 0;
        const auto Result = std::from_chars(Token.data(), Token.data() + Token.size(), Value);
        if (Token.empty() || Result.ec != std::errc{} || Result.ptr != Token.data() + Token.size())
        {
          fail("Expected unsigned integer");
          return 0;
        }
        next();
        return Value;
      }

      std::string string()
      {
        if (!good() || !quoted())
        {
          fail("Expected quoted string");
          return {};
        }
        std::string Result;
        for (std::size_t I = 1; I + 1 < Token.size(); ++I)
        {
          char C = Token[I];
          if (C == '\\')
          {
            C = Token[++I];
            switch (C)
            {
            case 'n':
              C = '\n';
              break;
            case 'r':
              C = '\r';
              break;
            case 't':
              C = '\t';
              break;
            case '\\':
            case '"':
              break;
            case 'x':
              if (I + 2 >= Token.size() - 1 || hexDigit(Token[I + 1]) < 0 || hexDigit(Token[I + 2]) < 0)
              {
                fail("Invalid hexadecimal escape");
                return {};
              }
              C = static_cast<char>((hexDigit(Token[I + 1]) << 4) | hexDigit(Token[I + 2]));
              I += 2;
              break;
            default:
              fail("Unknown string escape");
              return {};
            }
          }
          Result += C;
        }
        next();
        return Result;
      }

      // Used for independently versioned AST sections; quotes and comments hide braces.
      std::string_view block(std::size_t MaxDepth)
      {
        const auto Begin = Start;
        if (!expect("{"))
        {
          return {};
        }
        std::size_t Depth = 1;
        while (good() && !Token.empty())
        {
          if (Token == "{" && ++Depth > MaxDepth)
          {
            fail("Text nesting limit exceeded");
            break;
          }
          if (Token == "}" && --Depth == 0)
          {
            const auto End = Position;
            next();
            return Input.substr(Begin, End - Begin);
          }
          next();
        }
        fail("Unterminated block");
        return {};
      }

    private:
      static bool wordChar(char C)
      {
        return (C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') || (C >= '0' && C <= '9') || C == '_' || C == '.' || C == '-';
      }

      std::string_view Input;
      std::string_view Token;
      std::size_t Position = 0;
      std::size_t Start = 0;
      std::string Error;
  };
} // namespace ink::core::archive

#endif
