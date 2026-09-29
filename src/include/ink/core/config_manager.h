#ifndef INK_CORE_CONFIG_MANAGER_H
#define INK_CORE_CONFIG_MANAGER_H

#include <cstddef>
#include <string>

namespace ink::core
{
  enum class ConfigKind : std::size_t
  {
#define INK_CONFIG(Kind, EnvironmentName, DefaultValue) Kind,
#include "ink/core/config.def"
#undef INK_CONFIG
    Count,
  };

  class ConfigManager
  {
    public:
      // Read a config.def entry selected at compile time. Environment overrides,
      // including empty values, take priority. Values are copied and each call
      // observes the current environment instead of a cached value.
      // Unregistered enum keys report an ICE and panic.
      template <ConfigKind Kind>
      static std::string get()
      {
        return read(Kind);
      }

      // Read an unsigned decimal size. Malformed or overflowing overrides use the
      // definition's default; invalid size defaults report an ICE and panic.
      template <ConfigKind Kind>
      static std::size_t getSize()
      {
        return readSize(Kind);
      }

    private:
      static std::string read(ConfigKind Kind);
      static std::size_t readSize(ConfigKind Kind);
  };
} // namespace ink::core

#endif
