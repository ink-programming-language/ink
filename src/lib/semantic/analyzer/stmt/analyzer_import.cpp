#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/parser/parser.h"
#include "ink/semantic/module_import.h"

#include <algorithm>

namespace ink::semantic
{
  namespace
  {
    bool ownsFunction(const ir::Module &Module, const ir::Function &Function)
    {
      return Function.outer() == &Module.entryBlock();
    }
  } // namespace

  bool moduleHasOtherDeclaration(const parser::ModuleAST &Root, std::string_view Name)
  {
    for (const parser::Stmt *Statement : Root.statements())
    {
      if (!parser::DeclStmt::classof(Statement))
      {
        continue;
      }
      const auto &Declaration = *static_cast<const parser::DeclStmt *>(Statement)->declaration();
      if (parser::VarDecl::classof(&Declaration))
      {
        const auto *Binding = static_cast<const parser::VarDecl &>(Declaration).binding();
        if (parser::NameBindingPattern::classof(Binding) && static_cast<const parser::NameBindingPattern *>(Binding)->name().Text == Name)
        {
          return true;
        }
      }
      if ((parser::ClassDecl::classof(&Declaration) && static_cast<const parser::ClassDecl &>(Declaration).name().Text == Name) || (parser::EnumDecl::classof(&Declaration) && static_cast<const parser::EnumDecl &>(Declaration).name().Text == Name) || (parser::InterfaceDecl::classof(&Declaration) && static_cast<const parser::InterfaceDecl &>(Declaration).name().Text == Name))
      {
        return true;
      }
    }
    return false;
  }

  bool Analyzer::analyzeDirectImportStmt(AnalysisState &State, const parser::DirectImportStmt &Node)
  {
    if (State.CurrentFunction || State.BlockDepth != 0 || Node.isComptime() || State.Evaluating)
    {
      State.report<core::DiagnosticKind::SemanticImportRequiresTopLevel>(Node.getSourceRange());
      return false;
    }
    bool Succeeded = true;
    for (const parser::ImportEntry &Import : Node.imports())
    {
      const auto Path = Import.path();
      const std::string Name = resolveImportModuleName({}, {Path.data(), Path.size()});
      if (Name.empty())
      {
        State.report<core::DiagnosticKind::SemanticInvalidModulePath>(Import.range(), Name);
        Succeeded = false;
        continue;
      }
      if (!State.Modules)
      {
        State.report<core::DiagnosticKind::SemanticModuleNotFound>(Import.range(), Name);
        Succeeded = false;
        continue;
      }
      const auto Found = State.Modules->Modules.find(Name);
      if (Found == State.Modules->Modules.end())
      {
        State.report<core::DiagnosticKind::SemanticModuleNotFound>(Import.range(), Name);
        Succeeded = false;
        continue;
      }
      const auto Alias = Import.alias().value_or(Path.back());
      const ir::Name Local = State.Context.namePool().intern(Alias.Text);
      const auto Bound = State.Resolver.bind(Local, *Found->second->Owner);
      if (Bound != NameResolver::BindResult::Inserted && Bound != NameResolver::BindResult::AlreadyBound)
      {
        State.report<core::DiagnosticKind::SemanticDuplicateName>(Alias.Range, Alias.Text);
        Succeeded = false;
      }
    }
    return Succeeded;
  }

