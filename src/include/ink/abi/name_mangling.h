#ifndef INK_ABI_NAME_MANGLING_H
#define INK_ABI_NAME_MANGLING_H

#include "ink/abi/linkage_identity.h"

#include <cstddef>
#include <initializer_list>
#include <optional>
#include <span>
#include <string_view>

namespace ink::abi
{
  inline constexpr std::string_view SymbolPrefix = "_INK2";

  struct ManglingLimits
  {
      std::size_t MaxBytes = 1024 * 1024;
      std::size_t MaxDepth = 128;
      std::size_t MaxRecords = 65536;
  };

  struct MangleResult
  {
      std::string Name;
      std::string Error;

      explicit operator bool() const noexcept
      {
        return Error.empty() && !Name.empty();
      }
  };

  struct DemangleResult
  {
      std::optional<Record> Identity;
      std::string Error;
      std::size_t ErrorOffset = 0;

      explicit operator bool() const noexcept
      {
        return Identity.has_value();
      }
  };

  std::string encodeRecord(const Record &Value);
  Record record(char Tag, std::span<const Record> Children);
  Record record(char Tag, std::initializer_list<Record> Children);
  Record nameRecord(std::string_view Name);
  std::optional<std::string> decodeName(const Record &Name);
  std::optional<std::vector<Record>> childRecords(const Record &Value, std::size_t PrefixBytes = 0);
  Record packageRecord(const PackageIdentity &Package);
  Record moduleRecord(const ModuleIdentity &Module);
  std::vector<std::string> modulePath(std::string_view DottedName);
  std::optional<ModuleIdentity> moduleIdentity(const Record &Reflection);
  // Validation rejects noncanonical spellings, invalid schemas and unsupported types.
  MangleResult mangle(const Record &Identity, ManglingLimits Limits = {});
  DemangleResult demangle(std::string_view Symbol, ManglingLimits Limits = {});
} // namespace ink::abi

#endif
