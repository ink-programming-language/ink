#include "ink/target/target_context.h"
#include "ink/ir/type/structure_type/array_type.h"
#include "ink/ir/type/builtin_type/bool_type.h"
#include "ink/ir/type/nominal_type/class_type.h"
#include "ink/ir/type/builtin_type/float_type.h"
#include "ink/ir/type/structure_type/function_type.h"
#include "ink/ir/type/generic_type/generic_class_type.h"
#include "ink/ir/type/generic_type/generic_function_type.h"
#include "ink/ir/type/generic_type/generic_var_type.h"
#include "ink/ir/type/builtin_type/integer_type.h"
#include "ink/ir/type/nominal_type/interface_type.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/ir/type/structure_type/reference_type.h"
#include "ink/ir/type/nominal_type/struct_type.h"
#include "ink/ir/type/type.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/builtin_type/void_type.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    constexpr std::uint64_t MaximumSize = std::numeric_limits<std::uint64_t>::max();
  } // namespace

  static_assert(std::has_virtual_destructor_v<Value>);
  static_assert(std::has_virtual_destructor_v<Type>);
  static_assert(!std::is_default_constructible_v<VoidType>);
  static_assert(!std::is_default_constructible_v<BoolType>);
  static_assert(!std::is_constructible_v<IntegerType, const MetaType &, const target::TargetContext &, std::uint32_t, bool>);
  static_assert(!std::is_constructible_v<FloatType, const MetaType &, const target::TargetContext &, std::uint32_t>);
  static_assert(!std::is_constructible_v<PointerType, const MetaType &, const target::TargetContext &, const Type *>);
  static_assert(!std::is_constructible_v<ReferenceType, const MetaType &, const target::TargetContext &, const Type *>);
  static_assert(!std::is_constructible_v<ArrayType, const MetaType &, const Type *, std::uint64_t, std::uint64_t, std::uint64_t>);
  static_assert(!std::is_constructible_v<FunctionType, const MetaType &, const target::TargetContext &, const Type *, std::span<const FunctionTypeParameter>>);
  static_assert(!std::is_constructible_v<ClassType, std::uint64_t, std::uint64_t>);
  static_assert(!std::is_constructible_v<StructType, std::uint64_t, std::uint64_t>);
  static_assert(!std::is_constructible_v<InterfaceType, std::uint64_t, std::uint64_t>);
  static_assert(!std::is_constructible_v<GenericClassType, const MetaType &, core::Name, std::vector<const Type *>>);
  static_assert(!std::is_constructible_v<GenericFunctionType, const MetaType &, core::Name, std::vector<const Type *>, const Type *, std::vector<const Type *>>);
  static_assert(!std::is_constructible_v<GenericVarType, const MetaType &, core::Name, std::vector<const Type *>, const Type *>);
  static_assert(!std::is_copy_constructible_v<BoolType> && !std::is_move_constructible_v<BoolType>);
  static_assert(!std::is_copy_constructible_v<IntegerType> && !std::is_move_constructible_v<IntegerType>);
  static_assert(!std::is_copy_constructible_v<FunctionType> && !std::is_move_constructible_v<FunctionType>);

  // Every supported integer width preserves signedness and uses the supplied target's scalar alignment.
  TEST(IRTypeTest, IntegerWidthsSignednessAndTargetAlignment)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TargetTypesContext(Target);
      Module TargetTypes(TargetTypesContext);
      for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
      {
        for (const bool Signed : {false, true})
        {
          const auto Integer = TargetTypes.typePool().getIntegerType(Width, Signed);
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

    IRContext Types32Context(Target32);
    Module Types32(Types32Context);
    const auto Integer32Target = Types32.typePool().getInt64Type();
    const auto Integer64Target = Types.typePool().getIntegerType(64, true);
    ASSERT_NE(Integer32Target, nullptr);
    ASSERT_NE(Integer64Target, nullptr);
    EXPECT_EQ(Integer32Target->alignment(), 4U);
    EXPECT_EQ(Integer64Target->alignment(), 8U);
  }

  // The unsigned byte type has one-byte storage and alignment on both pointer widths.
  TEST(IRTypeTest, UInt8HasOneByteLayoutOnEveryTarget)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TypesContext(Target);
      Module Types(TypesContext);
      const auto Byte = Types.typePool().getUInt8Type();
      ASSERT_NE(Byte, nullptr);
      EXPECT_EQ(Byte, Types.typePool().getIntegerType(8, false));
      EXPECT_EQ(Byte->kind(), ValueKind::Integer);
      EXPECT_EQ(Byte->width(), 8U);
      EXPECT_TRUE(Byte->isUnsigned());
      EXPECT_FALSE(Byte->isSigned());
      EXPECT_EQ(Byte->size(), 1U);
      EXPECT_EQ(Byte->alignment(), 1U);
      EXPECT_TRUE(Type::classof(Byte));
      EXPECT_TRUE(IntegerType::classof(Byte));
      EXPECT_EQ(Types.typePool().getIntegerType(8, true), Types.typePool().getInt8Type());
    }
  }

  // Integer limits include each signed minimum and the full unsigned range without overflowing host arithmetic.
  TEST(IRTypeTest, IntegerNumericBoundsMatchWidthAndSignedness)
  {
    IRContext TypesContext(Target64);
    auto &Types = TypesContext.typePool();
    struct Case
    {
        const IntegerType *Type;
        std::int64_t Minimum;
        std::uint64_t Maximum;
    };
    const Case Cases[] = {
        {Types.getInt8Type(), -128, 127},
        {Types.getUInt8Type(), 0, 255},
        {Types.getInt16Type(), -32768, 32767},
        {Types.getUInt16Type(), 0, 65535},
        {Types.getInt32Type(), std::numeric_limits<std::int32_t>::min(), std::numeric_limits<std::int32_t>::max()},
        {Types.getUInt32Type(), 0, std::numeric_limits<std::uint32_t>::max()},
        {Types.getInt64Type(), std::numeric_limits<std::int64_t>::min(), std::numeric_limits<std::int64_t>::max()},
        {Types.getUInt64Type(), 0, std::numeric_limits<std::uint64_t>::max()},
    };
    for (const auto &Entry : Cases)
    {
      ASSERT_NE(Entry.Type, nullptr);
      SCOPED_TRACE(Entry.Type->toString());
      EXPECT_EQ(Entry.Type->minimumValue(), Entry.Minimum);
      EXPECT_EQ(Entry.Type->maximumValue(), Entry.Maximum);
    }
  }

  // Unsupported integer widths return an explicit failure for either signedness.
  TEST(IRTypeTest, IntegerRejectsUnsupportedWidths)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    constexpr std::uint32_t UnsupportedWidths[] = {
        0,
        1,
        7,
        9,
        15,
        17,
        31,
        33,
        63,
        65,
        128,
        std::numeric_limits<std::uint32_t>::max(),
    };
    for (const std::uint32_t Width : UnsupportedWidths)
    {
      EXPECT_EQ(Types.typePool().getIntegerType(Width, false), nullptr);
      EXPECT_EQ(Types.typePool().getIntegerType(Width, true), nullptr);
    }
  }

  // Float represents both 32-bit and 64-bit values under the same kind and target-specific alignment.
  TEST(IRTypeTest, FloatWidthsAndTargetAlignment)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TargetTypesContext(Target);
      Module TargetTypes(TargetTypesContext);
      for (const std::uint32_t Width : {32U, 64U})
      {
        const auto Float = TargetTypes.typePool().getFloatType(Width);
        ASSERT_NE(Float, nullptr);
        EXPECT_EQ(Float->kind(), ValueKind::Float);
        EXPECT_EQ(Float->width(), Width);
        EXPECT_EQ(Float->size(), Width / 8);
        EXPECT_EQ(Float->alignment(), Target.floatAlignment(Width));
      }
    }

    IRContext Types32Context(Target32);
    Module Types32(Types32Context);
    const auto Float32Target = Types32.typePool().getFloat64Type();
    const auto Float64Target = Types.typePool().getFloatType(64);
    ASSERT_NE(Float32Target, nullptr);
    ASSERT_NE(Float64Target, nullptr);
    EXPECT_EQ(Float32Target->alignment(), 4U);
    EXPECT_EQ(Float64Target->alignment(), 8U);
  }

  // Float rejects zero, neighboring widths, unsupported scalar widths and oversized values.
  TEST(IRTypeTest, FloatRejectsUnsupportedWidths)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
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
      EXPECT_EQ(Types.typePool().getFloatType(Width), nullptr);
    }
  }

  // Bool exposes its one-byte layout through Type and its kind through Value.
  TEST(IRTypeTest, BoolHasOneBytePolymorphicLayout)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Bool = Types.typePool().getBoolType();
    const Type *TypeValue = Bool;
    const Value *ValueValue = Bool;
    EXPECT_EQ(TypeValue->kind(), ValueKind::Bool);
    EXPECT_EQ(TypeValue->size(), 1U);
    EXPECT_EQ(TypeValue->alignment(), 1U);
    EXPECT_EQ(ValueValue->kind(), ValueKind::Bool);
    EXPECT_TRUE(Type::classof(ValueValue));
    EXPECT_TRUE(BoolType::classof(ValueValue));
  }

  // Pointer and reference layouts follow the selected target while preserving the referenced type identity.
  TEST(IRTypeTest, PointerAndReferenceUseTargetWidth)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TypesContext(Target);
      Module Types(TypesContext);
      const auto Element = Types.typePool().getBoolType();
      const auto Pointer = Types.typePool().getPointerType(Element);
      const auto Reference = Types.typePool().getReferenceType(Element);
      ASSERT_NE(Pointer, nullptr);
      ASSERT_NE(Reference, nullptr);
      EXPECT_EQ(Pointer->kind(), ValueKind::Pointer);
      EXPECT_EQ(Reference->kind(), ValueKind::Reference);
      EXPECT_EQ(Pointer->pointeeType(), Element);
      EXPECT_EQ(Reference->referencedType(), Element);
      EXPECT_EQ(Pointer->size(), Target.pointerByteWidth());
      EXPECT_EQ(Pointer->alignment(), Target.pointerByteWidth());
      EXPECT_EQ(Reference->size(), Target.pointerByteWidth());
      EXPECT_EQ(Reference->alignment(), Target.pointerByteWidth());
    }
  }

  // Nested arrays retain their immediate element identity and compose element count, size and alignment.
  TEST(IRTypeTest, ArrayPreservesElementsAndNestedLayout)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Element = Types.typePool().getIntegerType(32, false);
    ASSERT_NE(Element, nullptr);
    const auto Inner = Types.typePool().getArrayType(Element, 3);
    ASSERT_NE(Inner, nullptr);
    const auto Outer = Types.typePool().getArrayType(Inner, 2);
    ASSERT_NE(Outer, nullptr);
    EXPECT_EQ(Inner->kind(), ValueKind::Array);
    EXPECT_EQ(Inner->elementType(), Element);
    EXPECT_EQ(Inner->elementCount(), 3U);
    EXPECT_EQ(Inner->size(), 12U);
    EXPECT_EQ(Inner->alignment(), 4U);
    EXPECT_EQ(Outer->elementType(), Inner);
    EXPECT_EQ(Outer->elementCount(), 2U);
    EXPECT_EQ(Outer->size(), 24U);
    EXPECT_EQ(Outer->alignment(), 4U);
  }

  // Empty arrays remain valid with zero storage even for the largest valid element layout.
  TEST(IRTypeTest, EmptyArrayAcceptsMaximumElementSize)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Element = Types.typePool().getArrayType(Types.typePool().getBoolType(), MaximumSize);
    ASSERT_NE(Element, nullptr);
    const auto Array = Types.typePool().getArrayType(Element, 0);
    ASSERT_NE(Array, nullptr);
    EXPECT_EQ(Array->size(), 0U);
    EXPECT_EQ(Array->alignment(), 1U);
    EXPECT_EQ(Array->elementCount(), 0U);
    EXPECT_EQ(Array->elementType(), Element);
  }

  // Zero-sized elements accept the largest element count without introducing storage or losing alignment.
  TEST(IRTypeTest, ArrayOfZeroSizedElementsAcceptsMaximumCount)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Element = Types.typePool().getArrayType(Types.typePool().getInt64Type(), 0);
    ASSERT_NE(Element, nullptr);
    EXPECT_TRUE(Element->hasLayout());
    const auto Array = Types.typePool().getArrayType(Element, MaximumSize);
    ASSERT_NE(Array, nullptr);
    EXPECT_EQ(Array->size(), 0U);
    EXPECT_EQ(Array->alignment(), 8U);
    EXPECT_EQ(Array->elementCount(), MaximumSize);
    const auto Nested = Types.typePool().getArrayType(Array, MaximumSize);
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested->size(), 0U);
    EXPECT_EQ(Nested->alignment(), 8U);
  }

  // Array multiplication accepts the last representable count and explicitly rejects the first overflowing count.
  TEST(IRTypeTest, ArrayChecksElementCountMultiplicationOverflow)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto Element = Types.typePool().getIntegerType(64, true);
    ASSERT_NE(Element, nullptr);
    const auto Largest = Types.typePool().getArrayType(Element, MaximumSize / 8);
    ASSERT_NE(Largest, nullptr);
    EXPECT_EQ(Largest->size(), MaximumSize - 7);
    EXPECT_EQ(Types.typePool().getArrayType(Element, MaximumSize / 8 + 1), nullptr);

    const auto HugeElement = Types.typePool().getArrayType(Types.typePool().getBoolType(), MaximumSize);
    ASSERT_NE(HugeElement, nullptr);
    const auto Single = Types.typePool().getArrayType(HugeElement, 1);
    ASSERT_NE(Single, nullptr);
    EXPECT_EQ(Single->size(), MaximumSize);
    EXPECT_EQ(Types.typePool().getArrayType(HugeElement, 2), nullptr);
  }

  // Arrays accept the largest size aligned to eight bytes without overflowing their element stride.
  TEST(IRTypeTest, ArrayAcceptsLargestAlignedElement)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const auto LargestAlignedElement = Types.typePool().getArrayType(Types.typePool().getInt64Type(), MaximumSize / 8);
    ASSERT_NE(LargestAlignedElement, nullptr);
    const auto Single = Types.typePool().getArrayType(LargestAlignedElement, 1);
    ASSERT_NE(Single, nullptr);
    EXPECT_EQ(Single->size(), MaximumSize - 7);
    EXPECT_EQ(Single->alignment(), 8U);
  }

  // Array construction rejects a missing element type for both empty and nonempty arrays.
  TEST(IRTypeTest, ArrayRejectsNullElementType)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    EXPECT_EQ(Types.typePool().getArrayType(nullptr, 0), nullptr);
    EXPECT_EQ(Types.typePool().getArrayType(nullptr, 1), nullptr);
  }

  // Registry names distinguish types, generic declarations, basic blocks, instructions and constants while retaining an unknown fallback.
  TEST(IRTypeTest, ValueKindNamesCoverConcreteTypes)
  {
    EXPECT_EQ(valueKindName(ValueKind::Void), "void");
    EXPECT_EQ(valueKindName(ValueKind::Integer), "integer");
    EXPECT_EQ(valueKindName(ValueKind::Float), "float");
    EXPECT_EQ(valueKindName(ValueKind::Bool), "bool");
    EXPECT_EQ(valueKindName(ValueKind::Array), "array");
    EXPECT_EQ(valueKindName(ValueKind::Class), "class");
    EXPECT_EQ(valueKindName(ValueKind::Interface), "interface");
    EXPECT_EQ(valueKindName(ValueKind::Struct), "struct");
    EXPECT_EQ(valueKindName(ValueKind::Pointer), "pointer");
    EXPECT_EQ(valueKindName(ValueKind::NullPointer), "null_pointer");
    EXPECT_EQ(valueKindName(ValueKind::Reference), "reference");
    EXPECT_EQ(valueKindName(ValueKind::FunctionType), "function_type");
    EXPECT_EQ(valueKindName(ValueKind::Function), "function");
    EXPECT_EQ(valueKindName(ValueKind::Parameter), "parameter");
    EXPECT_EQ(valueKindName(ValueKind::GenericClass), "generic_class");
    EXPECT_EQ(valueKindName(ValueKind::GenericFunction), "generic_function");
    EXPECT_EQ(valueKindName(ValueKind::GenericVar), "generic_var");
    EXPECT_EQ(valueKindName(ValueKind::BasicBlock), "basic_block");
    EXPECT_EQ(valueKindName(ValueKind::Instruction), "instruction");
    EXPECT_EQ(valueKindName(ValueKind::BoolConstant), "bool_constant");
    EXPECT_EQ(valueKindName(ValueKind::IntegerConstant), "integer_constant");
    EXPECT_EQ(valueKindName(ValueKind::FloatConstant), "float_constant");
    EXPECT_EQ(valueKindName(ValueKind::ArrayConstant), "array_constant");
    EXPECT_EQ(valueKindName(ValueKind::NullPointerConstant), "null_pointer_constant");
    EXPECT_EQ(valueKindName(ValueKind::StringConstant), "string_constant");
    EXPECT_EQ(valueKindName(static_cast<ValueKind>(255)), "unknown");
  }

  // TypePool-owned types expose their layout through borrowed Type and Value pointers.
  TEST(IRTypeTest, PoolOwnedTypePreservesConcretePolymorphicLayout)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    IRContext Types32Context(Target32);
    Module Types32(Types32Context);
    const Type *Integer = Types32.typePool().getInt64Type();
    ASSERT_NE(Integer, nullptr);
    const Value *IntegerValue = Integer;
    EXPECT_EQ(IntegerValue->kind(), ValueKind::Integer);
    EXPECT_EQ(Integer->size(), 8U);
    EXPECT_EQ(Integer->alignment(), 4U);
    EXPECT_TRUE(Type::classof(IntegerValue));
    EXPECT_TRUE(IntegerType::classof(IntegerValue));
    const Type *Float = Types.typePool().getFloat64Type();
    ASSERT_NE(Float, nullptr);
    const Value *FloatValue = Float;
    EXPECT_EQ(FloatValue->kind(), ValueKind::Float);
    EXPECT_EQ(Float->size(), 8U);
    EXPECT_EQ(Float->alignment(), 8U);
    EXPECT_TRUE(Type::classof(FloatValue));
    EXPECT_TRUE(FloatType::classof(FloatValue));
  }
} // namespace ink::ir::test
