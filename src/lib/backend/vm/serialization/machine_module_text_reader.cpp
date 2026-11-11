#include "ink/backend/vm/serialization/machine_module_text.h"

#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/name/name_pool.h"

#include <array>
#include <charconv>
#include <limits>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>

namespace ink::vm
{
  namespace
  {
    core::Diagnostic textError(std::size_t Offset, std::string_view Description)
    {
      return core::makeDiagnostic<core::DiagnosticKind::VMTextInvalid>(core::SourceRange::fromByteOffsets(Offset, Offset), std::string(Description));
    }

    bool isIdentifierStart(char Character) noexcept
    {
      return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || Character == '_' || Character == '.' || Character == '$';
    }

    bool isDigit(char Character) noexcept
    {
      return Character >= '0' && Character <= '9';
    }

    int hexDigit(char Character) noexcept
    {
      if (isDigit(Character))
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

    enum class TokenKind
    {
      Identifier,
      Number,
      String,
      Colon,
      Comma,
      LeftBracket,
      RightBracket,
      Plus,
      Minus,
      At,
    };

    struct Token
    {
        TokenKind Kind;
        std::string Text;
        std::size_t Offset;
    };

    std::expected<std::vector<Token>, core::Diagnostic> tokenizeLine(std::string_view Line, std::size_t LineOffset)
    {
      std::vector<Token> Tokens;
      for (std::size_t Index = 0; Index < Line.size();)
      {
        const char Character = Line[Index];
        if (Character == ' ' || Character == '\t' || Character == '\r')
        {
          ++Index;
          continue;
        }
        if (Character == ';')
        {
          break;
        }
        const std::size_t Start = Index++;
        if (Character == '"')
        {
          std::string Decoded;
          bool Closed = false;
          while (Index < Line.size())
          {
            const unsigned char Next = static_cast<unsigned char>(Line[Index++]);
            if (Next == '"')
            {
              Closed = true;
              break;
            }
            if (Next == '\\')
            {
              if (Index == Line.size())
              {
                return std::unexpected(textError(LineOffset + Index - 1, "incomplete string escape"));
              }
              const char Escaped = Line[Index++];
              if (Escaped == '"' || Escaped == '\\')
              {
                Decoded.push_back(Escaped);
              }
              else if (Index < Line.size() && hexDigit(Escaped) >= 0 && hexDigit(Line[Index]) >= 0)
              {
                Decoded.push_back(static_cast<char>(hexDigit(Escaped) * 16 + hexDigit(Line[Index++])));
              }
              else
              {
                return std::unexpected(textError(LineOffset + Index - 2, "expected a two-digit hexadecimal byte escape"));
              }
            }
            else if (Next < 0x20 || Next > 0x7e)
            {
              return std::unexpected(textError(LineOffset + Index - 1, "name bytes outside printable ASCII must be escaped"));
            }
            else
            {
              Decoded.push_back(static_cast<char>(Next));
            }
          }
          if (!Closed)
          {
            return std::unexpected(textError(LineOffset + Start, "unterminated quoted name"));
          }
          Tokens.push_back({TokenKind::String, std::move(Decoded), LineOffset + Start});
          continue;
        }
        if (isIdentifierStart(Character) || isDigit(Character))
        {
          while (Index < Line.size() && (isIdentifierStart(Line[Index]) || isDigit(Line[Index])))
          {
            ++Index;
          }
          Tokens.push_back({isDigit(Character) ? TokenKind::Number : TokenKind::Identifier, std::string(Line.substr(Start, Index - Start)), LineOffset + Start});
          continue;
        }
        TokenKind Kind;
        switch (Character)
        {
        case ':':
          Kind = TokenKind::Colon;
          break;
        case ',':
          Kind = TokenKind::Comma;
          break;
        case '[':
          Kind = TokenKind::LeftBracket;
          break;
        case ']':
          Kind = TokenKind::RightBracket;
          break;
        case '+':
          Kind = TokenKind::Plus;
          break;
        case '-':
          Kind = TokenKind::Minus;
          break;
        case '@':
          Kind = TokenKind::At;
          break;
        default:
          return std::unexpected(textError(LineOffset + Start, "unexpected character"));
        }
        Tokens.push_back({Kind, std::string(1, Character), LineOffset + Start});
      }
      return Tokens;
    }

    class LineParser
    {
      public:
        LineParser(const std::vector<Token> &Tokens, std::size_t EndOffset)
            : Tokens(Tokens),
              EndOffset(EndOffset)
        {
        }

        const Token *peek() const noexcept
        {
          return Position < Tokens.size() ? &Tokens[Position] : nullptr;
        }

        std::size_t offset() const noexcept
        {
          return peek() ? peek()->Offset : EndOffset;
        }

        bool consume(TokenKind Kind) noexcept
        {
          if (!peek() || peek()->Kind != Kind)
          {
            return false;
          }
          ++Position;
          return true;
        }

        const Token *take(TokenKind Kind) noexcept
        {
          const Token *Result = peek();
          return consume(Kind) ? Result : nullptr;
        }

        bool unsignedInteger(std::uint64_t &Value) noexcept
        {
          const Token *Number = take(TokenKind::Number);
          if (!Number)
          {
            return false;
          }
          std::string_view Digits = Number->Text;
          int Base = 10;
          if (Digits.starts_with("0x") || Digits.starts_with("0X"))
          {
            Digits.remove_prefix(2);
            Base = 16;
          }
          const auto Result = std::from_chars(Digits.data(), Digits.data() + Digits.size(), Value, Base);
          return !Digits.empty() && Result.ec == std::errc{} && Result.ptr == Digits.data() + Digits.size();
        }

        bool displacement(std::int64_t &Value) noexcept
        {
          const bool Negative = consume(TokenKind::Minus);
          if (!Negative)
          {
            consume(TokenKind::Plus);
          }
          std::uint64_t Magnitude = 0;
          if (!unsignedInteger(Magnitude))
          {
            return false;
          }
          constexpr std::uint64_t Limit = std::uint64_t{1} << 63;
          if (Magnitude > Limit || (!Negative && Magnitude == Limit))
          {
            return false;
          }
          Value = Negative ? (Magnitude == Limit ? std::numeric_limits<std::int64_t>::min() : -static_cast<std::int64_t>(Magnitude)) : static_cast<std::int64_t>(Magnitude);
          return true;
        }

      private:
        const std::vector<Token> &Tokens;
        std::size_t EndOffset;
        std::size_t Position = 0;
    };

    std::optional<RegisterId> parseRegister(std::string_view Text) noexcept
    {
#define REGISTER(Kind, Encoding, Spelling) \
  if (Text == Spelling)                    \
  {                                        \
    return RegisterId::Kind;               \
  }
#include "ink/backend/vm/instruction/register.def"
#undef REGISTER
      return std::nullopt;
    }

    using ParsedOperand = std::variant<std::monostate, RegisterOperand, AddressOperand, ImmediateOperand, BasicBlockOperand>;

    bool parseSymbol(LineParser &Line, core::Name &Symbol)
    {
      if (!Line.consume(TokenKind::At))
      {
        return false;
      }
      const Token *Name = Line.take(TokenKind::Identifier);
      if (!Name)
      {
        Name = Line.take(TokenKind::String);
      }
      if (!Name || Name->Text.empty())
      {
        return false;
      }
      Symbol = core::NamePool::instance().intern(Name->Text);
      return Symbol.valid();
    }

    bool parseOptionalDisplacement(LineParser &Line, std::int64_t &Offset) noexcept
    {
      return !Line.peek() || (Line.peek()->Kind != TokenKind::Plus && Line.peek()->Kind != TokenKind::Minus) || Line.displacement(Offset);
    }

    bool parseOperand(LineParser &Line, ParsedOperand &Result)
    {
      if (Line.consume(TokenKind::LeftBracket))
      {
        std::optional<RegisterId> Register;
        core::Name Symbol;
        if (Line.peek() && Line.peek()->Kind == TokenKind::At)
        {
          if (!parseSymbol(Line, Symbol))
          {
            return false;
          }
        }
        else
        {
          const Token *Base = Line.take(TokenKind::Identifier);
          if (!Base)
          {
            return false;
          }
          Register = parseRegister(Base->Text);
          if (!Register || !isAddressRegister(*Register))
          {
            return false;
          }
        }
        std::int64_t Offset = 0;
        if (!parseOptionalDisplacement(Line, Offset) || !Line.consume(TokenKind::RightBracket))
        {
          return false;
        }
        if (Register)
        {
          Result.emplace<AddressOperand>(*Register, Offset);
        }
        else
        {
          Result.emplace<AddressOperand>(Symbol, Offset);
        }
        return true;
      }
      if (Line.peek() && Line.peek()->Kind == TokenKind::At)
      {
        core::Name Symbol;
        std::int64_t Offset = 0;
        if (!parseSymbol(Line, Symbol) || !parseOptionalDisplacement(Line, Offset))
        {
          return false;
        }
        Result.emplace<ImmediateOperand>(Symbol, Offset);
        return true;
      }
      if (Line.peek() && Line.peek()->Kind == TokenKind::Identifier)
      {
        const auto Register = parseRegister(Line.take(TokenKind::Identifier)->Text);
        std::int64_t Offset = 0;
        if (!Register || !isArithmeticRegister(*Register) || !parseOptionalDisplacement(Line, Offset))
        {
          return false;
        }
        Result.emplace<RegisterOperand>(*Register, Offset);
        return true;
      }
      const bool Negative = Line.consume(TokenKind::Minus);
      if (!Negative)
      {
        Line.consume(TokenKind::Plus);
      }
      std::uint64_t Bits = 0;
      if (!Line.unsignedInteger(Bits) || (Negative && Bits > (std::uint64_t{1} << 63)))
      {
        return false;
      }
      Result.emplace<ImmediateOperand>(Negative ? std::uint64_t{0} - Bits : Bits);
      return true;
    }

    template <Opcode Operation, std::size_t... Indices>
    bool operandTypesMatch(const std::array<ParsedOperand, 3> &Operands, std::index_sequence<Indices...>)
    {
      using Tuple = typename OpcodeTraits<Operation>::OperandTuple;
      return (static_cast<bool>(std::get_if<std::tuple_element_t<Indices, Tuple>>(&Operands[Indices])) && ...);
    }

    template <Opcode Operation, std::size_t... Indices>
    bool appendInstruction(MachineInstructionBuilder &Builder, const std::array<ParsedOperand, 3> &Operands, std::index_sequence<Indices...> IndicesValue)
    {
      using Tuple = typename OpcodeTraits<Operation>::OperandTuple;
      if (!operandTypesMatch<Operation>(Operands, IndicesValue))
      {
        return false;
      }
      return Builder.tryCreate<Operation>(*std::get_if<std::tuple_element_t<Indices, Tuple>>(&Operands[Indices])...) != nullptr;
    }

    struct PendingInstruction
    {
        MachineBasicBlock *Block;
        Opcode Operation;
        std::array<ParsedOperand, 3> Operands;
        std::string TargetLabel;
        std::size_t Offset;
        std::size_t TargetOffset = 0;
    };

    enum class Section
    {
      None,
      Globals,
      Text,
    };

    enum class DataKind
    {
      None,
      Global,
    };

    class ModuleReader
    {
      public:
        std::expected<void, core::Diagnostic> readLine(const std::vector<Token> &Tokens, std::size_t EndOffset)
        {
          if (Tokens.empty())
          {
            return {};
          }
          LineParser Line(Tokens, EndOffset);
          const Token *First = Line.take(TokenKind::Identifier);
          if (!First)
          {
            return std::unexpected(textError(Line.offset(), "expected a directive, label, or instruction"));
          }
          const std::size_t Offset = First->Offset;
          if (!Module)
          {
            const Token *Name = Line.take(TokenKind::String);
            if (First->Text != ".module" || !Name || Line.peek())
            {
              return std::unexpected(textError(Offset, "expected .module followed by a quoted name"));
            }
            const auto Interned = internName(*Name);
            if (!Interned)
            {
              return std::unexpected(Interned.error());
            }
            Module = std::make_unique<MachineModule>(*Interned);
            return {};
          }
          if (CurrentData != DataKind::None)
          {
            return readDataLine(*First, Line);
          }
          if (Line.consume(TokenKind::Colon))
          {
            return readBlock(*First, Line);
          }
          if (First->Text == ".declare" || First->Text == ".native_function" || First->Text == ".native_data")
          {
            return readSymbolDeclaration(*First, Line);
          }
          if (First->Text == ".function" || First->Text == ".global")
          {
            return readDeclaration(*First, Line);
          }
          if (First->Text == ".section")
          {
            const Token *Name = Line.take(TokenKind::Identifier);
            if (Function || !Name || Line.peek())
            {
              return std::unexpected(textError(Offset, "section directive requires one section name outside a function"));
            }
            if (Name->Text == ".data")
            {
              CurrentSection = Section::Globals;
            }
            else if (Name->Text == ".text")
            {
              CurrentSection = Section::Text;
            }
            else
            {
              return std::unexpected(textError(Name->Offset, "unknown section"));
            }
            return {};
          }
          if (First->Text == ".endfunction")
          {
            if (!Function || Line.peek())
            {
              return std::unexpected(textError(Offset, "unexpected or malformed .endfunction"));
            }
            if (const auto Result = appendFunctionInstructions(); !Result)
            {
              return Result;
            }
            Function = nullptr;
            Builder.clearInsertPoint();
            return {};
          }
          if (First->Text == ".stackslot" || First->Text == ".frame")
          {
            return readFrameLine(*First, Line);
          }
          if (First->Text.starts_with('.'))
          {
            return std::unexpected(textError(Offset, "unknown or misplaced directive"));
          }
          if (!Builder.getInsertBlock())
          {
            return std::unexpected(textError(Offset, "an instruction requires a basic block"));
          }
          return readInstruction(*First, Line);
        }

        std::expected<std::unique_ptr<MachineModule>, core::Diagnostic> finish(std::size_t Offset)
        {
          if (!Module)
          {
            return std::unexpected(textError(Offset, "missing .module directive"));
          }
          if (CurrentData != DataKind::None || Function)
          {
            return std::unexpected(textError(Offset, "unterminated data declaration or function"));
          }
          return std::move(Module);
        }

      private:
        std::expected<MachineSymbolLinkage, core::Diagnostic> readLinkage(LineParser &Line, core::Name Name)
        {
          MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local;
          if (const Token *Keyword = Line.take(TokenKind::Identifier))
          {
            if (Keyword->Text == "external")
            {
              Linkage = MachineSymbolLinkage::External;
            }
            else if (Keyword->Text != "local")
            {
              return std::unexpected(textError(Keyword->Offset, "expected local or external linkage"));
            }
          }
          if (Line.peek() || (Linkage == MachineSymbolLinkage::External && !Name.valid()))
          {
            return std::unexpected(textError(Line.offset(), "external definitions require a nonempty name and linkage must end the declaration"));
          }
          return Linkage;
        }

        std::expected<void, core::Diagnostic> readSymbolDeclaration(const Token &Directive, LineParser &Line)
        {
          if (Function)
          {
            return std::unexpected(textError(Directive.Offset, "symbol declarations must appear outside a function"));
          }
          if (Directive.Text == ".declare")
          {
            const Token *Kind = Line.take(TokenKind::Identifier);
            core::Name Name;
            if (!Kind || !parseSymbol(Line, Name))
            {
              return std::unexpected(textError(Directive.Offset, "expected a symbol kind and nonempty symbol name"));
            }
            MachineSymbolKind SymbolKind;
            if (Kind->Text == "function")
            {
              SymbolKind = MachineSymbolKind::Function;
            }
            else if (Kind->Text == "global")
            {
              SymbolKind = MachineSymbolKind::GlobalVariable;
            }
            else
            {
              return std::unexpected(textError(Kind->Offset, "unknown symbol declaration kind"));
            }
            bool IsConstant = false;
            if (const Token *Attribute = Line.take(TokenKind::Identifier))
            {
              if (SymbolKind != MachineSymbolKind::GlobalVariable || Attribute->Text != "readonly")
              {
                return std::unexpected(textError(Attribute->Offset, "only global declarations accept the readonly attribute"));
              }
              IsConstant = true;
            }
            if (Line.peek())
            {
              return std::unexpected(textError(Line.offset(), "unexpected token after the symbol declaration"));
            }
            Module->declareSymbol(Name, SymbolKind, IsConstant);
            return {};
          }
          core::Name Name;
          if (!parseSymbol(Line, Name) || !Line.consume(TokenKind::Comma))
          {
            return std::unexpected(textError(Directive.Offset, "expected a nonempty native link name followed by a comma"));
          }
          if (Directive.Text == ".native_data")
          {
            core::Name HostName;
            if (!parseSymbol(Line, HostName) || Line.peek())
            {
              return std::unexpected(textError(Line.offset(), "expected exactly one nonempty host data name"));
            }
            Module->declareNativeData(Name, NativeDataImport{HostName});
            return {};
          }
          core::Name HostName;
          if (!parseSymbol(Line, HostName) || Line.peek())
          {
            return std::unexpected(textError(Line.offset(), "expected exactly one nonempty host function name"));
          }
          Module->declareNativeFunction(Name, NativeFunctionImport{HostName});
          return {};
        }

        std::expected<core::Name, core::Diagnostic> internName(const Token &TokenValue)
        {
          const auto Name = core::NamePool::instance().intern(TokenValue.Text);
          if (!TokenValue.Text.empty() && !Name.valid())
          {
            return std::unexpected(textError(TokenValue.Offset, "name pool capacity exhausted"));
          }
          return Name;
        }

        std::expected<core::Name, core::Diagnostic> readDeclarationName(const Token &Label, LineParser &Line, bool HasDataFields)
        {
          if (HasDataFields)
          {
            if (!Line.consume(TokenKind::Comma))
            {
              return std::unexpected(textError(Line.offset(), "expected a comma after the declaration name"));
            }
            if (!Line.peek() || Line.peek()->Kind != TokenKind::String)
            {
              return internName(Label);
            }
          }
          else if (!Line.consume(TokenKind::Comma))
          {
            return internName(Label);
          }
          const Token *Name = Line.take(TokenKind::String);
          if (!Name || (HasDataFields && !Line.consume(TokenKind::Comma)))
          {
            return std::unexpected(textError(Line.offset(), "expected a quoted original name before the declaration fields"));
          }
          return internName(*Name);
        }

        std::expected<void, core::Diagnostic> readBlock(const Token &Label, LineParser &Line)
        {
          const Token *OriginalName = Line.take(TokenKind::String);
          if (!Function || Line.peek())
          {
            return std::unexpected(textError(Label.Offset, "a block label must appear inside a function with at most one quoted original name"));
          }
          if (BlockLabels.contains(Label.Text))
          {
            return std::unexpected(textError(Label.Offset, "duplicate block label in the function"));
          }
          const auto Name = internName(OriginalName ? *OriginalName : Label);
          if (!Name)
          {
            return std::unexpected(Name.error());
          }
          HasBlock = true;
          MachineBasicBlock &Block = Function->createBasicBlock(*Name);
          BlockLabels.emplace(Label.Text, &Block);
          Builder.setInsertPoint(Block);
          return {};
        }

        std::expected<void, core::Diagnostic> readDeclaration(const Token &Directive, LineParser &Line)
        {
          const bool FunctionDeclaration = Directive.Text == ".function";
          Section RequiredSection = Section::Globals;
          if (FunctionDeclaration)
          {
            RequiredSection = Section::Text;
          }
          if (Function || CurrentSection != RequiredSection)
          {
            return std::unexpected(textError(Directive.Offset, "declaration must appear in its section outside a function"));
          }
          const Token *Label = Line.take(TokenKind::Identifier);
          if (!Label)
          {
            return std::unexpected(textError(Line.offset(), "expected a declaration identifier"));
          }
          const auto Name = readDeclarationName(*Label, Line, !FunctionDeclaration);
          if (!Name)
          {
            return std::unexpected(Name.error());
          }
          if (!ModuleLabels.insert(Label->Text).second)
          {
            return std::unexpected(textError(Label->Offset, "duplicate declaration label in the module"));
          }
          if (FunctionDeclaration)
          {
            const auto Linkage = readLinkage(Line, *Name);
            if (!Linkage)
            {
              return std::unexpected(Linkage.error());
            }
            Function = &Module->createFunction(*Name, *Linkage);
            BlockLabels.clear();
            HasFrame = false;
            HasBlock = false;
            return {};
          }
          std::uint64_t FirstValue = 0;
          std::uint64_t SecondValue = 0;
          if (!Line.unsignedInteger(FirstValue) || !Line.consume(TokenKind::Comma) || !Line.unsignedInteger(SecondValue))
          {
            return std::unexpected(textError(Directive.Offset, "expected global size and alignment"));
          }
          DataIsConstant = false;
          if (Line.peek() && Line.peek()->Kind == TokenKind::Identifier && Line.peek()->Text == "readonly")
          {
            Line.take(TokenKind::Identifier);
            DataIsConstant = true;
          }
          const auto Linkage = readLinkage(Line, *Name);
          if (!Linkage)
          {
            return std::unexpected(Linkage.error());
          }
          DataLinkage = *Linkage;
          DataAlignment = SecondValue;
          if (DataAlignment == 0 || (DataAlignment & (DataAlignment - 1)) != 0)
          {
            return std::unexpected(textError(Directive.Offset, "data alignment must be a nonzero power of two"));
          }
          DataName = *Name;
          DataSize = FirstValue;
          DataBytes.clear();
          CurrentData = DataKind::Global;
          return {};
        }

        std::expected<void, core::Diagnostic> readDataLine(const Token &Directive, LineParser &Line)
        {
          if (Directive.Text == ".byte")
          {
            do
            {
              const std::size_t Offset = Line.offset();
              std::uint64_t Byte = 0;
              if (!Line.unsignedInteger(Byte) || Byte > 255)
              {
                return std::unexpected(textError(Offset, "expected a byte value in the range 0 through 255"));
              }
              if (DataBytes.size() >= DataSize)
              {
                return std::unexpected(textError(Offset, "global initializer exceeds its declared byte size"));
              }
              DataBytes.push_back(static_cast<std::uint8_t>(Byte));
            } while (Line.consume(TokenKind::Comma));
            if (Line.peek())
            {
              return std::unexpected(textError(Line.offset(), "unexpected token after byte value"));
            }
            return {};
          }
          if (Directive.Text != ".endglobal" || Line.peek())
          {
            return std::unexpected(textError(Directive.Offset, "expected .byte or the matching end directive"));
          }
          if (!Module->createGlobalVariable(DataSize, DataAlignment, DataBytes, DataName, DataLinkage, DataIsConstant))
          {
            return std::unexpected(textError(Directive.Offset, "invalid global data layout"));
          }
          CurrentData = DataKind::None;
          return {};
        }

        std::expected<void, core::Diagnostic> readFrameLine(const Token &Directive, LineParser &Line)
        {
          if (!Function || HasBlock || HasFrame)
          {
            return std::unexpected(textError(Directive.Offset, "stack slots and one final frame layout must precede all basic blocks"));
          }
          std::uint64_t FirstValue = 0;
          std::uint64_t Alignment = 0;
          if (!Line.unsignedInteger(FirstValue))
          {
            return std::unexpected(textError(Directive.Offset, "expected an unsigned frame size or base offset"));
          }
          if (Directive.Text == ".frame")
          {
            if (Line.peek() || !Function->frameInfo().layout(FirstValue))
            {
              return std::unexpected(textError(Directive.Offset, "invalid or overflowing frame layout"));
            }
            HasFrame = true;
            return {};
          }
          if (!Line.consume(TokenKind::Comma) || !Line.unsignedInteger(Alignment) || Line.peek() || !Function->frameInfo().createStackSlot(FirstValue, Alignment))
          {
            return std::unexpected(textError(Directive.Offset, "invalid stack slot size or alignment"));
          }
          return {};
        }

        template <Opcode Operation>
        std::expected<void, core::Diagnostic> readKnownInstruction(const Token &Mnemonic, LineParser &Line)
        {
          PendingInstruction Pending{Builder.getInsertBlock(), Operation, {}, {}, Mnemonic.Offset};
          if constexpr (OpcodeTraits<Operation>::Category == InstructionCategory::Branch)
          {
            const Token *Target = Line.take(TokenKind::Identifier);
            if (!Target || Line.peek())
            {
              return std::unexpected(textError(Line.offset(), "a branch requires exactly one basic block label"));
            }
            Pending.TargetLabel = Target->Text;
            Pending.TargetOffset = Target->Offset;
          }
          else
          {
            std::size_t Count = 0;
            if (Line.peek())
            {
              do
              {
                const std::size_t Offset = Line.offset();
                if (Count == Pending.Operands.size() || !parseOperand(Line, Pending.Operands[Count++]))
                {
                  return std::unexpected(textError(Offset, "invalid instruction operand"));
                }
              } while (Line.consume(TokenKind::Comma));
            }
            if (Line.peek())
            {
              return std::unexpected(textError(Line.offset(), "expected a comma between instruction operands"));
            }
            if (Count != OpcodeTraits<Operation>::OperandCount || !operandTypesMatch<Operation>(Pending.Operands, std::make_index_sequence<OpcodeTraits<Operation>::OperandCount>{}))
            {
              return std::unexpected(textError(Mnemonic.Offset, "instruction operands do not match the opcode form"));
            }
            if constexpr (OpcodeTraits<Operation>::HasDestination)
            {
              const auto *Destination = std::get_if<RegisterOperand>(&Pending.Operands[0]);
              if (Destination && (!isWritableAddressRegister(Destination->getRegister()) || Destination->getByteOffset() != 0))
              {
                return std::unexpected(textError(Mnemonic.Offset, "a register destination must be BP or SP with zero displacement"));
              }
            }
          }
          PendingInstructions.push_back(std::move(Pending));
          return {};
        }

        std::expected<void, core::Diagnostic> readCallInstruction(const Token &Mnemonic, LineParser &Line)
        {
          const bool Native = Mnemonic.Text == "callNative";
          PendingInstruction Pending{Builder.getInsertBlock(), Opcode::Calli, {}, {}, Mnemonic.Offset};
          const std::size_t Offset = Line.offset();
          if (!parseOperand(Line, Pending.Operands[0]) || Line.peek())
          {
            return std::unexpected(textError(Offset, "a call requires exactly one immediate or memory target"));
          }
          if (const auto *Immediate = std::get_if<ImmediateOperand>(&Pending.Operands[0]))
          {
            if (Immediate->getSymbol() && *Immediate->getByteOffset() != 0)
            {
              return std::unexpected(textError(Offset, "a direct function symbol cannot have a nonzero displacement"));
            }
            if (Native && Immediate->bits() && *Immediate->bits() > std::numeric_limits<std::uint32_t>::max())
            {
              return std::unexpected(textError(Offset, "a native function import index must fit in u32"));
            }
            Pending.Operation = Native ? Opcode::CallNativei : Opcode::Calli;
          }
          else if (std::holds_alternative<AddressOperand>(Pending.Operands[0]))
          {
            Pending.Operation = Native ? Opcode::CallNativem : Opcode::Callm;
          }
          else
          {
            return std::unexpected(textError(Offset, "a call target must be an immediate or memory operand"));
          }
          PendingInstructions.push_back(std::move(Pending));
          return {};
        }

        std::expected<void, core::Diagnostic> readInstruction(const Token &Mnemonic, LineParser &Line)
        {
          if (Mnemonic.Text == "call" || Mnemonic.Text == "callNative")
          {
            return readCallInstruction(Mnemonic, Line);
          }
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) \
  if (Mnemonic.Text == Spelling)                                                                                        \
  {                                                                                                                     \
    return readKnownInstruction<Opcode::Kind>(Mnemonic, Line);                                                          \
  }
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
          return std::unexpected(textError(Mnemonic.Offset, "unknown instruction mnemonic"));
        }

        std::expected<void, core::Diagnostic> appendFunctionInstructions()
        {
          // Resolve all targets before inserting instructions so forward branches cannot reorder a block's code.
          for (PendingInstruction &Pending : PendingInstructions)
          {
            if (!Pending.TargetLabel.empty())
            {
              const auto Target = BlockLabels.find(Pending.TargetLabel);
              if (Target == BlockLabels.end())
              {
                return std::unexpected(textError(Pending.TargetOffset, "unknown basic block label in this function"));
              }
              Pending.Operands[0].emplace<BasicBlockOperand>(Target->second);
            }
          }
          for (const PendingInstruction &Pending : PendingInstructions)
          {
            Builder.setInsertPoint(*Pending.Block);
            bool Appended = false;
            switch (Pending.Operation)
            {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition)                          \
  case Opcode::Kind:                                                                                                                             \
    Appended = appendInstruction<Opcode::Kind>(Builder, Pending.Operands, std::make_index_sequence<OpcodeTraits<Opcode::Kind>::OperandCount>{}); \
    break;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
            }
            if (!Appended)
            {
              return std::unexpected(textError(Pending.Offset, "instruction operands cannot be inserted into their function"));
            }
          }
          PendingInstructions.clear();
          return {};
        }

