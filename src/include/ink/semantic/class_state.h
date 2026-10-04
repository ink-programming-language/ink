#ifndef INK_SEMANTIC_CLASS_STATE_H
#define INK_SEMANTIC_CLASS_STATE_H

#include "ink/ir/type/class_type.h"
#include "ink/parser/ast.h"

#include <unordered_map>
#include <vector>

namespace ink::ir
{
  class Function;
  class Module;
}

namespace ink::parser
{
  class TokenBuffer;
}

namespace ink::semantic
{
  class Scope;

  // Definition metadata belongs to the frontend; emitted IR retains resolved types and functions only.
  struct ClassDefinition
  {
      const parser::ClassDecl *Declaration = nullptr;
      const parser::TokenBuffer *Input = nullptr;
      ir::Module *Module = nullptr;
      Scope *Members = nullptr;
      Scope *Parent = nullptr;
      const ir::ClassType *EnclosingClass = nullptr;
      std::string Identity;
      std::vector<const parser::FieldDecl *> Fields;
      std::vector<ir::Function *> Defaults;
      std::unordered_map<const parser::FunctionDecl *, ir::Function *> Methods;
      bool MembersDeclared = false;
      bool BodyAnalyzing = false;
      bool BodyComplete = false;
  };

  struct ClassState
  {
      std::unordered_map<const ir::ClassType *, ClassDefinition> Definitions;
      std::unordered_map<const ir::Function *, const ir::ClassType *> MethodOwners;
  };
} // namespace ink::semantic

#endif
