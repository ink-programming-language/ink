#include "artifact_test_support.h"
#include "ink/ir/constant/class_constant.h"
#include "ink/ir/linkage.h"

#include <gtest/gtest.h>

namespace ink::execution::test
{
  namespace
  {
    std::string classIdentity(std::string_view Name)
    {
      const abi::ModuleIdentity Module{{}, {"models"}};
      return abi::mangle(abi::record('T', {abi::record('c', {ir::declarationRecord(Module, {}, 'c', Name), {'X', {}}})})).Name;
    }

    const ir::ClassType *recordType(ArtifactContext &Test, std::string_view Identity, bool Reverse = false)
    {
      const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Record"));
      const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Flag"), Reverse ? static_cast<const ir::Type *>(&Test.Int32) : &Test.Bool}, {Test.Context.namePool().intern("Value"), Reverse ? &Test.Bool : static_cast<const ir::Type *>(&Test.Int32)}};
      return Class && Test.Builder.defineClassType(*Class, Fields, classIdentity(Identity)) ? Class : nullptr;
    }

    BytecodeArtifactResult classProvider(std::string_view Identity = "models::Record", bool Reverse = false)
    {
      ArtifactContext Test;
      const auto *Class = recordType(Test, Identity, Reverse);
      if (!Class)
      {
        return {BytecodeStatus::InvalidInput, "cannot create class provider type", nullptr};
      }
      const ir::Constant *Fields[] = {Reverse ? static_cast<const ir::Constant *>(&Test.integer(42)) : &Test.Context.constantPool().getBoolConstant(true), Reverse ? &Test.Context.constantPool().getBoolConstant(true) : static_cast<const ir::Constant *>(&Test.integer(42))};
      const auto *Value = Test.Context.constantPool().getClassConstant(*Class, Fields);
      auto *Function = Test.function("record", *Class);
      if (!Function || !Test.begin(*Function) || !Value || !Test.Builder.createReturnInstruction(Value))
      {
        return {BytecodeStatus::InvalidInput, "cannot construct class provider body", nullptr};
      }
      const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "provider", "record")};
      return buildBytecodeObject("provider", Test.Bridge, Inputs);
    }

    BytecodeArtifactResult classConsumer(std::string_view Identity = "models::Record")
    {
      ArtifactContext Test;
      Test.Bridge.lowerType(Test.Int32);
      const auto *Class = recordType(Test, Identity);
      if (!Class)
      {
        return {BytecodeStatus::InvalidInput, "cannot create class consumer type", nullptr};
      }
      auto *Provider = Test.function("record", *Class);
      auto *Main = Test.function("main", Test.Int32);
      if (!Provider || !Main || !Test.begin(*Main))
      {
        return {BytecodeStatus::InvalidInput, "cannot construct class consumer", nullptr};
      }
      const auto *Call = Test.Builder.createCallInstruction(*Provider);
      const auto *Value = Call ? Test.Builder.createFieldExtractInstruction(*Call, 1) : nullptr;
      if (!Value || !Test.Builder.createReturnInstruction(Value))
      {
        return {BytecodeStatus::InvalidInput, "cannot construct class consumer body", nullptr};
      }
      const BytecodeFunctionInput Inputs[] = {Test.input(*Main, "consumer", "main"), Test.input(*Provider, "provider", "record", BytecodeSymbolKind::Import)};
      return buildBytecodeObject("consumer", Test.Bridge, Inputs);
    }
  } // namespace

  // Class constants retain field types and nominal identities after archive recovery without their original IR context.
  TEST(ClassArtifactTest, RoundTripsClassConstantsAndRejectsTruncation)
  {
    auto Object = classProvider();
    ASSERT_TRUE(Object) << Object.Message;
    const auto Encoded = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Encoded) << Encoded.Message;
    Object.Artifact.reset();
    auto Loaded = deserializeBytecodeArtifact(Encoded.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const auto Reencoded = serializeBytecodeArtifact(*Loaded.Artifact);
    ASSERT_TRUE(Reencoded) << Reencoded.Message;
    EXPECT_EQ(Reencoded.Bytes, Encoded.Bytes);
    const auto *Symbol = findArtifactSymbol(*Loaded.Artifact, "provider", "record");
    ASSERT_NE(Symbol, nullptr);
    const auto &Function = *Loaded.Artifact->Image.Functions.at(Symbol->Function);
    const auto &Value = Function.InitialSlots[Function.Code.back().Operands[1]];
    ASSERT_EQ(Value.kind(), RuntimeKind::Class);
    ASSERT_EQ(Value.fields().size(), 2U);
    EXPECT_EQ(Value.fields()[1].Bits, 42U);
    EXPECT_EQ(Function.Layouts->get(Value.Type)->classDesc().NominalIdentity, classIdentity("models::Record"));
    EXPECT_EQ(Function.Layouts->get(Value.Type)->classDesc().Fields[0].Name, "Flag");
    EXPECT_EQ(Function.Layouts->get(Value.Type)->classDesc().Fields[1].Name, "Value");
    for (std::size_t Size = 0; Size < Encoded.Bytes.size(); ++Size)
    {
      EXPECT_FALSE(deserializeBytecodeArtifact(std::string_view(Encoded.Bytes.data(), Size))) << Size;
    }
  }

  // Separately built and reloaded class objects remap every field ID and execute a linked call returning a class value.
  TEST(ClassArtifactTest, LinksClassValuesAcrossIndependentContexts)
  {
    auto Provider = classProvider();
    auto Consumer = classConsumer();
    ASSERT_TRUE(Provider) << Provider.Message;
    ASSERT_TRUE(Consumer) << Consumer.Message;
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
    const BytecodeArtifact *Objects[] = {Consumer.Artifact.get(), Provider.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Encoded = serializeBytecodeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Encoded);
    auto Loaded = deserializeBytecodeArtifact(Encoded.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const auto Result = executeArtifact(*Loaded.Artifact);
    ASSERT_TRUE(Result) << static_cast<unsigned>(Result.Status);
    EXPECT_EQ(Result.Value.Bits, 42U);
  }

  // Equal physical layouts do not merge distinct classes, and inconsistent definitions of the same nominal class fail linking.
  TEST(ClassArtifactTest, PreservesNominalIdentityAndRejectsConflictingDefinitions)
  {
    auto Provider = classProvider();
    auto Distinct = classConsumer("other::Record");
    ASSERT_TRUE(Provider);
    ASSERT_TRUE(Distinct);
    const BytecodeArtifact *Objects[] = {Distinct.Artifact.get(), Provider.Artifact.get()};
    EXPECT_FALSE(linkBytecodeArtifacts(Objects));
    auto Conflicting = classProvider("models::Record", true);
    auto Consumer = classConsumer();
    ASSERT_TRUE(Conflicting);
    ASSERT_TRUE(Consumer);
    const BytecodeArtifact *Conflict[] = {Consumer.Artifact.get(), Conflicting.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(Conflict).Status, BytecodeStatus::InvalidImage);
  }

  // Pointer-mediated recursive classes serialize and link successfully while field layout cycles remain invalid.
  TEST(ClassArtifactTest, RoundTripsRecursiveClassPointers)
  {
    ArtifactContext Test;
    const auto *Node = Test.Builder.createClassType(Test.Context.namePool().intern("Node"));
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Node, ir::AccessKind::ReadWrite);
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Next"), Pointer}, {Test.Context.namePool().intern("Value"), &Test.Int32}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Node, Fields, classIdentity("models::Node")));
    const ir::Type *Parameters[] = {Pointer};
    auto *Function = Test.function("node", Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "nodes", "node")};
    auto Object = buildBytecodeObject("nodes", Test.Bridge, Inputs);
    ASSERT_TRUE(Object) << Object.Message;
    const auto Encoded = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Encoded) << Encoded.Message;
    auto Loaded = deserializeBytecodeArtifact(Encoded.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const BytecodeArtifact *Objects[] = {Loaded.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects);
    ASSERT_TRUE(Linked) << Linked.Message;
    for (std::size_t Index = 0; Index < Linked.Artifact->Image.Layouts->size(); ++Index)
    {
      const auto &Layout = *Linked.Artifact->Image.Layouts->get(static_cast<RuntimeTypeId>(Index));
      if (Layout.Kind == RuntimeKind::Class)
      {
        EXPECT_EQ(Linked.Artifact->Image.Layouts->get(Layout.classDesc().Fields[0].Type)->pointerDesc().Pointee, Layout.Type);
      }
    }
  }

  // Corrupt class field offsets are rejected before serialization and cannot reach field projection dispatch.
  TEST(ClassArtifactTest, RejectsCorruptClassFieldOffsets)
  {
    auto Object = classProvider();
    ASSERT_TRUE(Object);
    for (std::size_t Index = 0; Index < Object.Artifact->Image.Layouts->size(); ++Index)
    {
      auto &Layout = const_cast<TypeDesc &>(*Object.Artifact->Image.Layouts->get(static_cast<RuntimeTypeId>(Index)));
      if (Layout.Kind == RuntimeKind::Class)
      {
        auto Description = std::make_shared<ClassDesc>(Layout.classDesc());
        ++Description->Fields[1].Offset;
        Layout.setDetails(std::move(Description));
        EXPECT_EQ(validateBytecodeArtifact(*Object.Artifact).Status, BytecodeStatus::InvalidImage);
        EXPECT_FALSE(serializeBytecodeArtifact(*Object.Artifact));
        return;
      }
    }
    FAIL() << "missing class layout";
  }
} // namespace ink::execution::test
