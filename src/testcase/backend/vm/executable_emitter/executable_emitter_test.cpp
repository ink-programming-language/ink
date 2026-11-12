#include "ink/backend/vm/executable/executable_file_validator.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/executable_emitter/function_emission_context.h"
#include "ink/backend/vm/executable_emitter/instruction_emitter.h"
#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
  namespace
  {
    std::uint64_t readLittleEndian(const ExecutableFile &Executable, std::size_t Offset, std::size_t Count = 8)
    {
      std::uint64_t Bits = 0;
      for (std::size_t Index = 0; Index < Count; ++Index)
      {
        Bits |= static_cast<std::uint64_t>(Executable.getImage()[Offset + Index]) << (Index * 8);
      }
      return Bits;
    }
  } // namespace

  static_assert(std::is_same_v<decltype(std::declval<ExecutableEmitter &>().getExecutable()), const ExecutableFile &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableEmitter &>().getExecutable()), const ExecutableFile &>);

  // Only declared names without VM definitions become Native symbols, including unused functions and read-only data.
  TEST(VMExecutableEmitterTest, DerivesNativeSymbolsBySubtractingFunctionsAndGlobalsFromDeclarations)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto FunctionName = Names.intern("emitter-declared-vm-function");
    const auto GlobalName = Names.intern("emitter-declared-vm-global");
    const auto NativeFunction = Names.intern("emitter-declared-native-function");
    const auto NativeData = Names.intern("emitter-declared-native-data");
    const auto NativeConstant = Names.intern("emitter-declared-native-constant");
    const auto UndeclaredName = Names.intern("emitter-undeclared-symbol");
    MachineModule Module;
    Module.declareSymbol(FunctionName, MachineSymbolKind::Function);
    Module.declareSymbol(GlobalName, MachineSymbolKind::GlobalVariable);
    Module.declareSymbol(NativeFunction, MachineSymbolKind::Function);
    Module.declareSymbol(NativeFunction, MachineSymbolKind::Function);
    Module.declareSymbol(NativeData, MachineSymbolKind::GlobalVariable);
    Module.declareSymbol(NativeConstant, MachineSymbolKind::GlobalVariable, true);
    Module.createFunction(FunctionName);
    ASSERT_TRUE(Module.createGlobalVariable(8, 8, {}, GlobalName).has_value());
    ExecutableEmitter Emitter(Diagnostics);
    Emitter.scanUnResolvedSymbol(Module);

    EXPECT_FALSE(Emitter.getContext().isNativeSymbol(FunctionName));
    EXPECT_FALSE(Emitter.getContext().isNativeSymbol(GlobalName));
    EXPECT_FALSE(Emitter.getContext().isNativeSymbol(UndeclaredName));
    EXPECT_TRUE(Emitter.getContext().isNativeSymbol(NativeFunction));
    EXPECT_TRUE(Emitter.getContext().isNativeSymbol(NativeData));
    EXPECT_TRUE(Emitter.getContext().isNativeSymbol(NativeConstant));
    EXPECT_TRUE(Emitter.getContext().hasUnResolvedSymbol(FunctionName));
    EXPECT_TRUE(Emitter.getContext().hasUnResolvedSymbol(GlobalName));
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedSymbol(NativeFunction));
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedSymbol(NativeData));
    EXPECT_TRUE(Emitter.getExecutable().getDynamicImportSymbols().empty());
  }

  // Function entries use their actual image positions after an existing prefix and aligned global storage, and leave the unresolved map.
  TEST(VMExecutableEmitterTest, ResolvesFunctionEntriesAfterPrefixAndGlobalStorage)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto GlobalName = Names.intern("emitter-function-layout-global");
    const auto FirstName = Names.intern("emitter-function-layout-first");
    const auto SecondName = Names.intern("emitter-function-layout-second");
    const std::array<std::uint8_t, 2> InitialBytes{0xA1, 0xB2};
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(4, 8, InitialBytes, GlobalName).has_value());
    MachineBasicBlock &First = Module.createFunction(FirstName).createBasicBlock();
    MachineBasicBlock &Second = Module.createFunction(SecondName).createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(First);
    Builder.create<Opcode::Ret>();
    Builder.setInsertPoint(Second);
    Builder.create<Opcode::Ret>();
    ExecutableEmitter Emitter(Diagnostics);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.getContext().padZero(3);
    Emitter.padGlobals(Module);
    ASSERT_TRUE(Emitter.getContext().hasUnResolvedSymbol(FirstName));
    ASSERT_TRUE(Emitter.getContext().hasUnResolvedSymbol(SecondName));
    Emitter.padFunction(Module);

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 16U);
    EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(GlobalName), 8U);
    EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(FirstName), 12U);
    EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(SecondName), 14U);
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedSymbol(FirstName));
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedSymbol(SecondName));
    EXPECT_EQ(Executable.getImage()[8], 0xA1U);
    EXPECT_EQ(Executable.getImage()[9], 0xB2U);
    EXPECT_EQ(readLittleEndian(Executable, 10, 2), 0U);
    EXPECT_EQ(readLittleEndian(Executable, 12, 2), 0x003AU);
    EXPECT_EQ(readLittleEndian(Executable, 14, 2), 0x003AU);
  }

  // Full module emission binds ordinary forward, self, and backward function-address immediates to their laid-out entries.
  TEST(VMExecutableEmitterTest, EmitsFunctionAddressRelocationsAcrossFunctionBodies)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto GlobalName = Names.intern("emitter-function-address-slots");
    const auto FirstName = Names.intern("emitter-function-address-first");
    const auto SecondName = Names.intern("emitter-function-address-second");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(24, 8, {}, GlobalName).has_value());
    MachineBasicBlock &First = Module.createFunction(FirstName).createBasicBlock();
    MachineBasicBlock &Second = Module.createFunction(SecondName).createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(First);
    Builder.create<Opcode::Store64i>(AddressOperand(GlobalName), ImmediateOperand(SecondName));
    Builder.create<Opcode::Store64i>(AddressOperand(GlobalName, 8), ImmediateOperand(FirstName));
    Builder.create<Opcode::Ret>();
    Builder.setInsertPoint(Second);
    Builder.create<Opcode::Store64i>(AddressOperand(GlobalName, 16), ImmediateOperand(FirstName));
    Builder.create<Opcode::Ret>();
    ExecutableEmitter Emitter(Diagnostics);
    const ExecutableFile Executable = Emitter.emit(Module, FirstName, {});

    ASSERT_EQ(Executable.byteSize(), 85U);
    EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(FirstName), 24U);
    EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(SecondName), 64U);
    EXPECT_EQ(readLittleEndian(Executable, 24, 2), 0x000CU);
    EXPECT_EQ(readLittleEndian(Executable, 27), static_cast<std::uint64_t>(-24));
    EXPECT_EQ(readLittleEndian(Executable, 46), static_cast<std::uint64_t>(-35));
    EXPECT_EQ(readLittleEndian(Executable, 64, 2), 0x000CU);
    EXPECT_EQ(readLittleEndian(Executable, 67), static_cast<std::uint64_t>(-48));
    const auto &Relocations = Executable.getImageBaseAddressRelocations();
    ASSERT_EQ(Relocations.size(), 3U);
    constexpr std::array<std::pair<std::uint64_t, std::uint64_t>, 3> ExpectedRelocations{{
        {35, 64},
        {54, 24},
        {75, 24},
    }};
    for (const auto &[ImageOffset, TargetOffset] : ExpectedRelocations)
    {
      const auto Position = std::ranges::find(Relocations, ImageOffset, &ImageBaseAddressRelocation::ImageOffset);
      ASSERT_NE(Position, Relocations.end());
      EXPECT_EQ(Position->TargetOffset, TargetOffset);
      EXPECT_EQ(Position->Addend, 0);
      EXPECT_EQ(readLittleEndian(Executable, ImageOffset), 0U);
    }
    EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Direct VM calls encode forward, self, and backward targets relative to each call opcode after global storage.
  TEST(VMExecutableEmitterTest, EncodesDirectVMCallsAcrossFunctionBodies)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto FirstName = Names.intern("emitter-direct-call-first");
    const auto SecondName = Names.intern("emitter-direct-call-second");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(5, 1, {}, Names.intern("emitter-direct-call-prefix")).has_value());
    MachineBasicBlock &First = Module.createFunction(FirstName).createBasicBlock();
    MachineBasicBlock &Second = Module.createFunction(SecondName).createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(First);
    Builder.create<Opcode::Calli>(ImmediateOperand(SecondName));
    Builder.create<Opcode::Calli>(ImmediateOperand(FirstName));
    Builder.create<Opcode::Ret>();
    Builder.setInsertPoint(Second);
    Builder.create<Opcode::Calli>(ImmediateOperand(FirstName));
    Builder.create<Opcode::Ret>();
    ExecutableEmitter Emitter(Diagnostics);
    const ExecutableFile Executable = Emitter.emit(Module, FirstName, {});

    ASSERT_EQ(Executable.byteSize(), 39U);
    EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(FirstName), 5U);
    EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(SecondName), 27U);
    EXPECT_EQ(readLittleEndian(Executable, 5, 2), 0x003BU);
    EXPECT_EQ(readLittleEndian(Executable, 7), 22U);
    EXPECT_EQ(readLittleEndian(Executable, 15, 2), 0x003BU);
    EXPECT_EQ(readLittleEndian(Executable, 17), static_cast<std::uint64_t>(-10));
    EXPECT_EQ(readLittleEndian(Executable, 25, 2), 0x003AU);
    EXPECT_EQ(readLittleEndian(Executable, 27, 2), 0x003BU);
    EXPECT_EQ(readLittleEndian(Executable, 29), static_cast<std::uint64_t>(-22));
    EXPECT_EQ(readLittleEndian(Executable, 37, 2), 0x003AU);
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // An unresolved direct VM call reserves exactly eight target bytes and patches them only during module finalization.
  TEST(VMExecutableEmitterTest, DefersForwardDirectVMCallUntilModuleFinalization)
  {
    const core::DiagnosticEngine Diagnostics;
    const auto TargetName = core::NamePool::instance().intern("emitter-forward-direct-call-target");
    MachineModule Module;
    MachineBasicBlock &Target = Module.createFunction(TargetName).createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Target);
    Builder.create<Opcode::Ret>();
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.getContext().padZero(3);
    Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(TargetName)));

    ASSERT_EQ(Emitter.getExecutable().byteSize(), 13U);
    EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 3, 2), 0x003BU);
    EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 5), 0U);
    Emitter.padFunction(Module);
    ASSERT_EQ(Emitter.getExecutable().byteSize(), 15U);
    EXPECT_EQ(Emitter.getContext().resolvedSymbolOffset(TargetName), 13U);
    EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 5), 0U);
    Emitter.resolveStaticRelocations();

    const ExecutableFile &Executable = Emitter.getExecutable();
    EXPECT_EQ(readLittleEndian(Executable, 5), 10U);
    EXPECT_EQ(readLittleEndian(Executable, 13, 2), 0x003AU);
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
  }

  // Native direct calls reuse address imports by symbol name and emit four-byte indices without pointer relocations.
  TEST(VMExecutableEmitterTest, SharesNativeImportsAcrossDirectCallsAndAddressFields)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto SharedSymbol = Names.intern("emitter-native-call-shared");
    const auto OtherSymbol = Names.intern("emitter-native-call-other");
    MachineModule Module;
    Module.declareSymbol(SharedSymbol, MachineSymbolKind::Function);
    Module.declareSymbol(SharedSymbol, MachineSymbolKind::Function);
    Module.declareSymbol(OtherSymbol, MachineSymbolKind::Function);
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Instructions.padOperand(ImmediateOperand(SharedSymbol), 64);
    Instructions.padInstruction(MachineCallNativeiInstruction(ImmediateOperand(SharedSymbol)));
    Instructions.padInstruction(MachineCallNativeiInstruction(ImmediateOperand(OtherSymbol)));
    Instructions.padInstruction(MachineCallNativeiInstruction(ImmediateOperand(SharedSymbol)));

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 26U);
    EXPECT_EQ(readLittleEndian(Executable, 0), 0U);
    EXPECT_EQ(readLittleEndian(Executable, 8, 2), 0x003CU);
    EXPECT_EQ(readLittleEndian(Executable, 10, 4), 0U);
    EXPECT_EQ(readLittleEndian(Executable, 14, 2), 0x003CU);
    EXPECT_EQ(readLittleEndian(Executable, 16, 4), 1U);
    EXPECT_EQ(readLittleEndian(Executable, 20, 2), 0x003CU);
    EXPECT_EQ(readLittleEndian(Executable, 22, 4), 0U);
    ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 2U);
    EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, SharedSymbol);
    EXPECT_EQ(Executable.getDynamicImportSymbols()[1].Name, OtherSymbol);
    ASSERT_EQ(Executable.getDynamicImportSymbolRelocations().size(), 1U);
    const auto &Relocation = Executable.getDynamicImportSymbolRelocations()[0];
    EXPECT_EQ(Relocation.ImageOffset, 0U);
    EXPECT_EQ(Relocation.SymbolIndex, 0U);
    EXPECT_EQ(Relocation.Addend, 0);
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Numeric direct calls preserve signed VM displacement bits and the largest native u32 index at their opcode-specific widths.
  TEST(VMExecutableEmitterTest, PreservesNumericDirectCallBoundaryValues)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Instructions.padInstruction(MachineCalliInstruction(ImmediateOperand(static_cast<std::uint64_t>(-16))));
    Instructions.padInstruction(MachineCallNativeiInstruction(ImmediateOperand(std::numeric_limits<std::uint32_t>::max())));

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 16U);
    EXPECT_EQ(readLittleEndian(Executable, 0, 2), 0x003BU);
    EXPECT_EQ(readLittleEndian(Executable, 2), static_cast<std::uint64_t>(-16));
    EXPECT_EQ(readLittleEndian(Executable, 10, 2), 0x003CU);
    EXPECT_EQ(readLittleEndian(Executable, 12, 4), std::numeric_limits<std::uint32_t>::max());
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
  }

  // An add instruction writes its opcode before ordered register and immediate fields, followed immediately by a two-byte return.
  TEST(VMExecutableEmitterTest, EncodesOpcodeBeforeOrderedRegisterAndImmediateOperands)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Instructions.padInstruction(MachineAdd64rriInstruction(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::BP, -8), ImmediateOperand(16)));
    Instructions.padInstruction(MachineRetInstruction());

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 30U);
    EXPECT_EQ(readLittleEndian(Executable, 0, 2), 0x001BU);
    EXPECT_EQ(Executable.getImage()[2], static_cast<std::uint8_t>(RegisterId::SP));
    EXPECT_EQ(readLittleEndian(Executable, 3), 0U);
    EXPECT_EQ(Executable.getImage()[11], static_cast<std::uint8_t>(RegisterId::BP));
    EXPECT_EQ(readLittleEndian(Executable, 12), static_cast<std::uint64_t>(-8));
    EXPECT_EQ(readLittleEndian(Executable, 20), 16U);
    EXPECT_EQ(readLittleEndian(Executable, 28, 2), 0x003AU);
  }

  // A 16-bit store emits only the low two immediate bytes after its nine-byte address without tags or alignment padding.
  TEST(VMExecutableEmitterTest, EncodesNarrowInstructionImmediateWithoutPadding)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Instructions.padInstruction(MachineStore16iInstruction(AddressOperand(RegisterId::SP, -8), ImmediateOperand(0x1122334455667788)));

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 13U);
    EXPECT_EQ(readLittleEndian(Executable, 0, 2), 0x0008U);
    EXPECT_EQ(Executable.getImage()[2], static_cast<std::uint8_t>(RegisterId::SP));
    EXPECT_EQ(readLittleEndian(Executable, 3), static_cast<std::uint64_t>(-8));
    EXPECT_EQ(readLittleEndian(Executable, 11, 2), 0x7788U);
  }

  // Every forward symbol address in one instruction uses the opcode's image offset as its PC-relative origin.
  TEST(VMExecutableEmitterTest, PreservesInstructionStartForForwardPCRelativeOperands)
  {
    const core::DiagnosticEngine Diagnostics;
    const auto Symbol = core::NamePool::instance().intern("emitter-instruction-forward-global");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(8, 16, {}, Symbol).has_value());
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.getContext().padZero(5);
    Instructions.padInstruction(MachineLoad64Instruction(AddressOperand(Symbol, 1), AddressOperand(Symbol, 4)));
    EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 8), 0U);
    EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 17), 0U);
    Emitter.padGlobals(Module);
    Emitter.resolveStaticRelocations();

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 40U);
    EXPECT_EQ(readLittleEndian(Executable, 5, 2), 0x0004U);
    EXPECT_EQ(Executable.getImage()[7], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Executable, 8), 28U);
    EXPECT_EQ(Executable.getImage()[16], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Executable, 17), 31U);
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
  }

  // Numeric immediates retain their low bits at every supported width without creating address relocations.
  TEST(VMExecutableEmitterTest, PreservesNumericImmediateWidths)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    constexpr std::uint64_t Bits = 0x1122334455667788;
    Instructions.padOperand(ImmediateOperand(Bits), 8);
    Instructions.padOperand(ImmediateOperand(Bits), 16);
    Instructions.padOperand(ImmediateOperand(Bits), 32);
    Instructions.padOperand(ImmediateOperand(Bits), 64);

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 15U);
    EXPECT_EQ(readLittleEndian(Executable, 0, 1), 0x88U);
    EXPECT_EQ(readLittleEndian(Executable, 1, 2), 0x7788U);
    EXPECT_EQ(readLittleEndian(Executable, 3, 4), 0x55667788U);
    EXPECT_EQ(readLittleEndian(Executable, 7), Bits);
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
  }

  // ZERO register values and memory addresses share the nine-byte register-relative layout and preserve every signed offset bit.
  TEST(VMExecutableEmitterTest, EncodesZeroRelativeRegisterAndAddressOperands)
  {
    const core::DiagnosticEngine Diagnostics;
    const std::array<std::int64_t, 5> Offsets{0, 17, -17, std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()};
    for (const auto Offset : Offsets)
    {
      SCOPED_TRACE(Offset);
      ExecutableEmitter Emitter(Diagnostics);
      FunctionEmissionContext FunctionContext;
      InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
      Instructions.padOperand(RegisterOperand(RegisterId::ZERO, Offset));
      Instructions.padOperand(AddressOperand(RegisterId::ZERO, Offset), 9);

      const ExecutableFile &Executable = Emitter.getExecutable();
      ASSERT_EQ(Executable.byteSize(), 18U);
      EXPECT_EQ(Executable.getImage()[0], static_cast<std::uint8_t>(RegisterId::ZERO));
      EXPECT_EQ(Executable.getImage()[9], static_cast<std::uint8_t>(RegisterId::ZERO));
      EXPECT_EQ(readLittleEndian(Executable, 1), static_cast<std::uint64_t>(Offset));
      EXPECT_EQ(readLittleEndian(Executable, 10), static_cast<std::uint64_t>(Offset));
      EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
      EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    }
  }

  // Already laid-out VM symbols still require image-base binding and preserve signed addends outside the placeholder.
  TEST(VMExecutableEmitterTest, EmitsBaseRelocationsForLaidOutVMSymbols)
  {
    const core::DiagnosticEngine Diagnostics;
    const auto Symbol = core::NamePool::instance().intern("emitter-resolved-global");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(16, 8, {}, Symbol).has_value());
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.getContext().padZero(3);
    Emitter.padGlobals(Module);
    Instructions.padOperand(ImmediateOperand(Symbol, -3), 64);

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 32U);
    ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 1U);
    const auto &Relocation = Executable.getImageBaseAddressRelocations()[0];
    EXPECT_EQ(Relocation.ImageOffset, 24U);
    EXPECT_EQ(Relocation.TargetOffset, 8U);
    EXPECT_EQ(Relocation.Addend, -3);
    EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Forward VM references remain pending through global layout and are processed together by explicit finalization.
  TEST(VMExecutableEmitterTest, ResolvesForwardVMAddressReferencesAfterLayoutFinalization)
  {
    const core::DiagnosticEngine Diagnostics;
    const auto Symbol = core::NamePool::instance().intern("emitter-forward-global");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(16, 16, {}, Symbol).has_value());
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.getContext().padZero(1);
    Instructions.padOperand(ImmediateOperand(Symbol, -1), 64);
    Instructions.padOperand(ImmediateOperand(Symbol, 8), 64);
    Instructions.padOperand(AddressOperand(Symbol, 4), 17);
    EXPECT_TRUE(Emitter.getExecutable().getImageBaseAddressRelocations().empty());
    Emitter.padGlobals(Module);
    EXPECT_TRUE(Emitter.getExecutable().getImageBaseAddressRelocations().empty());
    EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 18), 0U);
    Emitter.resolveStaticRelocations();

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 48U);
    ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 2U);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].ImageOffset, 1U);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].TargetOffset, 32U);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].Addend, -1);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].ImageOffset, 9U);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].TargetOffset, 32U);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[1].Addend, 8);
    EXPECT_EQ(readLittleEndian(Executable, 1), 0U);
    EXPECT_EQ(readLittleEndian(Executable, 9), 0U);
    EXPECT_EQ(Executable.getImage()[17], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Executable, 18), 19U);
    Emitter.resolveStaticRelocations();
    EXPECT_EQ(Executable.getImageBaseAddressRelocations().size(), 2U);
    EXPECT_EQ(readLittleEndian(Executable, 18), 19U);
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // VM memory addresses retain PC-relative encoding for both deferred forward references and resolved backward references.
  TEST(VMExecutableEmitterTest, PreservesPCRelativeAddressesBeforeAndAfterLayout)
  {
    const core::DiagnosticEngine Diagnostics;
    const auto Symbol = core::NamePool::instance().intern("emitter-pc-global");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(8, 16, {}, Symbol).has_value());
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.getContext().padZero(3);
    Instructions.padOperand(AddressOperand(Symbol, -2), 3);
    Emitter.padGlobals(Module);
    Instructions.padOperand(AddressOperand(Symbol, 4), 24);
    EXPECT_EQ(readLittleEndian(Emitter.getExecutable(), 4), 0U);
    Emitter.resolveStaticRelocations();

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 33U);
    EXPECT_EQ(Executable.getImage()[3], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Executable, 4), 11U);
    EXPECT_EQ(Executable.getImage()[24], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Executable, 25), static_cast<std::uint64_t>(-4));
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // VM and Native pointers retain separate loading records while their memory references share the same nine-byte register-relative format.
  TEST(VMExecutableEmitterTest, KeepsVMAndNativeAddressFieldsInSeparateLoadingTables)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto VMSymbol = Names.intern("emitter-mixed-vm");
    const auto NativeSymbol = Names.intern("emitter-mixed-native");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(16, 1, {}, VMSymbol).has_value());
    Module.declareSymbol(NativeSymbol, MachineSymbolKind::GlobalVariable);
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.padGlobals(Module);
    Instructions.padOperand(ImmediateOperand(VMSymbol, 8), 64);
    Instructions.padOperand(ImmediateOperand(NativeSymbol, -8), 64);
    Instructions.padOperand(AddressOperand(VMSymbol, 8), 32);
    Instructions.padOperand(AddressOperand(NativeSymbol, 12), 41);

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 50U);
    ASSERT_EQ(Executable.getImageBaseAddressRelocations().size(), 1U);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].ImageOffset, 16U);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].TargetOffset, 0U);
    EXPECT_EQ(Executable.getImageBaseAddressRelocations()[0].Addend, 8);
    ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 1U);
    EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, NativeSymbol);
    ASSERT_EQ(Executable.getDynamicImportSymbolRelocations().size(), 2U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].ImageOffset, 24U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex, 0U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].Addend, -8);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].ImageOffset, 42U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].SymbolIndex, 0U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].Addend, 12);
    EXPECT_EQ(readLittleEndian(Executable, 16), 0U);
    EXPECT_EQ(readLittleEndian(Executable, 24), 0U);
    EXPECT_EQ(Executable.getImage()[32], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Executable, 33), static_cast<std::uint64_t>(-24));
    EXPECT_EQ(Executable.getImage()[41], static_cast<std::uint8_t>(RegisterId::ZERO));
    EXPECT_EQ(readLittleEndian(Executable, 42), 0U);
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Repeated Native declarations share one import across ZERO-relative memory addresses and ordinary address immediates.
  TEST(VMExecutableEmitterTest, SharesNativeImportsAcrossImmediateAndMemoryAddressFields)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto NativeSymbol = Names.intern("emitter-shared-native");
    MachineModule Module;
    Module.declareSymbol(NativeSymbol, MachineSymbolKind::GlobalVariable);
    Module.declareSymbol(NativeSymbol, MachineSymbolKind::GlobalVariable);
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.getContext().padZero(1);
    Instructions.padOperand(AddressOperand(NativeSymbol, 12), 0);
    Instructions.padOperand(ImmediateOperand(NativeSymbol, -4), 64);

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 18U);
    EXPECT_EQ(Executable.getImage()[1], static_cast<std::uint8_t>(RegisterId::ZERO));
    EXPECT_EQ(readLittleEndian(Executable, 2), 0U);
    EXPECT_EQ(readLittleEndian(Executable, 10), 0U);
    ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 1U);
    EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, NativeSymbol);
    ASSERT_EQ(Executable.getDynamicImportSymbolRelocations().size(), 2U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].ImageOffset, 2U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex, 0U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].Addend, 12);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].ImageOffset, 10U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].SymbolIndex, 0U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].Addend, -4);
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Starting another module emission discards both loading tables and the previous image instead of leaking old pointer fields.
  TEST(VMExecutableEmitterTest, ClearsAddressRelocationsBeforeEmittingAnotherModule)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto VMSymbol = Names.intern("emitter-reset-vm");
    const auto NativeSymbol = Names.intern("emitter-reset-native");
    MachineModule FirstModule;
    ASSERT_TRUE(FirstModule.createGlobalVariable(8, 1, {}, VMSymbol).has_value());
    FirstModule.declareSymbol(NativeSymbol, MachineSymbolKind::GlobalVariable);
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(FirstModule);
    Emitter.padGlobals(FirstModule);
    Instructions.padOperand(ImmediateOperand(VMSymbol), 64);
    Instructions.padOperand(ImmediateOperand(NativeSymbol), 64);
    ASSERT_EQ(Emitter.getExecutable().getImageBaseAddressRelocations().size(), 1U);
    ASSERT_EQ(Emitter.getExecutable().getDynamicImportSymbolRelocations().size(), 1U);

    MachineModule SecondModule;
    const std::array<std::uint8_t, 2> InitialBytes{0xAB, 0xCD};
    ASSERT_TRUE(SecondModule.createGlobalVariable(2, 1, InitialBytes, Names.intern("emitter-reset-next")).has_value());
    const auto EntryName = Names.intern("emitter-reset-next-entry");
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(SecondModule.createFunction(EntryName).createBasicBlock());
    Builder.create<Opcode::Ret>();
    const ExecutableFile Executable = Emitter.emit(SecondModule, EntryName, {});
    ASSERT_EQ(Executable.byteSize(), 4U);
    EXPECT_EQ(Executable.getEntryOffset(), 2U);
    EXPECT_FALSE(Emitter.getContext().isNativeSymbol(NativeSymbol));
    EXPECT_EQ(Executable.getImage()[0], 0xABU);
    EXPECT_EQ(Executable.getImage()[1], 0xCDU);
    EXPECT_EQ(readLittleEndian(Executable, 2, 2), 0x003AU);
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Explicit entry selection can skip the first function, and listed local code and data exports preserve caller order regardless of linkage.
  TEST(VMExecutableEmitterTest, SelectsExplicitEntryAndOrderedMixedExports)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto GlobalName = Names.intern("emitter-selected-export-global");
    const auto FirstName = Names.intern("emitter-unlisted-external-function");
    const auto EntryName = Names.intern("emitter-selected-local-entry");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(1, 1, {}, Names.intern("emitter-selected-entry-prefix")).has_value());
    ASSERT_TRUE(Module.createGlobalVariable(8, 8, {}, GlobalName).has_value());
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Module.createFunction(FirstName, MachineSymbolLinkage::External).createBasicBlock());
    Builder.create<Opcode::Ret>();
    Builder.setInsertPoint(Module.createFunction(EntryName, MachineSymbolLinkage::Local).createBasicBlock());
    Builder.create<Opcode::Ret>();
    const std::array<core::Name, 2> ExportNames{EntryName, GlobalName};
    ExecutableEmitter Emitter(Diagnostics);
    const ExecutableFile Executable = Emitter.emit(Module, EntryName, ExportNames);

    ASSERT_EQ(Executable.byteSize(), 20U);
    EXPECT_EQ(Executable.getEntryOffset(), 18U);
    EXPECT_EQ(readLittleEndian(Executable, 18, 2), 0x003AU);
    ASSERT_EQ(Executable.getExportSymbols().size(), 2U);
    EXPECT_EQ(Executable.getExportSymbols()[0].Name, EntryName);
    EXPECT_EQ(Executable.getExportSymbols()[0].Offset, 18U);
    EXPECT_EQ(Executable.getExportSymbols()[1].Name, GlobalName);
    EXPECT_EQ(Executable.getExportSymbols()[1].Offset, 8U);
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // A reused emitter replaces a nonzero entry and populated exports with an explicitly selected zero-offset entry and an empty export list.
  TEST(VMExecutableEmitterTest, ReplacesEntryAndExportsWhenEmittingAnotherModule)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto GlobalName = Names.intern("emitter-entry-reset-global");
    const auto FirstEntryName = Names.intern("emitter-entry-reset-first");
    const auto SecondEntryName = Names.intern("emitter-entry-reset-second");
    MachineModule FirstModule;
    ASSERT_TRUE(FirstModule.createGlobalVariable(4, 1, {}, GlobalName).has_value());
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(FirstModule.createFunction(FirstEntryName).createBasicBlock());
    Builder.create<Opcode::Ret>();
    const std::array<core::Name, 2> FirstExports{FirstEntryName, GlobalName};
    ExecutableEmitter Emitter(Diagnostics);
    const ExecutableFile First = Emitter.emit(FirstModule, FirstEntryName, FirstExports);
    ASSERT_EQ(First.getEntryOffset(), 4U);
    ASSERT_EQ(First.getExportSymbols().size(), 2U);

    MachineModule SecondModule;
    Builder.setInsertPoint(SecondModule.createFunction(SecondEntryName).createBasicBlock());
    Builder.create<Opcode::Ret>();
    const ExecutableFile Second = Emitter.emit(SecondModule, SecondEntryName, {});

    ASSERT_EQ(Second.byteSize(), 2U);
    EXPECT_EQ(Second.getEntryOffset(), 0U);
    EXPECT_TRUE(Second.getExportSymbols().empty());
    EXPECT_EQ(readLittleEndian(Second, 0, 2), 0x003AU);
    EXPECT_EQ(First.getEntryOffset(), 4U);
    EXPECT_EQ(First.getExportSymbols().size(), 2U);
    ExecutableFileValidator::validate(Second, Diagnostics);
  }

  // An entry must be a registered VM function with instructions, even when an empty function shares its offset with later code.
  TEST(VMExecutableEmitterDeathTest, RejectsInvalidEntrySelections)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto GlobalName = Names.intern("emitter-invalid-entry-global");
    const auto EmptyName = Names.intern("emitter-invalid-entry-empty-function");
    const auto ValidName = Names.intern("emitter-invalid-entry-valid-function");
    const auto NativeName = Names.intern("emitter-invalid-entry-native");
    const auto UnknownName = Names.intern("emitter-invalid-entry-unknown");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(1, 1, {}, GlobalName).has_value());
    Module.createFunction(EmptyName);
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Module.createFunction(ValidName).createBasicBlock());
    Builder.create<Opcode::Ret>();
    Module.declareSymbol(NativeName, MachineSymbolKind::Function);
    ExecutableEmitter Emitter(Diagnostics);

    for (const auto EntryName : {UnknownName, GlobalName, NativeName})
    {
      SCOPED_TRACE(EntryName.index());
      EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, {})), "INK-V0002.*entry must name a defined VM function");
    }
    EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, core::NAME_None, {})), "INK-V0002.*entry function name must be registered and nonempty");
    EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EmptyName, {})), "INK-V0002.*entry function must contain at least one instruction");
  }

  // Exports reject duplicate or invalid names, missing VM offsets, and a trailing empty function whose offset equals the image size.
  TEST(VMExecutableEmitterDeathTest, RejectsInvalidExportSelections)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto GlobalName = Names.intern("emitter-invalid-export-global");
    const auto EntryName = Names.intern("emitter-invalid-export-entry");
    const auto EmptyName = Names.intern("emitter-invalid-export-trailing-empty-function");
    const auto NativeName = Names.intern("emitter-invalid-export-native");
    const auto UnknownName = Names.intern("emitter-invalid-export-unknown");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(1, 1, {}, GlobalName).has_value());
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Module.createFunction(EntryName).createBasicBlock());
    Builder.create<Opcode::Ret>();
    Module.createFunction(EmptyName);
    Module.declareSymbol(NativeName, MachineSymbolKind::Function);
    ExecutableEmitter Emitter(Diagnostics);
    const std::array<core::Name, 2> Duplicated{GlobalName, GlobalName};
    EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, Duplicated)), "INK-V0002.*export symbol name is duplicated");
    for (const auto Name : {UnknownName, NativeName})
    {
      SCOPED_TRACE(Name.index());
      const std::array<core::Name, 1> ExportNames{Name};
      EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, ExportNames)), "INK-V0002.*export symbol has no VM image offset");
    }
    const std::array<core::Name, 1> InvalidNames{core::NAME_None};
    EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, InvalidNames)), "INK-V0002.*export symbol name must be registered and nonempty");
    const std::array<core::Name, 1> TrailingEmptyFunction{EmptyName};
    EXPECT_DEATH(static_cast<void>(Emitter.emit(Module, EntryName, TrailingEmptyFunction)), "INK-V0002.*export symbol offset is outside the image");
  }

  // Finalization rejects a referenced function whose entry was never assigned an image offset.
  TEST(VMExecutableEmitterDeathTest, RejectsUnlaidOutFunctionTargetsDuringFinalization)
  {
    const core::DiagnosticEngine Diagnostics;
    const auto Symbol = core::NamePool::instance().intern("emitter-unlaid-function");
    MachineModule Module;
    Module.createFunction(Symbol);
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Instructions.padOperand(ImmediateOperand(Symbol), 64);
    Emitter.padGlobals(Module);
    EXPECT_DEATH(Emitter.resolveStaticRelocations(), "INK-V0002.*VM symbol has no image offset after layout");
  }

  // Ordinary VM and Native symbol addresses reject narrow fields even when their image offsets would fit those fields.
  TEST(VMExecutableEmitterDeathTest, RejectsNarrowSymbolAddressImmediateFields)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto VMSymbol = Names.intern("emitter-narrow-vm");
    const auto NativeSymbol = Names.intern("emitter-narrow-native");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(8, 1, {}, VMSymbol).has_value());
    Module.declareSymbol(NativeSymbol, MachineSymbolKind::GlobalVariable);
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.padGlobals(Module);
    for (const std::uint32_t Width : {8U, 16U, 32U})
    {
      SCOPED_TRACE(Width);
      EXPECT_DEATH(Instructions.padOperand(ImmediateOperand(VMSymbol), Width), "INK-V0002.*64-bit immediate");
      EXPECT_DEATH(Instructions.padOperand(ImmediateOperand(NativeSymbol), Width), "INK-V0002.*64-bit immediate");
    }
  }
} // namespace ink::vm::test
