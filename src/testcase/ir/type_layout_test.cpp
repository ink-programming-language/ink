#include "ink/ir/context.h"
#include "ink/ir/type_layout.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <optional>
#include <vector>

namespace ink::ir
{
  namespace
  {
    struct TypeLayoutTestContext
    {
      core::CompilationContext Compilation;
      IRContext IR{Compilation};
    };

    // Verifies that every fixed-width integer and floating-point scalar has identical size, alignment, and stride.
    TEST(TypeLayoutTest, ComputesFixedWidthScalarLayouts)
    {
      struct Case
      {
        TypeKind Kind;
        std::size_t Width;
      };
      const Case Cases[] = {
          {TypeKind::Bool, 1},
          {TypeKind::Byte, 1},
          {TypeKind::F16, 2},
          {TypeKind::I32, 4},
          {TypeKind::F32, 4},
          {TypeKind::F64, 8},
      };
      TypeLayoutTestContext Context;
      const core::TargetContext Target(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);

      for (const Case &CaseValue : Cases)
      {
        const std::optional<TypeLayout> Layout = computeTypeLayout(Context.IR.getType(CaseValue.Kind), Target);
        ASSERT_TRUE(Layout.has_value()) << typeKindName(CaseValue.Kind);
        EXPECT_EQ(Layout->Size, CaseValue.Width) << typeKindName(CaseValue.Kind);
        EXPECT_EQ(Layout->Alignment, CaseValue.Width) << typeKindName(CaseValue.Kind);
        EXPECT_EQ(Layout->StrideSize, CaseValue.Width) << typeKindName(CaseValue.Kind);
        EXPECT_TRUE(Layout->FieldOffsets.empty()) << typeKindName(CaseValue.Kind);
      }
    }

    // Verifies that ptrsize and both byte pointer types follow the configured 32-bit or 64-bit target width rather than the host width.
    TEST(TypeLayoutTest, ComputesTargetDependentPointerLayouts)
    {
      const TypeKind Kinds[] = {
          TypeKind::PointerSize,
          TypeKind::BytePointer,
          TypeKind::ConstBytePointer,
      };
      TypeLayoutTestContext Context;
      const core::TargetContext Target32(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian);
      const core::TargetContext Target64(core::PointerWidth::Bits64, core::ByteOrder::BigEndian);

      for (TypeKind Kind : Kinds)
      {
        const std::optional<TypeLayout> Layout32 = computeTypeLayout(Context.IR.getType(Kind), Target32);
        const std::optional<TypeLayout> Layout64 = computeTypeLayout(Context.IR.getType(Kind), Target64);
        ASSERT_TRUE(Layout32.has_value()) << typeKindName(Kind);
        ASSERT_TRUE(Layout64.has_value()) << typeKindName(Kind);
        EXPECT_EQ(Layout32->Size, 4U) << typeKindName(Kind);
        EXPECT_EQ(Layout32->Alignment, 4U) << typeKindName(Kind);
        EXPECT_EQ(Layout32->StrideSize, 4U) << typeKindName(Kind);
        EXPECT_EQ(Layout64->Size, 8U) << typeKindName(Kind);
        EXPECT_EQ(Layout64->Alignment, 8U) << typeKindName(Kind);
        EXPECT_EQ(Layout64->StrideSize, 8U) << typeKindName(Kind);
      }
    }

    // Verifies that a byte followed by i32 receives three bytes of internal padding and an eight-byte aggregate stride.
    TEST(TypeLayoutTest, ComputesInternallyPaddedStructLayout)
    {
      TypeLayoutTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const StructType &Struct = Context.IR.createStructType("ByteThenI32", {&ByteType, &I32Type});
      const core::TargetContext Target(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);

      const std::optional<TypeLayout> Layout = computeTypeLayout(Struct, Target);

      ASSERT_TRUE(Layout.has_value());
      EXPECT_EQ(Layout->Size, 8U);
      EXPECT_EQ(Layout->Alignment, 4U);
      EXPECT_EQ(Layout->StrideSize, 8U);
      EXPECT_EQ(Layout->FieldOffsets, (std::vector<std::size_t>{0, 4}));
    }

    // Verifies that an i32 followed by byte has a five-byte occupied size but rounds its array stride up to eight bytes.
    TEST(TypeLayoutTest, DistinguishesStructSizeFromTailPaddedStride)
    {
      TypeLayoutTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const StructType &Struct = Context.IR.createStructType("I32ThenByte", {&I32Type, &ByteType});
      const core::TargetContext Target(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);

      const std::optional<TypeLayout> Layout = computeTypeLayout(Struct, Target);

      ASSERT_TRUE(Layout.has_value());
      EXPECT_EQ(Layout->Size, 5U);
      EXPECT_EQ(Layout->Alignment, 4U);
      EXPECT_EQ(Layout->StrideSize, 8U);
      EXPECT_EQ(Layout->FieldOffsets, (std::vector<std::size_t>{0, 4}));
    }

