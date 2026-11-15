#include "../../../core/support/environment_test_support.h"

#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/linker/module_linker.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <string_view>
#include <utility>

namespace ink::vm::test
{
    namespace
    {
        std::uint64_t addressOf(const void* Pointer) noexcept
        {
            return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Pointer));
        }

        std::size_t appendCallInstruction(ExecutableFile& Executable, Opcode Operation, std::initializer_list<std::uint64_t> Operands = {}, RegisterId First = RegisterId::ZERO, RegisterId Second = RegisterId::ZERO)
        {
            const auto Offset = Executable.byteSize();
            auto Header = InstructionEncoding::encodeOpcode(Operation);
            Header = InstructionEncoding::encodeRegister(Header, 0, First);
            Header = InstructionEncoding::encodeRegister(Header, 1, Second);
            auto& Image = Executable.getImage();
            Image.resize(Image.size() + sizeof(Header) + Operands.size() * sizeof(std::uint64_t));
            std::memcpy(Image.data() + Offset, &Header, sizeof(Header));
            auto OperandOffset = Offset + sizeof(Header);
            for (const auto Value : Operands)
            {
                std::memcpy(Image.data() + OperandOffset, &Value, sizeof(Value));
                OperandOffset += sizeof(Value);
            }
            return Offset;
        }

        void appendRegisterOffset(ExecutableFile& Executable, RegisterId Register, std::int64_t Offset)
        {
            appendCallInstruction(Executable, Opcode::Add64rri, {0, 0, static_cast<std::uint64_t>(Offset)}, Register, Register);
        }

        std::uint64_t readWord(const void* Address)
        {
            std::uint64_t Value;
            std::memcpy(&Value, Address, sizeof(Value));
            return Value;
        }

        void expectCallFailure(const core::CollectingDiagnosticConsumer& Collector)
        {
            ASSERT_EQ(Collector.diagnostics().size(), 1U);
            EXPECT_EQ(Collector.diagnostics().front().Kind, core::DiagnosticKind::VMExecutionFailed);
        }
    } // namespace

    // Nested Ink calls save each caller's BP and next PC, and RET discards callee locals before restoring its caller.
    TEST(VMInterpreterCallTest, NestedInkCallsRestoreFramesAndContinueAfterEachCall)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("64"));
        Interpreter VM;
        ExecutableFile Executable;
        Executable.getImage().resize(8);
        const auto Main = appendCallInstruction(Executable, Opcode::Calli, {0});
        const auto MainReturn = appendCallInstruction(Executable, Opcode::Ret);
        const auto Middle = appendCallInstruction(Executable, Opcode::Calli, {0});
        const auto MiddleReturn = appendCallInstruction(Executable, Opcode::Ret);
        const auto Leaf = Executable.byteSize();
        appendRegisterOffset(Executable, RegisterId::SP, 24);
        const auto Store = Executable.byteSize();
        appendCallInstruction(Executable, Opcode::Store64i, {std::uint64_t{0} - Store, 0x123456789ABCDEF0ULL}, RegisterId::PC);
        appendCallInstruction(Executable, Opcode::Ret);
        Executable.getImageBaseAddressRelocations() = {
            {Main + 8, Middle, 0},
            {Middle + 8, Leaf, 0},
        };
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + Main));

        const auto Stack = READ_REGISTER(RegisterId::SP, VM.getFrame());
        const auto* StackBytes = reinterpret_cast<const std::uint8_t*>(static_cast<std::uintptr_t>(Stack));
        EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), Stack);
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(VM.getCodeSegment().data() + MainReturn));
        EXPECT_EQ(readWord(VM.getCodeSegment().data()), 0x123456789ABCDEF0ULL);
        EXPECT_EQ(readWord(StackBytes), Stack);
        EXPECT_EQ(readWord(StackBytes + 8), addressOf(VM.getCodeSegment().data() + MainReturn));
        EXPECT_EQ(readWord(StackBytes + 16), Stack);
        EXPECT_EQ(readWord(StackBytes + 24), addressOf(VM.getCodeSegment().data() + MiddleReturn));
    }

    // A direct native call passes the existing Frame to identity without reserving an Ink save area, then resumes at its next instruction.
    TEST(VMInterpreterCallTest, CallsIdentityStubUsingCallerPreparedArgumentAndReturnSlots)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("16"));
        Interpreter VM;
        ExecutableFile Executable;
        Executable.getImage().resize(8);
        const auto Main = Executable.byteSize();
        appendRegisterOffset(Executable, RegisterId::SP, 16);
        appendCallInstruction(Executable, Opcode::Store64i, {static_cast<std::uint64_t>(-8), static_cast<std::uint64_t>(-1234567)}, RegisterId::SP);
        const auto Call = appendCallInstruction(Executable, Opcode::Calli, {0});
        const auto Copy = Executable.byteSize();
        appendCallInstruction(Executable, Opcode::Load64, {std::uint64_t{0} - Copy, static_cast<std::uint64_t>(-16)}, RegisterId::PC, RegisterId::SP);
        appendRegisterOffset(Executable, RegisterId::SP, -16);
        const auto Return = appendCallInstruction(Executable, Opcode::Ret);
        Executable.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("identity")});
        Executable.getDynamicImportSymbolRelocations().push_back({Call + 8, 0, 0});
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + Main));

        EXPECT_EQ(readWord(VM.getCodeSegment().data()), static_cast<std::uint64_t>(-1234567));
        EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), READ_REGISTER(RegisterId::SP, VM.getFrame()));
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(VM.getCodeSegment().data() + Return));
        const auto* Stack = reinterpret_cast<const std::uint8_t*>(static_cast<std::uintptr_t>(READ_REGISTER(RegisterId::SP, VM.getFrame())));
        EXPECT_EQ(readWord(Stack), static_cast<std::uint64_t>(-1234567));
        EXPECT_EQ(readWord(Stack + 8), static_cast<std::uint64_t>(-1234567));
    }

    // An Ink callee reads its caller's BP-relative argument, invokes a native stub with its own call area, and returns through the caller's result slot.
    TEST(VMInterpreterCallTest, InkCalleePassesArgumentToNativeStubAndReturnsItsResult)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("48"));
        Interpreter VM;
        ExecutableFile Executable;
        Executable.getImage().resize(8);
        const auto Main = Executable.byteSize();
        appendRegisterOffset(Executable, RegisterId::SP, 16);
        appendCallInstruction(Executable, Opcode::Store64i, {static_cast<std::uint64_t>(-8), 789123}, RegisterId::SP);
        const auto InkCall = appendCallInstruction(Executable, Opcode::Calli, {0});
        const auto CopyResult = Executable.byteSize();
        appendCallInstruction(Executable, Opcode::Load64, {std::uint64_t{0} - CopyResult, static_cast<std::uint64_t>(-16)}, RegisterId::PC, RegisterId::SP);
        appendRegisterOffset(Executable, RegisterId::SP, -16);
        const auto MainReturn = appendCallInstruction(Executable, Opcode::Ret);
        const auto Callee = Executable.byteSize();
        appendRegisterOffset(Executable, RegisterId::SP, 16);
        appendCallInstruction(Executable, Opcode::Load64, {static_cast<std::uint64_t>(-8), static_cast<std::uint64_t>(-8)}, RegisterId::SP, RegisterId::BP);
        const auto NativeCall = appendCallInstruction(Executable, Opcode::Calli, {0});
        appendCallInstruction(Executable, Opcode::Load64, {static_cast<std::uint64_t>(-16), static_cast<std::uint64_t>(-16)}, RegisterId::BP, RegisterId::SP);
        appendCallInstruction(Executable, Opcode::Ret);
        Executable.getImageBaseAddressRelocations().push_back({InkCall + 8, Callee, 0});
        Executable.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("identity")});
        Executable.getDynamicImportSymbolRelocations().push_back({NativeCall + 8, 0, 0});
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + Main));

        EXPECT_EQ(readWord(VM.getCodeSegment().data()), 789123U);
        const auto Stack = READ_REGISTER(RegisterId::SP, VM.getFrame());
        const auto* StackBytes = reinterpret_cast<const std::uint8_t*>(static_cast<std::uintptr_t>(Stack));
        EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), Stack);
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(VM.getCodeSegment().data() + MainReturn));
        EXPECT_EQ(readWord(StackBytes), 789123U);
        EXPECT_EQ(readWord(StackBytes + 8), 789123U);
        EXPECT_EQ(readWord(StackBytes + 16), Stack);
        EXPECT_EQ(readWord(StackBytes + 24), addressOf(VM.getCodeSegment().data() + CopyResult));
        EXPECT_EQ(readWord(StackBytes + 32), 789123U);
        EXPECT_EQ(readWord(StackBytes + 40), 789123U);
    }

    // Assembly calls pass through parsing, local-symbol linking, address emission and loading before an Ink helper forwards its argument to identity.
    TEST(VMInterpreterCallTest, ExecutesLinkedAssemblyWithInkAndNativeCallsAndStackResults)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("48"));
        constexpr std::string_view Text = R"(.module "call-pipeline"
