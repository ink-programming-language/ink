#include "artifact_test_support.h"

#include <gtest/gtest.h>

#include <limits>

namespace ink::execution::test
{
  namespace
  {
    std::shared_ptr<RuntimeTypeTable> extendLayouts(BytecodeArtifact &Artifact)
    {
      auto Types = std::make_shared<RuntimeTypeTable>();
      for (std::size_t Index = 0; Index < Artifact.Image.Layouts->size(); ++Index)
      {
        Types->append(*Artifact.Image.Layouts->get(static_cast<RuntimeTypeId>(Index)));
      }
      Artifact.Image.Layouts = Types;
      for (auto &[Id, Function] : Artifact.Image.Functions)
      {
        Function->Layouts = Types;
      }
      return Types;
    }

    const StorageLayout *integerLayout(const BytecodeArtifact &Artifact)
    {
      for (std::size_t Index = 0; Index < Artifact.Image.Layouts->size(); ++Index)
      {
        const auto *Layout = Artifact.Image.Layouts->get(static_cast<RuntimeTypeId>(Index));
        if (Layout->Kind == RuntimeKind::Integer)
        {
          return Layout;
        }
      }
      return nullptr;
    }

    StorageLayout pointerLayout(RuntimeTypeId Pointee)
    {
      StorageLayout Layout;
      Layout.Kind = RuntimeKind::Pointer;
      Layout.Size = sizeof(void *);
      Layout.Alignment = sizeof(void *);
      Layout.Pointee = Pointee;
      Layout.Writable = true;
      return Layout;
    }
  } // namespace

  // Forged C ABI, import and export flags cannot turn an ordinary bytecode body into a native boundary.
  TEST(BytecodeValidationTest, RejectsInconsistentNativeBoundaryMetadata)
  {
    for (int Case = 0; Case < 6; ++Case)
    {
      SCOPED_TRACE(Case);
      auto Object = constantObject("library", "nativeValue", 42, BytecodeVisibility::Private, ir::LanguageLinkage::C, ir::FunctionBinding::Export);
      ASSERT_TRUE(Object) << Object.Message;
      auto &Descriptor = Object.Artifact->Image.Descriptors.begin()->second;
      if (Case == 0)
      {
        Descriptor.CAbi = false;
      }
      else if (Case == 1)
      {
        Descriptor.External = true;
      }
      else if (Case == 2)
      {
        Descriptor.NativeAbi = false;
      }
      else if (Case == 3)
      {
        Descriptor.Supported = false;
      }
      else if (Case == 4)
      {
        Descriptor.Symbol.clear();
      }
      else
      {
        Object.Artifact->Symbols.front().Kind = BytecodeSymbolKind::Import;
        Object.Artifact->Image.Functions.clear();
      }
      EXPECT_EQ(validateBytecodeArtifact(*Object.Artifact).Status, BytecodeStatus::InvalidImage);
      EXPECT_FALSE(serializeBytecodeArtifact(*Object.Artifact));
    }
  }

  // Even unused layouts are checked for zero or excessive widths, invalid target sizes and alignments, and false native representation claims.
  TEST(BytecodeValidationTest, RejectsMalformedAndOversizedUnusedLayouts)
  {
    for (int Case = 0; Case < 7; ++Case)
    {
      SCOPED_TRACE(Case);
      auto Object = constantObject("library", "value", 42);
      ASSERT_TRUE(Object) << Object.Message;
      const auto *Integer = integerLayout(*Object.Artifact);
      ASSERT_NE(Integer, nullptr);
      StorageLayout Layout = *Integer;
      if (Case == 0)
      {
        Layout.BitWidth = 0;
        Layout.Size = 0;
        Layout.Alignment = 1;
        Layout.Native = false;
      }
      else if (Case == 1)
      {
        Layout.BitWidth = std::numeric_limits<std::uint32_t>::max();
        Layout.Size = (static_cast<std::uint64_t>(Layout.BitWidth) + 7) / 8;
        Layout.Alignment = 1;
        Layout.Native = false;
      }
      else if (Case == 2)
      {
        Layout.Size = 1;
      }
      else if (Case == 3)
      {
        Layout.Alignment = 0;
      }
      else if (Case == 4)
      {
        Layout.Alignment = 3;
      }
      else if (Case == 5)
      {
        Layout.Native = false;
      }
      else
      {
        Layout.Kind = static_cast<RuntimeKind>(255);
      }
      auto Types = extendLayouts(*Object.Artifact);
      ASSERT_NE(Types->append(std::move(Layout)), InvalidRuntimeType);
      EXPECT_FALSE(validateBytecodeArtifact(*Object.Artifact));
      const auto Serialized = serializeBytecodeArtifact(*Object.Artifact);
      EXPECT_FALSE(Serialized);
      EXPECT_TRUE(Serialized.Bytes.empty());
    }
  }

  // Missing pointee types and self-referential type graphs fail before symbol resolution or runtime allocation.
  TEST(BytecodeValidationTest, RejectsMissingAndCyclicTypeReferences)
  {
    for (bool Cycle : {false, true})
    {
      SCOPED_TRACE(Cycle);
      auto Object = constantObject("library", "value", 42);
      ASSERT_TRUE(Object) << Object.Message;
      auto Types = extendLayouts(*Object.Artifact);
      const auto Pointee = Cycle ? static_cast<RuntimeTypeId>(Types->size()) : InvalidRuntimeType;
      ASSERT_NE(Types->append(pointerLayout(Pointee)), InvalidRuntimeType);
      EXPECT_FALSE(validateBytecodeArtifact(*Object.Artifact));
      EXPECT_FALSE(serializeBytecodeArtifact(*Object.Artifact));
    }
  }

