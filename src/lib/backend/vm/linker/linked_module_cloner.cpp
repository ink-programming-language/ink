#include "linked_module_cloner.h"

#include "link_bindings.h"
#include "link_diagnostic.h"
#include "ink/backend/vm/module/machine_instruction_clone.h"

#include <cstddef>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace ink::vm::detail
{
  namespace
  {
    using BlockMap = std::unordered_map<const MachineBasicBlock *, MachineBasicBlock *>;

    struct OperandRemapper
    {
        const LinkBindings &Bindings;
        std::size_t ModuleIndex;
        const BlockMap &Blocks;

        bool operator()(RegisterOperand &) const
        {
          return true;
        }

        bool operator()(AddressOperand &Value) const
        {
          if (const auto Name = Value.getSymbol())
          {
            Value = AddressOperand(Bindings.outputName(ModuleIndex, *Name), Value.getByteOffset());
          }
          return true;
        }

        bool operator()(ImmediateOperand &Value) const
        {
          if (const auto Name = Value.getSymbol())
          {
            Value = ImmediateOperand(Bindings.outputName(ModuleIndex, *Name), *Value.getByteOffset());
          }
          return true;
        }

        bool operator()(BasicBlockOperand &Value) const
        {
          const auto Target = Blocks.find(Value.getBlock());
          if (Target == Blocks.end())
          {
            return false;
          }
          Value = BasicBlockOperand(Target->second);
          return true;
        }
    };

    class LinkedModuleCloner
    {
      public:
        LinkedModuleCloner(std::span<const MachineModule *const> Inputs, const LinkBindings &Bindings, const core::DiagnosticEngine &Diagnostics)
            : Inputs(Inputs),
              Bindings(Bindings),
              Diagnostics(Diagnostics)
        {
        }

        std::unique_ptr<MachineModule> clone(core::Name OutputName) const
        {
          auto Output = std::make_unique<MachineModule>(OutputName);
          for (const auto &Symbol : Bindings.symbols())
          {
            if (const auto *FunctionImport = std::get_if<NativeFunctionImport>(&Symbol.Target))
            {
              Output->declareNativeFunction(Symbol.OutputName, *FunctionImport);
            }
            else if (const auto *DataImport = std::get_if<NativeDataImport>(&Symbol.Target))
            {
              Output->declareNativeData(Symbol.OutputName, *DataImport);
            }
          }
          for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
          {
            const MachineModule &Source = *Inputs[Index];
            for (const auto &Global : Source.globalVariables())
            {
              if (!Output->createGlobalVariable(Global.byteSize(), Global.alignment(), Global.initialBytes(), Bindings.outputName(Index, Global.name()), Global.linkage(), Global.isConstant()))
              {
                fail(Index, "global variable data could not be cloned");
                return nullptr;
              }
            }
            for (const auto &Function : Source.functions())
            {
              if (!cloneFunction(Index, Function, *Output))
              {
                return nullptr;
              }
            }
          }
          return Output;
        }

      private:
        bool fail(std::size_t Index, std::string_view Description) const
        {
          return reportLinkFailure(Diagnostics, Index, Inputs[Index], Description);
        }

        bool cloneFrame(std::size_t Index, const MachineFrameInfo &Source, MachineFrameInfo &Destination) const
        {
          for (const auto &Slot : Source.stackSlots())
          {
            if (!Destination.createStackSlot(Slot.byteSize(), Slot.alignment()))
            {
              return fail(Index, "stack slot could not be cloned");
            }
          }
          if (const auto BaseOffset = Source.baseOffset())
          {
            if (!Destination.layout(*BaseOffset))
            {
              return fail(Index, "stack frame layout could not be cloned");
            }
          }
          return true;
        }

        bool cloneInstructions(std::size_t Index, const MachineFunction &Source, const BlockMap &Blocks) const
        {
          const OperandRemapper Remapper{Bindings, Index, Blocks};
          MachineInstructionBuilder Builder;
          for (const auto &Block : Source.basicBlocks())
          {
            Builder.setInsertPoint(*Blocks.find(&Block)->second);
            for (const auto &Instruction : Block.instructions())
            {
              if (!tryCloneInstruction(Instruction, Builder, Remapper))
              {
                return fail(Index, "instruction cannot be cloned into its destination block");
              }
            }
          }
          return true;
        }

        bool cloneFunction(std::size_t Index, const MachineFunction &Source, MachineModule &Output) const
        {
          auto &Destination = Output.createFunction(Bindings.outputName(Index, Source.name()), Source.linkage());
          if (!cloneFrame(Index, Source.frameInfo(), Destination.frameInfo()))
          {
            return false;
          }
          // Establish every destination block before rebuilding forward, backward and self branches.
          BlockMap Blocks;
          for (const auto &Block : Source.basicBlocks())
          {
            Blocks.emplace(&Block, &Destination.createBasicBlock(Block.name()));
          }
          return cloneInstructions(Index, Source, Blocks);
        }

        std::span<const MachineModule *const> Inputs;
        const LinkBindings &Bindings;
        const core::DiagnosticEngine &Diagnostics;
    };
  } // namespace

  std::unique_ptr<MachineModule> cloneLinkedModule(std::span<const MachineModule *const> Inputs, const LinkBindings &Bindings, const core::DiagnosticEngine &Diagnostics, core::Name OutputName)
  {
    return LinkedModuleCloner(Inputs, Bindings, Diagnostics).clone(OutputName);
  }
} // namespace ink::vm::detail
