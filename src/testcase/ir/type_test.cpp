#include "ink/target/target_context.h"
#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/null_pointer_constant.h"
#include "ink/ir/constant/string_constant.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/param/parameter.h"
#include "ink/ir/type/structure_type/array_type.h"
#include "ink/ir/type/builtin_type/bool_type.h"
#include "ink/ir/type/nominal_type/class_type.h"
#include "ink/ir/type/builtin_type/float_type.h"
#include "ink/ir/type/structure_type/function_type.h"
#include "ink/ir/type/generic_type/generic_class_type.h"
#include "ink/ir/type/generic_type/generic_function_type.h"
#include "ink/ir/type/generic_type/generic_type.h"
#include "ink/ir/type/generic_type/generic_var_type.h"
#include "ink/ir/type/builtin_type/integer_type.h"
#include "ink/ir/type/nominal_type/interface_type.h"
#include "ink/ir/type/builtin_type/null_pointer_type.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/ir/type/structure_type/reference_type.h"
#include "ink/ir/type/nominal_type/struct_type.h"
#include "ink/ir/type/type.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/builtin_type/void_type.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
#include <type_traits>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
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

    class SyntheticInstruction final : public Instruction
    {
      public:
        SyntheticInstruction() noexcept = default;
    };
  } // namespace

  static_assert(std::has_virtual_destructor_v<Value>);
  static_assert(std::has_virtual_destructor_v<Type>);
  static_assert(!std::is_default_constructible_v<VoidType>);
  static_assert(!std::is_default_constructible_v<BoolType>);
  static_assert(!std::is_constructible_v<IntegerType, const target::TargetContext &, std::uint32_t, bool>);
  static_assert(!std::is_constructible_v<FloatType, const target::TargetContext &, std::uint32_t>);
  static_assert(!std::is_constructible_v<PointerType, const target::TargetContext &, const Type *>);
  static_assert(!std::is_constructible_v<ReferenceType, const target::TargetContext &, const Type *>);
  static_assert(!std::is_constructible_v<ArrayType, const Type *, std::uint64_t, std::uint64_t, std::uint64_t>);
  static_assert(!std::is_constructible_v<FunctionType, const target::TargetContext &, const Type *, std::vector<const Type *>>);
  static_assert(!std::is_constructible_v<ClassType, std::uint64_t, std::uint64_t>);
  static_assert(!std::is_constructible_v<StructType, std::uint64_t, std::uint64_t>);
  static_assert(!std::is_constructible_v<InterfaceType, std::uint64_t, std::uint64_t>);
  static_assert(!std::is_constructible_v<GenericClassType, core::Name, std::vector<const Type *>>);
  static_assert(!std::is_constructible_v<GenericFunctionType, core::Name, std::vector<const Type *>, const Type *, std::vector<const Type *>>);
  static_assert(!std::is_constructible_v<GenericVarType, core::Name, std::vector<const Type *>, const Type *>);
  static_assert(!std::is_copy_constructible_v<BoolType> && !std::is_move_constructible_v<BoolType>);
  static_assert(!std::is_copy_constructible_v<IntegerType> && !std::is_move_constructible_v<IntegerType>);
  static_assert(!std::is_copy_constructible_v<FunctionType> && !std::is_move_constructible_v<FunctionType>);

  // Both integer widths preserve signedness and use the supplied target's scalar alignment.
  TEST(IRTypeTest, IntegerWidthsSignednessAndTargetAlignment)
  {
    Module Types(Target64);
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      Module TargetTypes(Target);
      for (const std::uint32_t Width : {32U, 64U})
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

    Module Types32(Target32);
    const auto Integer32Target = Types32.typePool().getInt64Type();
    const auto Integer64Target = Types.typePool().getIntegerType(64, true);
    ASSERT_NE(Integer32Target, nullptr);
    ASSERT_NE(Integer64Target, nullptr);
    EXPECT_EQ(Integer32Target->alignment(), 4U);
    EXPECT_EQ(Integer64Target->alignment(), 8U);
  }

  // The unsigned byte type has one-byte storage and alignment on both pointer widths, while signed bytes remain unsupported.
  TEST(IRTypeTest, UInt8HasOneByteLayoutOnEveryTarget)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      Module Types(Target);
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
      EXPECT_EQ(Types.typePool().getIntegerType(8, true), nullptr);
    }
  }

  // Unsupported integer widths return an explicit failure for either signedness.
  TEST(IRTypeTest, IntegerRejectsUnsupportedWidths)
  {
    Module Types(Target64);
    constexpr std::uint32_t UnsupportedWidths[] = {
        0,
        1,
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
      EXPECT_EQ(Types.typePool().getIntegerType(Width, false), nullptr);
      EXPECT_EQ(Types.typePool().getIntegerType(Width, true), nullptr);
    }
  }

  // Float represents both 32-bit and 64-bit values under the same kind and target-specific alignment.
  TEST(IRTypeTest, FloatWidthsAndTargetAlignment)
  {
    Module Types(Target64);
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      Module TargetTypes(Target);
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

    Module Types32(Target32);
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
    Module Types(Target64);
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
    Module Types(Target64);
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
      Module Types(Target);
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
    Module Types(Target64);
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
    Module Types(Target64);
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
    Module Types(Target64);
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
    Module Types(Target64);
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
    Module Types(Target64);
    const auto LargestAlignedElement = Types.typePool().getArrayType(Types.typePool().getInt64Type(), MaximumSize / 8);
    ASSERT_NE(LargestAlignedElement, nullptr);
    const auto Single = Types.typePool().getArrayType(LargestAlignedElement, 1);
    ASSERT_NE(Single, nullptr);
    EXPECT_EQ(Single->size(), MaximumSize - 7);
    EXPECT_EQ(Single->alignment(), 8U);
  }

  // Arrays reject externally constructed types regardless of their layout or requested element count.
  TEST(IRTypeTest, ArrayRejectsExternallyConstructedTypes)
  {
    Module Types(Target64);
    const SyntheticType ValidLayout(4, 4);
    const SyntheticType ZeroAlignment(4, 0);
    const SyntheticType NonPowerOfTwoAlignment(4, 3);
    EXPECT_EQ(Types.typePool().getArrayType(&ValidLayout, 0), nullptr);
    EXPECT_EQ(Types.typePool().getArrayType(&ValidLayout, 1), nullptr);
    EXPECT_EQ(Types.typePool().getArrayType(&ZeroAlignment, 0), nullptr);
    EXPECT_EQ(Types.typePool().getArrayType(&ZeroAlignment, 1), nullptr);
    EXPECT_EQ(Types.typePool().getArrayType(&NonPowerOfTwoAlignment, 0), nullptr);
    EXPECT_EQ(Types.typePool().getArrayType(&NonPowerOfTwoAlignment, 1), nullptr);
  }

  // Array construction rejects a missing element type for both empty and nonempty arrays.
  TEST(IRTypeTest, ArrayRejectsNullElementType)
  {
    Module Types(Target64);
    EXPECT_EQ(Types.typePool().getArrayType(nullptr, 0), nullptr);
    EXPECT_EQ(Types.typePool().getArrayType(nullptr, 1), nullptr);
  }

  // Type, function, parameter, instruction, block and constant classifiers reject unrelated, null and unknown values.
  TEST(IRTypeTest, ClassifiersRejectNullAndOtherKinds)
  {
    Module Types(Target64);
    auto &Constants = Types.constantPool();
    const auto Integer = Types.typePool().getIntegerType(32, true);
    const auto Float = Types.typePool().getFloatType(64);
    const auto Bool = Types.typePool().getBoolType();
    const auto Array = Types.typePool().getArrayType(Bool, 2);
    const auto Pointer = Types.typePool().getPointerType(Bool);
    const auto Reference = Types.typePool().getReferenceType(Bool);
    const auto Function = Types.typePool().getFunctionType(Bool, {Bool});
    const BasicBlock Block;
    const SyntheticInstruction Operation;
    ASSERT_NE(Integer, nullptr);
    ASSERT_NE(Float, nullptr);
    ASSERT_NE(Array, nullptr);
    ASSERT_NE(Function, nullptr);

    const ink::ir::Function Callable(*Function);
    const Parameter *ParameterValue = Callable.parameter(0);
    ASSERT_NE(ParameterValue, nullptr);

    const auto BoolValue = Constants.getBoolConstant(true);
    const auto IntegerValue = Constants.getSignedIntegerConstant(Integer, -1);
    const auto FloatValue = Constants.getFloatConstantFromBits(Float, 0);
    const auto ArrayValue = Constants.getArrayConstant(Array, {BoolValue, BoolValue});
    const auto NullValue = Constants.getNullPointerConstant();
    const auto StringValue = Constants.getStringConstant("text");
    ASSERT_NE(BoolValue, nullptr);
    ASSERT_NE(IntegerValue, nullptr);
    ASSERT_NE(FloatValue, nullptr);
    ASSERT_NE(ArrayValue, nullptr);
    ASSERT_NE(NullValue, nullptr);
    ASSERT_NE(StringValue, nullptr);

    const std::array<const Value *, 19> Values = {
        Types.typePool().getVoidType(),
        Integer,
        Float,
        Bool,
        Array,
        Pointer,
        Reference,
        Function,
        &Callable,
        ParameterValue,
        &Block,
        &Operation,
        BoolValue,
        IntegerValue,
        FloatValue,
        ArrayValue,
        NullValue,
        StringValue,
        Types.typePool().getNullPointerType(),
    };
    using Classifier = bool (*)(const Value *);
    const std::array<Classifier, 19> Classifiers = {
        VoidType::classof,
        IntegerType::classof,
        FloatType::classof,
        BoolType::classof,
        ArrayType::classof,
        PointerType::classof,
        ReferenceType::classof,
        FunctionType::classof,
        ink::ir::Function::classof,
        Parameter::classof,
        BasicBlock::classof,
        Instruction::classof,
        BoolConstant::classof,
        IntegerConstant::classof,
        FloatConstant::classof,
        ArrayConstant::classof,
        NullPointerConstant::classof,
        StringConstant::classof,
        NullPointerType::classof,
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

    const std::array<Classifier, 6> DeclarationClassifiers = {
        ClassType::classof,
        InterfaceType::classof,
        StructType::classof,
        GenericClassType::classof,
        GenericFunctionType::classof,
        GenericVarType::classof,
    };
    for (const Classifier Classify : DeclarationClassifiers)
    {
      EXPECT_FALSE(Classify(nullptr));
      EXPECT_FALSE(Classify(&Unknown));
      for (const Value *Value : Values)
      {
        EXPECT_FALSE(Classify(Value));
      }
    }

    EXPECT_FALSE(GenericType::classof(nullptr));
    EXPECT_FALSE(GenericType::classof(&Unknown));
    EXPECT_FALSE(Type::classof(nullptr));
    EXPECT_FALSE(Type::classof(&Unknown));
    EXPECT_FALSE(Constant::classof(nullptr));
    EXPECT_FALSE(Constant::classof(&Unknown));
    for (const Value *Value : Values)
    {
      const bool IsConstant = Value == BoolValue || Value == IntegerValue || Value == FloatValue || Value == ArrayValue || Value == NullValue || Value == StringValue;
      EXPECT_FALSE(GenericType::classof(Value));
      EXPECT_EQ(Constant::classof(Value), IsConstant);
      EXPECT_EQ(Type::classof(Value), Value != &Callable && Value != ParameterValue && Value != &Block && Value != &Operation && !IsConstant);
    }
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
    Module Types(Target64);
    Module Types32(Target32);
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
