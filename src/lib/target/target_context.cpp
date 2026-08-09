#include "ink/target/target_context.h"

#include <stdexcept>
#include <utility>

namespace ink::target
{
  namespace
  {
    std::string hostArchitecture()
    {
#if defined(_M_X64) || defined(__x86_64__)
      return "x86_64";
#elif defined(_M_IX86) || defined(__i386__)
      return "i686";
#elif defined(_M_ARM64) || defined(__aarch64__)
      return "aarch64";
#elif defined(_M_ARM) || defined(__arm__)
      return "arm";
#else
      return "unknown";
#endif
    }

    std::string hostTriple()
    {
#if defined(_WIN32)
      return hostArchitecture() + "-pc-windows-msvc";
#elif defined(__APPLE__)
      return hostArchitecture() + "-apple-darwin";
#elif defined(__linux__)
      return hostArchitecture() + "-unknown-linux-gnu";
#else
      return hostArchitecture() + "-unknown-unknown";
#endif
    }

    TargetEndianness hostEndianness() noexcept
    {
      const std::uint16_t Value = 1;
      return *reinterpret_cast<const std::uint8_t *>(&Value) == 1 ? TargetEndianness::Little : TargetEndianness::Big;
    }
  } // namespace

  TargetContext::TargetContext(TargetKey Key) : Key(std::move(Key))
  {
    if (!this->Key.isValid())
    {
      throw std::invalid_argument("target context requires a valid target key");
    }
  }

  TargetContext TargetContext::host()
  {
    return TargetContext({hostTriple(), "generic", "", static_cast<std::uint32_t>(sizeof(void *) * 8), hostEndianness()});
  }

  const TargetKey &TargetContext::key() const noexcept
  {
    return Key;
  }

  std::uint32_t TargetContext::pointerBitWidth() const noexcept
  {
    return Key.PointerBitWidth;
  }

  TargetEndianness TargetContext::endianness() const noexcept
  {
    return Key.Endianness;
  }

  bool TargetContext::supportsIntegerBitWidth(std::uint32_t BitWidth) const noexcept
  {
    return BitWidth == 1 || BitWidth == 8 || BitWidth == 16 || BitWidth == 32 || BitWidth == 64;
  }
} // namespace ink::target
