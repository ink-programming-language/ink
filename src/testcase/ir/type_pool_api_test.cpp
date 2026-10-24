#include "ink/ir/context.h"
#include "ink/ir/type/nominal_type/class_type.h"
#include "ink/ir/type/type_pool.h"

#include <gtest/gtest.h>

#include <concepts>
#include <cstdint>
#include <initializer_list>
#include <span>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    template <typename PoolT, typename TypeT, typename... Args>
    concept CanGetType = requires(PoolT &Pool, Args &&... Arguments)
    {
      { Pool.template get<TypeT>(std::forward<Args>(Arguments)...) } -> std::same_as<const TypeT *>;
    };
  } // namespace

  // Template construction exposes only supported families and arguments, and cache-mutating lookups require a mutable pool.
  static_assert(CanGetType<TypePool, MetaType> && CanGetType<const TypePool, MetaType>);
  static_assert(CanGetType<TypePool, VariadicType> && CanGetType<const TypePool, VariadicType>);
  static_assert(CanGetType<const TypePool, VoidType> && CanGetType<const TypePool, VoidType, bool>);
  static_assert(CanGetType<const TypePool, BoolType> && CanGetType<const TypePool, BoolType, bool>);
  static_assert(CanGetType<const TypePool, NullPointerType> && CanGetType<const TypePool, NullPointerType, bool>);
  static_assert(CanGetType<TypePool, IntegerType, std::uint32_t, bool> && CanGetType<TypePool, IntegerType, std::uint32_t, bool, bool>);
  static_assert(CanGetType<TypePool, FloatType, std::uint32_t> && CanGetType<TypePool, FloatType, std::uint32_t, bool>);
  static_assert(!CanGetType<const TypePool, IntegerType, std::uint32_t, bool> && !CanGetType<const TypePool, FloatType, std::uint32_t>);
  static_assert(CanGetType<TypePool, PointerType, const Type *> && CanGetType<TypePool, PointerType, const Type *, bool>);
  static_assert(CanGetType<TypePool, ReferenceType, const Type *> && CanGetType<TypePool, ReferenceType, const Type *, bool>);
  static_assert(CanGetType<TypePool, ArrayType, const Type *, std::uint64_t> && CanGetType<TypePool, ArrayType, const Type *, std::uint64_t, bool>);
  static_assert(CanGetType<TypePool, FunctionType, const Type *>);
  static_assert(CanGetType<TypePool, FunctionType, const Type *, std::initializer_list<FunctionTypeParameter>>);
  static_assert(CanGetType<TypePool, FunctionType, const Type *, std::span<const FunctionTypeParameter>, bool>);
  static_assert(!CanGetType<const TypePool, PointerType, const Type *> && !CanGetType<const TypePool, FunctionType, const Type *>);
  static_assert(!CanGetType<TypePool, IntegerType> && !CanGetType<TypePool, FloatType>);
  static_assert(!CanGetType<TypePool, MetaType, bool> && !CanGetType<TypePool, VariadicType, bool>);
  static_assert(!CanGetType<TypePool, Type> && !CanGetType<TypePool, ClassType> && !CanGetType<TypePool, int>);

  // Numeric cache entries retain their identity and qualifier links across lookups of other widths and families.
  TEST(IRTypePoolApiTest, NumericLookupsPreserveBuiltinIdentityAndQualifiers)
  {
    IRContext Context(Target);
    IRContext Other(Target);
    auto &Types = Context.typePool();
    const auto *ConstFloat32 = Types.get<FloatType>(32, false);
    const auto *Float32 = Types.get<FloatType>(32);
    const auto *ConstFloat64 = Types.get<FloatType>(64, false);
    const auto *Float64 = Types.get<FloatType>(64);
    ASSERT_NE(ConstFloat32, nullptr);
    ASSERT_NE(Float32, nullptr);
    ASSERT_NE(ConstFloat64, nullptr);
    ASSERT_NE(Float64, nullptr);
    EXPECT_NE(Float32, Float64);
    EXPECT_NE(ConstFloat32, Float32);
    EXPECT_NE(ConstFloat64, Float64);
    EXPECT_EQ(ConstFloat32->unqualifiedType(), Float32);
    EXPECT_EQ(ConstFloat64->unqualifiedType(), Float64);
    EXPECT_EQ(Types.get(Float32, false), ConstFloat32);
    EXPECT_EQ(Types.get(ConstFloat64, true), Float64);
    for (const std::uint32_t Width : {8U, 16U, 32U, 64U})
    {
      for (const bool Signed : {false, true})
      {
        const auto *Const = Types.get<IntegerType>(Width, Signed, false);
        const auto *Mutable = Types.get<IntegerType>(Width, Signed);
        ASSERT_NE(Const, nullptr);
        ASSERT_NE(Mutable, nullptr);
        EXPECT_EQ(Types.get<IntegerType>(Width, Signed), Mutable);
        EXPECT_EQ(Types.get<IntegerType>(Width, Signed, false), Const);
        EXPECT_EQ(Const->unqualifiedType(), Mutable);
        EXPECT_EQ(Types.get(Mutable, false), Const);
        EXPECT_NE(Mutable, Types.get<IntegerType>(Width, !Signed));
        EXPECT_NE(Mutable, Other.typePool().get<IntegerType>(Width, Signed));
        EXPECT_NE(static_cast<const Type *>(Mutable), Float32);
        EXPECT_NE(static_cast<const Type *>(Mutable), Float64);
        EXPECT_TRUE(Context.containsType(Const));
        EXPECT_TRUE(Context.containsType(Mutable));
      }
    }
    for (std::uint32_t Width = 1; Width <= 256; ++Width)
    {
      ASSERT_NE(Types.get<IntegerType>(Width, true), nullptr);
    }
    EXPECT_EQ(Float32, Types.get<FloatType>(32));
    EXPECT_EQ(Float64, Types.get<FloatType>(64));
    EXPECT_EQ(ConstFloat32, Types.get<FloatType>(32, false));
    EXPECT_EQ(ConstFloat64, Types.get<FloatType>(64, false));
    EXPECT_EQ(ConstFloat32->unqualifiedType(), Float32);
    EXPECT_NE(Float32, Other.typePool().get<FloatType>(32));
    EXPECT_NE(ConstFloat64, Other.typePool().get<FloatType>(64, false));
    EXPECT_TRUE(Types.owns(Float32));
    EXPECT_TRUE(Types.owns(ConstFloat64));
    EXPECT_FALSE(Other.typePool().owns(Float32));
  }
} // namespace ink::ir::test