.declare function @helper
.declare function @identity
.section .data
.global result, 8, 8 external
    .byte 0, 0, 0, 0, 0, 0, 0, 0
.endglobal
.section .text
.function main external
entry:
    add64rri sp, sp, 16
    store64i [sp - 8], -987654321
    call @helper
    load64 [@result], [sp - 16]
    add64rri sp, sp, -16
    ret
.endfunction
.function helper
entry:
    add64rri sp, sp, 16
    load64 [sp - 8], [bp - 8]
    call @identity
    load64 [bp - 16], [sp - 16]
    ret
.endfunction
)";
        auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        Interpreter VM;
        core::CollectingDiagnosticConsumer Collector;
        VM.getDiagnostics().addConsumer(Collector);
        const std::array<const MachineModule*, 1> Inputs{Parsed->get()};
        const auto Linked = ModuleLinker::link(Inputs, VM.getDiagnostics());
        ASSERT_NE(Linked, nullptr);
        ASSERT_EQ(Linked->symbolDeclarations().size(), 1U);
        const auto Identity = core::NamePool::instance().intern("identity");
        EXPECT_EQ(Linked->symbolDeclarations().front().Name, Identity);
        const auto Main = core::NamePool::instance().intern("main");
        const auto Result = core::NamePool::instance().intern("result");
        const std::array Exports{Main, Result};
        ExecutableEmitter Emitter(VM.getDiagnostics());
        auto Executable = Emitter.emit(*Linked, Main, Exports);
        ASSERT_EQ(Executable.getDynamicImportSymbols().size(), 1U);
        EXPECT_EQ(Executable.getDynamicImportSymbols().front().Name, Identity);
        EXPECT_EQ(Executable.getDynamicImportSymbolRelocations().size(), 1U);
        EXPECT_EQ(Executable.getImageBaseAddressRelocations().size(), 2U);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.resolveSymbolAddress(Main)));

        EXPECT_EQ(readWord(VM.resolveSymbolAddress(Result)), static_cast<std::uint64_t>(-987654321));
        EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), READ_REGISTER(RegisterId::SP, VM.getFrame()));
        EXPECT_TRUE(Collector.diagnostics().empty());
        VM.getDiagnostics().removeConsumer(Collector);
    }

    // Cross-library absolute call targets retain their selected Ink function when loading another image moves code storage.
    TEST(VMInterpreterCallTest, CallsImportedInkFunctionAfterCodeSegmentRelocation)
    {
        core::test::ScopedEnvironmentVariable CodeEnvironment("INK_VM_INITIAL_CODE_SEGMENT_BYTES");
        ASSERT_TRUE(CodeEnvironment.set("64"));
        Interpreter VM;
        const auto TargetName = core::NamePool::instance().intern("call-test-cross-library-target");
        const auto CallerName = core::NamePool::instance().intern("call-test-cross-library-caller");
        ExecutableFile Target;
        Target.getImage().resize(8);
        const auto TargetEntry = appendCallInstruction(Target, Opcode::Store64i, {static_cast<std::uint64_t>(-8), 99}, RegisterId::PC);
        appendCallInstruction(Target, Opcode::Ret);
        Target.getExportSymbols().push_back({TargetName, TargetEntry});
        ASSERT_NE(VM.loadLibrary(std::move(Target)), Interpreter::InvalidLibraryHandle);
        const auto CallerBase = VM.getCodeSegment().size();
        ExecutableFile Caller;
        appendCallInstruction(Caller, Opcode::Calli, {0});
        appendCallInstruction(Caller, Opcode::Ret);
        Caller.getDynamicImportSymbols().push_back({TargetName});
        Caller.getDynamicImportSymbolRelocations().push_back({8, 0, 0});
        Caller.getExportSymbols().push_back({CallerName, 0});
        ASSERT_NE(VM.loadLibrary(std::move(Caller)), Interpreter::InvalidLibraryHandle);
        const auto OldAddress = addressOf(VM.getCodeSegment().data());
        ExecutableFile Growth;
        Growth.getImage().resize(VM.getCodeSegmentCapacity() + 1);
        ASSERT_NE(VM.loadLibrary(std::move(Growth)), Interpreter::InvalidLibraryHandle);
        ASSERT_NE(addressOf(VM.getCodeSegment().data()), OldAddress);
        EXPECT_EQ(readWord(VM.getCodeSegment().data() + CallerBase + 8), addressOf(VM.resolveSymbolAddress(TargetName)));

        ASSERT_TRUE(VM.execute(VM.resolveSymbolAddress(CallerName)));

        EXPECT_EQ(readWord(VM.getCodeSegment().data()), 99U);
        EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), READ_REGISTER(RegisterId::SP, VM.getFrame()));
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(VM.getCodeSegment().data() + CallerBase + 16));
    }

    // A VM export named identity replaces name resolution while dispatch still classifies the resolved address rather than its name.
    TEST(VMInterpreterCallTest, ClassifiesOverridingInkExportByItsResolvedAddress)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("16"));
        Interpreter VM;
        const auto Identity = core::NamePool::instance().intern("identity");
        ExecutableFile Target;
        appendCallInstruction(Target, Opcode::Ret);
        Target.getExportSymbols().push_back({Identity, 0});
        ASSERT_NE(VM.loadLibrary(std::move(Target)), Interpreter::InvalidLibraryHandle);
        const auto CallerBase = VM.getCodeSegment().size();
        ExecutableFile Caller;
        appendCallInstruction(Caller, Opcode::Calli, {0});
        appendCallInstruction(Caller, Opcode::Ret);
        Caller.getDynamicImportSymbols().push_back({Identity});
        Caller.getDynamicImportSymbolRelocations().push_back({8, 0, 0});
        ASSERT_NE(VM.loadLibrary(std::move(Caller)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data() + CallerBase));

        const auto Stack = READ_REGISTER(RegisterId::SP, VM.getFrame());
        EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), Stack);
        EXPECT_EQ(readWord(reinterpret_cast<const void*>(static_cast<std::uintptr_t>(Stack + 8))), addressOf(VM.getCodeSegment().data() + CallerBase + 16));
    }

    // Invalid direct-call targets and a missing return instruction fail before changing registers or writing an Ink save area.
    TEST(VMInterpreterCallTest, RejectsInvalidCallTargetsWithoutCommittingFrameOrStack)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("32"));
        for (std::size_t InvalidKind = 0; InvalidKind < 5; ++InvalidKind)
        {
            SCOPED_TRACE(InvalidKind);
            Interpreter VM;
            ExecutableFile Executable;
            appendCallInstruction(Executable, Opcode::Ret);
            const auto Call = appendCallInstruction(Executable, Opcode::Calli, {InvalidKind == 1 ? std::numeric_limits<std::uint64_t>::max() : 0});
            if (InvalidKind != 4)
            {
                appendCallInstruction(Executable, Opcode::Ret);
            }
            if (InvalidKind >= 2)
            {
                const auto TargetOffset = InvalidKind == 3 ? Executable.byteSize() : 0;
                Executable.getImageBaseAddressRelocations().push_back({Call + 8, InvalidKind == 2 ? 1 : TargetOffset, 0});
            }
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));
            const auto Stack = READ_REGISTER(RegisterId::SP, VM.getFrame());
            auto* StackBytes = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(Stack));
            std::fill_n(StackBytes, 32, 0xA5);
            core::CollectingDiagnosticConsumer Collector;
            VM.getDiagnostics().addConsumer(Collector);

            EXPECT_FALSE(VM.execute(VM.getCodeSegment().data() + Call));

            EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), Stack);
            EXPECT_EQ(READ_REGISTER(RegisterId::SP, VM.getFrame()), Stack);
            EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(VM.getCodeSegment().data() + Call));
            EXPECT_TRUE(std::all_of(StackBytes, StackBytes + 32, [](std::uint8_t Byte)
                                    {
                                        return Byte == 0xA5;
                                    }));
            expectCallFailure(Collector);
            VM.getDiagnostics().removeConsumer(Collector);
        }
    }

    // CALL rejects out-of-stack or unaligned BP/SP and an insufficient Ink save area without partially writing that area.
    TEST(VMInterpreterCallTest, ChecksCallStackBoundsBeforeCommittingFrame)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("32"));
        struct InvalidFrame
        {
                std::int64_t SP;
                std::int64_t BP;
                bool Native;
        };
        constexpr InvalidFrame Cases[] = {
            {1, 0, false},
            {-8, 0, false},
            {40, 0, false},
            {24, 0, false},
            {32, 0, false},
            {0, 1, false},
            {0, 40, false},
            {0, 8, false},
            {1, 0, true},
            {-8, 0, true},
            {40, 0, true},
            {0, 1, true},
            {0, 40, true},
            {0, 8, true},
        };
        for (const auto& Invalid : Cases)
        {
            SCOPED_TRACE(Invalid.SP);
            SCOPED_TRACE(Invalid.BP);
            SCOPED_TRACE(Invalid.Native);
            Interpreter VM;
            ExecutableFile Executable;
            appendCallInstruction(Executable, Opcode::Ret);
            const auto Entry = Executable.byteSize();
            appendRegisterOffset(Executable, RegisterId::SP, Invalid.SP);
            appendRegisterOffset(Executable, RegisterId::BP, Invalid.BP);
            const auto Call = appendCallInstruction(Executable, Opcode::Calli, {0});
            appendCallInstruction(Executable, Opcode::Ret);
            if (Invalid.Native)
            {
                Executable.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("identity")});
                Executable.getDynamicImportSymbolRelocations().push_back({Call + 8, 0, 0});
            }
            else
            {
                Executable.getImageBaseAddressRelocations().push_back({Call + 8, 0, 0});
            }
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));
            const auto Stack = READ_REGISTER(RegisterId::SP, VM.getFrame());
            auto* StackBytes = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(Stack));
            std::fill_n(StackBytes, 32, 0x6D);
            core::CollectingDiagnosticConsumer Collector;
            VM.getDiagnostics().addConsumer(Collector);

            EXPECT_FALSE(VM.execute(VM.getCodeSegment().data() + Entry));

            EXPECT_EQ(READ_REGISTER(RegisterId::SP, VM.getFrame()), Stack + static_cast<std::uint64_t>(Invalid.SP));
            EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), Stack + static_cast<std::uint64_t>(Invalid.BP));
            EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(VM.getCodeSegment().data() + Call));
            EXPECT_TRUE(std::all_of(StackBytes, StackBytes + 32, [](std::uint8_t Byte)
                                    {
                                        return Byte == 0x6D;
                                    }));
            expectCallFailure(Collector);
            VM.getDiagnostics().removeConsumer(Collector);
        }
    }

    // RET validates its active save area and both saved registers before restoring any caller register.
    TEST(VMInterpreterCallTest, RejectsCorruptReturnStateWithoutPartiallyRestoringCaller)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("32"));
        for (std::size_t InvalidKind = 0; InvalidKind < 5; ++InvalidKind)
        {
            SCOPED_TRACE(InvalidKind);
            Interpreter VM;
            ExecutableFile Executable;
            appendCallInstruction(Executable, Opcode::Ret);
            const auto Main = appendCallInstruction(Executable, Opcode::Calli, {0});
            appendCallInstruction(Executable, Opcode::Ret);
            const auto Callee = Executable.byteSize();
            if (InvalidKind < 2)
            {
                appendCallInstruction(Executable, Opcode::Store64i, {InvalidKind * 8, std::numeric_limits<std::uint64_t>::max()}, RegisterId::BP);
            }
            else if (InvalidKind == 2)
            {
                appendRegisterOffset(Executable, RegisterId::BP, 1);
            }
            else
            {
                appendRegisterOffset(Executable, RegisterId::SP, InvalidKind == 3 ? -8 : 24);
            }
            const auto Return = appendCallInstruction(Executable, Opcode::Ret);
            Executable.getImageBaseAddressRelocations().push_back({Main + 8, Callee, 0});
            ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
            ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));
            const auto Stack = READ_REGISTER(RegisterId::SP, VM.getFrame());
            core::CollectingDiagnosticConsumer Collector;
            VM.getDiagnostics().addConsumer(Collector);

            EXPECT_FALSE(VM.execute(VM.getCodeSegment().data() + Main));

            EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), Stack + (InvalidKind == 2 ? 1 : 0));
            const auto ExpectedStackOffset = InvalidKind == 4 ? 40U : 16U;
            const auto ExpectedStackPointer = InvalidKind == 3 ? Stack + 8 : Stack + ExpectedStackOffset;
            EXPECT_EQ(READ_REGISTER(RegisterId::SP, VM.getFrame()), ExpectedStackPointer);
            EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(VM.getCodeSegment().data() + Return));
            expectCallFailure(Collector);
            VM.getDiagnostics().removeConsumer(Collector);
        }
    }

    // The outermost RET remains a successful completion even when no VM stack allocation is configured.
    TEST(VMInterpreterCallTest, OutermostReturnDoesNotReadAnInkSaveArea)
    {
        core::test::ScopedEnvironmentVariable StackEnvironment("INK_VM_STACK_BYTES");
        ASSERT_TRUE(StackEnvironment.set("0"));
        Interpreter VM;
        ExecutableFile Executable;
        appendCallInstruction(Executable, Opcode::Ret);
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

        ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

        EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), 0U);
        EXPECT_EQ(READ_REGISTER(RegisterId::SP, VM.getFrame()), 0U);
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(VM.getCodeSegment().data()));
    }
} // namespace ink::vm::test
