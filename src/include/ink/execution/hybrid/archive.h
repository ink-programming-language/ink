#ifndef INK_EXECUTION_HYBRID_ARCHIVE_H
#define INK_EXECUTION_HYBRID_ARCHIVE_H

#include "ink/execution/artifact/bytecode_archive.h"

namespace ink::execution
{
  enum class HybridArtifactKind
  {
    Base,
    Patch,
  };

  struct HybridArtifact
  {
      HybridArtifactKind Kind = HybridArtifactKind::Base;
      std::string BuildId;
      std::unique_ptr<BytecodeArtifact> Bytecode;
  };

  struct HybridArtifactResult
  {
      std::string Error;
      HybridArtifact Artifact;

      explicit operator bool() const noexcept
      {
        return Error.empty() && Artifact.Bytecode != nullptr;
      }
  };

  std::string hybridBuildId(std::string_view Bytes);
  BytecodeSerializeResult serializeHybridArtifact(HybridArtifactKind Kind, std::string_view BuildId, const BytecodeArtifact &Artifact);
  HybridArtifactResult deserializeHybridArtifact(std::string_view Bytes);
  HybridArtifactResult readHybridFile(const std::filesystem::path &Path);
  BytecodeResult writeHybridFile(const std::filesystem::path &Path, std::string_view Bytes);
  // Checks all shared nominal classes, including layouts reachable only through pointers.
  BytecodeResult compatibleHybridTypes(const RuntimeTypeTable &Base, const RuntimeTypeTable &Patch);
} // namespace ink::execution

#endif
