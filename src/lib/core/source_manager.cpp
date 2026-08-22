#include "ink/core/source_manager.h"

#include <algorithm>
#include <atomic>
#include <utility>

namespace ink::core
{
  namespace
  {
    std::atomic<std::uint64_t> NextSourceId{1};
  } // namespace

  SourceBuffer::SourceBuffer(SourceId Id, std::string Name, std::string Text)
      : Id(Id),
        Name(std::move(Name)),
        Text(std::move(Text))
  {
    LineStarts.push_back(0);
    for (std::size_t Index = 0; Index < this->Text.size(); ++Index)
    {
      if (this->Text[Index] == '\n')
      {
        LineStarts.push_back(Index + 1);
      }
    }
  }

  std::size_t SourceBuffer::lineNumber(std::size_t ByteOffset) const noexcept
  {
    const std::size_t ClampedOffset = std::min(ByteOffset, Text.size());
    return static_cast<std::size_t>(std::upper_bound(LineStarts.begin(), LineStarts.end(), ClampedOffset) - LineStarts.begin());
  }

  SourceId SourceManager::addSource(std::string Name, std::string Text)
  {
    const SourceId Id(NextSourceId.fetch_add(1, std::memory_order_relaxed));
    std::shared_ptr<const SourceBuffer> Source(new SourceBuffer(Id, std::move(Name), std::move(Text)));
    const std::lock_guard<std::mutex> Lock(Mutex);
    Sources.emplace(Id.value(), std::move(Source));
    return Id;
  }

  std::shared_ptr<const SourceBuffer> SourceManager::findSource(SourceId Id) const noexcept
  {
    const std::lock_guard<std::mutex> Lock(Mutex);
    if (!Id.valid())
    {
      return {};
    }
    const auto Iterator = Sources.find(Id.value());
    return Iterator == Sources.end() ? std::shared_ptr<const SourceBuffer>{} : Iterator->second;
  }

  std::size_t SourceManager::sourceCount() const noexcept
  {
    const std::lock_guard<std::mutex> Lock(Mutex);
    return Sources.size();
  }
} // namespace ink::core
