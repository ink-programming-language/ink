#include "bytecode_archive_internal.h"

#include <algorithm>
#include <vector>

namespace ink::execution
{
  namespace
  {
    class ArchiveWriter final : private archive::Budget
    {
      public:
        explicit ArchiveWriter(BytecodeLimits Limits)
            : Budget(Limits)
        {
        }

        BytecodeSerializeResult write(const BytecodeArtifact &Artifact)
        {
          if (!raw(archive::Magic) || !u32(archive::FormatVersion) || !u32(archive::InstructionVersion) || !tag(Artifact.Kind, archive::ArtifactKindTags) || !string(Artifact.Target) || !string(Artifact.ModuleName) || !u32(Artifact.Entry))
          {
            return result();
          }
          if (!count(Artifact.Image.Layouts->size()))
          {
            return result();
          }
          for (std::size_t Index = 0; Index < Artifact.Image.Layouts->size(); ++Index)
          {
            if (!layout(*Artifact.Image.Layouts->get(static_cast<RuntimeTypeId>(Index))))
            {
              return result();
            }
          }
          if (!descriptors(Artifact.Image) || !functions(Artifact.Image) || !symbols(Artifact.Symbols))
          {
            return result();
          }
          return result();
        }

      private:
        BytecodeSerializeResult result()
        {
          if (Status != BytecodeStatus::Success)
          {
            return {Status, std::move(Message), {}};
          }
          return {BytecodeStatus::Success, {}, std::move(Bytes)};
        }

        bool raw(std::string_view Value)
        {
          if (Status != BytecodeStatus::Success)
          {
            return false;
          }
          if (Value.size() > Limits.MaxBytes - Bytes.size() || Value.size() > Bytes.max_size() - Bytes.size())
          {
            return fail(BytecodeStatus::LimitExceeded, "Bytecode file size budget exceeded");
          }
          if (!allocate(Value.size()))
          {
            return false;
          }
          Bytes.append(Value);
          return true;
        }

        bool u8(std::uint8_t Value)
        {
          const char Byte = static_cast<char>(Value);
          return raw(std::string_view(&Byte, 1));
        }

        bool u32(std::uint32_t Value)
        {
          std::array<char, 4> Encoded{};
          for (unsigned Index = 0; Index < Encoded.size(); ++Index)
          {
            Encoded[Index] = static_cast<char>((Value >> (Index * 8)) & 0xffU);
          }
          return raw(std::string_view(Encoded.data(), Encoded.size()));
        }

        bool u64(std::uint64_t Value)
        {
          std::array<char, 8> Encoded{};
          for (unsigned Index = 0; Index < Encoded.size(); ++Index)
          {
            Encoded[Index] = static_cast<char>((Value >> (Index * 8)) & 0xffU);
          }
          return raw(std::string_view(Encoded.data(), Encoded.size()));
        }

        bool length(std::size_t Value)
        {
          if (Value > std::numeric_limits<std::uint32_t>::max())
          {
            return fail(BytecodeStatus::LimitExceeded, "Bytecode field exceeds its 32-bit file length");
          }
          return u32(static_cast<std::uint32_t>(Value));
        }

        bool count(std::size_t Value)
        {
          return records(Value) && length(Value);
        }

        bool string(std::string_view Value)
        {
          return stringBytes(Value.size()) && length(Value.size()) && raw(Value);
        }

        template <typename Enum, std::size_t Size>
        bool tag(Enum Value, const std::array<std::pair<Enum, std::uint32_t>, Size> &Table)
        {
          std::uint32_t Wire = 0;
          return archive::encodeTag(Value, Table, Wire) ? u32(Wire) : fail(BytecodeStatus::InvalidInput, "Unknown bytecode enum value");
        }

