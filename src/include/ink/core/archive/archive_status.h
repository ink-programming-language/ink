#ifndef INK_CORE_ARCHIVE_STATUS_H
#define INK_CORE_ARCHIVE_STATUS_H

namespace ink::core
{
  // Common failures for syntax and IR archives; format-specific details accompany the status.
  enum class ArchiveStatus
  {
    Success,
    InvalidInput,
    InvalidArchive,
    UnsupportedVersion,
    LimitExceeded,
  };
} // namespace ink::core

#endif
