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
      // Cumulative decoded storage charged against MaxAllocationBytes.
      std::size_t AllocationBytes = 0;
  };

  // V5 stores native import/export direction separately from the function's ABI literal.
  inline constexpr std::uint32_t ASTArchiveVersion = 5;
  inline constexpr std::uint32_t ASTTextArchiveVersion = 4;

  // Deterministic binary snapshots of syntax, tokens, source and recovery metadata.
  // No semantic state or declaration index is included. Bytes may contain NUL.
  // The first failure reports an ICE through Context.diagnosticEngine() and panics.
  // Failed operations do not return a result or partial bytes/AST to the caller.
  ASTSerializeResult serializeAST(core::FrontendContext &Context, const ParseResult &Parsed, ASTArchiveLimits Limits = {});
  ASTDeserializeResult deserializeAST(core::FrontendContext &Context, std::string_view Bytes, ASTArchiveLimits Limits = {});

  // Composable archive operations: return the same errors without reporting an ICE or panicking.
  // Intended for containers such as module archives that handle invalid input through explicit status.
  ASTSerializeResult trySerializeAST(const ParseResult &Parsed, ASTArchiveLimits Limits = {});
  ASTDeserializeResult tryDeserializeAST(core::FrontendContext &Context, std::string_view Bytes, ASTArchiveLimits Limits = {});
  // Readable, independently versioned syntax snapshots used by textual module archives.
  ASTSerializeResult trySerializeASTText(const ParseResult &Parsed, ASTArchiveLimits Limits = {});
  ASTDeserializeResult tryDeserializeASTText(core::FrontendContext &Context, std::string_view Text, ASTArchiveLimits Limits = {});
} // namespace ink::parser
