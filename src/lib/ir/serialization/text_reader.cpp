#include "text_format.h"

#include "ink/core/config.h"
#include "ink/core/numeric/integer_bits.h"

#include <algorithm>
#include <charconv>
#include <limits>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ink::ir::detail
{
  namespace
  {
    constexpr RecordKind PendingKind = static_cast<RecordKind>(0);

    struct Token
    {
        std::string_view Text;
        std::size_t Offset = 0;
        bool Quoted = false;
    };

    struct TypedValue
    {
        std::uint32_t Type = NullId;
        std::uint32_t Value = NullId;
    };

    struct TypeCheck
    {
        std::uint32_t Value = NullId;
        std::uint32_t Type = NullId;
        std::size_t Offset = 0;
    };

    class AssemblyParser
    {
      public:
        AssemblyParser(std::string_view Input, const IRReadOptions &Options)
            : Input(Input),
              Options(Options)
        {
          advance();
        }

        IRSerializationResult<Model> parse()
        {
          if (Input.size() > Options.MaxBytes)
          {
            fail(IRSerializationErrorCode::ResourceLimit, "Text input exceeds MaxBytes");
          }
          if (!consume("ink_ir"))
          {
            fail(IRSerializationErrorCode::InvalidMagic, "Expected Ink text IR magic");
          }
          if (!consume(INK_IR_VERSION))
          {
            fail(IRSerializationErrorCode::UnsupportedVersion, "Unsupported Ink text IR version");
          }
          parseTarget();
          expect(recordTextName(RecordKind::Module));
          Record ModuleRecord = makeRecord(RecordKind::Module);
          ModuleRecord.Strings = stringList();
          const auto ModuleId = reserveRecord();
          attributes([&](std::string_view Name)
          {
            if (Name == "init" || Name == "deinit")
            {
              expect("=");
              ModuleRecord.References[Name == "init" ? schema::Module::Init : schema::Module::Deinit] = symbol(GlobalSymbols, '@');
            }
            else
            {
              unknownAttribute(Name);
            }
          });
          setRecord(ModuleId, std::move(ModuleRecord));
          expect("{");
          while (!Error && !consume("}"))
          {
            if (is("%"))
            {
              parseNominal();
            }
            else if (is("@"))
            {
              parseGlobal();
            }
            else if (is("declare") || is(recordTextName(RecordKind::Function)))
            {
              parseFunction();
            }
            else if (consume(recordTextName(RecordKind::Import)))
            {
              parseImport();
            }
            else if (consume(recordTextName(RecordKind::Export)))
            {
              parseExport();
            }
            else
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Expected a module declaration");
            }
          }
          if (!Error && !Current.Text.empty())
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Unexpected text after module");
          }
          resolveReferences();
          if (Error)
          {
            return std::unexpected(std::move(*Error));
          }
          orderDefinitions();
          return std::move(Data);
        }

      private:
        bool fail(IRSerializationErrorCode Code, std::string Message)
        {
          if (!Error)
          {
            Error = IRSerializationError{Code, Current.Offset, std::move(Message)};
          }
          return false;
        }

        static bool isSpace(char Character)
        {
          return Character == ' ' || Character == '\t' || Character == '\r' || Character == '\n' || Character == '\f';
        }

        static bool isPunctuation(char Character)
        {
          return Character == '(' || Character == ')' || Character == '[' || Character == ']' || Character == '{' || Character == '}' || Character == ',' || Character == ':' || Character == '=' || Character == '*' || Character == '&' || Character == '%' || Character == '@' || Character == '"' || Character == ';';
        }

        void advance()
        {
          while (Position < Input.size())
          {
            if (isSpace(Input[Position]))
            {
              ++Position;
            }
            else if (Input[Position] == ';')
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
          const auto Start = Position;
          Current = {{}, Start, false};
          if (Position == Input.size())
          {
            return;
          }
          if (Input[Position] == '"')
          {
            Current.Quoted = true;
            ++Position;
            while (Position < Input.size() && Input[Position] != '"')
            {
              if (Input[Position] == '\\' && Position + 1 < Input.size())
              {
                ++Position;
              }
              ++Position;
            }
            if (Position == Input.size())
            {
              fail(IRSerializationErrorCode::TruncatedInput, "Unterminated string literal");
            }
            else
            {
              ++Position;
            }
          }
          else if (isPunctuation(Input[Position]))
          {
            ++Position;
          }
          else if (Input.substr(Position, 3) == "...")
          {
            Position += 3;
          }
          else if (Input.substr(Position, 2) == "->")
          {
            Position += 2;
          }
          else
          {
            while (Position < Input.size() && !isSpace(Input[Position]) && !isPunctuation(Input[Position]) && Input.substr(Position, 3) != "..." && Input.substr(Position, 2) != "->")
            {
              ++Position;
            }
          }
          Current.Text = Input.substr(Start, Position - Start);
        }

        bool is(std::string_view Text) const
        {
          return !Current.Quoted && Current.Text == Text;
        }

        bool consume(std::string_view Text)
        {
          if (Error || !is(Text))
          {
            return false;
          }
          advance();
          return true;
        }

        bool expect(std::string_view Text)
        {
          return consume(Text) || fail(Current.Text.empty() ? IRSerializationErrorCode::TruncatedInput : IRSerializationErrorCode::InvalidSyntax, "Expected '" + std::string(Text) + "'");
        }

        static int hexDigit(char Character)
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

        std::string string()
        {
          std::string Result;
          if (Error || !Current.Quoted)
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Expected a quoted string");
            return Result;
          }
          const auto Text = Current.Text;
          for (std::size_t Index = 1; !Error && Index + 1 < Text.size(); ++Index)
          {
            char Character = Text[Index];
            if (static_cast<unsigned char>(Character) < 0x20 || static_cast<unsigned char>(Character) == 0x7f)
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Control bytes require hexadecimal escapes");
              break;
            }
            if (Character == '\\')
            {
              if (++Index + 1 >= Text.size())
              {
                fail(IRSerializationErrorCode::TruncatedInput, "Incomplete string escape");
                break;
              }
              Character = Text[Index];
              if (hexDigit(Character) < 0 || Index + 2 >= Text.size() || hexDigit(Text[Index + 1]) < 0)
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Byte escape requires exactly two hexadecimal digits");
                break;
              }
              Character = static_cast<char>((hexDigit(Character) << 4) | hexDigit(Text[Index + 1]));
              ++Index;
            }
            if (Result.size() >= Options.MaxStringBytes - StringBytes)
            {
              fail(IRSerializationErrorCode::ResourceLimit, "Decoded strings exceed MaxStringBytes");
              break;
            }
            Result.push_back(Character);
          }
          advance();
          return Result;
        }

        std::string name()
        {
          NameWasQuoted = Current.Quoted;
          if (Current.Quoted)
          {
            return string();
          }
          if (Current.Text.empty() || isPunctuation(Current.Text.front()) || is("->") || is("..."))
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Expected a symbol name");
            return {};
          }
          std::string Result(Current.Text);
          advance();
          return Result;
        }

        static bool isNumericName(const std::string &Symbol)
        {
          return !Symbol.empty() && std::all_of(Symbol.begin(), Symbol.end(), [](char Character)
          {
            return Character >= '0' && Character <= '9';
          });
        }

        static std::string inferredName(const std::string &Symbol, bool Quoted = false)
        {
          return !Quoted && isNumericName(Symbol) ? std::string{} : Symbol;
        }

        std::uint64_t number()
        {
          std::uint64_t Result = 0;
          auto Text = Current.Text;
          int Base = 10;
          if (Text.starts_with("0x"))
          {
            Base = 16;
            Text.remove_prefix(2);
          }
          const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Result, Base);
          if (Current.Quoted || Text.empty() || Parsed.ec != std::errc{} || Parsed.ptr != Text.data() + Text.size())
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Expected an unsigned integer within 64 bits");
          }
          advance();
          return Result;
        }

        bool listRoom(std::size_t Count)
        {
          return Count < Options.MaxOperands - OperandCount || fail(IRSerializationErrorCode::ResourceLimit, "Record fields exceed MaxOperands");
        }

        std::vector<std::string> stringList()
        {
          std::vector<std::string> Result;
          std::size_t PendingBytes = 0;
          expect("[");
          if (!consume("]"))
          {
            do
            {
              if (!listRoom(Result.size()))
              {
                break;
              }
              auto Value = string();
              if (Value.size() > Options.MaxStringBytes - StringBytes - PendingBytes)
              {
                fail(IRSerializationErrorCode::ResourceLimit, "Decoded strings exceed MaxStringBytes");
                break;
              }
              PendingBytes += Value.size();
              Result.push_back(std::move(Value));
            } while (consume(","));
            expect("]");
          }
          return Result;
        }

        template <typename Handler>
        void attributes(Handler Handle)
        {
          if (!consume("["))
          {
            return;
          }
          std::unordered_set<std::string> Seen;
          if (!consume("]"))
          {
            do
            {
              const auto Name = name();
              if (!Seen.insert(Name).second)
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Duplicate attribute '" + Name + "'");
              }
              Handle(Name);
            } while (consume(","));
            expect("]");
          }
        }

        void unknownAttribute(std::string_view Name)
        {
          fail(IRSerializationErrorCode::InvalidSyntax, "Unknown attribute '" + std::string(Name) + "'");
        }

        std::uint32_t reserveRecord()
        {
          if (Error || Data.Records.size() >= Options.MaxRecords || Data.Records.size() >= NullId)
          {
            fail(IRSerializationErrorCode::ResourceLimit, "Text IR exceeds MaxRecords");
            return NullId;
          }
          const auto Id = static_cast<std::uint32_t>(Data.Records.size());
          Data.Records.push_back({PendingKind, {}, {}, {}});
          return Id;
        }

        void setRecord(std::uint32_t Id, Record Value)
        {
          if (Error || Id == NullId)
          {
            return;
          }
          if (Data.Records[Id].Kind != PendingKind)
          {
            fail(IRSerializationErrorCode::InvalidReference, "Duplicate symbol definition");
            return;
          }
          const auto Count = Value.Numbers.size() + Value.References.size() + Value.Strings.size();
          if (Count > Options.MaxOperands - OperandCount)
          {
            fail(IRSerializationErrorCode::ResourceLimit, "Record fields exceed MaxOperands");
            return;
          }
          OperandCount += Count;
          for (const auto &Text : Value.Strings)
          {
            if (Text.size() > Options.MaxStringBytes - StringBytes)
            {
              fail(IRSerializationErrorCode::ResourceLimit, "Decoded strings exceed MaxStringBytes");
              return;
            }
            StringBytes += Text.size();
          }
          Data.Records[Id] = std::move(Value);
          DefinitionOrder.push_back(Id);
        }

        std::uint32_t appendRecord(Record Value)
        {
          const auto Id = reserveRecord();
          setRecord(Id, std::move(Value));
          return Id;
        }

        std::uint32_t internType(Record Value)
        {
          if (Error)
          {
            return NullId;
          }
          std::string Key = std::to_string(static_cast<std::uint32_t>(Value.Kind));
          for (const auto Number : Value.Numbers)
          {
            Key += ":" + std::to_string(Number);
          }
          Key += "/";
          for (const auto Reference : Value.References)
          {
            Key += ":" + std::to_string(Reference);
          }
          const auto It = Types.find(Key);
          if (It != Types.end())
          {
            return It->second;
          }
          const auto Id = appendRecord(std::move(Value));
          Types.emplace(std::move(Key), Id);
          return Id;
        }

        std::uint32_t lookup(std::unordered_map<std::string, std::uint32_t> &Symbols, const std::string &Name, bool Quoted = false)
        {
          const auto Key = std::string(1, Quoted && isNumericName(Name) ? '\1' : '\0') + Name;
          const auto It = Symbols.find(Key);
          if (It != Symbols.end())
          {
            return It->second;
          }
          const auto Id = reserveRecord();
          Symbols.emplace(Key, Id);
          return Id;
        }

        std::uint32_t symbol(std::unordered_map<std::string, std::uint32_t> &Symbols, char Prefix)
        {
          expect(std::string_view(&Prefix, 1));
          const auto Name = name();
          return lookup(Symbols, Name, NameWasQuoted);
        }

        std::uint32_t primitive(RecordKind Kind, bool Mutable = true)
        {
          Record Value = makeRecord(Kind);
          Value.Numbers[schema::Type::Mutable] = Mutable;
          return internType(std::move(Value));
        }

        std::uint32_t type(std::size_t Depth = 0)
        {
          if (Error)
          {
            return NullId;
          }
          if (Depth >= Options.MaxDepth)
          {
            fail(IRSerializationErrorCode::ResourceLimit, "Type nesting exceeds MaxDepth");
            return NullId;
          }
          const bool Mutable = !consume(recordTextName(RecordKind::QualifiedType));
          if (consume(recordTextName(RecordKind::PointerType)))
          {
            const auto Element = type(Depth + 1);
            Record Value = makeRecord(RecordKind::PointerType);
            Value.Numbers[schema::PointerType::Mutable] = Mutable;
            Value.References[schema::PointerType::Element] = Element;
            return internType(std::move(Value));
          }
          if (consume(recordTextName(RecordKind::ReferenceType)))
          {
            const auto Element = type(Depth + 1);
            Record Value = makeRecord(RecordKind::ReferenceType);
            Value.Numbers[schema::ReferenceType::Mutable] = Mutable;
            Value.References[schema::ReferenceType::Element] = Element;
            return internType(std::move(Value));
          }
          if (consume(recordTextName(RecordKind::ArrayType)))
          {
            const auto Count = number();
            expect("x");
            const auto Element = type(Depth + 1);
            expect("]");
            Record Value = makeRecord(RecordKind::ArrayType);
            Value.Numbers[schema::ArrayType::Mutable] = Mutable;
            Value.Numbers[schema::ArrayType::Count] = Count;
            Value.References[schema::ArrayType::Element] = Element;
            return internType(std::move(Value));
          }
          if (consume(recordTextName(RecordKind::FunctionType)))
          {
            Record Signature = makeRecord(RecordKind::FunctionType);
            Signature.Numbers[schema::FunctionType::Mutable] = Mutable;
            expect("(");
            if (!consume(")"))
            {
              do
              {
                if (!listRoom(Signature.References.size() + Signature.Numbers.size()))
                {
                  break;
                }
                Signature.References.push_back(type(Depth + 1));
                Signature.Numbers.push_back(consume("..."));
              } while (consume(","));
              expect(")");
            }
            expect("->");
            Signature.References[schema::FunctionType::Return] = type(Depth + 1);
            return internType(std::move(Signature));
          }
          if (is("%"))
          {
            const auto Declaration = symbol(TypeSymbols, '%');
            if (Mutable)
            {
              return Declaration;
            }
            Record Qualified = makeRecord(RecordKind::QualifiedType);
            Qualified.References[schema::QualifiedType::Declaration] = Declaration;
            return internType(std::move(Qualified));
          }
          if (consume(recordTextName(RecordKind::BoolType)))
          {
            return primitive(RecordKind::BoolType, Mutable);
          }
          if (consume(recordTextName(RecordKind::VoidType)))
          {
            return primitive(RecordKind::VoidType, Mutable);
          }
          if (consume(recordTextName(RecordKind::NullPointerType)))
          {
            return primitive(RecordKind::NullPointerType, Mutable);
          }
          if (is(recordTextName(RecordKind::MetaType)) || is(recordTextName(RecordKind::VariadicType)))
          {
            const auto Kind = is(recordTextName(RecordKind::MetaType)) ? RecordKind::MetaType : RecordKind::VariadicType;
            advance();
            if (!Mutable)
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "This type cannot be const-qualified");
            }
            return internType(makeRecord(Kind));
          }
          auto Spelling = Current.Text;
          const char FloatPrefix = recordTextName(RecordKind::FloatType).front();
          if (!Current.Quoted && Spelling.size() > 1 && (Spelling.front() == 'i' || Spelling.front() == 'u' || Spelling.front() == FloatPrefix))
          {
            const char Prefix = Spelling.front();
            Spelling.remove_prefix(1);
            std::uint64_t Width = 0;
            const auto Parsed = std::from_chars(Spelling.data(), Spelling.data() + Spelling.size(), Width);
            if (Parsed.ec != std::errc{} || Parsed.ptr != Spelling.data() + Spelling.size() || Width == 0 || (Prefix == FloatPrefix ? Width != 32 && Width != 64 : Width > core::IntegerBits::MaxBitWidth))
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Invalid numeric type width");
              return NullId;
            }
            advance();
            if (Prefix == FloatPrefix)
            {
              Record Value = makeRecord(RecordKind::FloatType);
              Value.Numbers[schema::FloatType::Mutable] = Mutable;
              Value.Numbers[schema::FloatType::Width] = Width;
              return internType(std::move(Value));
            }
            Record Value = makeRecord(RecordKind::IntegerType);
            Value.Numbers[schema::IntegerType::Mutable] = Mutable;
            Value.Numbers[schema::IntegerType::Width] = Width;
            Value.Numbers[schema::IntegerType::Signed] = Prefix == 'i';
            return internType(std::move(Value));
          }
          fail(IRSerializationErrorCode::InvalidSyntax, "Expected an IR type");
          return NullId;
        }

        void parseTarget()
        {
          expect("target");
          expect("(");
          expect("pointer_bits");
          expect("=");
          const auto PointerBits = number();
          if (PointerBits > std::numeric_limits<std::uint32_t>::max())
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Pointer width exceeds 32 bits");
          }
          Data.PointerBits = static_cast<std::uint32_t>(PointerBits);
          expect(",");
          expect("byte_order");
          expect("=");
          const auto ByteOrder = Current.Quoted ? std::nullopt : parseEnum<target::ByteOrder>(Current.Text);
          if (ByteOrder)
          {
            Data.ByteOrder = static_cast<std::uint32_t>(*ByteOrder);
            advance();
          }
          else
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Expected target byte order");
          }
          expect(",");
          expect("native_abi");
          expect("=");
          const auto NativeAbi = number();
          if (NativeAbi > 1)
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Native ABI flag must be zero or one");
          }
          Data.NativeAbi = NativeAbi != 0;
          expect(",");
          expect("alignments");
          expect("=");
          expect("[");
          for (std::size_t Index = 0; Index < Data.Alignments.size(); ++Index)
          {
            if (Index)
            {
              expect(",");
            }
            Data.Alignments[Index] = number();
          }
          expect("]");
          expect(")");
        }

        void parseNominal()
        {
          expect("%");
          const auto Symbol = name();
          const bool Quoted = NameWasQuoted;
          const auto Id = lookup(TypeSymbols, Symbol, Quoted);
          expect("=");
          expect("type");
          Record Value = makeRecord(RecordKind::StructType);
          Value.Numbers[schema::NominalType::Alignment] = 1;
          Value.Strings[schema::NominalType::Name] = inferredName(Symbol, Quoted);
          bool HasSize = false;
          bool HasAlignment = false;
          if (consume(recordTextName(RecordKind::StructType)))
          {
            Value.Kind = RecordKind::StructType;
          }
          else if (consume(recordTextName(RecordKind::ClassType)))
          {
            Value.Kind = RecordKind::ClassType;
          }
          else if (consume(recordTextName(RecordKind::InterfaceType)))
          {
            Value.Kind = RecordKind::InterfaceType;
          }
          else
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Expected nominal type kind");
          }
          attributes([&](std::string_view Name)
          {
            if (Name == "size" || Name == "alignment")
            {
              expect("=");
              Value.Numbers[Name == "size" ? schema::NominalType::Size : schema::NominalType::Alignment] = number();
              HasSize = HasSize || Name == "size";
              HasAlignment = HasAlignment || Name == "alignment";
            }
            else if (Name == "frozen")
            {
              Value.Numbers[schema::NominalType::Frozen] = 1;
            }
            else if (Name == "name")
            {
              expect("=");
              Value.Strings[schema::NominalType::Name] = string();
            }
            else
            {
              unknownAttribute(Name);
            }
          });
          if (!HasSize || !HasAlignment)
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Nominal types require explicit size and alignment");
          }
          setRecord(Id, std::move(Value));
          expect("{");
          while (!Error && !consume("}"))
          {
            expect(recordTextName(RecordKind::Field));
            const auto FieldType = type();
            Record Field = makeRecord(RecordKind::Field);
            Field.References[schema::Field::Owner] = Id;
            Field.References[schema::Field::Type] = FieldType;
            Field.Strings[schema::Field::Name] = string();
            bool HasOffset = false;
            attributes([&](std::string_view Name)
            {
              if (Name == "offset")
              {
                expect("=");
                Field.Numbers[schema::Field::Offset] = number();
                HasOffset = true;
              }
              else
              {
                unknownAttribute(Name);
              }
            });
            if (!HasOffset)
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Nominal fields require an explicit offset");
            }
            expect("=");
            if (!consume("none"))
            {
              const auto Initializer = typedValue(0, true);
              Field.References[schema::Field::Initializer] = Initializer.Value;
            }
            appendRecord(std::move(Field));
            if (!Error)
            {
              NominalFields[Id].push_back(FieldType);
            }
          }
        }

        void parseGlobal()
        {
          expect("@");
          const auto Symbol = name();
          const bool Quoted = NameWasQuoted;
          const auto Id = lookup(GlobalSymbols, Symbol, Quoted);
          expect("=");
          expect(recordTextName(RecordKind::Global));
          Record Value = makeRecord(RecordKind::Global);
          Value.References[schema::Global::Type] = type();
          Value.Strings[schema::Global::Name] = inferredName(Symbol, Quoted);
          expect("=");
          if (!consume("none"))
          {
            Value.References[schema::Global::Initializer] = typedValue(0, true).Value;
          }
          attributes([&](std::string_view Name)
          {
            if (Name == "string_storage")
            {
              Value.Numbers[schema::Global::StringStorage] = 1;
            }
            else if (Name == "name")
            {
              expect("=");
              Value.Strings[schema::Global::Name] = string();
            }
            else
            {
              unknownAttribute(Name);
            }
          });
          setRecord(Id, std::move(Value));
        }

        void parseImport()
        {
          const auto Id = symbol(GlobalSymbols, '@');
          expect(":");
          const auto Type = type();
          expect("from");
          Record Value = makeRecord(RecordKind::Import);
          Value.References[schema::Import::Type] = Type;
          Value.Strings = stringList();
          setRecord(Id, std::move(Value));
        }

        void parseExport()
        {
          const auto Id = consume("type") ? type() : symbol(GlobalSymbols, '@');
          expect("as");
          Record Value = makeRecord(RecordKind::Export);
          Value.References[schema::Export::Declaration] = Id;
          Value.Strings[schema::Export::Name] = string();
          appendRecord(std::move(Value));
        }

        void parseFunction()
        {
          const bool HasBody = consume(recordTextName(RecordKind::Function));
          if (!HasBody)
          {
            expect("declare");
          }
          Record Function = makeRecord(RecordKind::Function);
          Function.Numbers[schema::Function::Linkage] = encodeEnum(SymbolLinkage::Local);
          Function.Numbers[schema::Function::Language] = encodeEnum(LanguageLinkage::Ink);
          Function.Numbers[schema::Function::Visibility] = encodeEnum(FunctionVisibility::Public);
          Function.Numbers[schema::Function::Dispatch] = encodeEnum(MethodDispatch::Direct);
          Function.Numbers[schema::Function::Slot] = NoVirtualSlot;
          if (consume(enumName<SymbolLinkage>(encodeEnum(SymbolLinkage::Import))))
          {
            Function.Numbers[schema::Function::Linkage] = encodeEnum(SymbolLinkage::Import);
          }
          else if (consume(enumName<SymbolLinkage>(encodeEnum(SymbolLinkage::Export))))
          {
            Function.Numbers[schema::Function::Linkage] = encodeEnum(SymbolLinkage::Export);
          }
          if (consume("c"))
          {
            Function.Numbers[schema::Function::Language] = encodeEnum(LanguageLinkage::C);
          }
          else if (Current.Quoted)
          {
            if (string() != enumName<LanguageLinkage>(encodeEnum(LanguageLinkage::C)))
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Unknown language linkage");
            }
            Function.Numbers[schema::Function::Language] = encodeEnum(LanguageLinkage::C);
          }
          Record Signature = makeRecord(RecordKind::FunctionType);
          Signature.Numbers[schema::FunctionType::Mutable] = 1;
          Signature.References[schema::FunctionType::Return] = type();
          expect("@");
          const auto Symbol = name();
          const bool Quoted = NameWasQuoted;
          const auto Id = lookup(GlobalSymbols, Symbol, Quoted);
          Function.Strings[schema::Function::Name] = inferredName(Symbol, Quoted);
          LocalSymbols.clear();
          expect("(");
          if (!consume(")"))
          {
            do
            {
              if (!listRoom(Signature.Numbers.size() + Signature.References.size()))
              {
                break;
              }
              Signature.References.push_back(type());
              Signature.Numbers.push_back(consume("..."));
              expect("%");
              const auto ParameterSymbol = name();
              const bool ParameterQuoted = NameWasQuoted;
              const auto ParameterId = lookup(LocalSymbols, ParameterSymbol, ParameterQuoted);
              Record Parameter = makeRecord(RecordKind::Parameter);
              Parameter.Numbers[schema::Parameter::Index] = Signature.References.size() - schema::FunctionType::ParametersBegin - 1;
              Parameter.References[schema::Parameter::Function] = Id;
              Parameter.Strings[schema::Parameter::Name] = inferredName(ParameterSymbol, ParameterQuoted);
              attributes([&](std::string_view Name)
              {
                if (Name == "name")
                {
                  expect("=");
                  Parameter.Strings[schema::Parameter::Name] = string();
                }
                else
                {
                  unknownAttribute(Name);
                }
              });
              setRecord(ParameterId, std::move(Parameter));
            } while (consume(","));
            expect(")");
          }
          attributes([&](std::string_view Name)
          {
            if (Name == enumName<FunctionVisibility>(encodeEnum(FunctionVisibility::Private)))
            {
              Function.Numbers[schema::Function::Visibility] = encodeEnum(FunctionVisibility::Private);
            }
            else if (Name == "owner")
            {
              expect("=");
              Function.References[schema::Function::Owner] = symbol(TypeSymbols, '%');
            }
            else if (Name == enumName<MethodDispatch>(encodeEnum(MethodDispatch::Virtual)) || Name == enumName<MethodDispatch>(encodeEnum(MethodDispatch::Abstract)))
            {
              if (Function.Numbers[schema::Function::Dispatch] != encodeEnum(MethodDispatch::Direct))
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Conflicting method dispatch attributes");
              }
              Function.Numbers[schema::Function::Dispatch] = *parseEnum<MethodDispatch>(Name);
            }
            else if (Name == "override" || Name == "final")
            {
              Function.Numbers[Name == "override" ? schema::Function::Override : schema::Function::Final] = 1;
            }
            else if (Name == "slot")
            {
              expect("=");
              Function.Numbers[schema::Function::Slot] = number();
              if (Function.Numbers[schema::Function::Slot] > std::numeric_limits<std::uint32_t>::max())
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Virtual slot exceeds 32 bits");
              }
            }
            else if (Name == "signature_const")
            {
              Signature.Numbers[schema::FunctionType::Mutable] = 0;
            }
            else if (Name == "name")
            {
              expect("=");
              Function.Strings[schema::Function::Name] = string();
            }
            else
            {
              unknownAttribute(Name);
            }
          });
          Function.References[schema::Function::Signature] = internType(std::move(Signature));
          setRecord(Id, std::move(Function));
          if (HasBody)
          {
            parseBody(Id);
          }
          for (const auto &[Name, LocalId] : LocalSymbols)
          {
            if (!Error && Data.Records[LocalId].Kind == PendingKind)
            {
              fail(IRSerializationErrorCode::InvalidReference, "Undefined local symbol '%" + Name.substr(1) + "'");
            }
          }
          LocalSymbols.clear();
        }

        void parseBody(std::uint32_t FunctionId)
        {
          expect("{");
          std::uint32_t BlockId = NullId;
          while (!Error && !consume("}"))
          {
            std::uint32_t ResultId = NullId;
            std::string ResultName;
            bool ResultQuoted = false;
            if (consume("%"))
            {
              ResultName = name();
              ResultQuoted = NameWasQuoted;
              ResultId = lookup(LocalSymbols, ResultName, ResultQuoted);
              expect("=");
            }
            const auto Word = name();
            const bool WordQuoted = NameWasQuoted;
            if (ResultId == NullId && consume(":"))
            {
              BlockId = lookup(LocalSymbols, Word, WordQuoted);
              Record Block = makeRecord(RecordKind::Block);
              Block.References[schema::Block::Function] = FunctionId;
              Block.Strings[schema::Block::Name] = inferredName(Word, WordQuoted);
              attributes([&](std::string_view Name)
              {
                if (Name == "name")
                {
                  expect("=");
                  Block.Strings[schema::Block::Name] = string();
                }
                else
                {
                  unknownAttribute(Name);
                }
              });
              setRecord(BlockId, std::move(Block));
              continue;
            }
            if (BlockId == NullId)
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "An instruction requires a basic block label");
              break;
            }
            parseInstruction(Word, ResultId, inferredName(ResultName, ResultQuoted), BlockId);
          }
          if (!Error && BlockId == NullId)
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "A function definition requires a basic block");
          }
        }

        TypedValue typedValue(std::size_t Depth = 0, bool ConstantOnly = false)
        {
          if (Depth >= Options.MaxDepth)
          {
            fail(IRSerializationErrorCode::ResourceLimit, "Constant nesting exceeds MaxDepth");
            return {};
          }
          const auto Type = type(Depth);
          if (Error)
          {
            return {};
          }
          if (is("@") || is("%"))
          {
            if (ConstantOnly)
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Expected an inline constant");
              return {};
            }
            const auto Offset = Current.Offset;
            const auto Value = is("@") ? symbol(GlobalSymbols, '@') : symbol(LocalSymbols, '%');
            Checks.push_back({Value, Type, Offset});
            return {Type, Value};
          }
          return {Type, constant(Type, Depth)};
        }

        std::uint32_t constant(std::uint32_t Type, std::size_t Depth)
        {
          if (Error || Type == NullId)
          {
            return NullId;
          }
          const auto TypeKind = Data.Records[Type].Kind;
          Record Value;
          if (TypeKind == RecordKind::BoolType)
          {
            Value = makeRecord(RecordKind::BoolConstant);
            if (consume("true"))
            {
              Value.Numbers[schema::BoolConstant::Value] = 1;
            }
            else if (consume("false"))
            {
              Value.Numbers[schema::BoolConstant::Value] = 0;
            }
            else
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Expected a boolean constant");
            }
          }
          else if (TypeKind == RecordKind::NullPointerType)
          {
            Value = makeRecord(RecordKind::NullPointerConstant);
            expect(recordTextName(RecordKind::NullPointerConstant));
          }
          else if (TypeKind == RecordKind::IntegerType)
          {
            Value = makeRecord(RecordKind::IntegerConstant);
            const auto Width = Data.Records[Type].Numbers[schema::IntegerType::Width];
            const auto WordCount = static_cast<std::size_t>((Width + 63) / 64);
            if (WordCount >= Options.MaxOperands - OperandCount)
            {
              fail(IRSerializationErrorCode::ResourceLimit, "Integer constant exceeds MaxOperands");
              return NullId;
            }
            Value.Numbers.assign(WordCount, 0);
            auto Literal = Current.Text;
            const bool Negative = Literal.starts_with("-");
            if (Negative)
            {
              Literal.remove_prefix(1);
            }
            if (Current.Quoted || Literal.empty())
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Expected an integer constant");
            }
            else if (Literal.starts_with("0x"))
            {
              if (Negative)
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Hexadecimal bit patterns cannot have a sign");
              }
              Literal.remove_prefix(2);
              if (Literal.empty())
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Expected hexadecimal integer digits");
              }
              for (std::size_t Index = 0; !Error && Index < Literal.size(); ++Index)
              {
                const int Digit = hexDigit(Literal[Literal.size() - 1 - Index]);
                if (Digit < 0 || (Index >= WordCount * 16 && Digit != 0))
                {
                  fail(IRSerializationErrorCode::InvalidSyntax, "Integer literal does not fit its type");
                }
                else if (Index < WordCount * 16)
                {
                  Value.Numbers[Index / 16] |= static_cast<std::uint64_t>(Digit) << ((Index % 16) * 4);
                }
              }
            }
            else
            {
              std::uint64_t Magnitude = 0;
              const auto Parsed = std::from_chars(Literal.data(), Literal.data() + Literal.size(), Magnitude);
              if (Parsed.ec != std::errc{} || Parsed.ptr != Literal.data() + Literal.size())
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Decimal integer magnitude exceeds 64 bits; use hexadecimal for wider values");
              }
              Value.Numbers[schema::IntegerConstant::WordsBegin] = Magnitude;
              const bool Signed = Data.Records[Type].Numbers[schema::IntegerType::Signed] != 0;
              if ((Negative && !Signed) || (Signed && Width <= 64 && (Magnitude > (std::uint64_t{1} << (Width - 1)) || (!Negative && Magnitude == (std::uint64_t{1} << (Width - 1))))))
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Decimal integer literal does not fit its signedness and width");
              }
            }
            if (!Error && Width % 64 && Value.Numbers.back() >> (Width % 64))
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Integer literal does not fit its type");
            }
            if (Negative && !Error)
            {
              std::uint64_t Carry = 1;
              for (auto &Word : Value.Numbers)
              {
                Word = ~Word;
                const auto Previous = Word;
                Word += Carry;
                Carry = Carry && Word < Previous;
              }
              if (Width % 64)
              {
                Value.Numbers.back() &= (std::uint64_t{1} << (Width % 64)) - 1;
              }
            }
            advance();
          }
          else if (TypeKind == RecordKind::FloatType)
          {
            Value = makeRecord(RecordKind::FloatConstant);
            expect(recordTextName(RecordKind::FloatConstant));
            expect("(");
            Value.Numbers[schema::FloatConstant::Bits] = number();
            expect(")");
          }
          else if (TypeKind == RecordKind::ArrayType)
          {
            if (consume(recordTextName(RecordKind::StringConstant)))
            {
              Value = makeRecord(RecordKind::StringConstant);
              Value.Strings[schema::StringConstant::Bytes] = string();
            }
            else
            {
              Value = makeRecord(RecordKind::ArrayConstant);
              expect(recordTextName(RecordKind::ArrayConstant));
              if (!consume("]"))
              {
                do
                {
                  if (!listRoom(Value.References.size()))
                  {
                    break;
                  }
                  Value.References.push_back(typedValue(Depth + 1, true).Value);
                } while (consume(","));
                expect("]");
              }
            }
          }
          else
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "This type has no inline constant syntax");
            return NullId;
          }
          Value.References[schema::Typed::Type] = Type;
          return appendRecord(std::move(Value));
        }

        std::uint32_t blockReference()
        {
          expect("label");
          return symbol(LocalSymbols, '%');
        }

        void parseInstruction(const std::string &Opcode, std::uint32_t ResultId, const std::string &ResultName, std::uint32_t BlockId)
        {
          Record Value;
          bool HasResult = true;
          bool CallWithoutSignature = false;
          if (Opcode == recordTextName(RecordKind::Alloca))
          {
            Value = makeRecord(RecordKind::Alloca);
            const auto Allocated = type();
            Record Pointer = makeRecord(RecordKind::PointerType);
            Pointer.Numbers[schema::PointerType::Mutable] = 1;
            Pointer.References[schema::PointerType::Element] = Allocated;
            Value.References[schema::Alloca::Type] = internType(std::move(Pointer));
            Value.Strings[schema::Alloca::Name] = ResultName;
          }
          else if (Opcode == recordTextName(RecordKind::Load))
          {
            Value = makeRecord(RecordKind::Load);
            Value.References[schema::Load::Type] = type();
            expect(",");
            Value.References[schema::Load::Pointer] = typedValue().Value;
          }
          else if (Opcode == recordTextName(RecordKind::Store))
          {
            Value = makeRecord(RecordKind::Store);
            HasResult = false;
            Value.References[schema::Store::Type] = primitive(RecordKind::VoidType);
            Value.References[schema::Store::Value] = typedValue().Value;
            expect(",");
            Value.References[schema::Store::Pointer] = typedValue().Value;
          }
          else if (Opcode == recordTextName(RecordKind::Neg))
          {
            Value = makeRecord(RecordKind::Neg);
            const auto Operand = typedValue();
            Value.References[schema::Neg::Type] = unqualified(Operand.Type);
            Value.References[schema::Neg::Operand] = Operand.Value;
          }
          else if (Opcode == recordTextName(RecordKind::ICmp))
          {
            Value = makeRecord(RecordKind::ICmp);
            const auto Predicate = parseEnum<ICmpPredicate>(Current.Text);
            if (!Predicate)
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Unknown integer comparison predicate");
            }
            Value.Numbers[schema::ICmp::Predicate] = Predicate.value_or(InvalidEnumValue);
            advance();
            Value.References[schema::ICmp::Type] = primitive(RecordKind::BoolType);
            Value.References[schema::ICmp::Left] = typedValue().Value;
            expect(",");
            Value.References[schema::ICmp::Right] = typedValue().Value;
          }
          else if (Opcode == recordTextName(RecordKind::Branch))
          {
            Value = makeRecord(RecordKind::Branch);
            HasResult = false;
            Value.References[schema::Branch::Type] = primitive(RecordKind::VoidType);
            Value.References[schema::Branch::Target] = blockReference();
          }
          else if (Opcode == recordTextName(RecordKind::CondBranch))
          {
            Value = makeRecord(RecordKind::CondBranch);
            HasResult = false;
            Value.References[schema::CondBranch::Type] = primitive(RecordKind::VoidType);
            Value.References[schema::CondBranch::Condition] = typedValue().Value;
            expect(",");
            Value.References[schema::CondBranch::TrueTarget] = blockReference();
            expect(",");
            Value.References[schema::CondBranch::FalseTarget] = blockReference();
          }
          else if (Opcode == recordTextName(RecordKind::Return))
          {
            Value = makeRecord(RecordKind::Return);
            HasResult = false;
            Value.References[schema::Return::Type] = primitive(RecordKind::VoidType);
            Value.References[schema::Return::Value] = consume(recordTextName(RecordKind::VoidType)) ? NullId : typedValue().Value;
          }
          else if (Opcode == recordTextName(RecordKind::Call))
          {
            Value = makeRecord(RecordKind::Call);
            Value.References[schema::Call::Type] = type();
            if (is("@"))
            {
              Value.References[schema::Call::Callee] = symbol(GlobalSymbols, '@');
            }
            else
            {
              Value.References[schema::Call::Callee] = symbol(LocalSymbols, '%');
            }
            expect("(");
            if (!consume(")"))
            {
              do
              {
                if (!listRoom(Value.References.size() + Value.Strings.size()))
                {
                  break;
                }
                Value.References.push_back(typedValue().Value);
                Value.Strings.emplace_back();
                attributes([&](std::string_view Name)
                {
                  if (Name == "name")
                  {
                    expect("=");
                    Value.Strings.back() = string();
                  }
                  else
                  {
                    unknownAttribute(Name);
                  }
                });
              } while (consume(","));
              expect(")");
            }
            CallWithoutSignature = true;
            HasResult = !Error && Data.Records[Value.References[schema::Call::Type]].Kind != RecordKind::VoidType;
          }
          else if (Opcode == recordTextName(RecordKind::GetElementPtr))
          {
            Value = makeRecord(RecordKind::GetElementPtr);
            Value.References[schema::GetElementPtr::SourceType] = type();
            expect(",");
            Value.References[schema::GetElementPtr::Pointer] = typedValue().Value;
            while (consume(","))
            {
              if (!listRoom(Value.References.size()))
              {
                break;
              }
              Value.References.push_back(typedValue().Value);
            }
          }
          else
          {
            fail(IRSerializationErrorCode::InvalidSyntax, "Unknown instruction '" + Opcode + "'");
            return;
          }
          Value.References[schema::Instruction::Block] = BlockId;
          attributes([&](std::string_view Name)
          {
            if (Name == "initialized" && Value.Kind == RecordKind::Alloca)
            {
              Value.Numbers[schema::Alloca::Initialized] = 1;
            }
            else if (Name == "name" && Value.Kind == RecordKind::Alloca)
            {
              expect("=");
              Value.Strings[schema::Alloca::Name] = string();
            }
            else if (Name == "inbounds" && Value.Kind == RecordKind::GetElementPtr)
            {
              Value.Numbers[schema::GetElementPtr::InBounds] = 1;
            }
            else if (Name == "signature" && Value.Kind == RecordKind::Call)
            {
              expect("=");
              Value.References[schema::Call::Signature] = type();
              CallWithoutSignature = false;
            }
            else
            {
              unknownAttribute(Name);
            }
          });
          if ((HasResult && ResultId == NullId) || (!HasResult && ResultId != NullId && Value.Kind != RecordKind::Call))
          {
            fail(IRSerializationErrorCode::InvalidSyntax, HasResult ? "Instruction requires a result name" : "Void instruction cannot define a result name");
          }
          if (ResultId == NullId)
          {
            ResultId = reserveRecord();
          }
          setRecord(ResultId, std::move(Value));
          if (CallWithoutSignature)
          {
            Calls.push_back(ResultId);
          }
          if (Opcode == recordTextName(RecordKind::GetElementPtr))
          {
            GetElementPtrs.push_back(ResultId);
          }
        }

        std::uint32_t unqualified(std::uint32_t Id)
        {
          if (Error || Id == NullId)
          {
            return NullId;
          }
          const auto &Value = Data.Records[Id];
          if (Value.Kind == RecordKind::QualifiedType)
          {
            return Value.References[schema::QualifiedType::Declaration];
          }
          if (Value.Numbers.empty() || Value.Kind == PendingKind || isNominal(Value.Kind) || Value.Numbers[schema::Type::Mutable])
          {
            return Id;
          }
          Record Copy = Value;
          Copy.Numbers[schema::Type::Mutable] = 1;
          return internType(std::move(Copy));
        }

        std::uint32_t valueType(std::uint32_t Id)
        {
          if (Id == NullId || Id >= Data.Records.size())
          {
            return NullId;
          }
          const auto &Value = Data.Records[Id];
          if (Value.Kind == RecordKind::Parameter)
          {
            const auto &Function = Data.Records[Value.References[schema::Parameter::Function]];
            if (Function.Kind != RecordKind::Function)
            {
              return NullId;
            }
            const auto &Signature = Data.Records[Function.References[schema::Function::Signature]];
            const auto Index = static_cast<std::size_t>(Value.Numbers[schema::Parameter::Index]) + schema::FunctionType::ParametersBegin;
            return Index < Signature.References.size() ? Signature.References[Index] : NullId;
          }
          if (Value.Kind == RecordKind::Block || Value.Kind == PendingKind || Value.References.empty())
          {
            return NullId;
          }
          return Value.References[schema::Typed::Type];
        }

        bool isMutableType(std::uint32_t Id) const
        {
          const auto &Value = Data.Records[Id];
          if (Value.Kind == RecordKind::QualifiedType)
          {
            return false;
          }
          return isNominal(Value.Kind) || Value.Numbers.empty() || Value.Numbers[schema::Type::Mutable] != 0;
        }

        std::uint32_t constQualified(std::uint32_t Id)
        {
          if (!isMutableType(Id))
          {
            return Id;
          }
          const auto &Value = Data.Records[Id];
          if (isNominal(Value.Kind))
          {
            Record Qualified = makeRecord(RecordKind::QualifiedType);
            Qualified.References[schema::QualifiedType::Declaration] = Id;
            return internType(std::move(Qualified));
          }
          if (Value.Numbers.empty())
          {
            fail(IRSerializationErrorCode::InvalidIR, "Getelementptr selected type cannot be const-qualified");
            return NullId;
          }
          Record Copy = Value;
          Copy.Numbers[schema::Type::Mutable] = 0;
          return internType(std::move(Copy));
        }

        void inferGetElementPtrType(std::uint32_t Id)
        {
          auto Selected = Data.Records[Id].References[schema::GetElementPtr::SourceType];
          bool Mutable = isMutableType(Selected);
          for (std::size_t Index = schema::GetElementPtr::IndicesBegin; Index < Data.Records[Id].References.size(); ++Index)
          {
            const auto IndexId = Data.Records[Id].References[Index];
            const auto IndexType = valueType(IndexId);
            if (IndexType == NullId || Data.Records[IndexType].Kind != RecordKind::IntegerType)
            {
              fail(IRSerializationErrorCode::InvalidIR, "Getelementptr index must be an integer value");
              return;
            }
            if (Index == schema::GetElementPtr::IndicesBegin)
            {
              continue;
            }
            if (Data.Records[Selected].Kind == RecordKind::QualifiedType)
            {
              Selected = Data.Records[Selected].References[schema::QualifiedType::Declaration];
            }
            const auto Kind = Data.Records[Selected].Kind;
            if (Kind == RecordKind::ArrayType)
            {
              Selected = Data.Records[Selected].References[schema::ArrayType::Element];
            }
            else if (isNominal(Kind))
            {
              const auto &IndexValue = Data.Records[IndexId];
              if (IndexValue.Kind != RecordKind::IntegerConstant)
              {
                fail(IRSerializationErrorCode::InvalidIR, "Getelementptr nominal field index must be an integer constant");
                return;
              }
              const auto &IntegerType = Data.Records[IndexType];
              const auto Bits = core::IntegerBits::fromWords(static_cast<std::uint32_t>(IntegerType.Numbers[schema::IntegerType::Width]), IndexValue.Numbers);
              const auto FieldIndex = Bits && (!IntegerType.Numbers[schema::IntegerType::Signed] || !Bits->isNegative()) ? Bits->tryGetUInt64() : std::nullopt;
              const auto Fields = NominalFields.find(Selected);
              if (!FieldIndex || Fields == NominalFields.end() || *FieldIndex >= Fields->second.size())
              {
                fail(IRSerializationErrorCode::InvalidIR, "Getelementptr field index is outside the nominal type");
                return;
              }
              Selected = Fields->second[static_cast<std::size_t>(*FieldIndex)];
            }
            else
            {
              fail(IRSerializationErrorCode::InvalidIR, "Getelementptr index does not select an aggregate element");
              return;
            }
            Mutable = Mutable && isMutableType(Selected);
          }
          if (!Mutable)
          {
            Selected = constQualified(Selected);
          }
          if (!Error)
          {
            Record Pointer = makeRecord(RecordKind::PointerType);
            Pointer.Numbers[schema::PointerType::Mutable] = 1;
            Pointer.References[schema::PointerType::Element] = Selected;
            const auto ResultType = internType(std::move(Pointer));
            if (!Error)
            {
              Data.Records[Id].References[schema::GetElementPtr::Type] = ResultType;
            }
          }
        }

        void resolveReferences()
        {
          if (Error)
          {
            return;
          }
          for (const auto &Value : Data.Records)
          {
            if (Value.Kind == PendingKind)
            {
              fail(IRSerializationErrorCode::InvalidReference, "Undefined global or nominal symbol");
              return;
            }
          }
          for (const auto Id : GetElementPtrs)
          {
            inferGetElementPtrType(Id);
            if (Error)
            {
              return;
            }
          }
          for (const auto Id : Calls)
          {
            const auto CalleeType = valueType(Data.Records[Id].References[schema::Call::Callee]);
            auto Signature = CalleeType;
            if (Signature != NullId && Data.Records[Signature].Kind == RecordKind::PointerType)
            {
              Signature = Data.Records[Signature].References[schema::PointerType::Element];
            }
            if (Signature == NullId || Data.Records[Signature].Kind != RecordKind::FunctionType)
            {
              fail(IRSerializationErrorCode::InvalidReference, "Call requires a callable callee or explicit signature");
              return;
            }
            Data.Records[Id].References[schema::Call::Signature] = Signature;
          }
          for (const auto &Check : Checks)
          {
            if (valueType(Check.Value) != Check.Type)
            {
              Error = IRSerializationError{IRSerializationErrorCode::InvalidIR, Check.Offset, "Operand type annotation does not match its referenced value"};
              return;
            }
          }
        }

        void orderDefinitions()
        {
          std::vector<std::uint32_t> Remap(Data.Records.size());
          for (std::size_t Index = 0; Index < DefinitionOrder.size(); ++Index)
          {
            Remap[DefinitionOrder[Index]] = static_cast<std::uint32_t>(Index);
          }
          std::vector<Record> Ordered;
          Ordered.reserve(Data.Records.size());
          for (const auto Id : DefinitionOrder)
          {
            auto &Value = Data.Records[Id];
            for (auto &Reference : Value.References)
            {
              if (Reference != NullId)
              {
                Reference = Remap[Reference];
              }
            }
            Ordered.push_back(std::move(Value));
          }
          Data.Records = std::move(Ordered);
        }

        std::string_view Input;
        const IRReadOptions &Options;
        std::size_t Position = 0;
        std::size_t OperandCount = 0;
        std::size_t StringBytes = 0;
        Token Current;
        bool NameWasQuoted = false;
        Model Data;
        std::optional<IRSerializationError> Error;
        std::unordered_map<std::string, std::uint32_t> GlobalSymbols;
        std::unordered_map<std::string, std::uint32_t> TypeSymbols;
        std::unordered_map<std::string, std::uint32_t> LocalSymbols;
        std::unordered_map<std::string, std::uint32_t> Types;
        std::unordered_map<std::uint32_t, std::vector<std::uint32_t>> NominalFields;
        std::vector<TypeCheck> Checks;
        std::vector<std::uint32_t> Calls;
        std::vector<std::uint32_t> GetElementPtrs;
        std::vector<std::uint32_t> DefinitionOrder;
    };
  } // namespace

  IRSerializationResult<Model> parseAssembly(std::string_view Text, const IRReadOptions &Options)
  {
    return AssemblyParser(Text, Options).parse();
  }
} // namespace ink::ir::detail
