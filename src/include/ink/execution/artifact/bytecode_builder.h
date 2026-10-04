#ifndef INK_EXECUTION_ARTIFACT_BYTECODE_BUILDER_H
#define INK_EXECUTION_ARTIFACT_BYTECODE_BUILDER_H

#include "ink/execution/artifact/bytecode_artifact.h"
#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/abi/linkage_identity.h"

#include <span>
#include <optional>
#include <string_view>

namespace ink::execution
{
  // Frontend metadata supplies module-qualified names, visibility and closed generic arguments.
  // Signature is derived from the IR function; a supplied signature must match it exactly.
  struct BytecodeFunctionInput
  {
      const ir::Function *Function = nullptr;
      BytecodeSymbolIdentity Identity;
      BytecodeSymbolKind Kind = BytecodeSymbolKind::Definition;
      BytecodeVisibility Visibility = BytecodeVisibility::Public;
      // For a generic whose parameters depend on its arguments, supply the open H pattern.
      // Empty means its parameters are the fixed closed types of Function.
      std::optional<abi::Record> OverloadPattern;
  };

  // Compile every supplied definition, including functions not yet executed. All referenced
  // functions must be supplied explicitly so missing metadata cannot silently export a symbol.
  // Imports may refer to analyzed definitions in another source module; their bodies are not emitted.
  BytecodeArtifactResult buildBytecodeObject(std::string_view ModuleName, SemanticValueBridge &Bridge, std::span<const BytecodeFunctionInput> Functions, BytecodeLimits Limits = {});
} // namespace ink::execution

#endif
