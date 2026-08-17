#include "ink/ir/serialization.h"

#include "ink/ir/arithmetic.h"
#include "ink/ir/control_flow.h"
#include "ink/ir/memory.h"
#include "ink/ir/verifier.h"

#include <cctype>
#include <charconv>
#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ir
{
  using core::Diagnostic;
  using core::DiagnosticKind;
  using core::SourceRange;

  namespace
  {
    enum class TokenKind
    {
      Identifier,
      GlobalName,
      TypeName,
      ValueName,
      Integer,
      Hexadecimal,
      String,
      LeftParenthesis,
      RightParenthesis,
      LeftBrace,
      RightBrace,
      LeftBracket,
      RightBracket,
      Comma,
      Colon,
      Equal,
      Star,
      End,
    };

    struct Token
    {
      TokenKind Kind = TokenKind::End;
      std::string Text;
      SourceRange Span;
    };

    bool isNameStart(char Character)
    {
      const unsigned char Value = static_cast<unsigned char>(Character);
      return std::isalpha(Value) != 0 || Character == '_' || Character == '.' || Character == '$';
    }

    bool isNameContinue(char Character)
    {
      const unsigned char Value = static_cast<unsigned char>(Character);
      return std::isalnum(Value) != 0 || Character == '_' || Character == '.' || Character == '$';
    }

    int hexadecimalValue(char Character)
    {
      if (Character >= '0' && Character <= '9')
      {
        return Character - '0';
      }
      if (Character >= 'a' && Character <= 'f')
      {
        return Character - 'a' + 10;
      }
      if (Character >= 'A' && Character <= 'F')
      {
        return Character - 'A' + 10;
      }
      return -1;
    }

    class Lexer
    {
    public:
      explicit Lexer(std::string_view Text)
          : Text(Text)
      {
      }

      bool tokenize(std::vector<Token> &Tokens, Diagnostic &Error)
      {
        while (true)
        {
          skipTrivia();
          if (Position == Text.size())
          {
            Tokens.push_back({TokenKind::End, {}, {Position, Position}});
            return true;
          }

          const std::size_t Start = Position;
          const char Character = Text[Position];
          if (isNameStart(Character))
          {
            Tokens.push_back(readName(TokenKind::Identifier));
            continue;
          }
          if (Character == '@')
          {
            advance();
            if (Position == Text.size() || !isNameStart(Text[Position]))
            {
              Error = core::makeDiagnostic<DiagnosticKind::IrExpectedGlobalNameAfterAt>({Start, Position});
              return false;
            }
            Token Name = readName(TokenKind::GlobalName);
            Name.Span.Start = Start;
            Tokens.push_back(std::move(Name));
            continue;
          }
          if (Character == '%')
          {
            advance();
            if (Position < Text.size() && isNameStart(Text[Position]))
            {
              Token Name = readName(TokenKind::TypeName);
              Name.Span.Start = Start;
              Tokens.push_back(std::move(Name));
              continue;
            }
            const std::size_t DigitsStart = Position;
            while (Position < Text.size() && std::isdigit(static_cast<unsigned char>(Text[Position])) != 0)
            {
              advance();
            }
            if (DigitsStart == Position)
            {
              Error = core::makeDiagnostic<DiagnosticKind::IrExpectedTypeOrSsaNameAfterPercent>({Start, Position});
              return false;
            }
            Tokens.push_back({TokenKind::ValueName, std::string(Text.substr(DigitsStart, Position - DigitsStart)), {Start, Position}});
            continue;
          }
          if (Character == '0' && Position + 1 < Text.size() && (Text[Position + 1] == 'x' || Text[Position + 1] == 'X'))
          {
            advance();
            advance();
            const std::size_t DigitsStart = Position;
            while (Position < Text.size() && std::isxdigit(static_cast<unsigned char>(Text[Position])) != 0)
            {
              advance();
            }
            if (DigitsStart == Position)
            {
              Error = core::makeDiagnostic<DiagnosticKind::IrFloatBitPatternRequiresDigits>({Start, Position});
              return false;
            }
            Tokens.push_back({TokenKind::Hexadecimal, std::string(Text.substr(DigitsStart, Position - DigitsStart)), {Start, Position}});
            continue;
          }
          if (Character == '-' || std::isdigit(static_cast<unsigned char>(Character)) != 0)
          {
            if (Character == '-' && (Position + 1 == Text.size() || std::isdigit(static_cast<unsigned char>(Text[Position + 1])) == 0))
            {
              Error = core::makeDiagnostic<DiagnosticKind::IrMinusRequiresDecimalInteger>({Start, Start + 1});
              return false;
            }
            advance();
            while (Position < Text.size() && std::isdigit(static_cast<unsigned char>(Text[Position])) != 0)
            {
              advance();
            }
            Tokens.push_back({TokenKind::Integer, std::string(Text.substr(Start, Position - Start)), {Start, Position}});
            continue;
          }
          if (Character == '"')
          {
            Token StringToken;
            if (!readString(StringToken, Error))
            {
              return false;
            }
            Tokens.push_back(std::move(StringToken));
            continue;
          }

          TokenKind Kind;
          switch (Character)
          {
          case '(':
            Kind = TokenKind::LeftParenthesis;
            break;
          case ')':
            Kind = TokenKind::RightParenthesis;
            break;
          case '{':
            Kind = TokenKind::LeftBrace;
            break;
          case '}':
            Kind = TokenKind::RightBrace;
            break;
          case '[':
            Kind = TokenKind::LeftBracket;
            break;
          case ']':
            Kind = TokenKind::RightBracket;
            break;
          case ',':
            Kind = TokenKind::Comma;
            break;
          case ':':
            Kind = TokenKind::Colon;
            break;
          case '=':
            Kind = TokenKind::Equal;
            break;
          case '*':
            Kind = TokenKind::Star;
            break;
          default:
            Error = core::makeDiagnostic<DiagnosticKind::IrUnexpectedCharacter>({Start, Start + 1}, std::string(1, Character));
            return false;
          }
          advance();
          Tokens.push_back({Kind, std::string(1, Character), {Start, Position}});
        }
      }

    private:
      void advance()
      {
        ++Position;
      }

      void skipTrivia()
      {
        while (Position < Text.size())
        {
          if (std::isspace(static_cast<unsigned char>(Text[Position])) != 0)
          {
            advance();
            continue;
          }
          if (Text[Position] == ';')
          {
            while (Position < Text.size() && Text[Position] != '\n')
            {
              advance();
            }
            continue;
          }
          break;
        }
      }

      Token readName(TokenKind Kind)
      {
        const std::size_t Start = Position;
        advance();
        while (Position < Text.size() && isNameContinue(Text[Position]))
        {
          advance();
        }
        return {Kind, std::string(Text.substr(Start, Position - Start)), {Start, Position}};
      }

      bool readString(Token &Result, Diagnostic &Error)
      {
        const std::size_t Start = Position;
        advance();
        std::string Value;
        while (Position < Text.size() && Text[Position] != '"')
        {
          const char Character = Text[Position];
          if (Character == '\n' || Character == '\r')
          {
            Error = core::makeDiagnostic<DiagnosticKind::IrStringLiteralRawLineBreak>({Start, Position + 1});
            return false;
          }
          if (Character != '\\')
          {
            Value.push_back(Character);
            advance();
            continue;
          }

          advance();
          if (Position == Text.size())
          {
            Error = core::makeDiagnostic<DiagnosticKind::IrUnterminatedStringEscape>({Start, Position});
            return false;
          }
          const int FirstHexadecimal = hexadecimalValue(Text[Position]);
          if (FirstHexadecimal >= 0)
          {
            if (Position + 1 >= Text.size())
            {
              Error = core::makeDiagnostic<DiagnosticKind::IrInvalidHexByteEscape>({Position, Text.size()});
              return false;
            }
            const int SecondHexadecimal = hexadecimalValue(Text[Position + 1]);
            if (SecondHexadecimal < 0)
            {
              Error = core::makeDiagnostic<DiagnosticKind::IrInvalidHexByteEscape>({Position, Position + 2});
              return false;
            }
            Value.push_back(static_cast<char>((FirstHexadecimal << 4) | SecondHexadecimal));
            advance();
            advance();
            continue;
          }

          switch (Text[Position])
          {
          case 'n':
            Value.push_back('\n');
            break;
          case 'r':
            Value.push_back('\r');
            break;
          case 't':
            Value.push_back('\t');
            break;
          case '\\':
            Value.push_back('\\');
            break;
          case '"':
            Value.push_back('"');
            break;
          default:
            Error = core::makeDiagnostic<DiagnosticKind::IrUnknownStringEscape>({Position, Position + 1}, std::string(1, Text[Position]));
            return false;
          }
          advance();
        }
        if (Position == Text.size())
        {
          Error = core::makeDiagnostic<DiagnosticKind::IrUnterminatedStringLiteral>({Start, Position});
          return false;
        }
        advance();
        Result = {TokenKind::String, std::move(Value), {Start, Position}};
        return true;
      }

      std::string_view Text;
      std::size_t Position = 0;
    };

    struct CallFixup
    {
      std::size_t FunctionIndex = 0;
      std::size_t BlockIndex = 0;
      std::size_t InstructionIndex = 0;
      std::string CalleeName;
      Token Location;
    };

    enum class BranchFixupTarget
    {
      Branch,
      ConditionalTrue,
      ConditionalFalse,
    };

    struct BranchFixup
    {
      std::size_t FunctionIndex = 0;
      std::size_t BlockIndex = 0;
      std::size_t InstructionIndex = 0;
      BranchFixupTarget Target = BranchFixupTarget::Branch;
      std::string BlockName;
      Token Location;
    };

    struct PhiFixup
    {
      std::size_t FunctionIndex = 0;
      std::size_t BlockIndex = 0;
      std::size_t InstructionIndex = 0;
      std::size_t IncomingIndex = 0;
      std::string BlockName;
      Token Location;
    };

    struct GlobalFixup
    {
      GlobalAddressOperand *Address = nullptr;
      std::string GlobalName;
      Token Location;
    };

    struct GlobalVariableFixup
    {
      GlobalVariableAddressOperand *Address = nullptr;
      std::string GlobalName;
      Token Location;
    };

    struct ParsedOperand
    {
      std::unique_ptr<Value> ParsedValue;
      Token Location;
    };

    class TextParser
    {
    public:
      TextParser(IRContext &Context, std::vector<Token> Tokens)
          : Context(Context),
            Tokens(std::move(Tokens)),
            ModuleValue(Context)
      {
      }

      bool parse(Module &Result, Diagnostic &Error)
      {
        if (!parseModule())
        {
          Error = std::move(ParseDiagnostic);
          return false;
        }
        Result = std::move(ModuleValue);
        return true;
      }

    private:
      bool parseModule()
      {
        if (!expectIdentifier("inkir"))
        {
          return false;
        }
        const Token *Version = expect(TokenKind::Integer, "IR format version");
        if (Version == nullptr)
        {
          return false;
        }
        const std::optional<std::uint64_t> VersionNumber = parseUnsigned(*Version, "IR format version");
        if (!VersionNumber)
        {
          return false;
        }
        if (*VersionNumber != 1)
        {
          return fail<DiagnosticKind::IrUnsupportedFormatVersion>(*Version, *VersionNumber, std::uint64_t{1});
        }

        while (!at(TokenKind::End))
        {
          if (atIdentifier("module"))
          {
            if (!parseModuleDeclaration())
            {
              return false;
            }
          }
          else if (atIdentifier("initializer"))
          {
            if (!parseLifecycleFunction(true))
            {
              return false;
            }
          }
          else if (atIdentifier("finalizer"))
          {
            if (!parseLifecycleFunction(false))
            {
              return false;
            }
          }
          else if (at(TokenKind::TypeName))
          {
            if (!parseStructType())
            {
              return false;
            }
          }
          else if (at(TokenKind::GlobalName))
          {
            if (atIdentifier("global", 2))
            {
              if (!parseGlobalVariable())
              {
                return false;
              }
            }
            else if (!parseByteConstant())
            {
              return false;
            }
          }
          else if (atIdentifier("declare"))
          {
            if (!parseExternalFunction())
            {
              return false;
            }
          }
          else if (atIdentifier("define"))
          {
            if (!parseFunctionDefinition())
            {
              return false;
            }
          }
          else
          {
            return fail<DiagnosticKind::IrExpectedTopLevelDeclaration>(current());
          }
        }
        return resolveReferences();
      }

      bool parseModuleDeclaration()
      {
        const Token Keyword = consume();
        if (HasModuleDeclaration)
        {
          return fail<DiagnosticKind::IrExpected>(Keyword, "only one module declaration");
        }
        const Token *Id = expect(TokenKind::Integer, "a module id");
        if (Id == nullptr)
        {
          return false;
        }
        const std::optional<std::size_t> ParsedId = parseId(*Id, "module id");
        if (!ParsedId)
        {
          return false;
        }
        ModuleValue.Id = ModuleId{*ParsedId};
        HasModuleDeclaration = true;
        return true;
      }

      bool parseLifecycleFunction(bool IsInitializer)
      {
        const Token Keyword = consume();
        std::optional<std::string> &Name = IsInitializer ? InitializerName : FinalizerName;
        Token &Location = IsInitializer ? InitializerLocation : FinalizerLocation;
        if (Name.has_value())
        {
          return fail<DiagnosticKind::IrExpected>(Keyword, IsInitializer ? "only one initializer declaration" : "only one finalizer declaration");
        }
        const Token *FunctionName = expect(TokenKind::GlobalName, "a lifecycle function name");
        if (FunctionName == nullptr)
        {
          return false;
        }
        Name = FunctionName->Text;
        Location = *FunctionName;
        return true;
      }

      const Token &current(std::size_t Offset = 0) const
      {
        const std::size_t Target = Index + Offset;
        return Tokens[Target < Tokens.size() ? Target : Tokens.size() - 1];
      }

      bool at(TokenKind Kind, std::size_t Offset = 0) const
      {
        return current(Offset).Kind == Kind;
      }

      bool atIdentifier(std::string_view Name, std::size_t Offset = 0) const
      {
        return at(TokenKind::Identifier, Offset) && current(Offset).Text == Name;
      }

      const Token &consume()
      {
        return Tokens[Index++];
      }

      const Token *expect(TokenKind Kind, std::string_view Expected)
      {
        if (!at(Kind))
        {
          fail<DiagnosticKind::IrExpected>(current(), Expected);
          return nullptr;
        }
        return &consume();
      }

      bool expectIdentifier(std::string_view Name)
      {
        if (!atIdentifier(Name))
        {
          return fail<DiagnosticKind::IrExpectedIdentifier>(current(), Name);
        }
        consume();
        return true;
      }

      template <DiagnosticKind Kind, typename... ArgumentTypes>
      bool fail(const Token &Location, ArgumentTypes &&...Arguments)
      {
        ParseDiagnostic = core::makeDiagnostic<Kind>(Location.Span, std::forward<ArgumentTypes>(Arguments)...);
        return false;
      }

      std::optional<std::uint64_t> parseUnsigned(const Token &TokenValue, std::string_view Description)
      {
        if (!TokenValue.Text.empty() && TokenValue.Text.front() == '-')
        {
          fail<DiagnosticKind::IrNegativeNumericValue>(TokenValue, Description);
          return std::nullopt;
        }
        std::uint64_t Result = 0;
        const char *Begin = TokenValue.Text.data();
        const char *End = Begin + TokenValue.Text.size();
        const auto Conversion = std::from_chars(Begin, End, Result);
        if (Conversion.ec != std::errc() || Conversion.ptr != End)
        {
          fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
          return std::nullopt;
        }
        return Result;
      }

      std::optional<std::int64_t> parseSigned(const Token &TokenValue, std::string_view Description)
      {
        std::int64_t Result = 0;
        const char *Begin = TokenValue.Text.data();
        const char *End = Begin + TokenValue.Text.size();
        const auto Conversion = std::from_chars(Begin, End, Result);
        if (Conversion.ec != std::errc() || Conversion.ptr != End)
        {
          fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
          return std::nullopt;
        }
        return Result;
      }

      std::optional<std::uint64_t> parseHexadecimal(const Token &TokenValue, std::string_view Description)
      {
        std::uint64_t Result = 0;
        const char *Begin = TokenValue.Text.data();
        const char *End = Begin + TokenValue.Text.size();
        const auto Conversion = std::from_chars(Begin, End, Result, 16);
        if (Conversion.ec != std::errc() || Conversion.ptr != End)
        {
          fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
          return std::nullopt;
        }
        return Result;
      }

      std::optional<FloatFormat> parseFloatFormat()
      {
        const Token *Format = expect(TokenKind::Identifier, "a floating-point format");
        if (Format == nullptr)
        {
          return std::nullopt;
        }
        if (Format->Text == "f16")
        {
          return FloatFormat::F16;
        }
        if (Format->Text == "f32")
        {
          return FloatFormat::F32;
        }
        if (Format->Text == "f64")
        {
          return FloatFormat::F64;
        }
        fail<DiagnosticKind::IrUnknownFloatFormat>(*Format, Format->Text);
        return std::nullopt;
      }

      std::optional<std::size_t> parseIndex(const Token &TokenValue, std::string_view Description)
      {
        const std::optional<std::uint64_t> Value = parseUnsigned(TokenValue, Description);
        if (!Value)
        {
          return std::nullopt;
        }
        if (*Value > std::numeric_limits<std::size_t>::max())
        {
          fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
          return std::nullopt;
        }
        return static_cast<std::size_t>(*Value);
      }

      std::optional<std::size_t> parseId(const Token &TokenValue, std::string_view Description)
      {
        const std::optional<std::size_t> Value = parseIndex(TokenValue, Description);
        if (!Value || *Value == InvalidId)
        {
          if (Value)
          {
            fail<DiagnosticKind::IrNumericValueOutOfRange>(TokenValue, Description);
          }
          return std::nullopt;
        }
        return Value;
      }

      std::optional<const Type *> parseType()
      {
        if (at(TokenKind::TypeName))
        {
          const Token &Name = consume();
          const auto Type = TypeNames.find(Name.Text);
          if (Type == TypeNames.end())
          {
            fail<DiagnosticKind::IrUnknownStructType>(Name, Name.Text);
            return std::nullopt;
          }
          return Type->second;
        }
        if (atIdentifier("const"))
        {
          consume();
          if (!expectIdentifier("byte"))
          {
            return std::nullopt;
          }
          if (at(TokenKind::Star))
          {
            consume();
            return &Context.getType(TypeKind::ConstBytePointer);
          }
          if (at(TokenKind::LeftBracket) && at(TokenKind::RightBracket, 1))
          {
            consume();
            consume();
            return &Context.getType(TypeKind::ConstByteSlice);
          }
          fail<DiagnosticKind::IrExpected>(current(), "'*' or '[]' after 'const byte'");
          return std::nullopt;
        }
        if (atIdentifier("byte") && at(TokenKind::Star, 1))
        {
          consume();
          consume();
          return &Context.getType(TypeKind::BytePointer);
        }
        if (atIdentifier("byte") && at(TokenKind::LeftBracket, 1) && at(TokenKind::RightBracket, 2))
        {
          consume();
          consume();
          consume();
          return &Context.getType(TypeKind::ByteSlice);
        }
        const Token *Type = expect(TokenKind::Identifier, "an IR type");
        if (Type == nullptr)
        {
          return std::nullopt;
        }
#define INK_IR_TYPE(Name, Spelling)          \
  if (Type->Text == Spelling)                \
  {                                          \
    return &Context.getType(TypeKind::Name); \
  }
#include "ink/ir/ir.def"
        fail<DiagnosticKind::IrUnknownType>(*Type, Type->Text);
        return std::nullopt;
      }

      bool parseStructType()
      {
        const Token *Name = expect(TokenKind::TypeName, "a struct type name");
        if (Name == nullptr)
        {
          return false;
        }
        if (TypeNames.find(Name->Text) != TypeNames.end())
        {
          return fail<DiagnosticKind::IrDuplicateStructType>(*Name, Name->Text);
        }
        if (expect(TokenKind::Equal, "'='") == nullptr || !expectIdentifier("type") || expect(TokenKind::LeftBrace, "'{'") == nullptr)
        {
          return false;
        }

        std::vector<const Type *> FieldTypes;
        if (!at(TokenKind::RightBrace))
        {
          const std::optional<const Type *> FirstField = parseType();
          if (!FirstField)
          {
            return false;
          }
          FieldTypes.push_back(*FirstField);
          while (at(TokenKind::Comma))
          {
            consume();
            const std::optional<const Type *> Field = parseType();
            if (!Field)
            {
              return false;
            }
            FieldTypes.push_back(*Field);
          }
        }
        if (expect(TokenKind::RightBrace, "'}'") == nullptr)
        {
          return false;
        }

        const StructType &TypeValue = Context.createStructType(Name->Text, std::move(FieldTypes));
        TypeNames.emplace(Name->Text, &TypeValue);
        ModuleValue.StructTypes.push_back(&TypeValue);
        return true;
      }

      bool reserveGlobalSymbol(const Token &Name)
      {
        if (!GlobalSymbolNames.insert(Name.Text).second)
        {
          return fail<DiagnosticKind::IrDuplicateGlobalSymbol>(Name, Name.Text);
        }
        return true;
      }

      bool parseByteConstant()
      {
        const Token *Name = expect(TokenKind::GlobalName, "a global symbol name");
        if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::Equal, "'='") == nullptr || !expectIdentifier("private") || !expectIdentifier("constant") || expect(TokenKind::LeftBracket, "'['") == nullptr)
        {
          return false;
        }
        const Token *SizeToken = expect(TokenKind::Integer, "a byte constant size");
        if (SizeToken == nullptr)
        {
          return false;
        }
        const std::optional<std::size_t> DeclaredSize = parseIndex(*SizeToken, "byte constant size");
        if (!DeclaredSize || !expectIdentifier("x") || !expectIdentifier("byte") || expect(TokenKind::RightBracket, "']'") == nullptr || !expectIdentifier("c"))
        {
          return false;
        }
        const Token *Data = expect(TokenKind::String, "a byte string literal");
        if (Data == nullptr)
        {
          return false;
        }
        if (Data->Text.size() != *DeclaredSize)
        {
          return fail<DiagnosticKind::IrByteConstantSizeMismatch>(*SizeToken, *DeclaredSize, Data->Text.size());
        }
        const ByteConstantId Id{ModuleValue.ByteConstants.size()};
        ByteConstantNames.emplace(Name->Text, Id);
        ModuleValue.ByteConstants.push_back({Name->Text, Data->Text});
        return true;
      }

      bool parseGlobalVariable()
      {
        const Token *Name = expect(TokenKind::GlobalName, "a global variable name");
        if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::Equal, "'='") == nullptr || !expectIdentifier("global"))
        {
          return false;
        }

        bool Mutable;
        if (atIdentifier("mutable"))
        {
          consume();
          Mutable = true;
        }
        else if (atIdentifier("constant"))
        {
          consume();
          Mutable = false;
        }
        else
        {
          return fail<DiagnosticKind::IrExpected>(current(), "'mutable' or 'constant'");
        }

        const std::optional<const Type *> ValueType = parseType();
        if (!ValueType)
        {
          return false;
        }
        const GlobalId Id{ModuleValue.Globals.size()};
        GlobalVariableNames.emplace(Name->Text, Id);
        ModuleValue.Globals.push_back({Name->Text, *ValueType, Mutable});
        return true;
      }

      std::optional<std::vector<const Type *>> parseExternalParameterTypes()
      {
        std::vector<const Type *> Result;
        if (at(TokenKind::RightParenthesis))
        {
          return Result;
        }
        const std::optional<const Type *> FirstType = parseType();
        if (!FirstType)
        {
          return std::nullopt;
        }
        Result.push_back(*FirstType);
        while (at(TokenKind::Comma))
        {
          consume();
          const std::optional<const Type *> Type = parseType();
          if (!Type)
          {
            return std::nullopt;
          }
          Result.push_back(*Type);
        }
        return Result;
      }

      bool parseExternalFunction()
      {
        if (!expectIdentifier("declare") || !expectIdentifier("extern"))
        {
          return false;
        }
        const Token *Convention = expect(TokenKind::String, "the external calling convention");
        if (Convention == nullptr)
        {
          return false;
        }
        if (Convention->Text != "C")
        {
          return fail<DiagnosticKind::IrUnsupportedCallingConvention>(*Convention, Convention->Text);
        }
        const std::optional<const Type *> ResultType = parseType();
        if (!ResultType)
        {
          return false;
        }
        Function FunctionValue(**ResultType);
        FunctionValue.Kind = FunctionKind::External;
        FunctionValue.Convention = CallingConvention::C;
        const Token *Name = expect(TokenKind::GlobalName, "an external function name");
        if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::LeftParenthesis, "'('") == nullptr)
        {
          return false;
        }
        FunctionValue.Name = Name->Text;
        std::optional<std::vector<const Type *>> ParameterTypes = parseExternalParameterTypes();
        if (!ParameterTypes || expect(TokenKind::RightParenthesis, "')'") == nullptr)
        {
          return false;
        }
        FunctionValue.ParameterTypes = std::move(*ParameterTypes);
        if (at(TokenKind::LeftBracket))
        {
          consume();
          if (!expectIdentifier("sideeffect") || expect(TokenKind::RightBracket, "']'") == nullptr)
          {
            return false;
          }
          FunctionValue.HasSideEffects = true;
        }
        const FunctionId Id{ModuleValue.Functions.size()};
        FunctionNames.emplace(FunctionValue.Name, Id);
        ModuleValue.Functions.push_back(std::move(FunctionValue));
        return true;
      }

      std::optional<std::vector<const Type *>> parseDefinitionParameterTypes()
      {
        std::vector<const Type *> Result;
        if (at(TokenKind::RightParenthesis))
        {
          return Result;
        }
        while (true)
        {
          const std::optional<const Type *> Type = parseType();
          if (!Type)
          {
            return std::nullopt;
          }
          Result.push_back(*Type);
          const Token *ValueName = expect(TokenKind::ValueName, "a function parameter SSA value");
          if (ValueName == nullptr)
          {
            return std::nullopt;
          }
          const std::optional<std::size_t> ParameterIndex = parseId(*ValueName, "function parameter SSA value");
          if (!ParameterIndex)
          {
            return std::nullopt;
          }
          if (*ParameterIndex != Result.size() - 1)
          {
            fail<DiagnosticKind::IrNonConsecutiveParameterSsa>(*ValueName, Result.size() - 1, *ParameterIndex);
            return std::nullopt;
          }
          if (!at(TokenKind::Comma))
          {
            break;
          }
          consume();
        }
        return Result;
      }

      bool startsBasicBlock() const
      {
        return at(TokenKind::Identifier) && at(TokenKind::Colon, 1);
      }

      std::optional<ParsedOperand> parseOperandValue(const Type &OperandType)
      {
        ParsedOperand Result;
        Result.Location = current();
        if (at(TokenKind::ValueName))
        {
          const Token &Value = consume();
          const std::optional<std::size_t> Id = parseId(Value, "SSA value");
          if (!Id)
          {
            return std::nullopt;
          }
          Result.ParsedValue = std::make_unique<ValueOperand>(OperandType, ValueId{*Id});
          return Result;
        }
        if (at(TokenKind::Integer))
        {
          const Token &Integer = consume();
          if (OperandType.kind() == TypeKind::PointerSize && (Integer.Text.empty() || Integer.Text.front() != '-'))
          {
            const std::optional<std::uint64_t> Value = parseUnsigned(Integer, "integer constant");
            if (!Value)
            {
              return std::nullopt;
            }
            Result.ParsedValue = std::make_unique<IntegerConstant>(OperandType, *Value);
            return Result;
          }
          const std::optional<std::int64_t> Value = parseSigned(Integer, "integer constant");
          if (!Value)
          {
            return std::nullopt;
          }
          Result.ParsedValue = std::make_unique<IntegerConstant>(OperandType, *Value);
          return Result;
        }
        if (atIdentifier("floatbits"))
        {
          consume();
          if (expect(TokenKind::LeftParenthesis, "'(' after 'floatbits'") == nullptr)
          {
            return std::nullopt;
          }
          const std::optional<FloatFormat> Format = parseFloatFormat();
          if (!Format || expect(TokenKind::Comma, "',' after a floating-point format") == nullptr)
          {
            return std::nullopt;
          }
          const Token *BitPatternToken = expect(TokenKind::Hexadecimal, "a hexadecimal floating-point bit pattern");
          if (BitPatternToken == nullptr)
          {
            return std::nullopt;
          }
          const std::size_t ExpectedDigits = floatFormatBitWidth(*Format) / 4;
          if (BitPatternToken->Text.size() != ExpectedDigits)
          {
            fail<DiagnosticKind::IrFloatBitPatternWidthMismatch>(*BitPatternToken, floatFormatName(*Format), ExpectedDigits, BitPatternToken->Text.size());
            return std::nullopt;
          }
          const std::optional<std::uint64_t> BitPattern = parseHexadecimal(*BitPatternToken, "floating-point bit pattern");
          if (!BitPattern || expect(TokenKind::RightParenthesis, "')' after a floating-point bit pattern") == nullptr)
          {
            return std::nullopt;
          }
          Result.ParsedValue = std::make_unique<FloatConstant>(OperandType, *Format, *BitPattern);
          return Result;
        }
        if (atIdentifier("c") && at(TokenKind::String, 1))
        {
          consume();
          Result.ParsedValue = std::make_unique<StringConstant>(OperandType, consume().Text);
          return Result;
        }
        if (atIdentifier("null"))
        {
          consume();
          Result.ParsedValue = std::make_unique<NullConstant>(OperandType);
          return Result;
        }
        if (at(TokenKind::LeftBrace))
        {
          consume();
          std::vector<std::unique_ptr<Value>> Elements;
          if (!at(TokenKind::RightBrace))
          {
            while (true)
            {
              std::optional<ParsedOperand> Element = parseOperand();
              if (!Element)
              {
                return std::nullopt;
              }
              Elements.push_back(std::move(Element->ParsedValue));
              if (!at(TokenKind::Comma))
              {
                break;
              }
              consume();
            }
          }
          if (expect(TokenKind::RightBrace, "'}' after an aggregate constant") == nullptr)
          {
            return std::nullopt;
          }
          Result.ParsedValue = std::make_unique<AggregateConstant>(OperandType, std::move(Elements));
          return Result;
        }
        if (atIdentifier("zeroinitializer"))
        {
          consume();
          Result.ParsedValue = std::make_unique<ZeroInitializer>(OperandType);
          return Result;
        }
        if (at(TokenKind::GlobalName))
        {
          const Token &Global = consume();
          if (!at(TokenKind::LeftBracket))
          {
            auto Address = std::make_unique<GlobalVariableAddressOperand>(OperandType, GlobalRef{});
            GlobalVariableFixups.push_back({Address.get(), Global.Text, Global});
            Result.ParsedValue = std::move(Address);
            Result.Location = Global;
            return Result;
          }
          consume();
          const Token *Offset = expect(TokenKind::Integer, "a global byte offset");
          if (Offset == nullptr)
          {
            return std::nullopt;
          }
          const std::optional<std::size_t> ByteOffset = parseIndex(*Offset, "global byte offset");
          if (!ByteOffset || expect(TokenKind::RightBracket, "']'") == nullptr)
          {
            return std::nullopt;
          }
          auto Address = std::make_unique<GlobalAddressOperand>(OperandType, ByteConstantId{}, *ByteOffset);
          GlobalFixups.push_back({Address.get(), Global.Text, Global});
          Result.ParsedValue = std::move(Address);
          Result.Location = Global;
          return Result;
        }
        if (atIdentifier("global"))
        {
          const Token Location = consume();
          if (expect(TokenKind::LeftParenthesis, "'(' after 'global'") == nullptr)
          {
            return std::nullopt;
          }
          const Token *Module = expect(TokenKind::Integer, "a module id");
          if (Module == nullptr || expect(TokenKind::Comma, "','") == nullptr)
          {
            return std::nullopt;
          }
          const Token *Global = expect(TokenKind::Integer, "a global variable id");
          if (Global == nullptr || expect(TokenKind::RightParenthesis, "')'") == nullptr)
          {
            return std::nullopt;
          }
          const std::optional<std::size_t> ModuleIndex = parseId(*Module, "module id");
          const std::optional<std::size_t> GlobalIndex = parseId(*Global, "global variable id");
          if (!ModuleIndex || !GlobalIndex)
          {
            return std::nullopt;
          }
          Result.ParsedValue = std::make_unique<GlobalVariableAddressOperand>(OperandType, GlobalRef{ModuleId{*ModuleIndex}, GlobalId{*GlobalIndex}});
          Result.Location = Location;
          return Result;
        }
        fail<DiagnosticKind::IrExpectedOperand>(current());
        return std::nullopt;
      }

      std::optional<ParsedOperand> parseOperand()
      {
        const std::optional<const Type *> Type = parseType();
        if (!Type)
        {
          return std::nullopt;
        }
        return parseOperandValue(**Type);
      }

      std::optional<std::vector<ParsedOperand>> parseArguments()
      {
        std::vector<ParsedOperand> Result;
        if (at(TokenKind::RightParenthesis))
        {
          return Result;
        }
        std::optional<ParsedOperand> FirstOperand = parseOperand();
        if (!FirstOperand)
        {
          return std::nullopt;
        }
        Result.push_back(std::move(*FirstOperand));
        while (at(TokenKind::Comma))
        {
          consume();
          std::optional<ParsedOperand> Operand = parseOperand();
          if (!Operand)
          {
            return std::nullopt;
          }
          Result.push_back(std::move(*Operand));
        }
        return Result;
      }

      std::unique_ptr<Instruction> parseCall(std::size_t FunctionIndex, std::size_t BlockIndex, std::size_t InstructionIndex, std::optional<ValueId> ResultValue)
      {
        if (!expectIdentifier("call"))
        {
          return nullptr;
        }
        const std::optional<const Type *> ResultType = parseType();
        if (!ResultType)
        {
          return nullptr;
        }
        auto Call = std::make_unique<CallInstruction>(**ResultType);
        Call->Result = ResultValue;
        if (at(TokenKind::GlobalName))
        {
          const Token Callee = consume();
          CallFixups.push_back({FunctionIndex, BlockIndex, InstructionIndex, Callee.Text, Callee});
        }
        else
        {
          if (!expectIdentifier("module") || expect(TokenKind::LeftParenthesis, "'(' after 'module'") == nullptr)
          {
            return nullptr;
          }
          const Token *Module = expect(TokenKind::Integer, "a module id");
          if (Module == nullptr || expect(TokenKind::Comma, "','") == nullptr)
          {
            return nullptr;
          }
          const Token *Function = expect(TokenKind::Integer, "a function id");
          if (Function == nullptr || expect(TokenKind::RightParenthesis, "')'") == nullptr)
          {
            return nullptr;
          }
          const std::optional<std::size_t> ModuleIndex = parseId(*Module, "module id");
          const std::optional<std::size_t> FunctionIndexValue = parseId(*Function, "function id");
          if (!ModuleIndex || !FunctionIndexValue)
          {
            return nullptr;
          }
          Call->Callee = FunctionRef{ModuleId{*ModuleIndex}, FunctionId{*FunctionIndexValue}};
        }
        if (expect(TokenKind::LeftParenthesis, "'('") == nullptr)
        {
          return nullptr;
        }
        std::optional<std::vector<ParsedOperand>> Arguments = parseArguments();
        if (!Arguments || expect(TokenKind::RightParenthesis, "')'") == nullptr)
        {
          return nullptr;
        }
        for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments->size(); ++ArgumentIndex)
        {
          Call->Arguments.push_back(std::move((*Arguments)[ArgumentIndex].ParsedValue));
        }
        return Call;
      }

      std::unique_ptr<Instruction> parseImport()
      {
        if (!expectIdentifier("import"))
        {
          return nullptr;
        }
        const Token *Module = expect(TokenKind::Integer, "a module id");
        if (Module == nullptr)
        {
          return nullptr;
        }
        const std::optional<std::size_t> ModuleIndex = parseId(*Module, "module id");
        if (!ModuleIndex)
        {
          return nullptr;
        }
        return std::make_unique<ImportInstruction>(ModuleId{*ModuleIndex});
      }

      std::unique_ptr<Instruction> parseReturn(std::size_t, std::size_t, std::size_t)
      {
        if (!expectIdentifier("ret"))
        {
          return nullptr;
        }
        auto Return = std::make_unique<ReturnInstruction>();
        if (atIdentifier("void"))
        {
          consume();
          return Return;
        }
        std::optional<ParsedOperand> Value = parseOperand();
        if (!Value)
        {
          return nullptr;
        }
        Return->ReturnValue = std::move(Value->ParsedValue);
        return Return;
      }

      std::unique_ptr<Instruction> parseInsertValue(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrInsertValueRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("insertvalue"))
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Aggregate = parseOperand();
        if (!Aggregate || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Element = parseOperand();
        if (!Element || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        const Token *FieldIndex = expect(TokenKind::Integer, "a struct field index");
        if (FieldIndex == nullptr)
        {
          return nullptr;
        }
        const std::optional<std::size_t> ParsedFieldIndex = parseIndex(*FieldIndex, "struct field index");
        if (!ParsedFieldIndex)
        {
          return nullptr;
        }

        auto Insert = std::make_unique<InsertValueInstruction>(Aggregate->ParsedValue->type());
        Insert->Result = *ResultValue;
        Insert->Aggregate = std::move(Aggregate->ParsedValue);
        Insert->Element = std::move(Element->ParsedValue);
        Insert->FieldIndex = *ParsedFieldIndex;
        return Insert;
      }

      std::unique_ptr<Instruction> parseExtractValue(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrExtractValueRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("extractvalue"))
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Aggregate = parseOperand();
        if (!Aggregate || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        const Token *FieldIndex = expect(TokenKind::Integer, "a struct field index");
        if (FieldIndex == nullptr)
        {
          return nullptr;
        }
        const std::optional<std::size_t> ParsedFieldIndex = parseIndex(*FieldIndex, "struct field index");
        if (!ParsedFieldIndex)
        {
          return nullptr;
        }
        if (Aggregate->ParsedValue->type().kind() != TypeKind::Struct)
        {
          fail<DiagnosticKind::IrExtractAggregateMustBeStruct>(Aggregate->Location);
          return nullptr;
        }
        const StructType &AggregateType = static_cast<const StructType &>(Aggregate->ParsedValue->type());
        if (*ParsedFieldIndex >= AggregateType.fieldTypes().size())
        {
          fail<DiagnosticKind::IrFieldIndexOutOfRange>(*FieldIndex, *ParsedFieldIndex, AggregateType.fieldTypes().size());
          return nullptr;
        }

        auto Extract = std::make_unique<ExtractValueInstruction>(*AggregateType.fieldTypes()[*ParsedFieldIndex]);
        Extract->Result = *ResultValue;
        Extract->Aggregate = std::move(Aggregate->ParsedValue);
        Extract->FieldIndex = *ParsedFieldIndex;
        return Extract;
      }

      std::unique_ptr<Instruction> parseAlloca(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrAllocaRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("alloca"))
        {
          return nullptr;
        }
        const std::optional<const Type *> ResultType = parseType();
        if (!ResultType)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Size = parseOperand();
        if (!Size)
        {
          return nullptr;
        }
        auto Alloca = std::make_unique<AllocaInstruction>(**ResultType);
        Alloca->Result = *ResultValue;
        Alloca->Size = std::move(Size->ParsedValue);
        return Alloca;
      }

      std::unique_ptr<Instruction> parseLoad(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrLoadRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("load"))
        {
          return nullptr;
        }
        const std::optional<const Type *> ResultType = parseType();
        if (!ResultType || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Pointer = parseOperand();
        if (!Pointer)
        {
          return nullptr;
        }
        auto Load = std::make_unique<LoadInstruction>(**ResultType);
        Load->Result = *ResultValue;
        Load->Pointer = std::move(Pointer->ParsedValue);
        return Load;
      }

      std::unique_ptr<Instruction> parseGetElementPointer(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrGetElementPointerRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("getelementptr"))
        {
          return nullptr;
        }
        const std::optional<const Type *> ElementType = parseType();
        if (!ElementType || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Pointer = parseOperand();
        if (!Pointer || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> IndexValue = parseOperand();
        if (!IndexValue)
        {
          return nullptr;
        }
        auto GetElementPointer = std::make_unique<GetElementPointerInstruction>(Pointer->ParsedValue->type(), **ElementType);
        GetElementPointer->Result = *ResultValue;
        GetElementPointer->Pointer = std::move(Pointer->ParsedValue);
        GetElementPointer->Index = std::move(IndexValue->ParsedValue);
        while (at(TokenKind::Comma))
        {
          consume();
          std::optional<ParsedOperand> FieldIndex = parseOperand();
          if (!FieldIndex)
          {
            return nullptr;
          }
          GetElementPointer->FieldIndices.push_back(std::move(FieldIndex->ParsedValue));
        }
        return GetElementPointer;
      }

      std::unique_ptr<Instruction> parseStore(std::size_t, std::size_t, std::size_t)
      {
        if (!expectIdentifier("store"))
        {
          return nullptr;
        }
        std::optional<ParsedOperand> StoredValue = parseOperand();
        if (!StoredValue || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Pointer = parseOperand();
        if (!Pointer)
        {
          return nullptr;
        }
        auto Store = std::make_unique<StoreInstruction>();
        Store->StoredValue = std::move(StoredValue->ParsedValue);
        Store->Pointer = std::move(Pointer->ParsedValue);
        return Store;
      }

      std::unique_ptr<Instruction> parseLifetimeEnd(std::size_t, std::size_t, std::size_t)
      {
        if (!expectIdentifier("lifetime.end"))
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Slice = parseOperand();
        if (!Slice)
        {
          return nullptr;
        }
        auto LifetimeEnd = std::make_unique<LifetimeEndInstruction>();
        LifetimeEnd->Slice = std::move(Slice->ParsedValue);
        return LifetimeEnd;
      }

      std::unique_ptr<Instruction> parseSliceData(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrSliceDataRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("slice.data"))
        {
          return nullptr;
        }
        const std::optional<const Type *> ResultType = parseType();
        if (!ResultType)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Slice = parseOperand();
        if (!Slice)
        {
          return nullptr;
        }
        auto SliceData = std::make_unique<SliceDataInstruction>(**ResultType);
        SliceData->Result = *ResultValue;
        SliceData->Slice = std::move(Slice->ParsedValue);
        return SliceData;
      }

      std::unique_ptr<Instruction> parseSliceLength(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrSliceLengthRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("slice.length"))
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Slice = parseOperand();
        if (!Slice)
        {
          return nullptr;
        }
        auto SliceLength = std::make_unique<SliceLengthInstruction>(Context.getType(TypeKind::PointerSize));
        SliceLength->Result = *ResultValue;
        SliceLength->Slice = std::move(Slice->ParsedValue);
        return SliceLength;
      }

      std::unique_ptr<Instruction> parseAdd(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrAddRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("add"))
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Left = parseOperand();
        if (!Left || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Right = parseOperand();
        if (!Right)
        {
          return nullptr;
        }
        auto Add = std::make_unique<AddInstruction>(Left->ParsedValue->type());
        Add->Result = *ResultValue;
        Add->Left = std::move(Left->ParsedValue);
        Add->Right = std::move(Right->ParsedValue);
        return Add;
      }

      std::optional<ComparePredicate> parseComparePredicate()
      {
        const Token *Predicate = expect(TokenKind::Identifier, "an integer comparison predicate");
        if (Predicate == nullptr)
        {
          return std::nullopt;
        }
        if (Predicate->Text == "eq")
        {
          return ComparePredicate::Equal;
        }
        if (Predicate->Text == "ne")
        {
          return ComparePredicate::NotEqual;
        }
        if (Predicate->Text == "lt")
        {
          return ComparePredicate::LessThan;
        }
        if (Predicate->Text == "le")
        {
          return ComparePredicate::LessEqual;
        }
        if (Predicate->Text == "gt")
        {
          return ComparePredicate::GreaterThan;
        }
        if (Predicate->Text == "ge")
        {
          return ComparePredicate::GreaterEqual;
        }
        fail<DiagnosticKind::IrUnknownComparePredicate>(*Predicate, Predicate->Text);
        return std::nullopt;
      }

      std::unique_ptr<Instruction> parseCompare(std::size_t, std::size_t, std::size_t, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrCompareRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("icmp"))
        {
          return nullptr;
        }
        const std::optional<ComparePredicate> Predicate = parseComparePredicate();
        if (!Predicate)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Left = parseOperand();
        if (!Left || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Right = parseOperand();
        if (!Right)
        {
          return nullptr;
        }
        auto Compare = std::make_unique<CompareInstruction>(Context.getType(TypeKind::Bool));
        Compare->Result = *ResultValue;
        Compare->Predicate = *Predicate;
        Compare->Left = std::move(Left->ParsedValue);
        Compare->Right = std::move(Right->ParsedValue);
        return Compare;
      }

      std::unique_ptr<Instruction> parsePhi(std::size_t FunctionIndex, std::size_t BlockIndex, std::size_t InstructionIndex, std::optional<ValueId> ResultValue)
      {
        if (!ResultValue.has_value())
        {
          fail<DiagnosticKind::IrPhiRequiresResult>(current());
          return nullptr;
        }
        if (!expectIdentifier("phi"))
        {
          return nullptr;
        }
        const std::optional<const Type *> ResultType = parseType();
        if (!ResultType)
        {
          return nullptr;
        }

        auto Phi = std::make_unique<PhiInstruction>(**ResultType);
        Phi->Result = *ResultValue;
        while (true)
        {
          if (expect(TokenKind::LeftBracket, "'[' to begin a phi incoming value") == nullptr)
          {
            return nullptr;
          }
          std::optional<ParsedOperand> IncomingValue = parseOperandValue(**ResultType);
          if (!IncomingValue || expect(TokenKind::Comma, "',' between a phi value and predecessor") == nullptr)
          {
            return nullptr;
          }
          const Token *Predecessor = expect(TokenKind::Identifier, "a phi predecessor block");
          if (Predecessor == nullptr || expect(TokenKind::RightBracket, "']' to end a phi incoming value") == nullptr)
          {
            return nullptr;
          }

          const std::size_t IncomingIndex = Phi->IncomingValues.size();
          Phi->IncomingValues.push_back({std::move(IncomingValue->ParsedValue), BlockId{}});
          PhiFixups.push_back({FunctionIndex, BlockIndex, InstructionIndex, IncomingIndex, Predecessor->Text, *Predecessor});
          if (!at(TokenKind::Comma))
          {
            break;
          }
          consume();
        }
        return Phi;
      }

      std::optional<BlockTarget> parseBlockTarget(std::size_t FunctionIndex, std::size_t BlockIndex, std::size_t InstructionIndex, BranchFixupTarget FixupTarget)
      {
        const Token *Name = expect(TokenKind::Identifier, "a basic block target");
        if (Name == nullptr)
        {
          return std::nullopt;
        }
        BlockTarget Result;
        BranchFixups.push_back({FunctionIndex, BlockIndex, InstructionIndex, FixupTarget, Name->Text, *Name});
        return Result;
      }

      std::unique_ptr<Instruction> parseBranch(std::size_t FunctionIndex, std::size_t BlockIndex, std::size_t InstructionIndex)
      {
        if (!expectIdentifier("br"))
        {
          return nullptr;
        }
        std::optional<BlockTarget> Target = parseBlockTarget(FunctionIndex, BlockIndex, InstructionIndex, BranchFixupTarget::Branch);
        if (!Target)
        {
          return nullptr;
        }
        auto Branch = std::make_unique<BranchInstruction>();
        Branch->Target = std::move(*Target);
        return Branch;
      }

      std::unique_ptr<Instruction> parseConditionalBranch(std::size_t FunctionIndex, std::size_t BlockIndex, std::size_t InstructionIndex)
      {
        if (!expectIdentifier("condbr"))
        {
          return nullptr;
        }
        std::optional<ParsedOperand> Condition = parseOperand();
        if (!Condition || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<BlockTarget> TrueTarget = parseBlockTarget(FunctionIndex, BlockIndex, InstructionIndex, BranchFixupTarget::ConditionalTrue);
        if (!TrueTarget || expect(TokenKind::Comma, "','") == nullptr)
        {
          return nullptr;
        }
        std::optional<BlockTarget> FalseTarget = parseBlockTarget(FunctionIndex, BlockIndex, InstructionIndex, BranchFixupTarget::ConditionalFalse);
        if (!FalseTarget)
        {
          return nullptr;
        }
        auto Branch = std::make_unique<ConditionalBranchInstruction>();
        Branch->Condition = std::move(Condition->ParsedValue);
        Branch->TrueTarget = std::move(*TrueTarget);
        Branch->FalseTarget = std::move(*FalseTarget);
        return Branch;
      }

      std::unique_ptr<Instruction> parseInstruction(std::size_t FunctionIndex, std::size_t BlockIndex, std::size_t InstructionIndex)
      {
        std::optional<ValueId> ResultValue;
        if (at(TokenKind::ValueName))
        {
          const Token &Result = consume();
          const std::optional<std::size_t> Id = parseId(Result, "SSA result");
          if (!Id || expect(TokenKind::Equal, "'=' after an SSA result") == nullptr)
          {
            return nullptr;
          }
          ResultValue = ValueId{*Id};
        }
        if (atIdentifier("call"))
        {
          return parseCall(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("phi"))
        {
          return parsePhi(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("alloca"))
        {
          return parseAlloca(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("getelementptr"))
        {
          return parseGetElementPointer(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("load"))
        {
          return parseLoad(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("slice.data"))
        {
          return parseSliceData(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("slice.length"))
        {
          return parseSliceLength(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("add"))
        {
          return parseAdd(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("icmp"))
        {
          return parseCompare(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("insertvalue"))
        {
          return parseInsertValue(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (atIdentifier("extractvalue"))
        {
          return parseExtractValue(FunctionIndex, BlockIndex, InstructionIndex, ResultValue);
        }
        if (ResultValue.has_value())
        {
          if (atIdentifier("import") || atIdentifier("store") || atIdentifier("lifetime.end") || atIdentifier("br") || atIdentifier("condbr") || atIdentifier("ret"))
          {
            fail<DiagnosticKind::IrInstructionCannotDefineResult>(current(), current().Text);
            return nullptr;
          }
          fail<DiagnosticKind::IrExpectedValueProducingInstruction>(current());
          return nullptr;
        }
        if (atIdentifier("store"))
        {
          return parseStore(FunctionIndex, BlockIndex, InstructionIndex);
        }
        if (atIdentifier("import"))
        {
          return parseImport();
        }
        if (atIdentifier("lifetime.end"))
        {
          return parseLifetimeEnd(FunctionIndex, BlockIndex, InstructionIndex);
        }
        if (atIdentifier("br"))
        {
          return parseBranch(FunctionIndex, BlockIndex, InstructionIndex);
        }
        if (atIdentifier("condbr"))
        {
          return parseConditionalBranch(FunctionIndex, BlockIndex, InstructionIndex);
        }
        if (atIdentifier("ret"))
        {
          return parseReturn(FunctionIndex, BlockIndex, InstructionIndex);
        }
        fail<DiagnosticKind::IrExpectedInstruction>(current());
        return nullptr;
      }

      std::optional<BasicBlock> parseBasicBlock(std::size_t FunctionIndex, std::size_t BlockIndex)
      {
        BasicBlock Result;
        const Token *Name = expect(TokenKind::Identifier, "a basic block label");
        if (Name == nullptr)
        {
          return std::nullopt;
        }
        Result.Name = Name->Text;
        if (expect(TokenKind::Colon, "':' after a basic block label") == nullptr)
        {
          return std::nullopt;
        }
        while (!at(TokenKind::RightBrace) && !startsBasicBlock())
        {
          std::unique_ptr<Instruction> InstructionValue = parseInstruction(FunctionIndex, BlockIndex, Result.Instructions.size());
          if (!InstructionValue)
          {
            return std::nullopt;
          }
          Result.Instructions.push_back(std::move(InstructionValue));
        }
        return Result;
      }

      bool parseFunctionDefinition()
      {
        if (!expectIdentifier("define"))
        {
          return false;
        }
        const std::optional<const Type *> ResultType = parseType();
        if (!ResultType)
        {
          return false;
        }
        Function FunctionValue(**ResultType);
        const Token *Name = expect(TokenKind::GlobalName, "a function name");
        if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::LeftParenthesis, "'('") == nullptr)
        {
          return false;
        }
        FunctionValue.Name = Name->Text;
        std::optional<std::vector<const Type *>> ParameterTypes = parseDefinitionParameterTypes();
        if (!ParameterTypes || expect(TokenKind::RightParenthesis, "')'") == nullptr || expect(TokenKind::LeftBrace, "'{'") == nullptr)
        {
          return false;
        }
        FunctionValue.ParameterTypes = std::move(*ParameterTypes);
        const std::size_t FunctionIndex = ModuleValue.Functions.size();
        while (!at(TokenKind::RightBrace))
        {
          if (at(TokenKind::End))
          {
            return fail<DiagnosticKind::IrUnterminatedFunctionDefinition>(current());
          }
          std::optional<BasicBlock> Block = parseBasicBlock(FunctionIndex, FunctionValue.Blocks.size());
          if (!Block)
          {
            return false;
          }
          FunctionValue.Blocks.push_back(std::move(*Block));
        }
        consume();
        const FunctionId Id{ModuleValue.Functions.size()};
        FunctionNames.emplace(FunctionValue.Name, Id);
        ModuleValue.Functions.push_back(std::move(FunctionValue));
        return true;
      }

      BlockTarget &blockTargetAt(const BranchFixup &Fixup)
      {
        Instruction &InstructionValue = *ModuleValue.Functions[Fixup.FunctionIndex].Blocks[Fixup.BlockIndex].Instructions[Fixup.InstructionIndex];
        switch (Fixup.Target)
        {
        case BranchFixupTarget::Branch:
          return static_cast<BranchInstruction &>(InstructionValue).Target;
        case BranchFixupTarget::ConditionalTrue:
          return static_cast<ConditionalBranchInstruction &>(InstructionValue).TrueTarget;
        case BranchFixupTarget::ConditionalFalse:
          return static_cast<ConditionalBranchInstruction &>(InstructionValue).FalseTarget;
        }
        return static_cast<BranchInstruction &>(InstructionValue).Target;
      }

      bool resolveReferences()
      {
        for (const CallFixup &Fixup : CallFixups)
        {
          const auto Callee = FunctionNames.find(Fixup.CalleeName);
          if (Callee == FunctionNames.end())
          {
            return fail<DiagnosticKind::IrUnknownCallTarget>(Fixup.Location, Fixup.CalleeName);
          }
          Instruction &InstructionValue = *ModuleValue.Functions[Fixup.FunctionIndex].Blocks[Fixup.BlockIndex].Instructions[Fixup.InstructionIndex];
          static_cast<CallInstruction &>(InstructionValue).Callee = Callee->second;
        }
        for (const GlobalFixup &Fixup : GlobalFixups)
        {
          const auto Global = ByteConstantNames.find(Fixup.GlobalName);
          if (Global == ByteConstantNames.end())
          {
            return fail<DiagnosticKind::IrUnknownGlobalByteConstant>(Fixup.Location, Fixup.GlobalName);
          }
          Fixup.Address->resolveByteConstant(Global->second);
        }
        for (const GlobalVariableFixup &Fixup : GlobalVariableFixups)
        {
          const auto Global = GlobalVariableNames.find(Fixup.GlobalName);
          if (Global == GlobalVariableNames.end())
          {
            return fail<DiagnosticKind::IrExpected>(Fixup.Location, "a declared global variable");
          }
          Fixup.Address->resolveGlobal(GlobalRef{Global->second});
        }
        for (const BranchFixup &Fixup : BranchFixups)
        {
          const Function &FunctionValue = ModuleValue.Functions[Fixup.FunctionIndex];
          std::size_t TargetIndex = FunctionValue.Blocks.size();
          for (std::size_t BlockIndex = 0; BlockIndex < FunctionValue.Blocks.size(); ++BlockIndex)
          {
            if (FunctionValue.Blocks[BlockIndex].Name == Fixup.BlockName)
            {
              TargetIndex = BlockIndex;
              break;
            }
          }
          if (TargetIndex == FunctionValue.Blocks.size())
          {
            return fail<DiagnosticKind::IrUnknownBasicBlockTarget>(Fixup.Location, Fixup.BlockName);
          }
          blockTargetAt(Fixup).Block = BlockId{TargetIndex};
        }
        for (const PhiFixup &Fixup : PhiFixups)
        {
          const Function &FunctionValue = ModuleValue.Functions[Fixup.FunctionIndex];
          std::size_t PredecessorIndex = FunctionValue.Blocks.size();
          for (std::size_t BlockIndex = 0; BlockIndex < FunctionValue.Blocks.size(); ++BlockIndex)
          {
            if (FunctionValue.Blocks[BlockIndex].Name == Fixup.BlockName)
            {
              PredecessorIndex = BlockIndex;
              break;
            }
          }
          if (PredecessorIndex == FunctionValue.Blocks.size())
          {
            return fail<DiagnosticKind::IrUnknownBasicBlockTarget>(Fixup.Location, Fixup.BlockName);
          }
          Instruction &InstructionValue = *ModuleValue.Functions[Fixup.FunctionIndex].Blocks[Fixup.BlockIndex].Instructions[Fixup.InstructionIndex];
          static_cast<PhiInstruction &>(InstructionValue).IncomingValues[Fixup.IncomingIndex].Predecessor = BlockId{PredecessorIndex};
        }
        if (InitializerName.has_value())
        {
          const auto InitializerFunction = FunctionNames.find(*InitializerName);
          if (InitializerFunction == FunctionNames.end())
          {
            return fail<DiagnosticKind::IrUnknownCallTarget>(InitializerLocation, *InitializerName);
          }
          ModuleValue.Initializer = InitializerFunction->second;
        }
        if (FinalizerName.has_value())
        {
          const auto FinalizerFunction = FunctionNames.find(*FinalizerName);
          if (FinalizerFunction == FunctionNames.end())
          {
            return fail<DiagnosticKind::IrUnknownCallTarget>(FinalizerLocation, *FinalizerName);
          }
          ModuleValue.Finalizer = FinalizerFunction->second;
        }
        return true;
      }

      IRContext &Context;
      std::vector<Token> Tokens;
      std::size_t Index = 0;
      Module ModuleValue;
      std::unordered_set<std::string> GlobalSymbolNames;
      std::unordered_map<std::string, const StructType *> TypeNames;
      std::unordered_map<std::string, ByteConstantId> ByteConstantNames;
      std::unordered_map<std::string, GlobalId> GlobalVariableNames;
      std::unordered_map<std::string, FunctionId> FunctionNames;
      std::vector<CallFixup> CallFixups;
      std::vector<BranchFixup> BranchFixups;
      std::vector<PhiFixup> PhiFixups;
      std::vector<GlobalFixup> GlobalFixups;
      std::vector<GlobalVariableFixup> GlobalVariableFixups;
      bool HasModuleDeclaration = false;
      std::optional<std::string> InitializerName;
      std::optional<std::string> FinalizerName;
      Token InitializerLocation;
      Token FinalizerLocation;
      Diagnostic ParseDiagnostic;
    };

    std::string escapeBytes(std::string_view Data)
    {
      constexpr char HexadecimalDigits[] = "0123456789ABCDEF";
      std::string Result;
      for (const unsigned char Byte : Data)
      {
        if (Byte >= 0x20U && Byte <= 0x7EU && Byte != static_cast<unsigned char>('"') && Byte != static_cast<unsigned char>('\\'))
        {
          Result.push_back(static_cast<char>(Byte));
        }
        else
        {
          Result.push_back('\\');
          Result.push_back(HexadecimalDigits[Byte >> 4U]);
          Result.push_back(HexadecimalDigits[Byte & 0x0FU]);
        }
      }
      return Result;
    }

    void writeType(std::ostringstream &Output, const Type &TypeValue)
    {
      if (TypeValue.kind() == TypeKind::Struct)
      {
        Output << '%' << static_cast<const StructType &>(TypeValue).name();
        return;
      }
      Output << typeKindName(TypeValue.kind());
    }

    void writeOperand(std::ostringstream &Output, const Module &ModuleValue, const Value &OperandValue);

    void writeHexadecimalBitPattern(std::ostringstream &Output, std::uint64_t BitPattern, std::size_t BitWidth)
    {
      constexpr char HexadecimalDigits[] = "0123456789ABCDEF";
      Output << "0x";
      for (std::size_t DigitIndex = BitWidth / 4; DigitIndex > 0; --DigitIndex)
      {
        const std::size_t Shift = (DigitIndex - 1) * 4;
        Output << HexadecimalDigits[(BitPattern >> Shift) & 0x0FU];
      }
    }

    void writeOperandValue(std::ostringstream &Output, const Module &ModuleValue, const Value &OperandValue)
    {
      switch (OperandValue.kind())
      {
      case ValueKind::IntegerConstant:
      {
        const IntegerConstant &Constant = static_cast<const IntegerConstant &>(OperandValue);
        if (Constant.isNegative())
        {
          Output << Constant.signedValue();
        }
        else
        {
          Output << Constant.unsignedValue();
        }
        return;
      }
      case ValueKind::ValueOperand:
        Output << '%' << static_cast<const ValueOperand &>(OperandValue).id().value();
        return;
      case ValueKind::GlobalAddressOperand:
      {
        const GlobalAddressOperand &Address = static_cast<const GlobalAddressOperand &>(OperandValue);
        Output << '@' << ModuleValue.ByteConstants[Address.global().value()].Name << '[' << Address.byteOffset() << ']';
        return;
      }
      case ValueKind::GlobalVariableAddressOperand:
      {
        const GlobalRef Global = static_cast<const GlobalVariableAddressOperand &>(OperandValue).global();
        if (!Global.Module.valid() || (ModuleValue.Id.valid() && Global.Module == ModuleValue.Id))
        {
          Output << '@' << ModuleValue.Globals[Global.Global.value()].Name;
        }
        else
        {
          Output << "global(" << Global.Module.value() << ", " << Global.Global.value() << ')';
        }
        return;
      }
      case ValueKind::ZeroInitializer:
        Output << "zeroinitializer";
        return;
      case ValueKind::FloatConstant:
      {
        const FloatConstant &Constant = static_cast<const FloatConstant &>(OperandValue);
        Output << "floatbits(" << floatFormatName(Constant.format()) << ',';
        writeHexadecimalBitPattern(Output, Constant.bitPattern(), floatFormatBitWidth(Constant.format()));
        Output << ')';
        return;
      }
      case ValueKind::StringConstant:
        Output << "c\"" << escapeBytes(static_cast<const StringConstant &>(OperandValue).data()) << '"';
        return;
      case ValueKind::NullConstant:
        Output << "null";
        return;
      case ValueKind::AggregateConstant:
      {
        const AggregateConstant &Constant = static_cast<const AggregateConstant &>(OperandValue);
        Output << '{';
        for (std::size_t ElementIndex = 0; ElementIndex < Constant.elements().size(); ++ElementIndex)
        {
          if (ElementIndex != 0)
          {
            Output << ", ";
          }
          writeOperand(Output, ModuleValue, *Constant.elements()[ElementIndex]);
        }
        Output << '}';
        return;
      }
      }
    }

    void writeOperand(std::ostringstream &Output, const Module &ModuleValue, const Value &OperandValue)
    {
      writeType(Output, OperandValue.type());
      Output << ' ';
      writeOperandValue(Output, ModuleValue, OperandValue);
    }

    void writeCall(std::ostringstream &Output, const Module &ModuleValue, const CallInstruction &Call)
    {
      Output << "  ";
      if (Call.Result.has_value())
      {
        Output << '%' << Call.Result->value() << " = ";
      }
      Output << "call ";
      writeType(Output, *Call.ResultType);
      if (!Call.Callee.Module.valid() || (ModuleValue.Id.valid() && Call.Callee.Module == ModuleValue.Id))
      {
        Output << " @" << ModuleValue.Functions[Call.Callee.Function.value()].Name;
      }
      else
      {
        Output << " module(" << Call.Callee.Module.value() << ", " << Call.Callee.Function.value() << ')';
      }
      Output << '(';
      for (std::size_t ArgumentIndex = 0; ArgumentIndex < Call.Arguments.size(); ++ArgumentIndex)
      {
        if (ArgumentIndex != 0)
        {
          Output << ", ";
        }
        writeOperand(Output, ModuleValue, *Call.Arguments[ArgumentIndex]);
      }
      Output << ")\n";
    }

    void writeImport(std::ostringstream &Output, const ImportInstruction &Import)
    {
      Output << "  import " << Import.Module.value() << '\n';
    }

    void writeAlloca(std::ostringstream &Output, const Module &ModuleValue, const AllocaInstruction &Alloca)
    {
      Output << "  %" << Alloca.Result.value() << " = alloca ";
      writeType(Output, *Alloca.ResultType);
      Output << ' ';
      writeOperand(Output, ModuleValue, *Alloca.Size);
      Output << '\n';
    }

    void writeLoad(std::ostringstream &Output, const Module &ModuleValue, const LoadInstruction &Load)
    {
      Output << "  %" << Load.Result.value() << " = load ";
      writeType(Output, *Load.ResultType);
      Output << ", ";
      writeOperand(Output, ModuleValue, *Load.Pointer);
      Output << '\n';
    }

    void writeGetElementPointer(std::ostringstream &Output, const Module &ModuleValue, const GetElementPointerInstruction &GetElementPointer)
    {
      Output << "  %" << GetElementPointer.Result.value() << " = getelementptr ";
      writeType(Output, *GetElementPointer.ElementType);
      Output << ", ";
      writeOperand(Output, ModuleValue, *GetElementPointer.Pointer);
      Output << ", ";
      writeOperand(Output, ModuleValue, *GetElementPointer.Index);
      for (const std::unique_ptr<Value> &FieldIndex : GetElementPointer.FieldIndices)
      {
        Output << ", ";
        writeOperand(Output, ModuleValue, *FieldIndex);
      }
      Output << '\n';
    }

    void writeStore(std::ostringstream &Output, const Module &ModuleValue, const StoreInstruction &Store)
    {
      Output << "  store ";
      writeOperand(Output, ModuleValue, *Store.StoredValue);
      Output << ", ";
      writeOperand(Output, ModuleValue, *Store.Pointer);
      Output << '\n';
    }

    void writeLifetimeEnd(std::ostringstream &Output, const Module &ModuleValue, const LifetimeEndInstruction &LifetimeEnd)
    {
      Output << "  lifetime.end ";
      writeOperand(Output, ModuleValue, *LifetimeEnd.Slice);
      Output << '\n';
    }

    void writeSliceData(std::ostringstream &Output, const Module &ModuleValue, const SliceDataInstruction &SliceData)
    {
      Output << "  %" << SliceData.Result.value() << " = slice.data ";
      writeType(Output, *SliceData.ResultType);
      Output << ' ';
      writeOperand(Output, ModuleValue, *SliceData.Slice);
      Output << '\n';
    }

    void writeSliceLength(std::ostringstream &Output, const Module &ModuleValue, const SliceLengthInstruction &SliceLength)
    {
      Output << "  %" << SliceLength.Result.value() << " = slice.length ";
      writeOperand(Output, ModuleValue, *SliceLength.Slice);
      Output << '\n';
    }

    void writePhi(std::ostringstream &Output, const Module &ModuleValue, const Function &FunctionValue, const PhiInstruction &Phi)
    {
      Output << "  %" << Phi.Result.value() << " = phi ";
      writeType(Output, *Phi.ResultType);
      Output << ' ';
      for (std::size_t IncomingIndex = 0; IncomingIndex < Phi.IncomingValues.size(); ++IncomingIndex)
      {
        if (IncomingIndex != 0)
        {
          Output << ", ";
        }
        const PhiIncoming &Incoming = Phi.IncomingValues[IncomingIndex];
        Output << '[';
        writeOperandValue(Output, ModuleValue, *Incoming.Value);
        Output << ", " << FunctionValue.Blocks[Incoming.Predecessor.value()].Name << ']';
      }
      Output << '\n';
    }

    void writeAdd(std::ostringstream &Output, const Module &ModuleValue, const AddInstruction &Add)
    {
      Output << "  %" << Add.Result.value() << " = add ";
      writeOperand(Output, ModuleValue, *Add.Left);
      Output << ", ";
      writeOperand(Output, ModuleValue, *Add.Right);
      Output << '\n';
    }

    void writeCompare(std::ostringstream &Output, const Module &ModuleValue, const CompareInstruction &Compare)
    {
      Output << "  %" << Compare.Result.value() << " = icmp " << comparePredicateName(Compare.Predicate) << ' ';
      writeOperand(Output, ModuleValue, *Compare.Left);
      Output << ", ";
      writeOperand(Output, ModuleValue, *Compare.Right);
      Output << '\n';
    }

    void writeBlockTarget(std::ostringstream &Output, const Function &FunctionValue, const BlockTarget &Target)
    {
      Output << FunctionValue.Blocks[Target.Block.value()].Name;
    }

    void writeBranch(std::ostringstream &Output, const Function &FunctionValue, const BranchInstruction &Branch)
    {
      Output << "  br ";
      writeBlockTarget(Output, FunctionValue, Branch.Target);
      Output << '\n';
    }

    void writeConditionalBranch(std::ostringstream &Output, const Module &ModuleValue, const Function &FunctionValue, const ConditionalBranchInstruction &Branch)
    {
      Output << "  condbr ";
      writeOperand(Output, ModuleValue, *Branch.Condition);
      Output << ", ";
      writeBlockTarget(Output, FunctionValue, Branch.TrueTarget);
      Output << ", ";
      writeBlockTarget(Output, FunctionValue, Branch.FalseTarget);
      Output << '\n';
    }

    void writeReturn(std::ostringstream &Output, const Module &ModuleValue, const ReturnInstruction &Return)
    {
      Output << "  ret ";
      if (!Return.ReturnValue)
      {
        Output << "void\n";
        return;
      }
      writeOperand(Output, ModuleValue, *Return.ReturnValue);
      Output << '\n';
    }

    void writeInsertValue(std::ostringstream &Output, const Module &ModuleValue, const InsertValueInstruction &Insert)
    {
      Output << "  %" << Insert.Result.value() << " = insertvalue ";
      writeOperand(Output, ModuleValue, *Insert.Aggregate);
      Output << ", ";
      writeOperand(Output, ModuleValue, *Insert.Element);
      Output << ", " << Insert.FieldIndex << '\n';
    }

    void writeExtractValue(std::ostringstream &Output, const Module &ModuleValue, const ExtractValueInstruction &Extract)
    {
      Output << "  %" << Extract.Result.value() << " = extractvalue ";
      writeOperand(Output, ModuleValue, *Extract.Aggregate);
      Output << ", " << Extract.FieldIndex << '\n';
    }

    void writeParameterTypes(std::ostringstream &Output, const std::vector<const Type *> &ParameterTypes, bool IncludeNames)
    {
      for (std::size_t ParameterIndex = 0; ParameterIndex < ParameterTypes.size(); ++ParameterIndex)
      {
        if (ParameterIndex != 0)
        {
          Output << ", ";
        }
        writeType(Output, *ParameterTypes[ParameterIndex]);
        if (IncludeNames)
        {
          Output << " %" << ParameterIndex;
        }
      }
    }
  } // namespace

  SerializeResult serialize(IRContext &Context, const Module &ModuleValue)
  {
    SerializeResult Result;
    const VerificationResult Verification = verify(Context, ModuleValue);
    if (!Verification.succeeded())
    {
      Result.Diagnostics = Verification.diagnostics();
      return Result;
    }

    std::ostringstream Output;
    Output << "inkir 1\n";
    if (ModuleValue.Id.valid())
    {
      Output << "module " << ModuleValue.Id.value() << '\n';
    }
    if (ModuleValue.Initializer.has_value())
    {
      Output << "initializer @" << ModuleValue.Functions[ModuleValue.Initializer->value()].Name << '\n';
    }
    if (ModuleValue.Finalizer.has_value())
    {
      Output << "finalizer @" << ModuleValue.Functions[ModuleValue.Finalizer->value()].Name << '\n';
    }
    for (const StructType *TypeValue : ModuleValue.StructTypes)
    {
      Output << "\n%" << TypeValue->name() << " = type {";
      for (std::size_t FieldIndex = 0; FieldIndex < TypeValue->fieldTypes().size(); ++FieldIndex)
      {
        if (FieldIndex != 0)
        {
          Output << ", ";
        }
        writeType(Output, *TypeValue->fieldTypes()[FieldIndex]);
      }
      Output << "}\n";
    }
    for (const ByteConstant &Constant : ModuleValue.ByteConstants)
    {
      Output << "\n@" << Constant.Name << " = private constant [" << Constant.Data.size() << " x byte] c\"" << escapeBytes(Constant.Data) << "\"\n";
    }
    for (const GlobalVariable &Global : ModuleValue.Globals)
    {
      Output << "\n@" << Global.Name << " = global " << (Global.Mutable ? "mutable " : "constant ");
      writeType(Output, *Global.ValueType);
      Output << '\n';
    }
    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      if (FunctionValue.Kind == FunctionKind::External)
      {
        Output << "\ndeclare extern \"C\" ";
        writeType(Output, *FunctionValue.ResultType);
        Output << " @" << FunctionValue.Name << '(';
        writeParameterTypes(Output, FunctionValue.ParameterTypes, false);
        Output << ')';
        if (FunctionValue.HasSideEffects)
        {
          Output << " [sideeffect]";
        }
        Output << '\n';
        continue;
      }
      Output << "\ndefine ";
      writeType(Output, *FunctionValue.ResultType);
      Output << " @" << FunctionValue.Name << '(';
      writeParameterTypes(Output, FunctionValue.ParameterTypes, true);
      Output << ") {\n";
      for (const BasicBlock &Block : FunctionValue.Blocks)
      {
        Output << Block.Name << ":\n";
        for (const std::unique_ptr<Instruction> &InstructionPointer : Block.Instructions)
        {
          const Instruction &InstructionValue = *InstructionPointer;
          switch (InstructionValue.kind())
          {
          case InstructionKind::Call:
            writeCall(Output, ModuleValue, static_cast<const CallInstruction &>(InstructionValue));
            break;
          case InstructionKind::Import:
            writeImport(Output, static_cast<const ImportInstruction &>(InstructionValue));
            break;
          case InstructionKind::Alloca:
            writeAlloca(Output, ModuleValue, static_cast<const AllocaInstruction &>(InstructionValue));
            break;
          case InstructionKind::GetElementPointer:
            writeGetElementPointer(Output, ModuleValue, static_cast<const GetElementPointerInstruction &>(InstructionValue));
            break;
          case InstructionKind::Load:
            writeLoad(Output, ModuleValue, static_cast<const LoadInstruction &>(InstructionValue));
            break;
          case InstructionKind::Store:
            writeStore(Output, ModuleValue, static_cast<const StoreInstruction &>(InstructionValue));
            break;
          case InstructionKind::LifetimeEnd:
            writeLifetimeEnd(Output, ModuleValue, static_cast<const LifetimeEndInstruction &>(InstructionValue));
            break;
          case InstructionKind::SliceData:
            writeSliceData(Output, ModuleValue, static_cast<const SliceDataInstruction &>(InstructionValue));
            break;
          case InstructionKind::SliceLength:
            writeSliceLength(Output, ModuleValue, static_cast<const SliceLengthInstruction &>(InstructionValue));
            break;
          case InstructionKind::Phi:
            writePhi(Output, ModuleValue, FunctionValue, static_cast<const PhiInstruction &>(InstructionValue));
            break;
          case InstructionKind::Add:
            writeAdd(Output, ModuleValue, static_cast<const AddInstruction &>(InstructionValue));
            break;
          case InstructionKind::Compare:
            writeCompare(Output, ModuleValue, static_cast<const CompareInstruction &>(InstructionValue));
            break;
          case InstructionKind::InsertValue:
            writeInsertValue(Output, ModuleValue, static_cast<const InsertValueInstruction &>(InstructionValue));
            break;
          case InstructionKind::ExtractValue:
            writeExtractValue(Output, ModuleValue, static_cast<const ExtractValueInstruction &>(InstructionValue));
            break;
          case InstructionKind::Branch:
            writeBranch(Output, FunctionValue, static_cast<const BranchInstruction &>(InstructionValue));
            break;
          case InstructionKind::ConditionalBranch:
            writeConditionalBranch(Output, ModuleValue, FunctionValue, static_cast<const ConditionalBranchInstruction &>(InstructionValue));
            break;
          case InstructionKind::Return:
            writeReturn(Output, ModuleValue, static_cast<const ReturnInstruction &>(InstructionValue));
            break;
          }
        }
      }
      Output << "}\n";
    }
    Result.Text = Output.str();
    return Result;
  }

  SerializeResult serialize(const Module &ModuleValue)
  {
    return serialize(ModuleValue.context(), ModuleValue);
  }

  DeserializeResult deserialize(IRContext &Context, std::string_view Text)
  {
    DeserializeResult Result;
    std::vector<Token> Tokens;
    Diagnostic Error;
    if (!Lexer(Text).tokenize(Tokens, Error))
    {
      Context.diagnosticEngine().report(Error);
      Result.Diagnostics.push_back(std::move(Error));
      return Result;
    }

    Module ModuleValue(Context);
    if (!TextParser(Context, std::move(Tokens)).parse(ModuleValue, Error))
    {
      Context.diagnosticEngine().report(Error);
      Result.Diagnostics.push_back(std::move(Error));
      return Result;
    }

    const VerificationResult Verification = verify(Context, ModuleValue, core::DiagnosticClass::User);
    if (!Verification.succeeded())
    {
      Result.Diagnostics = Verification.diagnostics();
      return Result;
    }
    Result.Value = std::move(ModuleValue);
    return Result;
  }

} // namespace ink::ir
