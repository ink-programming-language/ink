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
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) Opcode::Kind,
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
    };

    template <typename OperandType>
    OperandType exampleOperand(std::size_t Index)
    {
      if constexpr (std::is_same_v<OperandType, RegisterOperand>)
      {
        constexpr std::array Registers{RegisterId::BP, RegisterId::SP, RegisterId::PC};
        return RegisterOperand(Registers[Index]);
      }
      else if constexpr (std::is_same_v<OperandType, AddressOperand>)
      {
        return Index == 1 ? AddressOperand(std::numeric_limits<std::uint32_t>::max(), std::numeric_limits<std::int64_t>::max()) : AddressOperand(RegisterId::BP, std::numeric_limits<std::int64_t>::min());
      }
      else
      {
        return ImmediateOperand(std::numeric_limits<std::uint64_t>::max() - Index);
      }
    }

    template <Opcode Operation, std::size_t... Indices>
    void appendExampleInstruction(MachineInstructionBuilder &Builder, const MachineBasicBlock *Target, std::index_sequence<Indices...>)
    {
      using Operands = typename OpcodeTraits<Operation>::OperandTuple;
      if constexpr (OpcodeTraits<Operation>::Category == InstructionCategory::Branch)
      {
        Builder.create<Operation>(Target);
      }
      else
      {
        Builder.create<Operation>(exampleOperand<std::tuple_element_t<Indices, Operands>>(Indices)...);
      }
    }

    void expectEqualOperands(const Operand &Expected, const Operand &Actual, const std::unordered_map<const MachineBasicBlock *, const MachineBasicBlock *> &BlockMap)
    {
      ASSERT_EQ(Actual.getKind(), Expected.getKind());
      switch (Expected.getKind())
      {
      case OperandKind::Register:
        EXPECT_EQ(static_cast<const RegisterOperand &>(Actual).getRegister(), static_cast<const RegisterOperand &>(Expected).getRegister());
        break;
      case OperandKind::Address:
      {
        const auto &ExpectedAddress = static_cast<const AddressOperand &>(Expected);
        const auto &ActualAddress = static_cast<const AddressOperand &>(Actual);
        EXPECT_EQ(ActualAddress.getBaseKind(), ExpectedAddress.getBaseKind());
        EXPECT_EQ(ActualAddress.getRegister(), ExpectedAddress.getRegister());
        EXPECT_EQ(ActualAddress.getBaseReferenceIndex(), ExpectedAddress.getBaseReferenceIndex());
        EXPECT_EQ(ActualAddress.getByteOffset(), ExpectedAddress.getByteOffset());
        break;
      }
      case OperandKind::Immediate:
        EXPECT_EQ(static_cast<const ImmediateOperand &>(Actual).bits(), static_cast<const ImmediateOperand &>(Expected).bits());
        break;
      case OperandKind::BasicBlock:
      {
        const auto Target = BlockMap.find(static_cast<const BasicBlockOperand &>(Expected).getBlock());
        ASSERT_NE(Target, BlockMap.end());
        EXPECT_EQ(static_cast<const BasicBlockOperand &>(Actual).getBlock(), Target->second);
        break;
      }
      }
    }

    void expectEqualModules(const MachineModule &Expected, const MachineModule &Actual)
    {
      EXPECT_EQ(Actual.name(), Expected.name());
      ASSERT_EQ(Actual.constantCount(), Expected.constantCount());
      auto ActualConstant = Actual.constants().begin();
      for (const auto &ExpectedConstant : Expected.constants())
      {
        EXPECT_EQ(ActualConstant->name(), ExpectedConstant.name());
        EXPECT_EQ(ActualConstant->alignment(), ExpectedConstant.alignment());
        EXPECT_TRUE(std::ranges::equal(ActualConstant->bytes(), ExpectedConstant.bytes()));
        EXPECT_EQ(ActualConstant->parent(), &Actual);
        ++ActualConstant;
      }
      ASSERT_EQ(Actual.globalVariableCount(), Expected.globalVariableCount());
      auto ActualGlobal = Actual.globalVariables().begin();
      for (const auto &ExpectedGlobal : Expected.globalVariables())
      {
        EXPECT_EQ(ActualGlobal->name(), ExpectedGlobal.name());
        EXPECT_EQ(ActualGlobal->byteSize(), ExpectedGlobal.byteSize());
        EXPECT_EQ(ActualGlobal->alignment(), ExpectedGlobal.alignment());
        EXPECT_TRUE(std::ranges::equal(ActualGlobal->initialBytes(), ExpectedGlobal.initialBytes()));
        EXPECT_EQ(ActualGlobal->parent(), &Actual);
        ++ActualGlobal;
      }
      ASSERT_EQ(Actual.functionCount(), Expected.functionCount());
      auto ActualFunction = Actual.functions().begin();
      for (const auto &ExpectedFunction : Expected.functions())
      {
        EXPECT_EQ(ActualFunction->name(), ExpectedFunction.name());
        EXPECT_EQ(ActualFunction->parent(), &Actual);
        const auto &ExpectedFrame = ExpectedFunction.frameInfo();
        const auto &ActualFrame = ActualFunction->frameInfo();
        EXPECT_EQ(ActualFrame.alignment(), ExpectedFrame.alignment());
        EXPECT_EQ(ActualFrame.baseOffset(), ExpectedFrame.baseOffset());
        EXPECT_EQ(ActualFrame.byteSize(), ExpectedFrame.byteSize());
        ASSERT_EQ(ActualFrame.stackSlotCount(), ExpectedFrame.stackSlotCount());
        auto ActualSlot = ActualFrame.stackSlots().begin();
        for (const auto &ExpectedSlot : ExpectedFrame.stackSlots())
        {
          EXPECT_EQ(ActualSlot->byteSize(), ExpectedSlot.byteSize());
          EXPECT_EQ(ActualSlot->alignment(), ExpectedSlot.alignment());
          EXPECT_EQ(ActualSlot->byteOffset(), ExpectedSlot.byteOffset());
          ++ActualSlot;
        }
        ASSERT_EQ(ActualFunction->basicBlockCount(), ExpectedFunction.basicBlockCount());
        std::unordered_map<const MachineBasicBlock *, const MachineBasicBlock *> BlockMap;
        auto MappedBlock = ActualFunction->basicBlocks().begin();
        for (const auto &ExpectedBlock : ExpectedFunction.basicBlocks())
        {
          BlockMap.emplace(&ExpectedBlock, &*MappedBlock++);
        }
        auto ActualBlock = ActualFunction->basicBlocks().begin();
        for (const auto &ExpectedBlock : ExpectedFunction.basicBlocks())
        {
          EXPECT_EQ(ActualBlock->name(), ExpectedBlock.name());
          EXPECT_EQ(ActualBlock->parent(), &*ActualFunction);
          ASSERT_EQ(ActualBlock->instructionCount(), ExpectedBlock.instructionCount());
          auto ActualInstructions = ActualBlock->instructions();
          auto ActualInstruction = ActualInstructions.begin();
          for (const auto &ExpectedInstruction : ExpectedBlock.instructions())
          {
            const auto &ActualValue = *ActualInstruction;
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

  // Every opcode retains its exact ordered operand types, signed offsets, reference indices, and raw immediate bits.
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
    ASSERT_TRUE(Module.createConstant(Bytes, 16, SharedName).has_value());
    ASSERT_TRUE(Module.createConstant({}, 1, SharedName).has_value());
    ASSERT_TRUE(Module.createGlobalVariable(std::numeric_limits<std::uint64_t>::max(), 8, Bytes, SharedName).has_value());
    ASSERT_TRUE(Module.createGlobalVariable(0).has_value());
    auto &Function = Module.createFunction(SharedName);
    ASSERT_TRUE(Function.frameInfo().createStackSlot(4, 4).has_value());
    ASSERT_TRUE(Function.frameInfo().createStackSlot(8, 8).has_value());
    ASSERT_TRUE(Function.frameInfo().layout(17).has_value());
    auto &Block = Function.createBasicBlock(SharedName);
    auto &Target = Function.createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Condition) appendExampleInstruction<Opcode::Kind>(Builder, &Target, std::make_index_sequence<OpcodeTraits<Opcode::Kind>::OperandCount>{});
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
    auto &UnlaidFunction = Module.createFunction(SharedName);
    ASSERT_TRUE(UnlaidFunction.frameInfo().createStackSlot(7, 16).has_value());
    auto &EmptyLaidFunction = Module.createFunction();
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
    auto &Names = core::NamePool::instance();
    MachineModule Module;
    ASSERT_TRUE(Module.createConstant({}, 1, Names.intern("message")).has_value());
    ASSERT_TRUE(Module.createGlobalVariable(16, 8, {}, Names.intern("scratch")).has_value());
    auto &Sum = Module.createFunction(Names.intern("sum"));
    Sum.createBasicBlock(Names.intern("entry"));
    Sum.createBasicBlock(Names.intern("sum"));
    Module.createFunction(Names.intern("another")).createBasicBlock(Names.intern("entry"));
    const auto Text = writeMachineModuleText(Module);
    ASSERT_TRUE(Text.has_value());
    EXPECT_NE(Text->find(".constant message, 1\n"), std::string::npos);
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
    auto &Names = core::NamePool::instance();
    MachineModule Module;
    ASSERT_TRUE(Module.createConstant({}).has_value());
    ASSERT_TRUE(Module.createConstant({}, 1, Names.intern(".C0")).has_value());
    ASSERT_TRUE(Module.createGlobalVariable(0).has_value());
    ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, Names.intern(".G0")).has_value());
    Module.createFunction();
    Module.createFunction(Names.intern(".F0"));
    auto &Sum = Module.createFunction(Names.intern("sum"));
    Sum.createBasicBlock(Names.intern("entry"));
    Sum.createBasicBlock(Names.intern("entry"));
    Sum.createBasicBlock(Names.intern("entry.1"));
    Sum.createBasicBlock();
    Sum.createBasicBlock(Names.intern(".L0"));
    Module.createFunction(Names.intern("sum"));
    Module.createFunction(Names.intern("sum.1"));
    const std::string UnusualName("space name\0\xff", 12);
    const auto Unusual = Names.intern(UnusualName);
    ASSERT_TRUE(Module.createConstant({}, 1, Unusual).has_value());
    ASSERT_TRUE(Module.createGlobalVariable(0, 1, {}, Unusual).has_value());
    Module.createFunction(Unusual).createBasicBlock(Unusual);
    const auto Text = writeMachineModuleText(Module);
    ASSERT_TRUE(Text.has_value());
    EXPECT_NE(Text->find(".constant .C1, \"\", 1\n"), std::string::npos);
    EXPECT_NE(Text->find(".constant .C0, 1\n"), std::string::npos);
    EXPECT_NE(Text->find(".global .G1, \"\", 0, 1\n"), std::string::npos);
    EXPECT_NE(Text->find(".global .G0, 0, 1\n"), std::string::npos);
    EXPECT_NE(Text->find(".function .F1, \"\"\n"), std::string::npos);
    EXPECT_NE(Text->find(".function .F0\n"), std::string::npos);
    EXPECT_NE(Text->find(".function sum.2, \"sum\"\n"), std::string::npos);
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
.section .rodata
.constant C, "shared", 1
.endconstant
.section .data
.global G, "shared", 8, 8
.endglobal
.section .text
.function F, "shared"
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
    const auto &Module = **Parsed;
    auto &Names = core::NamePool::instance();
    EXPECT_EQ(Module.constants().begin()->name(), Names.intern("shared"));
    EXPECT_EQ(Module.globalVariables().begin()->name(), Names.intern("shared"));
    const auto &Function = *Module.functions().begin();
    EXPECT_EQ(Function.name(), Names.intern("shared"));
    ASSERT_EQ(Function.basicBlockCount(), 3U);
    const auto Blocks = Function.basicBlocks();
    EXPECT_EQ(Blocks.begin()->name(), Names.intern("entry"));
    EXPECT_EQ(std::next(Blocks.begin())->name(), Names.intern("entry"));
    EXPECT_EQ(std::next(Blocks.begin(), 2)->name(), core::NAME_None);
    EXPECT_EQ(std::next(Module.functions().begin())->basicBlocks().begin()->name(), Names.intern("entry"));
  }

  // Handwritten Intel-style text accepts comments, CRLF, byte escapes, signed immediates, and explicit symbol-reference indices.
  TEST(VMMachineModuleTextTest, ReadsAssemblySyntaxAndOwnsParsedPayloads)
  {
    std::string Text = R"(.module "demo;name" ; a semicolon within the string remains part of its name
.section .rodata
.constant message, "A\00\FF\"\\", 4
    .byte 0x48, 105
.endconstant
.section .data
.global buffer, "", 64, 8
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
    add64rrm bp, pc, [ref4294967295 - 9223372036854775808]
.endfunction
)";
    for (std::size_t Position = 0; (Position = Text.find('\n', Position)) != std::string::npos; Position += 2)
    {
      Text.insert(Position, 1, '\r');
    }
    auto Parsed = readMachineModuleText(Text);
    ASSERT_TRUE(Parsed.has_value());
    Text.assign(Text.size(), 'x');
    const auto &Module = **Parsed;
    EXPECT_EQ(core::NamePool::instance().text(Module.name()), "demo;name");
    ASSERT_EQ(Module.constantCount(), 1U);
    const auto &Constant = *Module.constants().begin();
    const std::array<std::uint8_t, 2> ExpectedBytes{0x48, 105};
    EXPECT_TRUE(std::ranges::equal(Constant.bytes(), ExpectedBytes));
    const std::string ExpectedName("A\0\xff\"\\", 5);
    EXPECT_EQ(core::NamePool::instance().text(Constant.name()), ExpectedName);
    const auto &Function = *Module.functions().begin();
    EXPECT_EQ(Function.frameInfo().baseOffset(), 16U);
    EXPECT_EQ(Function.frameInfo().byteSize(), 16U);
    auto Instructions = Function.basicBlocks().begin()->instructions();
    const auto &Add = *std::next(Instructions.begin());
    EXPECT_EQ(static_cast<const ImmediateOperand &>(*Add.getOperand(2)).bits(), std::uint64_t(0) - 16);
    const auto &Address = static_cast<const AddressOperand &>(*(*std::next(Instructions.begin(), 2)).getOperand(2));
    EXPECT_EQ(Address.getBaseReferenceIndex(), std::numeric_limits<std::uint32_t>::max());
    EXPECT_EQ(Address.getByteOffset(), std::numeric_limits<std::int64_t>::min());
  }

  // Numeric spelling variants preserve boundary bit patterns, and the final function terminator needs no newline.
  TEST(VMMachineModuleTextTest, AcceptsBoundaryImmediatesAndNumericSpellings)
  {
    constexpr std::string_view Text = R"(.module ""
.section .text
.function F, ""
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
    for (const auto &Instruction : Instructions)
    {
      EXPECT_EQ(static_cast<const ImmediateOperand &>(*Instruction.getOperand(1)).bits(), ExpectedBits[Index]);
      ++Index;
    }
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
        ".module \"\"\n.section .rodata\n.constant C, 3\n.endconstant\n",
        ".module \"\"\n.section .rodata\n.constant C, 1\n.byte 256\n.endconstant\n",
        ".module \"\"\n.section .rodata\n.constant C, 1\n.byte\n.endconstant\n",
        ".module \"\"\n.section .rodata\n.constant C, 1\n.byte 1,\n.endconstant\n",
        ".module \"\"\n.section .rodata\n.constant C, 1\n.section .data\n.endconstant\n",
        ".module \"\"\n.section .rodata\n.constant C, 1\n.endglobal\n",
        ".module \"\"\n.section .rodata\n.constant C, 1\n",
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
        ".module \"\"\n.section .rodata\n.constant shared, 1\n.endconstant\n.section .data\n.global shared, 0, 1\n.endglobal\n",
        ".module \"\"\n.section .rodata\n.constant shared, 1\n.endconstant\n.section .text\n.function shared\n.endfunction\n",
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
    cmp32mi [bp], 20
    je done
    add32mmi [bp + 4], [bp], 22
    jmp bp
bp:
    cmp64ri sp, 0
    jne entry
    jmp .frame
.frame:
    jmp eflags
eflags:
    jmp eflags
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
    const auto &Function = *(*Parsed)->functions().begin();
    ASSERT_EQ(Function.basicBlockCount(), 5U);
    const auto Blocks = Function.basicBlocks();
    const auto &Entry = *Blocks.begin();
    const auto &BP = *std::next(Blocks.begin());
    const auto &Frame = *std::next(Blocks.begin(), 2);
    const auto &Flags = *std::next(Blocks.begin(), 3);
    const auto &Done = *std::next(Blocks.begin(), 4);
    const auto EntryInstructions = Entry.instructions();
    ASSERT_EQ(Entry.instructionCount(), 5U);
    EXPECT_EQ((*EntryInstructions.begin()).getOpcode(), Opcode::Store32i);
    EXPECT_EQ((*std::next(EntryInstructions.begin())).getOpcode(), Opcode::Cmp32mi);
    EXPECT_EQ(static_cast<const BasicBlockOperand &>(*(*std::next(EntryInstructions.begin(), 2)).getOperand(0)).getBlock(), &Done);
    EXPECT_EQ((*std::next(EntryInstructions.begin(), 3)).getOpcode(), Opcode::Add32mmi);
    EXPECT_EQ(static_cast<const BasicBlockOperand &>(*(*std::next(EntryInstructions.begin(), 4)).getOperand(0)).getBlock(), &BP);
    const auto BPInstructions = BP.instructions();
    EXPECT_EQ(static_cast<const BasicBlockOperand &>(*(*std::next(BPInstructions.begin())).getOperand(0)).getBlock(), &Entry);
    EXPECT_EQ(static_cast<const BasicBlockOperand &>(*(*std::next(BPInstructions.begin(), 2)).getOperand(0)).getBlock(), &Frame);
    EXPECT_EQ(static_cast<const BasicBlockOperand &>(*(*Frame.instructions().begin()).getOperand(0)).getBlock(), &Flags);
    EXPECT_EQ(static_cast<const BasicBlockOperand &>(*(*Flags.instructions().begin()).getOperand(0)).getBlock(), &Flags);
    EXPECT_EQ((*Done.instructions().begin()).getOpcode(), Opcode::Ret);
    const auto Rewritten = writeMachineModuleText(**Parsed);
    ASSERT_TRUE(Rewritten.has_value());
    const auto Reparsed = readMachineModuleText(*Rewritten);
    ASSERT_TRUE(Reparsed.has_value());
    expectEqualModules(**Parsed, **Reparsed);
  }

  // Branch aliases preserve exact target objects even when names are duplicated, anonymous, or reserved for another block.
  TEST(VMMachineModuleTextTest, RoundTripsBranchTargetsWithDuplicateAndAnonymousNames)
  {
    auto &Names = core::NamePool::instance();
    MachineModule Module;
    auto &Function = Module.createFunction(Names.intern("sum"));
    auto &First = Function.createBasicBlock(Names.intern("entry"));
    auto &Second = Function.createBasicBlock(Names.intern("entry"));
    auto &Anonymous = Function.createBasicBlock();
    auto &Reserved = Function.createBasicBlock(Names.intern(".L0"));
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(First);
    Builder.create<Opcode::Je>(&Second);
    Builder.create<Opcode::Jmp>(&Anonymous);
    Builder.setInsertPoint(Second);
    Builder.create<Opcode::Jne>(&First);
    Builder.create<Opcode::Jmp>(&Reserved);
    Builder.setInsertPoint(Anonymous);
    Builder.create<Opcode::Ret>();
    Builder.setInsertPoint(Reserved);
    Builder.create<Opcode::Ret>();
    const auto Text = writeMachineModuleText(Module);
    ASSERT_TRUE(Text.has_value());
    EXPECT_NE(Text->find("je entry.1\n"), std::string::npos);
    EXPECT_NE(Text->find("jmp .L1\n"), std::string::npos);
    EXPECT_NE(Text->find("jne entry\n"), std::string::npos);
    EXPECT_NE(Text->find("jmp .L0\n"), std::string::npos);
    const auto Parsed = readMachineModuleText(*Text);
    ASSERT_TRUE(Parsed.has_value());
    expectEqualModules(Module, **Parsed);
    const auto Rewritten = writeMachineModuleText(**Parsed);
    ASSERT_TRUE(Rewritten.has_value());
    EXPECT_EQ(*Rewritten, *Text);
  }

  // Control-transfer arity, function-local target labels, comparison operand kinds, and implicit EFLAGS access are checked before returning a module.
  TEST(VMMachineModuleTextTest, RejectsInvalidControlFlowAndExplicitEFlagsOperands)
  {
    constexpr std::string_view InvalidBodies[] = {
        "jmp",
        "je",
        "jmp 0",
        "jmp [bp]",
        "jmp [sp]",
        "jmp [eflags]",
        "jmp [ref0]",
        "je [bp + 4]",
        "jmp [pc + 9223372036854775808]",
        "jmp [pc - 9223372036854775809]",
        "jmp [pc]",
        "jmp [pc + 1], [pc]",
        "jmp entry, entry",
        "je entry,",
        "ret 1",
        "cmp32mi [bp]",
        "cmp32mi [bp], 1, 2",
        "cmp32mi bp, 1",
        "cmp64rr bp, [sp]",
        "add64rri eflags, sp, 1",
        "add64rri sp, eflags, 1",
        "cmp64ri eflags, 0",
        "store32i [eflags], 1",
        "cmp32mi [eflags + 4], 0",
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

  // Explicit EFLAGS registers are rejected when writing either direct register values or memory bases.
  TEST(VMMachineModuleTextTest, RejectsExplicitEFlagsWhenWriting)
  {
    MachineModule Module;
    auto &Block = Module.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(Block);
    Builder.create<Opcode::Cmp64ri>(RegisterOperand(RegisterId::EFLAGS), ImmediateOperand(0));
    const auto RegisterText = writeMachineModuleText(Module);
    ASSERT_FALSE(RegisterText.has_value());
    EXPECT_EQ(RegisterText.error().Kind, core::DiagnosticKind::VMTextInvalid);
    MachineModule AddressModule;
    auto &AddressBlock = AddressModule.createFunction().createBasicBlock();
    Builder.setInsertPoint(AddressBlock);
    Builder.create<Opcode::Store32i>(AddressOperand(RegisterId::EFLAGS), ImmediateOperand(1));
    const auto AddressText = writeMachineModuleText(AddressModule);
    ASSERT_FALSE(AddressText.has_value());
    EXPECT_EQ(AddressText.error().Kind, core::DiagnosticKind::VMTextInvalid);
  }

  // Unsupported register values fail serialization explicitly in direct register and memory-address operands.
  TEST(VMMachineModuleTextTest, RejectsUnknownRegisterIdsWhenWriting)
  {
    MachineModule RegisterModule;
    auto &RegisterBlock = RegisterModule.createFunction().createBasicBlock();
    MachineInstructionBuilder Builder;
    Builder.setInsertPoint(RegisterBlock);
    Builder.create<Opcode::Add64rri>(RegisterOperand(static_cast<RegisterId>(255)), RegisterOperand(RegisterId::SP), ImmediateOperand(1));
    const auto RegisterText = writeMachineModuleText(RegisterModule);
    ASSERT_FALSE(RegisterText.has_value());
    EXPECT_EQ(RegisterText.error().Kind, core::DiagnosticKind::VMTextInvalid);
    MachineModule AddressModule;
    auto &AddressBlock = AddressModule.createFunction().createBasicBlock();
    Builder.setInsertPoint(AddressBlock);
    Builder.create<Opcode::Store8i>(AddressOperand(static_cast<RegisterId>(255)), ImmediateOperand(1));
    const auto AddressText = writeMachineModuleText(AddressModule);
    ASSERT_FALSE(AddressText.has_value());
    EXPECT_EQ(AddressText.error().Kind, core::DiagnosticKind::VMTextInvalid);
  }
} // namespace ink::vm::test
