#include "ink/ir/type/struct_type.h"
#include "type_layout.h"

namespace ink::ir
{
  std::unique_ptr<StructType> StructType::create(std::uint64_t Size, std::uint64_t Alignment)
  {
    if (!detail::validLayout(Size, Alignment))
    {
      return nullptr;
    }
    return std::unique_ptr<StructType>(new StructType(Size, Alignment));
  }

  StructType::StructType(std::uint64_t Size, std::uint64_t Alignment) noexcept
      : Type(ValueKind::Struct),
        Size(Size),
        Alignment(Alignment)
  {
  }

  std::uint64_t StructType::size() const noexcept
  {
    return Size;
  }

  std::uint64_t StructType::alignment() const noexcept
  {
    return Alignment;
  }

  bool StructType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::Struct;
  }
} // namespace ink::ir
