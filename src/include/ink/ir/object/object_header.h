#pragma once

#include "ink/core/config/config_manager.h"
#include "ink/core/support/version.h"

#include <array>
#include <cstdint>

namespace ink::ir
{
  inline constexpr std::array<std::uint8_t, 4> ObjectFileMagic = {
      'I',
      'N',
      'K',
      'O',
  };

  enum class ObjectByteOrder : std::uint8_t
  {
    Little,
    Big,
  };

  class ObjectHeader final
  {
    public:
      auto &getMagic(this auto &&Self) noexcept
      {
        return Self.Magic;
      }

      auto &getFormatVersion(this auto &&Self) noexcept
      {
        return Self.FormatVersion;
      }

      auto &getInstructionSetVersion(this auto &&Self) noexcept
      {
        return Self.InstructionSetVersion;
      }

      auto &getAddressBitWidth(this auto &&Self) noexcept
      {
        return Self.AddressBitWidth;
      }

      auto &getByteOrder(this auto &&Self) noexcept
      {
        return Self.ByteOrder;
      }

    private:
      std::array<std::uint8_t, 4> Magic = ObjectFileMagic;
      core::Version FormatVersion = core::ConfigManager::getVersion<core::ConfigKind::ObjectFormatVersion>();
      core::Version InstructionSetVersion = core::ConfigManager::getVersion<core::ConfigKind::ObjectInstructionSetVersion>();
      std::uint16_t AddressBitWidth = 64;
      ObjectByteOrder ByteOrder = ObjectByteOrder::Little;
  };
} // namespace ink::ir