    // Verifies that nested aggregate offsets compose inner stride, outer alignment, and trailing alignment without flattening fields.
    TEST(TypeLayoutTest, ComputesNestedStructLayout)
    {
      TypeLayoutTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const Type &F64Type = Context.IR.getType(TypeKind::F64);
      const StructType &Inner = Context.IR.createStructType("Inner", {&ByteType, &I32Type});
      const StructType &Outer = Context.IR.createStructType("Outer", {&ByteType, &Inner, &F64Type});
      const core::TargetContext Target(core::PointerWidth::Bits64, core::ByteOrder::BigEndian);

      const std::optional<TypeLayout> InnerLayout = computeTypeLayout(Inner, Target);
      const std::optional<TypeLayout> OuterLayout = computeTypeLayout(Outer, Target);

      ASSERT_TRUE(InnerLayout.has_value());
      ASSERT_TRUE(OuterLayout.has_value());
      EXPECT_EQ(InnerLayout->FieldOffsets, (std::vector<std::size_t>{0, 4}));
      EXPECT_EQ(InnerLayout->StrideSize, 8U);
      EXPECT_EQ(OuterLayout->Size, 24U);
      EXPECT_EQ(OuterLayout->Alignment, 8U);
      EXPECT_EQ(OuterLayout->StrideSize, 24U);
      EXPECT_EQ(OuterLayout->FieldOffsets, (std::vector<std::size_t>{0, 4, 16}));
    }

    // Verifies that a trailing nested struct contributes its occupied size to the outer size while its padded stride still governs array spacing.
    TEST(TypeLayoutTest, PreservesTrailingNestedStructTailPadding)
    {
      TypeLayoutTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const StructType &Inner = Context.IR.createStructType("TrailingInner", {&I32Type, &ByteType});
      const StructType &Outer = Context.IR.createStructType("TrailingOuter", {&ByteType, &Inner});
      const core::TargetContext Target(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);

      const std::optional<TypeLayout> Layout = computeTypeLayout(Outer, Target);

      ASSERT_TRUE(Layout.has_value());
      EXPECT_EQ(Layout->FieldOffsets, (std::vector<std::size_t>{0, 4}));
      EXPECT_EQ(Layout->Size, 9U);
      EXPECT_EQ(Layout->Alignment, 4U);
      EXPECT_EQ(Layout->StrideSize, 12U);
    }

    // Verifies that void, slices, null fields, and structs containing unsized fields have no computable memory layout.
    TEST(TypeLayoutTest, RejectsUnsizedAndMalformedLayouts)
    {
      TypeLayoutTestContext Context;
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      const Type &ByteSliceType = Context.IR.getType(TypeKind::ByteSlice);
      const Type &ConstByteSliceType = Context.IR.getType(TypeKind::ConstByteSlice);
      const StructType &VoidContainer = Context.IR.createStructType("VoidContainer", {&VoidType});
      const StructType &SliceContainer = Context.IR.createStructType("SliceContainer", {&ByteSliceType});
      const StructType &NullContainer = Context.IR.createStructType("NullContainer", {nullptr});
      const core::TargetContext Target(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);

      EXPECT_FALSE(computeTypeLayout(VoidType, Target).has_value());
      EXPECT_FALSE(computeTypeLayout(ByteSliceType, Target).has_value());
      EXPECT_FALSE(computeTypeLayout(ConstByteSliceType, Target).has_value());
      EXPECT_FALSE(computeTypeLayout(VoidContainer, Target).has_value());
      EXPECT_FALSE(computeTypeLayout(SliceContainer, Target).has_value());
      EXPECT_FALSE(computeTypeLayout(NullContainer, Target).has_value());
    }

    // Verifies that memory-value eligibility recursively accepts scalar aggregates and rejects pointer, slice, void, and null fields at any nesting depth.
    TEST(TypeLayoutTest, ClassifiesNestedMemoryValueTypesRecursively)
    {
      TypeLayoutTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &F32Type = Context.IR.getType(TypeKind::F32);
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      const Type &PointerType = Context.IR.getType(TypeKind::BytePointer);
      const Type &ConstPointerType = Context.IR.getType(TypeKind::ConstBytePointer);
      const Type &SliceType = Context.IR.getType(TypeKind::ByteSlice);
      const Type &ConstSliceType = Context.IR.getType(TypeKind::ConstByteSlice);
      const StructType &ValidInner = Context.IR.createStructType("ValidInner", {&ByteType, &F32Type});
      const StructType &ValidOuter = Context.IR.createStructType("ValidOuter", {&ValidInner, &ByteType});
      const StructType &PointerInner = Context.IR.createStructType("PointerInner", {&PointerType});
      const StructType &PointerOuter = Context.IR.createStructType("PointerOuter", {&ValidInner, &PointerInner});
      const StructType &SliceInner = Context.IR.createStructType("SliceInner", {&SliceType});
      const StructType &SliceOuter = Context.IR.createStructType("SliceOuter", {&ValidInner, &SliceInner});
      const StructType &VoidContainer = Context.IR.createStructType("VoidContainer", {&VoidType});
      const StructType &NullContainer = Context.IR.createStructType("NullContainer", {nullptr});

      EXPECT_TRUE(isMemoryValueType(ByteType));
      EXPECT_TRUE(isMemoryValueType(F32Type));
      EXPECT_TRUE(isMemoryValueType(ValidInner));
      EXPECT_TRUE(isMemoryValueType(ValidOuter));
      EXPECT_FALSE(isMemoryValueType(VoidType));
      EXPECT_FALSE(isMemoryValueType(PointerType));
      EXPECT_FALSE(isMemoryValueType(ConstPointerType));
      EXPECT_FALSE(isMemoryValueType(SliceType));
      EXPECT_FALSE(isMemoryValueType(ConstSliceType));
      EXPECT_FALSE(isMemoryValueType(PointerInner));
      EXPECT_FALSE(isMemoryValueType(PointerOuter));
      EXPECT_FALSE(isMemoryValueType(SliceInner));
      EXPECT_FALSE(isMemoryValueType(SliceOuter));
      EXPECT_FALSE(isMemoryValueType(VoidContainer));
      EXPECT_FALSE(isMemoryValueType(NullContainer));
    }
  } // namespace
} // namespace ink::ir
