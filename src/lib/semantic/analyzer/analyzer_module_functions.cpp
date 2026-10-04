#include "analyzer_internal.h"

#include "ink/parser/parser.h"

namespace ink::semantic
{
  bool Analyzer::analyzeModuleStatement(ModuleAnalysis &Module, const parser::Stmt &Statement)
  {
    const parser::Stmt *Saved = Module.ActiveStatement;
    Module.ActiveStatement = &Statement;
    const bool Succeeded = analyzeStmt(*Module.State, Statement);
    Module.ActiveStatement = Saved;
    return Succeeded;
  }

  bool Analyzer::ensureModuleFunctionBody(AnalysisState &State, const ir::Function &FunctionValue, const parser::ASTNodeBase &Use)
  {
    const auto Method = State.Context.classState().MethodOwners.find(&FunctionValue);
    if (Method != State.Context.classState().MethodOwners.end() && !FunctionValue.entryBlock())
    {
      auto &Definition = State.Context.classState().Definitions.at(Method->second);
      if (Definition.BodyAnalyzing)
      {
        State.report<core::DiagnosticKind::SemanticComptimeBodyDependency>(Use.getSourceRange(), State.Context.namePool().text(FunctionValue.name()));
        return false;
      }
      return ensureClassDefinition(State, *Method->second, Use);
    }
    if (!State.Modules)
    {
      return true;
    }
    auto &Graph = *State.Modules;
    const auto Found = Graph.Bodies.find(&FunctionValue);
    if (Found == Graph.Bodies.end())
    {
      return true;
    }
    auto &Body = Found->second;
    if (Body.State == ModuleGraph::BodyState::Complete || Body.State == ModuleGraph::BodyState::Failed)
    {
      return Body.State == ModuleGraph::BodyState::Complete;
    }
    if (Body.State == ModuleGraph::BodyState::Analyzing)
    {
      State.report<core::DiagnosticKind::SemanticComptimeBodyDependency>(Use.getSourceRange(), State.Context.namePool().text(FunctionValue.name()));
      return false;
    }
    if (Graph.LoweringDepth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Use.getSourceRange());
      return false;
    }
    Body.State = ModuleGraph::BodyState::Analyzing;
    ++Graph.LoweringDepth;
    bool Succeeded = true;
    const ir::Function *EarlierActive = nullptr;
    // Earlier module statements establish the lexical environment before a demanded body is checked.
    // Each statement executes once even when a dependency is requested before the ordinary module walk.
    for (const parser::Stmt *Statement : Body.Module->Source->Input->Unit->root()->statements())
    {
      if (parser::DeclStmt::classof(Statement))
      {
        const parser::Decl *Declaration = static_cast<const parser::DeclStmt *>(Statement)->declaration();
        if (Declaration == Body.Declaration)
        {
          break;
        }
        if (parser::FunctionDecl::classof(Declaration))
        {
          const auto *Earlier = static_cast<const parser::FunctionDecl *>(Declaration);
          const auto *Value = Body.Module->Functions.find(Earlier)->second;
          const auto EarlierState = Graph.Bodies.find(Value)->second.State;
          if (Graph.ActiveBodies.contains(Value))
          {
            EarlierActive = Value;
          }
          if (EarlierState != ModuleGraph::BodyState::Analyzing && !ensureModuleFunctionBody(State, *Value, Use))
          {
            Succeeded = false;
          }
          continue;
        }
      }
      if (parser::DirectImportStmt::classof(Statement) || parser::FromImportStmt::classof(Statement))
      {
        continue;
      }
      if (EarlierActive && !Body.Module->ProcessedStatements.contains(Statement))
      {
        State.report<core::DiagnosticKind::SemanticComptimeModuleDependency>(Use.getSourceRange(), State.Context.namePool().text(EarlierActive->name()));
        Succeeded = false;
        break;
      }
      if (Body.Module->ActiveStatement && !Body.Module->ProcessedStatements.contains(Statement))
      {
        State.report<core::DiagnosticKind::SemanticComptimeStatementDependency>(Use.getSourceRange());
        Succeeded = false;
        break;
      }
      if (Body.Module->ProcessedStatements.insert(Statement).second && !analyzeModuleStatement(*Body.Module, *Statement))
      {
        Succeeded = false;
      }
    }
    if (Succeeded)
    {
      AnalysisState DefinitionState(State.Context, *Body.Module->ModuleScope, Body.Module->State->Input);
      DefinitionState.CurrentModule = Body.Module->Owner;
      DefinitionState.Modules = &Graph;
      DefinitionState.Frame = Body.Module->ModuleFrame;
      Succeeded = analyzeFunctionBody(DefinitionState, *Body.Declaration, *Body.Module->Functions.find(Body.Declaration)->second);
    }
    --Graph.LoweringDepth;
    Body.State = Succeeded ? ModuleGraph::BodyState::Complete : ModuleGraph::BodyState::Failed;
    return Succeeded;
  }

  bool Analyzer::prepareComptimeFunctions(AnalysisState &State, const ir::Function &FunctionValue, const parser::ASTNodeBase &Use)
  {
    if (!State.Modules)
    {
      return true;
    }
    // Follow function references as well as direct calls: a local may hold an imported callable.
    // The visited set accepts ordinary mutually recursive IR while rejecting only incomplete bodies.
    std::vector<const ir::Function *> Pending{&FunctionValue};
    std::unordered_set<const ir::Function *> Visited;
    while (!Pending.empty())
    {
      const ir::Function *Current = Pending.back();
      Pending.pop_back();
      if (!Visited.insert(Current).second)
      {
        continue;
      }
      if (State.Modules->ActiveBodies.contains(Current))
      {
        State.report<core::DiagnosticKind::SemanticComptimeBodyDependency>(Use.getSourceRange(), State.Context.namePool().text(Current->name()));
        return false;
      }
      if (!ensureModuleFunctionBody(State, *Current, Use))
      {
        return false;
      }
      const auto Dependencies = State.Modules->Dependencies.find(Current);
      if (Dependencies != State.Modules->Dependencies.end())
      {
        Pending.insert(Pending.end(), Dependencies->second.begin(), Dependencies->second.end());
      }
    }
    return true;
  }
} // namespace ink::semantic
