#include "ink/ir/module/module.h"
#include "ink/ir/type/builtin_type/variadic_type.h"

#include <gtest/gtest.h>

#include <optional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::BigEndian);

    constexpr std::optional<std::uint64_t> fixedValueSize(ValueKind Kind) noexcept
    {
      switch (Kind)
      {
#define INK_VALUE(Kind, Name, FixedSize) \
  case ValueKind::Kind:                  \
    return FixedSize;
#include "ink/ir/type/Value.def"
#undef INK_VALUE
      }
      return std::nullopt;
    }

    static_assert(std::is_base_of_v<Type, VariadicType>);
    static_assert(std::is_final_v<VariadicType>);
    static_assert(!std::is_default_constructible_v<VariadicType>);
    static_assert(!std::is_copy_constructible_v<VariadicType> && !std::is_move_constructible_v<VariadicType>);
    static_assert(std::is_same_v<decltype(std::declval<const Module &>().typePool().get<ink::ir::VariadicType>()), const VariadicType *>);
    static_assert(noexcept(std::declval<const Module &>().typePool().get<ink::ir::VariadicType>()));
    static_assert(!fixedValueSize(ValueKind::Variadic).has_value());
  } // namespace

  // Each type pool owns one immutable variadic marker, with separate identities even for equal targets.
  TEST(IRVariadicTypeTest, TypePoolOwnsCanonicalMarker)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext TypesContext(Target);
      const Module Types(TypesContext);
      IRContext OtherContext(Target);
      const Module Other(OtherContext);
      const VariadicType *Variadic = Types.typePool().get<ink::ir::VariadicType>();
      ASSERT_NE(Variadic, nullptr);
      EXPECT_EQ(Variadic, Types.typePool().get<ink::ir::VariadicType>());
      EXPECT_NE(Variadic, Other.typePool().get<ink::ir::VariadicType>());
      EXPECT_TRUE(Types.ownsType(Variadic));
      EXPECT_FALSE(Other.ownsType(Variadic));
    }
  }

  // The ellipsis marker is a Type and a Value, but its zero size does not denote a runtime unit layout.
  TEST(IRVariadicTypeTest, MarkerHasNoRuntimeLayout)
  {
    IRContext TypesContext(Target64);
    const Module Types(TypesContext);
    const Type *TypeValue = Types.typePool().get<ink::ir::VariadicType>();
    const Value *ValueValue = TypeValue;
    EXPECT_EQ(ValueValue->kind(), ValueKind::Variadic);
    EXPECT_EQ(valueKindName(ValueValue->kind()), "variadic");
    EXPECT_TRUE(Type::classof(ValueValue));
    EXPECT_TRUE(VariadicType::classof(ValueValue));
    EXPECT_FALSE(VariadicType::classof(Types.typePool().get<ink::ir::BoolType>()));
    EXPECT_FALSE(VariadicType::classof(Types.typePool().get<ink::ir::VoidType>()));
    EXPECT_FALSE(VariadicType::classof(nullptr));
    EXPECT_FALSE(TypeValue->hasLayout());
    EXPECT_EQ(TypeValue->size(), 0U);
    EXPECT_EQ(TypeValue->alignment(), 1U);
    EXPECT_TRUE(Types.typePool().get<ink::ir::VoidType>()->hasLayout());
    EXPECT_FALSE(fixedValueSize(ValueKind::Variadic).has_value());
  }

  // Layout-bearing arrays and function signatures reject the marker as an element, return type or parameter type.
  TEST(IRVariadicTypeTest, LayoutFactoriesRejectMarkerOperands)
  {
    IRContext TypesContext(Target64);
    Module Types(TypesContext);
    const VariadicType *Variadic = Types.typePool().get<ink::ir::VariadicType>();
    const VoidType *Void = Types.typePool().get<ink::ir::VoidType>();
    EXPECT_EQ(Types.typePool().get<ink::ir::ArrayType>(Variadic, 0), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::ArrayType>(Variadic, 1), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::FunctionType>(Variadic), nullptr);
    EXPECT_EQ(Types.typePool().get<ink::ir::FunctionType>(Void, {Variadic}), nullptr);
  }
} // namespace ink::ir::test
