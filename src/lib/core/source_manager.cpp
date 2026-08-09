#include "ink/core/source_manager.h"

#include <stdexcept>
#include <utility>

namespace ink::core
{
  SourceFileId SourceManager::addSourceFile(std::string Path, std::string Contents)
  {
    if (SourceFiles.size() >= SourceFileId::InvalidValue)
    {
      throw std::length_error("SourceManager cannot represent another source file ID");
    }
    const SourceFileId Id = SourceFileId::fromValue(static_cast<SourceFileId::ValueType>(SourceFiles.size()));
    SourceFiles.push_back({Id, std::move(Path), std::move(Contents)});
    return Id;
  }

  bool SourceManager::contains(SourceFileId Id) const noexcept
  {
    return Id.isValid() && Id.value() < SourceFiles.size();
  }

  const SourceFile &SourceManager::sourceFile(SourceFileId Id) const
  {
    if (!contains(Id))
    {
      throw std::out_of_range("SourceFileId does not identify a source file in this SourceManager");
    }
    return SourceFiles[Id.value()];
  }

  std::size_t SourceManager::size() const noexcept
  {
    return SourceFiles.size();
  }

  bool SourceManager::empty() const noexcept
  {
    return SourceFiles.empty();
  }
} // namespace ink::core
