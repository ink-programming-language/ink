#ifndef INK_IR_FUNCTION_H
#define INK_IR_FUNCTION_H

#include "ink/ir/basic_block.h"
#include "ink/ir/type.h"

#include <cstdint>
#include <string>
#include <vector>

namespace ink::ir
{
  enum class FunctionKind : std::uint8_t
  {
    Definition,
    External,
  };

  enum class CallingConvention : std::uint8_t
  {
    Ink,
    C,
  };

  struct Function
  {
    explicit Function(const Type &ResultType) noexcept
        : ResultType(&ResultType)
    {
    }

    std::string Name;
    FunctionKind Kind = FunctionKind::Definition;
    CallingConvention Convention = CallingConvention::Ink;
    const Type *ResultType;
    std::vector<const Type *> ParameterTypes;
    bool HasSideEffects = false;
    std::vector<BasicBlock> Blocks;
  };
} // namespace ink::ir

#endif
