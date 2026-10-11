#pragma once

namespace ink::frontend::parser
{
  class ASTNodeBase;
  class Expr;
  class Stmt;
  class Decl;
  class SimpleItem;
  class BindingPattern;
  class MatchPattern;

#define AST_NODE(Name, Base, Category, Id) class Name;
#include "ink/frontend/parser/ASTNodes.def"
#undef AST_NODE

  class Parameter;
  class FunctionTypeParameter;
  class Argument;
  class PathSegment;
  class Attribute;
  class BaseSpec;
  class MatchArm;
  class SwitchClause;
  class ImportEntry;
} // namespace ink::frontend::parser
