#include "ink/backend/vm/executable_emitter/executable_emission_context.h"
#include "ink/backend/vm/executable_emitter/function_emitter.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
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
        std::uint64_t readLittleEndian(const ExecutableEmissionContext& Context, std::size_t Offset, std::size_t Count = 8)
        {
            std::uint64_t Bits = 0;
            for (std::size_t Index = 0; Index < Count; ++Index)
            {
                Bits |= static_cast<std::uint64_t>(Context.getExecutable().getImage()[Offset + Index]) << (Index * 8);
            }
            return Bits;
        }

        template <Opcode Operation>
        void verifyComparisonBranchEncoding(std::uint16_t Encoding)
        {
            SCOPED_TRACE(Encoding);
            constexpr auto Width = OpcodeTraits<Operation>::BitWidth;
            constexpr bool MemoryRHS = OpcodeTraits<Operation>::Form == InstructionForm::Mmb;
            constexpr auto InstructionBytes = InstructionEncoding::byteSize(Operation);
            constexpr std::uint64_t ImmediateBits = 0xFFEEDDCCBBAA9988;
            const core::DiagnosticEngine Diagnostics;
            ExecutableEmissionContext Context(Diagnostics);
            Context.padZero(1);
            MachineFunction Function;
            auto& Source = Function.createBasicBlock();
            auto& Target = Function.createBasicBlock();
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Source);
            if constexpr (MemoryRHS)
            {
                Builder.create<Operation>(AddressOperand(RegisterId::BP, -3), AddressOperand(RegisterId::SP, 5), BasicBlockOperand(&Target));
            }
            else
            {
                Builder.create<Operation>(AddressOperand(RegisterId::BP, -3), ImmediateOperand(ImmediateBits), BasicBlockOperand(&Target));
            }
            Builder.setInsertPoint(Target);
            Builder.create<Opcode::Ret>();
            FunctionEmitter Emitter(Context);
            Emitter.emit(Function);

            ASSERT_EQ(Context.byteSize(), InstructionBytes + 16);
            EXPECT_EQ(readLittleEndian(Context, 8, 2), Encoding);
            EXPECT_EQ(Context.getExecutable().getImage()[10], static_cast<std::uint8_t>(RegisterId::BP));
            EXPECT_EQ(readLittleEndian(Context, 16), static_cast<std::uint64_t>(-3));
            if constexpr (MemoryRHS)
            {
                EXPECT_EQ(Context.getExecutable().getImage()[11], static_cast<std::uint8_t>(RegisterId::SP));
                EXPECT_EQ(readLittleEndian(Context, 24), 5U);
            }
            else
            {
                constexpr auto Mask = ~std::uint64_t{0} >> (64 - Width);
                EXPECT_EQ(readLittleEndian(Context, 24), ImmediateBits & Mask);
            }
            constexpr auto TargetField = 8 + InstructionBytes - 8;
            EXPECT_EQ(TargetField % 8, 0U);
            EXPECT_EQ(Context.getExecutable().getImage()[12], static_cast<std::uint8_t>(RegisterId::PC));
            EXPECT_EQ(readLittleEndian(Context, TargetField), InstructionBytes);
            EXPECT_EQ(readLittleEndian(Context, 8 + InstructionBytes, 2), 0x003AU);
            EXPECT_TRUE(Context.getExecutable().getImageBaseAddressRelocations().empty());
            EXPECT_TRUE(Context.getExecutable().getDynamicImportSymbolRelocations().empty());
        }
    } // namespace

    static_assert(std::is_same_v<decltype(std::declval<FunctionEmitter&>().getContext()), FunctionEmissionContext&>);
    static_assert(std::is_same_v<decltype(std::declval<const FunctionEmitter&>().getContext()), const FunctionEmissionContext&>);

    // All eighty comparison opcodes emit aligned payload words, truncate the typed RHS, and resolve the final target relative to the header.
    TEST(VMFunctionEmitterTest, EncodesEveryComparisonBranchFormAndWidth)
    {
#define COMPARE_BRANCH(Kind, Encoding, Spelling, Form, Width, Operator, Signed) verifyComparisonBranchEncoding<Opcode::Kind>(Encoding);
#include "ink/backend/vm/instruction/compare_branch.def"
#undef COMPARE_BRANCH
    }

    // Explicit block emission aligns its first instruction and resolves each block at its header in the complete image.
    TEST(VMFunctionEmitterTest, AppendsOrderedInstructionsAndResolvesImageOffsets)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmissionContext Context(Diagnostics);
        Context.padZero(3);
        MachineFunction Function;
        MachineBasicBlock& First = Function.createBasicBlock();
        MachineBasicBlock& Second = Function.createBasicBlock();
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

        ASSERT_EQ(Context.byteSize(), 40U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(First), 8U);
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(First));
        EXPECT_FALSE(Emitter.getContext().resolvedBasicBlockOffset(Second).has_value());
        EXPECT_TRUE(Emitter.getContext().hasUnResolvedBasicBlock(Second));
        Emitter.padBasicBlock(Second);

        ASSERT_EQ(Context.byteSize(), 72U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Second), 40U);
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(Second));
        EXPECT_EQ(readLittleEndian(Context, 8, 2), 0x0006U);
        EXPECT_EQ(Context.getExecutable().getImage()[10], static_cast<std::uint8_t>(RegisterId::BP));
        EXPECT_EQ(readLittleEndian(Context, 16), 8U);
        EXPECT_EQ(readLittleEndian(Context, 24), 0xABU);
        EXPECT_EQ(readLittleEndian(Context, 32, 2), 0x003AU);
        EXPECT_EQ(readLittleEndian(Context, 40, 2), 0x0008U);
        EXPECT_EQ(Context.getExecutable().getImage()[42], static_cast<std::uint8_t>(RegisterId::SP));
        EXPECT_EQ(readLittleEndian(Context, 48), static_cast<std::uint64_t>(-8));
        EXPECT_EQ(readLittleEndian(Context, 56), 0xBEEFU);
        EXPECT_EQ(readLittleEndian(Context, 64, 2), 0x003AU);
    }

    // Empty, anonymous, and equally named blocks keep separate identities while empty blocks share the following byte offset.
    TEST(VMFunctionEmitterTest, PreservesEmptyAnonymousAndDuplicateNamedBlockIdentities)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmissionContext Context(Diagnostics);
        Context.padZero(5);
        const auto BlockName = core::NamePool::instance().intern("function-emitter-duplicate-block");
        MachineFunction Function;
        MachineBasicBlock& EmptyAnonymous = Function.createBasicBlock();
        MachineBasicBlock& FirstNamed = Function.createBasicBlock(BlockName);
        MachineBasicBlock& EmptyNamed = Function.createBasicBlock(BlockName);
        MachineBasicBlock& LastAnonymous = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(FirstNamed);
        Builder.create<Opcode::Ret>();
        Builder.setInsertPoint(LastAnonymous);
        Builder.create<Opcode::Ret>();
        FunctionEmitter Emitter(Context);
        Emitter.emit(Function);

        ASSERT_EQ(Context.byteSize(), 24U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(EmptyAnonymous), 8U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(FirstNamed), 8U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(EmptyNamed), 16U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(LastAnonymous), 16U);
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(EmptyAnonymous));
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(FirstNamed));
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(EmptyNamed));
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(LastAnonymous));
        EXPECT_EQ(readLittleEndian(Context, 8, 2), 0x003AU);
        EXPECT_EQ(readLittleEndian(Context, 16, 2), 0x003AU);
    }

    // Beginning another function discards old block offsets and pending branch relocations while preserving previously emitted bytes.
    TEST(VMFunctionEmitterTest, ClearsPreviousFunctionStateWhileAppendingNextFunction)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmissionContext Context(Diagnostics);
        Context.padZero(1);
        MachineFunction FirstFunction;
        MachineBasicBlock& Resolved = FirstFunction.createBasicBlock();
        MachineBasicBlock& Unresolved = FirstFunction.createBasicBlock();
        MachineFunction SecondFunction;
        MachineBasicBlock& Next = SecondFunction.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Resolved);
        Builder.create<Opcode::Jmp>(&Unresolved);
        Builder.setInsertPoint(Next);
        Builder.create<Opcode::Ret>();
        FunctionEmitter Emitter(Context);
        Emitter.scanUnResolvedBasicBlock(FirstFunction);
        Emitter.padBasicBlock(Resolved);
        ASSERT_EQ(Context.byteSize(), 24U);
        ASSERT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Resolved), 8U);
        ASSERT_TRUE(Emitter.getContext().hasUnResolvedBasicBlock(Unresolved));
        ASSERT_EQ(readLittleEndian(Context, 16), 0U);

        Emitter.emit(SecondFunction);

        ASSERT_EQ(Context.byteSize(), 32U);
        EXPECT_FALSE(Emitter.getContext().resolvedBasicBlockOffset(Resolved).has_value());
        EXPECT_FALSE(Emitter.getContext().resolvedBasicBlockOffset(Unresolved).has_value());
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(Resolved));
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(Unresolved));
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Next), 24U);
        EXPECT_FALSE(Emitter.getContext().hasUnResolvedBasicBlock(Next));
        EXPECT_EQ(readLittleEndian(Context, 8, 2), 0x002BU);
        EXPECT_EQ(readLittleEndian(Context, 16), 0U);
        EXPECT_EQ(readLittleEndian(Context, 24, 2), 0x003AU);
    }

    // Forward references share a target, backward references reach image offset zero, and a self-targeting branch encodes zero displacement.
    TEST(VMFunctionEmitterTest, ResolvesForwardBackwardAndSelfBranchesAtInstructionOrigins)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmissionContext Context(Diagnostics);
        MachineFunction Function;
        MachineBasicBlock& First = Function.createBasicBlock();
        MachineBasicBlock& Later = Function.createBasicBlock();
        MachineBasicBlock& Self = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(First);
        Builder.create<Opcode::Jmp>(&Later);
        Builder.create<Opcode::Jmp>(&Later);
        Builder.create<Opcode::Jmp>(&First);
        Builder.setInsertPoint(Later);
        Builder.create<Opcode::Jmp>(&First);
        Builder.setInsertPoint(Self);
        Builder.create<Opcode::Jmp>(&Self);
        FunctionEmitter Emitter(Context);
        Emitter.emit(Function);

        ASSERT_EQ(Context.byteSize(), 80U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(First), 0U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Later), 48U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Self), 64U);
        EXPECT_EQ(readLittleEndian(Context, 0, 2), 0x002BU);
        EXPECT_EQ(Context.getExecutable().getImage()[2], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 8), 48U);
        EXPECT_EQ(readLittleEndian(Context, 16, 2), 0x002BU);
        EXPECT_EQ(Context.getExecutable().getImage()[18], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 24), 32U);
        EXPECT_EQ(readLittleEndian(Context, 32, 2), 0x002BU);
        EXPECT_EQ(Context.getExecutable().getImage()[34], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 40), static_cast<std::uint64_t>(-32));
        EXPECT_EQ(readLittleEndian(Context, 48, 2), 0x002BU);
        EXPECT_EQ(Context.getExecutable().getImage()[50], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 56), static_cast<std::uint64_t>(-48));
        EXPECT_EQ(readLittleEndian(Context, 64, 2), 0x002BU);
        EXPECT_EQ(Context.getExecutable().getImage()[66], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 72), 0U);
        EXPECT_TRUE(Context.getExecutable().getImageBaseAddressRelocations().empty());
        EXPECT_TRUE(Context.getExecutable().getDynamicImportSymbolRelocations().empty());
    }

    // Mixed comparison widths resolve forward, backward and self targets from each opcode rather than the final operand's location.
    TEST(VMFunctionEmitterTest, ResolvesComparisonTargetsAtInstructionOrigins)
    {
        const core::DiagnosticEngine Diagnostics;
        ExecutableEmissionContext Context(Diagnostics);
        Context.padZero(3);
        MachineFunction Function;
        auto& First = Function.createBasicBlock();
        auto& Later = Function.createBasicBlock();
        auto& Self = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(First);
        Builder.create<Opcode::Je8mib>(AddressOperand(RegisterId::BP), ImmediateOperand(0), BasicBlockOperand(&Later));
        Builder.create<Opcode::Jne64mmb>(AddressOperand(RegisterId::BP, 1), AddressOperand(RegisterId::SP, -1), BasicBlockOperand(&Later));
        Builder.setInsertPoint(Later);
        Builder.create<Opcode::Jlt16mib>(AddressOperand(RegisterId::BP, 2), ImmediateOperand(0x1122), BasicBlockOperand(&First));
        Builder.setInsertPoint(Self);
        Builder.create<Opcode::Jgeu32mmb>(AddressOperand(RegisterId::BP), AddressOperand(RegisterId::SP), BasicBlockOperand(&Self));
        FunctionEmitter Emitter(Context);
        Emitter.emit(Function);

        ASSERT_EQ(Context.byteSize(), 136U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(First), 8U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Later), 72U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Self), 104U);
        EXPECT_EQ(Context.getExecutable().getImage()[12], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 32), 64U);
        EXPECT_EQ(Context.getExecutable().getImage()[44], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 64), 32U);
        EXPECT_EQ(Context.getExecutable().getImage()[76], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 96), static_cast<std::uint64_t>(-64));
        EXPECT_EQ(Context.getExecutable().getImage()[108], static_cast<std::uint8_t>(RegisterId::PC));
        EXPECT_EQ(readLittleEndian(Context, 128), 0U);
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
        MachineBasicBlock& Source = Function.createBasicBlock();
        MachineBasicBlock& Target = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Source);
        Builder.create<Opcode::Jmp>(&Target);
        Builder.create<Opcode::Jmp>(&Target);
        Builder.setInsertPoint(Target);
        Builder.create<Opcode::Ret>();
        FunctionEmitter Emitter(Context);
        Emitter.scanUnResolvedBasicBlock(Function);
        Emitter.padBasicBlock(Source);

        ASSERT_EQ(Context.byteSize(), 40U);
        EXPECT_EQ(readLittleEndian(Context, 16), 0U);
        EXPECT_EQ(readLittleEndian(Context, 32), 0U);
        Emitter.padBasicBlock(Target);
        ASSERT_EQ(Context.byteSize(), 48U);
        EXPECT_EQ(Emitter.getContext().resolvedBasicBlockOffset(Target), 40U);
        EXPECT_EQ(readLittleEndian(Context, 16), 0U);
        EXPECT_EQ(readLittleEndian(Context, 32), 0U);
        Emitter.resolveBasicBlockStaticRelocations();

        EXPECT_EQ(readLittleEndian(Context, 16), 32U);
        EXPECT_EQ(readLittleEndian(Context, 32), 16U);
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
        MachineBasicBlock& Source = Function.createBasicBlock();
        MachineBasicBlock& Target = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Source);
        Builder.create<Opcode::Jmp>(&Target);
        FunctionEmitter Emitter(Context);
        Emitter.scanUnResolvedBasicBlock(Function);
        Emitter.padBasicBlock(Source);

        ASSERT_EQ(Context.byteSize(), 16U);
        ASSERT_EQ(readLittleEndian(Context, 8), 0U);
        EXPECT_DEATH(Emitter.resolveBasicBlockStaticRelocations(), "INK-V0002.*basic block has no image offset after function layout");
    }
} // namespace ink::vm::test
