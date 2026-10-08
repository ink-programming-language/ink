#include "ink/core/name_pool.h"

namespace ink::core
{
  NamePool::NamePool() = default;
  NamePool::~NamePool() = default;

  Name NamePool::intern(std::string_view Text)
  {
    if (Text.empty())
    {
      return {};
    }
    const auto Existing = Names.find(Text);
    if (Existing != Names.end())
    {
      return Existing->second;
    }
    if (Spellings.size() >= Name::InvalidIndex)
    {
      return {};
    }
    const Name Result(static_cast<Name::IndexType>(Spellings.size()));
    Spellings.emplace_back(Text);
    Names.emplace(std::string_view(Spellings.back()), Result);
    return Result;
  }

  Name NamePool::find(std::string_view Text) const noexcept
  {
    if (Text.empty())
    {
      return {};
    }
    const auto Entry = Names.find(Text);
    return Entry == Names.end() ? Name{} : Entry->second;
  }

  std::string_view NamePool::text(Name Value) const noexcept
  {
    return contains(Value) ? std::string_view(Spellings[Value.index()]) : std::string_view{};
  }

  bool NamePool::contains(Name Value) const noexcept
  {
    return Value.valid() && Value.index() < Spellings.size();
  }

  std::size_t NamePool::size() const noexcept
  {
    return Spellings.size();
  }
} // namespace ink::core
