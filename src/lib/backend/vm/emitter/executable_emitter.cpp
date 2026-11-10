#include "ink/backend/vm/emitter/executable_emitter.h"

#include "ink/backend/vm/bytecode/instruction_codec.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/core/name/name_pool.h"

#include <spdlog/fmt/fmt.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace ink::vm
{
  namespace
  {
    enum class EmissionSymbolKind : std::uint8_t
    {
      Function,
      Data,
      NativeFunction,
      NativeData,
    };

    struct EmissionSymbol
    {
        EmissionSymbolKind Kind;
        std::uint64_t Value;
    };

    // All address arithmetic is mathematical, never an unchecked wrapping host calculation.
    std::optional<std::uint64_t> addOffset(std::uint64_t Base, std::int64_t Offset)
    {
      const auto Magnitude = Offset < 0 ? std::uint64_t{0} - static_cast<std::uint64_t>(Offset) : static_cast<std::uint64_t>(Offset);
      if (Offset < 0)
      {
        return Magnitude <= Base ? std::optional{Base - Magnitude} : std::nullopt;
      }
      return Magnitude <= std::numeric_limits<std::uint64_t>::max() - Base ? std::optional{Base + Magnitude} : std::nullopt;
    }

    std::optional<std::uint64_t> relativeBits(std::uint64_t Target, std::uint64_t PC)
    {
      if (Target >= PC)
      {
        const auto Distance = Target - PC;
        return Distance <= static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()) ? std::optional{Distance} : std::nullopt;
      }
      const auto Distance = PC - Target;
      return Distance <= (std::uint64_t{1} << 63) ? std::optional{std::uint64_t{0} - Distance} : std::nullopt;
    }

    class Emitter
    {
      public:
        Emitter(const MachineModule &Module, const MachineFunction &Entry, const core::DiagnosticEngine &Diagnostics, ByteOrder Order)
            : Module(Module),
              Entry(Entry),
              Diagnostics(Diagnostics),
              Order(Order)
        {
        }

        std::optional<ExecutableFile> emit()
        {
          if (Order != ByteOrder::Little && Order != ByteOrder::Big)
          {
            fail("byte order is invalid");
            return std::nullopt;
          }
          Output.getByteOrder() = Order;
          if (!layoutCode() || !layoutData(true) || !layoutData(false))
          {
            return std::nullopt;
          }
          collectImports();
          Output.getEntryOffset() = FunctionOffsets.find(&Entry)->second;
          Output.getImage().resize(static_cast<std::size_t>(Cursor), 0);
          for (const auto &Global : Module.globalVariables())
          {
            const auto Offset = GlobalOffsets.find(&Global)->second;
            std::copy(Global.initialBytes().begin(), Global.initialBytes().end(), Output.getImage().begin() + static_cast<std::ptrdiff_t>(Offset));
          }
          if (!emitCode())
          {
            return std::nullopt;
          }
          return std::move(Output);
        }

      private:
        bool fail(std::string_view Description) const
        {
          Diagnostics.report<core::DiagnosticKind::VMExecutableEmissionFailed>({}, Context, Description);
          return false;
        }

        void addSymbol(core::Name Name, EmissionSymbolKind Kind, std::uint64_t Value)
        {
          if (Name != core::NAME_None)
          {
            Symbols.emplace(Name, EmissionSymbol{Kind, Value});
          }
        }

        bool advance(std::uint64_t Size)
        {
          const auto Limit = static_cast<std::uint64_t>(Output.getImage().max_size());
          if (Cursor > Limit || Size > Limit - Cursor)
          {
            return fail("image size exceeds the host container's representable limit");
          }
          Cursor += Size;
          return true;
        }

        bool layoutCode()
        {
          for (const auto &Function : Module.functions())
          {
            Context = fmt::format("function '{}'", core::NamePool::instance().text(Function.name()));
            addSymbol(Function.name(), EmissionSymbolKind::Function, Cursor);
            FunctionOffsets.emplace(&Function, Cursor);
            for (const auto &Block : Function.basicBlocks())
            {
              BlockOffsets.emplace(&Block, Cursor);
              for (const auto &Instruction : Block.instructions())
              {
                if (!advance(*InstructionCodec::byteSize(Instruction.getOpcode())))
                {
                  return false;
                }
              }
            }
          }
          Output.getSections().push_back({ExecutableSectionKind::Code, 0, Cursor});
          return true;
        }

        bool layoutData(bool ReadOnly)
        {
          const auto Begin = Cursor;
          for (const auto &Global : Module.globalVariables())
          {
            if (Global.isConstant() != ReadOnly)
            {
              continue;
            }
            Context = fmt::format("global '{}'", core::NamePool::instance().text(Global.name()));
            const auto Alignment = Global.alignment();
            const auto Padding = (Alignment - (Cursor & (Alignment - 1))) & (Alignment - 1);
            if (!advance(Padding))
            {
              return false;
            }
            addSymbol(Global.name(), EmissionSymbolKind::Data, Cursor);
            GlobalOffsets.emplace(&Global, Cursor);
            if (!advance(Global.byteSize()))
            {
              return false;
            }
          }
          if (Cursor != Begin)
          {
            Output.getSections().push_back({ReadOnly ? ExecutableSectionKind::ReadOnlyData : ExecutableSectionKind::WritableData, Begin, Cursor - Begin});
          }
          return true;
        }

        void collectImports()
        {
          Context = "native imports";
          std::unordered_map<std::uint64_t, std::uint32_t> FunctionIndices;
          std::unordered_map<core::Name, std::uint32_t> DataIndices;
          for (const auto &Declaration : Module.nativeFunctions())
          {
            const auto &Import = Declaration.Import;
            const auto Key = (static_cast<std::uint64_t>(Import.Provider) << 32) | Import.Name.index();
            auto Position = FunctionIndices.find(Key);
            if (Position == FunctionIndices.end())
            {
              Position = FunctionIndices.emplace(Key, static_cast<std::uint32_t>(Output.getNativeFunctions().size())).first;
              Output.getNativeFunctions().push_back(Import);
            }
            addSymbol(Declaration.Name, EmissionSymbolKind::NativeFunction, Position->second);
          }
          for (const auto &Declaration : Module.nativeData())
          {
            auto Position = DataIndices.find(Declaration.Import.Name);
            if (Position == DataIndices.end())
            {
              Position = DataIndices.emplace(Declaration.Import.Name, static_cast<std::uint32_t>(Output.getNativeData().size())).first;
              Output.getNativeData().push_back(Declaration.Import);
            }
            addSymbol(Declaration.Name, EmissionSymbolKind::NativeData, Position->second);
          }
        }

        const EmissionSymbol &symbol(core::Name Name) const
        {
          return Symbols.find(Name)->second;
        }

        bool resolveAddress(const AddressOperand &Address, BytecodeOperand &Result, std::uint64_t PC, std::uint64_t FieldOffset)
        {
          Result.Kind = BytecodeOperandKind::Address;
          if (const auto Register = Address.getRegister())
          {
            Result.AddressBase = static_cast<EncodedAddressBase>(*Register);
            Result.Bits = static_cast<std::uint64_t>(Address.getByteOffset());
            return true;
          }
          const auto &Symbol = symbol(*Address.getSymbol());
          if (Symbol.Kind == EmissionSymbolKind::NativeData)
          {
            Result.AddressBase = EncodedAddressBase::Native;
            Output.getNativeDataRelocations().push_back({FieldOffset + 1, static_cast<std::uint32_t>(Symbol.Value), Address.getByteOffset()});
            return true;
          }
          if (Symbol.Kind == EmissionSymbolKind::NativeFunction)
          {
            return fail("native function addresses require a callable adapter; use a direct callNative import");
          }
          const auto AddressValue = addOffset(Symbol.Value, Address.getByteOffset());
          const auto Relative = AddressValue ? relativeBits(*AddressValue, PC) : std::nullopt;
          if (!Relative)
          {
            return fail("symbol address or PC-relative displacement is out of range");
          }
          Result.AddressBase = EncodedAddressBase::PC;
          Result.Bits = *Relative;
          return true;
        }

        bool resolveImmediate(const ImmediateOperand &Immediate, BytecodeOperand &Result, const MachineInstruction &Instruction, std::uint64_t PC, std::uint64_t FieldOffset)
        {
          Result.Kind = BytecodeOperandKind::Immediate;
          const auto Operation = Instruction.getOpcode();
          if (const auto Bits = Immediate.bits())
          {
            Result.Bits = *Bits;
          }
          else
          {
            const auto &Symbol = symbol(*Immediate.getSymbol());
            const auto Offset = *Immediate.getByteOffset();
            if (Operation == Opcode::Calli || Operation == Opcode::CallNativei)
            {
              const auto Value = Operation == Opcode::Calli ? relativeBits(Symbol.Value, PC) : std::optional{Symbol.Value};
              if (!Value)
              {
                return fail("call displacement exceeds i64 range");
              }
              Result.Bits = *Value;
            }
            else if (Symbol.Kind == EmissionSymbolKind::NativeData)
            {
              Output.getNativeDataRelocations().push_back({FieldOffset, static_cast<std::uint32_t>(Symbol.Value), Offset});
            }
            else
            {
              if (Symbol.Kind == EmissionSymbolKind::NativeFunction)
              {
                return fail("native function addresses require a callable adapter; use a direct callNative import");
              }
              const auto Value = addOffset(Symbol.Value, Offset);
              if (!Value || (Instruction.bitWidth() < 64 && *Value >= (std::uint64_t{1} << Instruction.bitWidth())))
              {
                return fail("symbol address cannot be represented by the immediate field");
              }
              Result.Bits = *Value;
            }
          }
          return true;
        }

        bool emitInstruction(const MachineInstruction &Instruction, std::uint64_t PC)
        {
          BytecodeInstruction Encoded;
          Encoded.Operation = Instruction.getOpcode();
          Encoded.OperandCount = static_cast<std::uint8_t>(Instruction.operandCount());
          std::uint64_t FieldOffset = PC + 2;
          for (std::size_t Index = 0; Index < Instruction.operandCount(); ++Index)
          {
            const auto &Operand = *Instruction.getOperand(Index);
            auto &Result = Encoded.Operands[Index];
            switch (Operand.getKind())
            {
            case OperandKind::Register:
              Result.Kind = BytecodeOperandKind::Register;
              Result.Register = static_cast<const RegisterOperand &>(Operand).getRegister();
              FieldOffset += 1;
              break;
            case OperandKind::Address:
              if (!resolveAddress(static_cast<const AddressOperand &>(Operand), Result, PC, FieldOffset))
              {
                return false;
              }
              FieldOffset += 9;
              break;
            case OperandKind::Immediate:
              if (!resolveImmediate(static_cast<const ImmediateOperand &>(Operand), Result, Instruction, PC, FieldOffset))
              {
                return false;
              }
              FieldOffset += Instruction.bitWidth() / 8;
              break;
            case OperandKind::BasicBlock:
            {
              const auto *Block = static_cast<const BasicBlockOperand &>(Operand).getBlock();
              const auto Relative = relativeBits(BlockOffsets.find(Block)->second, PC);
              if (!Relative)
              {
                return fail("branch displacement exceeds i64 range");
              }
              Result.Kind = BytecodeOperandKind::Address;
              Result.AddressBase = EncodedAddressBase::PC;
              Result.Bits = *Relative;
              FieldOffset += 9;
              break;
            }
            }
          }
          const auto Bytes = InstructionCodec::encodeUnchecked(Encoded, Order);
          std::copy(Bytes.begin(), Bytes.end(), Output.getImage().begin() + static_cast<std::ptrdiff_t>(PC));
          return true;
        }

        bool emitCode()
        {
          std::uint64_t PC = 0;
          for (const auto &Function : Module.functions())
          {
            for (const auto &Block : Function.basicBlocks())
            {
              for (const auto &Instruction : Block.instructions())
              {
                Context = fmt::format("function '{}', instruction at image offset {}", core::NamePool::instance().text(Function.name()), PC);
                if (!emitInstruction(Instruction, PC))
                {
                  return false;
                }
                PC += *InstructionCodec::byteSize(Instruction.getOpcode());
              }
            }
          }
          return true;
        }

        const MachineModule &Module;
        const MachineFunction &Entry;
        const core::DiagnosticEngine &Diagnostics;
        ByteOrder Order;
        ExecutableFile Output;
        std::uint64_t Cursor = 0;
        std::string Context = "module";
        std::unordered_map<core::Name, EmissionSymbol> Symbols;
        std::unordered_map<const MachineFunction *, std::uint64_t> FunctionOffsets;
        std::unordered_map<const MachineBasicBlock *, std::uint64_t> BlockOffsets;
        std::unordered_map<const MachineGlobalVariable *, std::uint64_t> GlobalOffsets;
    };
  } // namespace

  std::optional<ExecutableFile> ExecutableEmitter::emit(const MachineModule &Module, const MachineFunction &Entry, const core::DiagnosticEngine &Diagnostics, ByteOrder Order)
  {
    return Emitter(Module, Entry, Diagnostics, Order).emit();
  }
} // namespace ink::vm
