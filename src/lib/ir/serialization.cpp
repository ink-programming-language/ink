#include "ink/ir/serialization.h"

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
  using core::DiagnosticArgumentName;
  using core::DiagnosticBuilder;
  using core::DiagnosticKind;
  using core::SourceRange;

  namespace
  {
    enum class TokenKind
    {
      Identifier,
      GlobalName,
      ValueName,
      Integer,
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

    Diagnostic makeTextDiagnostic(SourceRange Span, std::string Message)
    {
      return DiagnosticBuilder(DiagnosticKind::InvalidIrText, Span).argument(DiagnosticArgumentName::Detail, std::move(Message)).build();
    }

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
      explicit Lexer(std::string_view Text) : Text(Text)
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
              Error = makeTextDiagnostic({Start, Position}, "expected a global symbol name after '@'");
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
            const std::size_t DigitsStart = Position;
            while (Position < Text.size() && std::isdigit(static_cast<unsigned char>(Text[Position])) != 0)
            {
              advance();
            }
            if (DigitsStart == Position)
            {
              Error = makeTextDiagnostic({Start, Position}, "expected a numeric SSA value name after '%'");
              return false;
            }
            Tokens.push_back({TokenKind::ValueName, std::string(Text.substr(DigitsStart, Position - DigitsStart)), {Start, Position}});
            continue;
          }
          if (Character == '-' || std::isdigit(static_cast<unsigned char>(Character)) != 0)
          {
            if (Character == '-' && (Position + 1 == Text.size() || std::isdigit(static_cast<unsigned char>(Text[Position + 1])) == 0))
            {
              Error = makeTextDiagnostic({Start, Start + 1}, "'-' must be followed by a decimal integer");
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
            Error = makeTextDiagnostic({Start, Start + 1}, std::string("unexpected character '") + Character + "'");
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
            Error = makeTextDiagnostic({Start, Position + 1}, "string literal cannot contain a raw line break");
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
            Error = makeTextDiagnostic({Start, Position}, "unterminated string escape");
            return false;
          }
          const int FirstHexadecimal = hexadecimalValue(Text[Position]);
          if (FirstHexadecimal >= 0)
          {
            if (Position + 1 >= Text.size())
            {
              Error = makeTextDiagnostic({Position, Text.size()}, "hexadecimal byte escape requires two digits");
              return false;
            }
            const int SecondHexadecimal = hexadecimalValue(Text[Position + 1]);
            if (SecondHexadecimal < 0)
            {
              Error = makeTextDiagnostic({Position, Position + 2}, "hexadecimal byte escape requires two digits");
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
            Error = makeTextDiagnostic({Position, Position + 1}, "unknown string escape");
            return false;
          }
          advance();
        }
        if (Position == Text.size())
        {
          Error = makeTextDiagnostic({Start, Position}, "unterminated string literal");
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

    struct GlobalFixup
    {
      std::size_t FunctionIndex = 0;
      std::size_t BlockIndex = 0;
      std::size_t InstructionIndex = 0;
      std::size_t OperandIndex = 0;
      bool IsReturn = false;
      std::string GlobalName;
      Token Location;
    };

    struct ParsedOperand
    {
      std::unique_ptr<Value> ParsedValue;
      std::optional<std::string> GlobalName;
      Token Location;
    };

    class TextParser
    {
    public:
      explicit TextParser(std::vector<Token> Tokens) : Tokens(std::move(Tokens))
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
          return fail(*Version, "unsupported InkIR format version");
        }

        while (!at(TokenKind::End))
        {
          if (at(TokenKind::GlobalName))
          {
            if (!parseByteConstant())
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
            return fail(current(), "expected a global constant, external declaration, or function definition");
          }
        }
        return resolveReferences();
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
          fail(current(), "expected " + std::string(Expected));
          return nullptr;
        }
        return &consume();
      }

      bool expectIdentifier(std::string_view Name)
      {
        if (!atIdentifier(Name))
        {
          return fail(current(), "expected '" + std::string(Name) + "'");
        }
        consume();
        return true;
      }

      bool fail(const Token &Location, std::string Message)
      {
        ParseDiagnostic = makeTextDiagnostic(Location.Span, std::move(Message));
        return false;
      }

      std::optional<std::uint64_t> parseUnsigned(const Token &TokenValue, std::string_view Description)
      {
        if (!TokenValue.Text.empty() && TokenValue.Text.front() == '-')
        {
          fail(TokenValue, std::string(Description) + " cannot be negative");
          return std::nullopt;
        }
        std::uint64_t Result = 0;
        const char *Begin = TokenValue.Text.data();
        const char *End = Begin + TokenValue.Text.size();
        const auto Conversion = std::from_chars(Begin, End, Result);
        if (Conversion.ec != std::errc() || Conversion.ptr != End)
        {
          fail(TokenValue, std::string(Description) + " is outside the supported range");
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
          fail(TokenValue, std::string(Description) + " is outside the supported range");
          return std::nullopt;
        }
        return Result;
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
          fail(TokenValue, std::string(Description) + " is outside the supported range");
          return std::nullopt;
        }
        return static_cast<std::size_t>(*Value);
      }

      std::optional<TypeKind> parseType()
      {
        if (atIdentifier("const"))
        {
          consume();
          if (!expectIdentifier("byte") || expect(TokenKind::Star, "'*'") == nullptr)
          {
            return std::nullopt;
          }
          return TypeKind::ConstBytePointer;
        }
        const Token *Type = expect(TokenKind::Identifier, "an IR type");
        if (Type == nullptr)
        {
          return std::nullopt;
        }
#define INK_IR_TYPE(Name, Spelling) \
  if (Type->Text == Spelling)       \
  {                                 \
    return TypeKind::Name;          \
  }
#include "ink/ir/ir.def"
        fail(*Type, "unknown IR type '" + Type->Text + "'");
        return std::nullopt;
      }

      bool reserveGlobalSymbol(const Token &Name)
      {
        if (!GlobalSymbolNames.insert(Name.Text).second)
        {
          return fail(Name, "duplicate global symbol @" + Name.Text);
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
          return fail(*SizeToken, "declared byte constant size does not match the decoded string length");
        }
        const GlobalId Id{ModuleValue.ByteConstants.size()};
        GlobalNames.emplace(Name->Text, Id);
        ModuleValue.ByteConstants.push_back({Name->Text, Data->Text});
        return true;
      }

      std::optional<std::vector<TypeKind>> parseExternalParameterTypes()
      {
        std::vector<TypeKind> Result;
        if (at(TokenKind::RightParenthesis))
        {
          return Result;
        }
        const std::optional<TypeKind> FirstType = parseType();
        if (!FirstType)
        {
          return std::nullopt;
        }
        Result.push_back(*FirstType);
        while (at(TokenKind::Comma))
        {
          consume();
          const std::optional<TypeKind> Type = parseType();
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
          return fail(*Convention, "only the external calling convention \"C\" is supported");
        }
        Function FunctionValue;
        FunctionValue.Kind = FunctionKind::External;
        FunctionValue.Convention = CallingConvention::C;
        const std::optional<TypeKind> ResultType = parseType();
        if (!ResultType)
        {
          return false;
        }
        FunctionValue.ResultType = *ResultType;
        const Token *Name = expect(TokenKind::GlobalName, "an external function name");
        if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::LeftParenthesis, "'('") == nullptr)
        {
          return false;
        }
        FunctionValue.Name = Name->Text;
        std::optional<std::vector<TypeKind>> ParameterTypes = parseExternalParameterTypes();
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

      std::optional<std::vector<TypeKind>> parseDefinitionParameterTypes()
      {
        std::vector<TypeKind> Result;
        if (at(TokenKind::RightParenthesis))
        {
          return Result;
        }
        while (true)
        {
          const std::optional<TypeKind> Type = parseType();
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
          const std::optional<std::size_t> ParameterIndex = parseIndex(*ValueName, "function parameter SSA value");
          if (!ParameterIndex)
          {
            return std::nullopt;
          }
          if (*ParameterIndex != Result.size() - 1)
          {
            fail(*ValueName, "function parameter SSA values must be numbered consecutively from %0");
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

      std::optional<ParsedOperand> parseOperand()
      {
        ParsedOperand Result;
        const std::optional<TypeKind> Type = parseType();
        if (!Type)
        {
          return std::nullopt;
        }
        Result.Location = current();
        if (at(TokenKind::ValueName))
        {
          const Token &Value = consume();
          const std::optional<std::size_t> Id = parseIndex(Value, "SSA value");
          if (!Id)
          {
            return std::nullopt;
          }
          Result.ParsedValue = std::make_unique<ValueOperand>(*Type, ValueId{*Id});
          return Result;
        }
        if (at(TokenKind::Integer))
        {
          const Token &Integer = consume();
          const std::optional<std::int64_t> Value = parseSigned(Integer, "integer constant");
          if (!Value)
          {
            return std::nullopt;
          }
          Result.ParsedValue = std::make_unique<IntegerConstant>(*Type, *Value);
          return Result;
        }
        if (at(TokenKind::GlobalName))
        {
          const Token &Global = consume();
          if (expect(TokenKind::LeftBracket, "'[' after a global byte constant") == nullptr)
          {
            return std::nullopt;
          }
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
          Result.ParsedValue = std::make_unique<GlobalAddressOperand>(*Type, GlobalId{}, *ByteOffset);
          Result.GlobalName = Global.Text;
          Result.Location = Global;
          return Result;
        }
        fail(current(), "expected an SSA value, integer constant, or global byte address");
        return std::nullopt;
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
        auto Call = std::make_unique<CallInstruction>();
        Call->Result = ResultValue;
        const std::optional<TypeKind> ResultType = parseType();
        if (!ResultType)
        {
          return nullptr;
        }
        Call->ResultType = *ResultType;
        const Token *Callee = expect(TokenKind::GlobalName, "a call target");
        if (Callee == nullptr || expect(TokenKind::LeftParenthesis, "'('") == nullptr)
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
          if ((*Arguments)[ArgumentIndex].GlobalName.has_value())
          {
            GlobalFixups.push_back({FunctionIndex, BlockIndex, InstructionIndex, ArgumentIndex, false, *(*Arguments)[ArgumentIndex].GlobalName, (*Arguments)[ArgumentIndex].Location});
          }
          Call->Arguments.push_back(std::move((*Arguments)[ArgumentIndex].ParsedValue));
        }
        CallFixups.push_back({FunctionIndex, BlockIndex, InstructionIndex, Callee->Text, *Callee});
        return Call;
      }

      std::unique_ptr<Instruction> parseReturn(std::size_t FunctionIndex, std::size_t BlockIndex, std::size_t InstructionIndex)
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
        if (Value->GlobalName.has_value())
        {
          GlobalFixups.push_back({FunctionIndex, BlockIndex, InstructionIndex, 0, true, *Value->GlobalName, Value->Location});
        }
        Return->ReturnValue = std::move(Value->ParsedValue);
        return Return;
      }

      std::unique_ptr<Instruction> parseInstruction(std::size_t FunctionIndex, std::size_t BlockIndex, std::size_t InstructionIndex)
      {
        std::optional<ValueId> ResultValue;
        if (at(TokenKind::ValueName))
        {
          const Token &Result = consume();
          const std::optional<std::size_t> Id = parseIndex(Result, "SSA result");
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
        if (ResultValue.has_value())
        {
          fail(current(), "only call instructions can define an SSA result in this InkIR version");
          return nullptr;
        }
        if (atIdentifier("ret"))
        {
          return parseReturn(FunctionIndex, BlockIndex, InstructionIndex);
        }
        fail(current(), "expected a call or ret instruction");
        return nullptr;
      }

      std::optional<BasicBlock> parseBasicBlock(std::size_t FunctionIndex, std::size_t BlockIndex)
      {
        BasicBlock Result;
        const Token *Name = expect(TokenKind::Identifier, "a basic block label");
        if (Name == nullptr || expect(TokenKind::Colon, "':' after a basic block label") == nullptr)
        {
          return std::nullopt;
        }
        Result.Name = Name->Text;
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
        Function FunctionValue;
        const std::optional<TypeKind> ResultType = parseType();
        if (!ResultType)
        {
          return false;
        }
        FunctionValue.ResultType = *ResultType;
        const Token *Name = expect(TokenKind::GlobalName, "a function name");
        if (Name == nullptr || !reserveGlobalSymbol(*Name) || expect(TokenKind::LeftParenthesis, "'('") == nullptr)
        {
          return false;
        }
        FunctionValue.Name = Name->Text;
        std::optional<std::vector<TypeKind>> ParameterTypes = parseDefinitionParameterTypes();
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
            return fail(current(), "expected '}' to end the function definition");
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

      std::unique_ptr<Value> &valueAt(const GlobalFixup &Fixup)
      {
        Instruction &InstructionValue = *ModuleValue.Functions[Fixup.FunctionIndex].Blocks[Fixup.BlockIndex].Instructions[Fixup.InstructionIndex];
        if (Fixup.IsReturn)
        {
          return static_cast<ReturnInstruction &>(InstructionValue).ReturnValue;
        }
        return static_cast<CallInstruction &>(InstructionValue).Arguments[Fixup.OperandIndex];
      }

      bool resolveReferences()
      {
        for (const CallFixup &Fixup : CallFixups)
        {
          const auto Callee = FunctionNames.find(Fixup.CalleeName);
          if (Callee == FunctionNames.end())
          {
            return fail(Fixup.Location, "unknown call target @" + Fixup.CalleeName);
          }
          Instruction &InstructionValue = *ModuleValue.Functions[Fixup.FunctionIndex].Blocks[Fixup.BlockIndex].Instructions[Fixup.InstructionIndex];
          static_cast<CallInstruction &>(InstructionValue).Callee = Callee->second;
        }
        for (const GlobalFixup &Fixup : GlobalFixups)
        {
          const auto Global = GlobalNames.find(Fixup.GlobalName);
          if (Global == GlobalNames.end())
          {
            return fail(Fixup.Location, "unknown global byte constant @" + Fixup.GlobalName);
          }
          std::unique_ptr<Value> &ValuePointer = valueAt(Fixup);
          const GlobalAddressOperand &Address = static_cast<const GlobalAddressOperand &>(*ValuePointer);
          ValuePointer = std::make_unique<GlobalAddressOperand>(Address.type(), Global->second, Address.byteOffset());
        }
        return true;
      }

      std::vector<Token> Tokens;
      std::size_t Index = 0;
      Module ModuleValue;
      std::unordered_set<std::string> GlobalSymbolNames;
      std::unordered_map<std::string, GlobalId> GlobalNames;
      std::unordered_map<std::string, FunctionId> FunctionNames;
      std::vector<CallFixup> CallFixups;
      std::vector<GlobalFixup> GlobalFixups;
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

    void writeType(std::ostringstream &Output, TypeKind Type)
    {
      Output << typeKindName(Type);
    }

    void writeOperandValue(std::ostringstream &Output, const Module &ModuleValue, const Value &OperandValue)
    {
      if (OperandValue.kind() == ValueKind::IntegerConstant)
      {
        Output << static_cast<const IntegerConstant &>(OperandValue).value();
      }
      else if (OperandValue.kind() == ValueKind::ValueOperand)
      {
        Output << '%' << static_cast<const ValueOperand &>(OperandValue).id().value();
      }
      else
      {
        const GlobalAddressOperand &Address = static_cast<const GlobalAddressOperand &>(OperandValue);
        Output << '@' << ModuleValue.ByteConstants[Address.global().value()].Name << '[' << Address.byteOffset() << ']';
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
      writeType(Output, Call.ResultType);
      Output << " @" << ModuleValue.Functions[Call.Callee.value()].Name << '(';
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

    void writeParameterTypes(std::ostringstream &Output, const std::vector<TypeKind> &ParameterTypes, bool IncludeNames)
    {
      for (std::size_t ParameterIndex = 0; ParameterIndex < ParameterTypes.size(); ++ParameterIndex)
      {
        if (ParameterIndex != 0)
        {
          Output << ", ";
        }
        writeType(Output, ParameterTypes[ParameterIndex]);
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
    for (const ByteConstant &Constant : ModuleValue.ByteConstants)
    {
      Output << "\n@" << Constant.Name << " = private constant [" << Constant.Data.size() << " x byte] c\"" << escapeBytes(Constant.Data) << "\"\n";
    }
    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      if (FunctionValue.Kind != FunctionKind::External)
      {
        continue;
      }
      Output << "\ndeclare extern \"C\" ";
      writeType(Output, FunctionValue.ResultType);
      Output << " @" << FunctionValue.Name << '(';
      writeParameterTypes(Output, FunctionValue.ParameterTypes, false);
      Output << ')';
      if (FunctionValue.HasSideEffects)
      {
        Output << " [sideeffect]";
      }
      Output << '\n';
    }
    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      if (FunctionValue.Kind != FunctionKind::Definition)
      {
        continue;
      }
      Output << "\ndefine ";
      writeType(Output, FunctionValue.ResultType);
      Output << " @" << FunctionValue.Name << '(';
      writeParameterTypes(Output, FunctionValue.ParameterTypes, true);
      Output << ") {\n";
      for (const BasicBlock &Block : FunctionValue.Blocks)
      {
        Output << Block.Name << ":\n";
        for (const std::unique_ptr<Instruction> &InstructionPointer : Block.Instructions)
        {
          const Instruction &InstructionValue = *InstructionPointer;
          if (InstructionValue.kind() == InstructionKind::Call)
          {
            writeCall(Output, ModuleValue, static_cast<const CallInstruction &>(InstructionValue));
          }
          else
          {
            writeReturn(Output, ModuleValue, static_cast<const ReturnInstruction &>(InstructionValue));
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
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    return serialize(Context, ModuleValue);
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

    Module ModuleValue;
    if (!TextParser(std::move(Tokens)).parse(ModuleValue, Error))
    {
      Context.diagnosticEngine().report(Error);
      Result.Diagnostics.push_back(std::move(Error));
      return Result;
    }

    const VerificationResult Verification = verify(Context, ModuleValue);
    if (!Verification.succeeded())
    {
      Result.Diagnostics = Verification.diagnostics();
      return Result;
    }
    Result.Value = std::move(ModuleValue);
    return Result;
  }

  DeserializeResult deserialize(std::string_view Text)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    return deserialize(Context, Text);
  }
} // namespace ink::ir
