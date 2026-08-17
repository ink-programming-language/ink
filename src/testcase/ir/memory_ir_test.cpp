#include "ink/ir/analysis/verifier.h"
#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    struct MemoryIrTestContext
    {
        MemoryIrTestContext()
            : Compilation(),
              IR(Compilation)
        {
        }

        explicit MemoryIrTestContext(core::TargetContext Target)
            : Compilation(Target),
              IR(Compilation)
        {
        }

        core::CompilationContext Compilation;
        IRContext IR;
    };

    bool hasDiagnostic(const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind)
    {
      for (const core::Diagnostic &DiagnosticEntry : Diagnostics)
      {
        if (DiagnosticEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    VerificationResult verifySingleMemoryInstruction(MemoryIrTestContext &Context, std::unique_ptr<Instruction> InstructionValue, const StructType *ReferencedStructType = nullptr)
    {
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      Module ModuleValue(Context.IR);
      if (ReferencedStructType != nullptr)
      {
        ModuleValue.StructTypes.push_back(ReferencedStructType);
      }
      Function Main(VoidType);
      Main.Name = "main";
      BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(InstructionValue));
      Entry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Main.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Main));
      return verify(Context.IR, ModuleValue);
    }

    const std::string MemoryText =
        "inkir 1\n"
        "\n"
        "define i32 @memory(byte[] %0, const byte[] %1) {\n"
        "entry:\n"
        "  %2 = alloca byte[] ptrsize 8\n"
        "  %3 = slice.data byte* byte[] %2\n"
        "  %4 = getelementptr i32, byte* %3, ptrsize 1\n"
        "  store i32 65, byte* %4\n"
        "  %5 = load i32, byte* %4\n"
        "  %6 = slice.data const byte* const byte[] %1\n"
        "  %7 = getelementptr byte, const byte* %6, ptrsize 0\n"
        "  %8 = load byte, const byte* %7\n"
        "  %9 = slice.length byte[] %2\n"
        "  lifetime.end byte[] %2\n"
        "  ret i32 %5\n"
        "}\n";

    // Verifies that both slice types, typed pointer arithmetic, and every memory instruction survive canonical text round-tripping.
    TEST(MemoryIrSerializationTest, RoundTripsSliceAndPointerMemoryInstructions)
    {
      MemoryIrTestContext Context;
      DeserializeResult Parsed = deserialize(Context.IR, MemoryText);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), MemoryText);
    }

    // Verifies that getelementptr preserves a const pointer result and round-trips a named struct element type canonically.
    TEST(MemoryIrSerializationTest, RoundTripsConstStructElementPointer)
    {
      MemoryIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "\n"
          "%Pair = type {byte, i32}\n"
          "\n"
          "define const byte* @advance(const byte* %0) {\n"
          "entry:\n"
          "  %1 = getelementptr %Pair, const byte* %0, ptrsize 2\n"
          "  ret const byte* %1\n"
          "}\n";

      DeserializeResult Parsed = deserialize(Context.IR, Text);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ASSERT_EQ(Parsed.module()->Functions.size(), 1U);
      ASSERT_EQ(Parsed.module()->Functions[0].Blocks.size(), 1U);
      ASSERT_EQ(Parsed.module()->Functions[0].Blocks[0].Instructions.size(), 2U);
      const auto &Pointer = static_cast<const GetElementPointerInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[0]);
      ASSERT_NE(Pointer.ResultType, nullptr);
      ASSERT_NE(Pointer.ElementType, nullptr);
      EXPECT_EQ(Pointer.ResultType->kind(), TypeKind::ConstBytePointer);
      EXPECT_EQ(Pointer.ElementType->kind(), TypeKind::Struct);
      SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), Text);
    }

    // Verifies that a dynamic root index followed by nested constant i32 field indices round-trips and preserves const pointer type.
    TEST(MemoryIrSerializationTest, RoundTripsConstNestedStructFieldPointerPath)
    {
      MemoryIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "\n"
          "%Inner = type {byte, i32}\n"
          "\n"
          "%Outer = type {byte, %Inner}\n"
          "\n"
          "define const byte* @field(const byte* %0, ptrsize %1) {\n"
          "entry:\n"
          "  %2 = getelementptr %Outer, const byte* %0, ptrsize %1, i32 1, i32 1\n"
          "  ret const byte* %2\n"
          "}\n";

      DeserializeResult Parsed = deserialize(Context.IR, Text);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ASSERT_EQ(Parsed.module()->Functions.size(), 1U);
      ASSERT_EQ(Parsed.module()->Functions[0].Blocks.size(), 1U);
      ASSERT_EQ(Parsed.module()->Functions[0].Blocks[0].Instructions.size(), 2U);
      const auto &Pointer = static_cast<const GetElementPointerInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[0]);
      ASSERT_NE(Pointer.ResultType, nullptr);
      ASSERT_NE(Pointer.ElementType, nullptr);
      ASSERT_NE(Pointer.Index, nullptr);
      EXPECT_EQ(Pointer.ResultType->kind(), TypeKind::ConstBytePointer);
      EXPECT_EQ(Pointer.ElementType->kind(), TypeKind::Struct);
      EXPECT_EQ(Pointer.Index->kind(), ValueKind::ValueOperand);
      ASSERT_EQ(Pointer.FieldIndices.size(), 2U);
      for (const std::unique_ptr<Value> &FieldIndex : Pointer.FieldIndices)
      {
        ASSERT_NE(FieldIndex, nullptr);
        EXPECT_EQ(FieldIndex->type().kind(), TypeKind::I32);
        ASSERT_EQ(FieldIndex->kind(), ValueKind::IntegerConstant);
        EXPECT_EQ(static_cast<const IntegerConstant &>(*FieldIndex).unsignedValue(), 1U);
      }
      SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), Text);
    }

    // Verifies that textual GEP rejects an SSA struct field index even when the operand has the required i32 type.
    TEST(MemoryIrVerifierTest, RejectsDynamicStructFieldIndex)
    {
      MemoryIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define void @field(byte* %0, i32 %1) {\n"
          "entry:\n"
          "  %2 = getelementptr %Pair, byte* %0, ptrsize 0, i32 %1\n"
          "  ret void\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrGetElementPointerFieldIndexNotConstant));
    }

    // Verifies that load and store accept every integer, floating-point, and recursively valid aggregate memory value type.
    TEST(MemoryIrVerifierTest, AcceptsEveryMemoryValueLoadAndStore)
    {
      MemoryIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {i32, f32}\n"
          "define void @typed(byte* %0, const byte* %1) {\n"
          "entry:\n"
          "  %2 = load bool, const byte* %1\n"
          "  %3 = load byte, const byte* %1\n"
          "  %4 = load i32, const byte* %1\n"
          "  %5 = load ptrsize, const byte* %1\n"
          "  %6 = load f16, const byte* %1\n"
          "  %7 = load f32, const byte* %1\n"
          "  %8 = load f64, const byte* %1\n"
          "  %9 = load %Pair, const byte* %1\n"
          "  store bool 1, byte* %0\n"
          "  store byte 2, byte* %0\n"
          "  store i32 3, byte* %0\n"
          "  store ptrsize 4, byte* %0\n"
          "  store f16 floatbits(f16,0x3C00), byte* %0\n"
          "  store f32 floatbits(f32,0x3F800000), byte* %0\n"
          "  store f64 floatbits(f64,0x3FF0000000000000), byte* %0\n"
          "  store %Pair zeroinitializer, byte* %0\n"
          "  ret void\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      EXPECT_TRUE(Result.succeeded());
      EXPECT_TRUE(Result.module().has_value());
    }

    // Verifies that void, pointer, slice, and recursively invalid aggregate types cannot define typed pointer memory accesses.
    TEST(MemoryIrVerifierTest, RejectsUnsupportedTypedMemoryValueTypes)
    {
      MemoryIrTestContext Context;
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      const Type &BytePointerType = Context.IR.getType(TypeKind::BytePointer);
      const Type &ConstBytePointerType = Context.IR.getType(TypeKind::ConstBytePointer);
      const Type &ByteSliceType = Context.IR.getType(TypeKind::ByteSlice);
      const Type &ConstByteSliceType = Context.IR.getType(TypeKind::ConstByteSlice);
      const StructType &AggregateType = Context.IR.createStructType("MemoryAggregate", {&ByteSliceType});
      const Type *UnsupportedTypes[] = {
          &VoidType,
          &BytePointerType,
          &ConstBytePointerType,
          &ByteSliceType,
          &ConstByteSliceType,
          &AggregateType,
      };

      for (const Type *UnsupportedType : UnsupportedTypes)
      {
        const StructType *ReferencedStructType = UnsupportedType->kind() == TypeKind::Struct ? &AggregateType : nullptr;
        auto Load = std::make_unique<LoadInstruction>(*UnsupportedType);
        Load->Result = ValueId{0};
        Load->Pointer = std::make_unique<NullConstant>(ConstBytePointerType);
        const VerificationResult LoadResult = verifySingleMemoryInstruction(Context, std::move(Load), ReferencedStructType);
        EXPECT_FALSE(LoadResult.succeeded()) << typeKindName(UnsupportedType->kind());
        EXPECT_TRUE(hasDiagnostic(LoadResult.diagnostics(), core::DiagnosticKind::IrLoadUnsupportedResultType)) << typeKindName(UnsupportedType->kind());

        auto Store = std::make_unique<StoreInstruction>();
        Store->StoredValue = std::make_unique<ZeroInitializer>(*UnsupportedType);
        Store->Pointer = std::make_unique<NullConstant>(BytePointerType);
        const VerificationResult StoreResult = verifySingleMemoryInstruction(Context, std::move(Store), ReferencedStructType);
        EXPECT_FALSE(StoreResult.succeeded()) << typeKindName(UnsupportedType->kind());
        EXPECT_TRUE(hasDiagnostic(StoreResult.diagnostics(), core::DiagnosticKind::IrStoreUnsupportedValueType)) << typeKindName(UnsupportedType->kind());
      }
    }

    // Verifies that ptrsize constants use the configured 32-bit or 64-bit target limit rather than the host pointer width.
    TEST(MemoryIrVerifierTest, ValidatesPointerSizeConstantsAgainstConfiguredTarget)
    {
      constexpr std::uint64_t Maximum32 = std::numeric_limits<std::uint32_t>::max();
      constexpr std::uint64_t First64Only = Maximum32 + 1;
      constexpr std::uint64_t Maximum64 = std::numeric_limits<std::uint64_t>::max();
      const auto TextWithValue = [](std::uint64_t Value)
      {
        return "inkir 1\n\ndefine void @main() {\nentry:\n  %0 = alloca byte[] ptrsize " + std::to_string(Value) + "\n  ret void\n}\n";
      };
      MemoryIrTestContext Target32(core::TargetContext(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian));
      MemoryIrTestContext Target64(core::TargetContext(core::PointerWidth::Bits64, core::ByteOrder::BigEndian));
      const core::TargetContext NativeTarget = core::TargetContext::native();
      const core::TargetContext SyntheticNativeLayout(NativeTarget.pointerWidth(), NativeTarget.byteOrder());
      MemoryIrTestContext DefaultTarget;

      const DeserializeResult Maximum32Result = deserialize(Target32.IR, TextWithValue(Maximum32));
      const DeserializeResult Overflow32Result = deserialize(Target32.IR, TextWithValue(First64Only));
      const DeserializeResult Extended64Result = deserialize(Target64.IR, TextWithValue(First64Only));
      const std::string Maximum64Text = TextWithValue(Maximum64);
      const DeserializeResult Maximum64Result = deserialize(Target64.IR, Maximum64Text);
      const DeserializeResult NegativeResult = deserialize(Target64.IR, "inkir 1\ndefine void @main() {\nentry:\n  %0 = alloca byte[] ptrsize -1\n  ret void\n}\n");
      auto ProgrammaticAlloca = std::make_unique<AllocaInstruction>(Target64.IR.getType(TypeKind::ByteSlice));
      ProgrammaticAlloca->Result = ValueId{0};
      ProgrammaticAlloca->Size = std::make_unique<IntegerConstant>(Target64.IR.getType(TypeKind::PointerSize), Maximum64);
      const VerificationResult ProgrammaticMaximum64Result = verifySingleMemoryInstruction(Target64, std::move(ProgrammaticAlloca));

      EXPECT_EQ(Target32.Compilation.targetContext().pointerWidth(), core::PointerWidth::Bits32);
      EXPECT_EQ(Target32.Compilation.targetContext().pointerByteWidth(), 4U);
      EXPECT_EQ(Target32.Compilation.targetContext().maximumPointerSizeValue(), Maximum32);
      EXPECT_EQ(Target32.Compilation.targetContext().byteOrder(), core::ByteOrder::LittleEndian);
      EXPECT_FALSE(Target32.Compilation.targetContext().isNativeAbiCompatible());
      EXPECT_EQ(Target64.Compilation.targetContext().pointerWidth(), core::PointerWidth::Bits64);
      EXPECT_EQ(Target64.Compilation.targetContext().pointerByteWidth(), 8U);
      EXPECT_EQ(Target64.Compilation.targetContext().maximumPointerSizeValue(), std::numeric_limits<std::uint64_t>::max());
      EXPECT_EQ(Target64.Compilation.targetContext().byteOrder(), core::ByteOrder::BigEndian);
      EXPECT_FALSE(Target64.Compilation.targetContext().isNativeAbiCompatible());
      EXPECT_TRUE(NativeTarget.isNativeAbiCompatible());
      EXPECT_EQ(DefaultTarget.Compilation.targetContext(), NativeTarget);
      EXPECT_EQ(NativeTarget, core::TargetContext::native());
      EXPECT_NE(NativeTarget, SyntheticNativeLayout);
      EXPECT_EQ(SyntheticNativeLayout, core::TargetContext(NativeTarget.pointerWidth(), NativeTarget.byteOrder()));
      EXPECT_TRUE(Maximum32Result.succeeded());
      ASSERT_FALSE(Overflow32Result.succeeded());
      ASSERT_EQ(Overflow32Result.diagnostics().size(), 1U);
      EXPECT_EQ(Overflow32Result.diagnostics()[0].Kind, core::DiagnosticKind::IrPointerSizeConstantOutOfRange);
      EXPECT_EQ(core::DiagnosticFormatter().format(Overflow32Result.diagnostics()[0]).Message, "ptrsize integer constant 4294967296 in function @main exceeds the target maximum 4294967295");
      EXPECT_TRUE(Extended64Result.succeeded());
      ASSERT_TRUE(Maximum64Result.succeeded());
      ASSERT_TRUE(Maximum64Result.module().has_value());
      const SerializeResult Maximum64Serialized = serialize(Target64.IR, *Maximum64Result.module());
      ASSERT_TRUE(Maximum64Serialized.succeeded());
      ASSERT_TRUE(Maximum64Serialized.text().has_value());
      EXPECT_EQ(*Maximum64Serialized.text(), Maximum64Text);
      const AllocaInstruction &Maximum64Alloca = static_cast<const AllocaInstruction &>(*Maximum64Result.module()->Functions[0].Blocks[0].Instructions[0]);
      const IntegerConstant &Maximum64Constant = static_cast<const IntegerConstant &>(*Maximum64Alloca.Size);
      EXPECT_EQ(Maximum64Constant.unsignedValue(), Maximum64);
      EXPECT_FALSE(Maximum64Constant.isNegative());
      EXPECT_TRUE(ProgrammaticMaximum64Result.succeeded());
      ASSERT_FALSE(NegativeResult.succeeded());
      ASSERT_EQ(NegativeResult.diagnostics().size(), 1U);
      EXPECT_EQ(NegativeResult.diagnostics()[0].Kind, core::DiagnosticKind::IrPointerSizeConstantNegative);
      EXPECT_EQ(core::DiagnosticFormatter().format(NegativeResult.diagnostics()[0]).Message, "ptrsize integer constant -1 in function @main cannot be negative");
    }

    // Verifies that safe byte slices remain valid internal parameters while their data pointers feed typed memory operations.
    TEST(MemoryIrVerifierTest, AcceptsInternalMutableAndConstSliceParameters)
    {
      MemoryIrTestContext Context;
      DeserializeResult Result = deserialize(Context.IR, MemoryText);

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.module().has_value());
      ASSERT_EQ(Result.module()->Functions.size(), 1u);
      ASSERT_EQ(Result.module()->Functions[0].ParameterTypes.size(), 2u);
      EXPECT_EQ(Result.module()->Functions[0].ParameterTypes[0]->kind(), TypeKind::ByteSlice);
      EXPECT_EQ(Result.module()->Functions[0].ParameterTypes[1]->kind(), TypeKind::ConstByteSlice);
    }

    // Verifies that safe slices cannot cross a C external ABI boundary.
    TEST(MemoryIrVerifierTest, RejectsSliceInExternalSignature)
    {
      MemoryIrTestContext Context;
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      const Type &SliceType = Context.IR.getType(TypeKind::ByteSlice);
      Module ModuleValue(Context.IR);
      Function External(VoidType);
      External.Name = "external";
      External.Kind = FunctionKind::External;
      External.Convention = CallingConvention::C;
      External.ParameterTypes = {&SliceType};
      ModuleValue.Functions.push_back(std::move(External));

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrSliceInExternalSignature));
    }

    // Verifies that a safe slice cannot escape through a function result.
    TEST(MemoryIrVerifierTest, RejectsSliceFunctionResult)
    {
      MemoryIrTestContext Context;
      const Type &SliceType = Context.IR.getType(TypeKind::ByteSlice);
      Module ModuleValue(Context.IR);
      Function FunctionValue(SliceType);
      FunctionValue.Name = "escape";
      FunctionValue.ParameterTypes = {&SliceType};
      BasicBlock Entry;
      Entry.Name = "entry";
      auto Return = std::make_unique<ReturnInstruction>();
      Return->ReturnValue = std::make_unique<ValueOperand>(SliceType, ValueId{0});
      Entry.Instructions.push_back(std::move(Return));
      FunctionValue.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(FunctionValue));

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrSliceFunctionResultForbidden));
    }

    // Verifies that a safe slice cannot be hidden inside a struct and thereby escape its frame lifetime.
    TEST(MemoryIrVerifierTest, RejectsSliceStructField)
    {
      MemoryIrTestContext Context;
      const Type &ConstSliceType = Context.IR.getType(TypeKind::ConstByteSlice);
      const StructType &Container = Context.IR.createStructType("Container", {&ConstSliceType});
      Module ModuleValue(Context.IR);
      ModuleValue.StructTypes.push_back(&Container);

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrSliceStructFieldForbidden));
    }

    // Verifies that store cannot mutate memory through a const byte pointer.
    TEST(MemoryIrVerifierTest, RejectsStoreToConstPointer)
    {
      MemoryIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main(const byte* %0) {\n"
          "entry:\n"
          "  store byte 1, const byte* %0\n"
          "  ret void\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrStoreDestinationNotMutablePointer));
    }

    // Verifies that getelementptr derives mutable and const result pointers without changing either base pointer's mutability.
    TEST(MemoryIrVerifierTest, PreservesPointerConstnessThroughGetElementPointer)
    {
      MemoryIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main(byte* %0, const byte* %1) {\n"
          "entry:\n"
          "  %2 = getelementptr i32, byte* %0, ptrsize 1\n"
          "  %3 = getelementptr i32, const byte* %1, ptrsize 1\n"
          "  store i32 7, byte* %2\n"
          "  %4 = load i32, const byte* %3\n"
          "  ret void\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_TRUE(Result.succeeded());
      ASSERT_TRUE(Result.module().has_value());
      const auto &MutablePointer = static_cast<const GetElementPointerInstruction &>(*Result.module()->Functions[0].Blocks[0].Instructions[0]);
      const auto &ConstPointer = static_cast<const GetElementPointerInstruction &>(*Result.module()->Functions[0].Blocks[0].Instructions[1]);
      ASSERT_NE(MutablePointer.ResultType, nullptr);
      ASSERT_NE(ConstPointer.ResultType, nullptr);
      EXPECT_EQ(MutablePointer.ResultType->kind(), TypeKind::BytePointer);
      EXPECT_EQ(ConstPointer.ResultType->kind(), TypeKind::ConstBytePointer);
    }

    // Verifies that extracting mutable data from a const slice is rejected while const extraction is accepted.
    TEST(MemoryIrVerifierTest, RejectsSliceDataThatDropsConst)
    {
      MemoryIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main(const byte[] %0) {\n"
          "entry:\n"
          "  %1 = slice.data byte* const byte[] %0\n"
          "  ret void\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrSliceDataDropsConst));
    }

    // Verifies that frame-owned allocation is restricted to the function entry block.
    TEST(MemoryIrVerifierTest, RejectsAllocaOutsideEntryBlock)
    {
      MemoryIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  br body\n"
          "body:\n"
          "  %0 = alloca byte[] ptrsize 1\n"
          "  ret void\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrAllocaOutsideEntryBlock));
    }

    // Verifies every null, result-type, source-type, value-type, constness, and index-type invariant for all pointer and slice memory instructions.
    TEST(MemoryIrVerifierTest, RejectsEveryMalformedMemoryInstructionShape)
    {
      MemoryIrTestContext Context;
      const Type &BoolType = Context.IR.getType(TypeKind::Bool);
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &PointerSizeType = Context.IR.getType(TypeKind::PointerSize);
      const Type &BytePointerType = Context.IR.getType(TypeKind::BytePointer);
      const Type &ConstBytePointerType = Context.IR.getType(TypeKind::ConstBytePointer);
      const Type &ByteSliceType = Context.IR.getType(TypeKind::ByteSlice);
      const Type &ConstByteSliceType = Context.IR.getType(TypeKind::ConstByteSlice);
      const auto PointerSizeZero = [&]()
      {
        return std::make_unique<IntegerConstant>(PointerSizeType, 0);
      };
      const auto MutableSlice = [&]()
      {
        return std::make_unique<ZeroInitializer>(ByteSliceType);
      };
      const auto MutablePointer = [&]()
      {
        return std::make_unique<NullConstant>(BytePointerType);
      };
      const auto ConstPointer = [&]()
      {
        return std::make_unique<NullConstant>(ConstBytePointerType);
      };
      const auto ExpectDiagnostic = [&](std::unique_ptr<Instruction> InstructionValue, core::DiagnosticKind Expected)
      {
        const VerificationResult Result = verifySingleMemoryInstruction(Context, std::move(InstructionValue));
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), Expected)) << static_cast<unsigned int>(Expected);
      };

      {
        auto InstructionValue = std::make_unique<AllocaInstruction>(BoolType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Size = PointerSizeZero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrAllocaInvalidResultType);
      }
      {
        auto InstructionValue = std::make_unique<AllocaInstruction>(ByteSliceType);
        InstructionValue->Result = ValueId{0};
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrAllocaNullSize);
      }
      {
        auto InstructionValue = std::make_unique<AllocaInstruction>(ByteSliceType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Size = std::make_unique<IntegerConstant>(BoolType, 0);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrAllocaSizeNotPointerSize);
      }
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(I32Type, ByteType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = MutablePointer();
        InstructionValue->Index = PointerSizeZero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerInvalidResultType);
      }
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(BytePointerType, Context.IR.getType(TypeKind::Void));
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = MutablePointer();
        InstructionValue->Index = PointerSizeZero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerUnsupportedElementType);
      }
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(BytePointerType, ByteType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->ElementType = nullptr;
        InstructionValue->Pointer = MutablePointer();
        InstructionValue->Index = PointerSizeZero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerUnsupportedElementType);
      }
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(BytePointerType, ByteType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Index = PointerSizeZero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerNullPointer);
      }
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(BytePointerType, ByteType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = std::make_unique<IntegerConstant>(ByteType, 0);
        InstructionValue->Index = PointerSizeZero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerInvalidPointerType);
      }
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(BytePointerType, ByteType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = ConstPointer();
        InstructionValue->Index = PointerSizeZero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerResultTypeMismatch);
      }
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(BytePointerType, ByteType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = MutablePointer();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerNullIndex);
      }
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(BytePointerType, ByteType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = MutablePointer();
        InstructionValue->Index = std::make_unique<IntegerConstant>(BoolType, 0);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerIndexNotPointerSize);
      }
      {
        auto InstructionValue = std::make_unique<LoadInstruction>(BytePointerType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = ConstPointer();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrLoadUnsupportedResultType);
      }
      {
        auto InstructionValue = std::make_unique<LoadInstruction>(ByteType);
        InstructionValue->Result = ValueId{0};
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrLoadNullPointer);
      }
      {
        auto InstructionValue = std::make_unique<LoadInstruction>(ByteType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = std::make_unique<IntegerConstant>(ByteType, 0);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrLoadInvalidPointerType);
      }
      {
        auto InstructionValue = std::make_unique<StoreInstruction>();
        InstructionValue->Pointer = MutablePointer();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrStoreNullValue);
      }
      {
        auto InstructionValue = std::make_unique<StoreInstruction>();
        InstructionValue->StoredValue = std::make_unique<ZeroInitializer>(BytePointerType);
        InstructionValue->Pointer = MutablePointer();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrStoreUnsupportedValueType);
      }
      {
        auto InstructionValue = std::make_unique<StoreInstruction>();
        InstructionValue->StoredValue = std::make_unique<IntegerConstant>(ByteType, 1);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrStoreNullPointer);
      }
      {
        auto InstructionValue = std::make_unique<StoreInstruction>();
        InstructionValue->StoredValue = std::make_unique<IntegerConstant>(ByteType, 1);
        InstructionValue->Pointer = std::make_unique<IntegerConstant>(ByteType, 0);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrStoreDestinationNotMutablePointer);
      }
      {
        auto InstructionValue = std::make_unique<StoreInstruction>();
        InstructionValue->StoredValue = std::make_unique<IntegerConstant>(ByteType, 1);
        InstructionValue->Pointer = ConstPointer();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrStoreDestinationNotMutablePointer);
      }
      {
        ExpectDiagnostic(std::make_unique<LifetimeEndInstruction>(), core::DiagnosticKind::IrLifetimeEndNullSlice);
      }
      {
        auto InstructionValue = std::make_unique<LifetimeEndInstruction>();
        InstructionValue->Slice = std::make_unique<ZeroInitializer>(ConstByteSliceType);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrLifetimeEndInvalidSliceType);
      }
      {
        auto InstructionValue = std::make_unique<SliceDataInstruction>(I32Type);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Slice = MutableSlice();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrSliceDataInvalidResultType);
      }
      {
        auto InstructionValue = std::make_unique<SliceDataInstruction>(BytePointerType);
        InstructionValue->Result = ValueId{0};
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrSliceDataNullSlice);
      }
      {
        auto InstructionValue = std::make_unique<SliceDataInstruction>(BytePointerType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Slice = std::make_unique<ZeroInitializer>(ByteType);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrSliceDataInvalidSliceType);
      }
      {
        auto InstructionValue = std::make_unique<SliceLengthInstruction>(I32Type);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Slice = MutableSlice();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrSliceLengthInvalidResultType);
      }
      {
        auto InstructionValue = std::make_unique<SliceLengthInstruction>(PointerSizeType);
        InstructionValue->Result = ValueId{0};
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrSliceLengthNullSlice);
      }
      {
        auto InstructionValue = std::make_unique<SliceLengthInstruction>(PointerSizeType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Slice = std::make_unique<ZeroInitializer>(ByteType);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrSliceLengthInvalidSliceType);
      }
    }

    // Verifies that malformed struct field paths diagnose null, mistyped, nonconstant, negative, out-of-range, and scalar-traversing indices independently.
    TEST(MemoryIrVerifierTest, RejectsMalformedGetElementPointerStructFieldPaths)
    {
      MemoryIrTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &PointerSizeType = Context.IR.getType(TypeKind::PointerSize);
      const Type &BytePointerType = Context.IR.getType(TypeKind::BytePointer);
      const StructType &PairType = Context.IR.createStructType("Pair", {&ByteType, &I32Type});
      const auto CreatePointer = [&]()
      {
        auto InstructionValue = std::make_unique<GetElementPointerInstruction>(BytePointerType, PairType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Pointer = std::make_unique<NullConstant>(BytePointerType);
        InstructionValue->Index = std::make_unique<IntegerConstant>(PointerSizeType, 0);
        return InstructionValue;
      };
      const auto ExpectDiagnostic = [&](std::unique_ptr<GetElementPointerInstruction> InstructionValue, core::DiagnosticKind Expected)
      {
        const VerificationResult Result = verifySingleMemoryInstruction(Context, std::move(InstructionValue), &PairType);
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), Expected)) << static_cast<unsigned int>(Expected);
      };

      {
        auto InstructionValue = CreatePointer();
        InstructionValue->FieldIndices.push_back(nullptr);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerNullFieldIndex);
      }
      {
        auto InstructionValue = CreatePointer();
        InstructionValue->FieldIndices.push_back(std::make_unique<IntegerConstant>(PointerSizeType, 0));
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerFieldIndexNotI32);
      }
      {
        auto InstructionValue = CreatePointer();
        InstructionValue->FieldIndices.push_back(std::make_unique<ZeroInitializer>(I32Type));
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerFieldIndexNotConstant);
      }
      {
        auto InstructionValue = CreatePointer();
        InstructionValue->FieldIndices.push_back(std::make_unique<IntegerConstant>(I32Type, -1));
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerFieldIndexNegative);
      }
      {
        auto InstructionValue = CreatePointer();
        InstructionValue->FieldIndices.push_back(std::make_unique<IntegerConstant>(I32Type, 2));
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerFieldIndexOutOfRange);
      }
      {
        auto InstructionValue = CreatePointer();
        InstructionValue->FieldIndices.push_back(std::make_unique<IntegerConstant>(I32Type, 1));
        InstructionValue->FieldIndices.push_back(std::make_unique<IntegerConstant>(I32Type, 0));
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrGetElementPointerFieldIndexIntoNonStruct);
      }
    }

    // Verifies that every value-producing memory or arithmetic instruction requires an explicit SSA result.
    TEST(MemoryIrDeserializationTest, RequiresResultsForValueProducingInstructions)
    {
      struct Case
      {
          const char *InstructionText;
          core::DiagnosticKind Expected;
      };
      const Case Cases[] = {
          {"alloca byte[] ptrsize 1", core::DiagnosticKind::IrAllocaRequiresResult},
          {"getelementptr byte, byte* %1, ptrsize 0", core::DiagnosticKind::IrGetElementPointerRequiresResult},
          {"load byte, byte* %1", core::DiagnosticKind::IrLoadRequiresResult},
          {"slice.data byte* byte[] %0", core::DiagnosticKind::IrSliceDataRequiresResult},
          {"slice.length byte[] %0", core::DiagnosticKind::IrSliceLengthRequiresResult},
          {"add byte 1, byte 2", core::DiagnosticKind::IrAddRequiresResult},
          {"icmp eq byte 1, byte 2", core::DiagnosticKind::IrCompareRequiresResult},
      };

      for (const Case &CaseValue : Cases)
      {
        MemoryIrTestContext Context;
        const std::string Text = "inkir 1\ndefine void @main(byte[] %0, byte* %1) {\nentry:\n  " + std::string(CaseValue.InstructionText) + "\n  ret void\n}\n";
        const DeserializeResult Result = deserialize(Context.IR, Text);
        ASSERT_FALSE(Result.succeeded()) << CaseValue.InstructionText;
        ASSERT_FALSE(Result.diagnostics().empty()) << CaseValue.InstructionText;
        EXPECT_EQ(Result.diagnostics()[0].Kind, CaseValue.Expected) << CaseValue.InstructionText;
      }
    }

    // Verifies that side-effecting and terminating instructions cannot be assigned an SSA result.
    TEST(MemoryIrDeserializationTest, RejectsResultsOnNonValueInstructions)
    {
      const char *Instructions[] = {
          "%2 = store byte 1, byte* %1",
          "%2 = lifetime.end byte[] %0",
          "%2 = br exit",
          "%2 = condbr bool 1, exit, exit",
          "%2 = ret void",
      };

      for (const char *InstructionText : Instructions)
      {
        MemoryIrTestContext Context;
        const std::string Text = "inkir 1\ndefine void @main(byte[] %0, byte* %1) {\nentry:\n  " + std::string(InstructionText) + "\nexit:\n  ret void\n}\n";
        const DeserializeResult Result = deserialize(Context.IR, Text);
        ASSERT_FALSE(Result.succeeded()) << InstructionText;
        ASSERT_FALSE(Result.diagnostics().empty()) << InstructionText;
        EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::IrInstructionCannotDefineResult) << InstructionText;
      }
    }
  } // namespace
} // namespace ink::ir
