#include "ink/backend/vm/linker/module_linker.h"

#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/core/name/name_pool.h"

#include <spdlog/fmt/fmt.h>

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::vm
{
  namespace
  {
    struct SymbolBinding
    {
        core::Name OutputName;
        MachineSymbolKind Kind;
        // Canonical import identity; Kind selects the function or data table.
        std::size_t ImportIndex;
    };

    struct DefinitionBinding
    {
        core::Name OutputName;
        MachineSymbolKind Kind;
        MachineSymbolLinkage Linkage;
    };

    struct ExternalDefinition
    {
        MachineSymbolKind Kind;
        bool IsConstant;
        std::size_t ModuleIndex;
    };

    struct InputState
    {
        const MachineModule *Module;
        std::unordered_map<core::Name, DefinitionBinding> Definitions;
        std::unordered_map<core::Name, MachineSymbolDeclaration> Declarations;
        std::unordered_map<core::Name, SymbolBinding> NativeSymbols;
    };

    using BlockMap = std::unordered_map<const MachineBasicBlock *, MachineBasicBlock *>;

    bool isValidKind(MachineSymbolKind Kind) noexcept
    {
      return Kind == MachineSymbolKind::Function || Kind == MachineSymbolKind::GlobalVariable;
    }

    class LinkContext
    {
      public:
        explicit LinkContext(const core::DiagnosticEngine &Diagnostics)
            : Diagnostics(Diagnostics)
        {
        }

        std::unique_ptr<MachineModule> link(std::span<const MachineModule *const> Inputs, core::Name OutputName)
        {
          if (!validName(OutputName, true))
          {
            fail("output module", "module name is not present in the name pool");
            return nullptr;
          }
          std::unordered_set<const MachineModule *> Seen;
          for (const MachineModule *Input : Inputs)
          {
            const std::size_t Index = States.size();
            if (!Input || !Seen.insert(Input).second)
            {
              fail(fmt::format("input #{}", Index), Input ? "module was supplied more than once" : "module pointer is null");
              return nullptr;
            }
            States.push_back({Input, {}, {}, {}});
            if (!validName(Input->name(), true))
            {
              fail(Index, "module name is not present in the name pool");
              return nullptr;
            }
          }
          for (std::size_t Index = 0; Index < States.size(); ++Index)
          {
            if (!collectDefinitions(Index))
            {
              return nullptr;
            }
          }
          for (std::size_t Index = 0; Index < States.size(); ++Index)
          {
            if (!collectNativeSymbols(Index) || !collectDeclarations(Index))
            {
              return nullptr;
            }
          }
          for (std::size_t Index = 0; Index < States.size(); ++Index)
          {
            if (!assignLocalNames(Index))
            {
              return nullptr;
            }
          }
          auto Output = std::make_unique<MachineModule>(OutputName);
          for (const MachineNativeFunctionDeclaration &Declaration : NativeFunctions)
          {
            Output->declareNativeFunction(Declaration.Name, Declaration.Import);
          }
          for (const MachineNativeDataDeclaration &Declaration : NativeData)
          {
            Output->declareNativeData(Declaration.Name, Declaration.Import);
          }
          for (std::size_t Index = 0; Index < States.size(); ++Index)
          {
            if (!cloneModule(Index, *Output))
            {
              return nullptr;
            }
          }
          return Output;
        }

      private:
        static bool validName(core::Name Name, bool AllowAnonymous = false) noexcept
        {
          return (AllowAnonymous && Name == core::NAME_None) || core::NamePool::instance().contains(Name);
        }

        static std::string_view spelling(core::Name Name) noexcept
        {
          return core::NamePool::instance().text(Name);
        }

        bool fail(std::string_view Context, std::string_view Description)
        {
          if (!HasError)
          {
            HasError = true;
            Diagnostics.report<core::DiagnosticKind::VMModuleLinkFailed>({}, Context, Description);
          }
          return false;
        }

        bool fail(std::size_t Index, std::string_view Description)
        {
          return fail(fmt::format("input #{} ('{}')", Index, spelling(States[Index].Module->name())), Description);
        }

        bool addDefinition(std::size_t Index, core::Name Name, MachineSymbolKind Kind, MachineSymbolLinkage Linkage, bool IsConstant = false)
        {
          if (Linkage != MachineSymbolLinkage::Local && Linkage != MachineSymbolLinkage::External)
          {
            return fail(Index, "definition has invalid linkage");
          }
          if (!validName(Name, Linkage == MachineSymbolLinkage::Local))
          {
            return fail(Index, "a named definition is required for External linkage, and names must be registered");
          }
          if (Name == core::NAME_None)
          {
            return true;
          }
          if (!States[Index].Definitions.emplace(Name, DefinitionBinding{Name, Kind, Linkage}).second)
          {
            return fail(Index, fmt::format("duplicate definition of '{}' in the same module", spelling(Name)));
          }
          ReservedNames.insert(Name);
          if (Linkage == MachineSymbolLinkage::External)
          {
            const auto [Position, Inserted] = Externals.emplace(Name, ExternalDefinition{Kind, IsConstant, Index});
            if (!Inserted)
            {
              return fail(Index, fmt::format("duplicate External definition of '{}'; first defined in input #{}", spelling(Name), Position->second.ModuleIndex));
            }
          }
          return true;
        }

        bool collectDefinitions(std::size_t Index)
        {
          const MachineModule &Module = *States[Index].Module;
          for (const MachineGlobalVariable &Global : Module.globalVariables())
          {
            if (Global.parent() != &Module || !addDefinition(Index, Global.name(), MachineSymbolKind::GlobalVariable, Global.linkage(), Global.isConstant()))
            {
              return HasError ? false : fail(Index, "global variable has an invalid parent module");
            }
          }
          for (const MachineFunction &Function : Module.functions())
          {
            if (Function.parent() != &Module || !addDefinition(Index, Function.name(), MachineSymbolKind::Function, Function.linkage()))
            {
              return HasError ? false : fail(Index, "function has an invalid parent module");
            }
          }
          return true;
        }

        bool bindNative(std::size_t Index, core::Name Name, SymbolBinding Binding)
        {
          if (States[Index].Definitions.contains(Name) || Externals.contains(Name))
          {
            return fail(Index, fmt::format("native symbol '{}' conflicts with a VM definition", spelling(Name)));
          }
          const auto [Position, Inserted] = NativeAliases.emplace(Name, Binding);
          if (!Inserted && (Position->second.Kind != Binding.Kind || Position->second.ImportIndex != Binding.ImportIndex))
          {
            return fail(Index, fmt::format("native declarations for '{}' have incompatible host identities", spelling(Name)));
          }
          States[Index].NativeSymbols.emplace(Name, Binding);
          ReservedNames.insert(Name);
          return true;
        }

        bool collectNativeSymbols(std::size_t Index)
        {
          for (const MachineNativeFunctionDeclaration &Declaration : States[Index].Module->nativeFunctions())
          {
            const NativeFunctionImport &Import = Declaration.Import;
            if (!validName(Declaration.Name) || !validName(Import.Name))
            {
              return fail(Index, "native function requires nonempty VM and host names registered in the name pool");
            }
            if ((Import.Provider != NativeSymbolProvider::Native && Import.Provider != NativeSymbolProvider::SystemApi) || (Import.CallKind != NativeCallKind::C && Import.CallKind != NativeCallKind::InkNative))
            {
              return fail(Index, fmt::format("native function '{}' has an invalid provider or call kind", spelling(Declaration.Name)));
            }
            auto &ByName = NativeFunctionIndices[Import.Provider];
            const auto [Position, Inserted] = ByName.emplace(Import.Name, NativeFunctions.size());
            if (Inserted)
            {
              NativeFunctions.push_back(Declaration);
            }
            const MachineNativeFunctionDeclaration &Canonical = NativeFunctions[Position->second];
            if (Canonical.Import.CallKind != Import.CallKind)
            {
              return fail(Index, fmt::format("native function '{}' has incompatible call kinds for host symbol '{}'", spelling(Declaration.Name), spelling(Import.Name)));
            }
            if (!bindNative(Index, Declaration.Name, {Canonical.Name, MachineSymbolKind::Function, Position->second}))
            {
              return false;
            }
          }
          for (const MachineNativeDataDeclaration &Declaration : States[Index].Module->nativeData())
          {
            if (!validName(Declaration.Name) || !validName(Declaration.Import.Name))
            {
              return fail(Index, "native data requires nonempty VM and host names registered in the name pool");
            }
            const auto [Position, Inserted] = NativeDataIndices.emplace(Declaration.Import.Name, NativeData.size());
            if (Inserted)
            {
              NativeData.push_back(Declaration);
            }
            if (!bindNative(Index, Declaration.Name, {NativeData[Position->second].Name, MachineSymbolKind::GlobalVariable, Position->second}))
            {
              return false;
            }
          }
          return true;
        }

        bool collectDeclarations(std::size_t Index)
        {
          InputState &State = States[Index];
          for (const MachineSymbolDeclaration &Declaration : State.Module->symbolDeclarations())
          {
            if (!validName(Declaration.Name) || !isValidKind(Declaration.Kind) || (Declaration.Kind == MachineSymbolKind::Function && Declaration.IsConstant))
            {
              return fail(Index, "External declaration has an invalid name, symbol kind, or read-only attribute");
            }
            ReservedNames.insert(Declaration.Name);
            const auto [Position, Inserted] = State.Declarations.emplace(Declaration.Name, Declaration);
            if (!Inserted && (Position->second.Kind != Declaration.Kind || Position->second.IsConstant != Declaration.IsConstant))
            {
              return fail(Index, fmt::format("incompatible declarations of '{}'", spelling(Declaration.Name)));
            }
            const auto Definition = State.Definitions.find(Declaration.Name);
            if ((Definition != State.Definitions.end() && Definition->second.Linkage == MachineSymbolLinkage::Local) || State.NativeSymbols.contains(Declaration.Name))
            {
              return fail(Index, fmt::format("External declaration '{}' conflicts with a Local definition or native declaration", spelling(Declaration.Name)));
            }
            const auto External = Externals.find(Declaration.Name);
            if (External == Externals.end())
            {
              return fail(Index, fmt::format("undefined External symbol '{}'", spelling(Declaration.Name)));
            }
            if (External->second.Kind != Declaration.Kind)
            {
              return fail(Index, fmt::format("External declaration '{}' does not match the definition's symbol kind in input #{}", spelling(Declaration.Name), External->second.ModuleIndex));
            }
            if (External->second.IsConstant != Declaration.IsConstant)
            {
              return fail(Index, fmt::format("External declaration '{}' does not match the definition's read-only attribute in input #{}", spelling(Declaration.Name), External->second.ModuleIndex));
            }
          }
          return true;
        }

        bool assignLocalName(std::size_t Index, core::Name Name)
        {
          if (Name == core::NAME_None)
          {
            return true;
          }
          DefinitionBinding &Binding = States[Index].Definitions.find(Name)->second;
          if (Binding.Linkage != MachineSymbolLinkage::Local)
          {
            return true;
          }
          for (;;)
          {
            const core::Name Candidate = core::NamePool::instance().intern(fmt::format(".__ink_link_local_{}", NextLocalIndex++));
            if (!Candidate.valid())
            {
              return fail(Index, "name pool cannot represent a generated Local symbol name");
            }
            if (ReservedNames.insert(Candidate).second)
            {
              Binding.OutputName = Candidate;
              return true;
            }
          }
        }

        bool assignLocalNames(std::size_t Index)
        {
          const MachineModule &Module = *States[Index].Module;
          for (const MachineGlobalVariable &Global : Module.globalVariables())
          {
            if (!assignLocalName(Index, Global.name()))
            {
              return false;
            }
          }
          for (const MachineFunction &Function : Module.functions())
          {
            if (!assignLocalName(Index, Function.name()))
            {
              return false;
            }
          }
          return true;
        }

        core::Name definitionName(std::size_t Index, core::Name Name) const
        {
          return Name == core::NAME_None ? Name : States[Index].Definitions.find(Name)->second.OutputName;
        }

        std::optional<core::Name> resolve(std::size_t Index, core::Name Name)
        {
          if (!validName(Name))
          {
            fail(Index, "symbol operand requires a nonempty name registered in the name pool");
            return std::nullopt;
          }
          const InputState &State = States[Index];
          if (const auto Position = State.Definitions.find(Name); Position != State.Definitions.end())
          {
            return Position->second.OutputName;
          }
          if (const auto Position = State.NativeSymbols.find(Name); Position != State.NativeSymbols.end())
          {
            return Position->second.OutputName;
          }
          if (Externals.contains(Name))
          {
            return Name;
          }
          fail(Index, fmt::format("undefined symbol '{}'; native symbols require an explicit declaration in the referencing module", spelling(Name)));
          return std::nullopt;
        }

        template <typename OperandType>
        std::optional<OperandType> cloneOperand(std::size_t Index, const MachineInstruction &Instruction, std::size_t OperandIndex, const BlockMap &Blocks)
        {
          // Resolve names in the original module's scope, then rebuild operands with output names and unchanged addends.
          const auto &Value = static_cast<const OperandType &>(*Instruction.getOperand(OperandIndex));
          if constexpr (std::is_same_v<OperandType, BasicBlockOperand>)
          {
            const auto Target = Blocks.find(Value.getBlock());
            if (Target == Blocks.end())
            {
              fail(Index, "branch target does not belong to its source function");
              return std::nullopt;
            }
            return BasicBlockOperand(Target->second);
          }
          else if constexpr (std::is_same_v<OperandType, RegisterOperand>)
          {
            if (!isArithmeticRegister(Value.getRegister()))
            {
              fail(Index, "instruction contains an invalid explicit register");
              return std::nullopt;
            }
            return Value;
          }
          else if constexpr (std::is_same_v<OperandType, AddressOperand>)
          {
            if (const auto Register = Value.getRegister())
            {
              if (!isAddressRegister(*Register))
              {
                fail(Index, "address operand contains an invalid base register");
                return std::nullopt;
              }
              return Value;
            }
            if (const auto Name = resolve(Index, *Value.getSymbol()))
            {
              return AddressOperand(*Name, Value.getByteOffset());
            }
            return std::nullopt;
          }
          else
          {
            static_assert(std::is_same_v<OperandType, ImmediateOperand>);
            if (Value.bits())
            {
              return Value;
            }
            if (const auto Name = resolve(Index, *Value.getSymbol()))
            {
              return ImmediateOperand(*Name, *Value.getByteOffset());
            }
            return std::nullopt;
          }
        }

        template <Opcode Operation, std::size_t... Indices>
        bool cloneTypedInstruction(std::size_t Index, const MachineInstruction &Instruction, MachineInstructionBuilder &Builder, const BlockMap &Blocks, std::index_sequence<Indices...>)
        {
          using OperandTuple = typename OpcodeTraits<Operation>::OperandTuple;
          const std::tuple<std::optional<std::tuple_element_t<Indices, OperandTuple>>...> Values{cloneOperand<std::tuple_element_t<Indices, OperandTuple>>(Index, Instruction, Indices, Blocks)...};
          if (!(std::get<Indices>(Values).has_value() && ...))
          {
            return false;
          }
          if (!Builder.tryCreate<Operation>(*std::get<Indices>(Values)...))
          {
            return fail(Index, "instruction cannot be cloned into its destination block");
          }
          return true;
        }

        bool cloneInstruction(std::size_t Index, const MachineInstruction &Instruction, MachineInstructionBuilder &Builder, const BlockMap &Blocks)
        {
          switch (Instruction.getOpcode())
          {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) \
  case Opcode::Kind:                                                                                                    \
    return cloneTypedInstruction<Opcode::Kind>(Index, Instruction, Builder, Blocks, std::make_index_sequence<OpcodeTraits<Opcode::Kind>::OperandCount>{});
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
          }
          return fail(Index, "instruction has an unknown opcode");
        }

        bool cloneFunction(std::size_t Index, const MachineFunction &Source, MachineModule &Output)
        {
          MachineFunction &Destination = Output.createFunction(definitionName(Index, Source.name()), Source.linkage());
          for (const MachineStackSlot &Slot : Source.frameInfo().stackSlots())
          {
            if (!Destination.frameInfo().createStackSlot(Slot.byteSize(), Slot.alignment()))
            {
              return fail(Index, "stack slot could not be cloned");
            }
          }
          if (const auto BaseOffset = Source.frameInfo().baseOffset())
          {
            if (!Destination.frameInfo().layout(*BaseOffset))
            {
              return fail(Index, "stack frame layout could not be cloned");
            }
          }
          BlockMap Blocks;
          for (const MachineBasicBlock &Block : Source.basicBlocks())
          {
            if (Block.parent() != &Source || !validName(Block.name(), true))
            {
              return fail(Index, "basic block has an invalid parent or name");
            }
            Blocks.emplace(&Block, &Destination.createBasicBlock(Block.name()));
          }
          MachineInstructionBuilder Builder;
          for (const MachineBasicBlock &Block : Source.basicBlocks())
          {
            Builder.setInsertPoint(*Blocks.find(&Block)->second);
            for (const MachineInstruction &Instruction : Block.instructions())
            {
              if (Instruction.parent() != &Block)
              {
                return fail(Index, "instruction has an invalid parent basic block");
              }
              if (!cloneInstruction(Index, Instruction, Builder, Blocks))
              {
                return false;
              }
            }
          }
          return true;
        }

        bool cloneModule(std::size_t Index, MachineModule &Output)
        {
          const MachineModule &Source = *States[Index].Module;
          for (const MachineGlobalVariable &Global : Source.globalVariables())
          {
            if (!Output.createGlobalVariable(Global.byteSize(), Global.alignment(), Global.initialBytes(), definitionName(Index, Global.name()), Global.linkage(), Global.isConstant()))
            {
              return fail(Index, "global variable data could not be cloned");
            }
          }
          for (const MachineFunction &Function : Source.functions())
          {
            if (!cloneFunction(Index, Function, Output))
            {
              return false;
            }
          }
          return true;
        }

        const core::DiagnosticEngine &Diagnostics;
        bool HasError = false;
        std::vector<InputState> States;
        std::unordered_map<core::Name, ExternalDefinition> Externals;
        std::unordered_map<core::Name, SymbolBinding> NativeAliases;
        std::unordered_map<NativeSymbolProvider, std::unordered_map<core::Name, std::size_t>> NativeFunctionIndices;
        std::unordered_map<core::Name, std::size_t> NativeDataIndices;
        std::vector<MachineNativeFunctionDeclaration> NativeFunctions;
        std::vector<MachineNativeDataDeclaration> NativeData;
        std::unordered_set<core::Name> ReservedNames;
        std::size_t NextLocalIndex = 0;
    };
  } // namespace

  std::unique_ptr<MachineModule> ModuleLinker::link(std::span<const MachineModule *const> Inputs, const core::DiagnosticEngine &Diagnostics, core::Name OutputName)
  {
    return LinkContext(Diagnostics).link(Inputs, OutputName);
  }
} // namespace ink::vm
