#include "analyzer_internal.h"

#include "ink/parser/parser.h"
#include "ink/semantic/module_import.h"

namespace ink::semantic
{
  ir::Module *Analyzer::analyzeModules(SemanticContext &Context, std::span<const ModuleInput> Inputs, std::string_view EntryModuleName)
  {
    if (Inputs.empty())
    {
      return nullptr;
    }
    for (const ModuleInput &Input : Inputs)
    {
      if (!Input.Input || !Input.Input->succeeded() || !Input.Input->Unit->root() || !Input.Input->Unit->input().lexedFile().isRegisteredWith(Context.compilationContext().sourceManager()))
      {
        return nullptr;
      }
    }
    ModuleGraph Graph;
    std::vector<std::unique_ptr<ModuleAnalysis>> Modules;
    bool Succeeded = true;
    for (const ModuleInput &Input : Inputs)
    {
      const auto &Root = *Input.Input->Unit->root();
      const auto &Tokens = Input.Input->Unit->input();
      AnalysisState Diagnostics(Context, Context.scopeStore().rootScope(), Tokens);
      std::vector<parser::NameToken> NameParts;
      std::size_t Begin = 0;
      while (Begin <= Input.Name.size())
      {
        const std::size_t Separator = Input.Name.find('.', Begin);
        const std::size_t End = Separator == std::string_view::npos ? Input.Name.size() : Separator;
        NameParts.push_back({parser::InvalidTokenId, Input.Name.substr(Begin, End - Begin), {}});
        if (Separator == std::string_view::npos)
        {
          break;
        }
        Begin = Separator + 1;
      }
      if (resolveImportModuleName({}, NameParts) != Input.Name || Input.Name.empty())
      {
        Diagnostics.report<core::DiagnosticKind::SemanticInvalidModulePath>(Root.getSourceRange(), Input.Name);
        Succeeded = false;
        continue;
      }
      if (Graph.Modules.contains(std::string(Input.Name)))
      {
        Diagnostics.report<core::DiagnosticKind::SemanticDuplicateModule>(Root.getSourceRange(), Input.Name);
        Succeeded = false;
        continue;
      }
      auto Module = std::make_unique<ModuleAnalysis>();
      Module->Source = &Input;
      Module->Owner = Diagnostics.Builder.createModule(Context.namePool().intern(Input.Name));
      if (!Module->Owner || !Diagnostics.Builder.createModuleDecl(*Module->Owner, Root))
      {
        Diagnostics.report<core::DiagnosticKind::SemanticConstructionFailed>(Root.getSourceRange());
        return nullptr;
      }
      Scope *ScopeValue = Diagnostics.Resolver.enterScope(*Module->Owner);
      if (!ScopeValue)
      {
        Diagnostics.report<core::DiagnosticKind::SemanticConstructionFailed>(Root.getSourceRange());
        return nullptr;
      }
      Module->State = std::make_unique<AnalysisState>(Context, *ScopeValue, Tokens);
      Module->ModuleScope = ScopeValue;
      Module->State->CurrentModule = Module->Owner;
      Module->State->Modules = &Graph;
      Module->State->Frame = Context.comptimeState().Engine.createFrame(execution::ExecutionFrameKind::Module);
      Module->ModuleFrame = Module->State->Frame;
      if (!Module->State->Frame || !Module->State->Builder.setInsertPoint(Module->Owner->entryBlock()))
      {
        reportExecution(*Module->State, Context.comptimeState().Engine.lastStatus(), Root);
        return nullptr;
      }
      Graph.Modules.emplace(std::string(Input.Name), Module.get());
      Modules.push_back(std::move(Module));
    }
    const auto Entry = Graph.Modules.find(std::string(EntryModuleName));
    if (Entry == Graph.Modules.end())
    {
      const auto &Input = *Inputs.front().Input;
      Context.compilationContext().diagnosticEngine().report<core::DiagnosticKind::SemanticModuleNotFound>(Input.Unit->input().lexedFile().sourceId(), Input.Unit->root()->getSourceRange(), EntryModuleName);
      return nullptr;
    }
    if (!Succeeded)
    {
      return nullptr;
    }
    // Publish nominal identities before field types and function signatures are resolved.
    for (unsigned Phase = 0; Phase != 3; ++Phase)
    {
      for (const auto &Module : Modules)
      {
        for (const parser::Stmt *Statement : Module->Source->Input->Unit->root()->statements())
        {
          if (!parser::DeclStmt::classof(Statement))
          {
            continue;
          }
          const auto *Declaration = static_cast<const parser::DeclStmt *>(Statement)->declaration();
          if (!parser::ClassDecl::classof(Declaration))
          {
            continue;
          }
          const auto &Class = static_cast<const parser::ClassDecl &>(*Declaration);
          if (!(Phase == 0 ? registerClass(*Module->State, Class) : Phase == 1 ? defineClass(*Module->State, Class) : declareClassMembers(*Module->State, Class)))
          {
            Succeeded = false;
          }
        }
      }
      if (!Succeeded)
      {
        return nullptr;
      }
      if (Phase == 0)
      {
        for (const auto &Module : Modules)
        {
          Module->State->ClassImportsOnly = true;
          for (const parser::Stmt *Statement : Module->Source->Input->Unit->root()->statements())
          {
            if ((parser::DirectImportStmt::classof(Statement) || parser::FromImportStmt::classof(Statement)) && !analyzeStmt(*Module->State, *Statement))
            {
              Succeeded = false;
            }
          }
          Module->State->ClassImportsOnly = false;
        }
        if (!Succeeded)
        {
          return nullptr;
        }
      }
    }
    // All modules own their original function identities before any cross-module binding is created.
    std::unordered_map<std::string, const ir::Function *> NativeExports;
    for (const auto &Module : Modules)
    {
      for (const parser::Stmt *Statement : Module->Source->Input->Unit->root()->statements())
      {
        if (!parser::DeclStmt::classof(Statement))
        {
          continue;
        }
        const parser::Decl &Declaration = *static_cast<const parser::DeclStmt *>(Statement)->declaration();
        if (!parser::FunctionDecl::classof(&Declaration))
        {
          continue;
        }
        const auto &Function = static_cast<const parser::FunctionDecl &>(Declaration);
        auto Owner = declareFunction(*Module->State, Function);
        if (!Owner)
        {
          Succeeded = false;
          continue;
        }
        ir::Function *Value = Owner.get();
        if (Value->isNativeExport() && !NativeExports.emplace(std::string(Function.name().Text), Value).second)
        {
          Module->State->report<core::DiagnosticKind::SemanticDuplicateNativeExport>(Function.name().Range, Function.name().Text);
          Succeeded = false;
          continue;
        }
        if (!Module->State->Builder.appendValue(Module->Owner->entryBlock(), std::move(Owner)))
        {
          Module->State->report<core::DiagnosticKind::SemanticConstructionFailed>(Function.getSourceRange());
          Succeeded = false;
          continue;
        }
        Module->Functions.emplace(&Function, Value);
        Graph.Bodies.emplace(Value, ModuleGraph::FunctionBody{Module.get(), &Function});
      }
    }
    if (!Succeeded)
    {
      return nullptr;
    }
    for (const auto &Module : Modules)
    {
      for (const auto &[Declaration, Function] : Module->Functions)
      {
        if (!Function->isNativeImport())
        {
          continue;
        }
        const auto Export = NativeExports.find(std::string(Declaration->name().Text));
        if (Export == NativeExports.end())
        {
          continue;
        }
        if (&Function->functionType() != &Export->second->functionType() || Function->callingConvention() != Export->second->callingConvention())
        {
          Module->State->report<core::DiagnosticKind::SemanticNativeImportSignatureMismatch>(Declaration->getSourceRange(), Declaration->name().Text);
          Succeeded = false;
          continue;
        }
        Graph.Dependencies[Function].insert(Export->second);
      }
      for (const parser::Stmt *Statement : Module->Source->Input->Unit->root()->statements())
      {
        if ((parser::DirectImportStmt::classof(Statement) || parser::FromImportStmt::classof(Statement)) && !analyzeStmt(*Module->State, *Statement))
        {
          Succeeded = false;
        }
      }
    }
    if (!Succeeded)
    {
      return nullptr;
    }
    for (const auto &Module : Modules)
    {
      for (const parser::Stmt *Statement : Module->Source->Input->Unit->root()->statements())
      {
        if (parser::DirectImportStmt::classof(Statement) || parser::FromImportStmt::classof(Statement))
        {
          continue;
        }
        if (parser::DeclStmt::classof(Statement))
        {
          const auto *Declaration = static_cast<const parser::DeclStmt *>(Statement)->declaration();
          if (parser::FunctionDecl::classof(Declaration))
          {
            const auto &Function = *static_cast<const parser::FunctionDecl *>(Declaration);
            if (!ensureModuleFunctionBody(*Module->State, *Module->Functions.find(&Function)->second, Function))
            {
              return nullptr;
            }
            continue;
          }
        }
        if (Module->ProcessedStatements.insert(Statement).second && !analyzeModuleStatement(*Module, *Statement))
        {
          return nullptr;
        }
      }
    }
    for (const auto &Module : Modules)
    {
      if (!validateRuntimeClassTypes(*Module->State, *Module->Owner, *Module->Source->Input->Unit->root()))
      {
        return nullptr;
      }
    }
    return Succeeded ? Entry->second->Owner : nullptr;
  }
} // namespace ink::semantic
