#ifndef INK_CORE_SOURCE_MANAGER_H
#define INK_CORE_SOURCE_MANAGER_H

#include "ink/core/source_id.h"

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace ink::core
{
  class SourceBuffer final
  {
    public:
      SourceId id() const noexcept
      {
        return Id;
      }

      const std::string &name() const noexcept
      {
        return Name;
      }

      const std::string &text() const noexcept
      {
        return Text;
      }

      const std::vector<std::size_t> &lineStarts() const noexcept
      {
        return LineStarts;
      }

      std::size_t lineNumber(std::size_t ByteOffset) const noexcept;

    private:
      SourceBuffer(SourceId Id, std::string Name, std::string Text);

      SourceId Id;
      std::string Name;
      std::string Text;
      std::vector<std::size_t> LineStarts;

      friend class SourceManager;
  };

  class SourceManager final
  {
    public:
      SourceManager() = default;
      SourceManager(const SourceManager &) = delete;
      SourceManager &operator=(const SourceManager &) = delete;
      SourceManager(SourceManager &&) = delete;
      SourceManager &operator=(SourceManager &&) = delete;

      SourceId addSource(std::string Name, std::string Text);
      std::shared_ptr<const SourceBuffer> findSource(SourceId Id) const noexcept;
      std::size_t sourceCount() const noexcept;

    private:
      mutable std::mutex Mutex;
      std::unordered_map<std::uint64_t, std::shared_ptr<const SourceBuffer>> Sources;
  };
} // namespace ink::core

#endif
