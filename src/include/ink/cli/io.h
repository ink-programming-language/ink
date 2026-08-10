#ifndef INK_CLI_IO_H
#define INK_CLI_IO_H

#include <filesystem>
#include <iosfwd>
#include <string_view>

namespace ink::cli
{
  std::filesystem::path pathFromUtf8(std::string_view Path);
  bool useBinaryStandardInput() noexcept;
  bool writeOutput(std::ostream &Output, std::string_view Message);
} // namespace ink::cli

#endif
