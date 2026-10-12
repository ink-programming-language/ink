#pragma once
#include "ink/core/diagnostic/diagnostic_engine.h"
namespace ink::frontend::parser
{
  class ParserDiagnosticEmitter
  {
    public:
      ParserDiagnosticEmitter(core::DiagnosticEngine &Engine, core::SourceId Source, std::size_t Limit)
          : Engine(Engine),
            Source(Source),
            Limit(Limit)
      {
      }
      void report(core::Diagnostic Diagnostic);
      void begin();
      void commit();
      void rollback();
      std::size_t errorCount() const noexcept
      {
        return Count;
      }

    private:
      struct Checkpoint
      {
          std::size_t Pending;
          std::size_t Count;
      };
      core::DiagnosticEngine &Engine;
      core::SourceId Source;
      std::size_t Limit;
      std::size_t Count = 0;
      std::vector<core::Diagnostic> Pending;
      std::vector<Checkpoint> Transactions;
  };
} // namespace ink::frontend::parser
