#include "ink/ir/type/interface_type.h"
#include "type_layout.h"

namespace ink::ir
{
  std::unique_ptr<InterfaceType> InterfaceType::create(std::uint64_t Size, std::uint64_t Alignment)
  {
    if (!detail::validLayout(Size, Alignment))
    {
      return nullptr;
    }
    return std::unique_ptr<InterfaceType>(new InterfaceType(Size, Alignment));
  }

  InterfaceType::InterfaceType(std::uint64_t Size, std::uint64_t Alignment) noexcept
      : Type(ValueKind::Interface),
        Size(Size),
        Alignment(Alignment)
  {
  }

  std::uint64_t InterfaceType::size() const noexcept
  {
    return Size;
  }

  std::uint64_t InterfaceType::alignment() const noexcept
  {
    return Alignment;
  }

  bool InterfaceType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Interface;
  }
} // namespace ink::ir
