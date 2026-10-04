#ifndef INK_EXECUTION_ARTIFACT_BYTECODE_ARTIFACT_H
#define INK_EXECUTION_ARTIFACT_BYTECODE_ARTIFACT_H

#include "ink/core/generic_argument_kind.h"

#include "ink/execution/bytecode/execution_image.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ink::execution
{
  enum class BytecodeArtifactKind : std::uint8_t
  {
    Object,
    Executable,
  };

  enum class BytecodeSymbolKind : std::uint8_t
  {
    Definition,
    Import,
    Native,
  };

  enum class BytecodeVisibility : std::uint8_t
  {
    Private,
    Module,
    Public,
  };

  // Canonical type identities and exact constant encodings, never context-local IDs.
  struct BytecodeGenericArgument
  {
      core::GenericArgumentKind Kind = core::GenericArgumentKind::Type;
      std::string Type;
      std::string Value;

      bool operator==(const BytecodeGenericArgument &) const = default;
  };

  struct BytecodeSymbolIdentity
  {
      std::string Module;
      std::string Name;
      std::string Signature;
      std::vector<BytecodeGenericArgument> GenericArguments;
      // Canonical _INK2 link name; the fields above remain diagnostic/ABI metadata.
      std::string LinkName;

      bool operator==(const BytecodeSymbolIdentity &) const = default;
  };

  struct BytecodeSymbol
  {
      FunctionId Function = InvalidFunction;
      BytecodeSymbolIdentity Identity;
      BytecodeSymbolKind Kind = BytecodeSymbolKind::Definition;
      BytecodeVisibility Visibility = BytecodeVisibility::Private;
  };

  // Objects contain unresolved Ink imports. Executables contain only definitions and native imports.
  // Private symbols are local to this artifact; Module symbols are visible within ModuleName.
  struct BytecodeArtifact
  {
      BytecodeArtifactKind Kind = BytecodeArtifactKind::Object;
      std::string ModuleName;
      std::string Target;
      FunctionId Entry = InvalidFunction;
      ExecutionImage Image;
      std::vector<BytecodeSymbol> Symbols;
  };

  enum class BytecodeStatus
  {
    Success,
    InvalidInput,
    InvalidFormat,
    UnsupportedVersion,
    IncompatibleTarget,
    LimitExceeded,
    InvalidImage,
    DuplicateSymbol,
    MissingSymbol,
    InvisibleSymbol,
    SignatureMismatch,
    UnsupportedConstant,
    IoError,
  };

  struct BytecodeLimits
  {
      std::size_t MaxBytes = 64 * 1024 * 1024;
      std::size_t MaxRecords = 1024 * 1024;
      std::size_t MaxStringBytes = 16 * 1024 * 1024;
      std::size_t MaxAllocationBytes = 256 * 1024 * 1024;
      std::size_t MaxTypeDepth = 256;
  };

  struct BytecodeResult
  {
      BytecodeStatus Status = BytecodeStatus::Success;
      std::string Message;

      explicit operator bool() const noexcept
      {
        return Status == BytecodeStatus::Success;
      }
  };

  struct BytecodeArtifactResult
  {
      BytecodeStatus Status = BytecodeStatus::InvalidInput;
      std::string Message;
      std::unique_ptr<BytecodeArtifact> Artifact;

      explicit operator bool() const noexcept
      {
        return Status == BytecodeStatus::Success && Artifact != nullptr;
      }
  };

  std::string nativeBytecodeTarget();
  // Empty indicates an invalid or unsupported type graph; encoding is independent of table IDs.
  std::string bytecodeTypeIdentity(const RuntimeTypeTable &Types, RuntimeTypeId Type, std::size_t MaxDepth = 256);
  std::string bytecodeSymbolKey(const BytecodeSymbolIdentity &Identity);
  BytecodeResult validateBytecodeArtifact(const BytecodeArtifact &Artifact, BytecodeLimits Limits = {});
} // namespace ink::execution

#endif
