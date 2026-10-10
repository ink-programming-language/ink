#include "ink/ir/type/class_type.h"
#include "type_layout.h"

namespace ink::ir
{
  std::unique_ptr<ClassType> ClassType::create(std::uint64_t Size, std::uint64_t Alignment)
  {
    if (!detail::validLayout(Size, Alignment))
    {
      return nullptr;
    }
    return std::unique_ptr<ClassType>(new ClassType(Size, Alignment));
  }

  ClassType::ClassType(std::uint64_t Size, std::uint64_t Alignment) noexcept
      : Type(ValueKind::Class),
        Size(Size),
        Alignment(Alignment)
  {
  }

  std::uint64_t ClassType::size() const noexcept
  {
    return Size;
  }

  std::uint64_t ClassType::alignment() const noexcept
  {
    return Alignment;
  }

  bool ClassType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Class;
  }
} // namespace ink::ir
