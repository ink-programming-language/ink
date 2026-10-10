#include "ink/core/target_context.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/class_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/interface_type.h"
#include "ink/ir/type/pointer_type.h"
#include "ink/ir/type/reference_type.h"
#include "ink/ir/type/struct_type.h"
#include "ink/ir/type/type.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <limits>
#include <memory>
#include <string_view>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    constexpr core::TargetContext Target32(core::PointerWidth::Bits32, core::ByteOrder::LittleEndian);
    constexpr core::TargetContext Target64(core::PointerWidth::Bits64, core::ByteOrder::LittleEndian);
    constexpr std::uint64_t MaximumSize = std::numeric_limits<std::uint64_t>::max();

    class SyntheticType final : public Type
    {
      public:
        SyntheticType(std::uint64_t Size, std::uint64_t Alignment, ValueKind Kind = ValueKind::Struct) noexcept
            : Type(Kind),
              Size(Size),
              Alignment(Alignment)
        {
        }

        std::uint64_t size() const noexcept override
        {
          return Size;
        }

        std::uint64_t alignment() const noexcept override
        {
          return Alignment;
        }

      private:
        std::uint64_t Size = 0;
        std::uint64_t Alignment = 0;
    };

    template <typename AggregateType>
    void checkAggregateLayouts(ValueKind ExpectedKind)
    {
      const auto Empty = AggregateType::create(0, 8);
      ASSERT_NE(Empty, nullptr);
      EXPECT_EQ(Empty->kind(), ExpectedKind);
      EXPECT_EQ(Empty->size(), 0U);
      EXPECT_EQ(Empty->alignment(), 8U);

      const auto Aligned = AggregateType::create(24, 8);
      ASSERT_NE(Aligned, nullptr);
      EXPECT_EQ(Aligned->size(), 24U);
      EXPECT_EQ(Aligned->alignment(), 8U);

      const auto Maximum = AggregateType::create(MaximumSize, 1);
      ASSERT_NE(Maximum, nullptr);
      EXPECT_EQ(Maximum->size(), MaximumSize);
      EXPECT_EQ(Maximum->alignment(), 1U);

      constexpr std::uint64_t LargestAlignment = std::uint64_t{1} << 63;
      const auto Large = AggregateType::create(LargestAlignment, LargestAlignment);
      ASSERT_NE(Large, nullptr);
      EXPECT_EQ(Large->size(), LargestAlignment);
      EXPECT_EQ(Large->alignment(), LargestAlignment);
      EXPECT_NE(AggregateType::create(0, LargestAlignment), nullptr);

      EXPECT_EQ(AggregateType::create(0, 0), nullptr);
      EXPECT_EQ(AggregateType::create(0, 3), nullptr);
      EXPECT_EQ(AggregateType::create(12, 6), nullptr);
      EXPECT_EQ(AggregateType::create(MaximumSize, MaximumSize), nullptr);
      EXPECT_EQ(AggregateType::create(1, 2), nullptr);
      EXPECT_EQ(AggregateType::create(15, 8), nullptr);
      EXPECT_EQ(AggregateType::create(MaximumSize, 8), nullptr);
    }
  } // namespace

  static_assert(std::has_virtual_destructor_v<Value>);
  static_assert(std::has_virtual_destructor_v<Type>);

  // Both integer widths preserve signedness and use the supplied target's scalar alignment.
  TEST(IRTypeTest, IntegerWidthsSignednessAndTargetAlignment)
  {
    for (const core::TargetContext &Target : {Target32, Target64})
    {
      for (const std::uint32_t Width : {32U, 64U})
      {
        for (const bool Signed : {false, true})
        {
          const auto Integer = IntegerType::create(Target, Width, Signed);
          ASSERT_NE(Integer, nullptr);
          EXPECT_EQ(Integer->kind(), ValueKind::Integer);
          EXPECT_EQ(Integer->width(), Width);
          EXPECT_EQ(Integer->isSigned(), Signed);
          EXPECT_EQ(Integer->isUnsigned(), !Signed);
          EXPECT_EQ(Integer->size(), Width / 8);
          EXPECT_EQ(Integer->alignment(), Target.integerAlignment(Width));
        }
      }
    }

    const auto Integer32Target = IntegerType::create(Target32, 64, true);
    const auto Integer64Target = IntegerType::create(Target64, 64, true);
    ASSERT_NE(Integer32Target, nullptr);
    ASSERT_NE(Integer64Target, nullptr);
    EXPECT_EQ(Integer32Target->alignment(), 4U);
    EXPECT_EQ(Integer64Target->alignment(), 8U);
  }

  // Unsupported integer widths return an explicit failure for either signedness.
  TEST(IRTypeTest, IntegerRejectsUnsupportedWidths)
  {
    constexpr std::uint32_t UnsupportedWidths[] = {
        0,
        1,
        8,
        16,
        31,
        33,
        63,
        65,
        128,
        std::numeric_limits<std::uint32_t>::max(),
    };
    for (const std::uint32_t Width : UnsupportedWidths)
    {
      EXPECT_EQ(IntegerType::create(Target64, Width, false), nullptr);
      EXPECT_EQ(IntegerType::create(Target64, Width, true), nullptr);
    }
  }

  // Float represents both 32-bit and 64-bit values under the same kind and target-specific alignment.
  TEST(IRTypeTest, FloatWidthsAndTargetAlignment)
  {
    for (const core::TargetContext &Target : {Target32, Target64})
    {
      for (const std::uint32_t Width : {32U, 64U})
      {
        const auto Float = FloatType::create(Target, Width);
        ASSERT_NE(Float, nullptr);
        EXPECT_EQ(Float->kind(), ValueKind::Float);
        EXPECT_EQ(Float->width(), Width);
        EXPECT_EQ(Float->size(), Width / 8);
        EXPECT_EQ(Float->alignment(), Target.floatAlignment(Width));
      }
    }

    const auto Float32Target = FloatType::create(Target32, 64);
    const auto Float64Target = FloatType::create(Target64, 64);
    ASSERT_NE(Float32Target, nullptr);
    ASSERT_NE(Float64Target, nullptr);
    EXPECT_EQ(Float32Target->alignment(), 4U);
    EXPECT_EQ(Float64Target->alignment(), 8U);
  }

  // Float rejects zero, neighboring widths, unsupported scalar widths and oversized values.
  TEST(IRTypeTest, FloatRejectsUnsupportedWidths)
  {
    constexpr std::uint32_t UnsupportedWidths[] = {
        0,
        1,
        16,
        31,
        33,
        63,
        65,
        80,
        128,
        std::numeric_limits<std::uint32_t>::max(),
    };
    for (const std::uint32_t Width : UnsupportedWidths)
    {
      EXPECT_EQ(FloatType::create(Target64, Width), nullptr);
    }
  }

  // Bool retains a one-byte representation through both Type and Value base interfaces.
  TEST(IRTypeTest, BoolHasOneBytePolymorphicLayout)
  {
    const BoolType Bool;
    const Type *TypeValue = &Bool;
    const Value *ValueValue = &Bool;
    EXPECT_EQ(TypeValue->kind(), ValueKind::Bool);
    EXPECT_EQ(TypeValue->size(), 1U);
    EXPECT_EQ(TypeValue->alignment(), 1U);
    EXPECT_EQ(ValueValue->kind(), ValueKind::Bool);
    EXPECT_EQ(ValueValue->size(), 1U);
    EXPECT_EQ(ValueValue->alignment(), 1U);
  }

  // Pointer and reference layouts follow the selected target while preserving the referenced type identity.
  TEST(IRTypeTest, PointerAndReferenceUseTargetWidth)
  {
    const BoolType Element;
    for (const core::TargetContext &Target : {Target32, Target64})
    {
      const PointerType Pointer(Target, &Element);
      const ReferenceType Reference(Target, &Element);
      EXPECT_EQ(Pointer.kind(), ValueKind::Pointer);
      EXPECT_EQ(Reference.kind(), ValueKind::Reference);
      EXPECT_EQ(Pointer.pointeeType(), &Element);
      EXPECT_EQ(Reference.referencedType(), &Element);
      EXPECT_EQ(Pointer.size(), Target.pointerByteWidth());
      EXPECT_EQ(Pointer.alignment(), Target.pointerByteWidth());
      EXPECT_EQ(Reference.size(), Target.pointerByteWidth());
      EXPECT_EQ(Reference.alignment(), Target.pointerByteWidth());
    }

    const PointerType Pointer32(Target32, &Element);
    const PointerType Pointer64(Target64, &Element);
    const ReferenceType Reference32(Target32, &Element);
    const ReferenceType Reference64(Target64, &Element);
    EXPECT_EQ(Pointer32.size(), 4U);
    EXPECT_EQ(Pointer64.size(), 8U);
    EXPECT_EQ(Reference32.size(), 4U);
    EXPECT_EQ(Reference64.size(), 8U);
  }

  // Nested arrays retain their immediate element identity and compose element count, size and alignment.
  TEST(IRTypeTest, ArrayPreservesElementsAndNestedLayout)
  {
    const auto Element = IntegerType::create(Target64, 32, false);
    ASSERT_NE(Element, nullptr);
    const auto Inner = ArrayType::create(Element.get(), 3);
    ASSERT_NE(Inner, nullptr);
    const auto Outer = ArrayType::create(Inner.get(), 2);
    ASSERT_NE(Outer, nullptr);
    EXPECT_EQ(Inner->kind(), ValueKind::Array);
    EXPECT_EQ(Inner->elementType(), Element.get());
    EXPECT_EQ(Inner->elementCount(), 3U);
    EXPECT_EQ(Inner->size(), 12U);
    EXPECT_EQ(Inner->alignment(), 4U);
    EXPECT_EQ(Outer->elementType(), Inner.get());
    EXPECT_EQ(Outer->elementCount(), 2U);
    EXPECT_EQ(Outer->size(), 24U);
    EXPECT_EQ(Outer->alignment(), 4U);
  }

  // Array storage rounds each element's size up to its ABI alignment before multiplying by the count.
  TEST(IRTypeTest, ArrayIncludesElementStridePadding)
  {
    const SyntheticType Element(5, 4);
    const auto Array = ArrayType::create(&Element, 3);
    ASSERT_NE(Array, nullptr);
    EXPECT_EQ(Array->size(), 24U);
    EXPECT_EQ(Array->alignment(), 4U);
    EXPECT_EQ(Array->elementCount(), 3U);
    EXPECT_EQ(Array->elementType(), &Element);
  }

  // Empty arrays remain valid with zero storage even when rounding a nonempty element would overflow.
  TEST(IRTypeTest, EmptyArrayDoesNotRequireRepresentableElementStride)
  {
    const SyntheticType Element(MaximumSize, 8);
    const auto Array = ArrayType::create(&Element, 0);
    ASSERT_NE(Array, nullptr);
    EXPECT_EQ(Array->size(), 0U);
    EXPECT_EQ(Array->alignment(), 8U);
    EXPECT_EQ(Array->elementCount(), 0U);
    EXPECT_EQ(Array->elementType(), &Element);
  }

  // Zero-sized elements accept the largest element count without introducing storage or losing alignment.
  TEST(IRTypeTest, ArrayOfZeroSizedElementsAcceptsMaximumCount)
  {
    const auto Element = StructType::create(0, 8);
    ASSERT_NE(Element, nullptr);
    const auto Array = ArrayType::create(Element.get(), MaximumSize);
    ASSERT_NE(Array, nullptr);
    EXPECT_EQ(Array->size(), 0U);
    EXPECT_EQ(Array->alignment(), 8U);
    EXPECT_EQ(Array->elementCount(), MaximumSize);
    const auto Nested = ArrayType::create(Array.get(), MaximumSize);
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested->size(), 0U);
    EXPECT_EQ(Nested->alignment(), 8U);
  }

  // Array multiplication accepts the last representable count and explicitly rejects the first overflowing count.
  TEST(IRTypeTest, ArrayChecksElementCountMultiplicationOverflow)
  {
    const auto Element = IntegerType::create(Target64, 64, true);
    ASSERT_NE(Element, nullptr);
    const auto Largest = ArrayType::create(Element.get(), MaximumSize / 8);
    ASSERT_NE(Largest, nullptr);
    EXPECT_EQ(Largest->size(), MaximumSize - 7);
    EXPECT_EQ(ArrayType::create(Element.get(), MaximumSize / 8 + 1), nullptr);

    const SyntheticType HugeElement(MaximumSize, 1);
    const auto Single = ArrayType::create(&HugeElement, 1);
    ASSERT_NE(Single, nullptr);
    EXPECT_EQ(Single->size(), MaximumSize);
    EXPECT_EQ(ArrayType::create(&HugeElement, 2), nullptr);
  }

  // Array stride rounding rejects overflow while accepting the adjacent aligned maximum-size element.
  TEST(IRTypeTest, ArrayChecksStrideRoundingOverflow)
  {
    const SyntheticType OverflowingElement(MaximumSize, 8);
    EXPECT_EQ(ArrayType::create(&OverflowingElement, 1), nullptr);
    const SyntheticType LargestAlignedElement(MaximumSize - 7, 8);
    const auto Single = ArrayType::create(&LargestAlignedElement, 1);
    ASSERT_NE(Single, nullptr);
    EXPECT_EQ(Single->size(), MaximumSize - 7);
    EXPECT_EQ(Single->alignment(), 8U);
  }

  // Array construction rejects invalid element alignments even when the requested element count is zero.
  TEST(IRTypeTest, ArrayRejectsInvalidElementAlignment)
  {
    const SyntheticType ZeroAlignment(4, 0);
    const SyntheticType NonPowerOfTwoAlignment(4, 3);
    EXPECT_EQ(ArrayType::create(&ZeroAlignment, 0), nullptr);
    EXPECT_EQ(ArrayType::create(&ZeroAlignment, 1), nullptr);
    EXPECT_EQ(ArrayType::create(&NonPowerOfTwoAlignment, 0), nullptr);
    EXPECT_EQ(ArrayType::create(&NonPowerOfTwoAlignment, 1), nullptr);
  }

  // Array construction rejects a missing element type for both empty and nonempty arrays.
  TEST(IRTypeTest, ArrayRejectsNullElementType)
  {
    EXPECT_EQ(ArrayType::create(nullptr, 0), nullptr);
    EXPECT_EQ(ArrayType::create(nullptr, 1), nullptr);
  }

  // Class layouts accept empty and extreme valid storage and reject invalid alignment or unaligned sizes.
  TEST(IRTypeTest, ClassValidatesExplicitStorageLayout)
  {
    checkAggregateLayouts<ClassType>(ValueKind::Class);
  }

  // Interface layouts apply the same nonzero power-of-two and size-alignment constraints as other aggregates.
  TEST(IRTypeTest, InterfaceValidatesExplicitStorageLayout)
  {
    checkAggregateLayouts<InterfaceType>(ValueKind::Interface);
  }

  // Struct layouts preserve valid caller-supplied storage and reject zero, non-power-of-two and mismatched alignment.
  TEST(IRTypeTest, StructValidatesExplicitStorageLayout)
  {
    checkAggregateLayouts<StructType>(ValueKind::Struct);
  }

  // Every concrete classifier accepts only its own kind and safely rejects null and unknown-kind values.
  TEST(IRTypeTest, ClassifiersRejectNullAndOtherKinds)
  {
    const auto Integer = IntegerType::create(Target64, 32, true);
    const auto Float = FloatType::create(Target64, 64);
    const BoolType Bool;
    const auto Array = ArrayType::create(&Bool, 2);
    const auto Class = ClassType::create(8, 8);
    const auto Interface = InterfaceType::create(16, 8);
    const auto Struct = StructType::create(4, 4);
    const PointerType Pointer(Target64, &Bool);
    const ReferenceType Reference(Target64, &Bool);
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Float, nullptr);
    ASSERT_NE(Array, nullptr);
    ASSERT_NE(Class, nullptr);
    ASSERT_NE(Interface, nullptr);
    ASSERT_NE(Struct, nullptr);

    const std::array<const Value *, 9> Values = {
        Integer.get(),
        Float.get(),
        &Bool,
        Array.get(),
        Class.get(),
        Interface.get(),
        Struct.get(),
        &Pointer,
        &Reference,
    };
    using Classifier = bool (*)(const Value *);
    const std::array<Classifier, 9> Classifiers = {
        IntegerType::classof,
        FloatType::classof,
        BoolType::classof,
        ArrayType::classof,
        ClassType::classof,
        InterfaceType::classof,
        StructType::classof,
        PointerType::classof,
        ReferenceType::classof,
    };
    const SyntheticType Unknown(1, 1, static_cast<ValueKind>(255));
    for (std::size_t ClassifierIndex = 0; ClassifierIndex < Classifiers.size(); ++ClassifierIndex)
    {
      EXPECT_FALSE(Classifiers[ClassifierIndex](nullptr));
      EXPECT_FALSE(Classifiers[ClassifierIndex](&Unknown));
      for (std::size_t ValueIndex = 0; ValueIndex < Values.size(); ++ValueIndex)
      {
        EXPECT_EQ(Classifiers[ClassifierIndex](Values[ValueIndex]), ClassifierIndex == ValueIndex) << "classifier " << ClassifierIndex << ", value " << ValueIndex;
      }
    }
  }

  // Registry names cover all nine kinds, with both float widths sharing the float name and unknown values staying identifiable.
  TEST(IRTypeTest, ValueKindNamesCoverConcreteTypes)
  {
    EXPECT_EQ(valueKindName(ValueKind::Integer), "integer");
    EXPECT_EQ(valueKindName(ValueKind::Float), "float");
    EXPECT_EQ(valueKindName(ValueKind::Bool), "bool");
    EXPECT_EQ(valueKindName(ValueKind::Array), "array");
    EXPECT_EQ(valueKindName(ValueKind::Class), "class");
    EXPECT_EQ(valueKindName(ValueKind::Interface), "interface");
    EXPECT_EQ(valueKindName(ValueKind::Struct), "struct");
    EXPECT_EQ(valueKindName(ValueKind::Pointer), "pointer");
    EXPECT_EQ(valueKindName(ValueKind::Reference), "reference");
    EXPECT_EQ(valueKindName(static_cast<ValueKind>(255)), "unknown");
  }

  // Owned concrete instances expose their original kind and target layout through a virtual Value base.
  TEST(IRTypeTest, OwnedValuePreservesConcretePolymorphicLayout)
  {
    std::unique_ptr<Value> Integer = IntegerType::create(Target32, 64, true);
    ASSERT_NE(Integer, nullptr);
    EXPECT_EQ(Integer->kind(), ValueKind::Integer);
    EXPECT_EQ(Integer->size(), 8U);
    EXPECT_EQ(Integer->alignment(), 4U);
    EXPECT_TRUE(IntegerType::classof(Integer.get()));
    std::unique_ptr<Value> Float = FloatType::create(Target64, 64);
    ASSERT_NE(Float, nullptr);
    EXPECT_EQ(Float->kind(), ValueKind::Float);
    EXPECT_EQ(Float->size(), 8U);
    EXPECT_EQ(Float->alignment(), 8U);
    EXPECT_TRUE(FloatType::classof(Float.get()));
  }
} // namespace ink::ir::test
