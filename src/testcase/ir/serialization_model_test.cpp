#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/serialization/serialization.h"

#include <gtest/gtest.h>

#include <limits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // An empty GEP index sequence preserves the original element pointer and remains valid through both codecs.
  TEST(IRSerializationModelTest, PreservesEmptyGetElementPtrIndices)
  {
    IRContext Context(Target);
    Module Source(Context);
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    const auto *Pointer = Context.typePool().get<PointerType>(Integer);
    auto *Function = Source.createGlobalFunction(*Context.typePool().get<FunctionType>(Pointer, {Pointer}));
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Function->createBlock()));
    auto *Address = Builder.createGetElementPtr(*Integer, *Function->parameter(0), {});
    ASSERT_NE(Address, nullptr);
    ASSERT_NE(Builder.createReturn(*Address), nullptr);
    const auto Binary = writeBinary(Source);
    ASSERT_TRUE(Binary) << Binary.error().Message;
    IRContext Destination(Target);
    const auto Loaded = readBinary(Destination, *Binary);
    ASSERT_TRUE(Loaded) << Loaded.error().Message;
    const auto *Restored = static_cast<const GetElementPtrInstruction *>((*Loaded)->globalFunction(0)->block(0)->instruction(0));
    EXPECT_EQ(Restored->indexCount(), 0U);
    const auto Text = writeText(**Loaded);
    ASSERT_TRUE(Text) << Text.error().Message;
    IRContext TextContext(Target);
    const auto TextModule = readText(TextContext, *Text);
    ASSERT_TRUE(TextModule) << TextModule.error().Message;
  }

  // The largest valid virtual slot remains distinct from an absent slot in the serialized method metadata.
  TEST(IRSerializationModelTest, PreservesMaximumVirtualSlot)
  {
    IRContext Context(Target);
    Module Source(Context);
    auto &Names = core::NamePool::instance();
    auto *Owner = Source.createClassType(8, 8, Names.intern("Slots"));
    const auto *Signature = Context.typePool().get<FunctionType>(Context.typePool().get<VoidType>());
    ASSERT_TRUE(Owner->addMethod(Method(std::make_unique<Function>(*Signature, Names.intern("maximum")), {MethodDispatch::Virtual, false, false, std::numeric_limits<std::uint32_t>::max()})));
    ASSERT_TRUE(Owner->addMethod(Method(std::make_unique<Function>(*Signature, Names.intern("unassigned")), {MethodDispatch::Virtual, false, false, std::nullopt})));
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text) << Text.error().Message;
    IRContext Destination(Target);
    const auto Loaded = readText(Destination, *Text);
    ASSERT_TRUE(Loaded) << Loaded.error().Message;
    EXPECT_EQ((*Loaded)->classType(0)->method(0)->virtualSlot(), std::numeric_limits<std::uint32_t>::max());
    EXPECT_FALSE((*Loaded)->classType(0)->method(1)->virtualSlot().has_value());
    const auto Binary = writeBinary(**Loaded);
    ASSERT_TRUE(Binary) << Binary.error().Message;
    IRContext BinaryContext(Target);
    const auto BinaryModule = readBinary(BinaryContext, *Binary);
    ASSERT_TRUE(BinaryModule) << BinaryModule.error().Message;
    EXPECT_EQ((*BinaryModule)->classType(0)->method(0)->virtualSlot(), std::numeric_limits<std::uint32_t>::max());
  }

  // Structural nesting limits also apply when every referenced child type was already decoded earlier in the file.
  TEST(IRSerializationModelTest, RejectsDeepTypesStoredChildrenFirst)
  {
    IRContext Context(Target);
    Module Source(Context);
    const Type *Nested = Context.typePool().get<IntegerType>(32, true);
    const auto Symbol = core::NamePool::instance().intern("nested");
    ASSERT_TRUE(Source.addExport({Symbol}, *Nested));
    for (unsigned Index = 0; Index < 20; ++Index)
    {
      Nested = Context.typePool().get<PointerType>(Nested);
      ASSERT_NE(Nested, nullptr);
      ASSERT_TRUE(Source.addExport({Symbol}, *Nested));
    }
    const auto Binary = writeBinary(Source);
    ASSERT_TRUE(Binary) << Binary.error().Message;
    IRContext Destination(Target);
    IRReadOptions Options;
    Options.MaxDepth = 8;
    const auto Rejected = readBinary(Destination, *Binary, Options);
    ASSERT_FALSE(Rejected);
    EXPECT_EQ(Rejected.error().Code, IRSerializationErrorCode::ResourceLimit);
    const auto Loaded = readBinary(Destination, *Binary);
    ASSERT_TRUE(Loaded) << Loaded.error().Message;
  }

  // A return block stored before its dominating body resolves a long SSA dependency chain without stack recursion.
  TEST(IRSerializationModelTest, ResolvesLongForwardSsaChainsWithSmallTypeDepthLimit)
  {
    IRContext Context(Target);
    Module Source(Context);
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    auto *Function = Source.createGlobalFunction(*Context.typePool().get<FunctionType>(Integer, {Integer}));
    auto *Entry = Function->createBlock();
    auto *Exit = Function->createBlock();
    auto *Body = Function->createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    ASSERT_NE(Builder.createBranch(*Body), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Body));
    const Value *Value = Function->parameter(0);
    for (unsigned Index = 0; Index < 4096; ++Index)
    {
      Value = Builder.createNeg(*Value);
      ASSERT_NE(Value, nullptr);
    }
    ASSERT_NE(Builder.createBranch(*Exit), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Exit));
    ASSERT_NE(Builder.createReturn(*Value), nullptr);
    const auto Binary = writeBinary(Source);
    ASSERT_TRUE(Binary) << Binary.error().Message;
    IRContext Destination(Target);
    IRReadOptions Options;
    Options.MaxDepth = 4;
    const auto Loaded = readBinary(Destination, *Binary, Options);
    ASSERT_TRUE(Loaded) << Loaded.error().Message;
    const auto *Restored = (*Loaded)->globalFunction(0);
    EXPECT_EQ(Restored->block(2)->instructionCount(), 4097U);
    EXPECT_EQ(static_cast<const ReturnInstruction *>(Restored->block(1)->instruction(0))->returnedValue(), Restored->block(2)->instruction(4095));
  }
} // namespace ink::ir::test
