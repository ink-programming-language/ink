#ifndef INK_CLI_IO_H
#define INK_CLI_IO_H

#include <filesystem>
#include <string_view>

namespace ink::cli
{
  std::filesystem::path pathFromUtf8(std::string_view Path);
  bool useBinaryStandardInput() noexcept;
} // namespace ink::cli

#endif
