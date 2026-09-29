#pragma once
#include "ink/core/config_manager.h"
#include "ink/parser/token_cursor.h"
#include <functional>
namespace ink::parser
{
  enum class ParseStatus
  {
    Completed,
    LimitExceeded,
    Cancelled
  };
  struct ParseLimits
  {
      // Snapshot configured defaults at construction; callers can override individual fields.
      std::size_t MaxNestingDepth = core::ConfigManager::getSize<core::ConfigKind::ParserMaxNestingDepth>();
      std::size_t MaxDiagnostics = core::ConfigManager::getSize<core::ConfigKind::ParserMaxDiagnostics>();
      std::size_t MaxWork = core::ConfigManager::getSize<core::ConfigKind::ParserMaxWork>();
      // Exhausting the allocation budget reports an ICE and panics without returning a partial tree.
      std::size_t MaxAllocationBytes = core::ConfigManager::getSize<core::ConfigKind::ParserMaxAllocationBytes>();
      std::function<bool()> IsCancelled;
  };
  enum class ExpectStatus
  {
    Matched,
    Inserted,
    Recovered
  };
  struct ExpectResult
  {
      TokenKind Expected;
      std::optional<TokenId> Actual;
      SourceRange Range;
      ExpectStatus Status;
  };
  struct RecoveryEntry
  {
      const ASTNodeBase *Node;
      ExpectResult Token;
      std::optional<SourceRange> Skipped;
  };
  struct SyntaxRecoveryInfo
  {
      std::vector<RecoveryEntry> Entries;
  };
  class ParsedUnit
  {
    public:
      explicit ParsedUnit(std::shared_ptr<const TokenBuffer> Input)
          : Input(std::move(Input)),
            Context(std::make_unique<ASTContext>())
      {
      }
      const TokenBuffer &input() const noexcept
      {
        return *Input;
      }
      ASTContext &context() noexcept
      {
        return *Context;
      }
      ModuleAST *root() noexcept
      {
        return Root;
      }
      const ModuleAST *root() const noexcept
      {
        return Root;
      }
      const SyntaxRecoveryInfo &recoveryInfo() const noexcept
      {
        return Recovery;
      }

    private:
      // Destruction order intentionally releases nodes before their input.
      std::shared_ptr<const TokenBuffer> Input;
      std::unique_ptr<ASTContext> Context;
      ModuleAST *Root = nullptr;
      SyntaxRecoveryInfo Recovery;
      friend class Parser;
      friend class ASTReader;
  };
  struct ParseResult
  {
      std::unique_ptr<ParsedUnit> Unit;
      ParseStatus Status = ParseStatus::Completed;
      bool HasSyntaxErrors = false;
      bool succeeded() const noexcept
      {
        return Unit && Status == ParseStatus::Completed && !HasSyntaxErrors && Unit->input().lexedFile().succeeded();
      }
  };
  ParseResult parse(core::FrontendContext &Context, tokenizer::TokenizedBuffer Input, ParseLimits Limits = {});
} // namespace ink::parser
