#include "ink/execution/runtime/runtime_type.h"

#include <utility>

namespace ink::execution
{
  const StorageLayout *RuntimeTypeTable::get(RuntimeTypeId Type) const noexcept
  {
    return Type < Layouts.size() ? &Layouts[Type] : nullptr;
  }

  std::size_t RuntimeTypeTable::size() const noexcept
  {
    return Layouts.size();
  }

  RuntimeTypeId RuntimeTypeTable::append(StorageLayout Layout)
  {
    if (Layouts.size() >= InvalidRuntimeType)
    {
      return InvalidRuntimeType;
    }
    Layout.Domain = Domain;
    Layout.Type = static_cast<RuntimeTypeId>(Layouts.size());
    Layouts.push_back(std::move(Layout));
    return Layouts.back().Type;
  }
} // namespace ink::execution
