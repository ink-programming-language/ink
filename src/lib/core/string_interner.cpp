#include "ink/core/string_interner.h"

#include <stdexcept>

namespace ink::core
{
  InternedStringId StringInterner::intern(std::string_view Value)
  {
    const auto Existing = StringIds.find(Value);
    if (Existing != StringIds.end())
    {
      return Existing->second;
    }
    if (Strings.size() >= InternedStringId::InvalidValue)
    {
      throw std::length_error("StringInterner cannot represent another interned string ID");
    }
    const InternedStringId Id = InternedStringId::fromValue(static_cast<InternedStringId::ValueType>(Strings.size()));
    Strings.emplace_back(Value);
    try
    {
      StringIds.emplace(Strings.back(), Id);
    }
    catch (...)
    {
      Strings.pop_back();
      throw;
    }
    return Id;
  }

  bool StringInterner::contains(InternedStringId Id) const noexcept
  {
    return Id.isValid() && Id.value() < Strings.size();
  }

  std::string_view StringInterner::string(InternedStringId Id) const
  {
    if (!contains(Id))
    {
      throw std::out_of_range("InternedStringId does not identify a string in this StringInterner");
    }
    return Strings[Id.value()];
  }

  std::size_t StringInterner::size() const noexcept
  {
    return Strings.size();
  }

  bool StringInterner::empty() const noexcept
  {
    return Strings.empty();
  }
} // namespace ink::core
