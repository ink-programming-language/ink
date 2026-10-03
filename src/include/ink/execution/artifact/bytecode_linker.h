#ifndef INK_EXECUTION_ARTIFACT_BYTECODE_LINKER_H
#define INK_EXECUTION_ARTIFACT_BYTECODE_LINKER_H

#include "ink/execution/artifact/bytecode_artifact.h"

#include <span>

namespace ink::execution
{
  // Entry is optional for a library image. An executable entry must identify a public definition.
  BytecodeArtifactResult linkBytecodeArtifacts(std::span<const BytecodeArtifact *const> Objects, const BytecodeSymbolIdentity *Entry = nullptr, BytecodeLimits Limits = {});
} // namespace ink::execution

#endif
