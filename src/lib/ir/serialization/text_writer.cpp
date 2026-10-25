#include "text_format.h"

#include "ink/core/config.h"

#include <charconv>
#include <limits>
#include <string>
#include <unordered_set>
#include <utility>

namespace ink::ir::detail
{
  namespace
  {
    bool isNominal(RecordKind Kind)
    {
      return Kind == RecordKind::ClassType || Kind == RecordKind::InterfaceType || Kind == RecordKind::StructType;
    }

    bool isType(RecordKind Kind)
    {
      return Kind >= RecordKind::BoolType && Kind <= RecordKind::QualifiedType;
    }

    bool isInstruction(RecordKind Kind)
    {
      return Kind >= RecordKind::Alloca && Kind <= RecordKind::GetElementPtr;
    }

    bool isPlainName(std::string_view Name)
    {
      if (Name.empty() || (Name.front() >= '0' && Name.front() <= '9') || Name.find("...") != std::string_view::npos || Name.find("->") != std::string_view::npos)
      {
        return false;
      }
      for (const unsigned char Character : Name)
      {
        if (!((Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || (Character >= '0' && Character <= '9') || Character == '_' || Character == '.' || Character == '$' || Character == '-'))
        {
          return false;
        }
      }
      return true;
    }

    struct Symbol
    {
        std::string Text;
        bool Quoted = false;
        bool ExplicitName = false;
    };

    struct SymbolScope
    {
        std::unordered_set<std::string> Used;
        std::size_t NextAnonymous = 0;
    };

    enum class RenderKind
    {
      Text,
      Number,
      Type,
      Value,
      TypedValue,
    };

    struct RenderStep
    {
        RenderKind Kind;
        std::uint64_t Number = 0;
        std::string_view Text;
    };

    class AssemblyWriter
    {
      public:
        AssemblyWriter(const Model &Data, std::size_t MaxBytes)
            : Data(Data),
              MaxBytes(MaxBytes),
              Symbols(Data.Records.size()),
              Parameters(Data.Records.size()),
              Blocks(Data.Records.size()),
              Instructions(Data.Records.size()),
              Fields(Data.Records.size()),
              Referenced(Data.Records.size(), false)
        {
        }

        IRSerializationResult<std::string> write()
        {
          index();
          append("ink_ir ");
          append(INK_IR_VERSION);
          append("\ntarget(pointer_bits = ");
          number(Data.PointerBits);
          append(", byte_order = ");
          append(Data.ByteOrder == 0 ? "little" : "big");
          append(", native_abi = ");
          number(Data.NativeAbi ? 1 : 0);
          append(", alignments = [");
          for (std::size_t Index = 0; Index < Data.Alignments.size(); ++Index)
          {
            separator(Index);
            number(Data.Alignments[Index]);
          }
          append("])\nmodule ");
          strings(Data.Records[0].Strings);
          bool Attributes = false;
          for (std::size_t Index = 0; Index < 2; ++Index)
          {
            if (Data.Records[0].References[Index] != NullId)
            {
              attribute(Attributes, Index == 0 ? "init = " : "deinit = ");
              reference(Data.Records[0].References[Index]);
            }
          }
          endAttributes(Attributes);
          append("\n{\n");
          bool First = true;
          for (std::uint32_t Id = 1; Id < Data.Records.size() && !Failed; ++Id)
          {
            if (isNominal(Data.Records[Id].Kind))
            {
              topSeparator(First);
              nominal(Id);
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size() && !Failed; ++Id)
          {
            if (Data.Records[Id].Kind == RecordKind::Global)
            {
              topSeparator(First);
              global(Id);
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size() && !Failed; ++Id)
          {
            if (Data.Records[Id].Kind == RecordKind::Function)
            {
              topSeparator(First);
              function(Id);
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size() && !Failed; ++Id)
          {
            const auto &Entry = Data.Records[Id];
            if (Entry.Kind == RecordKind::Import)
            {
              topSeparator(First);
              append("  import ");
              reference(Id);
              append(" : ");
              type(Entry.References[0]);
              append(" from ");
              strings(Entry.Strings);
              append('\n');
            }
            else if (Entry.Kind == RecordKind::Export)
            {
              topSeparator(First);
              append("  export ");
              const auto Declaration = Entry.References[0];
              if (isType(Data.Records[Declaration].Kind))
              {
                append("type ");
                type(Declaration);
              }
              else
              {
                reference(Declaration);
              }
              append(" as ");
              string(Entry.Strings[0]);
              append('\n');
            }
          }
          append("}\n");
          if (Failed)
          {
            return std::unexpected(IRSerializationError{IRSerializationErrorCode::ResourceLimit, 0, "Text output exceeds MaxBytes"});
          }
          return std::move(Output);
        }

      private:
        void append(std::string_view Text)
        {
          if (Failed || Text.size() > MaxBytes - Output.size())
          {
            Failed = true;
            return;
          }
          Output.append(Text);
        }

        void append(char Character)
        {
          append(std::string_view(&Character, 1));
        }

        void number(std::uint64_t Number)
        {
          char Buffer[20];
          const auto Result = std::to_chars(Buffer, Buffer + sizeof(Buffer), Number);
          append(std::string_view(Buffer, static_cast<std::size_t>(Result.ptr - Buffer)));
        }

        void hexadecimal(std::uint64_t Number, bool Pad = false)
        {
          constexpr char Digits[] = "0123456789ABCDEF";
          char Buffer[16];
          std::size_t Begin = sizeof(Buffer);
          do
          {
            Buffer[--Begin] = Digits[Number & 15];
            Number >>= 4;
          } while (Number != 0 || (Pad && Begin != 0));
          append(std::string_view(Buffer + Begin, sizeof(Buffer) - Begin));
        }

        void string(std::string_view Text)
        {
          constexpr char Digits[] = "0123456789ABCDEF";
          append('"');
          for (const unsigned char Byte : Text)
          {
            if (Byte >= 0x20 && Byte <= 0x7E && Byte != '"' && Byte != '\\')
            {
              append(static_cast<char>(Byte));
            }
            else
            {
              append('\\');
              append(Digits[Byte >> 4]);
              append(Digits[Byte & 15]);
            }
            if (Failed)
            {
              break;
            }
          }
          append('"');
        }

        void strings(const std::vector<std::string> &Values)
        {
          append('[');
          for (std::size_t Index = 0; Index < Values.size() && !Failed; ++Index)
          {
            separator(Index);
            string(Values[Index]);
          }
          append(']');
        }

        void separator(std::size_t Index)
        {
          if (Index != 0)
          {
            append(", ");
          }
        }

        void topSeparator(bool &First)
        {
          if (!First)
          {
            append('\n');
          }
          First = false;
        }

        void attribute(bool &Started, std::string_view Name)
        {
          append(Started ? ", " : " [");
          Started = true;
          append(Name);
        }

        void endAttributes(bool Started)
        {
          if (Started)
          {
            append(']');
          }
        }

        void nameAttribute(std::uint32_t Id, bool &Started)
        {
          if (Symbols[Id].ExplicitName)
          {
            attribute(Started, "name = ");
            string(Data.Records[Id].Strings[0]);
          }
        }

        void assign(std::uint32_t Id, SymbolScope &Scope)
        {
          const auto &Entry = Data.Records[Id];
          const bool HasName = isNominal(Entry.Kind) || Entry.Kind == RecordKind::Function || Entry.Kind == RecordKind::Global || Entry.Kind == RecordKind::Parameter || Entry.Kind == RecordKind::Block || Entry.Kind == RecordKind::Alloca;
          const std::string_view Name = HasName ? std::string_view(Entry.Strings[0]) : std::string_view{};
          auto &Result = Symbols[Id];
          if (Name.empty())
          {
            do
            {
              Result.Text = std::to_string(Scope.NextAnonymous++);
            } while (!Scope.Used.insert(Result.Text).second);
            return;
          }
          Result.Text = Name;
          for (std::size_t Suffix = 1; !Scope.Used.insert(Result.Text).second; ++Suffix)
          {
            Result.Text = std::string(Name) + "." + std::to_string(Suffix);
            Result.ExplicitName = true;
          }
          Result.Quoted = !isPlainName(Result.Text);
        }

        void index()
        {
          SymbolScope Globals;
          SymbolScope Types;
          for (const auto &Entry : Data.Records)
          {
            for (auto Reference : Entry.References)
            {
              if (Reference != NullId)
              {
                Referenced[Reference] = true;
              }
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size(); ++Id)
          {
            const auto &Entry = Data.Records[Id];
            if (isNominal(Entry.Kind))
            {
              assign(Id, Types);
            }
            else if (Entry.Kind == RecordKind::Function || Entry.Kind == RecordKind::Global || Entry.Kind == RecordKind::Import)
            {
              assign(Id, Globals);
            }
            else if (Entry.Kind == RecordKind::Parameter)
            {
              Parameters[Entry.References[0]].push_back(Id);
            }
            else if (Entry.Kind == RecordKind::Block)
            {
              Blocks[Entry.References[0]].push_back(Id);
            }
            else if (Entry.Kind == RecordKind::Field)
            {
              Fields[Entry.References[0]].push_back(Id);
            }
            else if (isInstruction(Entry.Kind))
            {
              Instructions[Entry.References[1]].push_back(Id);
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size(); ++Id)
          {
            if (Data.Records[Id].Kind != RecordKind::Function)
            {
              continue;
            }
            SymbolScope Locals;
            for (auto Parameter : Parameters[Id])
            {
              assign(Parameter, Locals);
            }
            for (auto Block : Blocks[Id])
            {
              assign(Block, Locals);
              for (auto Instruction : Instructions[Block])
              {
                if (hasResult(Instruction))
                {
                  assign(Instruction, Locals);
                }
              }
            }
          }
        }

        void symbol(std::uint32_t Id)
        {
          if (Symbols[Id].Quoted)
          {
            string(Symbols[Id].Text);
          }
          else
          {
            append(Symbols[Id].Text);
          }
        }

        void reference(std::uint32_t Id)
        {
          const auto Kind = Data.Records[Id].Kind;
          append(Kind == RecordKind::Function || Kind == RecordKind::Global || Kind == RecordKind::Import ? '@' : '%');
          symbol(Id);
        }

        std::uint32_t valueType(std::uint32_t Id) const
        {
          const auto &Entry = Data.Records[Id];
          if (Entry.Kind == RecordKind::Parameter)
          {
            return Data.Records[Data.Records[Entry.References[0]].References[0]].References[static_cast<std::size_t>(Entry.Numbers[0]) + 1];
          }
          return Entry.References[0];
        }

        void type(std::uint32_t Id)
        {
          render({RenderKind::Type, Id, {}});
        }

        void typedValue(std::uint32_t Id)
        {
          render({RenderKind::TypedValue, Id, {}});
        }

        void value(std::uint32_t Id)
        {
          render({RenderKind::Value, Id, {}});
        }

        void render(RenderStep First)
        {
          std::vector<RenderStep> Work{First};
          while (!Work.empty() && !Failed)
          {
            const auto Step = Work.back();
            Work.pop_back();
            if (Step.Kind == RenderKind::Text)
            {
              append(Step.Text);
              continue;
            }
            if (Step.Kind == RenderKind::Number)
            {
              number(Step.Number);
              continue;
            }
            const auto Id = static_cast<std::uint32_t>(Step.Number);
            const auto &Entry = Data.Records[Id];
            if (Step.Kind == RenderKind::TypedValue)
            {
              Work.push_back({RenderKind::Value, Id, {}});
              Work.push_back({RenderKind::Text, 0, " "});
              Work.push_back({RenderKind::Type, valueType(Id), {}});
              continue;
            }
            if (Step.Kind == RenderKind::Type)
            {
              renderType(Id, Work);
              continue;
            }
            switch (Entry.Kind)
            {
            case RecordKind::BoolConstant:
              append(Entry.Numbers[0] ? "true" : "false");
              break;
            case RecordKind::IntegerConstant:
              integer(Entry);
              break;
            case RecordKind::FloatConstant:
              append("bits(0x");
              hexadecimal(Entry.Numbers[0]);
              append(')');
              break;
            case RecordKind::NullPointerConstant:
              append("null");
              break;
            case RecordKind::StringConstant:
              append('c');
              string(Entry.Strings[0]);
              break;
            case RecordKind::ArrayConstant:
              append('[');
              Work.push_back({RenderKind::Text, 0, "]"});
              for (std::size_t Index = Entry.References.size(); Index > 1; --Index)
              {
                Work.push_back({RenderKind::TypedValue, Entry.References[Index - 1], {}});
                if (Index > 2)
                {
                  Work.push_back({RenderKind::Text, 0, ", "});
                }
              }
              break;
            default:
              reference(Id);
              break;
            }
          }
        }

        void renderType(std::uint32_t Id, std::vector<RenderStep> &Work)
        {
          const auto &Entry = Data.Records[Id];
          if (isNominal(Entry.Kind))
          {
            reference(Id);
            return;
          }
          if (Entry.Kind != RecordKind::MetaType && Entry.Kind != RecordKind::VariadicType && !Entry.Numbers[0])
          {
            append("const ");
          }
          switch (Entry.Kind)
          {
          case RecordKind::BoolType:
            append("bool");
            break;
          case RecordKind::IntegerType:
            append(Entry.Numbers[2] ? 'i' : 'u');
            number(Entry.Numbers[1]);
            break;
          case RecordKind::FloatType:
            append('f');
            number(Entry.Numbers[1]);
            break;
          case RecordKind::VoidType:
            append("void");
            break;
          case RecordKind::NullPointerType:
            append("nullptr");
            break;
          case RecordKind::MetaType:
            append("meta");
            break;
          case RecordKind::VariadicType:
            append("variadic");
            break;
          case RecordKind::PointerType:
          case RecordKind::ReferenceType:
            append(Entry.Kind == RecordKind::PointerType ? '*' : '&');
            Work.push_back({RenderKind::Type, Entry.References[0], {}});
            break;
          case RecordKind::QualifiedType:
            reference(Entry.References[0]);
            break;
          case RecordKind::ArrayType:
            append('[');
            number(Entry.Numbers[1]);
            append(" x ");
            Work.push_back({RenderKind::Text, 0, "]"});
            Work.push_back({RenderKind::Type, Entry.References[0], {}});
            break;
          case RecordKind::FunctionType:
            append("fn(");
            Work.push_back({RenderKind::Type, Entry.References[0], {}});
            Work.push_back({RenderKind::Text, 0, ") -> "});
            for (std::size_t Index = Entry.References.size(); Index > 1; --Index)
            {
              if (Entry.Numbers[Index - 1])
              {
                Work.push_back({RenderKind::Text, 0, "..."});
              }
              Work.push_back({RenderKind::Type, Entry.References[Index - 1], {}});
              if (Index > 2)
              {
                Work.push_back({RenderKind::Text, 0, ", "});
              }
            }
            break;
          default:
            break;
          }
        }

        void integer(const Record &Entry)
        {
          const auto &Type = Data.Records[Entry.References[0]];
          const auto Width = Type.Numbers[1];
          if (Width > 64)
          {
            append("0x");
            bool First = true;
            for (auto Index = Entry.Numbers.size(); Index != 0; --Index)
            {
              if (First && Entry.Numbers[Index - 1] == 0 && Index != 1)
              {
                continue;
              }
              hexadecimal(Entry.Numbers[Index - 1], !First);
              First = false;
            }
            return;
          }
          const auto Bits = Entry.Numbers[0];
          if (Type.Numbers[2] && (Bits & (std::uint64_t{1} << (Width - 1))))
          {
            append('-');
            const auto Mask = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
            number((~Bits + 1) & Mask);
          }
          else
          {
            number(Bits);
          }
        }

        void nominal(std::uint32_t Id)
        {
          const auto &Entry = Data.Records[Id];
          append("  ");
          reference(Id);
          append(" = type ");
          append(Entry.Kind == RecordKind::ClassType ? "class" : Entry.Kind == RecordKind::InterfaceType ? "interface" : "struct");
          bool Attributes = false;
          attribute(Attributes, "size = ");
          number(Entry.Numbers[0]);
          attribute(Attributes, "alignment = ");
          number(Entry.Numbers[1]);
          if (Entry.Numbers[2])
          {
            attribute(Attributes, "frozen");
          }
          nameAttribute(Id, Attributes);
          endAttributes(Attributes);
          append("\n  {\n");
          for (auto FieldId : Fields[Id])
          {
            const auto &Field = Data.Records[FieldId];
            append("    field ");
            type(Field.References[1]);
            append(' ');
            string(Field.Strings[0]);
            append(" [offset = ");
            number(Field.Numbers[0]);
            append("] = ");
            if (Field.References[2] == NullId)
            {
              append("none");
            }
            else
            {
              typedValue(Field.References[2]);
            }
            append('\n');
          }
          append("  }\n");
        }

        void global(std::uint32_t Id)
        {
          const auto &Entry = Data.Records[Id];
          append("  ");
          reference(Id);
          append(" = global ");
          type(Entry.References[0]);
          append(" = ");
          if (Entry.References[1] == NullId)
          {
            append("none");
          }
          else
          {
            typedValue(Entry.References[1]);
          }
          bool Attributes = false;
          if (Entry.Numbers[0])
          {
            attribute(Attributes, "string_storage");
          }
          nameAttribute(Id, Attributes);
          endAttributes(Attributes);
          append('\n');
        }

        void function(std::uint32_t Id)
        {
          const auto &Entry = Data.Records[Id];
          const auto &Signature = Data.Records[Entry.References[0]];
          append(Blocks[Id].empty() ? "  declare " : "  define ");
          if (Entry.Numbers[0] != 0)
          {
            append(Entry.Numbers[0] == 1 ? "import " : "export ");
          }
          if (Entry.Numbers[1] != 0)
          {
            append("\"C\" ");
          }
          type(Signature.References[0]);
          append(' ');
          reference(Id);
          append('(');
          for (std::size_t Index = 0; Index < Parameters[Id].size(); ++Index)
          {
            separator(Index);
            type(Signature.References[Index + 1]);
            if (Signature.Numbers[Index + 1])
            {
              append("...");
            }
            append(' ');
            reference(Parameters[Id][Index]);
            bool ParameterAttributes = false;
            nameAttribute(Parameters[Id][Index], ParameterAttributes);
            endAttributes(ParameterAttributes);
          }
          append(')');
          bool Attributes = false;
          if (Entry.Numbers[2] != 0)
          {
            attribute(Attributes, "private");
          }
          if (Entry.References[1] != NullId)
          {
            attribute(Attributes, "owner = ");
            reference(Entry.References[1]);
          }
          if (Entry.Numbers[3] != 0)
          {
            attribute(Attributes, Entry.Numbers[3] == 1 ? "virtual" : "abstract");
          }
          if (Entry.Numbers[4])
          {
            attribute(Attributes, "override");
          }
          if (Entry.Numbers[5])
          {
            attribute(Attributes, "final");
          }
          if (Entry.Numbers[6] != std::numeric_limits<std::uint64_t>::max())
          {
            attribute(Attributes, "slot = ");
            number(Entry.Numbers[6]);
          }
          if (!Signature.Numbers[0])
          {
            attribute(Attributes, "signature_const");
          }
          nameAttribute(Id, Attributes);
          endAttributes(Attributes);
          append('\n');
          if (Blocks[Id].empty())
          {
            return;
          }
          append("  {\n");
          for (auto Block : Blocks[Id])
          {
            append("  ");
            symbol(Block);
            append(':');
            bool BlockAttributes = false;
            nameAttribute(Block, BlockAttributes);
            endAttributes(BlockAttributes);
            append('\n');
            for (auto Instruction : Instructions[Block])
            {
              instruction(Instruction);
            }
          }
          append("  }\n");
        }

        void instruction(std::uint32_t Id)
        {
          const auto &Entry = Data.Records[Id];
          const auto &R = Entry.References;
          append("    ");
          if (hasResult(Id))
          {
            reference(Id);
            append(" = ");
          }
          bool Attributes = false;
          switch (Entry.Kind)
          {
          case RecordKind::Alloca:
            append("alloca ");
            type(Data.Records[R[0]].References[0]);
            if (Entry.Numbers[0])
            {
              attribute(Attributes, "initialized");
            }
            nameAttribute(Id, Attributes);
            break;
          case RecordKind::Load:
            append("load ");
            type(R[0]);
            append(", ");
            typedValue(R[2]);
            break;
          case RecordKind::Store:
            append("store ");
            typedValue(R[2]);
            append(", ");
            typedValue(R[3]);
            break;
          case RecordKind::Neg:
            append("neg ");
            typedValue(R[2]);
            break;
          case RecordKind::ICmp:
          {
            static constexpr std::string_view Predicates[] = {
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
            append("icmp ");
            append(Predicates[Entry.Numbers[0]]);
            append(' ');
            typedValue(R[2]);
            append(", ");
            typedValue(R[3]);
            break;
          }
          case RecordKind::Branch:
            append("br label ");
            reference(R[2]);
            break;
          case RecordKind::CondBranch:
            append("br_if ");
            typedValue(R[2]);
            append(", label ");
            reference(R[3]);
            append(", label ");
            reference(R[4]);
            break;
          case RecordKind::Return:
            append("ret ");
            if (R[2] == NullId)
            {
              append("void");
            }
            else
            {
              typedValue(R[2]);
            }
            break;
          case RecordKind::Call:
            append("call ");
            type(R[0]);
            append(' ');
            value(R[3]);
            append('(');
            for (std::size_t Index = 4; Index < R.size(); ++Index)
            {
              separator(Index - 4);
              typedValue(R[Index]);
              if (!Entry.Strings[Index - 4].empty())
              {
                append(" [name = ");
                string(Entry.Strings[Index - 4]);
                append(']');
              }
            }
            append(')');
            if (Data.Records[R[3]].Kind != RecordKind::Function || Data.Records[R[3]].References[0] != R[2])
            {
              attribute(Attributes, "signature = ");
              type(R[2]);
            }
            break;
          case RecordKind::GetElementPtr:
            append("getelementptr ");
            type(R[2]);
            append(", ");
            typedValue(R[3]);
            for (std::size_t Index = 4; Index < R.size(); ++Index)
            {
              append(", ");
              typedValue(R[Index]);
            }
            if (Entry.Numbers[0])
            {
              attribute(Attributes, "inbounds");
            }
            break;
          default:
            break;
          }
          endAttributes(Attributes);
          append('\n');
        }

        bool hasResult(std::uint32_t Id) const
        {
          const auto &Entry = Data.Records[Id];
          return Data.Records[Entry.References[0]].Kind != RecordKind::VoidType || (Entry.Kind == RecordKind::Call && Referenced[Id]);
        }

        const Model &Data;
        std::size_t MaxBytes;
        std::string Output;
        std::vector<Symbol> Symbols;
        std::vector<std::vector<std::uint32_t>> Parameters;
        std::vector<std::vector<std::uint32_t>> Blocks;
        std::vector<std::vector<std::uint32_t>> Instructions;
        std::vector<std::vector<std::uint32_t>> Fields;
        std::vector<bool> Referenced;
        bool Failed = false;
    };
  } // namespace

  IRSerializationResult<std::string> writeAssembly(const Model &Data, std::size_t MaxBytes)
  {
    return AssemblyWriter(Data, MaxBytes).write();
  }
} // namespace ink::ir::detail