  // A valid but deeply nested unused pointer type is subject to the caller's depth budget rather than host recursion limits.
  TEST(BytecodeValidationTest, EnforcesTypeDepthOnUnusedLayouts)
  {
    auto Object = constantObject("library", "value", 42);
    ASSERT_TRUE(Object) << Object.Message;
    const auto *Integer = integerLayout(*Object.Artifact);
    ASSERT_NE(Integer, nullptr);
    auto Pointee = Integer->Type;
    auto Types = extendLayouts(*Object.Artifact);
    for (std::size_t Index = 0; Index < 12; ++Index)
    {
      Pointee = Types->append(pointerLayout(Pointee));
      ASSERT_NE(Pointee, InvalidRuntimeType);
    }
    ASSERT_TRUE(validateBytecodeArtifact(*Object.Artifact));
    BytecodeLimits Limits;
    Limits.MaxTypeDepth = 4;
    EXPECT_EQ(validateBytecodeArtifact(*Object.Artifact, Limits).Status, BytecodeStatus::LimitExceeded);
    EXPECT_EQ(serializeBytecodeArtifact(*Object.Artifact, Limits).Status, BytecodeStatus::LimitExceeded);
  }

  // An unused function-valued constant still has to name a real function with the exact stored signature.
  TEST(BytecodeValidationTest, RejectsForgedFunctionConstantsInUnusedSlots)
  {
    auto Object = constantObject("library", "value", 42);
    ASSERT_TRUE(Object) << Object.Message;
    auto &Function = *Object.Artifact->Image.Functions.begin()->second;
    Function.SlotTypes.push_back(Function.Signature);
    Function.InitialSlots.push_back(RuntimeValue::fromBits(Function.Id, Function.Signature));
    ASSERT_TRUE(validateBytecodeArtifact(*Object.Artifact));
    Function.InitialSlots.back().Bits = InvalidFunction;
    EXPECT_EQ(validateBytecodeArtifact(*Object.Artifact).Status, BytecodeStatus::InvalidImage);
    EXPECT_FALSE(serializeBytecodeArtifact(*Object.Artifact));
  }

  // Calls outside the executed instruction stream cannot hide missing targets, invalid indirect slots or incompatible argument records.
  TEST(BytecodeValidationTest, RejectsInvalidUnreferencedCallRecords)
  {
    for (int Case = 0; Case < 3; ++Case)
    {
      SCOPED_TRACE(Case);
      auto Object = constantObject("library", "value", 42);
      ASSERT_TRUE(Object) << Object.Message;
      auto &Function = *Object.Artifact->Image.Functions.begin()->second;
      ExecutionCallSite Call;
      Call.Target = Function.Id;
      Call.Signature = Function.Signature;
      if (Case == 0)
      {
        Call.Target = InvalidFunction - 1;
      }
      else if (Case == 1)
      {
        Call.Target = InvalidFunction;
        Call.CalleeSlot = InvalidSlot;
      }
      else
      {
        Call.Arguments.push_back(InvalidSlot);
      }
      Function.Calls.push_back(std::move(Call));
      EXPECT_EQ(validateBytecodeArtifact(*Object.Artifact).Status, BytecodeStatus::InvalidImage);
      EXPECT_FALSE(serializeBytecodeArtifact(*Object.Artifact));
    }
  }

  // Generic identities reject type arguments carrying values, noncanonical integer encodings, unknown types and invalid argument tags.
  TEST(BytecodeValidationTest, RejectsNoncanonicalGenericArguments)
  {
    for (int Case = 0; Case < 7; ++Case)
    {
      SCOPED_TRACE(Case);
      auto Object = constantObject("library", "value", 42);
      ASSERT_TRUE(Object) << Object.Message;
      const auto *Integer = integerLayout(*Object.Artifact);
      ASSERT_NE(Integer, nullptr);
      BytecodeGenericArgument Argument{BytecodeGenericArgumentKind::Constant, bytecodeTypeIdentity(*Object.Artifact->Image.Layouts, Integer->Type), "00000001"};
      if (Case == 0)
      {
        Argument.Kind = BytecodeGenericArgumentKind::Type;
      }
      else if (Case == 1)
      {
        Argument.Value = "1";
      }
      else if (Case == 2)
      {
        Argument.Value = "0000000A";
      }
      else if (Case == 3)
      {
        Argument.Value = "0x000001";
      }
      else if (Case == 4)
      {
        Argument.Type = "missing-type";
      }
      else if (Case == 5)
      {
        Argument.Kind = static_cast<BytecodeGenericArgumentKind>(255);
      }
      else
      {
        StorageLayout Narrow = *Integer;
        Narrow.BitWidth = 3;
        Narrow.Size = 1;
        Narrow.Alignment = 1;
        Narrow.Native = false;
        auto Types = extendLayouts(*Object.Artifact);
        const auto Type = Types->append(std::move(Narrow));
        Argument.Type = bytecodeTypeIdentity(*Types, Type);
        Argument.Value = "f";
      }
      Object.Artifact->Symbols.front().Identity.GenericArguments = {std::move(Argument)};
      EXPECT_EQ(validateBytecodeArtifact(*Object.Artifact).Status, BytecodeStatus::InvalidImage);
      EXPECT_FALSE(serializeBytecodeArtifact(*Object.Artifact));
    }
  }
} // namespace ink::execution::test
