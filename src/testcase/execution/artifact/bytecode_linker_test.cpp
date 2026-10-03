#include "artifact_test_support.h"

#include <gtest/gtest.h>

#include <array>
#include <string>

namespace ink::execution::test
{
  // Three separately archived modules preserve direct calls, returned function values and indirect calls after all source contexts die.
  TEST(BytecodeLinkerTest, LinksIndependentFilesAndRunsFunctionValuesWithoutSourceIR)
  {
    ArtifactFiles Files;
    ASSERT_TRUE(Files.ready());
    const auto ProviderPath = Files.file("provider.inko");
    const auto AdapterPath = Files.file("adapter.inko");
    const auto EntryPath = Files.file("entry.inko");
    const auto ImagePath = Files.file("program.inkb");
    RuntimeTypeId ProviderSignature = InvalidRuntimeType;
    RuntimeTypeId AdapterSignature = InvalidRuntimeType;
    {
      ArtifactContext Test;
      const ir::Type *Parameters[] = {&Test.Int32};
      auto *Value = Test.function("value", Test.Int32, Parameters);
      ASSERT_NE(Value, nullptr);
      ASSERT_TRUE(Test.begin(*Value));
      const auto *Sum = Test.Builder.createAddInstruction(*Value->parameters()[0], Test.integer(2));
      ASSERT_NE(Sum, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
      auto *Factory = Test.function("factory", Value->type());
      ASSERT_NE(Factory, nullptr);
      ASSERT_TRUE(Test.begin(*Factory));
      ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Value, "provider", "value"), Test.input(*Factory, "provider", "factory")};
      auto Object = buildBytecodeObject("provider", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      const auto *FactorySymbol = findArtifactSymbol(*Object.Artifact, "provider", "factory");
      ASSERT_NE(FactorySymbol, nullptr);
      ProviderSignature = Object.Artifact->Image.Descriptors.at(FactorySymbol->Function).Signature;
      ASSERT_TRUE(writeBytecodeFile(ProviderPath, *Object.Artifact));
    }
    {
      ArtifactContext Test;
      Test.Bridge.lowerType(Test.Bool);
      Test.Bridge.lowerType(*Test.Context.typePool().getType<ir::TypeKind::Integer>(64, false));
      const ir::Type *Parameters[] = {&Test.Int32};
      const auto *ValueType = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.Int32, Parameters);
      auto *Factory = Test.function("factory", *ValueType);
      auto *Run = Test.function("run", Test.Int32, Parameters);
      ASSERT_NE(Factory, nullptr);
      ASSERT_NE(Run, nullptr);
      ASSERT_TRUE(Test.begin(*Run));
      const auto *Callee = Test.Builder.createCallInstruction(*Factory);
      ASSERT_NE(Callee, nullptr);
      const ir::Value *Arguments[] = {Run->parameters()[0].get()};
      const auto *Indirect = Test.Builder.createCallInstruction(*Callee, Arguments);
      ASSERT_NE(Indirect, nullptr);
      const auto *Sum = Test.Builder.createAddInstruction(*Indirect, Test.integer(3));
      ASSERT_NE(Sum, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Run, "adapter", "run"), Test.input(*Factory, "provider", "factory", BytecodeSymbolKind::Import)};
      auto Object = buildBytecodeObject("adapter", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      const auto *FactorySymbol = findArtifactSymbol(*Object.Artifact, "provider", "factory");
      ASSERT_NE(FactorySymbol, nullptr);
      AdapterSignature = Object.Artifact->Image.Descriptors.at(FactorySymbol->Function).Signature;
      ASSERT_TRUE(writeBytecodeFile(AdapterPath, *Object.Artifact));
    }
    EXPECT_NE(ProviderSignature, AdapterSignature);
    {
      ArtifactContext Test;
      Test.Bridge.lowerType(*Test.Context.typePool().getType<ir::TypeKind::Integer>(16, false));
      const ir::Type *Parameters[] = {&Test.Int32};
      auto *Run = Test.function("run", Test.Int32, Parameters);
      auto *Main = Test.function("main", Test.Int32);
      ASSERT_NE(Run, nullptr);
      ASSERT_NE(Main, nullptr);
      ASSERT_TRUE(Test.begin(*Main));
      const ir::Value *Arguments[] = {&Test.integer(37)};
      const auto *Call = Test.Builder.createCallInstruction(*Run, Arguments);
      ASSERT_NE(Call, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Main, "entry", "main"), Test.input(*Run, "adapter", "run", BytecodeSymbolKind::Import)};
      auto Object = buildBytecodeObject("entry", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      ASSERT_TRUE(writeBytecodeFile(EntryPath, *Object.Artifact));
    }
    auto Provider = readBytecodeFile(ProviderPath);
    auto Adapter = readBytecodeFile(AdapterPath);
    auto Entry = readBytecodeFile(EntryPath);
    ASSERT_TRUE(Provider) << Provider.Message;
    ASSERT_TRUE(Adapter) << Adapter.Message;
    ASSERT_TRUE(Entry) << Entry.Message;
    const auto *EntrySymbol = findArtifactSymbol(*Entry.Artifact, "entry", "main");
    ASSERT_NE(EntrySymbol, nullptr);
    EXPECT_EQ(Provider.Artifact->Symbols.front().Function, Adapter.Artifact->Symbols.front().Function);
    const BytecodeArtifact *Objects[] = {Entry.Artifact.get(), Provider.Artifact.get(), Adapter.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &EntrySymbol->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    ASSERT_EQ(Linked.Artifact->Kind, BytecodeArtifactKind::Executable);
    ASSERT_TRUE(writeBytecodeFile(ImagePath, *Linked.Artifact));
    Provider.Artifact.reset();
    Adapter.Artifact.reset();
    Entry.Artifact.reset();
    Linked.Artifact.reset();
    auto Loaded = readBytecodeFile(ImagePath);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const auto Executed = executeArtifact(*Loaded.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
  }

  // Public symbols with the same unqualified name resolve by module identity even when local function IDs collide.
  TEST(BytecodeLinkerTest, DistinguishesEqualNamesFromDifferentModules)
  {
    auto Left = constantObject("left", "value", 19);
    auto Right = constantObject("right", "value", 23);
    ASSERT_TRUE(Left) << Left.Message;
    ASSERT_TRUE(Right) << Right.Message;
    ArtifactContext Test;
    auto *LeftImport = Test.function("leftValue", Test.Int32);
    auto *RightImport = Test.function("rightValue", Test.Int32);
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *First = Test.Builder.createCallInstruction(*LeftImport);
    const auto *Second = Test.Builder.createCallInstruction(*RightImport);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    const auto *Sum = Test.Builder.createAddInstruction(*First, *Second);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Main, "entry", "main"), Test.input(*LeftImport, "left", "value", BytecodeSymbolKind::Import), Test.input(*RightImport, "right", "value", BytecodeSymbolKind::Import)};
    auto Entry = buildBytecodeObject("entry", Test.Bridge, Inputs);
    ASSERT_TRUE(Entry) << Entry.Message;
    const auto *Symbol = findArtifactSymbol(*Entry.Artifact, "entry", "main");
    ASSERT_NE(Symbol, nullptr);
    const BytecodeArtifact *Objects[] = {Left.Artifact.get(), Right.Artifact.get(), Entry.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Symbol->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Executed = executeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
  }

