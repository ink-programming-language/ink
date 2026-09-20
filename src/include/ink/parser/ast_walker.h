#pragma once
#include "ink/parser/ast.h"
#include <functional>
#include <string>
namespace ink::parser
{
  void forEachChild(const ASTNodeBase *Node, const std::function<void(const ASTNodeBase *)> &Visit);
  enum class WalkAction
  {
    Continue,
    SkipChildren,
    Stop
  };
  class ASTWalker
  {
    public:
      using Enter = std::function<WalkAction(const ASTNodeBase *)>;
      using Leave = std::function<void(const ASTNodeBase *)>;
      bool walk(const ASTNodeBase *Root, const Enter &OnEnter, const Leave &OnLeave = {}) const;
  };
  bool verifyAST(const ASTNodeBase *Root, std::size_t SourceSize, std::string *Reason = nullptr);
  std::string dumpAST(const ASTNodeBase *Root);
  class ParsedUnit;
  std::string dumpAST(const ParsedUnit &Unit);
} // namespace ink::parser
