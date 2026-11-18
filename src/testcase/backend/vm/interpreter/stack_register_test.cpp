#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <limits>
#include <utility>

namespace ink::vm::test
{
    namespace
    {
        std::uint64_t addressOf(const void* Address)
        {
            return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Address));
        }

        std::size_t appendInstruction(ExecutableFile& Executable, Opcode Operation, std::initializer_list<std::uint64_t> Operands = {}, RegisterId First = RegisterId::ZERO, RegisterId Second = RegisterId::ZERO, RegisterId Third = RegisterId::ZERO)
        {
            const auto Offset = Executable.byteSize();
            auto Header = InstructionEncoding::encodeOpcode(Operation);
            Header = InstructionEncoding::encodeRegister(Header, 0, First);
            Header = InstructionEncoding::encodeRegister(Header, 1, Second);
            Header = InstructionEncoding::encodeRegister(Header, 2, Third);
            const auto AppendWord = [&Executable](std::uint64_t Word)
            {
                for (std::size_t Index = 0; Index < sizeof(Word); ++Index)
                {
                    Executable.getImage().push_back(static_cast<std::uint8_t>(Word >> (Index * 8)));
                }
            };
            AppendWord(Header);
            for (const auto Operand : Operands)
            {
                AppendWord(Operand);
            }
            return Offset;
        }
    } // namespace

    // Every implemented memory-write form updates live BP/SP storage before the next register read, including an unaligned partial write.
    TEST(VMStackRegisterTest, ReadsStackRegistersAfterWritesThroughMemoryAliases)
    {
        constexpr RegisterId Registers[] = {
            RegisterId::BP,
            RegisterId::SP,
        };
        constexpr Opcode Operations[] = {
            Opcode::Store32i,
            Opcode::Store64i,
            Opcode::Load64,
            Opcode::Add64mrr,
            Opcode::Add64mmi,
            Opcode::Add64mmm,
        };
        const std::array<std::uint64_t, 2> Sources = {17, 25};
        for (const auto Register : Registers)
        {
            for (const auto Operation : Operations)
            {
                SCOPED_TRACE(static_cast<std::uint8_t>(Register));
                SCOPED_TRACE(static_cast<std::uint16_t>(Operation));
                Interpreter VM;
                const auto Storage = addressOf(GET_REGISTER(Register, VM.getFrame()));
                const auto Bindings = VM.getFrame().Register;
                const std::uint64_t Initial = Operation == Opcode::Store32i ? 0x1122334455667788 : 37;
                std::uint64_t Expected = Initial;
                ExecutableFile Executable;
                appendInstruction(Executable, Opcode::Add64rri, {0, 0, Storage}, RegisterId::R0);
                appendInstruction(Executable, Opcode::Add64rri, {0, 0, Initial}, Register);
                switch (Operation)
                {
                case Opcode::Store32i:
                    {
                        constexpr std::uint32_t Value = 0xAABBCCDD;
                        appendInstruction(Executable, Operation, {2, Value}, RegisterId::R0);
                        std::memcpy(reinterpret_cast<std::uint8_t*>(&Expected) + 2, &Value, sizeof(Value));
                        break;
                    }
                case Opcode::Store64i:
                    {
                        Expected = 0x0102030405060708;
                        appendInstruction(Executable, Operation, {0, Expected}, RegisterId::R0);
                        break;
                    }
                case Opcode::Load64:
                    {
                        Expected = Sources[0];
                        appendInstruction(Executable, Operation, {0, addressOf(Sources.data())}, RegisterId::R0);
                        break;
                    }
                case Opcode::Add64mrr:
                    {
                        Expected = Sources[0] + Sources[1];
                        appendInstruction(Executable, Operation, {0, Sources[0], Sources[1]}, RegisterId::R0);
                        break;
                    }
                case Opcode::Add64mmi:
                    {
                        Expected = Initial + 5;
                        appendInstruction(Executable, Operation, {0, 0, 5}, RegisterId::R0, RegisterId::R0);
                        break;
                    }
                case Opcode::Add64mmm:
                    {
                        Expected = Sources[0] + Sources[1];
                        appendInstruction(Executable, Operation, {0, addressOf(Sources.data()), addressOf(Sources.data() + 1)}, RegisterId::R0);
                        break;
                    }
                default:
                    FAIL() << "Unexpected memory-write test case";
                }
                appendInstruction(Executable, Opcode::Add64rrr, {0, 0, 0}, RegisterId::R1, Register);
                const auto ReturnOffset = appendInstruction(Executable, Opcode::Ret);
                ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
                const auto* Entry = VM.getCodeSegment().data();

                ASSERT_TRUE(VM.execute(Entry));

                EXPECT_EQ(READ_REGISTER(RegisterId::R1, VM.getFrame()), Expected);
                EXPECT_EQ(READ_REGISTER(Register, VM.getFrame()), Expected);
                EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(Entry + ReturnOffset));
                EXPECT_EQ(VM.getFrame().Register, Bindings);
            }
        }
    }

    // Typed BP/SP writes are immediately visible through their byte-storage aliases, and aliased memory sources are read before committing the destination.
    TEST(VMStackRegisterTest, PublishesStackRegisterWritesBeforeAliasedMemoryReads)
    {
        constexpr RegisterId Registers[] = {
            RegisterId::BP,
            RegisterId::SP,
        };
        constexpr Opcode Operations[] = {
            Opcode::Add64rrr,
            Opcode::Add64rri,
            Opcode::Add64rrm,
        };
        for (const auto Register : Registers)
        {
            for (const auto Operation : Operations)
            {
                SCOPED_TRACE(static_cast<std::uint8_t>(Register));
                SCOPED_TRACE(static_cast<std::uint16_t>(Operation));
                Interpreter VM;
                const auto Storage = addressOf(GET_REGISTER(Register, VM.getFrame()));
                const auto Bindings = VM.getFrame().Register;
                const auto MemorySource = Operation == Opcode::Add64rrm;
                const auto Expected = MemorySource ? 34U : 42U;
                ExecutableFile Executable;
                appendInstruction(Executable, Opcode::Add64rri, {0, 0, Storage}, RegisterId::R0);
                appendInstruction(Executable, Opcode::Add64rri, {0, 0, 17}, Register);
                appendInstruction(Executable, Operation, {0, 0, MemorySource ? 0U : 25U}, Register, Register, MemorySource ? RegisterId::R0 : RegisterId::ZERO);
                appendInstruction(Executable, Opcode::Add64rrm, {0, 0, 0}, RegisterId::R1, RegisterId::ZERO, RegisterId::R0);
                appendInstruction(Executable, Opcode::Add64rrr, {0, 0, 0}, RegisterId::R2, Register);
                appendInstruction(Executable, Opcode::Ret);
                ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);

                ASSERT_TRUE(VM.execute(VM.getCodeSegment().data()));

                EXPECT_EQ(READ_REGISTER(RegisterId::R1, VM.getFrame()), Expected);
                EXPECT_EQ(READ_REGISTER(RegisterId::R2, VM.getFrame()), Expected);
                EXPECT_EQ(READ_REGISTER(Register, VM.getFrame()), Expected);
                EXPECT_EQ(VM.getFrame().Register, Bindings);
            }
        }
    }

