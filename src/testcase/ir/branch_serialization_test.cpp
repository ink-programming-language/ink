#include "ink/ir/module/module_serialization.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>
#include <llvm/Support/Endian.h>

#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr std::string_view BranchModuleText = R"(ink-ir 4
module @Branches {
  define i32 @choose(bool %flag) {
  entry:
    br bool %flag, ^then, ^otherwise
  then:
    br ^merge
  otherwise:
    br ^entry
  merge:
    ret i32 1
  }
  define void @other() {
  other_entry:
    ret void
  }
}
)";
  } // namespace

  // Forward branch targets, a back edge, and the condition retain identity in both archive formats.
  TEST(IRModuleSerializationTest, BranchesRoundTripWithForwardTargetsAndBackEdges)
  {
    core::CompilationContext Compilation;
    IRContext Source(Compilation);
    const auto Parsed = deserializeModuleText(Source, BranchModuleText);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    for (bool Binary : {false, true})
    {
      const auto Encoded = Binary ? serializeModuleBinary(*Parsed.ModuleValue) : serializeModuleText(*Parsed.ModuleValue);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      IRContext Destination(Compilation);
      const auto Decoded = Binary ? deserializeModuleBinary(Destination, Encoded.Bytes) : deserializeModuleText(Destination, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      const auto &FunctionValue = static_cast<const Function &>(*Decoded.ModuleValue->entryBlock().values()[0]);
      ASSERT_EQ(FunctionValue.blocks().size(), 4U);
      const auto &Blocks = FunctionValue.blocks();
      ASSERT_TRUE(ConditionalBranchInstruction::classof(Blocks[0]->terminator()));
      const auto &Conditional = static_cast<const ConditionalBranchInstruction &>(*Blocks[0]->terminator());
      EXPECT_EQ(&Conditional.condition(), FunctionValue.parameters()[0].get());
      EXPECT_EQ(&Conditional.trueTarget(), Blocks[1].get());
      EXPECT_EQ(&Conditional.falseTarget(), Blocks[2].get());
      ASSERT_TRUE(BranchInstruction::classof(Blocks[1]->terminator()));
      EXPECT_EQ(&static_cast<const BranchInstruction &>(*Blocks[1]->terminator()).target(), Blocks[3].get());
      ASSERT_TRUE(BranchInstruction::classof(Blocks[2]->terminator()));
      EXPECT_EQ(&static_cast<const BranchInstruction &>(*Blocks[2]->terminator()).target(), Blocks[0].get());
      const auto Reencoded = Binary ? serializeModuleBinary(*Decoded.ModuleValue) : serializeModuleText(*Decoded.ModuleValue);
      ASSERT_TRUE(Reencoded.succeeded()) << Reencoded.Message;
      EXPECT_EQ(Reencoded.Bytes, Encoded.Bytes);
    }
  }

  // A conditional self-loop with duplicate destinations is legal and does not create an operand dependency cycle.
  TEST(IRModuleSerializationTest, BranchesPreserveSelfLoopsAndIdenticalTargets)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    auto *Root = Builder.createModule(Context.namePool().intern("Loop"));
    auto FunctionOwner = Builder.createFunction(Context.namePool().intern("loop"), *Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>()));
    auto *FunctionValue = FunctionOwner.get();
    ASSERT_NE(FunctionValue, nullptr);
    ASSERT_TRUE(Builder.appendValue(Root->entryBlock(), std::move(FunctionOwner)));
    auto *Block = Builder.createBasicBlock(*FunctionValue);
    ASSERT_NE(Block, nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    ASSERT_NE(Builder.createConditionalBranchInstruction(Context.constantPool().getBoolConstant(false), *Block, *Block), nullptr);
    for (bool Binary : {false, true})
    {
      const auto Encoded = Binary ? serializeModuleBinary(*Root) : serializeModuleText(*Root);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      IRContext Destination(Compilation);
      const auto Decoded = Binary ? deserializeModuleBinary(Destination, Encoded.Bytes) : deserializeModuleText(Destination, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      const auto &RestoredFunction = static_cast<const Function &>(*Decoded.ModuleValue->entryBlock().values()[0]);
      const auto *RestoredBlock = RestoredFunction.blocks()[0].get();
      const auto &Branch = static_cast<const ConditionalBranchInstruction &>(*RestoredBlock->terminator());
      EXPECT_EQ(&Branch.trueTarget(), RestoredBlock);
      EXPECT_EQ(&Branch.falseTarget(), RestoredBlock);
      ASSERT_TRUE(BoolConstant::classof(&Branch.condition()));
      EXPECT_FALSE(static_cast<const BoolConstant &>(Branch.condition()).value());
    }
  }

  // Invalid condition types, targets, source blocks and values following terminators never publish a module.
  TEST(IRModuleSerializationTest, RejectsMalformedTextBranches)
  {
    const std::string_view Bodies[] = {
        "define void @f() { entry: br ^missing }",
        "define void @f() { entry: br i32 1, ^exit, ^exit exit: ret void }",
        "define void @f(i32 %flag) { entry: br bool %flag, ^exit, ^exit exit: ret void }",
        "define void @f() { entry: br ^exit ret void exit: ret void }",
        "define void @f() { entry: br bool true, ^exit, ^exit ret void exit: ret void }",
        "define void @f() { entry: br ^other } define void @g() { other: ret void }",
        "define void @f() { entry: br bool true, ^entry, ^other } define void @g() { other: ret void }",
        "block ^outer { br ^outer }",
        "define void @f() { entry: block ^inner { br ^entry } ret void }",
        "define void @f() { entry: br bool true, ^entry }",
    };
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    for (const auto Body : Bodies)
    {
      const auto Result = deserializeModuleText(Context, "ink-ir 4 module @Invalid { " + std::string(Body) + " }");
      EXPECT_EQ(Result.Status, ModuleArchiveStatus::InvalidArchive) << Body << ": " << Result.Message;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_TRUE(Context.modules().empty());
    }
  }

  // Binary branch references reject non-blocks, foreign function blocks, bad types and out-of-range IDs.
  TEST(IRModuleSerializationTest, RejectsMalformedBinaryBranches)
  {
    core::CompilationContext Compilation;
    IRContext Source(Compilation);
    const auto Parsed = deserializeModuleText(Source, BranchModuleText);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    const auto Encoded = serializeModuleBinary(*Parsed.ModuleValue);
    ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
    std::vector<std::size_t> Offsets{0};
    std::size_t BranchOffset = 0, ConditionalOffset = 0;
    std::uint32_t IntegerConstantId = 0, ParameterId = 0, OtherFunctionId = 0, OtherBlockId = 0, ModuleBlockId = 0, BoolTypeId = 0;
    for (std::size_t Offset = 16; Offset < Encoded.Bytes.size();)
    {
      Offsets.push_back(Offset);
      const auto Id = static_cast<std::uint32_t>(Offsets.size() - 1);
      const auto *Header = Encoded.Bytes.data() + Offset;
      const auto Kind = llvm::support::endian::read32le(Header);
      const auto Parent = llvm::support::endian::read32le(Header + 8);
      if (Kind == 50)
      {
        BranchOffset = Offset;
      }
      else if (Kind == 51)
      {
        ConditionalOffset = Offset;
      }
      else if (Kind == 31)
      {
        IntegerConstantId = Id;
      }
      else if (Kind == 38)
      {
        ParameterId = Id;
      }
      else if (Kind == 36)
      {
        OtherFunctionId = Id;
      }
      else if (Kind == 37 && Parent == 1)
      {
        ModuleBlockId = Id;
      }
      else if (Kind == 18)
      {
        BoolTypeId = Id;
      }
      Offset += 24 + static_cast<std::size_t>(llvm::support::endian::read32le(Header + 12)) * 8 + static_cast<std::size_t>(llvm::support::endian::read64le(Header + 16));
    }
    for (std::size_t Id = 1; Id < Offsets.size(); ++Id)
    {
      const auto *Header = Encoded.Bytes.data() + Offsets[Id];
      if (llvm::support::endian::read32le(Header) == 37 && llvm::support::endian::read32le(Header + 8) == OtherFunctionId)
      {
        OtherBlockId = static_cast<std::uint32_t>(Id);
      }
    }
    ASSERT_NE(BranchOffset, 0U);
    ASSERT_NE(ConditionalOffset, 0U);
    ASSERT_NE(IntegerConstantId, 0U);
    ASSERT_NE(ParameterId, 0U);
    ASSERT_NE(OtherBlockId, 0U);
    ASSERT_NE(ModuleBlockId, 0U);
    ASSERT_NE(BoolTypeId, 0U);
    const auto Reject = [&](std::string Bytes)
    {
      IRContext Destination(Compilation);
      const auto Result = deserializeModuleBinary(Destination, Bytes);
      EXPECT_EQ(Result.Status, ModuleArchiveStatus::InvalidArchive) << Result.Message;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_TRUE(Destination.modules().empty());
    };
    for (const std::uint64_t Target : {std::uint64_t{0}, std::uint64_t{UINT32_MAX}, std::uint64_t{ParameterId}, std::uint64_t{OtherBlockId}, std::uint64_t{ModuleBlockId}})
    {
      for (const auto FieldOffset : {BranchOffset + 24, ConditionalOffset + 32, ConditionalOffset + 40})
      {
        auto Bytes = Encoded.Bytes;
        llvm::support::endian::write64le(Bytes.data() + FieldOffset, Target);
        Reject(std::move(Bytes));
      }
    }
    auto WrongCondition = Encoded.Bytes;
    llvm::support::endian::write64le(WrongCondition.data() + ConditionalOffset + 24, IntegerConstantId);
    Reject(std::move(WrongCondition));
    auto WrongResult = Encoded.Bytes;
    llvm::support::endian::write32le(WrongResult.data() + ConditionalOffset + 4, BoolTypeId);
    Reject(std::move(WrongResult));
  }
} // namespace ink::ir::test
