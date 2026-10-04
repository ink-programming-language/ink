#include "analyzer_internal.h"

#include "ink/parser/parser.h"
#include "ink/semantic/context.h"
#include "ink/ir/ir_builder.h"
#include "ink/semantic/name_resolve/name_resolver.h"

namespace ink::semantic
{
  using namespace ink::ir;

  Module *Analyzer::analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName)
  {
    if (!Input.succeeded() || !Input.Unit->root())
    {
      return nullptr;
    }
    const auto &Lexed = Input.Unit->input().lexedFile();
    if (!Lexed.isRegisteredWith(Context.compilationContext().sourceManager()))
    {
      return nullptr;
    }
    const parser::ModuleAST &Root = *Input.Unit->root();
    IRBuilder Builder(Context.irContext());
    Module *Result = Builder.createModule(Context.namePool().intern(ModuleName));
    if (!Result || !Builder.createModuleDecl(*Result, Root))
    {
      Context.compilationContext().diagnosticEngine().report<core::DiagnosticKind::SemanticConstructionFailed>(Lexed.sourceId(), Root.getSourceRange());
      return nullptr;
    }

    // Each call starts at the shared root and enters a distinct module member scope.
    AnalysisState State(Context, Context.scopeStore().rootScope(), Input.Unit->input());
    State.CurrentModule = Result;
    State.Frame = Context.comptimeState().Engine.createFrame(execution::ExecutionFrameKind::Module);
    if (!State.Frame)
    {
      reportExecution(State, Context.comptimeState().Engine.lastStatus(), Root);
      return nullptr;
    }
    NameResolver::ScopeGuard ModuleScope(State.Resolver, *Result);
    if (!ModuleScope.scope() || !State.Builder.setInsertPoint(Result->entryBlock()))
    {
      return nullptr;
    }

    // Declarations are currently analyzed in source order. Failed functions are
    // discarded with their bindings, while later siblings still receive diagnostics.
    bool Succeeded = true;
    for (const parser::Stmt *Stmt : Root.statements())
    {
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
    }
    AnalysisState::EvaluationGuard CompileTime(State);
    return Succeeded && validateRuntimeClassTypes(State, *Result, Root) && cleanupObjects(State, 0, false, Root) ? Result : nullptr;
  }

  bool Analyzer::validateRuntimeClassTypes(AnalysisState &State, const Module &ModuleValue, const parser::ASTNodeBase &Root)
  {
    std::vector<const Value *> Values{&ModuleValue.entryBlock()};
    std::vector<const Type *> Types;
    while (!Values.empty())
    {
      const Value *Current = Values.back();
      Values.pop_back();
      Types.push_back(&Current->type());
      if (Function::classof(Current))
      {
        for (const auto &Block : static_cast<const Function *>(Current)->blocks())
        {
          Values.push_back(Block.get());
        }
      }
      else if (BasicBlock::classof(Current))
      {
        for (const auto &Child : static_cast<const BasicBlock *>(Current)->values())
        {
          Values.push_back(Child.get());
        }
      }
    }
    std::unordered_set<const Type *> Checked;
    while (!Types.empty())
    {
      const Type *Current = Types.back();
      Types.pop_back();
      if (!Checked.insert(Current).second)
      {
        continue;
      }
      if (ClassType::classof(Current))
      {
        const auto &Class = static_cast<const ClassType &>(*Current);
        if (!Class.isComplete())
        {
          const std::string Message = "class reachable from a function or stored value requires a complete definition: " + std::string(State.Context.namePool().text(Class.name()));
          const auto Definition = State.Context.classState().Definitions.find(&Class);
          if (Definition != State.Context.classState().Definitions.end())
          {
            State.Context.compilationContext().diagnosticEngine().report<core::DiagnosticKind::SemanticInvalidClass>(Definition->second.Input->lexedFile().sourceId(), Definition->second.Declaration->getSourceRange(), Message);
          }
          else
          {
            State.report<core::DiagnosticKind::SemanticInvalidClass>(Root.getSourceRange(), Message);
          }
          return false;
        }
        for (const ClassField &Field : Class.fields())
        {
          Types.push_back(Field.FieldType);
        }
      }
      else if (PointerType::classof(Current))
      {
        Types.push_back(&static_cast<const PointerType *>(Current)->pointeeType());
      }
      else if (ReferenceType::classof(Current))
      {
        Types.push_back(&static_cast<const ReferenceType *>(Current)->referentType());
      }
      else if (ArrayType::classof(Current))
      {
        Types.push_back(&static_cast<const ArrayType *>(Current)->elementType());
      }
      else if (FunctionType::classof(Current))
      {
        const auto &Signature = static_cast<const FunctionType &>(*Current);
        Types.push_back(&Signature.returnType());
        Types.insert(Types.end(), Signature.parameterTypes().begin(), Signature.parameterTypes().end());
      }
    }
    return true;
  }

  bool Analyzer::reportUnsupported(AnalysisState &State, const parser::ASTNodeBase &Node)
  {
    if (State.Evaluating)
    {
      return reportExecution(State, execution::ExecutionStatus::UnsupportedOperation, Node);
    }
    State.report<core::DiagnosticKind::SemanticUnsupported>(Node.getSourceRange(), parser::astKindName(Node.getKind()));
    return false;
  }

  // Recovery nodes are unsupported; analyze() rejects unsuccessful parse results.
  bool Analyzer::analyzeMissingStmt(AnalysisState &State, const parser::MissingStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorStmt(AnalysisState &State, const parser::ErrorStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeMissingDecl(AnalysisState &State, const parser::MissingDecl &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorDecl(AnalysisState &State, const parser::ErrorDecl &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
