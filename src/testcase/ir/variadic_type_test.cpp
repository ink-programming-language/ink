#include "ink/ir/type/type_context.h"
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
    static_assert(std::is_same_v<decltype(std::declval<const TypeContext &>().getVariadicType()), const VariadicType *>);
    static_assert(noexcept(std::declval<const TypeContext &>().getVariadicType()));
    static_assert(!fixedValueSize(ValueKind::Variadic).has_value());
  } // namespace

  // Each context owns one immutable variadic marker, with separate identities even for equal targets.
  TEST(IRVariadicTypeTest, ContextOwnsCanonicalMarker)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      const TypeContext Types(Target);
      const TypeContext Other(Target);
      const VariadicType *Variadic = Types.getVariadicType();
      ASSERT_NE(Variadic, nullptr);
      EXPECT_EQ(Variadic, Types.getVariadicType());
      EXPECT_NE(Variadic, Other.getVariadicType());
      EXPECT_TRUE(Types.owns(Variadic));
      EXPECT_FALSE(Other.owns(Variadic));
    }
  }

  // The ellipsis marker is a Type and a Value, but its zero size does not denote a runtime unit layout.
  TEST(IRVariadicTypeTest, MarkerHasNoRuntimeLayout)
  {
    const TypeContext Types(Target64);
    const Type *TypeValue = Types.getVariadicType();
    const Value *ValueValue = TypeValue;
    EXPECT_EQ(ValueValue->kind(), ValueKind::Variadic);
    EXPECT_EQ(valueKindName(ValueValue->kind()), "variadic");
    EXPECT_TRUE(Type::classof(ValueValue));
    EXPECT_TRUE(VariadicType::classof(ValueValue));
    EXPECT_FALSE(VariadicType::classof(Types.getBoolType()));
    EXPECT_FALSE(VariadicType::classof(Types.getVoidType()));
    EXPECT_FALSE(VariadicType::classof(nullptr));
    EXPECT_FALSE(TypeValue->hasLayout());
    EXPECT_EQ(TypeValue->size(), 0U);
    EXPECT_EQ(TypeValue->alignment(), 1U);
    EXPECT_TRUE(Types.getVoidType()->hasLayout());
    EXPECT_FALSE(fixedValueSize(ValueKind::Variadic).has_value());
  }

  // Layout-bearing arrays and function signatures reject the marker as an element, return type or parameter type.
  TEST(IRVariadicTypeTest, LayoutFactoriesRejectMarkerOperands)
  {
    TypeContext Types(Target64);
    const VariadicType *Variadic = Types.getVariadicType();
    const VoidType *Void = Types.getVoidType();
    EXPECT_EQ(Types.getArrayType(Variadic, 0), nullptr);
    EXPECT_EQ(Types.getArrayType(Variadic, 1), nullptr);
    EXPECT_EQ(Types.getFunctionType(Variadic), nullptr);
    EXPECT_EQ(Types.getFunctionType(Void, {Variadic}), nullptr);
  }
} // namespace ink::ir::test
