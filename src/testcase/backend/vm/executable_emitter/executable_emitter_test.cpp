#include "ink/backend/vm/executable/executable_file_validator.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/executable_emitter/function_emission_context.h"
#include "ink/backend/vm/executable_emitter/instruction_emitter.h"
#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/instruction/register_operand.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

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
    const auto HostSymbol = Names.intern("emitter-mixed-host");
    MachineModule Module;
    ASSERT_TRUE(Module.createGlobalVariable(16, 1, {}, VMSymbol).has_value());
    Module.declareNativeData(NativeSymbol, NativeDataImport{HostSymbol});
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
    EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, HostSymbol);
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

  // Native aliases share one import across ZERO-relative memory addresses and ordinary address immediates without moving their relocation fields.
  TEST(VMExecutableEmitterTest, SharesNativeImportsAcrossImmediateAndMemoryAddressFields)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    const auto FirstAlias = Names.intern("emitter-native-memory");
    const auto SecondAlias = Names.intern("emitter-native-immediate");
    const auto HostSymbol = Names.intern("emitter-shared-host");
    MachineModule Module;
    Module.declareNativeData(FirstAlias, NativeDataImport{HostSymbol});
    Module.declareNativeData(SecondAlias, NativeDataImport{HostSymbol});
    ExecutableEmitter Emitter(Diagnostics);
    FunctionEmissionContext FunctionContext;
    InstructionEmitter Instructions(Emitter.getContext(), FunctionContext);
    Emitter.scanUnResolvedSymbol(Module);
    Emitter.getContext().padZero(1);
    Instructions.padOperand(AddressOperand(FirstAlias, 12), 0);
    Instructions.padOperand(ImmediateOperand(SecondAlias, -4), 64);

    const ExecutableFile &Executable = Emitter.getExecutable();
    ASSERT_EQ(Executable.byteSize(), 18U);
    EXPECT_EQ(Executable.getImage()[1], static_cast<std::uint8_t>(RegisterId::ZERO));
    EXPECT_EQ(readLittleEndian(Executable, 2), 0U);
    EXPECT_EQ(readLittleEndian(Executable, 10), 0U);
    ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 1U);
    EXPECT_EQ(Executable.getDynamicImportSymbols()[0].Name, HostSymbol);
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
    FirstModule.declareNativeData(NativeSymbol, NativeDataImport{Names.intern("emitter-reset-host")});
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
    const ExecutableFile Executable = Emitter.emit(SecondModule);
    ASSERT_EQ(Executable.byteSize(), 2U);
    EXPECT_EQ(Executable.getImage()[0], 0xABU);
    EXPECT_EQ(Executable.getImage()[1], 0xCDU);
    EXPECT_TRUE(Executable.getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
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
    Module.declareNativeData(NativeSymbol, NativeDataImport{Names.intern("emitter-narrow-host")});
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
