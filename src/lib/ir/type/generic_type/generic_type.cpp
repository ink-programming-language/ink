#include "ink/ir/type/generic_type/generic_type.h"

#include "ink/core/name/name_pool.h"

#include <utility>

namespace ink::ir
{
  GenericType::GenericType(ValueKind Kind, const MetaType &Meta, core::Name Name, std::vector<const Type *> GenericParameters) noexcept
      : Type(Kind, Meta),
        Name(Name),
        GenericParameters(std::move(GenericParameters))
  {
  }

  GenericType::~GenericType() noexcept = default;

  core::Name GenericType::name() const noexcept
  {
    return Name;
  }

  const std::vector<const Type *> &GenericType::genericParameters() const noexcept
  {
    return GenericParameters;
  }

  std::size_t GenericType::genericParameterCount() const noexcept
  {
    return GenericParameters.size();
  }

  bool GenericType::hasLayout() const noexcept
  {
    return false;
  }

  std::uint64_t GenericType::size() const noexcept
  {
    return 0;
  }

  std::uint64_t GenericType::alignment() const noexcept
  {
    return 1;
  }

  std::string GenericType::toString() const
  {
    const auto NameText = core::NamePool::instance().text(Name);
    return NameText.empty() ? Value::toString() : std::string(NameText);
  }

  bool GenericType::classof(const Value *Value) noexcept
  {
    if (!Value)
    {
      return false;
    }
    switch (Value->kind())
    {
      case ValueKind::GenericClass:
      case ValueKind::GenericFunction:
      case ValueKind::GenericVar:
        return true;
      default:
        return false;
    }
  }
} // namespace ink::ir
