#pragma once

#include "ink/frontend/parser/ast/ast_node_base.h"
#include <cassert>

namespace ink::frontend::parser
{
  template <typename T>
  bool isa(const ASTNodeBase *Node)
  {
    return T::classof(Node);
  }
  template <typename T>
  T *dyn_cast(ASTNodeBase *Node)
  {
    return isa<T>(Node) ? static_cast<T *>(Node) : nullptr;
  }
  template <typename T>
  const T *dyn_cast(const ASTNodeBase *Node)
  {
    return isa<T>(Node) ? static_cast<const T *>(Node) : nullptr;
  }
  template <typename T>
  T *cast(ASTNodeBase *Node)
  {
    assert(isa<T>(Node));
    return static_cast<T *>(Node);
  }
  template <typename T>
  const T *cast(const ASTNodeBase *Node)
  {
    assert(isa<T>(Node));
    return static_cast<const T *>(Node);
  }
} // namespace ink::frontend::parser
