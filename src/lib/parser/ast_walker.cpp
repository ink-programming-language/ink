#include "ink/parser/ast_walker.h"
#include <algorithm>
#include <sstream>
namespace ink::parser
{
  bool ASTWalker::walk(const ASTNodeBase *Root, const Enter &OnEnter, const Leave &OnLeave) const
  {
    if (!Root)
    {
      return true;
    }
    struct Frame
    {
        const ASTNodeBase *Node;
        bool Leaving;
    };
    std::vector<Frame> Pending{{Root, false}};
    while (!Pending.empty())
    {
      const Frame Current = Pending.back();
      Pending.pop_back();
      if (Current.Leaving)
      {
        if (OnLeave)
        {
          OnLeave(Current.Node);
        }
        continue;
      }
      const WalkAction Action = OnEnter(Current.Node);
      if (Action == WalkAction::Stop)
      {
        return false;
      }
      Pending.push_back({Current.Node, true});
      if (Action == WalkAction::SkipChildren)
      {
        continue;
      }
      const std::size_t Begin = Pending.size();
      forEachChild(Current.Node, [&](const ASTNodeBase *Child)
                   {
                     Pending.push_back({Child, false});
                   });
      std::reverse(Pending.begin() + Begin, Pending.end());
    }
    return true;
  }
} // namespace ink::parser
