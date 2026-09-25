#include "ink/semantic/model/name_pool.h"

namespace ink::semantic
{
  std::size_t NamePool::StringHash::operator()(std::string_view Text) const noexcept
  {
    return std::hash<std::string_view>{}(Text);
  }

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
    const auto Entry = Names.try_emplace(std::string(Text), Result).first;
    Spellings.emplace_back(Entry->first);
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
    return contains(Value) ? Spellings[Value.index()] : std::string_view{};
  }

  bool NamePool::contains(Name Value) const noexcept
  {
    return Value.valid() && Value.index() < Spellings.size();
  }

  std::size_t NamePool::size() const noexcept
  {
    return Spellings.size();
  }
} // namespace ink::semantic