        std::unique_ptr<MachineModule> Module;
        std::unordered_set<std::string> ModuleLabels;
        std::unordered_map<std::string, MachineBasicBlock *> BlockLabels;
        std::vector<PendingInstruction> PendingInstructions;
        MachineInstructionBuilder Builder;
        MachineFunction *Function = nullptr;
        Section CurrentSection = Section::None;
        DataKind CurrentData = DataKind::None;
        core::Name DataName;
        MachineSymbolLinkage DataLinkage = MachineSymbolLinkage::Local;
        bool DataIsConstant = false;
        std::uint64_t DataSize = 0;
        std::uint64_t DataAlignment = 1;
        std::vector<std::uint8_t> DataBytes;
        bool HasFrame = false;
        bool HasBlock = false;
    };
  } // namespace

  std::expected<std::unique_ptr<MachineModule>, core::Diagnostic> readMachineModuleText(std::string_view Text)
  {
    if (Text.size() > core::SourceLocation::MaxByteOffset)
    {
      return std::unexpected(textError(0, "VM text exceeds the supported source offset range"));
    }
    ModuleReader Reader;
    for (std::size_t Offset = 0; Offset < Text.size();)
    {
      const std::size_t Newline = Text.find('\n', Offset);
      const std::size_t End = Newline == std::string_view::npos ? Text.size() : Newline;
      const auto Tokens = tokenizeLine(Text.substr(Offset, End - Offset), Offset);
      if (!Tokens)
      {
        return std::unexpected(Tokens.error());
      }
      const auto Result = Reader.readLine(*Tokens, End);
      if (!Result)
      {
        return std::unexpected(Result.error());
      }
      Offset = Newline == std::string_view::npos ? End : End + 1;
    }
    return Reader.finish(Text.size());
  }
} // namespace ink::vm
