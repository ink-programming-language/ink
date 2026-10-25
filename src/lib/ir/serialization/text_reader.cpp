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
          expect("module");
          Record ModuleRecord{RecordKind::Module, {}, {NullId, NullId}, stringList()};
          const auto ModuleId = reserveRecord();
          attributes([&](std::string_view Name)
          {
            if (Name == "init" || Name == "deinit")
            {
              expect("=");
              ModuleRecord.References[Name == "init" ? 0 : 1] = symbol(GlobalSymbols, '@');
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
            else if (is("declare") || is("define"))
            {
              parseFunction();
            }
            else if (consume("import"))
            {
              parseImport();
            }
            else if (consume("export"))
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
          return internType({Kind, {static_cast<std::uint64_t>(Mutable)}, {}, {}});
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
          const bool Mutable = !consume("const");
          if (consume("*"))
          {
            const auto Element = type(Depth + 1);
            return internType({RecordKind::PointerType, {Mutable}, {Element}, {}});
          }
          if (consume("&"))
          {
            const auto Element = type(Depth + 1);
            return internType({RecordKind::ReferenceType, {Mutable}, {Element}, {}});
          }
          if (consume("["))
          {
            const auto Count = number();
            expect("x");
            const auto Element = type(Depth + 1);
            expect("]");
            return internType({RecordKind::ArrayType, {Mutable, Count}, {Element}, {}});
          }
          if (consume("fn"))
          {
            Record Signature{RecordKind::FunctionType, {Mutable}, {NullId}, {}};
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
            Signature.References[0] = type(Depth + 1);
            return internType(std::move(Signature));
          }
          if (is("%"))
          {
            const auto Declaration = symbol(TypeSymbols, '%');
            return Mutable ? Declaration : internType({RecordKind::QualifiedType, {0}, {Declaration}, {}});
          }
          if (consume("bool"))
          {
            return primitive(RecordKind::BoolType, Mutable);
          }
          if (consume("void"))
          {
            return primitive(RecordKind::VoidType, Mutable);
          }
          if (consume("nullptr"))
          {
            return primitive(RecordKind::NullPointerType, Mutable);
          }
          if (is("meta") || is("variadic"))
          {
            const auto Kind = is("meta") ? RecordKind::MetaType : RecordKind::VariadicType;
            advance();
            if (!Mutable)
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "This type cannot be const-qualified");
            }
            return internType({Kind, {}, {}, {}});
          }
          auto Spelling = Current.Text;
          if (!Current.Quoted && Spelling.size() > 1 && (Spelling.front() == 'i' || Spelling.front() == 'u' || Spelling.front() == 'f'))
          {
            const char Prefix = Spelling.front();
            Spelling.remove_prefix(1);
            std::uint64_t Width = 0;
            const auto Parsed = std::from_chars(Spelling.data(), Spelling.data() + Spelling.size(), Width);
            if (Parsed.ec != std::errc{} || Parsed.ptr != Spelling.data() + Spelling.size() || Width == 0 || (Prefix == 'f' ? Width != 32 && Width != 64 : Width > core::IntegerBits::MaxBitWidth))
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Invalid numeric type width");
              return NullId;
            }
            advance();
            return Prefix == 'f' ? internType({RecordKind::FloatType, {Mutable, Width}, {}, {}}) : internType({RecordKind::IntegerType, {Mutable, Width, Prefix == 'i'}, {}, {}});
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
          if (consume("little"))
          {
            Data.ByteOrder = 0;
          }
          else if (consume("big"))
          {
            Data.ByteOrder = 1;
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
          Record Value{RecordKind::StructType, {0, 1, 0}, {}, {inferredName(Symbol, Quoted)}};
          bool HasSize = false;
          bool HasAlignment = false;
          if (consume("struct"))
          {
            Value.Kind = RecordKind::StructType;
          }
          else if (consume("class"))
          {
            Value.Kind = RecordKind::ClassType;
          }
          else if (consume("interface"))
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
              Value.Numbers[Name == "size" ? 0 : 1] = number();
              HasSize = HasSize || Name == "size";
              HasAlignment = HasAlignment || Name == "alignment";
            }
            else if (Name == "frozen")
            {
              Value.Numbers[2] = 1;
            }
            else if (Name == "name")
            {
              expect("=");
              Value.Strings[0] = string();
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
            expect("field");
            const auto FieldType = type();
            Record Field{RecordKind::Field, {0}, {Id, FieldType, NullId}, {string()}};
            bool HasOffset = false;
            attributes([&](std::string_view Name)
            {
              if (Name == "offset")
              {
                expect("=");
                Field.Numbers[0] = number();
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
              Field.References[2] = Initializer.Value;
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
          expect("global");
          Record Value{RecordKind::Global, {0}, {type(), NullId}, {inferredName(Symbol, Quoted)}};
          expect("=");
          if (!consume("none"))
          {
            Value.References[1] = typedValue(0, true).Value;
          }
          attributes([&](std::string_view Name)
          {
            if (Name == "string_storage")
            {
              Value.Numbers[0] = 1;
            }
            else if (Name == "name")
            {
              expect("=");
              Value.Strings[0] = string();
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
          setRecord(Id, {RecordKind::Import, {}, {Type}, stringList()});
        }

        void parseExport()
        {
          const auto Id = consume("type") ? type() : symbol(GlobalSymbols, '@');
          expect("as");
          appendRecord({RecordKind::Export, {}, {Id}, {string()}});
        }

        void parseFunction()
        {
          const bool HasBody = consume("define");
          if (!HasBody)
          {
            expect("declare");
          }
          Record Function{RecordKind::Function, {0, 0, 0, 0, 0, 0, std::numeric_limits<std::uint64_t>::max()}, {NullId, NullId}, {}};
          if (consume("import"))
          {
            Function.Numbers[0] = 1;
          }
          else if (consume("export"))
          {
            Function.Numbers[0] = 2;
          }
          if (consume("c"))
          {
            Function.Numbers[1] = 1;
          }
          else if (Current.Quoted)
          {
            if (string() != "C")
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Unknown language linkage");
            }
            Function.Numbers[1] = 1;
          }
          Record Signature{RecordKind::FunctionType, {1}, {type()}, {}};
          expect("@");
          const auto Symbol = name();
          const bool Quoted = NameWasQuoted;
          const auto Id = lookup(GlobalSymbols, Symbol, Quoted);
          Function.Strings.push_back(inferredName(Symbol, Quoted));
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
              Record Parameter{RecordKind::Parameter, {Signature.References.size() - 2}, {Id}, {inferredName(ParameterSymbol, ParameterQuoted)}};
              attributes([&](std::string_view Name)
              {
                if (Name == "name")
                {
                  expect("=");
                  Parameter.Strings[0] = string();
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
            if (Name == "private")
            {
              Function.Numbers[2] = 1;
            }
            else if (Name == "owner")
            {
              expect("=");
              Function.References[1] = symbol(TypeSymbols, '%');
            }
            else if (Name == "virtual" || Name == "abstract")
            {
              if (Function.Numbers[3] != 0)
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Conflicting method dispatch attributes");
              }
              Function.Numbers[3] = Name == "virtual" ? 1 : 2;
            }
            else if (Name == "override" || Name == "final")
            {
              Function.Numbers[Name == "override" ? 4 : 5] = 1;
            }
            else if (Name == "slot")
            {
              expect("=");
              Function.Numbers[6] = number();
              if (Function.Numbers[6] > std::numeric_limits<std::uint32_t>::max())
              {
                fail(IRSerializationErrorCode::InvalidSyntax, "Virtual slot exceeds 32 bits");
              }
            }
            else if (Name == "signature_const")
            {
              Signature.Numbers[0] = 0;
            }
            else if (Name == "name")
            {
              expect("=");
              Function.Strings[0] = string();
            }
            else
            {
              unknownAttribute(Name);
            }
          });
          Function.References[0] = internType(std::move(Signature));
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
              Record Block{RecordKind::Block, {}, {FunctionId}, {inferredName(Word, WordQuoted)}};
              attributes([&](std::string_view Name)
              {
                if (Name == "name")
                {
                  expect("=");
                  Block.Strings[0] = string();
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
          Record Value{RecordKind::BoolConstant, {}, {Type}, {}};
          if (TypeKind == RecordKind::BoolType)
          {
            Value.Kind = RecordKind::BoolConstant;
            if (consume("true"))
            {
              Value.Numbers = {1};
            }
            else if (consume("false"))
            {
              Value.Numbers = {0};
            }
            else
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Expected a boolean constant");
            }
          }
          else if (TypeKind == RecordKind::NullPointerType)
          {
            Value.Kind = RecordKind::NullPointerConstant;
            expect("null");
          }
          else if (TypeKind == RecordKind::IntegerType)
          {
            Value.Kind = RecordKind::IntegerConstant;
            const auto Width = Data.Records[Type].Numbers[1];
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
              Value.Numbers[0] = Magnitude;
              const bool Signed = Data.Records[Type].Numbers[2] != 0;
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
            Value.Kind = RecordKind::FloatConstant;
            expect("bits");
            expect("(");
            Value.Numbers = {number()};
            expect(")");
          }
          else if (TypeKind == RecordKind::ArrayType)
          {
            if (consume("c"))
            {
              Value.Kind = RecordKind::StringConstant;
              Value.Strings = {string()};
            }
            else
            {
              Value.Kind = RecordKind::ArrayConstant;
              expect("[");
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
          }
          return appendRecord(std::move(Value));
        }

        std::uint32_t blockReference()
        {
          expect("label");
          return symbol(LocalSymbols, '%');
        }

        void parseInstruction(const std::string &Opcode, std::uint32_t ResultId, const std::string &ResultName, std::uint32_t BlockId)
        {
          Record Value{RecordKind::Return, {}, {NullId, BlockId}, {}};
          bool HasResult = true;
          bool CallWithoutSignature = false;
          if (Opcode == "alloca")
          {
            Value.Kind = RecordKind::Alloca;
            Value.Numbers = {0};
            const auto Allocated = type();
            Value.References[0] = internType({RecordKind::PointerType, {1}, {Allocated}, {}});
            Value.Strings = {ResultName};
          }
          else if (Opcode == "load")
          {
            Value.Kind = RecordKind::Load;
            Value.References[0] = type();
            expect(",");
            Value.References.push_back(typedValue().Value);
          }
          else if (Opcode == "store")
          {
            Value.Kind = RecordKind::Store;
            HasResult = false;
            Value.References[0] = primitive(RecordKind::VoidType);
            Value.References.push_back(typedValue().Value);
            expect(",");
            Value.References.push_back(typedValue().Value);
          }
          else if (Opcode == "neg")
          {
            Value.Kind = RecordKind::Neg;
            const auto Operand = typedValue();
            Value.References[0] = unqualified(Operand.Type);
            Value.References.push_back(Operand.Value);
          }
          else if (Opcode == "icmp")
          {
            Value.Kind = RecordKind::ICmp;
            constexpr std::string_view Predicates[] = {
                "eq",
                "ne",
                "ugt",
                "uge",
                "ult",
                "ule",
                "sgt",
                "sge",
                "slt",
                "sle",
            };
            const auto It = std::find(std::begin(Predicates), std::end(Predicates), Current.Text);
            if (It == std::end(Predicates))
            {
              fail(IRSerializationErrorCode::InvalidSyntax, "Unknown integer comparison predicate");
            }
            Value.Numbers = {static_cast<std::uint64_t>(It - std::begin(Predicates))};
            advance();
            Value.References[0] = primitive(RecordKind::BoolType);
            Value.References.push_back(typedValue().Value);
            expect(",");
            Value.References.push_back(typedValue().Value);
          }
          else if (Opcode == "br")
          {
            Value.Kind = RecordKind::Branch;
            HasResult = false;
            Value.References[0] = primitive(RecordKind::VoidType);
            Value.References.push_back(blockReference());
          }
          else if (Opcode == "br_if")
          {
            Value.Kind = RecordKind::CondBranch;
            HasResult = false;
            Value.References[0] = primitive(RecordKind::VoidType);
            Value.References.push_back(typedValue().Value);
            expect(",");
            Value.References.push_back(blockReference());
            expect(",");
            Value.References.push_back(blockReference());
          }
          else if (Opcode == "ret")
          {
            Value.Kind = RecordKind::Return;
            HasResult = false;
            Value.References[0] = primitive(RecordKind::VoidType);
            Value.References.push_back(consume("void") ? NullId : typedValue().Value);
          }
          else if (Opcode == "call")
          {
            Value.Kind = RecordKind::Call;
            Value.References[0] = type();
            Value.References.push_back(NullId);
            if (is("@"))
            {
              Value.References.push_back(symbol(GlobalSymbols, '@'));
            }
            else
            {
              Value.References.push_back(symbol(LocalSymbols, '%'));
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
            HasResult = !Error && Data.Records[Value.References[0]].Kind != RecordKind::VoidType;
          }
          else if (Opcode == "getelementptr")
          {
            Value.Kind = RecordKind::GetElementPtr;
            Value.Numbers = {0};
            Value.References.push_back(type());
            expect(",");
            Value.References.push_back(typedValue().Value);
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
          }
          attributes([&](std::string_view Name)
          {
            if (Name == "initialized" && Value.Kind == RecordKind::Alloca)
            {
              Value.Numbers[0] = 1;
            }
            else if (Name == "name" && Value.Kind == RecordKind::Alloca)
            {
              expect("=");
              Value.Strings[0] = string();
            }
            else if (Name == "inbounds" && Value.Kind == RecordKind::GetElementPtr)
            {
              Value.Numbers[0] = 1;
            }
            else if (Name == "signature" && Value.Kind == RecordKind::Call)
            {
              expect("=");
              Value.References[2] = type();
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
          if (Opcode == "getelementptr")
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
            return Value.References[0];
          }
          if (Value.Numbers.empty() || Value.Kind == PendingKind || Value.Kind == RecordKind::ClassType || Value.Kind == RecordKind::StructType || Value.Kind == RecordKind::InterfaceType || Value.Numbers[0])
          {
            return Id;
          }
          Record Copy = Value;
          Copy.Numbers[0] = 1;
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
            const auto &Function = Data.Records[Value.References[0]];
            if (Function.Kind != RecordKind::Function)
            {
              return NullId;
            }
            const auto &Signature = Data.Records[Function.References[0]];
            const auto Index = static_cast<std::size_t>(Value.Numbers[0]) + 1;
            return Index < Signature.References.size() ? Signature.References[Index] : NullId;
          }
          if (Value.Kind == RecordKind::Block || Value.Kind == PendingKind || Value.References.empty())
          {
            return NullId;
          }
          return Value.References[0];
        }

        bool isMutableType(std::uint32_t Id) const
        {
          const auto &Value = Data.Records[Id];
          if (Value.Kind == RecordKind::QualifiedType)
          {
            return false;
          }
          return Value.Kind == RecordKind::ClassType || Value.Kind == RecordKind::StructType || Value.Kind == RecordKind::InterfaceType || Value.Numbers.empty() || Value.Numbers[0] != 0;
        }

        std::uint32_t constQualified(std::uint32_t Id)
        {
          if (!isMutableType(Id))
          {
            return Id;
          }
          const auto &Value = Data.Records[Id];
          if (Value.Kind == RecordKind::ClassType || Value.Kind == RecordKind::StructType || Value.Kind == RecordKind::InterfaceType)
          {
            return internType({RecordKind::QualifiedType, {0}, {Id}, {}});
          }
          if (Value.Numbers.empty())
          {
            fail(IRSerializationErrorCode::InvalidIR, "Getelementptr selected type cannot be const-qualified");
            return NullId;
          }
          Record Copy = Value;
          Copy.Numbers[0] = 0;
          return internType(std::move(Copy));
        }

        void inferGetElementPtrType(std::uint32_t Id)
        {
          auto Selected = Data.Records[Id].References[2];
          bool Mutable = isMutableType(Selected);
          for (std::size_t Index = 4; Index < Data.Records[Id].References.size(); ++Index)
          {
            const auto IndexId = Data.Records[Id].References[Index];
            const auto IndexType = valueType(IndexId);
            if (IndexType == NullId || Data.Records[IndexType].Kind != RecordKind::IntegerType)
            {
              fail(IRSerializationErrorCode::InvalidIR, "Getelementptr index must be an integer value");
              return;
            }
            if (Index == 4)
            {
              continue;
            }
            if (Data.Records[Selected].Kind == RecordKind::QualifiedType)
            {
              Selected = Data.Records[Selected].References[0];
            }
            const auto Kind = Data.Records[Selected].Kind;
            if (Kind == RecordKind::ArrayType)
            {
              Selected = Data.Records[Selected].References[0];
            }
            else if (Kind == RecordKind::ClassType || Kind == RecordKind::StructType || Kind == RecordKind::InterfaceType)
            {
              const auto &IndexValue = Data.Records[IndexId];
              if (IndexValue.Kind != RecordKind::IntegerConstant)
              {
                fail(IRSerializationErrorCode::InvalidIR, "Getelementptr nominal field index must be an integer constant");
                return;
              }
              const auto &IntegerType = Data.Records[IndexType];
              const auto Bits = core::IntegerBits::fromWords(static_cast<std::uint32_t>(IntegerType.Numbers[1]), IndexValue.Numbers);
              const auto FieldIndex = Bits && (!IntegerType.Numbers[2] || !Bits->isNegative()) ? Bits->tryGetUInt64() : std::nullopt;
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
            const auto ResultType = internType({RecordKind::PointerType, {1}, {Selected}, {}});
            if (!Error)
            {
              Data.Records[Id].References[0] = ResultType;
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
            const auto CalleeType = valueType(Data.Records[Id].References[3]);
            auto Signature = CalleeType;
            if (Signature != NullId && Data.Records[Signature].Kind == RecordKind::PointerType)
            {
              Signature = Data.Records[Signature].References[0];
            }
            if (Signature == NullId || Data.Records[Signature].Kind != RecordKind::FunctionType)
            {
              fail(IRSerializationErrorCode::InvalidReference, "Call requires a callable callee or explicit signature");
              return;
            }
            Data.Records[Id].References[2] = Signature;
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