        bool layout(const TypeDesc &Layout)
        {
          if (!tag(Layout.Kind, archive::RuntimeKindTags) || !u64(Layout.Size) || !u64(Layout.Alignment) || !u8(Layout.Native) || !string(Layout.Name))
          {
            return false;
          }
          switch (Layout.Kind)
          {
          case RuntimeKind::Integer:
            return u32(Layout.bitWidth()) && u8(Layout.isSigned());
          case RuntimeKind::Float:
            return u32(Layout.bitWidth());
          case RuntimeKind::Pointer:
            return u32(Layout.pointerDesc().Pointee) && u8(Layout.pointerDesc().Writable);
          case RuntimeKind::Function:
            if (!u32(Layout.functionDesc().ReturnType) || !count(Layout.functionDesc().Parameters.size()))
            {
              return false;
            }
            for (RuntimeTypeId Parameter : Layout.functionDesc().Parameters)
            {
              if (!u32(Parameter))
              {
                return false;
              }
            }
            return true;
          case RuntimeKind::Array:
            return u32(Layout.arrayDesc().ElementType) && u64(Layout.arrayDesc().ElementCount);
          case RuntimeKind::Class:
            if (!string(Layout.classDesc().NominalIdentity) || !count(Layout.classDesc().Fields.size()))
            {
              return false;
            }
            for (const FieldDesc &Field : Layout.classDesc().Fields)
            {
              if (!string(Field.Name) || !u32(Field.Type) || !u64(Field.Offset) || !u8(Field.Visibility == core::VisibilityKind::Private) || !u32(Field.Initializer))
              {
                return false;
              }
            }
            if (!count(Layout.classDesc().Methods.size()))
            {
              return false;
            }
            for (const MethodDesc &Method : Layout.classDesc().Methods)
            {
              if (!string(Method.Name) || !u32(Method.Signature) || !u32(Method.Function) || !u8(Method.Visibility == core::VisibilityKind::Private) || !u8(Method.WritableReceiver))
              {
                return false;
              }
            }
            return true;
          default:
            return true;
          }
        }

        bool descriptors(const ExecutionImage &Image)
        {
          if (!count(Image.Descriptors.size()) || !allocate(Image.Descriptors.size(), sizeof(FunctionId)))
          {
            return false;
          }
          std::vector<FunctionId> Ids;
          Ids.reserve(Image.Descriptors.size());
          for (const auto &[Id, Descriptor] : Image.Descriptors)
          {
            Ids.push_back(Id);
          }
          std::sort(Ids.begin(), Ids.end());
          for (FunctionId Id : Ids)
          {
            const auto &Descriptor = Image.Descriptors.find(Id)->second;
            if (!u32(Descriptor.Id) || !u32(Descriptor.Signature) || !string(Descriptor.Symbol) || !u8(Descriptor.External) || !u8(Descriptor.NativeAbi) || !u8(Descriptor.Supported) || !u8(Descriptor.CAbi) || !u8(Descriptor.Exported))
            {
              return false;
            }
          }
          return true;
        }

        bool value(const RuntimeValue &Value, std::size_t Depth = 0)
        {
          if (Depth >= Limits.MaxTypeDepth)
          {
            return fail(BytecodeStatus::LimitExceeded, "Bytecode array constant exceeds the nesting limit");
          }
          std::uint8_t Payload = 0;
          if (Value.Object)
          {
            switch (Value.kind())
            {
            case RuntimeKind::Integer:
              Payload = 1;
              break;
            case RuntimeKind::String:
              Payload = 2;
              break;
            case RuntimeKind::Class:
              Payload = 5;
              break;
            case RuntimeKind::Array:
              Payload = 4;
              break;
            case RuntimeKind::Pointer:
              if (Value.pointer().kind() != ExecutionPointer::Kind::Null)
              {
                return fail(BytecodeStatus::UnsupportedConstant, "Bytecode files cannot contain live storage or native pointers");
              }
              Payload = 3;
              break;
            default:
              return fail(BytecodeStatus::UnsupportedConstant, "Unsupported bytecode constant payload");
            }
          }
          if (!u32(Value.Type) || !u8(Value.Initialized) || !u8(Payload) || !u64(Value.Bits))
          {
            return false;
          }
          if (Payload == 1)
          {
            const ExecutionInteger &Integer = Value.integer();
            const std::size_t WordCount = static_cast<std::size_t>(Integer.bitWidth() / 64) + (Integer.bitWidth() % 64 != 0);
            if (!allocate(WordCount, sizeof(std::uint64_t)) || !u32(Integer.bitWidth()) || !count(WordCount))
            {
              return false;
            }
            const ir::IntegerBits Bits = Integer.bits();
            for (std::uint64_t Word : Bits.words())
            {
              if (!u64(Word))
              {
                return false;
              }
            }
          }
          else if (Payload == 2)
          {
            return string(Value.string());
          }
          else if (Payload == 4 || Payload == 5)
          {
            if (!count(Value.aggregate().size()))
            {
              return false;
            }
            for (const RuntimeValue &Element : Value.aggregate())
            {
              if (!value(Element, Depth + 1))
              {
                return false;
              }
            }
          }
          return true;
        }

        bool call(const ExecutionCallSite &Call)
        {
          if (!u32(Call.Target) || !u32(Call.CalleeSlot) || !u32(Call.Signature) || !count(Call.Arguments.size()))
          {
            return false;
          }
          for (SlotId Argument : Call.Arguments)
          {
            if (!u32(Argument))
            {
              return false;
            }
          }
          return true;
        }

