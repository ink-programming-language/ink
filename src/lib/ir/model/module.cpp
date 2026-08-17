#include "ink/ir/model/module.h"

#include <cstddef>

namespace ink::ir
{
  std::optional<FunctionId> Module::findFunction(const ink::ir::Name &SymbolName) const noexcept
  {
    for (std::size_t Index = 0; Index < Functions.size(); ++Index)
    {
      if (Functions[Index].Name == SymbolName)
      {
        return FunctionId{Index};
      }
    }
    return std::nullopt;
  }

  std::optional<GlobalId> Module::findGlobal(const ink::ir::Name &SymbolName) const noexcept
  {
    for (std::size_t Index = 0; Index < Globals.size(); ++Index)
    {
      if (Globals[Index].Name == SymbolName)
      {
        return GlobalId{Index};
      }
    }
    return std::nullopt;
  }
} // namespace ink::ir
