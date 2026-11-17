#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace ink::vm::test
{
    namespace
    {
        constexpr Opcode RegisteredOpcodes[] = {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category) Opcode::Kind,
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
        };

        template <Opcode Operation, typename OperandType>
        OperandType exampleOperand(std::size_t Index, const MachineBasicBlock* Target)
        {
            if constexpr (std::is_same_v<OperandType, RegisterOperand>)
            {
                if constexpr (OpcodeTraits<Operation>::Category == InstructionCategory::FloatingPoint)
                {
                    return RegisterOperand(Index == 0 ? RegisterId::R0 : RegisterId::R9);
                }
                constexpr std::array Registers{RegisterId::BP, RegisterId::SP, RegisterId::PC};
                constexpr std::array<std::int64_t, 3> Offsets{0, std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()};
                return RegisterOperand(Registers[Index], Offsets[Index]);
            }
            else if constexpr (std::is_same_v<OperandType, AddressOperand>)
            {
                if constexpr (OpcodeTraits<Operation>::BlockCount == 1)
                {
                    return AddressOperand(Index == 0 ? RegisterId::BP : RegisterId::SP, static_cast<std::int64_t>(Index * 8));
                }
                return Index == 1 ? AddressOperand(core::NamePool::instance().intern("external.data"), std::numeric_limits<std::int64_t>::max()) : AddressOperand(RegisterId::BP, std::numeric_limits<std::int64_t>::min());
            }
            else if constexpr (std::is_same_v<OperandType, BasicBlockOperand>)
            {
                return BasicBlockOperand(Target);
            }
            else
            {
                if constexpr (OpcodeTraits<Operation>::BlockCount == 1)
                {
                    return ImmediateOperand(127);
                }
                return Index == 1 ? ImmediateOperand(core::NamePool::instance().intern("external.address"), std::numeric_limits<std::int64_t>::min()) : ImmediateOperand(std::numeric_limits<std::uint64_t>::max() - Index);
            }
        }

        template <Opcode Operation, std::size_t... Indices>
        void appendExampleInstruction(MachineInstructionBuilder& Builder, const MachineBasicBlock* Target, std::index_sequence<Indices...>)
        {
            using Operands = typename OpcodeTraits<Operation>::OperandTuple;
            Builder.create<Operation>(exampleOperand<Operation, std::tuple_element_t<Indices, Operands>>(Indices, Target)...);
        }

        void expectEqualOperands(const Operand& Expected, const Operand& Actual, const std::unordered_map<const MachineBasicBlock*, const MachineBasicBlock*>& BlockMap)
        {
            ASSERT_EQ(Actual.getKind(), Expected.getKind());
            switch (Expected.getKind())
            {
            case OperandKind::Register:
                EXPECT_EQ(static_cast<const RegisterOperand&>(Actual).getRegister(), static_cast<const RegisterOperand&>(Expected).getRegister());
                EXPECT_EQ(static_cast<const RegisterOperand&>(Actual).getByteOffset(), static_cast<const RegisterOperand&>(Expected).getByteOffset());
                break;
            case OperandKind::Address:
                {
                    const auto& ExpectedAddress = static_cast<const AddressOperand&>(Expected);
                    const auto& ActualAddress = static_cast<const AddressOperand&>(Actual);
                    EXPECT_EQ(ActualAddress.getBaseKind(), ExpectedAddress.getBaseKind());
                    EXPECT_EQ(ActualAddress.getRegister(), ExpectedAddress.getRegister());
                    EXPECT_EQ(ActualAddress.getSymbol(), ExpectedAddress.getSymbol());
                    EXPECT_EQ(ActualAddress.getByteOffset(), ExpectedAddress.getByteOffset());
                    break;
                }
            case OperandKind::Immediate:
                {
                    const auto& ExpectedImmediate = static_cast<const ImmediateOperand&>(Expected);
                    const auto& ActualImmediate = static_cast<const ImmediateOperand&>(Actual);
                    EXPECT_EQ(ActualImmediate.getValueKind(), ExpectedImmediate.getValueKind());
                    EXPECT_EQ(ActualImmediate.bits(), ExpectedImmediate.bits());
                    EXPECT_EQ(ActualImmediate.getSymbol(), ExpectedImmediate.getSymbol());
                    EXPECT_EQ(ActualImmediate.getByteOffset(), ExpectedImmediate.getByteOffset());
                    break;
                }
            case OperandKind::BasicBlock:
                {
                    const auto Target = BlockMap.find(static_cast<const BasicBlockOperand&>(Expected).getBlock());
                    ASSERT_NE(Target, BlockMap.end());
                    EXPECT_EQ(static_cast<const BasicBlockOperand&>(Actual).getBlock(), Target->second);
                    break;
                }
            }
        }

        void expectEqualModules(const MachineModule& Expected, const MachineModule& Actual)
        {
            EXPECT_EQ(Actual.name(), Expected.name());
            ASSERT_EQ(Actual.symbolDeclarations().size(), Expected.symbolDeclarations().size());
            for (std::size_t Index = 0; Index != Expected.symbolDeclarations().size(); ++Index)
            {
                EXPECT_EQ(Actual.symbolDeclarations()[Index].Name, Expected.symbolDeclarations()[Index].Name);
                EXPECT_EQ(Actual.symbolDeclarations()[Index].Kind, Expected.symbolDeclarations()[Index].Kind);
                EXPECT_EQ(Actual.symbolDeclarations()[Index].IsConstant, Expected.symbolDeclarations()[Index].IsConstant);
            }
            ASSERT_EQ(Actual.globalVariableCount(), Expected.globalVariableCount());
            auto ActualGlobal = Actual.globalVariables().begin();
            for (const auto& ExpectedGlobal : Expected.globalVariables())
            {
                EXPECT_EQ(ActualGlobal->name(), ExpectedGlobal.name());
                EXPECT_EQ(ActualGlobal->linkage(), ExpectedGlobal.linkage());
                EXPECT_EQ(ActualGlobal->isConstant(), ExpectedGlobal.isConstant());
                EXPECT_EQ(ActualGlobal->byteSize(), ExpectedGlobal.byteSize());
                EXPECT_EQ(ActualGlobal->alignment(), ExpectedGlobal.alignment());
                EXPECT_TRUE(std::ranges::equal(ActualGlobal->initialBytes(), ExpectedGlobal.initialBytes()));
                EXPECT_EQ(ActualGlobal->parent(), &Actual);
                ++ActualGlobal;
            }
            ASSERT_EQ(Actual.functionCount(), Expected.functionCount());
            auto ActualFunction = Actual.functions().begin();
            for (const auto& ExpectedFunction : Expected.functions())
            {
                EXPECT_EQ(ActualFunction->name(), ExpectedFunction.name());
                EXPECT_EQ(ActualFunction->linkage(), ExpectedFunction.linkage());
                EXPECT_EQ(ActualFunction->parent(), &Actual);
                const auto& ExpectedFrame = ExpectedFunction.frameInfo();
                const auto& ActualFrame = ActualFunction->frameInfo();
                EXPECT_EQ(ActualFrame.alignment(), ExpectedFrame.alignment());
                EXPECT_EQ(ActualFrame.baseOffset(), ExpectedFrame.baseOffset());
                EXPECT_EQ(ActualFrame.byteSize(), ExpectedFrame.byteSize());
                ASSERT_EQ(ActualFrame.stackSlotCount(), ExpectedFrame.stackSlotCount());
                auto ActualSlot = ActualFrame.stackSlots().begin();
                for (const auto& ExpectedSlot : ExpectedFrame.stackSlots())
                {
                    EXPECT_EQ(ActualSlot->byteSize(), ExpectedSlot.byteSize());
                    EXPECT_EQ(ActualSlot->alignment(), ExpectedSlot.alignment());
                    EXPECT_EQ(ActualSlot->byteOffset(), ExpectedSlot.byteOffset());
                    ++ActualSlot;
                }
                ASSERT_EQ(ActualFunction->basicBlockCount(), ExpectedFunction.basicBlockCount());
                std::unordered_map<const MachineBasicBlock*, const MachineBasicBlock*> BlockMap;
                auto MappedBlock = ActualFunction->basicBlocks().begin();
                for (const auto& ExpectedBlock : ExpectedFunction.basicBlocks())
                {
                    BlockMap.emplace(&ExpectedBlock, &*MappedBlock++);
                }
                auto ActualBlock = ActualFunction->basicBlocks().begin();
                for (const auto& ExpectedBlock : ExpectedFunction.basicBlocks())
                {
                    EXPECT_EQ(ActualBlock->name(), ExpectedBlock.name());
                    EXPECT_EQ(ActualBlock->parent(), &*ActualFunction);
                    ASSERT_EQ(ActualBlock->instructionCount(), ExpectedBlock.instructionCount());
                    auto ActualInstructions = ActualBlock->instructions();
                    auto ActualInstruction = ActualInstructions.begin();
                    for (const auto& ExpectedInstruction : ExpectedBlock.instructions())
                    {
                        const auto& ActualValue = *ActualInstruction;
                        EXPECT_EQ(ActualValue.getOpcode(), ExpectedInstruction.getOpcode());
                        EXPECT_EQ(ActualValue.parent(), &*ActualBlock);
                        ASSERT_EQ(ActualValue.operandCount(), ExpectedInstruction.operandCount());
                        for (std::size_t Index = 0; Index < ExpectedInstruction.operandCount(); ++Index)
                        {
                            ASSERT_NE(ActualValue.getOperand(Index), nullptr);
                            expectEqualOperands(*ExpectedInstruction.getOperand(Index), *ActualValue.getOperand(Index), BlockMap);
                        }
                        ++ActualInstruction;
                    }
                    ++ActualBlock;
                }
                ++ActualFunction;
            }
        }
    } // namespace

    // Every opcode retains its exact ordered operand types, symbolic addresses, signed offsets, and raw immediate bits.
    TEST(VMMachineModuleTextTest, RoundTripsEveryOpcodeAndCompleteModuleState)
    {
        const auto SharedName = core::NamePool::instance().intern("same name");
        std::string EscapedName = "quote\";slash\\\n";
        EscapedName.push_back('\0');
        EscapedName.push_back(static_cast<char>(0xff));
        MachineModule Module(core::NamePool::instance().intern(EscapedName));
        std::array<std::uint8_t, 35> Bytes{};
        for (std::size_t Index = 0; Index < Bytes.size(); ++Index)
        {
            Bytes[Index] = static_cast<std::uint8_t>(Index * 7);
        }
        ASSERT_TRUE(Module.createGlobalVariable(Bytes.size(), 16, Bytes, SharedName, MachineSymbolLinkage::Local, true).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, SharedName, MachineSymbolLinkage::Local, true).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(std::numeric_limits<std::uint64_t>::max(), 8, Bytes, SharedName).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(0).has_value());
        auto& Function = Module.createFunction(SharedName);
        ASSERT_TRUE(Function.frameInfo().createStackSlot(4, 4).has_value());
        ASSERT_TRUE(Function.frameInfo().createStackSlot(8, 8).has_value());
        ASSERT_TRUE(Function.frameInfo().layout(17).has_value());
        auto& Block = Function.createBasicBlock(SharedName);
        auto& Target = Function.createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category) appendExampleInstruction<Opcode::Kind>(Builder, &Target, std::make_index_sequence<OpcodeTraits<Opcode::Kind>::OperandCount>{});
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
        auto& UnlaidFunction = Module.createFunction(SharedName);
        ASSERT_TRUE(UnlaidFunction.frameInfo().createStackSlot(7, 16).has_value());
        auto& EmptyLaidFunction = Module.createFunction();
        ASSERT_TRUE(EmptyLaidFunction.frameInfo().layout(31).has_value());
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Text);
        EXPECT_EQ((*Parsed)->functions().begin()->basicBlocks().begin()->instructionCount(), std::size(RegisteredOpcodes));
        EXPECT_LT(Text->size(), 10000U);
    }

    // Empty anonymous modules serialize deterministically and do not acquire declarations or a frame during parsing.
    TEST(VMMachineModuleTextTest, RoundTripsEmptyAnonymousModule)
    {
        MachineModule Module;
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find(".module \"\""), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
    }

    // Unique node names appear once, and each function can reuse entry or a module-level name as a block label.
    TEST(VMMachineModuleTextTest, UsesNodeNamesAndFunctionLocalBlockLabels)
    {
        auto& Names = core::NamePool::instance();
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, Names.intern("message"), MachineSymbolLinkage::Local, true).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(16, 8, {}, Names.intern("scratch")).has_value());
        auto& Sum = Module.createFunction(Names.intern("sum"));
        Sum.createBasicBlock(Names.intern("entry"));
        Sum.createBasicBlock(Names.intern("sum"));
        Module.createFunction(Names.intern("another")).createBasicBlock(Names.intern("entry"));
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find(".global message, 0, 1 readonly\n"), std::string::npos);
        EXPECT_NE(Text->find(".global scratch, 16, 8\n"), std::string::npos);
        EXPECT_NE(Text->find(".function sum\nentry:\nsum:\n.endfunction\n"), std::string::npos);
        EXPECT_NE(Text->find(".function another\nentry:\n.endfunction\n"), std::string::npos);
        EXPECT_EQ(Text->find("\"sum\""), std::string::npos);
        EXPECT_EQ(Text->find(".block"), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
    }

    // Generated labels avoid later real names, while aliases retain duplicate, anonymous, and arbitrary-byte node names.
    TEST(VMMachineModuleTextTest, PreservesNamesWhenGeneratedLabelsWouldCollide)
    {
        auto& Names = core::NamePool::instance();
        MachineModule Module;
        ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, core::NAME_None, MachineSymbolLinkage::Local, true).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, Names.intern(".G0"), MachineSymbolLinkage::Local, true).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(0).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, Names.intern(".G2")).has_value());
        Module.createFunction();
        Module.createFunction(Names.intern(".F0"));
        auto& Sum = Module.createFunction(Names.intern("sum"));
        Sum.createBasicBlock(Names.intern("entry"));
        Sum.createBasicBlock(Names.intern("entry"));
        Sum.createBasicBlock(Names.intern("entry.1"));
        Sum.createBasicBlock();
        Sum.createBasicBlock(Names.intern(".L0"));
        Module.createFunction(Names.intern("sum"));
        Module.createFunction(Names.intern("sum.1"));
        const std::string UnusualName("space name\0\xff", 12);
        const auto Unusual = Names.intern(UnusualName);
        ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, Unusual, MachineSymbolLinkage::Local, true).has_value());
        ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, Unusual).has_value());
        Module.createFunction(Unusual).createBasicBlock(Unusual);
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find(".global .G1 \"\", 0, 1 readonly\n"), std::string::npos);
        EXPECT_NE(Text->find(".global .G0, 0, 1 readonly\n"), std::string::npos);
        EXPECT_NE(Text->find(".global .G3 \"\", 0, 1\n"), std::string::npos);
        EXPECT_NE(Text->find(".global .G2, 0, 1\n"), std::string::npos);
        EXPECT_NE(Text->find(".function .F1 \"\"\n"), std::string::npos);
        EXPECT_NE(Text->find(".function .F0\n"), std::string::npos);
        EXPECT_NE(Text->find(".function sum.2 \"sum\"\n"), std::string::npos);
        EXPECT_NE(Text->find("entry.2: \"entry\"\n"), std::string::npos);
        EXPECT_NE(Text->find("entry.1:\n"), std::string::npos);
        EXPECT_NE(Text->find(".L1: \"\"\n"), std::string::npos);
        EXPECT_NE(Text->find(".L0:\n"), std::string::npos);
        EXPECT_NE(Text->find("\"space name\\00\\FF\""), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Text);
    }

    // Explicit original-name overrides preserve repeated and anonymous names independently of their textual aliases.
    TEST(VMMachineModuleTextTest, ReadsOriginalNameOverridesInEachDeclarationKind)
    {
        constexpr std::string_view Text = R"(.module "aliases"
.section .data
.global C "shared", 0, 1 readonly
.endglobal
.global G "shared", 8, 8
.endglobal
.section .text
.function F "shared"
entry:
entry.1: "entry"
.L0: ""
.endfunction
.function second
entry:
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto& Module = **Parsed;
        auto& Names = core::NamePool::instance();
        EXPECT_EQ(Module.globalVariables().begin()->name(), Names.intern("shared"));
        EXPECT_TRUE(Module.globalVariables().begin()->isConstant());
        EXPECT_EQ(std::next(Module.globalVariables().begin())->name(), Names.intern("shared"));
        EXPECT_FALSE(std::next(Module.globalVariables().begin())->isConstant());
        const auto& Function = *Module.functions().begin();
        EXPECT_EQ(Function.name(), Names.intern("shared"));
        ASSERT_EQ(Function.basicBlockCount(), 3U);
        const auto Blocks = Function.basicBlocks();
        EXPECT_EQ(Blocks.begin()->name(), Names.intern("entry"));
        EXPECT_EQ(std::next(Blocks.begin())->name(), Names.intern("entry"));
        EXPECT_EQ(std::next(Blocks.begin(), 2)->name(), core::NAME_None);
        EXPECT_EQ(std::next(Module.functions().begin())->basicBlocks().begin()->name(), Names.intern("entry"));
    }

    // Handwritten Intel-style text accepts comments, CRLF, byte escapes, signed immediates, and unresolved named symbols.
    TEST(VMMachineModuleTextTest, ReadsAssemblySyntaxAndOwnsParsedPayloads)
    {
        std::string Text = R"(.module "demo;name" ; a semicolon within the string remains part of its name
.section .data
.global message "A\00\FF\"\\", 2, 4 readonly
    .byte 0x48, 105
.endglobal
.global buffer "", 64, 8
    .byte 1
.endglobal
.section .text
.function sum
    .stackslot 4, 4
    .stackslot 8, 8
    .frame 16
entry: ""
    store32i [bp + 16], 20
    add64rri sp, sp, -16
    add64rrm bp, pc, [@external.data - 9223372036854775808]
.endfunction
)";
        for (std::size_t Position = 0; (Position = Text.find('\n', Position)) != std::string::npos; Position += 2)
        {
            Text.insert(Position, 1, '\r');
        }
        auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        Text.assign(Text.size(), 'x');
        const auto& Module = **Parsed;
        EXPECT_EQ(core::NamePool::instance().text(Module.name()), "demo;name");
        ASSERT_EQ(Module.globalVariableCount(), 2U);
        const auto& Global = *Module.globalVariables().begin();
        const std::array<std::uint8_t, 2> ExpectedBytes{0x48, 105};
        EXPECT_TRUE(std::ranges::equal(Global.initialBytes(), ExpectedBytes));
        EXPECT_TRUE(Global.isConstant());
        const std::string ExpectedName("A\0\xff\"\\", 5);
        EXPECT_EQ(core::NamePool::instance().text(Global.name()), ExpectedName);
        const auto& Function = *Module.functions().begin();
        EXPECT_EQ(Function.frameInfo().baseOffset(), 16U);
        EXPECT_EQ(Function.frameInfo().byteSize(), 16U);
        auto Instructions = Function.basicBlocks().begin()->instructions();
        const auto& Add = *std::next(Instructions.begin());
        EXPECT_EQ(static_cast<const ImmediateOperand&>(*Add.getOperand(2)).bits(), std::uint64_t(0) - 16);
        const auto& Address = static_cast<const AddressOperand&>(*(*std::next(Instructions.begin(), 2)).getOperand(2));
        EXPECT_EQ(Address.getSymbol(), core::NamePool::instance().intern("external.data"));
        EXPECT_EQ(Address.getByteOffset(), std::numeric_limits<std::int64_t>::min());
    }

    // Numeric spelling variants preserve boundary bit patterns, and the final function terminator needs no newline.
    TEST(VMMachineModuleTextTest, AcceptsBoundaryImmediatesAndNumericSpellings)
    {
        constexpr std::string_view Text = R"(.module ""
.section .text
.function F ""
B: ""
store64i [bp], 0
store64i [bp], 18446744073709551615
store64i [bp], -9223372036854775808
store64i [bp], +1
store64i [bp], 0XFF
.endfunction)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        constexpr std::uint64_t ExpectedBits[] = {
            0,
            std::numeric_limits<std::uint64_t>::max(),
            std::uint64_t(1) << 63,
            1,
            255,
        };
        const auto Instructions = (*Parsed)->functions().begin()->basicBlocks().begin()->instructions();
        ASSERT_EQ(std::ranges::distance(Instructions), std::size(ExpectedBits));
        std::size_t Index = 0;
        for (const auto& Instruction : Instructions)
        {
            EXPECT_EQ(static_cast<const ImmediateOperand&>(*Instruction.getOperand(1)).bits(), ExpectedBits[Index]);
            ++Index;
        }
    }

    // Register values retain signed byte offsets and remain distinct from dereferenced addresses with the same register and offset.
    TEST(VMMachineModuleTextTest, RoundTripsRegisterValueDisplacementsAndMemoryReads)
    {
        MachineModule Module;
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Module.createFunction().createBasicBlock());
        Builder.create<Opcode::Add64rrr>(RegisterOperand(RegisterId::BP), RegisterOperand(RegisterId::SP, -8), RegisterOperand(RegisterId::PC, 2));
        Builder.create<Opcode::Add64rrm>(RegisterOperand(RegisterId::SP), RegisterOperand(RegisterId::BP, std::numeric_limits<std::int64_t>::max()), AddressOperand(RegisterId::PC, 2));
        Builder.create<Opcode::Add64mrr>(AddressOperand(RegisterId::BP), RegisterOperand(RegisterId::PC, std::numeric_limits<std::int64_t>::min()), RegisterOperand(RegisterId::SP));
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find("add64rrr bp, sp - 8, pc + 2"), std::string::npos);
        EXPECT_NE(Text->find("add64rrm sp, bp + 9223372036854775807, [pc + 2]"), std::string::npos);
        EXPECT_NE(Text->find("add64mrr [bp], pc - 9223372036854775808, sp"), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Text);
    }

    // Explicit zero displacements retain ordinary register syntax, while hexadecimal register offsets normalize to signed decimal.
    TEST(VMMachineModuleTextTest, ReadsRegisterDisplacementsAndNormalizesZero)
    {
        constexpr std::string_view Text = R"(.module ""
.section .text
.function F
entry:
    add64rrr bp + 0, sp - 0, pc + 0
    add64rrr bp, pc + 0x2, sp - 0x8
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto Instructions = (*Parsed)->functions().begin()->basicBlocks().begin()->instructions();
        const auto& Add = static_cast<const MachineAdd64rrrInstruction&>(*Instructions.begin());
        EXPECT_EQ(Add.getDestination().getByteOffset(), 0);
        EXPECT_EQ(Add.getLHS().getByteOffset(), 0);
        EXPECT_EQ(Add.getRHS().getByteOffset(), 0);
        const auto& DisplacedAdd = static_cast<const MachineAdd64rrrInstruction&>(*std::next(Instructions.begin()));
        EXPECT_EQ(DisplacedAdd.getLHS().getRegister(), RegisterId::PC);
        EXPECT_EQ(DisplacedAdd.getLHS().getByteOffset(), 2);
        EXPECT_EQ(DisplacedAdd.getRHS().getByteOffset(), -8);
        const auto Written = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Written.has_value());
        EXPECT_NE(Written->find("add64rrr bp, sp, pc\n"), std::string::npos);
        EXPECT_NE(Written->find("add64rrr bp, pc + 2, sp - 8\n"), std::string::npos);
    }

    // ZERO is accepted as a read-only value and memory base, and zero or hexadecimal offsets normalize like other registers.
    TEST(VMMachineModuleTextTest, RoundTripsZeroRegisterSourcesAndAddressBases)
    {
        constexpr std::string_view Text = R"(.module ""
.section .text
.function F
entry:
    add64rrr bp, zero + 0, zero + 0x10
    store64m [zero + 0], [zero - 0x8]
    add64rrr sp, zero, zero - 8
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto Instructions = (*Parsed)->functions().begin()->basicBlocks().begin()->instructions();
        const auto& Add = static_cast<const MachineAdd64rrrInstruction&>(*Instructions.begin());
        EXPECT_EQ(Add.getLHS().getRegister(), RegisterId::ZERO);
        EXPECT_EQ(Add.getLHS().getByteOffset(), 0);
        EXPECT_EQ(Add.getRHS().getRegister(), RegisterId::ZERO);
        EXPECT_EQ(Add.getRHS().getByteOffset(), 16);
        const auto& Store = static_cast<const MachineStore64mInstruction&>(*std::next(Instructions.begin()));
        EXPECT_EQ(Store.getDestination().getRegister(), RegisterId::ZERO);
        EXPECT_EQ(Store.getDestination().getByteOffset(), 0);
        EXPECT_EQ(Store.getSource().getRegister(), RegisterId::ZERO);
        EXPECT_EQ(Store.getSource().getByteOffset(), -8);
        const auto Written = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Written.has_value());
        EXPECT_NE(Written->find("add64rrr bp, zero, zero + 16\n"), std::string::npos);
        EXPECT_NE(Written->find("store64m [zero], [zero - 8]\n"), std::string::npos);
        EXPECT_NE(Written->find("add64rrr sp, zero, zero - 8\n"), std::string::npos);
        const auto Reparsed = readMachineModuleText(*Written);
        ASSERT_TRUE(Reparsed.has_value());
        expectEqualModules(**Parsed, **Reparsed);
    }

    // All ten general-purpose registers round-trip as arithmetic destinations, value sources, and memory bases.
    TEST(VMMachineModuleTextTest, RoundTripsGeneralPurposeRegistersInExistingInstructionForms)
    {
        constexpr std::string_view Text = R"(.module ""
.section .text
.function F
entry:
    add64rri r0, zero, 0
    add64rri r1, zero, 1
    add64rri r2, zero, 2
    add64rri r3, zero, 3
    add64rri r4, zero, 4
    add64rri r5, zero, 5
    add64rri r6, zero, 6
    add64rri r7, zero, 7
    add64rri r8, zero, 8
    add64rri r9, zero, 9
    add64rrr r0, r8 - 8, r9 + 16
    add64rrm r9, r0, [r1 + 8]
    add64mrr [r2 + 16], r3, r4
    store64i [r5], 42
    jmpr r6 + 8
    call [r7]
    je64mib [r8], 0, done
done:
    ret
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto Instructions = (*Parsed)->functions().begin()->basicBlocks().begin()->instructions();
        ASSERT_EQ(std::ranges::distance(Instructions), 17);
        auto Position = Instructions.begin();
        for (std::uint8_t Index = 0; Index < 10; ++Index, ++Position)
        {
            const auto& Add = static_cast<const MachineAdd64rriInstruction&>(*Position);
            EXPECT_EQ(Add.getDestination().getRegister(), static_cast<RegisterId>(0x04 + Index));
            EXPECT_EQ(Add.getRHS().bits(), Index);
        }
        const auto& Add = static_cast<const MachineAdd64rrrInstruction&>(*Position);
        EXPECT_EQ(Add.getLHS().getRegister(), RegisterId::R8);
        EXPECT_EQ(Add.getLHS().getByteOffset(), -8);
        EXPECT_EQ(Add.getRHS().getRegister(), RegisterId::R9);
        EXPECT_EQ(Add.getRHS().getByteOffset(), 16);
        const auto Written = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Written.has_value());
        EXPECT_NE(Written->find("add64rrr r0, r8 - 8, r9 + 16\n"), std::string::npos);
        EXPECT_NE(Written->find("add64rrm r9, r0, [r1 + 8]\n"), std::string::npos);
        EXPECT_NE(Written->find("add64mrr [r2 + 16], r3, r4\n"), std::string::npos);
        EXPECT_NE(Written->find("store64i [r5], 0x2a\n"), std::string::npos);
        EXPECT_NE(Written->find("jmpr r6 + 8\n"), std::string::npos);
        EXPECT_NE(Written->find("call [r7]\n"), std::string::npos);
        EXPECT_NE(Written->find("je64mib [r8], 0x0, done\n"), std::string::npos);
        const auto Reparsed = readMachineModuleText(*Written);
        ASSERT_TRUE(Reparsed.has_value());
        expectEqualModules(**Parsed, **Reparsed);
    }

    // All five floating-point mnemonics round-trip GPR values and the read-only ZERO source without changing operand forms.
    TEST(VMMachineModuleTextTest, RoundTripsFloatingPointRegisterInstructions)
    {
        constexpr std::string_view Text = R"(.module ""
.section .text
.function F
entry:
    fadd64rrr r0, r1, r9
    fsub64rrr r1, r0, zero
    fmul64rrr r2, r1, r2
    fdiv64rrr r9, r2, r0
    fsqrt64rr r0, r9
    fsqrt64rr r1, zero
    ret
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto Written = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Written.has_value());
        EXPECT_NE(Written->find("fadd64rrr r0, r1, r9\n"), std::string::npos);
        EXPECT_NE(Written->find("fsub64rrr r1, r0, zero\n"), std::string::npos);
        EXPECT_NE(Written->find("fmul64rrr r2, r1, r2\n"), std::string::npos);
        EXPECT_NE(Written->find("fdiv64rrr r9, r2, r0\n"), std::string::npos);
        EXPECT_NE(Written->find("fsqrt64rr r0, r9\n"), std::string::npos);
        const auto Reparsed = readMachineModuleText(*Written);
        ASSERT_TRUE(Reparsed.has_value());
        expectEqualModules(**Parsed, **Reparsed);
    }

    // Floating-point sources and destinations reject every nonzero displacement, address register, and memory operand.
    TEST(VMMachineModuleTextTest, RejectsFloatingPointDisplacementsAndNonValueRegisters)
    {
        constexpr std::string_view BinaryMnemonics[] = {
            "fadd64rrr",
            "fsub64rrr",
            "fmul64rrr",
            "fdiv64rrr",
        };
        constexpr std::string_view BinaryOperands[] = {
            "r0 + 1, r1, r2",
            "r0, r1 - 1, r2",
            "r0, r1, r2 + 8",
            "bp, r1, r2",
            "sp, r1, r2",
            "pc, r1, r2",
            "zero, r1, r2",
            "r0, bp, r2",
            "r0, sp, r2",
            "r0, pc, r2",
            "r0, r1, bp",
            "r0, r1, sp",
            "r0, r1, pc",
            "r0, r1, [r2]",
        };
        for (const auto Mnemonic : BinaryMnemonics)
        {
            for (const auto Operands : BinaryOperands)
            {
                const std::string Text = ".section .text\n.function F\nentry:\n    " + std::string(Mnemonic) + " " + std::string(Operands) + "\n.endfunction\n";
                SCOPED_TRACE(Text);
                EXPECT_FALSE(readMachineModuleText(Text).has_value());
            }
        }
        constexpr std::string_view UnaryOperands[] = {
            "r0 + 1, r1",
            "r0, r1 - 8",
            "bp, r1",
            "sp, r1",
            "pc, r1",
            "zero, r1",
            "r0, bp",
            "r0, sp",
            "r0, pc",
            "r0, [r1]",
        };
        for (const auto Operands : UnaryOperands)
        {
            const std::string Text = ".section .text\n.function F\nentry:\n    fsqrt64rr " + std::string(Operands) + "\n.endfunction\n";
            SCOPED_TRACE(Text);
            EXPECT_FALSE(readMachineModuleText(Text).has_value());
        }
    }

    // Directly constructed floating-point modules cannot bypass the writer's zero-offset and value-register validation.
    TEST(VMMachineModuleTextTest, RejectsInvalidFloatingPointOperandsWhenWriting)
    {
        constexpr RegisterOperand InvalidOperands[] = {
            RegisterOperand(RegisterId::R0, 1),
            RegisterOperand(RegisterId::BP),
            RegisterOperand(RegisterId::SP),
            RegisterOperand(RegisterId::PC),
            RegisterOperand(static_cast<RegisterId>(0x0E)),
        };
        for (const auto& Invalid : InvalidOperands)
        {
            for (std::size_t Index = 0; Index < 3; ++Index)
            {
                std::array Operands{RegisterOperand(RegisterId::R0), RegisterOperand(RegisterId::R1), RegisterOperand(RegisterId::R9)};
                Operands[Index] = Invalid;
                MachineModule Module;
                auto& Block = Module.createFunction().createBasicBlock();
                MachineInstructionBuilder Builder;
                Builder.setInsertPoint(Block);
                Builder.create<Opcode::Fadd64rrr>(Operands[0], Operands[1], Operands[2]);
                EXPECT_FALSE(writeMachineModuleText(Module).has_value());
                if (Index < 2)
                {
                    MachineModule UnaryModule;
                    auto& UnaryBlock = UnaryModule.createFunction().createBasicBlock();
                    Builder.setInsertPoint(UnaryBlock);
                    Builder.create<Opcode::Fsqrt64rr>(Operands[0], Operands[1]);
                    EXPECT_FALSE(writeMachineModuleText(UnaryModule).has_value());
                }
            }
        }
    }

    // Register expressions reject missing, repeated, overflowing displacements and unknown register names.
    TEST(VMMachineModuleTextTest, RejectsMalformedRegisterDisplacements)
    {
        constexpr std::string_view InvalidOperands[] = {
            "pc +",
            "pc -",
            "pc + -1",
            "pc - +1",
            "pc + 9223372036854775808",
            "pc - 9223372036854775809",
            "pc + 18446744073709551616",
            "pc + 1 + 2",
            "invalid + 1",
            "r10",
            "r255",
        };
        for (const auto OperandText : InvalidOperands)
        {
            SCOPED_TRACE(OperandText);
            const std::string Text = ".module \"\"\n.section .text\n.function F\nentry:\n    add64rrr bp, sp, " + std::string(OperandText) + "\n.endfunction\n";
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
    }

    // Writable registers reject destination offsets, while PC and ZERO remain read-only arithmetic destinations.
    TEST(VMMachineModuleTextTest, RejectsNonzeroOrReadOnlyRegisterDestinations)
    {
        constexpr std::string_view InvalidDestinations[] = {
            "bp + 1",
            "sp - 8",
            "pc",
            "pc + 2",
            "zero",
            "zero + 8",
            "r0 + 1",
            "r9 - 8",
            "r10",
        };
        for (const auto DestinationText : InvalidDestinations)
        {
            SCOPED_TRACE(DestinationText);
            const std::string Text = ".module \"\"\n.section .text\n.function F\nentry:\n    add64rri " + std::string(DestinationText) + ", sp, 1\n.endfunction\n";
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
        constexpr RegisterOperand Destinations[] = {
            RegisterOperand(RegisterId::BP, 1),
            RegisterOperand(RegisterId::SP, -8),
            RegisterOperand(RegisterId::PC),
            RegisterOperand(RegisterId::PC, 2),
            RegisterOperand(RegisterId::ZERO),
            RegisterOperand(RegisterId::ZERO, 8),
            RegisterOperand(RegisterId::R0, 1),
            RegisterOperand(RegisterId::R9, -8),
            RegisterOperand(static_cast<RegisterId>(0x0E)),
        };
        for (const auto& Destination : Destinations)
        {
            MachineModule Module;
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Module.createFunction().createBasicBlock());
            Builder.create<Opcode::Add64rri>(Destination, RegisterOperand(RegisterId::SP), ImmediateOperand(1));
            const auto Written = writeMachineModuleText(Module);
            ASSERT_FALSE(Written.has_value());
            EXPECT_EQ(Written.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
    }

    // Symbolic immediates preserve address values separately from memory reads, including unusual names and signed offset limits.
    TEST(VMMachineModuleTextTest, RoundTripsSymbolAddressValuesAndMemoryReads)
    {
        auto& Names = core::NamePool::instance();
        const std::string UnusualName("quote\";slash\\\0\xff", 15);
        const core::Name Symbols[] = {
            Names.intern("bp"),
            Names.intern("1000"),
            Names.intern("external.data"),
            Names.intern("space name"),
            Names.intern(UnusualName),
        };
        constexpr std::int64_t Offsets[] = {
            0,
            std::numeric_limits<std::int64_t>::min(),
            std::numeric_limits<std::int64_t>::max(),
            -8,
            8,
        };
        MachineModule Module;
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Module.createFunction().createBasicBlock());
        for (std::size_t Index = 0; Index < std::size(Symbols); ++Index)
        {
            Builder.create<Opcode::Add64mii>(AddressOperand(RegisterId::BP), ImmediateOperand(Symbols[Index], Offsets[Index]), ImmediateOperand(1));
            Builder.create<Opcode::Add64mmi>(AddressOperand(RegisterId::BP), AddressOperand(Symbols[Index], Offsets[Index]), ImmediateOperand(1));
        }
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find("add64mii [bp], @bp, 0x1"), std::string::npos);
        EXPECT_NE(Text->find("add64mmi [bp], [@bp], 0x1"), std::string::npos);
        EXPECT_NE(Text->find("@\"1000\" - 9223372036854775808"), std::string::npos);
        EXPECT_NE(Text->find("@external.data + 9223372036854775807"), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Text);
    }

    // Bare and quoted symbol spellings remain unresolved and preserve signed decimal and hexadecimal addends.
    TEST(VMMachineModuleTextTest, ReadsUnresolvedSymbolsAndAddressAddends)
    {
        constexpr std::string_view Text = R"(.module ""
.section .text
.function F
entry:
    store64i [@external + 0x10], @reserved - 0x8
    add64mii [bp], @"123" + 0, @"quoted\00symbol" - 9223372036854775808
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        auto& Names = core::NamePool::instance();
        const auto Instructions = (*Parsed)->functions().begin()->basicBlocks().begin()->instructions();
        const auto& Store = static_cast<const MachineStore64iInstruction&>(*Instructions.begin());
        EXPECT_EQ(Store.getDestination().getSymbol(), Names.intern("external"));
        EXPECT_EQ(Store.getDestination().getByteOffset(), 16);
        EXPECT_EQ(Store.getSource().getSymbol(), Names.intern("reserved"));
        EXPECT_EQ(Store.getSource().getByteOffset(), -8);
        EXPECT_FALSE(Store.getSource().bits().has_value());
        const auto& Add = static_cast<const MachineAdd64miiInstruction&>(*std::next(Instructions.begin()));
        EXPECT_EQ(Add.getLHS().getSymbol(), Names.intern("123"));
        EXPECT_EQ(Add.getLHS().getByteOffset(), 0);
        EXPECT_EQ(Add.getRHS().getSymbol(), Names.intern(std::string_view("quoted\0symbol", 13)));
        EXPECT_EQ(Add.getRHS().getByteOffset(), std::numeric_limits<std::int64_t>::min());
    }

    // Missing symbol names, malformed addends, overflowing displacements, and removed reference-index syntax return diagnostics.
    TEST(VMMachineModuleTextTest, RejectsMalformedSymbolExpressions)
    {
        constexpr std::string_view InvalidOperands[] = {
            "@",
            "@\"\"",
            "@123",
            "external",
            "@external +",
            "@external -",
            "@external + -1",
            "@external - +1",
            "@external + 9223372036854775808",
            "@external - 9223372036854775809",
            "@external + 18446744073709551616",
            "@external + 1 + 2",
        };
        for (const auto OperandText : InvalidOperands)
        {
            SCOPED_TRACE(OperandText);
            const std::string Text = ".module \"\"\n.section .text\n.function F\nentry:\n    store64i [bp], " + std::string(OperandText) + "\n.endfunction\n";
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
        constexpr std::string_view InvalidAddresses[] = {
            "[ref0]",
            "[r10]",
            "[r255 + 8]",
            "[@]",
            "[@\"\"]",
            "[external]",
            "[@external + 9223372036854775808]",
            "[@external - 9223372036854775809]",
            "[@external + -1]",
        };
        for (const auto AddressText : InvalidAddresses)
        {
            SCOPED_TRACE(AddressText);
            const std::string Text = ".module \"\"\n.section .text\n.function F\nentry:\n    store64i " + std::string(AddressText) + ", 0\n.endfunction\n";
            EXPECT_FALSE(readMachineModuleText(Text).has_value());
        }
    }

    // Empty symbol identities are rejected in both immediate values and memory addresses before writing assembly.
    TEST(VMMachineModuleTextTest, RejectsEmptySymbolNamesWhenWriting)
    {
        MachineModule ImmediateModule;
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(ImmediateModule.createFunction().createBasicBlock());
        Builder.create<Opcode::Store64i>(AddressOperand(RegisterId::BP), ImmediateOperand(core::NAME_None));
        const auto ImmediateText = writeMachineModuleText(ImmediateModule);
        ASSERT_FALSE(ImmediateText.has_value());
        EXPECT_EQ(ImmediateText.error().Kind, core::DiagnosticKind::VMTextInvalid);
        MachineModule AddressModule;
        Builder.setInsertPoint(AddressModule.createFunction().createBasicBlock());
        Builder.create<Opcode::Store64i>(AddressOperand(core::NAME_None), ImmediateOperand(0));
        const auto AddressText = writeMachineModuleText(AddressModule);
        ASSERT_FALSE(AddressText.has_value());
        EXPECT_EQ(AddressText.error().Kind, core::DiagnosticKind::VMTextInvalid);
    }

    // An instruction error reports its position in the original source including preceding lines and indentation.
    TEST(VMMachineModuleTextTest, ReportsAbsoluteInstructionErrorOffset)
    {
        constexpr std::string_view Text = ".module \"\"\r\n.section .text\r\n.function F\r\nB:\r\n    unknown [bp], 1\r\n.endfunction";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_FALSE(Parsed.has_value());
        ASSERT_TRUE(Parsed.error().Span.isValid());
        EXPECT_EQ(Parsed.error().Span.getBegin().getByteOffset(), Text.find("unknown"));
    }

    // Malformed modules, declarations, stack layouts, and instruction operands return diagnostics instead of partial modules.
    TEST(VMMachineModuleTextTest, RejectsMalformedStructureAndOperands)
    {
        constexpr std::string_view InvalidTexts[] = {
            "",
            ".section .text\n",
            ".module \"\"\n.module \"\"\n",
            ".module \"unterminated\n",
            ".module \"bad\\G0\"\n",
            ".module \"\" trailing\n",
            ".module \"\"\n.section .unknown\n",
            ".module \"\"\n.section .data\n.global G, 0, 3 readonly\n.endglobal\n",
            ".module \"\"\n.section .data\n.global G, 1, 1 readonly\n.byte 256\n.endglobal\n",
            ".module \"\"\n.section .data\n.global G, 1, 1 readonly\n.byte\n.endglobal\n",
            ".module \"\"\n.section .data\n.global G, 1, 1 readonly\n.byte 1,\n.endglobal\n",
            ".module \"\"\n.section .data\n.global G, 0, 1 readonly\n.section .data\n.endglobal\n",
            ".module \"\"\n.section .data\n.global G, 0, 1 readonly\n.endconstant\n",
            ".module \"\"\n.section .data\n.global G, 0, 1 readonly\n",
            ".module \"\"\n.section .data\n.constant G, 1\n.endconstant\n",
            ".module \"\"\n.section .data\n.global G, 0, 1\n.byte 1\n.endglobal\n",
            ".module \"\"\n.section .data\n.global G, 18446744073709551616, 1\n.endglobal\n",
            ".module \"\"\n.section .text\n.endfunction\n",
            ".module \"\"\n.section .text\nB:\n",
            ".module \"\"\n.section .text\n.function F\n",
            ".module \"\"\n.section .text\n.function F\n.function G\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\n.section .data\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\n.stackslot 1, 0\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\n.stackslot 18446744073709551615, 1\n.frame 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\n.frame 0\n.frame 0\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\n.frame 0\n.stackslot 1, 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nstore8i [bp], 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nB:\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\n.stackslot 1, 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nunknown [bp], 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp]\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp], 1, 2\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i bp, 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [rax], 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [ref4294967296], 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp + 9223372036854775808], 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp - 9223372036854775809], 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp], 18446744073709551616\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp], -9223372036854775809\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp], 0x\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp], 0x1g\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp], 1abc\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [ref1x], 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp + -1], 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nB:\nstore8i [bp], 1,\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\n.endfunction\n.function F\n.endfunction\n",
            ".module \"\"\n.section .data\n.global shared, 0, 1 readonly\n.endglobal\n.global shared, 0, 1\n.endglobal\n",
            ".module \"\"\n.section .data\n.global shared, 0, 1 readonly\n.endglobal\n.section .text\n.function shared\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nentry:\nentry:\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nentry: \"first\"\nentry: \"second\"\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nentry: extra\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F\nentry: \"name\",\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F,\n.endfunction\n",
            ".module \"\"\n.section .text\n.function F, 1\n.endfunction\n",
            ".module \"\"\n.section .text\n.function \"sum\"\n.endfunction\n",
            ".module \"\"\n.section .text\nF: .function \"sum\"\n.endfunction\n",
            ".module \"\"\n.section .text\n.function sum\nentry: .block \"entry\"\n.endfunction\n",
            ".module \"\"\n.section .rodata\nC: .constant \"message\", 1\n.endconstant\n",
            ".module \"\"\n.section .data\nG: .global \"buffer\", 16, 8\n.endglobal\n",
        };
        for (const auto Text : InvalidTexts)
        {
            SCOPED_TRACE(Text);
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
            ASSERT_TRUE(Parsed.error().Span.isValid());
            EXPECT_LE(Parsed.error().Span.getBegin().getByteOffset(), Text.size());
        }
    }

    // Forward, backward, and self branches resolve within their function without changing source instruction order.
    TEST(VMMachineModuleTextTest, ResolvesBlockTargetsWithoutChangingInstructionOrder)
    {
        constexpr std::string_view Text = R"(.module "control-flow"
.section .text
.function sum
entry:
    store32i [bp], 20
    add32mmi [bp + 4], [bp], 20
    jmp done
    add32mmi [bp + 4], [bp], 22
    jmp bp
bp:
    add64rri sp, sp, 0
    jmp entry
    jmp .frame
.frame:
    jmp repeated
repeated:
    jmp repeated
done:
    ret
.endfunction
.function another
entry:
    ret
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto& Function = *(*Parsed)->functions().begin();
        ASSERT_EQ(Function.basicBlockCount(), 5U);
        const auto Blocks = Function.basicBlocks();
        const auto& Entry = *Blocks.begin();
        const auto& BP = *std::next(Blocks.begin());
        const auto& Frame = *std::next(Blocks.begin(), 2);
        const auto& Repeated = *std::next(Blocks.begin(), 3);
        const auto& Done = *std::next(Blocks.begin(), 4);
        const auto EntryInstructions = Entry.instructions();
        ASSERT_EQ(Entry.instructionCount(), 5U);
        EXPECT_EQ((*EntryInstructions.begin()).getOpcode(), Opcode::Store32i);
        EXPECT_EQ((*std::next(EntryInstructions.begin())).getOpcode(), Opcode::Add32mmi);
        EXPECT_EQ(static_cast<const BasicBlockOperand&>(*(*std::next(EntryInstructions.begin(), 2)).getOperand(0)).getBlock(), &Done);
        EXPECT_EQ((*std::next(EntryInstructions.begin(), 3)).getOpcode(), Opcode::Add32mmi);
        EXPECT_EQ(static_cast<const BasicBlockOperand&>(*(*std::next(EntryInstructions.begin(), 4)).getOperand(0)).getBlock(), &BP);
        const auto BPInstructions = BP.instructions();
        EXPECT_EQ(static_cast<const BasicBlockOperand&>(*(*std::next(BPInstructions.begin())).getOperand(0)).getBlock(), &Entry);
        EXPECT_EQ(static_cast<const BasicBlockOperand&>(*(*std::next(BPInstructions.begin(), 2)).getOperand(0)).getBlock(), &Frame);
        EXPECT_EQ(static_cast<const BasicBlockOperand&>(*(*Frame.instructions().begin()).getOperand(0)).getBlock(), &Repeated);
        EXPECT_EQ(static_cast<const BasicBlockOperand&>(*(*Repeated.instructions().begin()).getOperand(0)).getBlock(), &Repeated);
        EXPECT_EQ((*Done.instructions().begin()).getOpcode(), Opcode::Ret);
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        const auto Reparsed = readMachineModuleText(*Rewritten);
        ASSERT_TRUE(Reparsed.has_value());
        expectEqualModules(**Parsed, **Reparsed);
    }

    // Direct calls preserve unresolved link names and escaped bytes without requiring declarations during text parsing.
    TEST(VMMachineModuleTextTest, RoundTripsUnresolvedLocalAndImportedCallTargets)
    {
        auto& Names = core::NamePool::instance();
        const auto VMTarget = Names.intern("unresolved.vm");
        std::string NativeName = "native \"bridge\"\\";
        NativeName.push_back('\0');
        NativeName.push_back(static_cast<char>(0xff));
        const auto NativeTarget = Names.intern(NativeName);
        MachineModule Module;
        auto& Block = Module.createFunction(Names.intern("caller")).createBasicBlock(Names.intern("entry"));
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Calli>(ImmediateOperand(VMTarget));
        Builder.create<Opcode::Calli>(ImmediateOperand(NativeTarget));
        Builder.create<Opcode::Calli>(ImmediateOperand(Names.intern("bp")));
        Builder.create<Opcode::Ret>();
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find("call @unresolved.vm\n"), std::string::npos);
        EXPECT_NE(Text->find("call @\"native \\\"bridge\\\"\\\\\\00\\FF\"\n"), std::string::npos);
        EXPECT_NE(Text->find("call @bp\n"), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        EXPECT_TRUE((*Parsed)->symbolDeclarations().empty());
        expectEqualModules(Module, **Parsed);
        const auto Instructions = (*Parsed)->functions().begin()->basicBlocks().begin()->instructions();
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(*Instructions.begin()).getTarget().getSymbol(), VMTarget);
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(*std::next(Instructions.begin())).getTarget().getSymbol(), NativeTarget);
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Text);
    }

    // Shared call spellings select immediate or memory opcodes without losing signed offsets, full-width targets or 64-bit addresses.
    TEST(VMMachineModuleTextTest, RoundTripsImmediateAndMemoryCallForms)
    {
        constexpr std::string_view Text = R"(.module "call-forms"
.section .text
.function caller
entry:
    call @vm.target
    call 16
    call -9223372036854775808
    call 0xffffffffffffffff
    call [sp]
    call [bp + 8]
    call [@pointer_slot + 9223372036854775807]
    call @native.target
    call 0
    call 4294967296
    call [sp]
    call [bp + 4294967296]
    call [@pointer_slot - 9223372036854775808]
    ret
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto& Block = *(*Parsed)->functions().begin()->basicBlocks().begin();
        constexpr Opcode ExpectedOpcodes[] = {
            Opcode::Calli,
            Opcode::Calli,
            Opcode::Calli,
            Opcode::Calli,
            Opcode::Callm,
            Opcode::Callm,
            Opcode::Callm,
            Opcode::Calli,
            Opcode::Calli,
            Opcode::Calli,
            Opcode::Callm,
            Opcode::Callm,
            Opcode::Callm,
            Opcode::Ret,
        };
        ASSERT_EQ(Block.instructionCount(), std::size(ExpectedOpcodes));
        auto Position = Block.instructions().begin();
        for (const auto Expected : ExpectedOpcodes)
        {
            EXPECT_EQ((*Position).getOpcode(), Expected);
            ++Position;
        }
        const auto Instructions = Block.instructions();
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(*std::next(Instructions.begin(), 2)).getTarget().bits(), std::uint64_t{1} << 63);
        EXPECT_EQ(static_cast<const MachineCalliInstruction&>(*std::next(Instructions.begin(), 3)).getTarget().bits(), std::numeric_limits<std::uint64_t>::max());
        EXPECT_EQ(static_cast<const MachineCallmInstruction&>(*std::next(Instructions.begin(), 4)).getTarget().getRegister(), RegisterId::SP);
        EXPECT_EQ(static_cast<const MachineCallmInstruction&>(*std::next(Instructions.begin(), 5)).getTarget().getByteOffset(), 8);
        EXPECT_EQ(static_cast<const MachineCallmInstruction&>(*std::next(Instructions.begin(), 6)).getTarget().getByteOffset(), std::numeric_limits<std::int64_t>::max());
        const auto& FullWidthAddress = static_cast<const MachineCalliInstruction&>(*std::next(Instructions.begin(), 9));
        const auto& IndirectPointer = static_cast<const MachineCallmInstruction&>(*std::next(Instructions.begin(), 11));
        EXPECT_EQ(FullWidthAddress.getTarget().bits(), std::uint64_t{1} << 32);
        EXPECT_EQ(FullWidthAddress.bitWidth(), 64U);
        EXPECT_EQ(IndirectPointer.getTarget().getByteOffset(), 4294967296LL);
        EXPECT_EQ(IndirectPointer.bitWidth(), 64U);
        EXPECT_EQ(static_cast<const MachineCallmInstruction&>(*std::next(Instructions.begin(), 12)).getTarget().getByteOffset(), std::numeric_limits<std::int64_t>::min());
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        const auto Reparsed = readMachineModuleText(*Rewritten);
        ASSERT_TRUE(Reparsed.has_value());
        expectEqualModules(**Parsed, **Reparsed);
        const auto StableText = writeMachineModuleText(**Reparsed);
        ASSERT_TRUE(StableText.has_value());
        EXPECT_EQ(*StableText, *Rewritten);
    }

    // Calls reject registers, nonzero symbol displacements, malformed memory, missing targets and extra operands.
    TEST(VMMachineModuleTextTest, RejectsMalformedCallTargets)
    {
        constexpr std::string_view Mnemonics[] = {
            "call",
        };
        constexpr std::string_view InvalidTargets[] = {
            "",
            "bp",
            "sp",
            "pc",
            "reserved",
            "[reserved]",
            "[ref0]",
            "[bp + 9223372036854775808]",
            "[sp - 9223372036854775809]",
            "[@target +]",
            "@target + 1",
            "@target - 1",
            "@target,",
            "@target, @other",
            "@\"\"",
            "@0",
            "\"target\"",
            "@target trailing",
        };
        for (const auto Mnemonic : Mnemonics)
        {
            for (const auto Target : InvalidTargets)
            {
                const std::string Text = ".module \"\"\n.section .text\n.function caller\nentry:\n    " + std::string(Mnemonic) + " " + std::string(Target) + "\n.endfunction\n";
                SCOPED_TRACE(Text);
                const auto Parsed = readMachineModuleText(Text);
                ASSERT_FALSE(Parsed.has_value());
                EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
                EXPECT_TRUE(Parsed.error().Span.isValid());
            }
        }
    }

    // Invalid function names cannot be written as direct call targets.
    TEST(VMMachineModuleTextTest, RejectsEmptyFunctionSymbolsWhenWritingCalls)
    {
        MachineModule Module;
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Module.createFunction().createBasicBlock());
        Builder.create<Opcode::Calli>(ImmediateOperand(core::NAME_None));
        const auto Text = writeMachineModuleText(Module);
        ASSERT_FALSE(Text.has_value());
        EXPECT_EQ(Text.error().Kind, core::DiagnosticKind::VMTextInvalid);
    }

    // The removed native-only mnemonic is rejected for otherwise valid immediate and memory targets.
    TEST(VMMachineModuleTextTest, RejectsRemovedNativeCallMnemonic)
    {
        constexpr std::string_view Targets[] = {
            "@target",
            "0",
            "4294967296",
            "[sp]",
            "[@slot]",
        };
        for (const auto Target : Targets)
        {
            const std::string Text = ".module \"\"\n.section .text\n.function caller\nentry:\n    callNative " + std::string(Target) + "\n.endfunction\n";
            SCOPED_TRACE(Text);
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
    }

    // Direct symbol targets allow only a zero addend and normalize explicit zero displacements without changing the call form.
    TEST(VMMachineModuleTextTest, NormalizesZeroDirectSymbolDisplacementsAndRejectsNonzeroWrites)
    {
        const auto Parsed = readMachineModuleText(".module \"\"\n.section .text\n.function caller\nentry:\n    call @vm.target + 0\n    call @native.target - 0\n.endfunction\n");
        ASSERT_TRUE(Parsed.has_value());
        const auto Text = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find("call @vm.target\n"), std::string::npos);
        EXPECT_NE(Text->find("call @native.target\n"), std::string::npos);
        const auto Target = core::NamePool::instance().intern("displaced.target");
        for (const std::int64_t Offset : {-1, 1})
        {
            MachineModule Module;
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Module.createFunction().createBasicBlock());
            Builder.create<Opcode::Calli>(ImmediateOperand(Target, Offset));
            const auto Written = writeMachineModuleText(Module);
            ASSERT_FALSE(Written.has_value());
            EXPECT_EQ(Written.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
    }

    // Branch aliases preserve exact target objects even when names are duplicated, anonymous, or reserved for another block.
    TEST(VMMachineModuleTextTest, RoundTripsBranchTargetsWithDuplicateAndAnonymousNames)
    {
        auto& Names = core::NamePool::instance();
        MachineModule Module;
        auto& Function = Module.createFunction(Names.intern("sum"));
        auto& First = Function.createBasicBlock(Names.intern("entry"));
        auto& Second = Function.createBasicBlock(Names.intern("entry"));
        auto& Anonymous = Function.createBasicBlock();
        auto& Reserved = Function.createBasicBlock(Names.intern(".L0"));
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(First);
        Builder.create<Opcode::Jmp>(&Second);
        Builder.create<Opcode::Jmp>(&Anonymous);
        Builder.setInsertPoint(Second);
        Builder.create<Opcode::Jmp>(&First);
        Builder.create<Opcode::Jmp>(&Reserved);
        Builder.setInsertPoint(Anonymous);
        Builder.create<Opcode::Ret>();
        Builder.setInsertPoint(Reserved);
        Builder.create<Opcode::Ret>();
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find("jmp entry.1\n"), std::string::npos);
        EXPECT_NE(Text->find("jmp .L1\n"), std::string::npos);
        EXPECT_NE(Text->find("jmp entry\n"), std::string::npos);
        EXPECT_NE(Text->find("jmp .L0\n"), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Text);
    }

    // Immediate, register and memory jumps retain their distinct operand kinds, signed offsets and unresolved function or slot names.
    TEST(VMMachineModuleTextTest, RoundTripsImmediateRegisterAndMemoryJumpTargets)
    {
        constexpr std::string_view Text = R"(.module "jumps"
.section .text
.function caller
entry:
    jmpi 0
    jmpi -16
    jmpi 0x8000000000000000
    jmpi @function.target + 0
    jmpr pc + 2
    jmpr zero - 8
    jmpm [sp + 16]
    jmpm [@pointer.slot + 8]
    ret
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto Instructions = (*Parsed)->functions().begin()->basicBlocks().begin()->instructions();
        ASSERT_EQ(std::ranges::distance(Instructions), 9);
        EXPECT_EQ(static_cast<const MachineJmpiInstruction&>(*std::next(Instructions.begin(), 1)).getTarget().bits(), static_cast<std::uint64_t>(-16));
        EXPECT_EQ(static_cast<const MachineJmpiInstruction&>(*std::next(Instructions.begin(), 2)).getTarget().bits(), std::uint64_t{1} << 63);
        EXPECT_EQ(static_cast<const MachineJmpiInstruction&>(*std::next(Instructions.begin(), 3)).getTarget().getSymbol(), core::NamePool::instance().intern("function.target"));
        const auto& RegisterJump = static_cast<const MachineJmprInstruction&>(*std::next(Instructions.begin(), 4));
        EXPECT_EQ(RegisterJump.getTarget().getRegister(), RegisterId::PC);
        EXPECT_EQ(RegisterJump.getTarget().getByteOffset(), 2);
        const auto& MemoryJump = static_cast<const MachineJmpmInstruction&>(*std::next(Instructions.begin(), 7));
        EXPECT_EQ(MemoryJump.getTarget().getSymbol(), core::NamePool::instance().intern("pointer.slot"));
        EXPECT_EQ(MemoryJump.getTarget().getByteOffset(), 8);
        const auto Written = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Written.has_value());
        EXPECT_NE(Written->find("jmpi @function.target\n"), std::string::npos);
        EXPECT_NE(Written->find("jmpr pc + 2\n"), std::string::npos);
        EXPECT_NE(Written->find("jmpm [@pointer.slot + 8]\n"), std::string::npos);
        const auto Reparsed = readMachineModuleText(*Written);
        ASSERT_TRUE(Reparsed.has_value());
        expectEqualModules(**Parsed, **Reparsed);
        const auto Rewritten = writeMachineModuleText(**Reparsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Written);
    }

    // Compare-and-branch instructions keep their final target labels distinct from memory and immediate comparison operands.
    TEST(VMMachineModuleTextTest, ResolvesComparisonBranchTargetsFromFinalOperands)
    {
        constexpr std::string_view Text = R"(.module "comparison-targets"
.section .text
.function first
entry:
    je8mmb [bp + 1], [sp + 2], later
    jltu16mib [bp + 3], -1, entry
later:
    jge64mib [zero + 24], 0x8000000000000000, later
.endfunction
.function second
entry:
    ret
.endfunction
)";
        const auto Parsed = readMachineModuleText(Text);
        ASSERT_TRUE(Parsed.has_value());
        const auto& Function = *(*Parsed)->functions().begin();
        const auto& Entry = *Function.basicBlocks().begin();
        const auto& Later = *std::next(Function.basicBlocks().begin());
        const auto Instructions = Entry.instructions();
        const auto& Forward = static_cast<const MachineJe8mmbInstruction&>(*Instructions.begin());
        const auto& Backward = static_cast<const MachineJltu16mibInstruction&>(*std::next(Instructions.begin()));
        const auto& Self = static_cast<const MachineJge64mibInstruction&>(*Later.instructions().begin());
        EXPECT_EQ(Forward.getLHS().getRegister(), RegisterId::BP);
        EXPECT_EQ(Forward.getLHS().getByteOffset(), 1);
        EXPECT_EQ(Forward.getRHS().getRegister(), RegisterId::SP);
        EXPECT_EQ(Forward.getTarget(), &Later);
        EXPECT_EQ(Backward.getRHS().bits(), std::numeric_limits<std::uint64_t>::max());
        EXPECT_EQ(Backward.getTarget(), &Entry);
        EXPECT_EQ(Self.getRHS().bits(), std::uint64_t{1} << 63);
        EXPECT_EQ(Self.getTarget(), &Later);
        const auto Written = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Written.has_value());
        const auto Reparsed = readMachineModuleText(*Written);
        ASSERT_TRUE(Reparsed.has_value());
        expectEqualModules(**Parsed, **Reparsed);
    }

    // Comparison branches reject missing commas, wrong comparison operand kinds, extra operands and targets outside their function.
    TEST(VMMachineModuleTextTest, RejectsMalformedComparisonBranchOperandsAndTargets)
    {
        constexpr std::string_view InvalidBodies[] = {
            "je8mmb",
            "je8mmb [bp], [sp]",
            "je8mmb [bp], 1, entry",
            "je8mmb bp, [sp], entry",
            "je8mib [bp], [sp], entry",
            "je8mib [bp] 1, entry",
            "je8mib [bp], 1 entry",
            "je8mib [bp], 1, @entry",
            "je8mib [bp], 1, 0",
            "je8mib [bp], 1, [pc]",
            "je8mib [bp], 1, entry, entry",
            "je8mib [bp], 1, missing",
        };
        for (const auto Body : InvalidBodies)
        {
            SCOPED_TRACE(Body);
            const auto Text = std::string(".module \"\"\n.section .text\n.function F\nentry:\n    ") + std::string(Body) + "\n.endfunction\n";
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
        constexpr std::string_view CrossFunctionTexts[] = {
            ".module \"\"\n.section .text\n.function F\nentry:\n    je8mib [bp], 1, target\n.endfunction\n.function G\ntarget:\n    ret\n.endfunction\n",
            ".module \"\"\n.section .text\n.function G\ntarget:\n    ret\n.endfunction\n.function F\nentry:\n    je8mib [bp], 1, target\n.endfunction\n",
        };
        for (const auto Text : CrossFunctionTexts)
        {
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
            ASSERT_TRUE(Parsed.error().Span.isValid());
            EXPECT_EQ(Parsed.error().Span.getBegin().getByteOffset(), Text.find("je8mib [bp], 1, target") + std::string_view("je8mib [bp], 1, ").size());
        }
    }

    // Direct jump symbols reject nonzero displacements when serializing already constructed instructions.
    TEST(VMMachineModuleTextTest, RejectsDisplacedJumpFunctionSymbolsWhenWriting)
    {
        for (const std::int64_t Offset : {-1, 1})
        {
            MachineModule Module;
            MachineInstructionBuilder Builder;
            Builder.setInsertPoint(Module.createFunction().createBasicBlock());
            Builder.create<Opcode::Jmpi>(ImmediateOperand(core::NamePool::instance().intern("jump.target"), Offset));
            const auto Written = writeMachineModuleText(Module);
            ASSERT_FALSE(Written.has_value());
            EXPECT_EQ(Written.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
    }

    // Control-transfer forms require exactly one operand of the encoded kind, and direct function symbols require zero addends.
    TEST(VMMachineModuleTextTest, RejectsInvalidControlFlowOperands)
    {
        constexpr std::string_view InvalidBodies[] = {
            "jmp",
            "jmp 0",
            "jmp [bp]",
            "jmp [sp]",
            "jmp [reserved]",
            "jmp [ref0]",
            "jmp [pc + 9223372036854775808]",
            "jmp [pc - 9223372036854775809]",
            "jmp [pc]",
            "jmp [pc + 1], [pc]",
            "jmp entry, entry",
            "jmp entry,",
            "ret 1",
            "jmpi",
            "jmpi bp",
            "jmpi [bp]",
            "jmpi @target + 1",
            "jmpi @target - 1",
            "jmpi 1, 2",
            "jmpr",
            "jmpr 1",
            "jmpr [bp]",
            "jmpr @target",
            "jmpm",
            "jmpm 1",
            "jmpm bp",
            "jmpm @target",
            "jmp missing",
        };
        for (const auto Body : InvalidBodies)
        {
            SCOPED_TRACE(Body);
            const std::string Text = ".module \"\"\n.section .text\n.function F\nentry:\n    " + std::string(Body) + "\n.endfunction\n";
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
        constexpr std::string_view CrossFunctionTexts[] = {
            ".module \"\"\n.section .text\n.function F\nentry:\n    jmp target\n.endfunction\n.function G\ntarget:\n    ret\n.endfunction\n",
            ".module \"\"\n.section .text\n.function G\ntarget:\n    ret\n.endfunction\n.function F\nentry:\n    jmp target\n.endfunction\n",
        };
        for (const auto Text : CrossFunctionTexts)
        {
            const auto Parsed = readMachineModuleText(Text);
            ASSERT_FALSE(Parsed.has_value());
            ASSERT_TRUE(Parsed.error().Span.isValid());
            EXPECT_EQ(Parsed.error().Span.getBegin().getByteOffset(), Text.find("jmp target") + 4);
        }
    }

    // The first undefined register encoding after R9 is rejected for direct sources and memory bases.
    TEST(VMMachineModuleTextTest, RejectsUndefinedRegisterEncodingWhenWriting)
    {
        MachineModule Module;
        auto& Block = Module.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(Block);
        Builder.create<Opcode::Add64rri>(RegisterOperand(RegisterId::BP), RegisterOperand(static_cast<RegisterId>(0x0E)), ImmediateOperand(0));
        const auto RegisterText = writeMachineModuleText(Module);
        ASSERT_FALSE(RegisterText.has_value());
        EXPECT_EQ(RegisterText.error().Kind, core::DiagnosticKind::VMTextInvalid);
        MachineModule AddressModule;
        auto& AddressBlock = AddressModule.createFunction().createBasicBlock();
        Builder.setInsertPoint(AddressBlock);
        Builder.create<Opcode::Store32i>(AddressOperand(static_cast<RegisterId>(0x0E)), ImmediateOperand(1));
        const auto AddressText = writeMachineModuleText(AddressModule);
        ASSERT_FALSE(AddressText.has_value());
        EXPECT_EQ(AddressText.error().Kind, core::DiagnosticKind::VMTextInvalid);
    }

    // Unresolved and defined symbol declarations retain their names, kinds, and mutability without text parsing deciding which are native.
    TEST(VMMachineModuleTextTest, RoundTripsLinkageAndSymbolDeclarations)
    {
        auto& Names = core::NamePool::instance();
        MachineModule Module(Names.intern("link-input"));
        Module.declareSymbol(Names.intern("other.function"), MachineSymbolKind::Function);
        Module.declareSymbol(Names.intern("other.readonly"), MachineSymbolKind::GlobalVariable, true);
        Module.declareSymbol(Names.intern("other global"), MachineSymbolKind::GlobalVariable);
        Module.declareSymbol(Names.intern("host function"), MachineSymbolKind::Function);
        Module.declareSymbol(Names.intern("host.second"), MachineSymbolKind::Function);
        Module.declareSymbol(Names.intern("host.buffer"), MachineSymbolKind::GlobalVariable);
        Module.declareSymbol(Names.intern("readonly"), MachineSymbolKind::GlobalVariable, true);
        Module.declareSymbol(Names.intern("entry point"), MachineSymbolKind::Function);
        const std::array<std::uint8_t, 2> Bytes{1, 2};
        ASSERT_TRUE(Module.createGlobalVariable(Bytes.size(), 2, Bytes, Names.intern("readonly"), MachineSymbolLinkage::External, true));
        ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, core::NAME_None, MachineSymbolLinkage::Local, true));
        ASSERT_TRUE(Module.createGlobalVariable(8, 4, Bytes, Names.intern("global"), MachineSymbolLinkage::External));
        Module.createFunction(Names.intern("entry point"), MachineSymbolLinkage::External);
        Module.createFunction(Names.intern("entry point"));
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        EXPECT_NE(Text->find(".global readonly, 2, 2 readonly external\n"), std::string::npos);
        EXPECT_NE(Text->find(".global global, 8, 4 external\n"), std::string::npos);
        EXPECT_NE(Text->find(".declare global @\"other global\"\n"), std::string::npos);
        EXPECT_NE(Text->find(".declare global @other.readonly readonly\n"), std::string::npos);
        EXPECT_NE(Text->find(".declare function @\"host function\"\n"), std::string::npos);
        EXPECT_NE(Text->find(".declare function @host.second\n"), std::string::npos);
        EXPECT_NE(Text->find(".declare global @host.buffer\n"), std::string::npos);
        EXPECT_NE(Text->find(".declare global @readonly readonly\n"), std::string::npos);
        EXPECT_NE(Text->find(".declare function @\"entry point\"\n"), std::string::npos);
        EXPECT_EQ(Text->find(".native_"), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
        const auto Rewritten = writeMachineModuleText(**Parsed);
        ASSERT_TRUE(Rewritten.has_value());
        EXPECT_EQ(*Rewritten, *Text);
    }

    // Read-only and writable globals retain one insertion order and independent payloads through serialization.
    TEST(VMMachineModuleTextTest, PreservesInterleavedGlobalMutabilityAndOrder)
    {
        auto& Names = core::NamePool::instance();
        MachineModule Module;
        const std::array<std::uint8_t, 1> Bytes{42};
        ASSERT_TRUE(Module.createGlobalVariable(8, 8, Bytes, Names.intern("first")));
        ASSERT_TRUE(Module.createGlobalVariable(4, 4, Bytes, Names.intern("middle"), MachineSymbolLinkage::Local, true));
        ASSERT_TRUE(Module.createGlobalVariable(16, 16, {}, Names.intern("last")));
        const auto Text = writeMachineModuleText(Module);
        ASSERT_TRUE(Text.has_value());
        const auto First = Text->find(".global first, 8, 8\n");
        const auto Middle = Text->find(".global middle, 4, 4 readonly\n");
        const auto Last = Text->find(".global last, 16, 16\n");
        ASSERT_NE(First, std::string::npos);
        ASSERT_NE(Middle, std::string::npos);
        ASSERT_NE(Last, std::string::npos);
        EXPECT_LT(First, Middle);
        EXPECT_LT(Middle, Last);
        EXPECT_EQ(Text->find(".rodata"), std::string::npos);
        EXPECT_EQ(Text->find(".constant"), std::string::npos);
        const auto Parsed = readMachineModuleText(*Text);
        ASSERT_TRUE(Parsed.has_value());
        expectEqualModules(Module, **Parsed);
    }

    // Explicit local linkage remains compatible with the default, while malformed symbol declarations cannot produce a module.
    TEST(VMMachineModuleTextTest, RejectsMalformedLinkageAndSymbolDeclarations)
    {
        const auto Local = readMachineModuleText(".module \"\"\n.section .text\n.function F local\n.endfunction\n");
        ASSERT_TRUE(Local.has_value());
        EXPECT_EQ((*Local)->functions().begin()->linkage(), MachineSymbolLinkage::Local);
        constexpr std::string_view InvalidBodies[] = {
            ".declare function",
            ".declare unknown @name",
            ".declare constant @name",
            ".declare function @\"\"",
            ".declare function @name readonly",
            ".declare global @name readonly readonly",
            ".declare global @name trailing",
            ".section .text\n.function F invalid\n.endfunction",
            ".section .text\n.function F readonly\n.endfunction",
            ".section .text\n.function F external local\n.endfunction",
            ".section .text\n.function F \"\" external\n.endfunction",
            ".section .text\n.function F, \"\" external\n.endfunction",
            ".section .text\n.function F\n.declare function @name\n.endfunction",
            ".section .rodata\n.constant C \"\", 1 external\n.endconstant",
            ".section .data\n.global G \"\", 0, 1 external\n.endglobal",
            ".section .data\n.global G, 0, 1 weak\n.endglobal",
            ".section .data\n.global G, 0, 1 external readonly\n.endglobal",
            ".section .data\n.global G, 0, 1 readonly readonly\n.endglobal",
        };
        for (const std::string_view Body : InvalidBodies)
        {
            SCOPED_TRACE(Body);
            const auto Parsed = readMachineModuleText(".module \"\"\n" + std::string(Body) + "\n");
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
    }

    // Removed native directives are rejected even when their former two-name declaration syntax is well formed.
    TEST(VMMachineModuleTextTest, RejectsRemovedNativeDeclarationDirectives)
    {
        constexpr std::string_view Directives[] = {
            ".native_function @name, @host",
            ".native_data @name, @host",
        };
        for (const auto Directive : Directives)
        {
            SCOPED_TRACE(Directive);
            const auto Parsed = readMachineModuleText(".module \"\"\n" + std::string(Directive) + "\n");
            ASSERT_FALSE(Parsed.has_value());
            EXPECT_EQ(Parsed.error().Kind, core::DiagnosticKind::VMTextInvalid);
        }
    }

    // Writing rejects invalid symbol kinds, empty declaration names, and anonymous external definitions before losing metadata.
    TEST(VMMachineModuleTextTest, RejectsInvalidLinkMetadataWhenWriting)
    {
        auto& Names = core::NamePool::instance();
        const auto Name = Names.intern("symbol");
        MachineModule Anonymous;
        Anonymous.createFunction(core::NAME_None, MachineSymbolLinkage::External);
        EXPECT_FALSE(writeMachineModuleText(Anonymous).has_value());
        MachineModule InvalidLinkage;
        InvalidLinkage.createFunction(Name, static_cast<MachineSymbolLinkage>(255));
        EXPECT_FALSE(writeMachineModuleText(InvalidLinkage).has_value());
        MachineModule InvalidKind;
        InvalidKind.declareSymbol(Name, static_cast<MachineSymbolKind>(255));
        EXPECT_FALSE(writeMachineModuleText(InvalidKind).has_value());
        MachineModule ReadOnlyFunction;
        ReadOnlyFunction.declareSymbol(Name, MachineSymbolKind::Function, true);
        EXPECT_FALSE(writeMachineModuleText(ReadOnlyFunction).has_value());
        MachineModule EmptyGlobal;
        EmptyGlobal.declareSymbol(core::NAME_None, MachineSymbolKind::GlobalVariable);
        EXPECT_FALSE(writeMachineModuleText(EmptyGlobal).has_value());
        MachineModule EmptyFunction;
        EmptyFunction.declareSymbol(core::NAME_None, MachineSymbolKind::Function);
        EXPECT_FALSE(writeMachineModuleText(EmptyFunction).has_value());
    }

    // Unsupported register values fail serialization explicitly in direct register and memory-address operands.
    TEST(VMMachineModuleTextTest, RejectsUnknownRegisterIdsWhenWriting)
    {
        MachineModule RegisterModule;
        auto& RegisterBlock = RegisterModule.createFunction().createBasicBlock();
        MachineInstructionBuilder Builder;
        Builder.setInsertPoint(RegisterBlock);
        Builder.create<Opcode::Add64rri>(RegisterOperand(static_cast<RegisterId>(255)), RegisterOperand(RegisterId::SP), ImmediateOperand(1));
        const auto RegisterText = writeMachineModuleText(RegisterModule);
        ASSERT_FALSE(RegisterText.has_value());
        EXPECT_EQ(RegisterText.error().Kind, core::DiagnosticKind::VMTextInvalid);
        MachineModule AddressModule;
        auto& AddressBlock = AddressModule.createFunction().createBasicBlock();
        Builder.setInsertPoint(AddressBlock);
        Builder.create<Opcode::Store8i>(AddressOperand(static_cast<RegisterId>(255)), ImmediateOperand(1));
        const auto AddressText = writeMachineModuleText(AddressModule);
        ASSERT_FALSE(AddressText.has_value());
        EXPECT_EQ(AddressText.error().Kind, core::DiagnosticKind::VMTextInvalid);
    }
} // namespace ink::vm::test
