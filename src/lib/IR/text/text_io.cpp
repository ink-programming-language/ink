#include "text_io.h"

#include <charconv>
#include <limits>
#include <utility>

namespace ink::ir::text
{
  namespace
  {
    constexpr std::size_t MaxExpressionDepth = 128;

    bool isSpace(char Value) noexcept
    {
      return Value == ' ' || Value == '\t' || Value == '\n' || Value == '\r' || Value == '\f' || Value == '\v';
    }

    bool isWord(char Value) noexcept
    {
      return (Value >= 'a' && Value <= 'z') || (Value >= 'A' && Value <= 'Z') || (Value >= '0' && Value <= '9') || Value == '_' || Value == '.' || Value == '$';
    }

    int hexDigit(char Value) noexcept
    {
      if (Value >= '0' && Value <= '9')
      {
        return Value - '0';
      }
      if (Value >= 'a' && Value <= 'f')
      {
        return Value - 'a' + 10;
      }
      if (Value >= 'A' && Value <= 'F')
      {
        return Value - 'A' + 10;
      }
      return -1;
    }

    bool readUnsigned(std::string_view Text, std::uint64_t &Value)
    {
      int Base = 10;
      if (Text.starts_with("0x") || Text.starts_with("0X"))
      {
        Base = 16;
        Text.remove_prefix(2);
      }
      if (Text.empty())
      {
        return false;
      }
      const auto Result = std::from_chars(Text.data(), Text.data() + Text.size(), Value, Base);
      return Result.ec == std::errc{} && Result.ptr == Text.data() + Text.size();
    }
  } // namespace

  TextReader::TextReader(std::string_view Input)
  {
    lex(Input);
  }

  void TextReader::lex(std::string_view Input)
  {
    std::size_t Position = 0;
    if (Input.size() > core::SourceLocation::MaxByteOffset)
    {
      lexError(0, "IR text exceeds the supported source offset range");
    }
    while (Position < Input.size() && !Failure)
    {
      const char Character = Input[Position];
      if (isSpace(Character))
      {
        ++Position;
        continue;
      }
      if (Input.substr(Position, 2) == "//")
      {
        while (Position < Input.size() && Input[Position] != '\n' && Input[Position] != '\r')
        {
          ++Position;
        }
        continue;
      }
      const std::size_t Start = Position;
      if (Character == '"')
      {
        ++Position;
        std::string Decoded;
        bool Closed = false;
        while (Position < Input.size())
        {
          const unsigned char Byte = static_cast<unsigned char>(Input[Position++]);
          if (Byte == '"')
          {
            Closed = true;
            break;
          }
          if (Byte < 0x20 || Byte == 0x7f)
          {
            lexError(Position - 1, "control bytes in quoted text must be escaped");
            break;
          }
          if (Byte != '\\')
          {
            Decoded.push_back(static_cast<char>(Byte));
            continue;
          }
          if (Position + 1 < Input.size() && hexDigit(Input[Position]) >= 0 && hexDigit(Input[Position + 1]) >= 0)
          {
            Decoded.push_back(static_cast<char>((hexDigit(Input[Position]) << 4) | hexDigit(Input[Position + 1])));
            Position += 2;
            continue;
          }
          if (Position < Input.size() && (Input[Position] == '\\' || Input[Position] == '"'))
          {
            Decoded.push_back(Input[Position++]);
            continue;
          }
          lexError(Position - 1, "expected a two-digit hexadecimal byte escape");
          break;
        }
        if (!Failure && !Closed)
        {
          lexError(Start, "unterminated quoted text");
        }
        if (!Failure)
        {
          Tokens.push_back({TokenKind::String, std::move(Decoded), Start});
        }
        continue;
      }
      if (Character == '%' || Character == '@')
      {
        ++Position;
        while (Position < Input.size() && isWord(Input[Position]))
        {
          ++Position;
        }
        if (Position == Start + 1)
        {
          lexError(Start, "expected an identifier after '%' or '@'");
          break;
        }
        Tokens.push_back({TokenKind::Reference, std::string(Input.substr(Start, Position - Start)), Start});
        continue;
      }
      if (isWord(Character))
      {
        while (Position < Input.size() && isWord(Input[Position]))
        {
          ++Position;
        }
        Tokens.push_back({TokenKind::Word, std::string(Input.substr(Start, Position - Start)), Start});
        continue;
      }
      if (Input.substr(Position, 2) == "->")
      {
        Tokens.push_back({TokenKind::Punctuation, "->", Position});
        Position += 2;
        continue;
      }
      if (std::string_view("{}()[]<>,:;=").find(Character) != std::string_view::npos)
      {
        Tokens.push_back({TokenKind::Punctuation, std::string(1, Character), Position++});
        continue;
      }
      lexError(Position, "unrecognized byte in IR text");
    }
    Tokens.push_back({TokenKind::End, {}, Position});
  }

