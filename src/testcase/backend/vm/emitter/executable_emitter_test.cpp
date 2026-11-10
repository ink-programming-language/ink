#include "ink/backend/vm/emitter/executable_emitter.h"
#include "ink/backend/vm/bytecode/instruction_codec.h"
#include "ink/backend/vm/executable/executable_file_validator.h"
#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace ink::vm::test
{
  namespace
  {
    core::Name emissionName(std::string_view Text)
    {
      return core::NamePool::instance().intern(Text);
    }

    std::optional<ExecutableFile> emitModule(const MachineModule &Module, const MachineFunction &Entry, ByteOrder Order = ByteOrder::Little)
    {
      core::CollectingDiagnosticConsumer Collector;
      core::DiagnosticEngine Diagnostics;
      Diagnostics.addConsumer(Collector);
      auto Result = ExecutableEmitter::emit(Module, Entry, Diagnostics, Order);
      EXPECT_TRUE(Collector.diagnostics().empty());
      if (Result)
      {
        ExecutableFileValidator::validate(*Result, Diagnostics);
      }
      return Result;
    }

    void expectEmissionFailure(const MachineModule &Module, const MachineFunction &Entry, ByteOrder Order = ByteOrder::Little)
    {
      core::CollectingDiagnosticConsumer Collector;
      core::DiagnosticEngine Diagnostics;
      Diagnostics.addConsumer(Collector);
      EXPECT_FALSE(ExecutableEmitter::emit(Module, Entry, Diagnostics, Order));
      ASSERT_EQ(Collector.diagnostics().size(), 1U);
      EXPECT_EQ(Collector.diagnostics()[0].Kind, core::DiagnosticKind::VMExecutableEmissionFailed);
      EXPECT_EQ(Collector.diagnostics()[0].classification(), core::DiagnosticClass::User);
    }

    DecodedInstruction decodeAt(const ExecutableFile &Executable, std::size_t ByteOffset)
    {
      EXPECT_LT(ByteOffset, Executable.getImage().size());
      if (ByteOffset >= Executable.getImage().size())
      {
        return {};
      }
      const auto Decoded = InstructionCodec::decode(std::span<const std::uint8_t>(Executable.getImage()).subspan(ByteOffset), Executable.getByteOrder());
      EXPECT_TRUE(Decoded);
      return Decoded ? *Decoded : DecodedInstruction{};
    }

    std::int64_t signedBits(const BytecodeOperand &Operand)
    {
      return std::bit_cast<std::int64_t>(Operand.Bits);
    }

    void appendReturn(MachineFunction &Function)
    {
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Function.createBasicBlock());
      Builder.create<Opcode::Ret>();
    }
  } // namespace

  // A cross-module sum call becomes a PC-relative byte target after linking and remains independent of its source modules.
  TEST(VMExecutableEmitterTest, EmitsLinkedCrossModuleCallsAndArithmetic)
  {
    const auto CallerName = emissionName("emit.cross.caller");
    const auto SumName = emissionName("emit.cross.sum");
    std::optional<ExecutableFile> Executable;
    {
      MachineModule Caller;
      MachineModule Callee;
      Caller.declareSymbol(SumName, MachineSymbolKind::Function);
      auto &CallerFunction = Caller.createFunction(CallerName, MachineSymbolLinkage::External);
      auto &SumFunction = Callee.createFunction(SumName, MachineSymbolLinkage::External);
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(CallerFunction.createBasicBlock());
      Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), ImmediateOperand(24));
      Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::SP, -16), ImmediateOperand(20));
      Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::SP, -8), ImmediateOperand(22));
      const auto &SourceCall = Builder.create<Opcode::Calli>(ImmediateOperand(SumName));
      Builder.create<Opcode::Ret>();
      Builder.setInsertPoint(SumFunction.createBasicBlock());
      Builder.create<Opcode::Add64mmm>(AddressOperand(RegisterId::BP, -24), AddressOperand(RegisterId::BP, -16), AddressOperand(RegisterId::BP, -8));
      Builder.create<Opcode::Ret>();
      const std::array<const MachineModule *, 2> Inputs{&Caller, &Callee};
      core::CollectingDiagnosticConsumer Collector;
      core::DiagnosticEngine Diagnostics;
      Diagnostics.addConsumer(Collector);
      const auto Linked = ModuleLinker::link(Inputs, Diagnostics);
      ASSERT_NE(Linked, nullptr);
      ASSERT_TRUE(Collector.diagnostics().empty());
      Executable = emitModule(*Linked, *Linked->functions().begin());
      EXPECT_EQ(SourceCall.getTarget().getSymbol(), SumName);
      EXPECT_EQ(Caller.functionCount(), 1U);
      EXPECT_EQ(Callee.functionCount(), 1U);
    }
    ASSERT_TRUE(Executable);
    EXPECT_EQ(Executable->getEntryOffset(), 0U);
    EXPECT_EQ(Executable->byteSize(), 93U);
    const auto Call = decodeAt(*Executable, 50);
    EXPECT_EQ(Call.Instruction.Operation, Opcode::Calli);
    EXPECT_EQ(signedBits(Call.Instruction.Operands[0]), 12);
    EXPECT_EQ(decodeAt(*Executable, 62).Instruction.Operation, Opcode::Add64mmm);
    EXPECT_EQ(decodeAt(*Executable, 91).Instruction.Operation, Opcode::Ret);
  }

  // Both byte orders preserve the documented add64rri encoding and a nonfirst function's entry offset.
  TEST(VMExecutableEmitterTest, EmitsRequestedByteOrderAndExplicitEntry)
  {
    MachineModule Module;
    auto &First = Module.createFunction();
    appendReturn(First);
    auto &Entry = Module.createFunction();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Entry.createBasicBlock());
    Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::SP), ImmediateOperand(16));
    Builder.create<Opcode::Ret>();
    const std::array<std::uint8_t, 12> Little{0x1b, 0x00, 0x01, 0x01, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    const std::array<std::uint8_t, 12> Big{0x00, 0x1b, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10};
    for (const auto Order : {ByteOrder::Little, ByteOrder::Big})
    {
      const auto Executable = emitModule(Module, Entry, Order);
      ASSERT_TRUE(Executable);
      EXPECT_EQ(Executable->getByteOrder(), Order);
      EXPECT_EQ(Executable->getEntryOffset(), 2U);
      EXPECT_EQ(Executable->byteSize(), 16U);
      const auto &Expected = Order == ByteOrder::Little ? Little : Big;
      EXPECT_TRUE(std::equal(Expected.begin(), Expected.end(), Executable->getImage().begin() + 2));
      const auto Add = decodeAt(*Executable, 2);
      EXPECT_EQ(Add.Instruction.Operation, Opcode::Add64rri);
      EXPECT_EQ(Add.Instruction.Operands[2].Bits, 16U);
    }
  }

  // Read-only and writable globals form distinct ranges with aligned objects and zero-filled padding and suffixes.
  TEST(VMExecutableEmitterTest, GroupsAlignedGlobalDataAndZeroFillsStorage)
  {
    MachineModule Module;
    auto &Entry = Module.createFunction();
    appendReturn(Entry);
    const std::array<std::uint8_t, 2> WritableBytes{0x31, 0x32};
    const std::array<std::uint8_t, 1> ReadOnlyBytes{0xaa};
    const std::array<std::uint8_t, 2> SecondReadOnlyBytes{0xbb, 0xcc};
    ASSERT_TRUE(Module.createGlobalVariable(5, 8, WritableBytes, emissionName("emit.data.writable")));
    ASSERT_TRUE(Module.createGlobalVariable(4, 16, ReadOnlyBytes, emissionName("emit.data.readonly"), MachineSymbolLinkage::Local, true));
    ASSERT_TRUE(Module.createGlobalVariable(2, 4, SecondReadOnlyBytes, emissionName("emit.data.readonly.second"), MachineSymbolLinkage::Local, true));
    ASSERT_TRUE(Module.createGlobalVariable(8, 16, {}, emissionName("emit.data.writable.second")));
    const auto Executable = emitModule(Module, Entry);
    ASSERT_TRUE(Executable);
    EXPECT_EQ(Executable->byteSize(), 40U);
    ASSERT_EQ(Executable->getSections().size(), 3U);
    const auto &Sections = Executable->getSections();
    EXPECT_EQ(Sections[0].Kind, ExecutableSectionKind::Code);
    EXPECT_EQ(Sections[0].ImageOffset, 0U);
    EXPECT_EQ(Sections[0].ByteSize, 2U);
    EXPECT_EQ(Sections[1].Kind, ExecutableSectionKind::ReadOnlyData);
    EXPECT_EQ(Sections[1].ImageOffset, 2U);
    EXPECT_EQ(Sections[1].ByteSize, 20U);
    EXPECT_EQ(Sections[2].Kind, ExecutableSectionKind::WritableData);
    EXPECT_EQ(Sections[2].ImageOffset, 22U);
    EXPECT_EQ(Sections[2].ByteSize, 18U);
    std::vector<std::uint8_t> Expected(40, 0);
    Expected[0] = 0x3a;
    Expected[16] = 0xaa;
    Expected[20] = 0xbb;
    Expected[21] = 0xcc;
    Expected[24] = 0x31;
    Expected[25] = 0x32;
    EXPECT_EQ(Executable->getImage(), Expected);
    EXPECT_EQ(Module.globalVariables().begin()->initialBytes().size(), 2U);
    EXPECT_FALSE(Module.globalVariables().begin()->isConstant());
  }

  // VM function addresses are image offsets while symbol memory operands and indirect call slots become PC-relative.
  TEST(VMExecutableEmitterTest, ResolvesVMAddressValuesAndSymbolMemory)
  {
    MachineModule Module;
    const auto HelperName = emissionName("emit.address.helper");
    const auto SlotName = emissionName("emit.address.slot");
    auto &Entry = Module.createFunction();
    auto &Helper = Module.createFunction(HelperName);
    ASSERT_TRUE(Module.createGlobalVariable(16, 8, {}, SlotName));
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Entry.createBasicBlock());
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, 16), ImmediateOperand(HelperName));
    Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP, 24), AddressOperand(SlotName, 3));
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, 32), ImmediateOperand(SlotName, 1));
    Builder.create<Opcode::Callm>(AddressOperand(SlotName));
    Builder.create<Opcode::Ret>();
    appendReturn(Helper);
    const auto Executable = emitModule(Module, Entry);
    ASSERT_TRUE(Executable);
    EXPECT_EQ(Executable->byteSize(), 96U);
    EXPECT_EQ(decodeAt(*Executable, 0).Instruction.Operands[1].Bits, 71U);
    const auto Load = decodeAt(*Executable, 19);
    EXPECT_EQ(Load.Instruction.Operands[1].AddressBase, EncodedAddressBase::PC);
    EXPECT_EQ(signedBits(Load.Instruction.Operands[1]), 64);
    EXPECT_EQ(decodeAt(*Executable, 39).Instruction.Operands[1].Bits, 81U);
    const auto IndirectCall = decodeAt(*Executable, 58);
    EXPECT_EQ(IndirectCall.Instruction.Operation, Opcode::Callm);
    EXPECT_EQ(IndirectCall.Instruction.Operands[0].AddressBase, EncodedAddressBase::PC);
    EXPECT_EQ(signedBits(IndirectCall.Instruction.Operands[0]), 22);
    EXPECT_TRUE(Executable->getNativeDataRelocations().empty());
  }

  // Forward, backward and self branches retain their target identity, and recursive calls use the current instruction PC.
  TEST(VMExecutableEmitterTest, ResolvesBranchesAndRecursiveCallDisplacements)
  {
    MachineModule Module;
    const auto FunctionName = emissionName("emit.control.recursive");
    auto &Entry = Module.createFunction(FunctionName);
    auto &Begin = Entry.createBasicBlock();
    auto &Loop = Entry.createBasicBlock();
    auto &Tail = Entry.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Begin);
    Builder.create<Opcode::Jmp>(&Loop);
    Builder.setInsertPoint(Loop);
    Builder.create<Opcode::Calli>(ImmediateOperand(FunctionName));
    Builder.create<Opcode::Jne>(&Loop);
    Builder.create<Opcode::Jmp>(&Tail);
    Builder.setInsertPoint(Tail);
    Builder.create<Opcode::Jmp>(&Tail);
    Builder.create<Opcode::Ret>();
    const auto Executable = emitModule(Module, Entry);
    ASSERT_TRUE(Executable);
    EXPECT_EQ(Executable->byteSize(), 56U);
    EXPECT_EQ(signedBits(decodeAt(*Executable, 0).Instruction.Operands[0]), 11);
    EXPECT_EQ(signedBits(decodeAt(*Executable, 11).Instruction.Operands[0]), -11);
    EXPECT_EQ(signedBits(decodeAt(*Executable, 21).Instruction.Operands[0]), -10);
    EXPECT_EQ(signedBits(decodeAt(*Executable, 32).Instruction.Operands[0]), 11);
    EXPECT_EQ(signedBits(decodeAt(*Executable, 43).Instruction.Operands[0]), 0);
  }

  // Native calls reference the import table and host data uses distinct address fields with exact relocation offsets.
  TEST(VMExecutableEmitterTest, EmitsNativeImportsAndHostDataRelocations)
  {
    MachineModule Module;
    const auto FirstName = emissionName("emit.native.first");
    const auto SecondName = emissionName("emit.native.second");
    const auto DataName = emissionName("emit.native.buffer");
    Module.declareNativeFunction(FirstName, {NativeSymbolProvider::Native, emissionName("host.emit.first"), NativeCallKind::C});
    Module.declareNativeFunction(SecondName, {NativeSymbolProvider::SystemApi, emissionName("host.emit.second"), NativeCallKind::InkNative});
    Module.declareNativeData(DataName, {emissionName("host.emit.buffer")});
    auto &Entry = Module.createFunction();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Entry.createBasicBlock());
    Builder.create<Opcode::CallNativei>(ImmediateOperand(SecondName));
    Builder.create<Opcode::CallNativei>(ImmediateOperand(0));
    Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP, 16), AddressOperand(DataName, 12));
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, 24), ImmediateOperand(DataName, -3));
    Builder.create<Opcode::CallNativem>(AddressOperand(RegisterId::BP, 32));
    Builder.create<Opcode::CallNativem>(AddressOperand(DataName, 8));
    Builder.create<Opcode::Callm>(AddressOperand(RegisterId::BP, 40));
    Builder.create<Opcode::Ret>();
    for (const auto Order : {ByteOrder::Little, ByteOrder::Big})
    {
      const auto Executable = emitModule(Module, Entry, Order);
      ASSERT_TRUE(Executable);
      ASSERT_EQ(Executable->getNativeFunctions().size(), 2U);
      ASSERT_EQ(Executable->getNativeData().size(), 1U);
      EXPECT_EQ(Executable->getNativeFunctions()[0].Name, emissionName("host.emit.first"));
      EXPECT_EQ(Executable->getNativeFunctions()[1].CallKind, NativeCallKind::InkNative);
      EXPECT_EQ(Executable->getNativeData()[0].Name, emissionName("host.emit.buffer"));
      EXPECT_EQ(decodeAt(*Executable, 0).Instruction.Operands[0].Bits, 1U);
      EXPECT_EQ(decodeAt(*Executable, 6).Instruction.Operands[0].Bits, 0U);
      const auto Load = decodeAt(*Executable, 12);
      EXPECT_EQ(Load.Instruction.Operands[1].AddressBase, EncodedAddressBase::Native);
      EXPECT_EQ(Load.Instruction.Operands[1].Bits, 0U);
      EXPECT_EQ(decodeAt(*Executable, 32).Instruction.Operands[1].Bits, 0U);
      EXPECT_EQ(decodeAt(*Executable, 51).Instruction.Operands[0].AddressBase, EncodedAddressBase::BP);
      EXPECT_EQ(decodeAt(*Executable, 62).Instruction.Operands[0].AddressBase, EncodedAddressBase::Native);
      ASSERT_EQ(Executable->getNativeDataRelocations().size(), 3U);
      const std::array<std::uint64_t, 3> Offsets{24, 43, 65};
      const std::array<std::int64_t, 3> Addends{12, -3, 8};
      for (std::size_t Index = 0; Index < Offsets.size(); ++Index)
      {
        const auto &Relocation = Executable->getNativeDataRelocations()[Index];
        EXPECT_EQ(Relocation.ImageOffset, Offsets[Index]);
        EXPECT_EQ(Relocation.SymbolIndex, 0U);
        EXPECT_EQ(Relocation.Addend, Addends[Index]);
        ASSERT_LE(Relocation.ImageOffset, Executable->byteSize());
        ASSERT_LE(8U, Executable->byteSize() - Relocation.ImageOffset);
        const auto Field = std::span<const std::uint8_t>(Executable->getImage()).subspan(static_cast<std::size_t>(Relocation.ImageOffset), 8);
        EXPECT_TRUE(std::all_of(Field.begin(), Field.end(), [](std::uint8_t Byte)
                                {
                                  return Byte == 0;
                                }));
      }
    }
  }

  // Distinct VM aliases of one host identity share import indices while preserving each host-data relocation addend.
  TEST(VMExecutableEmitterTest, CanonicalizesNativeAliasesAndTheirRelocations)
  {
    MachineModule Module;
    const auto FirstFunction = emissionName("emit.alias.function.first");
    const auto SecondFunction = emissionName("emit.alias.function.second");
    const auto HostFunction = emissionName("host.emit.alias.function");
    const auto FirstData = emissionName("emit.alias.data.first");
    const auto SecondData = emissionName("emit.alias.data.second");
    const auto HostData = emissionName("host.emit.alias.data");
    Module.declareNativeFunction(FirstFunction, {NativeSymbolProvider::Native, HostFunction, NativeCallKind::InkNative});
    Module.declareNativeFunction(SecondFunction, {NativeSymbolProvider::Native, HostFunction, NativeCallKind::InkNative});
    Module.declareNativeData(FirstData, {HostData});
    Module.declareNativeData(SecondData, {HostData});
    auto &Entry = Module.createFunction();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Entry.createBasicBlock());
    Builder.create<Opcode::CallNativei>(ImmediateOperand(FirstFunction));
    Builder.create<Opcode::CallNativei>(ImmediateOperand(SecondFunction));
    Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP, 16), AddressOperand(FirstData, 4));
    Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, 24), ImmediateOperand(SecondData, -8));
    Builder.create<Opcode::Ret>();
    const auto Executable = emitModule(Module, Entry);
    ASSERT_TRUE(Executable);
    ASSERT_EQ(Executable->getNativeFunctions().size(), 1U);
    ASSERT_EQ(Executable->getNativeData().size(), 1U);
    EXPECT_EQ(Executable->getNativeFunctions()[0].Name, HostFunction);
    EXPECT_EQ(Executable->getNativeData()[0].Name, HostData);
    EXPECT_EQ(decodeAt(*Executable, 0).Instruction.Operands[0].Bits, 0U);
    EXPECT_EQ(decodeAt(*Executable, 6).Instruction.Operands[0].Bits, 0U);
    ASSERT_EQ(Executable->getNativeDataRelocations().size(), 2U);
    EXPECT_EQ(Executable->getNativeDataRelocations()[0].ImageOffset, 24U);
    EXPECT_EQ(Executable->getNativeDataRelocations()[0].SymbolIndex, 0U);
    EXPECT_EQ(Executable->getNativeDataRelocations()[0].Addend, 4);
    EXPECT_EQ(Executable->getNativeDataRelocations()[1].ImageOffset, 43U);
    EXPECT_EQ(Executable->getNativeDataRelocations()[1].SymbolIndex, 0U);
    EXPECT_EQ(Executable->getNativeDataRelocations()[1].Addend, -8);
    EXPECT_EQ(Module.nativeFunctions().size(), 2U);
    EXPECT_EQ(Module.nativeData().size(), 2U);
  }

  // Invalid byte order is an emission option error independent of the validated module's legality.
  TEST(VMExecutableEmitterTest, RejectsInvalidByteOrderOption)
  {
    MachineModule Module;
    auto &Entry = Module.createFunction();
    appendReturn(Entry);
    expectEmissionFailure(Module, Entry, static_cast<ByteOrder>(255));
  }

  // A validated numeric direct call preserves its PC-relative displacement in the emitted bytes.
  TEST(VMExecutableEmitterTest, EmitsNumericDirectCallDisplacements)
  {
    MachineModule Module;
    auto &Entry = Module.createFunction();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Entry.createBasicBlock());
    Builder.create<Opcode::Calli>(ImmediateOperand(12));
    Builder.create<Opcode::Ret>();
    appendReturn(Module.createFunction());
    const auto Executable = emitModule(Module, Entry);
    ASSERT_TRUE(Executable);
    EXPECT_EQ(decodeAt(*Executable, 0).Instruction.Operation, Opcode::Calli);
    EXPECT_EQ(decodeAt(*Executable, 0).Instruction.Operands[0].Bits, 12U);
  }

  // A validated empty intermediate block shares the next instruction's address during layout.
  TEST(VMExecutableEmitterTest, ResolvesEmptyIntermediateBlocks)
  {
    MachineModule Module;
    auto &Entry = Module.createFunction();
    auto &Begin = Entry.createBasicBlock();
    auto &Empty = Entry.createBasicBlock();
    auto &End = Entry.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Begin);
    Builder.create<Opcode::Jmp>(&Empty);
    Builder.setInsertPoint(End);
    Builder.create<Opcode::Ret>();
    const auto Executable = emitModule(Module, Entry);
    ASSERT_TRUE(Executable);
    EXPECT_EQ(signedBits(decodeAt(*Executable, 0).Instruction.Operands[0]), 11);
    EXPECT_EQ(decodeAt(*Executable, 11).Instruction.Operation, Opcode::Ret);
  }

  // VM address values cannot underflow or silently truncate to a narrow immediate field.
  TEST(VMExecutableEmitterTest, RejectsUnrepresentableSymbolAddresses)
  {
    const auto EntryName = emissionName("emit.invalid.address.entry");
    {
      MachineModule Module;
      auto &Entry = Module.createFunction(EntryName);
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Entry.createBasicBlock());
      Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, 16), ImmediateOperand(EntryName, -1));
      Builder.create<Opcode::Ret>();
      expectEmissionFailure(Module, Entry);
    }
    {
      MachineModule Module;
      const auto DataName = emissionName("emit.invalid.address.narrow");
      auto &Entry = Module.createFunction();
      ASSERT_TRUE(Module.createGlobalVariable(257));
      ASSERT_TRUE(Module.createGlobalVariable(1, 1, {}, DataName));
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Entry.createBasicBlock());
      Builder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP, 16), ImmediateOperand(DataName));
      Builder.create<Opcode::Ret>();
      expectEmissionFailure(Module, Entry);
    }
  }

  // INT64_MIN symbol offsets reject underflow without negating the minimum signed integer in address or immediate paths.
  TEST(VMExecutableEmitterTest, RejectsMostNegativeSymbolOffsetWithoutSignedOverflow)
  {
    const auto EntryName = emissionName("emit.invalid.address.minimum");
    for (const bool Address : {false, true})
    {
      MachineModule Module;
      auto &Entry = Module.createFunction(EntryName);
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Entry.createBasicBlock());
      if (Address)
      {
        Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP, 16), AddressOperand(EntryName, std::numeric_limits<std::int64_t>::min()));
      }
      else
      {
        Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, 16), ImmediateOperand(EntryName, std::numeric_limits<std::int64_t>::min()));
      }
      Builder.create<Opcode::Ret>();
      expectEmissionFailure(Module, Entry);
    }
  }

  // Native function address fields still require an adapter pointer encoding that the emitter does not yet provide.
  TEST(VMExecutableEmitterTest, RejectsUnsupportedNativeFunctionAddresses)
  {
    const auto Name = emissionName("emit.unsupported.native.address");
    for (const bool Address : {false, true})
    {
      MachineModule Module;
      Module.declareNativeFunction(Name, {NativeSymbolProvider::Native, emissionName("host.emit.address.function"), NativeCallKind::InkNative});
      auto &Entry = Module.createFunction();
      MachineInstructionBuilder Builder;
      Builder.setInsertPoint(Entry.createBasicBlock());
      if (Address)
      {
        Builder.create<Opcode::Load64>(AddressOperand(RegisterId::BP, 16), AddressOperand(Name));
      }
      else
      {
        Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP, 16), ImmediateOperand(Name));
      }
      Builder.create<Opcode::Ret>();
      expectEmissionFailure(Module, Entry);
    }
  }

  // Oversized global storage fails during checked layout before any host image allocation is attempted.
  TEST(VMExecutableEmitterTest, RejectsImageSizeOverflowBeforeAllocation)
  {
    MachineModule Module;
    auto &Entry = Module.createFunction();
    appendReturn(Entry);
    ASSERT_TRUE(Module.createGlobalVariable(std::numeric_limits<std::uint64_t>::max()));
    expectEmissionFailure(Module, Entry);
    EXPECT_EQ(Module.globalVariables().begin()->byteSize(), std::numeric_limits<std::uint64_t>::max());
  }

} // namespace ink::vm::test
