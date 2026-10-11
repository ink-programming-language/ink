#include "ink/target/target_context.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/constant_context.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::ir::test
{
  static_assert(std::is_abstract_v<Constant>);
  static_assert(std::is_base_of_v<Value, Constant>);
  static_assert(!std::is_base_of_v<Type, Constant>);
  static_assert(std::has_virtual_destructor_v<Constant>);

  // Boolean constants share their context-owned type through borrowed polymorphic values.
  TEST(IRConstantTest, BoolValuesUseSharedTypeThroughValue)
  {
    const target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    TypeContext Types(Target);
    ConstantContext Constants(Types);
    const Type *SharedType = nullptr;
    for (const bool Expected : {false, true})
    {
      const Value *Borrowed = Constants.getBoolConstant(Expected);
      ASSERT_NE(Borrowed, nullptr);
      ASSERT_TRUE(BoolConstant::classof(Borrowed));
      const auto *Stored = static_cast<const BoolConstant *>(Borrowed);
      EXPECT_EQ(Stored->value(), Expected);
      EXPECT_EQ(Stored->type(), Types.getBoolType());
      if (SharedType)
      {
        EXPECT_EQ(Stored->type(), SharedType);
      }
      SharedType = Stored->type();
      EXPECT_EQ(Stored->type()->size(), 1U);
      EXPECT_EQ(Stored->type()->alignment(), 1U);
    }
    ASSERT_NE(SharedType, nullptr);
    EXPECT_TRUE(BoolType::classof(SharedType));
    EXPECT_EQ(SharedType->size(), 1U);
    EXPECT_EQ(SharedType->alignment(), 1U);
  }
} // namespace ink::ir::test
