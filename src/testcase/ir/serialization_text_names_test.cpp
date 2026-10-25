#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/serialization/serialization.h"

#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext NamesTarget(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // Numeric, anonymous, quoted and duplicate names preserve declaration identity across every named IR entity category.
  TEST(IRTextSerializationNamesTest, PreservesEscapedNamesAndCollisionsAcrossNamespaces)
  {
    IRContext Context(NamesTarget);
    Module Source(Context);
    auto &Names = core::NamePool::instance();
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    const auto *Signature = Context.typePool().get<FunctionType>(Integer, {Integer, Integer, Integer});
    constexpr std::string_view Spellings[] = {
        "0",
        "",
        "0",
        "...",
        "foo...bar",
        "quote\"slash\\ space",
        "duplicate",
        "duplicate",
        "duplicate.1",
        "->",
    };
    for (const auto Spelling : Spellings)
    {
      SCOPED_TRACE(Spelling);
      const auto Name = Spelling.empty() ? core::NAME_None : Names.intern(Spelling);
      auto *Nominal = Source.createStructType(4, 4, Name);
      ASSERT_NE(Nominal, nullptr);
      ASSERT_TRUE(Nominal->addField(Field(Integer, Names.intern("field\"\\..."), 0)));
      ASSERT_NE(Source.createGlobalVariable(*Integer, Name), nullptr);
      auto *Callable = Source.createGlobalFunction(*Signature, Name);
      ASSERT_NE(Callable, nullptr);
      for (std::size_t Index = 0; Index < Callable->parameterCount(); ++Index)
      {
        Callable->parameter(Index)->setName(Name);
      }
      auto *Entry = Callable->createBlock(Name);
      auto *Exit = Callable->createBlock(Name);
      IRBuilder Builder(Context);
      ASSERT_TRUE(Builder.setInsertPoint(*Entry));
      for (unsigned Index = 0; Index < 2; ++Index)
      {
        auto *Storage = Builder.createAlloca(*Integer);
        ASSERT_NE(Storage, nullptr);
        Storage->setName(Name);
      }
      auto *Negation = Builder.createNeg(*Callable->parameter(0));
      ASSERT_NE(Negation, nullptr);
      ASSERT_NE(Builder.createBranch(*Exit), nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*Exit));
      ASSERT_NE(Builder.createReturn(*Negation), nullptr);
    }
    const auto Original = writeBinary(Source);
    ASSERT_TRUE(Original.has_value()) << Original.error().Message;
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_NE(Text->find("@\"0\" = global"), std::string::npos);
    EXPECT_NE(Text->find("%\"...\" = type struct"), std::string::npos);
    EXPECT_NE(Text->find("%\"foo...bar\" = type struct"), std::string::npos);
    EXPECT_NE(Text->find("quote\\22slash\\5C space"), std::string::npos);
    EXPECT_NE(Text->find("name = \"duplicate\""), std::string::npos);
    IRContext Destination(NamesTarget);
    const auto Loaded = readText(Destination, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    const auto Bytes = writeBinary(**Loaded);
    ASSERT_TRUE(Bytes.has_value()) << Bytes.error().Message;
    EXPECT_EQ(*Bytes, *Original);
    const auto Rewritten = writeText(**Loaded);
    ASSERT_TRUE(Rewritten.has_value()) << Rewritten.error().Message;
    EXPECT_EQ(*Rewritten, *Text);
  }

  // A unit-producing call receives an SSA name when another call consumes its unit value, while unused void calls stay unassigned.
  TEST(IRTextSerializationNamesTest, PreservesReferencedVoidCallResults)
  {
    IRContext Context(NamesTarget);
    Module Source(Context);
    auto &Names = core::NamePool::instance();
    const auto *Void = Context.typePool().get<VoidType>();
    const auto *ProducerType = Context.typePool().get<FunctionType>(Void);
    const auto *ConsumerType = Context.typePool().get<FunctionType>(Void, {Void});
    auto *Producer = Source.createGlobalFunction(*ProducerType, Names.intern("produce"));
    auto *Consumer = Source.createGlobalFunction(*ConsumerType, Names.intern("consume"));
    auto *Caller = Source.createGlobalFunction(*ProducerType, Names.intern("run"));
    ASSERT_NE(Producer, nullptr);
    ASSERT_NE(Consumer, nullptr);
    ASSERT_NE(Caller, nullptr);
    auto *Entry = Caller->createBlock(Names.intern("entry"));
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Unit = Builder.createCall(*Producer);
    ASSERT_NE(Unit, nullptr);
    ASSERT_NE(Builder.createCall(*Consumer, {Argument(*Unit)}), nullptr);
    ASSERT_NE(Builder.createReturn(), nullptr);
    const auto Original = writeBinary(Source);
    ASSERT_TRUE(Original.has_value()) << Original.error().Message;
    const auto Text = writeText(Source);
    ASSERT_TRUE(Text.has_value()) << Text.error().Message;
    EXPECT_NE(Text->find("%0 = call void @produce()"), std::string::npos);
    EXPECT_NE(Text->find("call void @consume(void %0)"), std::string::npos);
    EXPECT_EQ(Text->find(" = call void @consume"), std::string::npos);
    IRContext Destination(NamesTarget);
    const auto Loaded = readText(Destination, *Text);
    ASSERT_TRUE(Loaded.has_value()) << Loaded.error().Message;
    const auto Bytes = writeBinary(**Loaded);
    ASSERT_TRUE(Bytes.has_value()) << Bytes.error().Message;
    EXPECT_EQ(*Bytes, *Original);
    ASSERT_EQ((*Loaded)->globalFunctionCount(), 3U);
    const auto *Body = (*Loaded)->globalFunction(2)->entryBlock();
    ASSERT_EQ(Body->instructionCount(), 3U);
    const auto *LoadedConsumer = static_cast<const CallInstruction *>(Body->instruction(1));
    EXPECT_EQ(LoadedConsumer->argument(0)->value(), Body->instruction(0));
  }
} // namespace ink::ir::test
