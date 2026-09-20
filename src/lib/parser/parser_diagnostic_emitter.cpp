#include "ink/parser/parser_diagnostic_emitter.h"
#include <cassert>
namespace ink::parser
{
  void ParserDiagnosticEmitter::report(core::Diagnostic Diagnostic)
  {
    Diagnostic.Source = Source;
    if (Count++ >= Limit)
    {
      return;
    }
    if (Transactions.empty())
    {
      Engine.report(Diagnostic);
    }
    else
    {
      Pending.push_back(std::move(Diagnostic));
    }
  }
  void ParserDiagnosticEmitter::begin()
  {
    Transactions.push_back({Pending.size(), Count});
  }
  void ParserDiagnosticEmitter::commit()
  {
    assert(!Transactions.empty());
    Transactions.pop_back();
    if (Transactions.empty())
    {
      for (const auto &Entry : Pending)
      {
        Engine.report(Entry);
      }
      Pending.clear();
    }
  }
  void ParserDiagnosticEmitter::rollback()
  {
    assert(!Transactions.empty());
    const Checkpoint Saved = Transactions.back();
    Transactions.pop_back();
    Pending.resize(Saved.Pending);
    Count = Saved.Count;
  }
} // namespace ink::parser
