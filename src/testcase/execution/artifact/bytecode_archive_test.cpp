#include "artifact_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <limits>

#ifdef _WIN32
#define INK_ARTIFACT_EXPORT extern "C" __declspec(dllexport)
#else
#define INK_ARTIFACT_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace ink::execution::test
{
  INK_ARTIFACT_EXPORT std::int32_t inkTestArchivedCString(unsigned char *Value) noexcept
  {
    const auto First = Value[0];
    Value[0] = 0;
    return First;
  }

  // Archive bytes do not depend on unordered-map buckets and round trips preserve the canonical representation.
  TEST(BytecodeArchiveTest, ProducesDeterministicBytesAcrossHashLayoutsAndRoundTrips)
  {
    ArtifactContext Test;
    std::vector<BytecodeFunctionInput> Inputs;
    for (std::size_t Index = 0; Index < 5; ++Index)
    {
      const std::string Name = "constant" + std::to_string(Index);
      auto *Function = Test.function(Name, Test.Int32);
      ASSERT_TRUE(Test.begin(*Function));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(Index * 11)), nullptr);
      Inputs.push_back(Test.input(*Function, "library", Name));
    }
    auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
    ASSERT_TRUE(Object) << Object.Message;
    const auto First = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(First) << First.Message;
    Object.Artifact->Image.Descriptors.rehash(97);
    Object.Artifact->Image.Functions.rehash(193);
    const auto Second = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Second) << Second.Message;
    EXPECT_EQ(First.Bytes, Second.Bytes);
    auto Loaded = deserializeBytecodeArtifact(First.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    EXPECT_NE(Loaded.Artifact->Image.Layouts->domain(), Object.Artifact->Image.Layouts->domain());
    const auto Third = serializeBytecodeArtifact(*Loaded.Artifact);
    ASSERT_TRUE(Third) << Third.Message;
    EXPECT_EQ(Third.Bytes, First.Bytes);
  }

  // Local C ABI functions and private native exports retain independent metadata and execute their own bodies after reloading.
  TEST(BytecodeArchiveTest, PreservesLocalCAbiAndPrivateNativeExportBodies)
  {
    std::string ObjectBytes;
    {
      ArtifactContext Test;
      auto *Local = Test.function("inkBytecodeLocalC", Test.Int32, {}, ir::LanguageLinkage::C);
      auto *Exported = Test.function("inkBytecodeNativeExport", Test.Int32, {}, ir::LanguageLinkage::C, core::FunctionBinding::Export);
      auto *Main = Test.function("main", Test.Int32);
      ASSERT_NE(Local, nullptr);
      ASSERT_NE(Exported, nullptr);
      ASSERT_NE(Main, nullptr);
      ASSERT_TRUE(Test.begin(*Local));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(19)), nullptr);
      ASSERT_TRUE(Test.begin(*Exported));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.integer(23)), nullptr);
      ASSERT_TRUE(Test.begin(*Main));
      const auto *First = Test.Builder.createCallInstruction(*Local);
      const auto *Second = Test.Builder.createCallInstruction(*Exported);
      ASSERT_NE(First, nullptr);
      ASSERT_NE(Second, nullptr);
      const auto *Sum = Test.Builder.createAddInstruction(*First, *Second);
      ASSERT_NE(Sum, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Local, "library", "inkBytecodeLocalC"), Test.input(*Exported, "library", "inkBytecodeNativeExport", BytecodeSymbolKind::Definition, BytecodeVisibility::Private), Test.input(*Main, "library", "main")};
      auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      const auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
      ASSERT_TRUE(Serialized) << Serialized.Message;
      ObjectBytes = Serialized.Bytes;
    }
    auto Object = deserializeBytecodeArtifact(ObjectBytes);
    ASSERT_TRUE(Object) << Object.Message;
    for (std::string_view Name : {"inkBytecodeLocalC", "inkBytecodeNativeExport"})
    {
      const auto *Symbol = findArtifactSymbol(*Object.Artifact, "library", Name);
      ASSERT_NE(Symbol, nullptr);
      EXPECT_EQ(Symbol->Kind, BytecodeSymbolKind::Definition);
      const auto &Descriptor = Object.Artifact->Image.Descriptors.at(Symbol->Function);
      EXPECT_EQ(Descriptor.Symbol, Name);
      EXPECT_TRUE(Descriptor.CAbi);
      EXPECT_FALSE(Descriptor.External);
      EXPECT_EQ(Descriptor.Exported, Name == "inkBytecodeNativeExport");
      EXPECT_EQ(Symbol->Visibility, Descriptor.Exported ? BytecodeVisibility::Private : BytecodeVisibility::Public);
      EXPECT_TRUE(Object.Artifact->Image.Functions.contains(Symbol->Function));
    }
    const auto *Entry = findArtifactSymbol(*Object.Artifact, "library", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Object.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    const auto Serialized = serializeBytecodeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Serialized) << Serialized.Message;
    auto Loaded = deserializeBytecodeArtifact(Serialized.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const auto *Export = findArtifactSymbol(*Loaded.Artifact, "library", "inkBytecodeNativeExport");
    ASSERT_NE(Export, nullptr);
    EXPECT_TRUE(Loaded.Artifact->Image.Descriptors.at(Export->Function).Exported);
    const auto Executed = executeArtifact(*Loaded.Artifact);
    ASSERT_TRUE(Executed);
    EXPECT_EQ(Executed.Value.Bits, 42U);
  }

  // Every proper prefix of a valid archive is rejected and never exposes a partially decoded image.
  TEST(BytecodeArchiveTest, RejectsTruncationAtEveryByteBoundary)
  {
    auto Object = constantObject("library", "value", 42);
    ASSERT_TRUE(Object) << Object.Message;
    const auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Serialized) << Serialized.Message;
    ASSERT_FALSE(Serialized.Bytes.empty());
    for (std::size_t Size = 0; Size < Serialized.Bytes.size(); ++Size)
    {
      SCOPED_TRACE(Size);
      auto Result = deserializeBytecodeArtifact(std::string_view(Serialized.Bytes).substr(0, Size));
      EXPECT_FALSE(Result);
      EXPECT_EQ(Result.Artifact, nullptr);
    }
  }

  // An archive cannot conceal extra records after its declared payload or pass with a damaged magic header.
  TEST(BytecodeArchiveTest, RejectsBadMagicAndTrailingData)
  {
    auto Object = constantObject("library", "value", 42);
    ASSERT_TRUE(Object) << Object.Message;
    auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Serialized) << Serialized.Message;
    std::string Damaged = Serialized.Bytes;
    ASSERT_FALSE(Damaged.empty());
    Damaged[0] ^= 0x7f;
    EXPECT_EQ(deserializeBytecodeArtifact(Damaged).Status, BytecodeStatus::InvalidFormat);
    Serialized.Bytes.push_back('\0');
    EXPECT_EQ(deserializeBytecodeArtifact(Serialized.Bytes).Status, BytecodeStatus::InvalidFormat);
  }

  // Unknown container and instruction schema versions are rejected separately before any executable records are accepted.
  TEST(BytecodeArchiveTest, RejectsUnknownContainerAndInstructionVersions)
  {
    auto Object = constantObject("library", "value", 42);
    ASSERT_TRUE(Object) << Object.Message;
    const auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Serialized) << Serialized.Message;
    ASSERT_GE(Serialized.Bytes.size(), 16U);
    ASSERT_EQ(Serialized.Bytes.substr(0, 8), std::string("INKBC\0\r\n", 8));
    for (std::size_t Offset : {8U, 12U})
    {
      SCOPED_TRACE(Offset);
      std::string Damaged = Serialized.Bytes;
      std::fill_n(Damaged.begin() + Offset, 4, static_cast<char>(0xff));
      auto Result = deserializeBytecodeArtifact(Damaged);
      EXPECT_EQ(Result.Status, BytecodeStatus::UnsupportedVersion);
      EXPECT_EQ(Result.Artifact, nullptr);
    }
    std::string Previous = Serialized.Bytes;
    Previous[8] = 1;
    std::fill_n(Previous.begin() + 9, 3, '\0');
    EXPECT_EQ(deserializeBytecodeArtifact(Previous).Status, BytecodeStatus::UnsupportedVersion);
  }

  // A foreign target is rejected at serialization and at the file boundary instead of executing native layouts on the wrong host.
  TEST(BytecodeArchiveTest, RejectsIncompatibleTarget)
  {
    auto Object = constantObject("library", "value", 42);
    ASSERT_TRUE(Object) << Object.Message;
    auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Serialized) << Serialized.Message;
    const std::string Target = Object.Artifact->Target;
    ASSERT_FALSE(Target.empty());
    const auto Position = Serialized.Bytes.find(Target);
    ASSERT_NE(Position, std::string::npos);
    Serialized.Bytes[Position] = Target.front() == 'x' ? 'y' : 'x';
    EXPECT_EQ(deserializeBytecodeArtifact(Serialized.Bytes).Status, BytecodeStatus::IncompatibleTarget);
    Object.Artifact->Target = "foreign-invalid-target";
    EXPECT_EQ(serializeBytecodeArtifact(*Object.Artifact).Status, BytecodeStatus::IncompatibleTarget);
  }

  // Both reading and writing obey explicit archive byte, record, string and allocation budgets.
  TEST(BytecodeArchiveTest, EnforcesResourceBudgetsWithoutReturningPartialArtifacts)
  {
    auto Object = constantObject("library", "value", 42);
    ASSERT_TRUE(Object) << Object.Message;
    const auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
    ASSERT_TRUE(Serialized) << Serialized.Message;
    std::vector<BytecodeLimits> Limits(4);
    Limits[0].MaxBytes = Serialized.Bytes.size() - 1;
    Limits[1].MaxRecords = 0;
    Limits[2].MaxStringBytes = 0;
    Limits[3].MaxAllocationBytes = 0;
    for (std::size_t Index = 0; Index < Limits.size(); ++Index)
    {
      SCOPED_TRACE(Index);
      EXPECT_EQ(serializeBytecodeArtifact(*Object.Artifact, Limits[Index]).Status, BytecodeStatus::LimitExceeded);
      auto Loaded = deserializeBytecodeArtifact(Serialized.Bytes, Limits[Index]);
      EXPECT_EQ(Loaded.Status, BytecodeStatus::LimitExceeded);
      EXPECT_EQ(Loaded.Artifact, nullptr);
    }
    BytecodeLimits Exact;
    Exact.MaxBytes = Serialized.Bytes.size();
    EXPECT_TRUE(serializeBytecodeArtifact(*Object.Artifact, Exact));
    EXPECT_TRUE(deserializeBytecodeArtifact(Serialized.Bytes, Exact));
  }

  // Native host addresses are execution-only even if a pointer initializer would be overwritten before the function returns.
  TEST(BytecodeArchiveTest, RejectsEmbeddedHostPointers)
  {
    ArtifactContext Test;
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.Int32, ir::AccessKind::ReadWrite);
    auto *Function = Test.function("address", *Pointer);
    ASSERT_TRUE(Test.begin(*Function));
    const auto *Address = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Address, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Address), nullptr);
    const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "library", "address")};
    auto Object = buildBytecodeObject("library", Test.Bridge, Inputs);
    ASSERT_TRUE(Object) << Object.Message;
    auto &Code = *Object.Artifact->Image.Functions.begin()->second;
    const auto Slot = std::find_if(Code.SlotTypes.begin(), Code.SlotTypes.end(), [&](RuntimeTypeId Type)
    {
      return Code.Layouts->get(Type)->Kind == RuntimeKind::Pointer;
    });
    ASSERT_NE(Slot, Code.SlotTypes.end());
    int HostValue = 42;
    const auto Index = static_cast<std::size_t>(Slot - Code.SlotTypes.begin());
    Code.InitialSlots[Index] = RuntimeValue::fromPointer(ExecutionPointer::fromNative(&HostValue), *Slot);
    EXPECT_EQ(serializeBytecodeArtifact(*Object.Artifact).Status, BytecodeStatus::UnsupportedConstant);
  }

  // A malformed opcode cannot be serialized as a runnable artifact, even when its surrounding symbol metadata is valid.
  TEST(BytecodeArchiveTest, RejectsInvalidExecutableInstructions)
  {
    auto Object = constantObject("library", "value", 42);
    ASSERT_TRUE(Object) << Object.Message;
    auto &Code = Object.Artifact->Image.Functions.begin()->second->Code;
    ASSERT_FALSE(Code.empty());
    Code.front().Code = static_cast<BytecodeOpcode>(std::numeric_limits<std::uint16_t>::max());
    EXPECT_EQ(serializeBytecodeArtifact(*Object.Artifact).Status, BytecodeStatus::InvalidImage);
  }

  // Missing archive files report an explicit I/O status and cannot be mistaken for empty executable images.
  TEST(BytecodeArchiveTest, ReportsMissingFilesAsIoErrors)
  {
    ArtifactFiles Files;
    ASSERT_TRUE(Files.ready());
    auto Result = readBytecodeFile(Files.file("missing.inko"));
    EXPECT_EQ(Result.Status, BytecodeStatus::IoError);
    EXPECT_EQ(Result.Artifact, nullptr);
  }

  // Wide constants keep all 128 bits through object and executable archives and still carry correctly during later execution.
  TEST(BytecodeArchiveTest, PreservesWideIntegerConstantsWithoutSourceIR)
  {
    std::string ObjectBytes;
    {
      ArtifactContext Test;
      const auto *Wide = Test.Context.typePool().getType<ir::TypeKind::Integer>(128, false);
      auto *Function = Test.function("main", *Wide);
      ASSERT_TRUE(Test.begin(*Function));
      const std::uint64_t Words[] = {0xffffffffffffffffULL, 0x123456789abcdef0ULL};
      const auto *Value = Test.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, Words));
      const auto *One = Test.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, 1));
      ASSERT_NE(Value, nullptr);
      ASSERT_NE(One, nullptr);
      const auto *Sum = Test.Builder.createAddInstruction(*Value, *One);
      ASSERT_NE(Sum, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Function, "wide", "main")};
      auto Object = buildBytecodeObject("wide", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
      ASSERT_TRUE(Serialized) << Serialized.Message;
      ObjectBytes = std::move(Serialized.Bytes);
    }
    auto Object = deserializeBytecodeArtifact(ObjectBytes);
    ASSERT_TRUE(Object) << Object.Message;
    const auto *Entry = findArtifactSymbol(*Object.Artifact, "wide", "main");
    ASSERT_NE(Entry, nullptr);
    const BytecodeArtifact *Objects[] = {Object.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &Entry->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    auto Serialized = serializeBytecodeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Serialized) << Serialized.Message;
    Linked.Artifact.reset();
    Object.Artifact.reset();
    auto Loaded = deserializeBytecodeArtifact(Serialized.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const auto Executed = executeArtifact(*Loaded.Artifact);
    ASSERT_TRUE(Executed);
    ASSERT_EQ(Executed.Value.kind(), RuntimeKind::Integer);
    const std::uint64_t Expected[] = {0, 0x123456789abcdef1ULL};
    EXPECT_EQ(Executed.Value.integer().bits(), ir::IntegerBits(128, Expected));
  }

