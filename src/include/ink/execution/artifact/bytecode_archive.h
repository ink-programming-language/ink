#ifndef INK_EXECUTION_ARTIFACT_BYTECODE_ARCHIVE_H
#define INK_EXECUTION_ARTIFACT_BYTECODE_ARCHIVE_H

#include "ink/execution/artifact/bytecode_artifact.h"

#include <filesystem>
#include <string_view>

namespace ink::execution
{
  struct BytecodeSerializeResult
  {
      BytecodeStatus Status = BytecodeStatus::InvalidInput;
      std::string Message;
      std::string Bytes;

      explicit operator bool() const noexcept
      {
        return Status == BytecodeStatus::Success;
      }
  };

  BytecodeSerializeResult serializeBytecodeArtifact(const BytecodeArtifact &Artifact, BytecodeLimits Limits = {});
  BytecodeArtifactResult deserializeBytecodeArtifact(std::string_view Bytes, BytecodeLimits Limits = {});
  BytecodeResult writeBytecodeFile(const std::filesystem::path &Path, const BytecodeArtifact &Artifact, BytecodeLimits Limits = {});
  BytecodeArtifactResult readBytecodeFile(const std::filesystem::path &Path, BytecodeLimits Limits = {});
} // namespace ink::execution

#endif
