#include "ink/abi/name_mangling.h"

#include <utility>

namespace ink::abi
{
  const char *mangleErrorKindName(MangleErrorKind Kind) noexcept
  {
    switch (Kind)
    {
      case MangleErrorKind::None:
        return "none";
      case MangleErrorKind::MissingPackagePath:
        return "missing-package-path";
      case MangleErrorKind::EmptyNameComponent:
        return "empty-name-component";
      case MangleErrorKind::InvalidUtf8:
        return "invalid-utf8";
      case MangleErrorKind::NonNormalizedName:
        return "non-normalized-name";
      case MangleErrorKind::UnicodeNormalizationFailed:
        return "unicode-normalization-failed";
      case MangleErrorKind::UnsupportedValueArgumentType:
        return "unsupported-value-argument-type";
      case MangleErrorKind::InvalidCanonicalBits:
        return "invalid-canonical-bits";
      case MangleErrorKind::NestingLimitExceeded:
        return "nesting-limit-exceeded";
      case MangleErrorKind::InvalidFunctionKind:
        return "invalid-function-kind";
      case MangleErrorKind::InvalidGlobalMutability:
        return "invalid-global-mutability";
    }
    return "unknown";
  }

  MangleResult::MangleResult(std::optional<std::string> Name, MangleErrorKind Error) noexcept
      : Name(std::move(Name)),
        Error(Error)
  {
  }

  bool MangleResult::succeeded() const noexcept
  {
    return Name.has_value();
  }

  const std::optional<std::string> &MangleResult::mangledName() const noexcept
  {
    return Name;
  }

  MangleErrorKind MangleResult::error() const noexcept
  {
    return Error;
  }

  MangleResult MangleResult::success(std::string Name)
  {
    return MangleResult(std::move(Name), MangleErrorKind::None);
  }

  MangleResult MangleResult::failure(MangleErrorKind Error) noexcept
  {
    return MangleResult(std::nullopt, Error);
  }

  const char *demangleErrorKindName(DemangleErrorKind Kind) noexcept
  {
    switch (Kind)
    {
      case DemangleErrorKind::None:
        return "none";
      case DemangleErrorKind::InvalidPrefix:
        return "invalid-prefix";
      case DemangleErrorKind::UnsupportedVersion:
        return "unsupported-version";
      case DemangleErrorKind::UnexpectedEnd:
        return "unexpected-end";
      case DemangleErrorKind::UnexpectedCharacter:
        return "unexpected-character";
      case DemangleErrorKind::NonCanonicalNumber:
        return "non-canonical-number";
      case DemangleErrorKind::NumberOverflow:
        return "number-overflow";
      case DemangleErrorKind::InvalidNameLength:
        return "invalid-name-length";
      case DemangleErrorKind::InvalidHexadecimal:
        return "invalid-hexadecimal";
      case DemangleErrorKind::InvalidUtf8:
        return "invalid-utf8";
      case DemangleErrorKind::NonNormalizedName:
        return "non-normalized-name";
      case DemangleErrorKind::UnicodeNormalizationFailed:
        return "unicode-normalization-failed";
      case DemangleErrorKind::MissingPackagePath:
        return "missing-package-path";
      case DemangleErrorKind::EmptyNameComponent:
        return "empty-name-component";
      case DemangleErrorKind::InvalidType:
        return "invalid-type";
      case DemangleErrorKind::UnsupportedValueArgumentType:
        return "unsupported-value-argument-type";
      case DemangleErrorKind::InvalidCanonicalBits:
        return "invalid-canonical-bits";
      case DemangleErrorKind::NestingLimitExceeded:
        return "nesting-limit-exceeded";
      case DemangleErrorKind::TrailingCharacters:
        return "trailing-characters";
    }
    return "unknown";
  }

  DemangleResult::DemangleResult(std::optional<LinkageIdentity> Identity, DemangleErrorKind Error, std::size_t ErrorOffset) noexcept
      : Identity(std::move(Identity)),
        Error(Error),
        ErrorOffset(ErrorOffset)
  {
  }

  bool DemangleResult::succeeded() const noexcept
  {
    return Identity.has_value();
  }

  const std::optional<LinkageIdentity> &DemangleResult::identity() const noexcept
  {
    return Identity;
  }

  DemangleErrorKind DemangleResult::error() const noexcept
  {
    return Error;
  }

  std::size_t DemangleResult::errorOffset() const noexcept
  {
    return ErrorOffset;
  }

  DemangleResult DemangleResult::success(LinkageIdentity Identity)
  {
    return DemangleResult(std::move(Identity), DemangleErrorKind::None, 0);
  }

  DemangleResult DemangleResult::failure(DemangleErrorKind Error, std::size_t ErrorOffset) noexcept
  {
    return DemangleResult(std::nullopt, Error, ErrorOffset);
  }
} // namespace ink::abi