#if INK_PLATFORM_WINDOWS
    // A native output pointer can change BP through its live storage, and the next instruction must observe that write while native return slots use the current SP.
    TEST(VMStackRegisterTest, ReadsBasePointerChangedThroughNativeOutputArgument)
    {
        Interpreter VM;
        const auto BPStorage = addressOf(GET_REGISTER(RegisterId::BP, VM.getFrame()));
        const auto Bindings = VM.getFrame().Register;
        const std::uint8_t Byte = 0x41;
        ExecutableFile Executable;
        appendInstruction(Executable, Opcode::Ret);
        const auto Main = Executable.byteSize();
        appendInstruction(Executable, Opcode::Add64rri, {0, 0, 48}, RegisterId::SP, RegisterId::SP);
        appendInstruction(Executable, Opcode::Add64rri, {0, 0, 8}, RegisterId::BP, RegisterId::BP);
        appendInstruction(Executable, Opcode::Store64i, {std::uint64_t{0} - 48, 0}, RegisterId::SP);
        appendInstruction(Executable, Opcode::Store64i, {std::uint64_t{0} - 40, std::numeric_limits<std::uint64_t>::max()}, RegisterId::SP);
        appendInstruction(Executable, Opcode::Store64i, {std::uint64_t{0} - 32, addressOf(&Byte)}, RegisterId::SP);
        appendInstruction(Executable, Opcode::Store64i, {std::uint64_t{0} - 24, 1}, RegisterId::SP);
        appendInstruction(Executable, Opcode::Store64i, {std::uint64_t{0} - 16, BPStorage}, RegisterId::SP);
        appendInstruction(Executable, Opcode::Store64i, {std::uint64_t{0} - 8, 0}, RegisterId::SP);
        const auto Call = appendInstruction(Executable, Opcode::Calli, {0});
        appendInstruction(Executable, Opcode::Add64rrr, {0, 0, 0}, RegisterId::R1, RegisterId::BP);
        appendInstruction(Executable, Opcode::Add64rrm, {0, 0, std::uint64_t{0} - 48}, RegisterId::R2, RegisterId::ZERO, RegisterId::SP);
        const auto ReturnOffset = appendInstruction(Executable, Opcode::Ret);
        Executable.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("WriteFile")});
        Executable.getDynamicImportSymbolRelocations().push_back({Call + 8, 0, 0, RelocationKind::DirectCall});
        ASSERT_NE(VM.loadLibrary(std::move(Executable)), Interpreter::InvalidLibraryHandle);
        const auto* Entry = VM.getCodeSegment().data();
        ASSERT_TRUE(VM.execute(Entry));
        const auto StackStart = READ_REGISTER(RegisterId::SP, VM.getFrame());
        std::uint64_t ExpectedBP = StackStart + 8;
        constexpr std::uint32_t BytesWritten = 0;
        std::memcpy(&ExpectedBP, &BytesWritten, sizeof(BytesWritten));

        ASSERT_TRUE(VM.execute(Entry + Main));

        EXPECT_EQ(READ_REGISTER(RegisterId::R1, VM.getFrame()), ExpectedBP);
        EXPECT_EQ(READ_REGISTER(RegisterId::BP, VM.getFrame()), ExpectedBP);
        EXPECT_EQ(READ_REGISTER(RegisterId::R2, VM.getFrame()), 0U);
        EXPECT_EQ(READ_REGISTER(RegisterId::SP, VM.getFrame()), StackStart + 48);
        EXPECT_EQ(READ_REGISTER(RegisterId::PC, VM.getFrame()), addressOf(Entry + ReturnOffset));
        EXPECT_EQ(VM.getFrame().Register, Bindings);
    }
#endif
} // namespace ink::vm::test
