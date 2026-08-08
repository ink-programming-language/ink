#include "ink/parser/cst.h"

#include <stdexcept>

namespace ink::parser
{
  const char *cstKindName(CstKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_CST_KIND(Name) \
  case CstKind::Name:      \
    return #Name;
#include "ink/parser/cst_kind.def"
#undef INK_CST_KIND
    }
    return "Unknown";
  }

  const CstNode &CstTree::node(CstNodeId Id) const
  {
    return Nodes.at(Id);
  }
} // namespace ink::parser