  bool TextReader::lexError(std::size_t Offset, std::string_view Description)
  {
    if (!Failure)
    {
      Failure = text::error(Offset, Description);
    }
    return false;
  }

  const TextReader::Token &TextReader::current() const noexcept
  {
    return Tokens[Index];
  }

  bool TextReader::token(std::string_view Expected)
  {
    if (Failure)
    {
      return false;
    }
    if (!peek(Expected))
    {
      return fail(std::string("expected '") + std::string(Expected) + "'");
    }
    ++Index;
    return true;
  }

  bool TextReader::reference(std::string &Value)
  {
    if (Failure || current().Kind != TokenKind::Reference)
    {
      return fail("expected a textual value/type/block identifier");
    }
    Value = current().Text;
    ++Index;
    return true;
  }

  bool TextReader::quoted(std::string &Value)
  {
    if (Failure || current().Kind != TokenKind::String)
    {
      return fail("expected quoted bytes");
    }
    Value = current().Text;
    ++Index;
    return true;
  }

  bool TextReader::number(std::uint64_t &Value)
  {
    if (Failure || current().Kind != TokenKind::Word || !readUnsigned(current().Text, Value))
    {
      return fail("expected an unsigned 64-bit decimal or hexadecimal integer");
    }
    ++Index;
    return true;
  }

  bool TextReader::expressionKind(Expression &Node)
  {
    if (Failure)
    {
      return false;
    }
    Node.Offset = current().Offset;
    if (current().Kind == TokenKind::Reference)
    {
      Node.Kind = ExpressionKind::Reference;
      return reference(Node.Text);
    }
    const std::string_view Word = current().Text;
    if (current().Kind == TokenKind::Word && Word.size() >= 2 && (Word[0] == 'i' || Word[0] == 'u' || Word[0] == 'f'))
    {
      std::uint64_t Width = 0;
      if (readUnsigned(Word.substr(1), Width))
      {
        const bool Float = Word[0] == 'f';
        if ((Float && Width != 32 && Width != 64) || (!Float && Width != 8 && Width != 16 && Width != 32 && Width != 64))
        {
          return fail("unsupported primitive type width");
        }
        Node.Kind = Float ? ExpressionKind::FloatType : ExpressionKind::IntegerType;
        Node.Width = static_cast<std::uint32_t>(Width);
        Node.Signed = Word[0] != 'u';
        ++Index;
        return true;
      }
    }
    return enumValue(Node.Kind, ExpressionKinds);
  }

  bool TextReader::resultId(std::string &Value)
  {
    Value.clear();
    if (current().Kind == TokenKind::Reference)
    {
      return reference(Value) && token("=");
    }
    return !Failure;
  }

  bool TextReader::beginRegion()
  {
    return token("{");
  }

  bool TextReader::endRegion()
  {
    return token("}");
  }

  bool TextReader::newline() noexcept
  {
    return !Failure;
  }

  bool TextReader::indent() noexcept
  {
    return !Failure;
  }

  bool TextReader::unindent() noexcept
  {
    return !Failure;
  }

  bool TextReader::peek(std::string_view Value) const noexcept
  {
    return !Failure && current().Kind != TokenKind::String && current().Kind != TokenKind::End && current().Text == Value;
  }

  bool TextReader::atBlock() const noexcept
  {
    return !Failure && current().Kind == TokenKind::Reference && Index + 1 < Tokens.size() && Tokens[Index + 1].Kind == TokenKind::Word && Tokens[Index + 1].Text == "name";
  }

  bool TextReader::done()
  {
    return !Failure && (current().Kind == TokenKind::End || fail("unexpected tokens after module"));
  }

  bool TextReader::fail(std::string_view Description)
  {
    return lexError(current().Offset, Description);
  }

  bool TextReader::enterExpression()
  {
    if (ExpressionDepth >= MaxExpressionDepth)
    {
      return fail("type/value expression nesting exceeds 128 levels");
    }
    ++ExpressionDepth;
    return !Failure;
  }

  void TextReader::leaveExpression() noexcept
  {
    --ExpressionDepth;
  }

