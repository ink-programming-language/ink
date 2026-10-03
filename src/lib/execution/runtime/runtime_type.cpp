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
    if (Layout.Kind == RuntimeKind::Array)
    {
      const StorageLayout *Element = get(Layout.ElementType);
      if (!Element || Element->Kind == RuntimeKind::Invalid || Element->Kind == RuntimeKind::Void)
      {
        return InvalidRuntimeType;
      }
      if (Layout.ElementCount > std::numeric_limits<std::size_t>::max() || (Element->Size != 0 && Layout.ElementCount > std::numeric_limits<std::size_t>::max() / Element->Size) || Layout.Size != Element->Size * static_cast<std::size_t>(Layout.ElementCount) || Layout.Alignment != Element->Alignment || Layout.Native != Element->Native)
      {
        return InvalidRuntimeType;
      }
      Layout.ElementLayout = std::make_shared<const StorageLayout>(*Element);
    }
    Layout.Domain = Domain;
    Layout.Type = static_cast<RuntimeTypeId>(Layouts.size());
    Layouts.push_back(std::move(Layout));
    return Layouts.back().Type;
  }
} // namespace ink::execution
