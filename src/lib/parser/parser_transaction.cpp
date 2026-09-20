#include "parser_internal.h"
namespace ink::parser
{
  Parser::ParseTransaction::ParseTransaction(Parser &Owner)
      : Owner(Owner),
        Position(Owner.Cursor.position()),
        Allocation(Owner.AST.checkpoint()),
        Recovery(Owner.Recovery),
        RecoverySize(Owner.RecoveryInfo.Entries.size()),
        End(Owner.LastEnd),
        Strict(Owner.Strict),
        Failed(Owner.TrialFailed)
  {
    Owner.Diagnostics.begin();
    Owner.Strict = true;
    Owner.TrialFailed = false;
  }
  Parser::ParseTransaction::~ParseTransaction()
  {
    if (!Committed)
    {
      Owner.Cursor.restore(Position);
      Owner.AST.rollback(Allocation);
      Owner.Recovery = std::move(Recovery);
      Owner.RecoveryInfo.Entries.resize(RecoverySize);
      Owner.LastEnd = End;
      Owner.Diagnostics.rollback();
    }
    Owner.Strict = Strict;
    Owner.TrialFailed = Failed;
  }
  void Parser::ParseTransaction::commit()
  {
    assert(!Owner.TrialFailed);
    Committed = true;
    Owner.Strict = Strict;
    Owner.Diagnostics.commit();
  }
} // namespace ink::parser
