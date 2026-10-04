#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/parser/parser.h"
#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/constant/class_constant.h"
#include "ink/ir/linkage.h"

#include <algorithm>

namespace ink::semantic
{
  using namespace ir;

  bool Analyzer::registerClass(AnalysisState &State, const parser::ClassDecl &Node)
  {
    if (State.CurrentFunction || State.Evaluating || !Node.genericParameters().empty() || !Node.bases().empty() || !Node.attributes().empty() || Node.isComptime() || (State.CurrentClass && Node.name().Text == "this"))
    {
      State.report<core::DiagnosticKind::SemanticInvalidClass>(Node.getSourceRange(), "only non-generic module or nested classes without inheritance are supported");
      return false;
    }
    const Name Symbol = State.Context.namePool().intern(Node.name().Text);
    const ClassType *Class = nullptr;
    if (const auto *Existing = State.Resolver.lookupLocal(Symbol))
    {
      if (Existing->targets().size() != 1 || !ClassType::classof(Existing->targets().front()))
      {
        State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
        return false;
      }
      Class = static_cast<const ClassType *>(Existing->targets().front());
      auto &Definition = State.Context.classState().Definitions.at(Class);
      if (Definition.Declaration == &Node || !Node.body())
      {
        return true;
      }
      if (Definition.Declaration->body())
      {
        State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
        return false;
      }
      Definition.Declaration = &Node;
    }
    else
    {
      Class = State.Builder.createClassType(Symbol);
      if (!Class || State.Resolver.bind(Symbol, *const_cast<ClassType *>(Class)) != NameResolver::BindResult::Inserted)
      {
        State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
        return false;
      }
      State.Builder.registerClassType(*State.CurrentModule, *Class);
      Scope *Parent = &State.Resolver.currentScope();
      NameResolver::ScopeGuard Members(State.Resolver, *const_cast<ClassType *>(Class));
      auto &Definition = State.Context.classState().Definitions[Class];
      Definition.Declaration = &Node;
      Definition.Module = State.CurrentModule;
      Definition.Input = &State.Input;
      Definition.Parent = Parent;
      Definition.EnclosingClass = State.CurrentClass;
      Definition.Members = Members.scope();
      std::vector<abi::Record> Owners;
      if (State.CurrentClass)
      {
        const auto Parent = abi::demangle(State.Context.classState().Definitions.at(State.CurrentClass).Identity);
        const auto Children = Parent ? abi::childRecords(*Parent.Identity) : std::nullopt;
        if (!Children || Children->size() != 1)
        {
          State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
          return false;
        }
        Owners.push_back(Children->front());
      }
      const auto Identity = abi::mangle(abi::record('T', {abi::record('c', {declarationRecord(State.CurrentModule->linkageIdentity(), Owners, 'c', Node.name().Text), {'X', {}}})}));
      if (!Identity)
      {
        State.report<core::DiagnosticKind::SemanticInvalidClass>(Node.getSourceRange(), Identity.Error);
        return false;
      }
      Definition.Identity = Identity.Name;
    }
    auto &Definition = State.Context.classState().Definitions.at(Class);
    if (Node.body())
    {
      AnalysisState Members(State.Context, *Definition.Members, State.Input);
      Members.CurrentModule = State.CurrentModule;
      Members.CurrentClass = Class;
      Members.Frame = State.Frame;
      Members.Modules = State.Modules;
      Members.Builder.setInsertPoint(State.CurrentModule->entryBlock());
      for (const parser::Stmt *Statement : Node.body()->statements())
      {
        if (parser::DeclStmt::classof(Statement))
        {
          const auto *Declaration = static_cast<const parser::DeclStmt *>(Statement)->declaration();
          if (parser::ClassDecl::classof(Declaration) && !registerClass(Members, static_cast<const parser::ClassDecl &>(*Declaration)))
          {
            return false;
          }
        }
      }
    }
    return true;
  }

