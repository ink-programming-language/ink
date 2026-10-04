#include "artifact_test_support.h"

#include <gtest/gtest.h>

namespace ink::execution::test
{
  // Exact generic constant bits use the same uppercase canonical spelling as native Ink symbols.
  TEST(BytecodeBuilderTest, PreservesUppercaseGenericConstantBits)
  {
    ArtifactContext Test;
    auto *Function = Test.function("value", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    const auto Integer = bytecodeTypeIdentity(*Test.Bridge.types(), Test.Bridge.lowerType(Test.Int32));
    const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "math", "value", BytecodeSymbolKind::Definition, BytecodeVisibility::Public, {{core::GenericArgumentKind::Value, Integer, "FFFFFFFE"}})};
    const auto Built = buildBytecodeObject("math", Test.Bridge, Inputs);
    ASSERT_TRUE(Built) << Built.Message;
    EXPECT_NE(Built.Artifact->Symbols.front().Identity.LinkName.find("B8_FFFFFFFE"), std::string::npos);
    const auto Saved = serializeBytecodeArtifact(*Built.Artifact);
    ASSERT_TRUE(Saved);
    const auto Restored = deserializeBytecodeArtifact(Saved.Bytes);
    ASSERT_TRUE(Restored) << Restored.Message;
    EXPECT_EQ(Restored.Artifact->Symbols.front().Identity, Built.Artifact->Symbols.front().Identity);
  }

  // A source import borrows the analyzed target's real signature without copying its body into the consumer object.
  TEST(BytecodeBuilderTest, ImportsAnalyzedDefinitionsWithoutEmittingTheirBodies)
  {
    ArtifactContext Test;
    auto *Answer = Test.function("answer", Test.Int32);
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Answer));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *Call = Test.Builder.createCallInstruction(*Answer);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
    const BytecodeFunctionInput ConsumerInputs[] = {Test.input(*Main, "app", "main"), Test.input(*Answer, "library", "answer", BytecodeSymbolKind::Import)};
    auto Consumer = buildBytecodeObject("app", Test.Bridge, ConsumerInputs);
    ASSERT_TRUE(Consumer) << Consumer.Message;
    ASSERT_EQ(Consumer.Artifact->Image.Functions.size(), 1U);
    const auto *Import = findArtifactSymbol(*Consumer.Artifact, "library", "answer");
    ASSERT_NE(Import, nullptr);
    EXPECT_EQ(Import->Kind, BytecodeSymbolKind::Import);
    EXPECT_FALSE(Consumer.Artifact->Image.Functions.contains(Import->Function));
    const BytecodeFunctionInput ProviderInputs[] = {Test.input(*Answer, "library", "answer")};
    auto Provider = buildBytecodeObject("library", Test.Bridge, ProviderInputs);
    ASSERT_TRUE(Provider) << Provider.Message;
    const auto *Entry = findArtifactSymbol(*Consumer.Artifact, "app", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Consumer.Artifact.get(), Provider.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Executed = executeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
  }

  // Importing an analyzed C ABI definition preserves its ABI without publishing its native export from the consumer object.
  TEST(BytecodeBuilderTest, ImportsLocalAndExportedCAbiDefinitionsAsModuleReferences)
  {
    for (core::FunctionBinding Binding : {core::FunctionBinding::Local, core::FunctionBinding::Export})
    {
      SCOPED_TRACE(static_cast<int>(Binding));
      ArtifactContext Test;
      auto *Answer = Test.function("inkBytecodeAnswer", Test.Int32, {}, ir::LanguageLinkage::C, Binding);
      auto *Main = Test.function("main", Test.Int32);
      ASSERT_NE(Answer, nullptr);
      ASSERT_NE(Main, nullptr);
      ASSERT_TRUE(Test.begin(*Answer));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
      ASSERT_TRUE(Test.begin(*Main));
      const auto *Call = Test.Builder.createCallInstruction(*Answer);
      ASSERT_NE(Call, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
      const BytecodeFunctionInput ConsumerInputs[] = {Test.input(*Main, "app", "main"), Test.input(*Answer, "library", "inkBytecodeAnswer", BytecodeSymbolKind::Import)};
      auto Consumer = buildBytecodeObject("app", Test.Bridge, ConsumerInputs);
      ASSERT_TRUE(Consumer) << Consumer.Message;
      const auto *Imported = findArtifactSymbol(*Consumer.Artifact, "library", "inkBytecodeAnswer");
      ASSERT_NE(Imported, nullptr);
      EXPECT_EQ(Imported->Kind, BytecodeSymbolKind::Import);
      const auto &Descriptor = Consumer.Artifact->Image.Descriptors.at(Imported->Function);
      EXPECT_TRUE(Descriptor.CAbi);
      EXPECT_FALSE(Descriptor.External);
      EXPECT_FALSE(Descriptor.Exported);
      EXPECT_FALSE(Consumer.Artifact->Image.Functions.contains(Imported->Function));
      const BytecodeFunctionInput ProviderInputs[] = {Test.input(*Answer, "library", "inkBytecodeAnswer")};
      auto Provider = buildBytecodeObject("library", Test.Bridge, ProviderInputs);
      ASSERT_TRUE(Provider) << Provider.Message;
      const auto *Entry = findArtifactSymbol(*Consumer.Artifact, "app", "main");
      ASSERT_NE(Entry, nullptr);
      const BytecodeArtifact *Objects[] = {Consumer.Artifact.get(), Provider.Artifact.get()};
      auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
      ASSERT_TRUE(Linked) << Linked.Message;
      const auto *Defined = findArtifactSymbol(*Linked.Artifact, "library", "inkBytecodeAnswer");
      ASSERT_NE(Defined, nullptr);
      EXPECT_EQ(Linked.Artifact->Image.Descriptors.at(Defined->Function).Exported, Binding == core::FunctionBinding::Export);
      const auto Executed = executeArtifact(*Linked.Artifact);
      ASSERT_TRUE(Executed);
      EXPECT_EQ(Executed.Value.Bits, 42U);
    }
  }

  // Local and exported C ABI definitions cannot place unsupported wide-integer signatures in a portable bytecode object.
  TEST(BytecodeBuilderTest, RejectsUnsupportedCAbiDefinitionSignatures)
  {
    for (core::FunctionBinding Binding : {core::FunctionBinding::Local, core::FunctionBinding::Export})
    {
      ArtifactContext Test;
      const auto *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(128, true);
      auto *Function = Test.function("wide", *Wide, {}, ir::LanguageLinkage::C, Binding);
      ASSERT_NE(Function, nullptr);
      ASSERT_TRUE(Test.begin(*Function));
      const auto *Value = Test.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, 42));
      ASSERT_NE(Value, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "library", "wide")};
      EXPECT_EQ(buildBytecodeObject("library", Test.Bridge, Inputs).Status, BytecodeStatus::InvalidImage);
    }
  }

  // Object construction preserves unresolved native imports even when its bridge resolves exports for interpretation.
  TEST(BytecodeBuilderTest, TemporarilyDisablesRuntimeNativeResolution)
  {
    ArtifactContext Test;
    auto *Module = Test.Builder.createModule(Test.Context.namePool().intern("library"));
    ASSERT_NE(Module, nullptr);
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.Int32, std::span<const ir::Type *const>{});
    ASSERT_NE(Signature, nullptr);
    auto Exported = Test.Builder.createFunction(Test.Context.namePool().intern("inkNativeObjectBoundary"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Export);
    auto *Imported = Test.function("inkNativeObjectBoundary", Test.Int32, {}, ir::LanguageLinkage::C, core::FunctionBinding::Import);
    ASSERT_NE(Exported, nullptr);
    ASSERT_NE(Imported, nullptr);
    ASSERT_TRUE(Test.begin(*Exported));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    auto *Definition = Exported.get();
    ASSERT_TRUE(Test.Builder.appendValue(Module->entryBlock(), std::move(Exported)));
    EXPECT_FALSE(Test.Bridge.exchangeNativeImportResolution(true));
    const auto Resolved = Test.Bridge.lowerFunction(*Imported);
    EXPECT_EQ(Resolved, Test.Bridge.lowerFunction(*Definition));
    const BytecodeFunctionInput Inputs[] = {Test.input(*Imported, "C", "inkNativeObjectBoundary", BytecodeSymbolKind::Native), Test.input(*Definition, "library", "inkNativeObjectBoundary")};
    auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
    ASSERT_TRUE(Object) << Object.Message;
    const auto *Native = findArtifactSymbol(*Object.Artifact, "C", "inkNativeObjectBoundary");
    ASSERT_NE(Native, nullptr);
    EXPECT_TRUE(Object.Artifact->Image.Descriptors.at(Native->Function).External);
    EXPECT_NE(Native->Function, Resolved);
    EXPECT_EQ(Test.Bridge.lowerFunction(*Imported), Resolved);
  }

  // Every referenced function requires explicit module and visibility metadata instead of silently becoming a public dependency.
  TEST(BytecodeBuilderTest, RejectsReferencedFunctionsWithoutMetadata)
  {
    ArtifactContext Test;
    auto *Imported = Test.function("dependency", Test.Int32);
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *Call = Test.Builder.createCallInstruction(*Imported);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Main, "entry", "main")};
    const auto Result = buildBytecodeObject("entry", Test.Bridge, Inputs);
    EXPECT_FALSE(Result);
    EXPECT_EQ(Result.Artifact, nullptr);
  }

  // A definition's explicit symbol signature must describe the actual function rather than another valid local function type.
  TEST(BytecodeBuilderTest, RejectsMetadataWithAnotherFunctionSignature)
  {
    ArtifactContext Test;
    auto *Function = Test.function("value", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    const ir::Type *Parameters[] = {&Test.Bool};
    const auto *OtherSignature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.Int32, Parameters);
    const auto Signature = bytecodeTypeIdentity(*Test.Bridge.types(), Test.Bridge.lowerType(*OtherSignature));
    BytecodeFunctionInput Input = Test.input(*Function, "library", "value");
    Input.Identity.Signature = Signature;
    const BytecodeFunctionInput Inputs[] = {Input};
    const auto Result = buildBytecodeObject("library", Test.Bridge, Inputs);
    EXPECT_EQ(Result.Status, BytecodeStatus::SignatureMismatch);
    EXPECT_EQ(Result.Artifact, nullptr);
  }

  // A valid function signature that exceeds the type-depth budget reports resource exhaustion instead of a signature mismatch.
  TEST(BytecodeBuilderTest, PreservesTypeDepthLimitStatusDuringObjectConstruction)
  {
    ArtifactContext Test;
    auto *Function = Test.function("value", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "library", "value")};
    ASSERT_TRUE(buildBytecodeObject("library", Test.Bridge, Inputs));
    BytecodeLimits Limits;
    Limits.MaxTypeDepth = 1;
    const auto Result = buildBytecodeObject("library", Test.Bridge, Inputs, Limits);
    EXPECT_EQ(Result.Status, BytecodeStatus::LimitExceeded);
    EXPECT_EQ(Result.Artifact, nullptr);
  }

  // Object construction compiles exported definitions without executing them, and a later link can choose any definition as entry.
  TEST(BytecodeBuilderTest, CompilesUncalledDefinitionsForLaterEntrySelection)
  {
    ArtifactContext Test;
    auto *First = Test.function("first", Test.Int32);
    auto *Second = Test.function("second", Test.Int32);
    ASSERT_TRUE(Test.begin(*First));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(11)), nullptr);
    ASSERT_TRUE(Test.begin(*Second));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*First, "library", "first"), Test.input(*Second, "library", "second")};
    auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
    ASSERT_TRUE(Object) << Object.Message;
    ASSERT_EQ(Object.Artifact->Image.Functions.size(), 2U);
    const auto *Entry = findArtifactSymbol(*Object.Artifact, "library", "second");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Object.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Executed = executeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
  }

  // Adding unrelated types to the compiler bridge after object creation cannot change that object's type domain or archive bytes.
  TEST(BytecodeBuilderTest, FreezesTypeTableBeforeReturningObject)
  {
    ArtifactContext Test;
    auto *Function = Test.function("value", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "library", "value")};
    auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
    ASSERT_TRUE(Object) << Object.Message;
    const auto OriginalSize = Object.Artifact->Image.Layouts->size();
    EXPECT_NE(Object.Artifact->Image.Layouts, Test.Bridge.types());
    const auto Before = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Before) << Before.Message;
    const auto *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(129, false);
    ASSERT_NE(Test.Bridge.lowerType(*Wide), InvalidRuntimeType);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Wide, ir::AccessKind::ReadWrite);
    ASSERT_NE(Test.Bridge.lowerType(*Pointer), InvalidRuntimeType);
    EXPECT_GT(Test.Bridge.types()->size(), OriginalSize);
    EXPECT_EQ(Object.Artifact->Image.Layouts->size(), OriginalSize);
    const auto After = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(After) << After.Message;
    EXPECT_EQ(Before.Bytes, After.Bytes);
  }
} // namespace ink::execution::test
