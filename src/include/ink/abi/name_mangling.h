#ifndef INK_ABI_NAME_MANGLING_H
#define INK_ABI_NAME_MANGLING_H

#include "ink/abi/linkage_identity.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

namespace ink::abi
{
  inline constexpr std::size_t NameManglingNestingLimit = 128;

  using LinkageIdentity = std::variant<FunctionIdentity, GlobalIdentity>;

  enum class MangleErrorKind : std::uint8_t
  {
    None,
    MissingPackagePath,
    EmptyNameComponent,
    InvalidUtf8,
    NonNormalizedName,
    UnicodeNormalizationFailed,
    UnsupportedValueArgumentType,
    InvalidCanonicalBits,
    NestingLimitExceeded,
    InvalidFunctionKind,
    InvalidGlobalMutability,
  };

  const char *mangleErrorKindName(MangleErrorKind Kind) noexcept;

  class MangleResult final
  {
    public:
      bool succeeded() const noexcept;
      const std::optional<std::string> &mangledName() const noexcept;
      MangleErrorKind error() const noexcept;

      static MangleResult success(std::string Name);
      static MangleResult failure(MangleErrorKind Error) noexcept;

    private:
      MangleResult(std::optional<std::string> Name, MangleErrorKind Error) noexcept;

      std::optional<std::string> Name;
      MangleErrorKind Error;
  };

  enum class DemangleErrorKind : std::uint8_t
  {
    None,
    InvalidPrefix,
    UnsupportedVersion,
    UnexpectedEnd,
    UnexpectedCharacter,
    NonCanonicalNumber,
    NumberOverflow,
    InvalidNameLength,
    InvalidHexadecimal,
    InvalidUtf8,
    NonNormalizedName,
    UnicodeNormalizationFailed,
    MissingPackagePath,
    EmptyNameComponent,
    InvalidType,
    UnsupportedValueArgumentType,
    InvalidCanonicalBits,
    NestingLimitExceeded,
    TrailingCharacters,
  };

  const char *demangleErrorKindName(DemangleErrorKind Kind) noexcept;

  class DemangleResult final
  {
    public:
      bool succeeded() const noexcept;
      const std::optional<LinkageIdentity> &identity() const noexcept;
      DemangleErrorKind error() const noexcept;
      std::size_t errorOffset() const noexcept;

      static DemangleResult success(LinkageIdentity Identity);
      static DemangleResult failure(DemangleErrorKind Error, std::size_t ErrorOffset) noexcept;

    private:
      DemangleResult(std::optional<LinkageIdentity> Identity, DemangleErrorKind Error, std::size_t ErrorOffset) noexcept;

      std::optional<LinkageIdentity> Identity;
      DemangleErrorKind Error;
      std::size_t ErrorOffset;
  };

  MangleResult mangle(const FunctionIdentity &Identity);
  MangleResult mangle(const GlobalIdentity &Identity);
  MangleResult mangle(const LinkageIdentity &Identity);
  DemangleResult demangle(std::string_view Name);
} // namespace ink::abi

#endif
