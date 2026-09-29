#pragma once
#include "ink/core/config_manager.h"
#include "ink/parser/parser.h"
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::parser
{
  enum class ASTArchiveStatus
  {
    Success,
    InvalidInput,
    InvalidArchive,
    UnsupportedVersion,
    LimitExceeded
  };

  struct ASTArchiveLimits
  {
      // Snapshot configured defaults at construction; callers can override individual fields.
      std::size_t MaxArchiveBytes = core::ConfigManager::getSize<core::ConfigKind::ASTArchiveMaxBytes>();
      std::size_t MaxSourceBytes = core::ConfigManager::getSize<core::ConfigKind::ASTArchiveMaxSourceBytes>();
      std::size_t MaxNodes = core::ConfigManager::getSize<core::ConfigKind::ASTArchiveMaxNodes>();
      std::size_t MaxTokens = core::ConfigManager::getSize<core::ConfigKind::ASTArchiveMaxTokens>();
      std::size_t MaxArrayElements = core::ConfigManager::getSize<core::ConfigKind::ASTArchiveMaxArrayElements>();
      // Cumulative decoded storage budget, including temporary copies; not a process RSS limit.
      std::size_t MaxAllocationBytes = core::ConfigManager::getSize<core::ConfigKind::ASTArchiveMaxAllocationBytes>();
  };

  struct ASTSerializeResult
  {
      std::string Bytes;
      ASTArchiveStatus Status = ASTArchiveStatus::InvalidInput;
      std::string Message;
      bool succeeded() const noexcept
      {
        return Status == ASTArchiveStatus::Success;
      }
  };

  struct ASTDeserializeResult
  {
      ParseResult Parsed;
      ASTArchiveStatus Status = ASTArchiveStatus::InvalidArchive;
      std::string Message;
      // Successful restoration can contain a recovered or interrupted parse.
      bool succeeded() const noexcept
      {
        return Status == ASTArchiveStatus::Success && Parsed.Unit != nullptr;
      }
  };

  inline constexpr std::uint32_t ASTArchiveVersion = 2;

  // Deterministic binary snapshots of syntax, tokens, source and recovery metadata.
  // No semantic state or declaration index is included. Bytes may contain NUL.
  // The first failure reports an ICE through Context.diagnosticEngine() and panics.
  // Failed operations do not return a result or partial bytes/AST to the caller.
  ASTSerializeResult serializeAST(core::FrontendContext &Context, const ParseResult &Parsed, ASTArchiveLimits Limits = {});
  ASTDeserializeResult deserializeAST(core::FrontendContext &Context, std::string_view Bytes, ASTArchiveLimits Limits = {});
} // namespace ink::parser
