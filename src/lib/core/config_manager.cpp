#include "ink/core/config_manager.h"
#include "ink/core/diagnostic.h"

#include <charconv>
#include <cstdlib>
#include <optional>
#include <string_view>
#include <system_error>
#include <unordered_map>

namespace ink::core
{
  namespace
  {
    struct ConfigEntry
    {
        const char *EnvironmentName;
        std::string_view DefaultValue;
    };

    const ConfigEntry &findConfig(ConfigKind Kind)
    {
      static const std::unordered_map<ConfigKind, ConfigEntry> Configs = {
#define INK_CONFIG(Kind, EnvironmentName, DefaultValue) {ConfigKind::Kind, {EnvironmentName, DefaultValue}},
#include "ink/core/config.def"
#undef INK_CONFIG
      };
      const auto Entry = Configs.find(Kind);
      if (Entry == Configs.end())
      {
        DiagnosticEngine{}.report<DiagnosticKind::ConfigNotFound>({}, static_cast<std::size_t>(Kind));
      }
      return Entry->second;
    }

    std::string_view readValue(const ConfigEntry &Entry)
    {
      const char *EnvironmentValue = std::getenv(Entry.EnvironmentName);
      return EnvironmentValue != nullptr ? std::string_view(EnvironmentValue) : Entry.DefaultValue;
    }

    std::optional<std::size_t> parseSize(std::string_view Text)
    {
      if (Text.empty())
      {
        return std::nullopt;
      }
      std::size_t Value = 0;
      const auto Result = std::from_chars(Text.data(), Text.data() + Text.size(), Value);
      if (Result.ec != std::errc{} || Result.ptr != Text.data() + Text.size())
      {
        return std::nullopt;
      }
      return Value;
    }
  } // namespace

  std::string ConfigManager::read(ConfigKind Kind)
  {
    return std::string(readValue(findConfig(Kind)));
  }

  std::size_t ConfigManager::readSize(ConfigKind Kind)
  {
    const ConfigEntry &Entry = findConfig(Kind);
    if (const auto Value = parseSize(readValue(Entry)))
    {
      return *Value;
    }
    const auto DefaultValue = parseSize(Entry.DefaultValue);
    if (!DefaultValue)
    {
      DiagnosticEngine{}.report<DiagnosticKind::ConfigInvalidSizeDefault>({}, Entry.EnvironmentName, Entry.DefaultValue);
    }
    return *DefaultValue;
  }
} // namespace ink::core
