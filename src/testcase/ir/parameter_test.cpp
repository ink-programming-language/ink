#include "ink/core/name_pool.h"
#include "ink/target/target_context.h"
#include "ink/ir/parameter.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // Omitting metadata creates a fixed parameter with no name while retaining its borrowed type.
  TEST(IRParameterTest, DefaultsToUnnamedFixedParameter)
  {
    const target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target);
    const auto Bool = Types.getBoolType();
    const Parameter Value(Bool);
    EXPECT_EQ(Value.type(), Bool);
    EXPECT_EQ(Value.name(), core::NAME_None);
    EXPECT_FALSE(Value.hasName());
    EXPECT_FALSE(Value.isVariadic());
  }

  // A named variadic parameter retains both the interned name identity and its type.
  TEST(IRParameterTest, PreservesNamedVariadicMetadata)
  {
    const target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target);
    const auto Bool = Types.getBoolType();
    const core::Name Name = core::NamePool::instance().intern("IRParameterTest.Values");
    ASSERT_TRUE(Name.valid());
    const Parameter Value(Bool, Name, true);
    EXPECT_EQ(Value.type(), Bool);
    EXPECT_EQ(Value.name(), Name);
    EXPECT_TRUE(Value.hasName());
    EXPECT_TRUE(Value.isVariadic());
  }

  // Naming and variadic status are independent: fixed parameters may be named and variadic parameters may be unnamed.
  TEST(IRParameterTest, NameAndVariadicFlagAreIndependent)
  {
    const target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target);
    const auto Bool = Types.getBoolType();
    const core::Name Name = core::NamePool::instance().intern("IRParameterTest.Fixed");
    ASSERT_TRUE(Name.valid());
    const Parameter NamedFixed(Bool, Name);
    EXPECT_EQ(NamedFixed.name(), Name);
    EXPECT_TRUE(NamedFixed.hasName());
    EXPECT_FALSE(NamedFixed.isVariadic());
    const Parameter UnnamedVariadic(Bool, core::NAME_None, true);
    EXPECT_EQ(UnnamedVariadic.name(), core::NAME_None);
    EXPECT_FALSE(UnnamedVariadic.hasName());
    EXPECT_TRUE(UnnamedVariadic.isVariadic());
  }

  // Copying a descriptor retains its complete metadata even after the source is assigned a different parameter.
  TEST(IRParameterTest, CopiedDescriptorIsIndependentOfSourceAssignment)
  {
    const target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target);
    const auto Bool = Types.getBoolType();
    const auto Pointer = Types.getPointerType(Bool);
    const core::Name Name = core::NamePool::instance().intern("IRParameterTest.Copy");
    ASSERT_TRUE(Name.valid());
    Parameter Original(Bool, Name, true);
    const Parameter Copy = Original;
    Original = Parameter(Pointer);
    EXPECT_EQ(Copy.type(), Bool);
    EXPECT_EQ(Copy.name(), Name);
    EXPECT_TRUE(Copy.hasName());
    EXPECT_TRUE(Copy.isVariadic());
    EXPECT_EQ(Original.type(), Pointer);
    EXPECT_EQ(Original.name(), core::NAME_None);
    EXPECT_FALSE(Original.hasName());
    EXPECT_FALSE(Original.isVariadic());
  }

  // An unresolved type remains null so that each signature consumer can apply its own validation policy.
  TEST(IRParameterTest, PreservesUnresolvedTypeWithoutLosingMetadata)
  {
    const core::Name Name = core::NamePool::instance().intern("IRParameterTest.Unresolved");
    ASSERT_TRUE(Name.valid());
    const Parameter Value(nullptr, Name, true);
    EXPECT_EQ(Value.type(), nullptr);
    EXPECT_EQ(Value.name(), Name);
    EXPECT_TRUE(Value.hasName());
    EXPECT_TRUE(Value.isVariadic());
  }
} // namespace ink::ir::test