        bool instruction(const BytecodeInstruction &Instruction)
        {
          const auto *Metadata = bytecodeInstructionMetadata(Instruction.Code);
          if (!Metadata || !tag(Instruction.Code, archive::OpcodeTags))
          {
            return fail(BytecodeStatus::InvalidInput, "Unknown bytecode instruction");
          }
          for (std::size_t Index = 0; Index < Instruction.Operands.size(); ++Index)
          {
            std::uint32_t Operand = Instruction.Operands[Index];
            if (Metadata->Operands[Index] == BytecodeOperandKind::Status && !archive::encodeOperand(Operand, archive::StatusTags, Operand))
            {
              return fail(BytecodeStatus::InvalidInput, "Unknown bytecode failure status");
            }
            if (Metadata->Operands[Index] == BytecodeOperandKind::Predicate && !archive::encodeOperand(Operand, archive::PredicateTags, Operand))
            {
              return fail(BytecodeStatus::InvalidInput, "Unknown bytecode comparison predicate");
            }
            if (!u32(Operand))
            {
              return false;
            }
          }
          return true;
        }

        bool function(const ExecutableFunction &Function)
        {
          if (!u32(Function.Id) || !u32(Function.Signature) || !u32(Function.LocalStorageCount) || !count(Function.SlotTypes.size()))
          {
            return false;
          }
          for (RuntimeTypeId Type : Function.SlotTypes)
          {
            if (!u32(Type))
            {
              return false;
            }
          }
          if (!count(Function.InitialSlots.size()))
          {
            return false;
          }
          for (const RuntimeValue &Initial : Function.InitialSlots)
          {
            if (!value(Initial))
            {
              return false;
            }
          }
          if (!count(Function.Calls.size()))
          {
            return false;
          }
          for (const ExecutionCallSite &Call : Function.Calls)
          {
            if (!call(Call))
            {
              return false;
            }
          }
          if (!count(Function.Code.size()))
          {
            return false;
          }
          for (const BytecodeInstruction &Instruction : Function.Code)
          {
            if (!instruction(Instruction))
            {
              return false;
            }
          }
          return string(std::string_view(Function.ConstantData.data(), Function.ConstantData.size()));
        }

        bool functions(const ExecutionImage &Image)
        {
          if (!count(Image.Functions.size()) || !allocate(Image.Functions.size(), sizeof(FunctionId)))
          {
            return false;
          }
          std::vector<FunctionId> Ids;
          Ids.reserve(Image.Functions.size());
          for (const auto &[Id, Function] : Image.Functions)
          {
            Ids.push_back(Id);
          }
          std::sort(Ids.begin(), Ids.end());
          for (FunctionId Id : Ids)
          {
            if (!function(*Image.Functions.find(Id)->second))
            {
              return false;
            }
          }
          return true;
        }

        bool symbol(const BytecodeSymbol &Symbol)
        {
          const auto &Identity = Symbol.Identity;
          if (!u32(Symbol.Function) || !tag(Symbol.Kind, archive::SymbolKindTags) || !tag(Symbol.Visibility, archive::VisibilityTags) || !string(Identity.Module) || !string(Identity.Name) || !string(Identity.Signature) || !string(Identity.LinkName) || !count(Identity.GenericArguments.size()))
          {
            return false;
          }
          for (const auto &Argument : Identity.GenericArguments)
          {
            if (!tag(Argument.Kind, archive::GenericArgumentTags) || !string(Argument.Type) || !string(Argument.Value))
            {
              return false;
            }
          }
          return true;
        }

        bool symbols(const std::vector<BytecodeSymbol> &Symbols)
        {
          if (!count(Symbols.size()) || !allocate(Symbols.size(), sizeof(const BytecodeSymbol *)))
          {
            return false;
          }
          std::vector<const BytecodeSymbol *> Ordered;
          Ordered.reserve(Symbols.size());
          for (const BytecodeSymbol &Symbol : Symbols)
          {
            Ordered.push_back(&Symbol);
          }
          std::sort(Ordered.begin(), Ordered.end(), [](const BytecodeSymbol *Left, const BytecodeSymbol *Right)
          {
            return Left->Function < Right->Function;
          });
          for (const BytecodeSymbol *Symbol : Ordered)
          {
            if (!symbol(*Symbol))
            {
              return false;
            }
          }
          return true;
        }

        std::string Bytes;
    };
  } // namespace

  BytecodeSerializeResult serializeBytecodeArtifact(const BytecodeArtifact &Artifact, BytecodeLimits Limits)
  {
    BytecodeResult Validation = validateBytecodeArtifact(Artifact, Limits);
    if (!Validation)
    {
      return {Validation.Status, std::move(Validation.Message), {}};
    }
    return ArchiveWriter(Limits).write(Artifact);
  }
} // namespace ink::execution