  bool Analyzer::defineClass(AnalysisState &State, const parser::ClassDecl &Node)
  {
    const auto *Binding = State.Resolver.lookupLocal(State.Context.namePool().find(Node.name().Text));
    if (!Binding || Binding->targets().size() != 1 || !ClassType::classof(Binding->targets().front()))
    {
      return false;
    }
    const auto &Class = static_cast<const ClassType &>(*Binding->targets().front());
    auto &Definition = State.Context.classState().Definitions.at(&Class);
    if (!Node.body() || Class.isComplete())
    {
      return true;
    }
    AnalysisState Members(State.Context, *Definition.Members, State.Input);
    Members.CurrentModule = State.CurrentModule;
    Members.CurrentClass = &Class;
    Members.Frame = State.Frame;
    Members.Modules = State.Modules;
    Members.Builder.setInsertPoint(State.CurrentModule->entryBlock());
    std::vector<ClassField> Fields;
    std::unordered_set<Name> Names;
    for (const parser::Stmt *Statement : Node.body()->statements())
    {
      if (!parser::DeclStmt::classof(Statement))
      {
        State.report<core::DiagnosticKind::SemanticInvalidClass>(Statement->getSourceRange(), "class bodies contain fields, methods and nested classes only");
        return false;
      }
      const auto &Declaration = *static_cast<const parser::DeclStmt *>(Statement)->declaration();
      if (parser::ClassDecl::classof(&Declaration))
      {
        if (!defineClass(Members, static_cast<const parser::ClassDecl &>(Declaration)))
        {
          return false;
        }
        continue;
      }
      if (parser::FunctionDecl::classof(&Declaration))
      {
        continue;
      }
      if (!parser::FieldDecl::classof(&Declaration))
      {
        State.report<core::DiagnosticKind::SemanticInvalidClass>(Declaration.getSourceRange(), "unsupported class member declaration");
        return false;
      }
      const auto &Field = static_cast<const parser::FieldDecl &>(Declaration);
      if (Field.name().Text == "__init__" || Field.name().Text == "__del__")
      {
        State.report<core::DiagnosticKind::SemanticInvalidClass>(Field.getSourceRange(), "lifecycle names are reserved for methods");
        return false;
      }
      const Name FieldName = State.Context.namePool().intern(Field.name().Text);
      if (!Names.insert(FieldName).second || Members.Resolver.lookupLocal(FieldName) || Field.name().Text == "this")
      {
        State.report<core::DiagnosticKind::SemanticDuplicateName>(Field.name().Range, Field.name().Text);
        return false;
      }
      if (!Field.type() || !Field.payload().empty() || !Field.attributes().empty() || Field.isComptime())
      {
        State.report<core::DiagnosticKind::SemanticInvalidClass>(Field.getSourceRange(), "fields require an explicit value type and do not accept payloads, attributes or comptime");
        return false;
      }
      const Type *FieldType = analyzeType(Members, *Field.type()->expression());
      if (!FieldType)
      {
        return false;
      }
      if (FieldType->typeKind() == TypeKind::Void || FieldType->typeKind() == TypeKind::Meta || FieldType->typeKind() == TypeKind::Function)
      {
        State.report<core::DiagnosticKind::SemanticInvalidClass>(Field.getSourceRange(), "field type must have runtime storage");
        return false;
      }
      Fields.push_back({FieldName, FieldType, Field.visibility() == parser::DeclarationVisibility::Private ? core::VisibilityKind::Private : core::VisibilityKind::Public});
      Definition.Fields.push_back(&Field);
    }
    Definition.Defaults.resize(Fields.size(), nullptr);
    if (!State.Builder.defineClassType(Class, Fields, Definition.Identity))
    {
      State.report<core::DiagnosticKind::SemanticInvalidClass>(Node.getSourceRange(), "class contains an invalid or recursively embedded field type");
      return false;
    }
    return true;
  }

