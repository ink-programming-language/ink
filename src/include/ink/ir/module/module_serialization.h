#ifndef INK_IR_MODULE_SERIALIZATION_H
#define INK_IR_MODULE_SERIALIZATION_H

#include "ink/core/config_manager.h"
#include "ink/ir/module/module.h"
#include "ink/parser/ast_serialization.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace ink::ir
{
  enum class ModuleArchiveStatus
  {
    Success,
    InvalidInput,
    InvalidArchive,
    UnsupportedVersion,
    LimitExceeded,
  };

  struct ModuleArchiveLimits
  {
      // Snapshot configured defaults at construction; callers can override individual fields.
      std::size_t MaxArchiveBytes = core::ConfigManager::getSize<core::ConfigKind::ModuleArchiveMaxBytes>();
      std::size_t MaxObjects = core::ConfigManager::getSize<core::ConfigKind::ModuleArchiveMaxObjects>();
      std::size_t MaxFields = core::ConfigManager::getSize<core::ConfigKind::ModuleArchiveMaxFields>();
      std::size_t MaxStringBytes = core::ConfigManager::getSize<core::ConfigKind::ModuleArchiveMaxStringBytes>();
      // Conservative cumulative storage budget, not a process RSS limit.
      std::size_t MaxAllocationBytes = core::ConfigManager::getSize<core::ConfigKind::ModuleArchiveMaxAllocationBytes>();
      // Structural ownership and declaration depth; operand dependencies are restored iteratively.
      std::size_t MaxNestingDepth = core::ConfigManager::getSize<core::ConfigKind::ModuleArchiveMaxNestingDepth>();
      parser::ASTArchiveLimits AST;
  };

  struct ModuleSerializeResult
  {
      std::string Bytes;
      ModuleArchiveStatus Status = ModuleArchiveStatus::InvalidInput;
      std::string Message;

      bool succeeded() const noexcept
      {
        return Status == ModuleArchiveStatus::Success;
      }
  };

  struct ModuleDeserializeResult
  {
      Module *ModuleValue = nullptr;
      ModuleArchiveStatus Status = ModuleArchiveStatus::InvalidArchive;
      std::string Message;

      bool succeeded() const noexcept
      {
        return Status == ModuleArchiveStatus::Success && ModuleValue;
      }
  };

  inline constexpr std::uint32_t ModuleTextVersion = 2;
  inline constexpr std::uint32_t ModuleBinaryVersion = 2;

  // Archives IR, declaration trees and complete syntax snapshots, including tokens and recovery metadata.
  // Supply the ParseResults borrowed by declarations; missing AST owners are rejected, never omitted.
  // Previously restored modules automatically reuse archivedASTs(). Semantic bindings are not included.
  // Non-pool operands must belong to this module's ownership tree, including nested modules.
  // Output is deterministic and independent of context pool insertion order and host ABI.
  ModuleSerializeResult serializeModuleText(const Module &ModuleValue, ModuleArchiveLimits Limits = {}, std::span<const parser::ParseResult *const> Sources = {});
  ModuleSerializeResult serializeModuleBinary(const Module &ModuleValue, ModuleArchiveLimits Limits = {}, std::span<const parser::ParseResult *const> Sources = {});

  // Success appends one root to Context; all restored names/types/constants are context-local.
  // Failure returns no module and leaves existing modules intact. Interned pool/source entries may remain.
  // Text uses readable IR assembly; binary uses independent byte-aligned records and raw payloads.
  // Neither API performs I/O.
  ModuleDeserializeResult deserializeModuleText(IRContext &Context, std::string_view Text, ModuleArchiveLimits Limits = {});
  ModuleDeserializeResult deserializeModuleBinary(IRContext &Context, std::string_view Bytes, ModuleArchiveLimits Limits = {});
} // namespace ink::ir

#endif
