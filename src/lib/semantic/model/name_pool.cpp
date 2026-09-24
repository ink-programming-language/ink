#include "ink/semantic/model/name_pool.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace ink::semantic
{
  namespace
  {
    struct StringHash
    {
        using is_transparent = void;

        std::size_t operator()(std::string_view Text) const noexcept
        {
          return std::hash<std::string_view>{}(Text);
        }
    };
  } // namespace

  class NamePool::Impl
  {
    public:
      // Map keys own the only spelling copy; node references survive rehashing.
      std::unordered_map<std::string, Name, StringHash, std::equal_to<>> Names;
      std::vector<std::string_view> Spellings;
  };

  NamePool::NamePool()
      : Storage(std::make_unique<Impl>())
  {
  }

  NamePool::~NamePool() = default;

  Name NamePool::intern(std::string_view Text)
  {
    if (Text.empty())
    {
      return {};
    }
    const auto Existing = Storage->Names.find(Text);
    if (Existing != Storage->Names.end())
    {
      return Existing->second;
    }
    if (Storage->Spellings.size() >= Name::InvalidIndex)
    {
      return {};
    }
    const Name Result(static_cast<Name::IndexType>(Storage->Spellings.size()));
    const auto Entry = Storage->Names.try_emplace(std::string(Text), Result).first;
    Storage->Spellings.emplace_back(Entry->first);
    return Result;
  }

  Name NamePool::find(std::string_view Text) const noexcept
  {
    if (Text.empty())
    {
      return {};
    }
    const auto Entry = Storage->Names.find(Text);
    return Entry == Storage->Names.end() ? Name{} : Entry->second;
  }

  std::string_view NamePool::text(Name Value) const noexcept
  {
    return contains(Value) ? Storage->Spellings[Value.index()] : std::string_view{};
  }

  bool NamePool::contains(Name Value) const noexcept
  {
    return Value.valid() && Value.index() < Storage->Spellings.size();
  }

  std::size_t NamePool::size() const noexcept
  {
    return Storage->Spellings.size();
  }
} // namespace ink::semantic