  bool Analyzer::analyzeFromImportStmt(AnalysisState &State, const parser::FromImportStmt &Node)
  {
    if (State.CurrentFunction || State.BlockDepth != 0 || Node.isComptime() || State.Evaluating)
    {
      State.report<core::DiagnosticKind::SemanticImportRequiresTopLevel>(Node.getSourceRange());
      return false;
    }
    const auto Path = Node.path();
    const std::string_view Importer = State.CurrentModule ? State.Context.namePool().text(State.CurrentModule->name()) : std::string_view{};
    const std::string Name = resolveImportModuleName(Importer, {Path.data(), Path.size()}, Node.relativeLevel());
    if (Name.empty())
    {
      State.report<core::DiagnosticKind::SemanticInvalidModulePath>(Node.getSourceRange(), Importer);
      return false;
    }
    if (!State.Modules)
    {
      State.report<core::DiagnosticKind::SemanticModuleNotFound>(Node.getSourceRange(), Name);
      return false;
    }
    const auto Found = State.Modules->Modules.find(Name);
    if (Found == State.Modules->Modules.end())
    {
      State.report<core::DiagnosticKind::SemanticModuleNotFound>(Node.getSourceRange(), Name);
      return false;
    }
    ir::Module &Module = *Found->second->Owner;
    bool Succeeded = true;
    for (const parser::ImportEntry &Import : Node.imports())
    {
      if (Import.path().size() != 1)
      {
        State.report<core::DiagnosticKind::SemanticUnsupportedImport>(Import.range());
        Succeeded = false;
        continue;
      }
      const auto ImportedName = Import.path().front();
      const auto Alias = Import.alias().value_or(ImportedName);
      const auto *Binding = State.Resolver.lookupMember(Module, State.Context.namePool().find(ImportedName.Text));
      if (Binding && Binding->targets().size() == 1 && ir::ClassType::classof(Binding->targets().front()))
      {
        auto *Class = static_cast<ir::ClassType *>(Binding->targets().front());
        const auto &Definition = State.Context.classState().Definitions.at(Class);
        if (Definition.Module != &Module || (State.CurrentModule != &Module && Definition.Declaration->visibility() == parser::DeclarationVisibility::Private))
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Import.range(), "class is private or not defined in the imported module");
          Succeeded = false;
          continue;
        }
        const auto Bound = State.Resolver.bind(State.Context.namePool().intern(Alias.Text), *Class);
        if (Bound != NameResolver::BindResult::Inserted && Bound != NameResolver::BindResult::AlreadyBound)
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Alias.Range, Alias.Text);
          Succeeded = false;
        }
        continue;
      }
      if (State.ClassImportsOnly)
      {
        continue;
      }
      std::vector<ir::Function *> Functions;
      bool Private = false;
      if (Binding)
      {
        for (ir::Value *Target : Binding->targets())
        {
          if (ir::Function::classof(Target) && ownsFunction(Module, static_cast<const ir::Function &>(*Target)))
          {
            auto &Function = static_cast<ir::Function &>(*Target);
            if (State.CurrentModule == &Module || Function.visibility() == ir::VisibilityKind::Public)
            {
              Functions.push_back(&Function);
            }
            else
            {
              Private = true;
            }
          }
        }
      }
      if (Functions.empty())
      {
        if (Private)
        {
          State.report<core::DiagnosticKind::SemanticPrivateImport>(Import.range(), ImportedName.Text, Name);
        }
        else if (Binding || moduleHasOtherDeclaration(*Found->second->Source->Input->Unit->root(), ImportedName.Text))
        {
          State.report<core::DiagnosticKind::SemanticUnsupportedImport>(Import.range());
        }
        else
        {
          State.report<core::DiagnosticKind::SemanticImportNotFound>(Import.range(), Name, ImportedName.Text);
        }
        Succeeded = false;
        continue;
      }
      const ir::Name Local = State.Context.namePool().intern(Alias.Text);
      bool Conflict = State.Resolver.lookupLocal<ir::Decl *>(Local) != nullptr;
      if (const auto *Existing = State.Resolver.lookupLocal(Local))
      {
        for (const ir::Value *Target : Existing->targets())
        {
          for (const ir::Function *Function : Functions)
          {
            if (Target == Function)
            {
              continue;
            }
            if (!ir::Function::classof(Target))
            {
              Conflict = true;
              continue;
            }
            const auto Parameters = static_cast<const ir::Function *>(Target)->functionType().parameterTypes();
            const auto ImportedParameters = Function->functionType().parameterTypes();
            Conflict = Conflict || std::equal(Parameters.begin(), Parameters.end(), ImportedParameters.begin(), ImportedParameters.end());
          }
        }
      }
      if (Conflict)
      {
        State.report<core::DiagnosticKind::SemanticDuplicateName>(Alias.Range, Alias.Text);
        Succeeded = false;
        continue;
      }
      for (ir::Function *Function : Functions)
      {
        const auto Bound = State.Resolver.bind(Local, *Function);
        if (Bound != NameResolver::BindResult::Inserted && Bound != NameResolver::BindResult::AlreadyBound)
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Alias.Range, Alias.Text);
          Succeeded = false;
          break;
        }
        if (State.CurrentModule != &Module)
        {
          State.Context.recordModuleImport(*State.CurrentModule, *Function);
        }
      }
    }
    return Succeeded;
  }
} // namespace ink::semantic
