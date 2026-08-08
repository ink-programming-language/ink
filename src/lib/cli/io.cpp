#include "ink/cli/io.h"

#include <cstdio>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

namespace ink::cli
{
  std::filesystem::path pathFromUtf8(std::string_view Path)
  {
    return std::filesystem::u8path(Path.begin(), Path.end());
  }

  bool useBinaryStandardInput() noexcept
  {
#ifdef _WIN32
    return _setmode(_fileno(stdin), _O_BINARY) != -1;
#else
    return true;
#endif
  }
} // namespace ink::cli
