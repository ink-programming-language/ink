#ifndef INK_CORE_SOURCE_MANAGER_H
#define INK_CORE_SOURCE_MANAGER_H

#include "ink/core/source_file_id.h"

#include <cstddef>
#include <deque>
#include <string>

namespace ink::core
{
  struct SourceFile
  {
    SourceFileId Id;
    std::string Path;
    std::string Contents;
  };

  class SourceManager
  {
  public:
    SourceManager() = default;
    SourceManager(const SourceManager &) = delete;
    SourceManager &operator=(const SourceManager &) = delete;
    SourceManager(SourceManager &&) = delete;
    SourceManager &operator=(SourceManager &&) = delete;

    SourceFileId addSourceFile(std::string Path, std::string Contents);
    bool contains(SourceFileId Id) const noexcept;
    const SourceFile &sourceFile(SourceFileId Id) const;
    std::size_t size() const noexcept;
    bool empty() const noexcept;

  private:
    std::deque<SourceFile> SourceFiles;
  };
} // namespace ink::core

#endif
