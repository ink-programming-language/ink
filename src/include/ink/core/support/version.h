#pragma once

#include <charconv>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace ink::core
{
  // Numeric version components displayed as Major.Minor.Patch.
  class Version final
  {
    public:
      constexpr explicit Version(std::uint32_t Major = 0, std::uint32_t Minor = 0, std::uint32_t Patch = 0) noexcept
          : Major(Major),
            Minor(Minor),
            Patch(Patch)
      {
      }

      constexpr auto &getMajor(this auto &&Self) noexcept
      {
        return Self.Major;
      }

      constexpr auto &getMinor(this auto &&Self) noexcept
      {
        return Self.Minor;
      }

      constexpr auto &getPatch(this auto &&Self) noexcept
      {
        return Self.Patch;
      }

      // Each component is an unsigned decimal uint32_t; all three components must be present.
      static std::optional<Version> parse(std::string_view Text) noexcept
      {
        std::uint32_t Components[3] = {};
        for (std::uint32_t Index = 0; Index < 3; ++Index)
        {
          const auto Separator = Text.find('.');
          if ((Index < 2 && Separator == std::string_view::npos) || (Index == 2 && Separator != std::string_view::npos))
          {
            return std::nullopt;
          }
          const auto Component = Index < 2 ? Text.substr(0, Separator) : Text;
          if (Component.empty())
          {
            return std::nullopt;
          }
          const char *End = Component.data() + Component.size();
          const auto Parsed = std::from_chars(Component.data(), End, Components[Index], 10);
          if (Parsed.ec != std::errc{} || Parsed.ptr != End)
          {
            return std::nullopt;
          }
          if (Index < 2)
          {
            Text.remove_prefix(Separator + 1);
          }
        }
        return Version(Components[0], Components[1], Components[2]);
      }

      std::string toString() const
      {
        return std::to_string(Major) + "." + std::to_string(Minor) + "." + std::to_string(Patch);
      }

      constexpr bool operator==(const Version &) const noexcept = default;

    private:
      std::uint32_t Major;
      std::uint32_t Minor;
      std::uint32_t Patch;
  };
} // namespace ink::core
