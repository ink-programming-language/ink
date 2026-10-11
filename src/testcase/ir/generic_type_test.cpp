#include "ink/ir/type/generic_class_type.h"
#include "ink/ir/type/generic_function_type.h"
#include "ink/ir/type/generic_type.h"
#include "ink/ir/type/generic_var_type.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::ir::test
{
  static_assert(std::is_abstract_v<GenericType>);
  static_assert(std::is_base_of_v<Type, GenericType>);
  static_assert(std::is_base_of_v<GenericType, GenericClassType>);
  static_assert(std::is_base_of_v<GenericType, GenericFunctionType>);
  static_assert(std::is_base_of_v<GenericType, GenericVarType>);
  static_assert(std::is_final_v<GenericClassType>);
  static_assert(std::is_final_v<GenericFunctionType>);
  static_assert(std::is_final_v<GenericVarType>);
  static_assert(std::has_virtual_destructor_v<GenericType>);

  // Generic type classifiers reject a missing value for every retained declaration kind.
  TEST(IRGenericTypeTest, ClassifiersRejectNull)
  {
    EXPECT_FALSE(GenericType::classof(nullptr));
    EXPECT_FALSE(GenericClassType::classof(nullptr));
    EXPECT_FALSE(GenericFunctionType::classof(nullptr));
    EXPECT_FALSE(GenericVarType::classof(nullptr));
  }
} // namespace ink::ir::test
