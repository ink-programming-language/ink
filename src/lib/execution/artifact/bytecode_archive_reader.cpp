#include "bytecode_archive_internal.h"

#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    class ArchiveReader final : private archive::Budget
    {
      public:
        ArchiveReader(std::string_view Bytes, BytecodeLimits Limits)
            : Budget(Limits),
              Bytes(Bytes)
        {
        }

        BytecodeArtifactResult read()
        {
          if (Bytes.size() > Limits.MaxBytes)
          {
            fail(BytecodeStatus::LimitExceeded, "Bytecode file size budget exceeded");
            return failure();
          }
          std::string_view Magic;
          std::uint32_t Version = 0;
          std::uint32_t Instructions = 0;
          if (!raw(archive::Magic.size(), Magic) || Magic != archive::Magic)
          {
            fail(BytecodeStatus::InvalidFormat, "Invalid bytecode file magic");
            return failure();
          }
          if (!u32(Version) || !u32(Instructions))
          {
            return failure();
          }
          if (Version != archive::FormatVersion || Instructions != archive::InstructionVersion)
          {
            fail(BytecodeStatus::UnsupportedVersion, "Unsupported bytecode file or instruction set version");
            return failure();
          }
          if (!allocate(1, sizeof(BytecodeArtifact)) || !allocate(1, sizeof(RuntimeTypeTable) + 64))
          {
            return failure();
          }
          auto Artifact = std::make_unique<BytecodeArtifact>();
          if (!tag(Artifact->Kind, archive::ArtifactKindTags) || !string(Artifact->Target))
          {
            return failure();
          }
          if (Artifact->Target != nativeBytecodeTarget())
          {
            fail(BytecodeStatus::IncompatibleTarget, "Bytecode target does not match this runtime");
            return failure();
          }
          if (!string(Artifact->ModuleName) || !u32(Artifact->Entry))
          {
            return failure();
          }
          Layouts = std::make_shared<RuntimeTypeTable>();
          Artifact->Image.Layouts = Layouts;
          if (!layouts() || !descriptors(Artifact->Image) || !functions(Artifact->Image) || !symbols(Artifact->Symbols))
          {
            return failure();
          }
          if (Position != Bytes.size())
          {
            fail(BytecodeStatus::InvalidFormat, "Trailing bytes after bytecode artifact");
            return failure();
          }
          // Keep validation's transient indexes and canonical type strings inside
          // the same conservative allocation budget as the decoded artifact.
          BytecodeLimits ValidationLimits = Limits;
          ValidationLimits.MaxAllocationBytes -= Allocated;
          BytecodeResult Validation = validateBytecodeArtifact(*Artifact, ValidationLimits);
          if (!Validation)
          {
            return {Validation.Status, std::move(Validation.Message), nullptr};
          }
          return {BytecodeStatus::Success, {}, std::move(Artifact)};
        }

      private:
        BytecodeArtifactResult failure()
        {
          return {Status, std::move(Message), nullptr};
        }

        bool raw(std::size_t Length, std::string_view &Value)
        {
          if (Status != BytecodeStatus::Success)
          {
            return false;
          }
          if (Length > Bytes.size() - Position)
          {
            return fail(BytecodeStatus::InvalidFormat, "Truncated bytecode file");
          }
          Value = Bytes.substr(Position, Length);
          Position += Length;
          return true;
        }

        bool u8(std::uint8_t &Value)
        {
          std::string_view Encoded;
          if (!raw(1, Encoded))
          {
            return false;
          }
          Value = static_cast<std::uint8_t>(static_cast<unsigned char>(Encoded[0]));
          return true;
        }

        bool boolean(bool &Value)
        {
          std::uint8_t Encoded = 0;
          if (!u8(Encoded))
          {
            return false;
          }
          if (Encoded > 1)
          {
            return fail(BytecodeStatus::InvalidFormat, "Invalid bytecode boolean field");
          }
          Value = Encoded != 0;
          return true;
        }

        bool u32(std::uint32_t &Value)
        {
          std::string_view Encoded;
          if (!raw(4, Encoded))
          {
            return false;
          }
          Value = 0;
          for (unsigned Index = 0; Index < 4; ++Index)
          {
            Value |= static_cast<std::uint32_t>(static_cast<unsigned char>(Encoded[Index])) << (Index * 8);
          }
          return true;
        }

        bool u64(std::uint64_t &Value)
        {
          std::string_view Encoded;
          if (!raw(8, Encoded))
          {
            return false;
          }
          Value = 0;
          for (unsigned Index = 0; Index < 8; ++Index)
          {
            Value |= static_cast<std::uint64_t>(static_cast<unsigned char>(Encoded[Index])) << (Index * 8);
          }
          return true;
        }

        bool size(std::size_t &Value)
        {
          std::uint64_t Encoded = 0;
          if (!u64(Encoded))
          {
            return false;
          }
          if (Encoded > std::numeric_limits<std::size_t>::max())
          {
            return fail(BytecodeStatus::InvalidFormat, "Bytecode size does not fit the runtime address space");
          }
          Value = static_cast<std::size_t>(Encoded);
          return true;
        }

        bool count(std::size_t &Count, std::size_t MinimumBytes, std::size_t ElementSize)
        {
          std::uint32_t Encoded = 0;
          if (!u32(Encoded))
          {
            return false;
          }
          Count = Encoded;
          if (MinimumBytes != 0 && Count > (Bytes.size() - Position) / MinimumBytes)
          {
            return fail(BytecodeStatus::InvalidFormat, "Bytecode table length exceeds the remaining file");
          }
          return records(Count) && allocate(Count, ElementSize);
        }

        bool stringView(std::string_view &Value)
        {
          std::uint32_t Length = 0;
          return u32(Length) && stringBytes(Length) && raw(Length, Value);
        }

        bool string(std::string &Value)
        {
          std::string_view Encoded;
          if (!stringView(Encoded) || !allocate(Encoded.size()) || !allocate(1))
          {
            return false;
          }
          Value.assign(Encoded);
          return true;
        }

        template <typename Enum, std::size_t Size>
        bool tag(Enum &Value, const std::array<std::pair<Enum, std::uint32_t>, Size> &Table)
        {
          std::uint32_t Wire = 0;
          if (!u32(Wire))
          {
            return false;
          }
          return archive::decodeTag(Wire, Table, Value) || fail(BytecodeStatus::InvalidFormat, "Unknown bytecode enum tag");
        }

        bool layouts()
        {
          std::size_t Count = 0;
          if (!count(Count, 39, sizeof(StorageLayout) + 32))
          {
            return false;
          }
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            StorageLayout Layout;
            std::size_t Parameters = 0;
            if (!tag(Layout.Kind, archive::RuntimeKindTags) || !u32(Layout.BitWidth) || !boolean(Layout.Signed) || !size(Layout.Size) || !size(Layout.Alignment) || !boolean(Layout.Native) || !u32(Layout.Pointee) || !boolean(Layout.Writable) || !u32(Layout.ReturnType) || !count(Parameters, 4, sizeof(RuntimeTypeId)))
            {
              return false;
            }
            Layout.Parameters.reserve(Parameters);
            for (std::size_t Parameter = 0; Parameter < Parameters; ++Parameter)
            {
              RuntimeTypeId Type = InvalidRuntimeType;
              if (!u32(Type))
              {
                return false;
              }
              Layout.Parameters.push_back(Type);
            }
            if (Layouts->append(std::move(Layout)) == InvalidRuntimeType)
            {
              return fail(BytecodeStatus::LimitExceeded, "Bytecode type table exhausted its ID space");
            }
          }
          return true;
        }

        bool descriptors(ExecutionImage &Image)
        {
          std::size_t Count = 0;
          if (!count(Count, 17, sizeof(RuntimeFunctionDescriptor) + sizeof(FunctionId) + 5 * sizeof(void *)))
          {
            return false;
          }
          Image.Descriptors.reserve(Count);
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            RuntimeFunctionDescriptor Descriptor;
            if (!u32(Descriptor.Id) || !u32(Descriptor.Signature) || !string(Descriptor.Symbol) || !boolean(Descriptor.External) || !boolean(Descriptor.NativeAbi) || !boolean(Descriptor.Supported) || !boolean(Descriptor.CAbi) || !boolean(Descriptor.Exported))
            {
              return false;
            }
            if (!Image.Descriptors.emplace(Descriptor.Id, std::move(Descriptor)).second)
            {
              return fail(BytecodeStatus::InvalidFormat, "Duplicate bytecode function descriptor ID");
            }
          }
          return true;
        }

        bool value(RuntimeValue &Value)
        {
          RuntimeTypeId Type = InvalidRuntimeType;
          bool Initialized = false;
          std::uint8_t Payload = 0;
          std::uint64_t Bits = 0;
          if (!u32(Type) || !boolean(Initialized) || !u8(Payload) || !u64(Bits))
          {
            return false;
          }
          const StorageLayout *Layout = Layouts->get(Type);
          if (!Layout || Payload > 3 || (!Initialized && (Payload != 0 || Bits != 0)) || (Payload != 0 && Bits != 0))
          {
            return fail(BytecodeStatus::InvalidFormat, "Malformed bytecode initial value");
          }
          if (Payload == 0)
          {
            Value = RuntimeValue::fromBits(Bits, Type);
            Value.Initialized = Initialized;
            return true;
          }
          if (!allocate(128))
          {
            return false;
          }
          if (Payload == 1)
          {
            std::uint32_t Width = 0;
            std::size_t Count = 0;
            if (!u32(Width) || !count(Count, 8, sizeof(std::uint64_t)))
            {
              return false;
            }
            const std::size_t Expected = static_cast<std::size_t>(Width / 64) + (Width % 64 != 0);
            if (Layout->Kind != RuntimeKind::Integer || Width != Layout->BitWidth || Width <= 64 || Count != Expected)
            {
              return fail(BytecodeStatus::InvalidFormat, "Malformed bytecode wide integer constant");
            }
            if (!allocate(Count, 2 * sizeof(std::uint64_t)))
            {
              return false;
            }
            std::vector<std::uint64_t> Words;
            Words.reserve(Count);
            for (std::size_t Index = 0; Index < Count; ++Index)
            {
              std::uint64_t Word = 0;
              if (!u64(Word))
              {
                return false;
              }
              Words.push_back(Word);
            }
            const ir::IntegerBits Integer(Width, Words);
            if (!Integer.valid())
            {
              return fail(BytecodeStatus::InvalidFormat, "Bytecode integer contains bits outside its width");
            }
            Value = RuntimeValue::fromInteger(ExecutionInteger(Integer), Type);
            return true;
          }
          if (Payload == 2)
          {
            if (Layout->Kind != RuntimeKind::String)
            {
              return fail(BytecodeStatus::InvalidFormat, "Bytecode string constant has a non-string type");
            }
            std::string_view Encoded;
            if (!stringView(Encoded) || !allocate(Encoded.size()) || !allocate(1))
            {
              return false;
            }
            Value = RuntimeValue::fromString(Encoded, Type);
            return true;
          }
          if (Layout->Kind != RuntimeKind::Pointer)
          {
            return fail(BytecodeStatus::InvalidFormat, "Bytecode null pointer constant has a non-pointer type");
          }
          Value = RuntimeValue::fromPointer(ExecutionPointer{}, Type);
          return true;
        }

        bool call(ExecutionCallSite &Call)
        {
          std::size_t Count = 0;
          if (!u32(Call.Target) || !u32(Call.CalleeSlot) || !u32(Call.Signature) || !count(Count, 4, sizeof(SlotId)))
          {
            return false;
          }
          Call.Arguments.reserve(Count);
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            SlotId Slot = InvalidSlot;
            if (!u32(Slot))
            {
              return false;
            }
            Call.Arguments.push_back(Slot);
          }
          return true;
        }

        bool instruction(BytecodeInstruction &Instruction)
        {
          if (!tag(Instruction.Code, archive::OpcodeTags))
          {
            return false;
          }
          const auto *Metadata = bytecodeInstructionMetadata(Instruction.Code);
          if (!Metadata)
          {
            return fail(BytecodeStatus::InvalidFormat, "Unknown bytecode instruction metadata");
          }
          for (std::size_t Index = 0; Index < Instruction.Operands.size(); ++Index)
          {
            std::uint32_t &Operand = Instruction.Operands[Index];
            if (!u32(Operand))
            {
              return false;
            }
            if (Metadata->Operands[Index] == BytecodeOperandKind::Status && !archive::decodeOperand(Operand, archive::StatusTags, Operand))
            {
              return fail(BytecodeStatus::InvalidFormat, "Unknown bytecode failure status tag");
            }
            if (Metadata->Operands[Index] == BytecodeOperandKind::Predicate && !archive::decodeOperand(Operand, archive::PredicateTags, Operand))
            {
              return fail(BytecodeStatus::InvalidFormat, "Unknown bytecode comparison predicate tag");
            }
          }
          return true;
        }

        bool function(ExecutableFunction &Function)
        {
          std::size_t Count = 0;
          if (!u32(Function.Id) || !u32(Function.Signature) || !u32(Function.LocalStorageCount) || !count(Count, 4, sizeof(RuntimeTypeId)))
          {
            return false;
          }
          Function.Layouts = Layouts;
          Function.SlotTypes.reserve(Count);
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            RuntimeTypeId Type = InvalidRuntimeType;
            if (!u32(Type))
            {
              return false;
            }
            Function.SlotTypes.push_back(Type);
          }
          if (!count(Count, 14, sizeof(RuntimeValue)))
          {
            return false;
          }
          Function.InitialSlots.reserve(Count);
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            RuntimeValue Initial;
            if (!value(Initial))
            {
              return false;
            }
            Function.InitialSlots.push_back(std::move(Initial));
          }
          if (!count(Count, 16, sizeof(ExecutionCallSite)))
          {
            return false;
          }
          Function.Calls.reserve(Count);
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            ExecutionCallSite Call;
            if (!call(Call))
            {
              return false;
            }
            Function.Calls.push_back(std::move(Call));
          }
          if (!count(Count, 20, sizeof(BytecodeInstruction)))
          {
            return false;
          }
          Function.Code.reserve(Count);
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            BytecodeInstruction Instruction;
            if (!instruction(Instruction))
            {
              return false;
            }
            Function.Code.push_back(Instruction);
          }
          std::string_view Data;
          if (!stringView(Data) || !allocate(Data.size()))
          {
            return false;
          }
          Function.ConstantData.assign(Data.begin(), Data.end());
          return true;
        }

        bool functions(ExecutionImage &Image)
        {
          std::size_t Count = 0;
          if (!count(Count, 32, sizeof(ExecutableFunction) + sizeof(FunctionId) + 6 * sizeof(void *)))
          {
            return false;
          }
          Image.Functions.reserve(Count);
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            auto Function = std::make_unique<ExecutableFunction>();
            if (!function(*Function))
            {
              return false;
            }
            const FunctionId Id = Function->Id;
            if (!Image.Functions.emplace(Id, std::move(Function)).second)
            {
              return fail(BytecodeStatus::InvalidFormat, "Duplicate bytecode function body ID");
            }
          }
          return true;
        }

        bool symbols(std::vector<BytecodeSymbol> &Symbols)
        {
          std::size_t Count = 0;
          if (!count(Count, 28, sizeof(BytecodeSymbol)))
          {
            return false;
          }
          Symbols.reserve(Count);
          for (std::size_t Index = 0; Index < Count; ++Index)
          {
            BytecodeSymbol Symbol;
            auto &Identity = Symbol.Identity;
            std::size_t Arguments = 0;
            if (!u32(Symbol.Function) || !tag(Symbol.Kind, archive::SymbolKindTags) || !tag(Symbol.Visibility, archive::VisibilityTags) || !string(Identity.Module) || !string(Identity.Name) || !string(Identity.Signature) || !count(Arguments, 12, sizeof(BytecodeGenericArgument)))
            {
              return false;
            }
            Identity.GenericArguments.reserve(Arguments);
            for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments; ++ArgumentIndex)
            {
              BytecodeGenericArgument Argument;
              if (!tag(Argument.Kind, archive::GenericArgumentTags) || !string(Argument.Type) || !string(Argument.Value))
              {
                return false;
              }
              Identity.GenericArguments.push_back(std::move(Argument));
            }
            Symbols.push_back(std::move(Symbol));
          }
          return true;
        }

        std::string_view Bytes;
        std::size_t Position = 0;
        std::shared_ptr<RuntimeTypeTable> Layouts;
    };
  } // namespace

  BytecodeArtifactResult deserializeBytecodeArtifact(std::string_view Bytes, BytecodeLimits Limits)
  {
    return ArchiveReader(Bytes, Limits).read();
  }
} // namespace ink::execution
