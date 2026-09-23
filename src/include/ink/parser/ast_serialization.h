#pragma once
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
      std::size_t MaxArchiveBytes = 256 * 1024 * 1024;
      std::size_t MaxSourceBytes = 64 * 1024 * 1024;
      std::size_t MaxNodes = 1000000;
      std::size_t MaxTokens = 2000000;
      std::size_t MaxArrayElements = 1000000;
      // Cumulative decoded storage budget, including temporary copies; not a process RSS limit.
      std::size_t MaxAllocationBytes = 256 * 1024 * 1024;
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

  inline constexpr std::uint32_t ASTArchiveVersion = 1;

  // Deterministic binary snapshots of syntax, tokens, source and recovery metadata.
  // No semantic state or declaration index is included. Bytes may contain NUL.
  // Each failed operation reports its first failure as ICE through Context.diagnosticEngine().
  // Message contains the formatted diagnostic; callers must not report the same failure again.
  ASTSerializeResult serializeAST(core::FrontendContext &Context, const ParseResult &Parsed, ASTArchiveLimits Limits = {});
  ASTDeserializeResult deserializeAST(core::FrontendContext &Context, std::string_view Bytes, ASTArchiveLimits Limits = {});
} // namespace ink::parser
