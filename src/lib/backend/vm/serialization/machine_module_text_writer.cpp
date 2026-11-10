#include "ink/backend/vm/serialization/machine_module_text.h"

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/basic_block_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/name/name_pool.h"

#include <charconv>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ink::vm
{
  namespace
  {
    bool isLabelIdentifier(std::string_view Text) noexcept
    {
      const auto IsStart = [](char Character)
      {
        return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || Character == '_' || Character == '.' || Character == '$';
      };
      if (Text.empty() || !IsStart(Text.front()))
      {
        return false;
      }
      for (const char Character : Text)
      {
        if (!IsStart(Character) && (Character < '0' || Character > '9'))
        {
          return false;
        }
      }
      return true;
    }

    // Reserve real identifiers before generating aliases so an earlier anonymous node cannot take a later name.
    class LabelScope
    {
      public:
        void reserve(core::Name Name)
        {
          const std::string_view Preferred = core::NamePool::instance().text(Name);
          if (isLabelIdentifier(Preferred))
          {
            Reserved.emplace(Preferred);
          }
        }

        std::string allocate(core::Name Name, std::string_view Prefix, std::size_t &NextIndex)
        {
          const std::string_view Preferred = core::NamePool::instance().text(Name);
          if (isLabelIdentifier(Preferred))
          {
            std::string Label(Preferred);
            if (Used.insert(Label).second)
            {
              return Label;
            }
            const auto Position = NextSuffix.try_emplace(Label, 1).first;
            for (;;)
            {
              std::string Candidate = Label + "." + std::to_string(Position->second++);
              if (!Reserved.contains(Candidate) && Used.insert(Candidate).second)
              {
                return Candidate;
              }
            }
          }
          for (;;)
          {
            std::string Candidate = std::string(Prefix) + std::to_string(NextIndex++);
            if (!Reserved.contains(Candidate) && Used.insert(Candidate).second)
            {
              return Candidate;
            }
          }
        }

      private:
        std::unordered_set<std::string> Reserved;
        std::unordered_set<std::string> Used;
        std::unordered_map<std::string, std::size_t> NextSuffix;
    };

    class TextWriter
    {
      public:
        std::expected<std::string, core::Diagnostic> write(const MachineModule &Module)
        {
          Text = ".module ";
          if (!writeName(Module.name()))
          {
            return failure();
          }
          if (!writeSymbolDeclarations(Module))
          {
            return failure();
          }
          LabelScope ModuleLabels;
          for (const MachineGlobalVariable &Global : Module.globalVariables())
          {
            ModuleLabels.reserve(Global.name());
          }
          for (const MachineFunction &Function : Module.functions())
          {
            ModuleLabels.reserve(Function.name());
          }
          Text += "\n\n.section .data\n";
          std::size_t GlobalIndex = 0;
          for (const MachineGlobalVariable &Global : Module.globalVariables())
          {
            Text += ".global ";
            if (!writeLabel(ModuleLabels, Global.name(), ".G", GlobalIndex))
            {
              return failure();
            }
            Text += ", ";
            writeUnsigned(Global.byteSize());
            Text += ", ";
            writeUnsigned(Global.alignment());
            if (Global.isConstant())
            {
              Text += " readonly";
            }
            if (!writeLinkage(Global.linkage(), Global.name()))
            {
              return failure();
            }
            Text += '\n';
            writeBytes(Global.initialBytes());
            Text += ".endglobal\n";
          }
          Text += "\n.section .text\n";
          std::size_t FunctionIndex = 0;
          for (const MachineFunction &Function : Module.functions())
          {
            Text += ".function ";
            if (!writeLabel(ModuleLabels, Function.name(), ".F", FunctionIndex))
            {
              return failure();
            }
            if (!writeLinkage(Function.linkage(), Function.name()))
            {
              return failure();
            }
            Text += '\n';
            for (const MachineStackSlot &Slot : Function.frameInfo().stackSlots())
            {
              Text += "    .stackslot ";
              writeUnsigned(Slot.byteSize());
              Text += ", ";
              writeUnsigned(Slot.alignment());
              Text += '\n';
            }
            if (const auto BaseOffset = Function.frameInfo().baseOffset())
            {
              Text += "    .frame ";
              writeUnsigned(*BaseOffset);
              Text += '\n';
            }
            LabelScope BlockLabels;
            for (const MachineBasicBlock &Block : Function.basicBlocks())
            {
              BlockLabels.reserve(Block.name());
            }
            std::size_t BlockIndex = 0;
            FunctionBlockLabels.clear();
            for (const MachineBasicBlock &Block : Function.basicBlocks())
            {
              FunctionBlockLabels.emplace(&Block, BlockLabels.allocate(Block.name(), ".L", BlockIndex));
            }
            for (const MachineBasicBlock &Block : Function.basicBlocks())
            {
              if (!writeAllocatedLabel(FunctionBlockLabels.find(&Block)->second, Block.name(), true))
              {
                return failure();
              }
              Text += '\n';
              for (const MachineInstruction &Instruction : Block.instructions())
              {
                Text += "    ";
                Text += Instruction.getName();
                for (std::size_t Index = 0; Index != Instruction.operandCount(); ++Index)
                {
                  Text += Index == 0 ? " " : ", ";
                  if (!writeOperand(*Instruction.getOperand(Index)))
                  {
                    return failure();
                  }
                }
                Text += '\n';
              }
            }
            Text += ".endfunction\n";
          }
          return std::move(Text);
        }

      private:
        bool writeLinkage(MachineSymbolLinkage Linkage, core::Name Name)
        {
          if (Linkage == MachineSymbolLinkage::Local)
          {
            return true;
          }
          if (Linkage != MachineSymbolLinkage::External || !Name.valid() || core::NamePool::instance().text(Name).empty())
          {
            Error = "definition linkage must be local or external, and external definitions require a nonempty name";
            return false;
          }
          Text += " external";
          return true;
        }

        bool writeSymbolDeclarations(const MachineModule &Module)
        {
          for (const MachineSymbolDeclaration &Declaration : Module.symbolDeclarations())
          {
            Text += "\n.declare ";
            switch (Declaration.Kind)
            {
            case MachineSymbolKind::Function:
              Text += "function ";
              break;
            case MachineSymbolKind::GlobalVariable:
              Text += "global ";
              break;
            default:
              Error = "unknown symbol declaration kind";
              return false;
            }
            if (!writeSymbol(Declaration.Name))
            {
              return false;
            }
            if (Declaration.IsConstant)
            {
              if (Declaration.Kind != MachineSymbolKind::GlobalVariable)
              {
                Error = "only global declarations accept the readonly attribute";
                return false;
              }
              Text += " readonly";
            }
          }
          for (const MachineNativeFunctionDeclaration &Declaration : Module.nativeFunctions())
          {
            Text += "\n.native_function ";
            if (!writeSymbol(Declaration.Name))
            {
              return false;
            }
            switch (Declaration.Import.Provider)
            {
            case NativeSymbolProvider::Native:
              Text += ", native, ";
              break;
            case NativeSymbolProvider::SystemApi:
              Text += ", system_api, ";
              break;
            default:
              Error = "unknown native function provider";
              return false;
            }
            switch (Declaration.Import.CallKind)
            {
            case NativeCallKind::C:
              Text += "c, ";
              break;
            case NativeCallKind::InkNative:
              Text += "ink_native, ";
              break;
            default:
              Error = "unknown native call kind";
              return false;
            }
            if (!writeSymbol(Declaration.Import.Name))
            {
              return false;
            }
          }
          for (const MachineNativeDataDeclaration &Declaration : Module.nativeData())
          {
            Text += "\n.native_data ";
            if (!writeSymbol(Declaration.Name))
            {
              return false;
            }
            Text += ", ";
            if (!writeSymbol(Declaration.Import.Name))
            {
              return false;
            }
          }
          return true;
        }

        bool writeLabel(LabelScope &Scope, core::Name Name, std::string_view Prefix, std::size_t &NextIndex, bool IsBlock = false)
        {
          const std::string Label = Scope.allocate(Name, Prefix, NextIndex);
          return writeAllocatedLabel(Label, Name, IsBlock);
        }

        bool writeAllocatedLabel(std::string_view Label, core::Name Name, bool IsBlock)
        {
          Text += Label;
          if (IsBlock)
          {
            Text += ':';
          }
          if (Label != core::NamePool::instance().text(Name))
          {
            Text += IsBlock ? " " : ", ";
            return writeName(Name);
          }
          return true;
        }

        void writeUnsigned(std::uint64_t Value, int Base = 10)
        {
          char Buffer[32];
          const auto Result = std::to_chars(Buffer, Buffer + sizeof(Buffer), Value, Base);
          Text.append(Buffer, Result.ptr);
        }

        bool writeName(core::Name Name)
        {
          const core::NamePool &Pool = core::NamePool::instance();
          if (Name.valid() && !Pool.contains(Name))
          {
            Error = "name is not present in the process name pool";
            return false;
          }
          constexpr char HexDigits[] = "0123456789ABCDEF";
          Text += '"';
          for (const unsigned char Byte : Pool.text(Name))
          {
            if (Byte == '"' || Byte == '\\')
            {
              Text += '\\';
              Text += static_cast<char>(Byte);
            }
            else if (Byte >= 0x20 && Byte <= 0x7E)
            {
              Text += static_cast<char>(Byte);
            }
            else
            {
              Text += '\\';
              Text += HexDigits[Byte >> 4];
              Text += HexDigits[Byte & 0x0F];
            }
          }
          Text += '"';
          return true;
        }

        void writeBytes(std::span<const std::uint8_t> Bytes)
        {
          constexpr char HexDigits[] = "0123456789abcdef";
          for (std::size_t Index = 0; Index < Bytes.size(); ++Index)
          {
            Text += Index % 16 == 0 ? "    .byte " : ", ";
            Text += "0x";
            Text += HexDigits[Bytes[Index] >> 4];
            Text += HexDigits[Bytes[Index] & 0x0F];
            if (Index % 16 == 15 || Index + 1 == Bytes.size())
            {
              Text += '\n';
            }
          }
        }

        bool writeRegister(RegisterId Register)
        {
          const std::string_view Name = getRegisterName(Register);
          if (!isArithmeticRegister(Register))
          {
            Error = Name == "InvalidRegister" ? "instruction contains an unknown register ID" : "EFLAGS is only accessed implicitly by instructions";
            return false;
          }
          Text += Name;
          return true;
        }

        bool writeSymbol(core::Name Symbol)
        {
          const core::NamePool &Pool = core::NamePool::instance();
          if (!Symbol.valid() || !Pool.contains(Symbol) || Pool.text(Symbol).empty())
          {
            Error = "symbol name must be nonempty and present in the process name pool";
            return false;
          }
          Text += '@';
          const std::string_view Name = Pool.text(Symbol);
          if (isLabelIdentifier(Name))
          {
            Text += Name;
            return true;
          }
          return writeName(Symbol);
        }

        void writeDisplacement(std::int64_t Offset)
        {
          if (Offset > 0)
          {
            Text += " + ";
            writeUnsigned(static_cast<std::uint64_t>(Offset));
          }
          else if (Offset < 0)
          {
            Text += " - ";
            writeUnsigned(std::uint64_t{0} - static_cast<std::uint64_t>(Offset));
          }
        }

        bool writeOperand(const Operand &Value)
        {
          switch (Value.getKind())
          {
          case OperandKind::Register:
            return writeRegister(static_cast<const RegisterOperand &>(Value).getRegister());
          case OperandKind::Immediate:
          {
            const ImmediateOperand &Immediate = static_cast<const ImmediateOperand &>(Value);
            if (const auto Bits = Immediate.bits())
            {
              Text += "0x";
              writeUnsigned(*Bits, 16);
            }
            else
            {
              if (!writeSymbol(*Immediate.getSymbol()))
              {
                return false;
              }
              writeDisplacement(*Immediate.getByteOffset());
            }
            return true;
          }
          case OperandKind::BasicBlock:
          {
            const MachineBasicBlock *Target = static_cast<const BasicBlockOperand &>(Value).getBlock();
            const auto Label = FunctionBlockLabels.find(Target);
            if (Label == FunctionBlockLabels.end())
            {
              Error = "a branch target must belong to its function";
              return false;
            }
            Text += Label->second;
            return true;
          }
          case OperandKind::Address:
          {
            const AddressOperand &Address = static_cast<const AddressOperand &>(Value);
            Text += '[';
            if (const auto Register = Address.getRegister())
            {
              if (!writeRegister(*Register))
              {
                return false;
              }
            }
            else
            {
              if (!writeSymbol(*Address.getSymbol()))
              {
                return false;
              }
            }
            writeDisplacement(Address.getByteOffset());
            Text += ']';
            return true;
          }
          }
          Error = "instruction contains an unknown operand kind";
          return false;
        }

        std::unexpected<core::Diagnostic> failure() const
        {
          return std::unexpected(core::makeDiagnostic<core::DiagnosticKind::VMTextInvalid>(core::SourceRange::fromByteOffsets(0, 0), Error));
        }

        std::string Text;
        std::string Error;
        std::unordered_map<const MachineBasicBlock *, std::string> FunctionBlockLabels;
    };
  } // namespace

  std::expected<std::string, core::Diagnostic> writeMachineModuleText(const MachineModule &Module)
  {
    return TextWriter{}.write(Module);
  }
} // namespace ink::vm
