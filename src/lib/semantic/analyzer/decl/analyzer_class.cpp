#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/parser/parser.h"
#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/constant/class_constant.h"

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
      Definition.Identity = State.CurrentClass ? State.Context.classState().Definitions.at(State.CurrentClass).Identity : std::string(State.Context.namePool().text(State.CurrentModule->name()));
      Definition.Identity += ".";
      Definition.Identity += Node.name().Text;
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
      Fields.push_back({FieldName, FieldType, Field.visibility() == parser::DeclarationVisibility::Private ? VisibilityKind::Private : VisibilityKind::Public});
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
      const Name Name = State.Context.namePool().intern(Definition.Identity + ".__default_" + std::to_string(Index));
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
      if (!Value || !Default.Builder.createReturnInstruction(Value))
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
    auto &Definition = Found->second;
    if (!ensureClassDefinition(State, Class, Node))
    {
      return {};
    }
    if (Node.arguments().size() > Class.fields().size())
    {
      State.report<core::DiagnosticKind::SemanticInvalidConstruction>(Node.getSourceRange(), "too many field arguments");
      return {};
    }
    std::vector<const Value *> Values;
    std::vector<const Constant *> Constants;
    for (std::size_t Index = 0; Index < Class.fields().size(); ++Index)
    {
      const auto &Field = Class.fields()[Index];
      const Value *Value = nullptr;
      if (Index < Node.arguments().size())
      {
        if (Field.Visibility == VisibilityKind::Private && State.CurrentClass != &Class)
        {
          State.report<core::DiagnosticKind::SemanticInvalidConstruction>(Node.getSourceRange(), "private fields cannot be supplied outside their class");
          return {};
        }
        const auto &Argument = Node.arguments()[Index];
        if (Argument.form() != parser::ArgumentKind::Positional)
        {
          State.report<core::DiagnosticKind::SemanticInvalidConstruction>(Argument.range(), "class construction accepts positional arguments only");
          return {};
        }
        AnalysisState::EvaluationGuard Expected(State, State.Evaluating, Field.FieldType);
        const ExpressionResult Initial = analyzeExpr(State, *Argument.value(), Depth + 1);
        Value = Initial ? convertExpression(State, Initial, *Field.FieldType, *Argument.value()) : nullptr;
      }
      else if (const Function *Default = Definition.Defaults[Index])
      {
        if (State.CurrentModule && State.CurrentModule != Definition.Module)
        {
          State.Context.recordModuleImport(*State.CurrentModule, *Default);
        }
        if (State.Modules && State.CurrentFunction)
        {
          State.Modules->Dependencies[State.CurrentFunction].insert(Default);
        }
        Value = State.Evaluating ? callComptime(State, *Default, {}, Node).ValueObject : State.Builder.createCallInstruction(*Default, {});
      }
      else
      {
        State.report<core::DiagnosticKind::SemanticInvalidConstruction>(Node.getSourceRange(), "missing field argument without a default initializer");
        return {};
      }
      if (!Value)
      {
        return {};
      }
      Values.push_back(Value);
      if (Constant::classof(Value))
      {
        Constants.push_back(static_cast<const Constant *>(Value));
      }
    }
    if (Constants.size() == Values.size())
    {
      return {State.Context.constantPool().getClassConstant(Class, Constants)};
    }
    if (State.Evaluating)
    {
      reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      return {};
    }
    return {State.Builder.createClassInstruction(Class, Values)};
  }
} // namespace ink::semantic