  void TextReader::offset(std::size_t &Value) const noexcept
  {
    Value = current().Offset;
  }

  std::size_t TextReader::position() const noexcept
  {
    return Index;
  }

  const std::optional<core::Diagnostic> &TextReader::error() const noexcept
  {
    return Failure;
  }

  bool TextWriter::token(std::string_view Value)
  {
    if (Failure)
    {
      return false;
    }
    if (LineStart)
    {
      Output.append(Indent * 2, ' ');
      LineStart = false;
    }
    else
    {
      const bool TightBefore = Value == "," || Value == ";" || Value == ":" || Value == ")" || Value == "]" || Value == ">" || Value == "(" || Value == "<";
      const bool TightAfter = Previous == "(" || Previous == "[" || Previous == "<";
      if (!TightBefore && !TightAfter)
      {
        Output.push_back(' ');
      }
    }
    Output.append(Value);
    Previous = Value;
    return true;
  }

  bool TextWriter::reference(std::string &Value)
  {
    if (Value.size() < 2 || (Value[0] != '%' && Value[0] != '@'))
    {
      return fail("cannot print an invalid textual identifier");
    }
    return token(Value);
  }

  bool TextWriter::quoted(std::string &Value)
  {
    constexpr char Hex[] = "0123456789ABCDEF";
    std::string Encoded = "\"";
    for (unsigned char Byte : Value)
    {
      if (Byte < 0x20 || Byte >= 0x7f || Byte == '"' || Byte == '\\')
      {
        Encoded.push_back('\\');
        Encoded.push_back(Hex[Byte >> 4]);
        Encoded.push_back(Hex[Byte & 0xf]);
      }
      else
      {
        Encoded.push_back(static_cast<char>(Byte));
      }
    }
    Encoded.push_back('"');
    return token(Encoded);
  }

  bool TextWriter::number(std::uint64_t &Value)
  {
    return token(std::to_string(Value));
  }

  bool TextWriter::expressionKind(Expression &Node)
  {
    switch (Node.Kind)
    {
    case ExpressionKind::Reference:
      return reference(Node.Text);
    case ExpressionKind::IntegerType:
      return token(std::string(Node.Signed ? "i" : "u") + std::to_string(Node.Width));
    case ExpressionKind::FloatType:
      return token("f" + std::to_string(Node.Width));
    case ExpressionKind::MetaType:
    case ExpressionKind::BoolType:
    case ExpressionKind::VoidType:
    case ExpressionKind::PointerType:
    case ExpressionKind::ArrayType:
    case ExpressionKind::FunctionType:
    case ExpressionKind::IntegerConstant:
    case ExpressionKind::FloatConstant:
    case ExpressionKind::StringConstant:
      return enumValue(Node.Kind, ExpressionKinds);
    }
    return fail("unknown type/value expression");
  }

  bool TextWriter::resultId(std::string &Value)
  {
    return Value.empty() || (reference(Value) && token("="));
  }

  bool TextWriter::beginRegion()
  {
    if (!newline() || !token("{") || !newline())
    {
      return false;
    }
    ++Indent;
    return true;
  }

  bool TextWriter::endRegion()
  {
    if (Indent == 0)
    {
      return fail("unbalanced output region");
    }
    --Indent;
    return newline() && token("}") && newline();
  }

  bool TextWriter::newline()
  {
    if (!LineStart)
    {
      Output.push_back('\n');
      LineStart = true;
      Previous.clear();
    }
    return !Failure;
  }

  bool TextWriter::indent() noexcept
  {
    ++Indent;
    return !Failure;
  }

  bool TextWriter::unindent()
  {
    if (Indent == 0)
    {
      return fail("unbalanced output indentation");
    }
    --Indent;
    return !Failure;
  }

  bool TextWriter::fail(std::string_view Description)
  {
    if (!Failure)
    {
      Failure = text::error(0, Description);
    }
    return false;
  }

  bool TextWriter::enterExpression()
  {
    if (ExpressionDepth >= MaxExpressionDepth)
    {
      return fail("type/value expression nesting exceeds 128 levels");
    }
    ++ExpressionDepth;
    return !Failure;
  }

  void TextWriter::leaveExpression() noexcept
  {
    --ExpressionDepth;
  }

  void TextWriter::offset(std::size_t &) const noexcept
  {
  }

  const std::optional<core::Diagnostic> &TextWriter::error() const noexcept
  {
    return Failure;
  }

  std::string TextWriter::take()
  {
    return std::move(Output);
  }
} // namespace ink::ir::text
