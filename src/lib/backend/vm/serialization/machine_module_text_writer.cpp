#include "ink/backend/vm/serialization/machine_module_text.h"

#include "ink/backend/vm/instruction/address_operand.h"
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
          LabelScope ModuleLabels;
          for (const MachineConstant &Constant : Module.constants())
          {
            ModuleLabels.reserve(Constant.name());
          }
          for (const MachineGlobalVariable &Global : Module.globalVariables())
          {
            ModuleLabels.reserve(Global.name());
          }
          for (const MachineFunction &Function : Module.functions())
          {
            ModuleLabels.reserve(Function.name());
          }
          Text += "\n\n.section .rodata\n";
          std::size_t ConstantIndex = 0;
          for (const MachineConstant &Constant : Module.constants())
          {
            Text += ".constant ";
            if (!writeLabel(ModuleLabels, Constant.name(), ".C", ConstantIndex))
            {
              return failure();
            }
            Text += ", ";
            writeUnsigned(Constant.alignment());
            Text += '\n';
            writeBytes(Constant.bytes());
            Text += ".endconstant\n";
          }
          Text += "\n.section .data\n";
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
            for (const MachineBasicBlock &Block : Function.basicBlocks())
            {
              if (!writeLabel(BlockLabels, Block.name(), ".L", BlockIndex, true))
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
        bool writeLabel(LabelScope &Scope, core::Name Name, std::string_view Prefix, std::size_t &NextIndex, bool IsBlock = false)
        {
          const std::string Label = Scope.allocate(Name, Prefix, NextIndex);
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
          if (Name == "InvalidRegister")
          {
            Error = "instruction contains an unknown register ID";
            return false;
          }
          Text += Name;
          return true;
        }

        bool writeOperand(const Operand &Value)
        {
          switch (Value.getKind())
          {
          case OperandKind::Register:
            return writeRegister(static_cast<const RegisterOperand &>(Value).getRegister());
          case OperandKind::Immediate:
            Text += "0x";
            writeUnsigned(static_cast<const ImmediateOperand &>(Value).bits(), 16);
            return true;
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
              Text += "ref";
              writeUnsigned(*Address.getBaseReferenceIndex());
            }
            const std::int64_t Offset = Address.getByteOffset();
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
    };
  } // namespace

  std::expected<std::string, core::Diagnostic> writeMachineModuleText(const MachineModule &Module)
  {
    return TextWriter{}.write(Module);
  }
} // namespace ink::vm
