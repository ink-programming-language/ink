#include "ink/ir/instruction/instruction.h"
#include "ink/ir/type/type.h"

#include <type_traits>

namespace ink::ir::test
{
  static_assert(std::is_abstract_v<Instruction>);
  static_assert(std::is_base_of_v<Value, Instruction>);
  static_assert(!std::is_base_of_v<Type, Instruction>);
  static_assert(std::has_virtual_destructor_v<Instruction>);
} // namespace ink::ir::test
