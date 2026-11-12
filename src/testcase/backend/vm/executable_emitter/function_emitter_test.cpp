#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/executable_emitter/function_emitter.h"
#include "ink/backend/vm/module/machine_function.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
  namespace
  {
    std::uint64_t readLittleEndian(const ExecutableEmissionContext &Context, std::size_t Offset, std::size_t Count = 8)
    {
      std::uint64_t Bits = 0;
      for (std::size_t Index = 0; Index < Count; ++Index)
      {
        Bits |= static_cast<std::uint64_t>(Context.getExecutable().getImage()[Offset + Index]) << (Index * 8);
      }
      return Bits;
    }
  } // namespace

  static_assert(std::is_same_v<decltype(std::declval<FunctionEmitter &>().getContext()), FunctionEmissionContext &>);
  static_assert(std::is_same_v<decltype(std::declval<const FunctionEmitter &>().getContext()), const FunctionEmissionContext &>);

  // Explicit block emission appends instructions without padding and resolves each block at its first byte in the complete image.
  TEST(VMFunctionEmitterTest, AppendsOrderedInstructionsAndResolvesImageOffsets)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmissionContext Context(Diagnostics);
    Context.padZero(3);
    MachineFunction Function;
    MachineBasicBlock &First = Function.createBasicBlock();
    MachineBasicBlock &Second = Function.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(First);
    Builder.create<Opcode::Store8i>(AddressOperand(RegisterId::BP, 8), ImmediateOperand(0x1AB));
    Builder.create<Opcode::Ret>();
    Builder.setInsertPoint(Second);
    Builder.create<Opcode::Store16i>(AddressOperand(RegisterId::SP, -8), ImmediateOperand(0xBEEF));
    Builder.create<Opcode::Ret>();
    FunctionEmitter Emitter(Context);
    Emitter.scanUnResolvedBasicBlock(Function);
    Emitter.padBasicBlock(First);

    ASSERT_EQ(Context.byteSize(), 17U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(First), 3U);
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(First));
    EXPECT_FALSE(Emitter.getContext().resolvedBasicBlockOffset(Second).has_value());
    EXPECT_TRUE(Emitter.getContext().hasUnResolvedBasicBlock(Second));
    Emitter.padBasicBlock(Second);

    ASSERT_EQ(Context.byteSize(), 32U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Second), 17U);
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(Second));
    EXPECT_EQ(readLittleEndian(Context, 3, 2), 0x0006U);
    EXPECT_EQ(Context.getExecutable().getImage()[5], static_cast<std::uint8_t>(RegisterId::BP));
    EXPECT_EQ(readLittleEndian(Context, 6), 8U);
    EXPECT_EQ(readLittleEndian(Context, 14, 1), 0xABU);
    EXPECT_EQ(readLittleEndian(Context, 15, 2), 0x003AU);
    EXPECT_EQ(readLittleEndian(Context, 17, 2), 0x0008U);
    EXPECT_EQ(Context.getExecutable().getImage()[19], static_cast<std::uint8_t>(RegisterId::SP));
    EXPECT_EQ(readLittleEndian(Context, 20), static_cast<std::uint64_t>(-8));
    EXPECT_EQ(readLittleEndian(Context, 28, 2), 0xBEEFU);
    EXPECT_EQ(readLittleEndian(Context, 30, 2), 0x003AU);
  }

  // Empty, anonymous, and equally named blocks keep separate identities while empty blocks share the following byte offset.
  TEST(VMFunctionEmitterTest, PreservesEmptyAnonymousAndDuplicateNamedBlockIdentities)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmissionContext Context(Diagnostics);
    Context.padZero(5);
    const auto BlockName = core::NamePool::instance().intern("function-emitter-duplicate-block");
    MachineFunction Function;
    MachineBasicBlock &EmptyAnonymous = Function.createBasicBlock();
    MachineBasicBlock &FirstNamed = Function.createBasicBlock(BlockName);
    MachineBasicBlock &EmptyNamed = Function.createBasicBlock(BlockName);
    MachineBasicBlock &LastAnonymous = Function.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(FirstNamed);
    Builder.create<Opcode::Ret>();
    Builder.setInsertPoint(LastAnonymous);
    Builder.create<Opcode::Ret>();
    FunctionEmitter Emitter(Context);
    Emitter.emit(Function);

    ASSERT_EQ(Context.byteSize(), 9U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(EmptyAnonymous), 5U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(FirstNamed), 5U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(EmptyNamed), 7U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(LastAnonymous), 7U);
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(EmptyAnonymous));
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(FirstNamed));
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(EmptyNamed));
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(LastAnonymous));
    EXPECT_EQ(readLittleEndian(Context, 5, 2), 0x003AU);
    EXPECT_EQ(readLittleEndian(Context, 7, 2), 0x003AU);
  }

  // Beginning another function discards old block offsets and pending branch relocations while preserving previously emitted bytes.
  TEST(VMFunctionEmitterTest, ClearsPreviousFunctionStateWhileAppendingNextFunction)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmissionContext Context(Diagnostics);
    Context.padZero(1);
    MachineFunction FirstFunction;
    MachineBasicBlock &Resolved = FirstFunction.createBasicBlock();
    MachineBasicBlock &Unresolved = FirstFunction.createBasicBlock();
    MachineFunction SecondFunction;
    MachineBasicBlock &Next = SecondFunction.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Resolved);
    Builder.create<Opcode::Jmp>(&Unresolved);
    Builder.setInsertPoint(Next);
    Builder.create<Opcode::Ret>();
    FunctionEmitter Emitter(Context);
    Emitter.scanUnResolvedBasicBlock(FirstFunction);
    Emitter.padBasicBlock(Resolved);
    ASSERT_EQ(Context.byteSize(), 12U);
    ASSERT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Resolved), 1U);
    ASSERT_TRUE(Emitter.getContext().hasUnResolvedBasicBlock(Unresolved));
    ASSERT_EQ(readLittleEndian(Context, 4), 0U);

    Emitter.emit(SecondFunction);

    ASSERT_EQ(Context.byteSize(), 14U);
    EXPECT_FALSE(Emitter.getContext().resolvedBasicBlockOffset(Resolved).has_value());
    EXPECT_FALSE(Emitter.getContext().resolvedBasicBlockOffset(Unresolved).has_value());
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(Resolved));
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(Unresolved));
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Next), 12U);
    EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(Next));
    EXPECT_EQ(readLittleEndian(Context, 1, 2), 0x002BU);
    EXPECT_EQ(readLittleEndian(Context, 4), 0U);
    EXPECT_EQ(readLittleEndian(Context, 12, 2), 0x003AU);
  }

  // Forward references share a target, backward references reach image offset zero, and a self-targeting branch encodes zero displacement.
  TEST(VMFunctionEmitterTest, ResolvesForwardBackwardAndSelfBranchesAtInstructionOrigins)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmissionContext Context(Diagnostics);
    MachineFunction Function;
    MachineBasicBlock &First = Function.createBasicBlock();
    MachineBasicBlock &Later = Function.createBasicBlock();
    MachineBasicBlock &Self = Function.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(First);
    Builder.create<Opcode::Je>(&Later);
    Builder.create<Opcode::Jne>(&Later);
    Builder.create<Opcode::Jmp>(&First);
    Builder.setInsertPoint(Later);
    Builder.create<Opcode::Jmp>(&First);
    Builder.setInsertPoint(Self);
    Builder.create<Opcode::Jmp>(&Self);
    FunctionEmitter Emitter(Context);
    Emitter.emit(Function);

    ASSERT_EQ(Context.byteSize(), 55U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(First), 0U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Later), 33U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Self), 44U);
    EXPECT_EQ(readLittleEndian(Context, 0, 2), 0x002CU);
    EXPECT_EQ(Context.getExecutable().getImage()[2], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Context, 3), 33U);
    EXPECT_EQ(readLittleEndian(Context, 11, 2), 0x002DU);
    EXPECT_EQ(Context.getExecutable().getImage()[13], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Context, 14), 22U);
    EXPECT_EQ(readLittleEndian(Context, 22, 2), 0x002BU);
    EXPECT_EQ(Context.getExecutable().getImage()[24], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Context, 25), static_cast<std::uint64_t>(-22));
    EXPECT_EQ(readLittleEndian(Context, 33, 2), 0x002BU);
    EXPECT_EQ(Context.getExecutable().getImage()[35], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Context, 36), static_cast<std::uint64_t>(-33));
    EXPECT_EQ(readLittleEndian(Context, 44, 2), 0x002BU);
    EXPECT_EQ(Context.getExecutable().getImage()[46], static_cast<std::uint8_t>(RegisterId::PC));
    EXPECT_EQ(readLittleEndian(Context, 47), 0U);
    EXPECT_TRUE(Context.getExecutable().getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Context.getExecutable().getDynamicImportSymbolRelocations().empty());
  }

  // Forward branch fields remain zero after their target is laid out and are patched only by explicit, repeatable finalization.
  TEST(VMFunctionEmitterTest, DefersForwardBranchPatchingUntilExplicitFinalization)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmissionContext Context(Diagnostics);
    Context.padZero(5);
    MachineFunction Function;
    MachineBasicBlock &Source = Function.createBasicBlock();
    MachineBasicBlock &Target = Function.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Source);
    Builder.create<Opcode::Je>(&Target);
    Builder.create<Opcode::Jmp>(&Target);
    Builder.setInsertPoint(Target);
    Builder.create<Opcode::Ret>();
    FunctionEmitter Emitter(Context);
    Emitter.scanUnResolvedBasicBlock(Function);
    Emitter.padBasicBlock(Source);

    ASSERT_EQ(Context.byteSize(), 27U);
    EXPECT_EQ(readLittleEndian(Context, 8), 0U);
    EXPECT_EQ(readLittleEndian(Context, 19), 0U);
    Emitter.padBasicBlock(Target);
    ASSERT_EQ(Context.byteSize(), 29U);
    EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Target), 27U);
    EXPECT_EQ(readLittleEndian(Context, 8), 0U);
    EXPECT_EQ(readLittleEndian(Context, 19), 0U);
    Emitter.resolveBasicBlockStaticRelocations();

    EXPECT_EQ(readLittleEndian(Context, 8), 22U);
    EXPECT_EQ(readLittleEndian(Context, 19), 11U);
    EXPECT_TRUE(Context.getExecutable().getImageBaseAddressRelocations().empty());
    EXPECT_TRUE(Context.getExecutable().getDynamicImportSymbolRelocations().empty());
    const auto PatchedImage = Context.getExecutable().getImage();
    Emitter.resolveBasicBlockStaticRelocations();
    EXPECT_EQ(Context.getExecutable().getImage(), PatchedImage);
  }

  // Finalization rejects a pending branch whose target block was scanned but never assigned an image offset.
  TEST(VMFunctionEmitterDeathTest, RejectsUnlaidOutBranchTargetsDuringFinalization)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableEmissionContext Context(Diagnostics);
    MachineFunction Function;
    MachineBasicBlock &Source = Function.createBasicBlock();
    MachineBasicBlock &Target = Function.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Source);
    Builder.create<Opcode::Jmp>(&Target);
    FunctionEmitter Emitter(Context);
    Emitter.scanUnResolvedBasicBlock(Function);
    Emitter.padBasicBlock(Source);

    ASSERT_EQ(Context.byteSize(), 11U);
    ASSERT_EQ(readLittleEndian(Context, 3), 0U);
    EXPECT_DEATH(Emitter.resolveBasicBlockStaticRelocations(), "INK-V0002.*basic block has no image offset after function layout");
  }
} // namespace ink::vm::test
