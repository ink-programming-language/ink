#include "ink/ir/analysis/verifier.h"
#include "ink/ir/builder.h"
#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    struct IRBuilderTestContext
    {
        core::CompilationContext Compilation;
        IRContext IR{Compilation};
    };

    // Verifies that IRBuilder constructs every module-level category and emits a valid canonical module without text-parser involvement.
    TEST(IRBuilderTest, BuildsModuleDirectly)
    {
      IRBuilderTestContext Context;
      IRBuilder Builder(Context.IR);
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &PointerSizeType = Context.IR.getType(TypeKind::PointerSize);
      const Type &ConstBytePointerType = Context.IR.getType(TypeKind::ConstBytePointer);

      Builder.setModuleName(Name("application.main"));
      const StructType &PairType = Builder.createStructType("Pair", {StructField("first", &I32Type), StructField("second", &I32Type)});
      const ByteConstantId Message = Builder.addByteConstant("message", "hello");
      GlobalVariable Counter;
      Counter.Name = "counter";
      Counter.ValueType = &I32Type;
      Counter.Mutable = true;
      const GlobalId CounterId = Builder.addGlobal(std::move(Counter));

      Function Write(I32Type);
      Write.Name = "write";
      Write.Kind = FunctionKind::External;
      Write.Convention = CallingConvention::C;
      Write.Parameters.emplace_back(&ConstBytePointerType);
      Write.Parameters.emplace_back(&PointerSizeType);
      const FunctionId WriteId = Builder.addFunction(std::move(Write));

      const FunctionId MainId = Builder.createFunction("main", VoidType);
      const std::optional<BlockId> EntryId = Builder.createBlock(MainId, "entry");
      ASSERT_TRUE(EntryId.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(MainId, *EntryId));

      std::vector<ValueHandle> Arguments;
      Arguments.push_back(Builder.createGlobalAddress(ConstBytePointerType, Message, 0));
      Arguments.emplace_back(Builder.getIntegerConstant(PointerSizeType, 5));
      CallInstruction *Call = Builder.createCall(WriteId, std::move(Arguments));
      ASSERT_NE(Call, nullptr);
      ASSERT_NE(Builder.createReturn(), nullptr);

      Module ModuleValue = Builder.takeModule();
      const VerificationResult Verification = verify(Context.IR, ModuleValue);
      ASSERT_TRUE(Verification.succeeded());
      ASSERT_EQ(ModuleValue.StructTypes.size(), 1U);
      EXPECT_EQ(ModuleValue.StructTypes[0], &PairType);
      ASSERT_EQ(ModuleValue.ByteConstants.size(), 1U);
      EXPECT_EQ(Message.value(), 0U);
      ASSERT_EQ(ModuleValue.Globals.size(), 1U);
      EXPECT_EQ(CounterId.value(), 0U);
      ASSERT_EQ(ModuleValue.Functions.size(), 2U);
      EXPECT_EQ(WriteId.value(), 0U);
      EXPECT_EQ(MainId.value(), 1U);
      EXPECT_EQ(Call->Result, ValueId{0});

      const SerializeResult Serialized = printText(Context.IR, ModuleValue);
      ASSERT_TRUE(Serialized.succeeded());
      const std::string Expected =
          "inkir 1\n"
          "module application.main\n"
          "\n"
          "%Pair = type {first: i32, second: i32}\n"
          "\n"
          "@message = private constant [5 x byte] c\"hello\"\n"
          "\n"
          "@counter = global mutable i32\n"
          "\n"
          "declare extern \"C\" i32 @write(const byte*, ptrsize)\n"
          "\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = call i32 @write(const byte* @message[0], ptrsize 5)\n"
          "  ret void\n"
          "}\n";
      EXPECT_EQ(*Serialized.text(), Expected);

      IRBuilderTestContext ParsedContext;
      const DeserializeResult Parsed = parseText(ParsedContext.IR, Expected);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      EXPECT_TRUE(verify(ParsedContext.IR, *Parsed.module()).succeeded());
    }

    // Verifies that builder-managed SSA allocation starts after parameters and remains consecutive across generic value-instruction insertion.
    TEST(IRBuilderTest, AllocatesConsecutiveSsaValues)
    {
      IRBuilderTestContext Context;
      IRBuilder Builder(Context.IR);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const FunctionId Function = Builder.createFunction("increment", I32Type, {Parameter(&I32Type)});
      const std::optional<BlockId> Block = Builder.createBlock(Function, "entry");
      ASSERT_TRUE(Block.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Function, *Block));

      AddInstruction *Add = Builder.createAdd(Builder.createValueOperand(I32Type, ValueId{0}), Builder.getIntegerConstant(I32Type, 1));
      ASSERT_NE(Add, nullptr);
      ReturnInstruction *Return = Builder.createReturn(Builder.createValueOperand(I32Type, Add->Result));
      ASSERT_NE(Return, nullptr);

      EXPECT_EQ(Add->Result, ValueId{1});
      Module ModuleValue = Builder.takeModule();
      EXPECT_TRUE(verify(Context.IR, ModuleValue).succeeded());
    }

    // Verifies that invalid insertion points fail explicitly and taking a module resets all builder-owned structure and insertion state.
    TEST(IRBuilderTest, RejectsInvalidInsertionPointsAndResets)
    {
      IRBuilderTestContext Context;
      IRBuilder Builder(Context.IR);
      const Type &VoidType = Context.IR.getType(TypeKind::Void);

      EXPECT_FALSE(Builder.setInsertionPoint(FunctionId{0}, BlockId{0}));
      EXPECT_EQ(Builder.createReturn(), nullptr);
      Function FunctionValue(VoidType);
      FunctionValue.Name = "main";
      const FunctionId Function = Builder.addFunction(std::move(FunctionValue));
      BasicBlock Entry;
      Entry.Name = "entry";
      const std::optional<BlockId> Block = Builder.addBlock(Function, std::move(Entry));
      ASSERT_TRUE(Block.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Function, *Block));
      ASSERT_NE(Builder.createReturn(), nullptr);

      Module First = Builder.takeModule();
      EXPECT_EQ(First.Functions.size(), 1U);
      EXPECT_TRUE(Builder.module().Functions.empty());
      EXPECT_FALSE(Builder.insertionFunction().has_value());
      EXPECT_FALSE(Builder.insertionBlock().has_value());
      EXPECT_FALSE(Builder.setInitializer(Function));
      EXPECT_FALSE(Builder.setFinalizer(Function));
    }

    // Verifies that the typed import interface initializes its module operand before insertion and produces a structurally valid function body.
    TEST(IRBuilderTest, CreatesCompleteImportInstruction)
    {
      IRBuilderTestContext Context;
      IRBuilder Builder(Context.IR);
      Builder.setModuleName(Name("application.main"));
      const FunctionId Function = Builder.createFunction("load_dependency", Context.IR.getType(TypeKind::Void));
      const std::optional<BlockId> Block = Builder.createBlock(Function, "entry");
      ASSERT_TRUE(Block.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Function, *Block));

      ImportInstruction *Import = Builder.createImport("library.math");
      ASSERT_NE(Import, nullptr);
      EXPECT_EQ(Import->Module, Name("library.math"));
      ASSERT_NE(Builder.createReturn(), nullptr);

      Module ModuleValue = Builder.takeModule();
      EXPECT_TRUE(verify(Context.IR, ModuleValue).succeeded());
    }

    // Verifies that the centralized instruction table supports reverse mnemonic lookup, result policy queries, and result extraction for every policy category.
    TEST(IRInstructionMetadataTest, DescribesParsingAndResultBehavior)
    {
#define INK_IR_INSTRUCTION(Name, Mnemonic, Terminator, ResultPolicy) EXPECT_EQ(instructionKindFromMnemonic(Mnemonic), InstructionKind::Name); EXPECT_EQ(instructionResultPolicy(InstructionKind::Name), InstructionResultPolicy::ResultPolicy);
#include "ink/ir/ir.def"
      EXPECT_EQ(instructionKindFromMnemonic("missing"), std::nullopt);

      IRBuilderTestContext Context;
      AddInstruction Add(Context.IR.getType(TypeKind::I32));
      Add.Result = ValueId{7};
      CallInstruction Call(Context.IR.getType(TypeKind::Void));
      ReturnInstruction Return;
      EXPECT_EQ(instructionResultId(Add), ValueId{7});
      EXPECT_EQ(instructionResultId(Call), std::nullopt);
      EXPECT_EQ(instructionResultId(Return), std::nullopt);
    }
  } // namespace
} // namespace ink::ir