  // Native imports bind private exports by raw name and signature, including direct calls and returned function values after archiving.
  TEST(BytecodeLinkerTest, BindsNativeImportsToPrivateExportBodies)
  {
    auto Provider = constantObject("provider", "inkPrivateNativeLink", 20, BytecodeVisibility::Private, ir::LanguageLinkage::C, ir::FunctionBinding::Export);
    ASSERT_TRUE(Provider) << Provider.Message;
    ArtifactContext Test;
    auto *Native = Test.function("inkPrivateNativeLink", Test.Int32, {}, ir::LanguageLinkage::C, ir::FunctionBinding::Import);
    auto *Unresolved = Test.function("inkUnresolvedNativeLink", Test.Int32, {}, ir::LanguageLinkage::C, ir::FunctionBinding::Import);
    ASSERT_NE(Native, nullptr);
    ASSERT_NE(Unresolved, nullptr);
    auto *Factory = Test.function("factory", Native->type());
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_NE(Factory, nullptr);
    ASSERT_NE(Main, nullptr);
    ASSERT_TRUE(Test.begin(*Factory));
    ASSERT_NE(Test.Builder.createReturnInstruction(Native), nullptr);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *Direct = Test.Builder.createCallInstruction(*Native);
    const auto *Value = Test.Builder.createCallInstruction(*Factory);
    ASSERT_NE(Direct, nullptr);
    ASSERT_NE(Value, nullptr);
    const auto *Indirect = Test.Builder.createCallInstruction(*Value);
    ASSERT_NE(Indirect, nullptr);
    const auto *Sum = Test.Builder.createAddInstruction(*Direct, *Indirect);
    ASSERT_NE(Sum, nullptr);
    const auto *Result = Test.Builder.createAddInstruction(*Sum, Test.integer(2));
    ASSERT_NE(Result, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Result), nullptr);
    const BytecodeFunctionInput Inputs[] = {
        Test.input(*Native, "C", "inkPrivateNativeLink", BytecodeSymbolKind::Native),
        Test.input(*Unresolved, "C", "inkUnresolvedNativeLink", BytecodeSymbolKind::Native),
        Test.input(*Factory, "consumer", "factory"),
        Test.input(*Main, "consumer", "main"),
    };
    auto Consumer = buildBytecodeObject("consumer", Test.Bridge, Inputs);
    ASSERT_TRUE(Consumer) << Consumer.Message;
    const auto *Entry = findArtifactSymbol(*Consumer.Artifact, "consumer", "main");
    ASSERT_NE(Entry, nullptr);
    for (bool Reverse : {false, true})
    {
      const BytecodeArtifact *Objects[] = {Reverse ? Consumer.Artifact.get() : Provider.Artifact.get(), Reverse ? Provider.Artifact.get() : Consumer.Artifact.get()};
      auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
      ASSERT_TRUE(Linked) << Linked.Message;
      EXPECT_EQ(findArtifactSymbol(*Linked.Artifact, "C", "inkPrivateNativeLink"), nullptr);
      const auto *Remaining = findArtifactSymbol(*Linked.Artifact, "C", "inkUnresolvedNativeLink");
      ASSERT_NE(Remaining, nullptr);
      EXPECT_TRUE(Linked.Artifact->Image.Descriptors.at(Remaining->Function).External);
      const auto Serialized = serializeBytecodeArtifact(*Linked.Artifact);
      ASSERT_TRUE(Serialized) << Serialized.Message;
      auto Loaded = deserializeBytecodeArtifact(Serialized.Bytes);
      ASSERT_TRUE(Loaded) << Loaded.Message;
      const auto Executed = executeArtifact(*Loaded.Artifact);
      ASSERT_TRUE(Executed);
      EXPECT_EQ(Executed.Value.Bits, 42U);
    }
  }

  // A matching native name with an incompatible signature is a link error rather than a fallback to an unrelated host implementation.
  TEST(BytecodeLinkerTest, RejectsNativeImportExportSignatureMismatch)
  {
    auto Provider = constantObject("provider", "inkNativeSignature", 42, BytecodeVisibility::Private, ir::LanguageLinkage::C, ir::FunctionBinding::Export);
    ASSERT_TRUE(Provider) << Provider.Message;
    ArtifactContext Test;
    const auto *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(64, true);
    auto *Native = Test.function("inkNativeSignature", *Wide, {}, ir::LanguageLinkage::C, ir::FunctionBinding::Import);
    ASSERT_NE(Native, nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Native, "C", "inkNativeSignature", BytecodeSymbolKind::Native)};
    auto Consumer = buildBytecodeObject("consumer", Test.Bridge, Inputs);
    ASSERT_TRUE(Consumer) << Consumer.Message;
    const BytecodeArtifact *Objects[] = {Provider.Artifact.get(), Consumer.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(Objects).Status, BytecodeStatus::SignatureMismatch);
  }

  // C ABI alone does not publish a native symbol capable of satisfying an external import.
  TEST(BytecodeLinkerTest, DoesNotBindNativeImportsToUnexportedCAbiDefinitions)
  {
    auto Provider = constantObject("provider", "inkUnexportedC", 42, BytecodeVisibility::Public, ir::LanguageLinkage::C);
    ASSERT_TRUE(Provider) << Provider.Message;
    ArtifactContext Test;
    auto *Native = Test.function("inkUnexportedC", Test.Int32, {}, ir::LanguageLinkage::C, ir::FunctionBinding::Import);
    ASSERT_NE(Native, nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Native, "C", "inkUnexportedC", BytecodeSymbolKind::Native)};
    auto Consumer = buildBytecodeObject("consumer", Test.Bridge, Inputs);
    ASSERT_TRUE(Consumer) << Consumer.Message;
    const BytecodeArtifact *Objects[] = {Provider.Artifact.get(), Consumer.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto *Imported = findArtifactSymbol(*Linked.Artifact, "C", "inkUnexportedC");
    ASSERT_NE(Imported, nullptr);
    EXPECT_TRUE(Linked.Artifact->Image.Descriptors.at(Imported->Function).External);
  }

  // Native export names share one binary namespace, including definitions private to different Ink modules.
  TEST(BytecodeLinkerTest, RejectsDuplicatePrivateNativeExportsAcrossModules)
  {
    auto Left = constantObject("left", "inkRepeatedNativeExport", 19, BytecodeVisibility::Private, ir::LanguageLinkage::C, ir::FunctionBinding::Export);
    auto Right = constantObject("right", "inkRepeatedNativeExport", 23, BytecodeVisibility::Private, ir::LanguageLinkage::C, ir::FunctionBinding::Export);
    ASSERT_TRUE(Left) << Left.Message;
    ASSERT_TRUE(Right) << Right.Message;
    const BytecodeArtifact *Objects[] = {Left.Artifact.get(), Right.Artifact.get()};
    const auto Result = linkBytecodeArtifacts(Objects);
    EXPECT_EQ(Result.Status, BytecodeStatus::DuplicateSymbol);
    EXPECT_NE(Result.Message.find("inkRepeatedNativeExport"), std::string::npos);
    const BytecodeArtifact *Reversed[] = {Right.Artifact.get(), Left.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(Reversed).Status, BytecodeStatus::DuplicateSymbol);
  }

  // Ordinary C ABI definitions do not publish native names and may repeat the same spelling in separate modules.
  TEST(BytecodeLinkerTest, AllowsRepeatedNamesForLocalCAbiDefinitions)
  {
    auto Left = constantObject("left", "localC", 19, BytecodeVisibility::Private, ir::LanguageLinkage::C);
    auto Right = constantObject("right", "localC", 23, BytecodeVisibility::Private, ir::LanguageLinkage::C);
    ASSERT_TRUE(Left) << Left.Message;
    ASSERT_TRUE(Right) << Right.Message;
    const BytecodeArtifact *Objects[] = {Left.Artifact.get(), Right.Artifact.get()};
    const auto Result = linkBytecodeArtifacts(Objects);
    ASSERT_TRUE(Result) << Result.Message;
    EXPECT_EQ(Result.Artifact->Image.Functions.size(), 2U);
  }

  // Equal parameter and result types cannot hide disagreement between an imported Ink ABI and a C ABI definition.
  TEST(BytecodeLinkerTest, RejectsModuleImportAbiMismatches)
  {
    auto Provider = constantObject("library", "value", 42, BytecodeVisibility::Public, ir::LanguageLinkage::C);
    auto Consumer = forwardingObject("entry", "main", "library", "value");
    ASSERT_TRUE(Provider) << Provider.Message;
    ASSERT_TRUE(Consumer) << Consumer.Message;
    const BytecodeArtifact *Objects[] = {Provider.Artifact.get(), Consumer.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(Objects).Status, BytecodeStatus::SignatureMismatch);
  }

  // Module exports bind between objects of one module but reject a consumer from a different module.
  TEST(BytecodeLinkerTest, RestrictsModuleSymbolsToTheirModule)
  {
    auto Provider = constantObject("shared", "value", 40, BytecodeVisibility::Module);
    auto Local = forwardingObject("shared", "main", "shared", "value", 2);
    auto Foreign = forwardingObject("foreign", "main", "shared", "value");
    ASSERT_TRUE(Provider) << Provider.Message;
    ASSERT_TRUE(Local) << Local.Message;
    ASSERT_TRUE(Foreign) << Foreign.Message;
    const auto *Entry = findArtifactSymbol(*Local.Artifact, "shared", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *LocalObjects[] = {Provider.Artifact.get(), Local.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(LocalObjects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Executed = executeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
    const BytecodeArtifact *ForeignObjects[] = {Provider.Artifact.get(), Foreign.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(ForeignObjects).Status, BytecodeStatus::InvisibleSymbol);
  }

  // A private definition is usable inside its own object but cannot satisfy another object's import, including in the same module.
  TEST(BytecodeLinkerTest, KeepsPrivateBindingsWithinTheirObject)
  {
    ArtifactContext Test;
    auto *Hidden = Test.function("hidden", Test.Int32);
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Hidden));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(42)), nullptr);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *Call = Test.Builder.createCallInstruction(*Hidden);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Hidden, "shared", "hidden", BytecodeSymbolKind::Definition, BytecodeVisibility::Private), Test.input(*Main, "shared", "main")};
    auto Provider = buildBytecodeObject("shared", Test.Bridge, Inputs);
    ASSERT_TRUE(Provider) << Provider.Message;
    const auto *Entry = findArtifactSymbol(*Provider.Artifact, "shared", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *OnlyProvider[] = {Provider.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(OnlyProvider, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Executed = executeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
    auto Consumer = forwardingObject("shared", "other", "shared", "hidden");
    ASSERT_TRUE(Consumer) << Consumer.Message;
    const BytecodeArtifact *Objects[] = {Provider.Artifact.get(), Consumer.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(Objects).Status, BytecodeStatus::InvisibleSymbol);
  }

  // Linking reports duplicate exported definitions and unresolved imports before an executable can escape.
  TEST(BytecodeLinkerTest, RejectsDuplicateAndMissingSymbols)
  {
    auto First = constantObject("library", "value", 1);
    auto Second = constantObject("library", "value", 2);
    auto Missing = forwardingObject("entry", "main", "absent", "value");
    ASSERT_TRUE(First) << First.Message;
    ASSERT_TRUE(Second) << Second.Message;
    ASSERT_TRUE(Missing) << Missing.Message;
    const BytecodeArtifact *DuplicateObjects[] = {First.Artifact.get(), Second.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(DuplicateObjects).Status, BytecodeStatus::DuplicateSymbol);
    const BytecodeArtifact *MissingObjects[] = {Missing.Artifact.get()};
    auto Result = linkBytecodeArtifacts(MissingObjects);
    EXPECT_EQ(Result.Status, BytecodeStatus::MissingSymbol);
    EXPECT_EQ(Result.Artifact, nullptr);
  }

  // An import with the right module and name but a different function signature must not bind to an incompatible definition.
  TEST(BytecodeLinkerTest, RejectsMismatchedImportedSignature)
  {
    auto Consumer = forwardingObject("entry", "main", "library", "value");
    ASSERT_TRUE(Consumer) << Consumer.Message;
    ArtifactContext Test;
    const ir::Type *Parameters[] = {&Test.Int32};
    auto *Function = Test.function("value", Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(Function->parameters()[0].get()), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "library", "value")};
    auto Provider = buildBytecodeObject("library", Test.Bridge, Inputs);
    ASSERT_TRUE(Provider) << Provider.Message;
    const BytecodeArtifact *Objects[] = {Provider.Artifact.get(), Consumer.Artifact.get()};
    EXPECT_EQ(linkBytecodeArtifacts(Objects).Status, BytecodeStatus::SignatureMismatch);
  }

  // Equal private identities in separate objects remain distinct and each public wrapper reaches its own helper.
  TEST(BytecodeLinkerTest, IsolatesSameNamedPrivateDefinitionsAcrossObjects)
  {
    std::vector<std::unique_ptr<BytecodeArtifact>> Providers;
    for (std::uint64_t Index = 0; Index < 2; ++Index)
    {
      ArtifactContext Test;
      const std::string Name = Index == 0 ? "first" : "second";
      auto *Hidden = Test.function("hidden", Test.Int32);
      auto *Wrapper = Test.function(Name, Test.Int32);
      ASSERT_TRUE(Test.begin(*Hidden));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(19 + 4 * Index)), nullptr);
      ASSERT_TRUE(Test.begin(*Wrapper));
      const auto *Call = Test.Builder.createCallInstruction(*Hidden);
      ASSERT_NE(Call, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Hidden, "library", "hidden", BytecodeSymbolKind::Definition, BytecodeVisibility::Private), Test.input(*Wrapper, "library", Name)};
      auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      Providers.push_back(std::move(Object.Artifact));
    }
    ArtifactContext Test;
    auto *First = Test.function("first", Test.Int32);
    auto *Second = Test.function("second", Test.Int32);
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *FirstValue = Test.Builder.createCallInstruction(*First);
    const auto *SecondValue = Test.Builder.createCallInstruction(*Second);
    ASSERT_NE(FirstValue, nullptr);
    ASSERT_NE(SecondValue, nullptr);
    const auto *Sum = Test.Builder.createAddInstruction(*FirstValue, *SecondValue);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Main, "entry", "main"), Test.input(*First, "library", "first", BytecodeSymbolKind::Import), Test.input(*Second, "library", "second", BytecodeSymbolKind::Import)};
    auto Consumer = buildBytecodeObject("entry", Test.Bridge, Inputs);
    ASSERT_TRUE(Consumer) << Consumer.Message;
    const auto *Entry = findArtifactSymbol(*Consumer.Artifact, "entry", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Providers[0].get(), Providers[1].get(), Consumer.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Executed = executeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
  }

  // Functions sharing a qualified name remain separate overloads when their parameter signatures differ.
  TEST(BytecodeLinkerTest, ResolvesOverloadedSignaturesIndependently)
  {
    std::unique_ptr<BytecodeArtifact> Provider;
    {
      ArtifactContext Test;
      const ir::Type *Parameters[] = {&Test.Bool};
      auto *Zero = Test.function("zero", Test.Int32);
      auto *One = Test.function("one", Test.Int32, Parameters);
      ASSERT_TRUE(Test.begin(*Zero));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(17)), nullptr);
      ASSERT_TRUE(Test.begin(*One));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(25)), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Zero, "library", "overload"), Test.input(*One, "library", "overload")};
      auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      Provider = std::move(Object.Artifact);
    }
    ArtifactContext Test;
    Test.Bridge.lowerType(Test.Int32);
    const ir::Type *Parameters[] = {&Test.Bool};
    auto *Zero = Test.function("zero", Test.Int32);
    auto *One = Test.function("one", Test.Int32, Parameters);
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Main));
    const auto *First = Test.Builder.createCallInstruction(*Zero);
    const ir::Value *Arguments[] = {&Test.Context.constantPool().getBoolConstant(true)};
    const auto *Second = Test.Builder.createCallInstruction(*One, Arguments);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    const auto *Sum = Test.Builder.createAddInstruction(*First, *Second);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Main, "entry", "main"), Test.input(*Zero, "library", "overload", BytecodeSymbolKind::Import), Test.input(*One, "library", "overload", BytecodeSymbolKind::Import)};
    auto Consumer = buildBytecodeObject("entry", Test.Bridge, Inputs);
    ASSERT_TRUE(Consumer) << Consumer.Message;
    const auto *Entry = findArtifactSymbol(*Consumer.Artifact, "entry", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Provider.get(), Consumer.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Executed = executeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
  }

  // Closed generic symbols distinguish type arguments from constant arguments and retain exact constant bits across archives.
  TEST(BytecodeLinkerTest, DistinguishesGenericArgumentKindsTypesAndValues)
  {
    std::unique_ptr<BytecodeArtifact> Provider;
    {
      ArtifactContext Test;
      const auto IntegerId = Test.Bridge.lowerType(Test.Int32);
      const auto BooleanId = Test.Bridge.lowerType(Test.Bool);
      const std::string Integer = bytecodeTypeIdentity(*Test.Bridge.types(), IntegerId);
      const std::string Boolean = bytecodeTypeIdentity(*Test.Bridge.types(), BooleanId);
      const BytecodeGenericArgument Arguments[] = {
          {BytecodeGenericArgumentKind::Type, Integer, {}},
          {BytecodeGenericArgumentKind::Type, Boolean, {}},
          {BytecodeGenericArgumentKind::Constant, Integer, "00000001"},
          {BytecodeGenericArgumentKind::Constant, Integer, "00000002"},
      };
      std::vector<BytecodeFunctionInput> Inputs;
      for (std::size_t Index = 0; Index < std::size(Arguments); ++Index)
      {
        auto *Function = Test.function("instance" + std::to_string(Index), Test.Int32);
        ASSERT_TRUE(Test.begin(*Function));
        ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(1U << Index)), nullptr);
        Inputs.push_back(Test.input(*Function, "library", "choose", BytecodeSymbolKind::Definition, BytecodeVisibility::Public, {Arguments[Index]}));
      }
      auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
      ASSERT_TRUE(Serialized) << Serialized.Message;
      auto Loaded = deserializeBytecodeArtifact(Serialized.Bytes);
      ASSERT_TRUE(Loaded) << Loaded.Message;
      Provider = std::move(Loaded.Artifact);
    }
    ArtifactContext Test;
    const auto BooleanId = Test.Bridge.lowerType(Test.Bool);
    const auto IntegerId = Test.Bridge.lowerType(Test.Int32);
    const std::string Integer = bytecodeTypeIdentity(*Test.Bridge.types(), IntegerId);
    const std::string Boolean = bytecodeTypeIdentity(*Test.Bridge.types(), BooleanId);
    const BytecodeGenericArgument Arguments[] = {
        {BytecodeGenericArgumentKind::Type, Integer, {}},
        {BytecodeGenericArgumentKind::Type, Boolean, {}},
        {BytecodeGenericArgumentKind::Constant, Integer, "00000001"},
        {BytecodeGenericArgumentKind::Constant, Integer, "00000002"},
    };
    auto *Main = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Main));
    std::vector<BytecodeFunctionInput> Inputs{Test.input(*Main, "entry", "main")};
    const ir::Value *Sum = &Test.integer(0);
    for (std::size_t Index = 0; Index < std::size(Arguments); ++Index)
    {
      auto *Import = Test.function("import" + std::to_string(Index), Test.Int32);
      Inputs.push_back(Test.input(*Import, "library", "choose", BytecodeSymbolKind::Import, BytecodeVisibility::Public, {Arguments[Index]}));
      const auto *Call = Test.Builder.createCallInstruction(*Import);
      ASSERT_NE(Call, nullptr);
      Sum = Test.Builder.createAddInstruction(*Sum, *Call);
      ASSERT_NE(Sum, nullptr);
    }
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    auto Consumer = buildBytecodeObject("entry", Test.Bridge, Inputs);
    ASSERT_TRUE(Consumer) << Consumer.Message;
    const auto *Entry = findArtifactSymbol(*Consumer.Artifact, "entry", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Provider.get(), Consumer.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Executed = executeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 15U);
  }
} // namespace ink::execution::test
