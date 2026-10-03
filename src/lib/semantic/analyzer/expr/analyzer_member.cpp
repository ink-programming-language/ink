#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/parser/parser.h"

namespace ink::semantic
{
  bool Analyzer::resolveMemberFunctions(AnalysisState &State, const parser::MemberExpr &Node, std::vector<const ir::Value *> &Functions, std::size_t Depth)
  {
    if (Node.access() != tokenizer::TokenKind::Dot)
    {
      State.report<core::DiagnosticKind::SemanticModuleMemberExpected>(Node.getSourceRange());
      return false;
    }
    const ExpressionResult Object = analyzeExpr(State, *Node.object(), Depth + 1);
    if (!Object)
    {
      return false;
    }
    if (!Object.ValueObject || !ir::Module::classof(Object.ValueObject))
    {
      State.report<core::DiagnosticKind::SemanticModuleMemberExpected>(Node.getSourceRange());
      return false;
    }
    auto &Module = *const_cast<ir::Module *>(static_cast<const ir::Module *>(Object.ValueObject));
    const auto Member = Node.member();
    const std::string_view ModuleName = State.Context.namePool().text(Module.name());
    const auto *Binding = State.Resolver.lookupMember(Module, State.Context.namePool().find(Member.Text));
    bool Private = false;
    if (Binding)
    {
      for (const ir::Value *Target : Binding->targets())
      {
        if (!ir::Function::classof(Target) || Target->outer() != &Module.entryBlock())
        {
          continue;
        }
        const auto &Function = static_cast<const ir::Function &>(*Target);
        if (State.CurrentModule != &Module && Function.visibility() != ir::VisibilityKind::Public)
        {
          Private = true;
          continue;
        }
        Functions.push_back(Target);
        if (State.CurrentModule && State.CurrentModule != &Module)
        {
          State.Context.recordModuleImport(*State.CurrentModule, Function);
        }
      }
    }
    if (!Functions.empty())
    {
      return true;
    }
    if (Private)
    {
      State.report<core::DiagnosticKind::SemanticPrivateImport>(Member.Range, Member.Text, ModuleName);
    }
    else if (Binding || (State.Modules && State.Modules->Modules.contains(std::string(ModuleName)) && moduleHasOtherDeclaration(*State.Modules->Modules.find(std::string(ModuleName))->second->Source->Input->Unit->root(), Member.Text)))
    {
      State.report<core::DiagnosticKind::SemanticUnsupportedImport>(Member.Range);
    }
    else
    {
      State.report<core::DiagnosticKind::SemanticImportNotFound>(Member.Range, ModuleName, Member.Text);
    }
    return false;
  }

  Analyzer::ExpressionResult Analyzer::analyzeMemberExpr(AnalysisState &State, const parser::MemberExpr &Node, std::size_t Depth)
  {
    std::vector<const ir::Value *> Functions;
    if (!resolveMemberFunctions(State, Node, Functions, Depth))
    {
      return {};
    }
    if (Functions.size() != 1)
    {
      State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
      return {};
    }
    const auto *Function = Functions.front();
    if (State.Modules && State.CurrentFunction)
    {
      State.Modules->Dependencies[State.CurrentFunction].insert(static_cast<const ir::Function *>(Function));
    }
    const auto &Definitions = State.Context.comptimeState().Functions;
    if (const auto Found = Definitions.find(Function); !State.Evaluating && !State.ComptimeFunction && Found != Definitions.end() && Found->second.Comptime)
    {
      State.report<core::DiagnosticKind::SemanticComptimeFunctionAtRuntime>(Node.getSourceRange());
      return {};
    }
    return {Function};
  }
} // namespace ink::semantic