  bool Analyzer::declareClassMembers(AnalysisState &State, const parser::ClassDecl &Node)
  {
    const auto *Binding = State.Resolver.lookupLocal(State.Context.namePool().find(Node.name().Text));
    const auto &Class = static_cast<const ClassType &>(*Binding->targets().front());
    auto &Definition = State.Context.classState().Definitions.at(&Class);
    if (!Node.body() || Definition.MembersDeclared)
    {
      return true;
    }
    if (!computeTypeLayout(Class, State.Context.compilationContext().targetContext()))
    {
      State.report<core::DiagnosticKind::SemanticInvalidClass>(Node.getSourceRange(), "class fields require complete finite layouts");
      return false;
    }
    AnalysisState Members(State.Context, *Definition.Members, State.Input);
    Members.CurrentModule = State.CurrentModule;
    Members.CurrentClass = &Class;
    Members.DeclaringClass = &Class;
    Members.Frame = State.Frame;
    Members.Modules = State.Modules;
    Members.Builder.setInsertPoint(State.CurrentModule->entryBlock());
    for (const parser::Stmt *Statement : Node.body()->statements())
    {
      const auto &Declaration = *static_cast<const parser::DeclStmt *>(Statement)->declaration();
      if (parser::ClassDecl::classof(&Declaration))
      {
        if (!declareClassMembers(Members, static_cast<const parser::ClassDecl &>(Declaration)))
        {
          return false;
        }
      }
      else if (parser::FunctionDecl::classof(&Declaration))
      {
        const auto &Method = static_cast<const parser::FunctionDecl &>(Declaration);
        const Name MethodName = State.Context.namePool().intern(Method.name().Text);
        if (std::any_of(Class.fields().begin(), Class.fields().end(), [MethodName](const ClassField &Field)
                        {
                          return Field.FieldName == MethodName;
                        }))
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Method.name().Range, Method.name().Text);
          return false;
        }
        auto Owner = declareFunction(Members, Method);
        if (!Owner)
        {
          return false;
        }
        if (!State.Builder.setClassMethod(Class, *Owner))
        {
          return false;
        }
        Definition.Methods.emplace(&Method, Owner.get());
        State.Context.classState().MethodOwners.emplace(Owner.get(), &Class);
        if (!State.Builder.appendValue(State.CurrentModule->entryBlock(), std::move(Owner)))
        {
          return false;
        }
      }
    }
    if (!declareClassLifecycle(Members, Class))
    {
      return false;
    }
    Definition.MembersDeclared = true;
    return true;
  }

  bool Analyzer::analyzeClassDecl(AnalysisState &State, const parser::ClassDecl &Node)
  {
    if (!registerClass(State, Node))
    {
      return false;
    }
    const auto *Binding = State.Resolver.lookupLocal(State.Context.namePool().find(Node.name().Text));
    const auto &Class = static_cast<const ClassType &>(*Binding->targets().front());
    if (Node.body() && !Class.isComplete() && !defineClass(State, Node))
    {
      return false;
    }
    auto &Definition = State.Context.classState().Definitions.at(&Class);
    if (!Node.body() || Definition.BodyComplete || Definition.BodyAnalyzing)
    {
      return true;
    }
    if (!Definition.MembersDeclared && !declareClassMembers(State, Node))
    {
      return false;
    }
    Definition.BodyAnalyzing = true;
    struct BodyGuard
    {
        ClassDefinition &Definition;
        ~BodyGuard()
        {
          Definition.BodyAnalyzing = false;
        }
    };
    const BodyGuard Active{Definition};
    AnalysisState Members(State.Context, *Definition.Members, *Definition.Input);
    Members.CurrentModule = Definition.Module;
    Members.CurrentClass = &Class;
    Members.Frame = State.Frame;
    Members.Modules = State.Modules;
    Members.Builder.setInsertPoint(Definition.Module->entryBlock());
    for (std::size_t Index = 0; Index < Definition.Fields.size(); ++Index)
    {
      const auto *Initializer = Definition.Fields[Index]->initializer();
      if (!Initializer || Definition.Defaults[Index])
      {
        continue;
      }
      const Type &FieldType = *Class.fields()[Index].FieldType;
      const FunctionType *Signature = State.Context.typePool().getType<TypeKind::Function>(FieldType, {});
      const Name Name = State.Context.namePool().intern("__default_" + std::string(State.Context.namePool().text(Class.fields()[Index].FieldName)));
      auto Function = State.Builder.createFunction(Name, *Signature);
      AnalysisState Default(State.Context, *Definition.Members, *Definition.Input);
      Default.CurrentModule = Definition.Module;
      Default.CurrentClass = &Class;
      Default.CurrentFunction = Function.get();
      Default.Modules = State.Modules;
      Default.Frame = State.Frame;
      Default.ExpectedType = &FieldType;
      auto *Body = Default.Builder.createFunctionBody(*Function);
      Default.Builder.setInsertPoint(*Body);
      const ExpressionResult Initial = analyzeExpr(Default, *Initializer);
      const Value *Value = Initial ? convertExpression(Default, Initial, FieldType, *Initializer) : nullptr;
      takeTemporary(Default, Initial.TemporaryAddress);
      if (!Value || !cleanupObjects(Default, 0, false, *Initializer) || !Default.Builder.createReturnInstruction(Value))
      {
        return false;
      }
      if (!State.Builder.setFieldInitializer(Class, Index, *Function))
      {
        return false;
      }
      Definition.Defaults[Index] = Function.get();
      if (!State.Builder.appendValue(Definition.Module->entryBlock(), std::move(Function)))
      {
        return false;
      }
    }
    for (const parser::Stmt *Statement : Node.body()->statements())
    {
      const auto &Declaration = *static_cast<const parser::DeclStmt *>(Statement)->declaration();
      if (parser::ClassDecl::classof(&Declaration))
      {
        if (!analyzeClassDecl(Members, static_cast<const parser::ClassDecl &>(Declaration)))
        {
          return false;
        }
      }
      else if (parser::FunctionDecl::classof(&Declaration))
      {
        const auto &Method = static_cast<const parser::FunctionDecl &>(Declaration);
        auto *Function = Definition.Methods.at(&Method);
        if (!Function->entryBlock() && !analyzeFunctionBody(Members, Method, *Function))
        {
          return false;
        }
      }
    }
    for (const Function *Method : Class.methods())
    {
      if (Method->entryBlock())
      {
        continue;
      }
      AnalysisState Generated(State.Context, *Definition.Members, *Definition.Input);
      Generated.CurrentModule = Definition.Module;
      Generated.CurrentClass = &Class;
      Generated.CurrentFunction = const_cast<Function *>(Method);
      Generated.Modules = State.Modules;
      Generated.Frame = State.Frame;
      Generated.Constructing = State.Context.namePool().text(Method->name()) == "__init__";
      Generated.Destroying = !Generated.Constructing;
      auto *Body = Generated.Builder.createFunctionBody(*Generated.CurrentFunction);
      if (!Body || !Generated.Builder.setInsertPoint(*Body) || (Generated.Constructing && !initializeClassFields(Generated, Node)) || !destroyClassFields(Generated, Node) || !Generated.Builder.createReturnInstruction())
      {
        return false;
      }
    }
    Definition.BodyComplete = true;
    return true;
  }

  bool Analyzer::ensureClassDefinition(AnalysisState &State, const ClassType &Class, const parser::ASTNodeBase &Use)
  {
    auto &Definition = State.Context.classState().Definitions.at(&Class);
    if (Definition.BodyComplete || Definition.BodyAnalyzing)
    {
      return true;
    }
    execution::ExecutionFrame *Frame = State.Frame;
    const ClassDefinition *Boundary = &Definition;
    while (Boundary->EnclosingClass)
    {
      Boundary = &State.Context.classState().Definitions.at(Boundary->EnclosingClass);
    }
    if (State.Modules)
    {
      const auto Found = State.Modules->Modules.find(std::string(State.Context.namePool().text(Definition.Module->name())));
      if (Found != State.Modules->Modules.end())
      {
        auto &Module = *Found->second;
        Frame = Module.ModuleFrame;
        for (const parser::Stmt *Statement : Module.Source->Input->Unit->root()->statements())
        {
          if (parser::DeclStmt::classof(Statement))
          {
            const auto *Declaration = static_cast<const parser::DeclStmt *>(Statement)->declaration();
            if (Declaration == Boundary->Declaration)
            {
              break;
            }
            if (parser::FunctionDecl::classof(Declaration))
            {
              const auto *Function = Module.Functions.at(static_cast<const parser::FunctionDecl *>(Declaration));
              if (!State.Modules->ActiveBodies.contains(Function) && !ensureModuleFunctionBody(State, *Function, Use))
              {
                return false;
              }
              continue;
            }
          }
          if (parser::DirectImportStmt::classof(Statement) || parser::FromImportStmt::classof(Statement) || Module.ProcessedStatements.contains(Statement))
          {
            continue;
          }
          if (Module.ActiveStatement)
          {
            State.report<core::DiagnosticKind::SemanticComptimeStatementDependency>(Use.getSourceRange());
            return false;
          }
          Module.ProcessedStatements.insert(Statement);
          if (!analyzeModuleStatement(Module, *Statement))
          {
            return false;
          }
        }
      }
    }
    AnalysisState Defining(State.Context, *Definition.Parent, *Definition.Input);
    Defining.CurrentModule = Definition.Module;
    Defining.Modules = State.Modules;
    Defining.Frame = Frame;
    Defining.Builder.setInsertPoint(Definition.Module->entryBlock());
    return analyzeClassDecl(Defining, *Definition.Declaration);
  }

  Analyzer::ExpressionResult Analyzer::analyzeClassConstruction(AnalysisState &State, const ClassType &Class, const parser::CallExpr &Node, std::size_t Depth)
  {
    const auto Found = State.Context.classState().Definitions.find(&Class);
    if (!Class.isComplete() || Found == State.Context.classState().Definitions.end())
    {
      State.report<core::DiagnosticKind::SemanticInvalidConstruction>(Node.getSourceRange(), "class is incomplete");
      return {};
    }
    if (!ensureClassDefinition(State, Class, Node))
    {
      return {};
    }
    const auto *Binding = State.Resolver.lookupMember(*const_cast<ClassType *>(&Class), State.Context.namePool().find("__init__"));
    std::vector<const Value *> Candidates;
    if (Binding)
    {
      for (const Value *Target : Binding->targets())
      {
        if (Function::classof(Target) && (static_cast<const Function *>(Target)->visibility() == core::VisibilityKind::Public || State.CurrentClass == &Class))
        {
          Candidates.push_back(Target);
        }
      }
    }
    if (Candidates.empty())
    {
      State.report<core::DiagnosticKind::SemanticInvalidConstruction>(Node.getSourceRange(), "class has no accessible __init__; fields without defaults require an explicit constructor");
      return {};
    }
    const Value *Address = nullptr;
    if (State.Evaluating)
    {
      auto Storage = State.Builder.createDetachedAllocaInstruction(Class);
      auto &Execution = State.Context.comptimeState();
      const auto Place = Execution.Engine.allocate(*State.Frame, Storage.get(), Class);
      if (!reportExecution(State, Place.Status, Node))
      {
        return {};
      }
      Address = Storage.get();
      Execution.Bindings.push_back(std::move(Storage));
    }
    else
    {
      Address = State.Builder.createAllocaInstruction(Class);
    }
    if (!Address)
    {
      return {};
    }
    std::vector<ExpressionResult> Arguments{{Address}};
    std::vector<const parser::Expr *> Nodes{&Node};
    for (const auto &Argument : Node.arguments())
    {
      if (Argument.form() != parser::ArgumentKind::Positional)
      {
        State.report<core::DiagnosticKind::SemanticInvalidConstruction>(Argument.range(), "constructors accept positional arguments only");
        return {};
      }
      const auto Parameters = static_cast<const FunctionType &>(Candidates.front()->type()).parameterTypes();
      const Type *Parameter = Candidates.size() == 1 && Arguments.size() < Parameters.size() ? Parameters[Arguments.size()] : nullptr;
      const bool Deferred = State.Evaluating && !Parameter && findDeferredIntegerLiteral(*Argument.value(), Depth + 1, State.ExpressionDepthLimit);
      AnalysisState::EvaluationGuard Expected(State, State.Evaluating && !Deferred, Parameter);
      const ExpressionResult Value = analyzeExpr(State, *Argument.value(), Depth + 1);
      if (!Value)
      {
        return {};
      }
      Arguments.push_back(Value);
      Nodes.push_back(Argument.value());
    }
    if (!finishCall(State, Candidates, Arguments, Nodes, Node))
    {
      return {};
    }
    const Value *Result = nullptr;
    if (State.Evaluating)
    {
      auto &Engine = State.Context.comptimeState().Engine;
      const auto Place = Engine.lookup(*State.Frame, Address);
      const auto Loaded = Place ? Engine.load(Place.Place) : execution::ExecutionResult{Place.Status};
      if (!reportExecution(State, Loaded.Status, Node))
      {
        return {};
      }
      Result = Loaded.Value;
    }
    else
    {
      Result = State.Builder.createLoadInstruction(*Address);
    }
    return Result && trackObject(State, *Address, Class, true, Result) ? ExpressionResult{Result, nullptr, false, false, Address} : ExpressionResult{};
  }
} // namespace ink::semantic