#if defined(_WIN32) || defined(__linux__)
  // Reloaded CString instructions allocate fresh mutable copies and resolve native symbols again without archiving process addresses.
  TEST(BytecodeArchiveTest, ReloadsCStringBytesAndNativeImportsForFreshCalls)
  {
    std::string ObjectBytes;
    {
      ArtifactContext Test;
      const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
      const auto *Slice = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
      const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Byte, ir::AccessKind::ReadWrite);
      const ir::Type *Parameters[] = {Pointer};
      const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.Int32, Parameters);
      auto Native = Test.Builder.createFunction(Test.Context.namePool().intern("inkTestArchivedCString"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Import);
      ASSERT_NE(Native, nullptr);
      auto *Main = Test.function("main", Test.Int32);
      ASSERT_TRUE(Test.begin(*Main));
      const auto *Constant = Test.Context.constantPool().getStringConstant(*Slice, "payload");
      ASSERT_NE(Constant, nullptr);
      const auto *String = Test.Builder.createCStringInstruction(*Constant);
      ASSERT_NE(String, nullptr);
      const ir::Value *Arguments[] = {String};
      const auto *Call = Test.Builder.createCallInstruction(*Native, Arguments);
      ASSERT_NE(Call, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
      const BytecodeFunctionInput Inputs[] = {Test.input(*Main, "strings", "main"), Test.input(*Native, "C", "inkTestArchivedCString", BytecodeSymbolKind::Native)};
      auto Object = buildBytecodeObject("strings", Test.Bridge, Inputs);
      ASSERT_TRUE(Object) << Object.Message;
      auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
      ASSERT_TRUE(Serialized) << Serialized.Message;
      ObjectBytes = std::move(Serialized.Bytes);
    }
    auto Object = deserializeBytecodeArtifact(ObjectBytes);
    ASSERT_TRUE(Object) << Object.Message;
    const auto *EntrySymbol = findArtifactSymbol(*Object.Artifact, "strings", "main");
    ASSERT_NE(EntrySymbol, nullptr);
    const BytecodeArtifact *Objects[] = {Object.Artifact.get()};
    auto Linked = linkBytecodeArtifacts(Objects, &EntrySymbol->Identity);
    ASSERT_TRUE(Linked) << Linked.Message;
    auto Serialized = serializeBytecodeArtifact(*Linked.Artifact);
    ASSERT_TRUE(Serialized) << Serialized.Message;
    auto Loaded = deserializeBytecodeArtifact(Serialized.Bytes);
    ASSERT_TRUE(Loaded) << Loaded.Message;
    const auto Entry = Loaded.Artifact->Entry;
    core::CompilationContext Compilation;
    ir::IRContext EmptyContext(Compilation);
    ExecutionEngine Engine(EmptyContext);
    ExecutionLinker Linker(std::move(Loaded.Artifact->Image));
    ExecutionMachine Machine(Engine, Linker);
    for (int Call = 0; Call < 2; ++Call)
    {
      const auto Result = Machine.execute(Entry, {});
      ASSERT_TRUE(Result);
      EXPECT_EQ(Result.Value.Bits, static_cast<std::uint64_t>('p'));
      EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
    }
  }
#endif
} // namespace ink::execution::test

#undef INK_ARTIFACT_EXPORT
