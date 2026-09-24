#include "ink/semantic/model/name_pool.h"

#include <llvm/ADT/StringMap.h>

#include <vector>

namespace ink::semantic
{
  class NamePool::Impl
  {
    public:
      // StringMap owns the bytes in separately allocated entries, so rehashing
      // never invalidates the views and no second spelling copy is necessary.
      llvm::StringMap<Name> Names;
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
    const llvm::StringRef Key(Text.data(), Text.size());
    const auto Existing = Storage->Names.find(Key);
    if (Existing != Storage->Names.end())
    {
      return Existing->second;
    }
    if (Storage->Spellings.size() >= Name::InvalidIndex)
    {
      return {};
    }
    const Name Result(static_cast<Name::IndexType>(Storage->Spellings.size()));
    const auto Entry = Storage->Names.try_emplace(Key, Result).first;
    const llvm::StringRef Spelling = Entry->getKey();
    Storage->Spellings.emplace_back(Spelling.data(), Spelling.size());
    return Result;
  }

  Name NamePool::find(std::string_view Text) const noexcept
  {
    if (Text.empty())
    {
      return {};
    }
    const auto Entry = Storage->Names.find(llvm::StringRef(Text.data(), Text.size()));
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
