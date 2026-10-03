#include "artifact_test_support.h"

#include <gtest/gtest.h>

namespace ink::execution::test
{
  namespace
  {
    BytecodeArtifactResult arrayProvider(std::uint64_t Count = 2)
    {
      ArtifactContext Test;
      const auto *Row = Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, Count);
      const auto *Grid = Test.Context.typePool().getType<ir::TypeKind::Array>(*Row, 2);
      std::vector<const ir::Constant *> Elements(static_cast<std::size_t>(Count), &Test.integer(21));
      const auto *RowValue = Test.Context.constantPool().getArrayConstant(*Row, Elements);
      const ir::Constant *Rows[] = {RowValue, RowValue};
      const auto *GridValue = Test.Context.constantPool().getArrayConstant(*Grid, Rows);
      auto *Function = Test.function("grid", *Grid);
      if (!Function || !Test.begin(*Function) || !Test.Builder.createReturnInstruction(GridValue))
      {
        return {BytecodeStatus::InvalidInput, "cannot construct array provider", nullptr};
      }
      const auto *Empty = Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, 0);
      const auto *NestedEmpty = Test.Context.typePool().getType<ir::TypeKind::Array>(*Empty, 2);
      const auto *EmptyValue = Test.Context.constantPool().getArrayConstant(*Empty, {});
      const ir::Constant *EmptyRows[] = {EmptyValue, EmptyValue};
      const auto *NestedEmptyValue = Test.Context.constantPool().getArrayConstant(*NestedEmpty, EmptyRows);
      auto *EmptyFunction = Test.function("empty", *NestedEmpty);
      if (!EmptyFunction || !Test.begin(*EmptyFunction) || !Test.Builder.createReturnInstruction(NestedEmptyValue))
      {
        return {BytecodeStatus::InvalidInput, "cannot construct nested empty array provider", nullptr};
      }
      const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "provider", "grid"), Test.input(*EmptyFunction, "provider", "empty")};
      return buildBytecodeObject("provider", Test.Bridge, Inputs);
    }

    BytecodeArtifactResult arrayConsumer()
    {
      ArtifactContext Test;
      Test.Bridge.lowerType(Test.Bool);
      Test.Bridge.lowerType(*Test.Context.typePool().getType<ir::TypeKind::Integer>(64, false));
      const auto *Row = Test.Context.typePool().getType<ir::TypeKind::Array>(Test.Int32, 2);
      const auto *Grid = Test.Context.typePool().getType<ir::TypeKind::Array>(*Row, 2);
      auto *Provider = Test.function("grid", *Grid);
      auto *Main = Test.function("main", Test.Int32);
      if (!Provider || !Main || !Test.begin(*Main))
      {
        return {BytecodeStatus::InvalidInput, "cannot construct array consumer", nullptr};
      }
      const auto *Call = Test.Builder.createCallInstruction(*Provider);
      const auto *First = Call ? Test.Builder.createArrayExtractInstruction(*Call, Test.integer(1)) : nullptr;
      const auto *Value = First ? Test.Builder.createArrayExtractInstruction(*First, Test.integer(0)) : nullptr;
      const auto *Sum = Value ? Test.Builder.createAddInstruction(*Value, *Value) : nullptr;
      if (!Sum || !Test.Builder.createReturnInstruction(Sum))
      {
        return {BytecodeStatus::InvalidInput, "cannot construct array consumer body", nullptr};
      }
      const BytecodeFunctionInput Inputs[] = {Test.input(*Main, "consumer", "main"), Test.input(*Provider, "provider", "grid", BytecodeSymbolKind::Import)};
      return buildBytecodeObject("consumer", Test.Bridge, Inputs);
    }
  } // namespace

  // Nested and zero-length array constants survive deterministic archive round trips after all original IR has been destroyed.
  TEST(ArrayArtifactTest, RoundTripsNestedAndEmptyArrayConstants)
  {
    auto Object = arrayProvider();
    ASSERT_TRUE(Object) << Object.Message;
    const auto Encoded = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Encoded) << Encoded.Message;
    Object.Artifact.reset();
    auto Loaded = deserializeBytecodeArtifact(Encoded.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const auto Reencoded = serializeBytecodeArtifact(*Loaded.Artifact);
    ASSERT_TRUE(Reencoded) << Reencoded.Message;
    EXPECT_EQ(Reencoded.Bytes, Encoded.Bytes);
    for (const std::string_view Name : {"grid", "empty"})
    {
      const auto *Symbol = findArtifactSymbol(*Loaded.Artifact, "provider", Name);
      ASSERT_NE(Symbol, nullptr);
      const auto &Function = *Loaded.Artifact->Image.Functions.at(Symbol->Function);
      const auto &Initial = Function.InitialSlots[Function.Code.back().Operands[1]];
      ASSERT_EQ(Initial.kind(), RuntimeKind::Array);
      ASSERT_EQ(Initial.array().size(), 2U);
      EXPECT_EQ(Initial.array()[0].array().size(), Name == "empty" ? 0U : 2U);
      const auto *Layout = Function.Layouts->get(Initial.Type);
      ASSERT_NE(Layout->ElementLayout, nullptr);
      EXPECT_EQ(Layout->ElementLayout->Domain, Function.Layouts->domain());
      EXPECT_EQ(Layout->ElementLayout->Type, Initial.array()[0].Type);
    }
  }

  // Independent objects remap nested array type IDs, constant element IDs and return signatures before executing linked array calls.
  TEST(ArrayArtifactTest, LinksArraysAcrossIndependentTypeTables)
  {
    auto Provider = arrayProvider();
    auto Consumer = arrayConsumer();
    auto Unrelated = constantObject("unrelated", "value", 99);
    ASSERT_TRUE(Provider) << Provider.Message;
    ASSERT_TRUE(Consumer) << Consumer.Message;
    ASSERT_TRUE(Unrelated) << Unrelated.Message;
    const auto ProviderBytes = serializeBytecodeArtifact(*Provider.Artifact);
    const auto ConsumerBytes = serializeBytecodeArtifact(*Consumer.Artifact);
    ASSERT_TRUE(ProviderBytes);
    ASSERT_TRUE(ConsumerBytes);
    Provider = deserializeBytecodeArtifact(ProviderBytes.Bytes);
    Consumer = deserializeBytecodeArtifact(ConsumerBytes.Bytes);
    ASSERT_TRUE(Provider);
    ASSERT_TRUE(Consumer);
    const auto *Entry = findArtifactSymbol(*Consumer.Artifact, "consumer", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Unrelated.Artifact.get(), Consumer.Artifact.get(), Provider.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Encoded = serializeBytecodeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Encoded) << Encoded.Message;
    Linked.Artifact.reset();
    Provider.Artifact.reset();
    Consumer.Artifact.reset();
    Unrelated.Artifact.reset();
    auto Loaded = deserializeBytecodeArtifact(Encoded.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const auto Result = executeArtifact(*Loaded.Artifact);
    ASSERT_TRUE(Result) << static_cast<unsigned>(Result.Status);
    EXPECT_EQ(Result.Value.Bits, 42U);
  }

  // Array lengths participate in linked function identity, preventing equally named providers with incompatible aggregate returns.
  TEST(ArrayArtifactTest, RejectsArrayLengthSignatureMismatch)
  {
    auto Provider = arrayProvider(3);
    auto Consumer = arrayConsumer();
    ASSERT_TRUE(Provider);
    ASSERT_TRUE(Consumer);
    const auto *Entry = findArtifactSymbol(*Consumer.Artifact, "consumer", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Consumer.Artifact.get(), Provider.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(Objects, &Entry->Identity).Status, BytecodeStatus::SignatureMismatch);
  }

  // Malformed aggregate initializers cannot enter an archive with incorrect element count, element type or initialization state.
  TEST(ArrayArtifactTest, RejectsCorruptArrayConstantPayloads)
  {
    for (unsigned Case = 0; Case < 3; ++Case)
    {
      auto Object = arrayProvider();
      ASSERT_TRUE(Object) << Object.Message;
      const auto *Symbol = findArtifactSymbol(*Object.Artifact, "provider", "grid");
      ASSERT_NE(Symbol, nullptr);
      auto &Function = *Object.Artifact->Image.Functions.at(Symbol->Function);
      auto &Initial = Function.InitialSlots[Function.Code.back().Operands[1]];
      std::vector<RuntimeValue> Elements(Initial.array().begin(), Initial.array().end());
      if (Case == 0)
      {
        Elements.pop_back();
      }
      else if (Case == 1)
      {
        Elements[0].Type = Initial.Type;
      }
      else
      {
        Elements[0].Initialized = false;
      }
      Initial = RuntimeValue::fromArray(std::move(Elements), Initial.Type);
      EXPECT_FALSE(validateBytecodeArtifact(*Object.Artifact));
      EXPECT_FALSE(serializeBytecodeArtifact(*Object.Artifact));
    }
  }

  // Every truncated prefix of a nested-array archive fails instead of exposing a partially decoded aggregate.
  TEST(ArrayArtifactTest, RejectsTruncatedNestedArrayArchive)
  {
    auto Object = arrayProvider();
    ASSERT_TRUE(Object);
    const auto Encoded = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Encoded);
    for (std::size_t Size = 0; Size < Encoded.Bytes.size(); ++Size)
    {
      const auto Decoded = deserializeBytecodeArtifact(std::string_view(Encoded.Bytes).substr(0, Size));
      EXPECT_FALSE(Decoded) << Size;
      EXPECT_EQ(Decoded.Artifact, nullptr);
    }
  }
} // namespace ink::execution::test
