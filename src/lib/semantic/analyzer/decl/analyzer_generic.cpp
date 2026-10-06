#include "../analyzer_internal.h"

#include "ink/ir/linkage.h"
#include "ink/parser/parser.h"
#include "ink/parser/ast_walker.h"

#include <algorithm>

namespace ink::semantic
{
  using namespace ir;

  namespace
  {
    class GenericDepthGuard
    {
      public:
        explicit GenericDepthGuard(GenericState &State)
            : State(State)
        {
          ++State.Depth;
        }

        ~GenericDepthGuard()
        {
          --State.Depth;
        }

      private:
        GenericState &State;
    };

    const parser::Expr &unparenthesized(const parser::Expr &Node)
    {
      const parser::Expr *Result = &Node;
      while (parser::ParenExpr::classof(Result))
      {
        Result = static_cast<const parser::ParenExpr *>(Result)->expression();
      }
      return *Result;
    }

    std::string constantBits(const Value &Value)
    {
      if (BoolConstant::classof(&Value))
      {
        return static_cast<const BoolConstant &>(Value).value() ? "1" : "0";
      }
      const auto &Bits = static_cast<const IntegerConstant &>(Value).value();
      std::string Result((Bits.bitWidth() + 3) / 4, '0');
      constexpr std::string_view Digits = "0123456789ABCDEF";
      for (std::size_t Index = 0; Index < Result.size(); ++Index)
      {
        Result[Result.size() - Index - 1] = Digits[(Bits.words()[Index / 16] >> ((Index % 16) * 4)) & 15];
      }
      return Result;
    }

    std::string genericSignatureKey(const parser::FunctionDecl &Declaration, const parser::TokenBuffer &Input)
    {
      std::function<std::string(const parser::Expr &)> Key = [&](const parser::Expr &Expression) -> std::string
      {
        const auto &Node = unparenthesized(Expression);
        if (parser::NameExpr::classof(&Node))
        {
          const auto Name = static_cast<const parser::NameExpr &>(Node).name().Text;
          for (std::size_t Index = 0; Index < Declaration.genericParameters().size(); ++Index)
          {
            if (Declaration.genericParameters()[Index].name().Text == Name)
            {
              return "$" + std::to_string(Index);
            }
          }
          return std::string(Name);
        }
        if (parser::LiteralExpr::classof(&Node))
        {
          return std::string(Input.spelling(static_cast<const parser::LiteralExpr &>(Node).token()));
        }
        std::string Result = std::to_string(static_cast<unsigned>(Node.getKind())) + ":";
        if (parser::UnaryExpr::classof(&Node))
        {
          Result += std::to_string(static_cast<unsigned>(static_cast<const parser::UnaryExpr &>(Node).op()));
        }
        if (parser::BinaryExpr::classof(&Node))
        {
          Result += std::to_string(static_cast<unsigned>(static_cast<const parser::BinaryExpr &>(Node).op()));
        }
        if (parser::MemberExpr::classof(&Node))
        {
          Result += static_cast<const parser::MemberExpr &>(Node).member().Text;
        }
        parser::forEachChild(&Node, [&](const parser::ASTNodeBase *Child)
        {
          if (parser::Expr::classof(Child))
          {
            const auto Text = Key(static_cast<const parser::Expr &>(*Child));
            Result += std::to_string(Text.size()) + ":" + Text;
          }
        });
        return Result;
      };
      std::string Result;
      for (const auto &Parameter : Declaration.genericParameters())
      {
        Result += "[" + Key(*Parameter.type()->expression()) + "]";
      }
      for (const auto &Parameter : Declaration.parameters())
      {
        Result += "(" + Key(*Parameter.type()->expression()) + ")";
      }
      return Result;
    }

    bool hasGenericEffects(const parser::ASTNodeBase &Root)
    {
      std::vector<const parser::ASTNodeBase *> Pending{&Root};
      while (!Pending.empty())
      {
        const auto *Node = Pending.back();
        Pending.pop_back();
        if (parser::CallExpr::classof(Node) || parser::PostfixUpdateExpr::classof(Node) || parser::GenericApplyExpr::classof(Node))
        {
          return true;
        }
        if (parser::UnaryExpr::classof(Node))
        {
          const auto Op = static_cast<const parser::UnaryExpr *>(Node)->op();
          if (Op == tokenizer::TokenKind::PlusPlus || Op == tokenizer::TokenKind::MinusMinus)
          {
            return true;
          }
        }
        parser::forEachChild(Node, [&](const parser::ASTNodeBase *Child)
        {
          Pending.push_back(Child);
        });
      }
      return false;
    }
  } // namespace

  bool Analyzer::registerGenericFunction(AnalysisState &State, const parser::FunctionDecl &Node, bool Capture)
  {
    const auto Linkage = analyzeFunctionLinkage(State, Node);
    if (!Linkage)
    {
      return false;
    }
    if (State.Evaluating || State.CurrentClass || *Linkage != LanguageLinkage::Ink || Node.nativeSymbolKind() != core::FunctionBinding::Local)
    {
      State.report<core::DiagnosticKind::SemanticInvalidGeneric>(Node.getSourceRange(), "generic functions require a lexical declaration with Ink linkage");
      return false;
    }
    if (!Node.body())
    {
      State.report<core::DiagnosticKind::SemanticFunctionRequiresBody>(Node.getSourceRange(), Node.name().Text);
      return false;
    }
    if ((State.CurrentFunction || State.BlockDepth) && Node.visibility() == parser::DeclarationVisibility::Public)
    {
      State.report<core::DiagnosticKind::SemanticPublicLocal>(Node.getSourceRange(), Node.name().Text);
      return false;
    }
    std::unordered_set<std::string_view> Names;
    for (const auto &Parameter : Node.genericParameters())
    {
      if (!Names.insert(Parameter.name().Text).second)
      {
        State.report<core::DiagnosticKind::SemanticDuplicateParameterName>(Parameter.range(), Parameter.name().Text);
        return false;
      }
      if (Parameter.variadic() || !Parameter.type())
      {
        State.report<core::DiagnosticKind::SemanticInvalidGeneric>(Parameter.range(), "generic parameters require explicit types and cannot be variadic");
        return false;
      }
    }
    for (const auto &Parameter : Node.parameters())
    {
      if (!Names.insert(Parameter.name().Text).second)
      {
        State.report<core::DiagnosticKind::SemanticDuplicateParameterName>(Parameter.range(), Parameter.name().Text);
        return false;
      }
      if (Parameter.variadic() || Parameter.defaultValue())
      {
        State.report<core::DiagnosticKind::SemanticInvalidGeneric>(Parameter.range(), "runtime parameters must be fixed positional parameters without defaults");
        return false;
      }
    }
    const Name Name = State.Context.namePool().intern(Node.name().Text);
    if (const auto *Values = State.Resolver.lookupLocal(Name))
    {
      for (const Value *Value : Values->targets())
      {
        if (!Function::classof(Value) || static_cast<const Function *>(Value)->binding() != core::FunctionBinding::Local)
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
          return false;
        }
      }
    }
    if (const auto *Existing = State.Resolver.lookupLocal<Decl *>(Name))
    {
      for (Decl *Value : Existing->targets())
      {
        if (FunctionDecl::classof(Value) && &static_cast<FunctionDecl *>(Value)->ast() == &Node)
        {
          return !Capture || captureGenericDefinition(State, static_cast<const FunctionDecl &>(*Value));
        }
        if (FunctionDecl::classof(Value))
        {
          const auto &Previous = State.Context.genericState().Definitions.at(static_cast<FunctionDecl *>(Value));
          if (genericSignatureKey(static_cast<FunctionDecl *>(Value)->ast(), *Previous.Input) == genericSignatureKey(Node, State.Input))
          {
            State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
            return false;
          }
        }
      }
    }
    FunctionDecl *Declaration = State.Builder.createFunctionDecl(*State.CurrentModule->declarationRoot(), Name, Node);
    if (!Declaration || State.Resolver.bind(Name, *Declaration) != NameResolver::BindResult::Inserted)
    {
      State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
      return false;
    }
    auto &Definition = State.Context.genericState().Definitions[Declaration];
    Definition.Declaration = Declaration;
    Definition.Input = &State.Input;
    Definition.Owner = State.Builder.insertBlock();
    Definition.Frame = State.Frame;
    Definition.LexicalScope = State.LexicalScope;
    Definition.Local = State.CurrentFunction || State.BlockDepth != 0;
    return !Capture || captureGenericDefinition(State, *Declaration);
  }

  bool Analyzer::captureGenericDefinition(AnalysisState &State, const FunctionDecl &Declaration)
  {
    auto &Definition = State.Context.genericState().Definitions.at(&Declaration);
    if (Definition.ScopeValue)
    {
      return true;
    }
    if (Definition.Capturing)
    {
      State.report<core::DiagnosticKind::SemanticGenericCycle>(Declaration.ast().getSourceRange());
      return false;
    }
    Definition.Capturing = true;
    struct CaptureGuard
    {
        bool &Capturing;
        ~CaptureGuard()
        {
          Capturing = false;
        }
    } Guard{Definition.Capturing};
    Scope *ScopeValue = State.Resolver.definitionScope(Declaration);
    if (State.Modules && Definition.Owner == &Declaration.module().entryBlock())
    {
      auto &Module = *State.Modules->Modules.at(std::string(State.Context.namePool().text(Declaration.module().name())));
      for (const parser::Stmt *Statement : Module.Source->Input->Unit->root()->statements())
      {
        if (parser::DeclStmt::classof(Statement))
        {
          const auto *Earlier = static_cast<const parser::DeclStmt *>(Statement)->declaration();
          if (Earlier == &Declaration.ast())
          {
            break;
          }
          if (parser::FunctionDecl::classof(Earlier))
          {
            const auto &Function = static_cast<const parser::FunctionDecl &>(*Earlier);
            if (!Function.genericParameters().empty())
            {
              if (!registerGenericFunction(*Module.State, Function))
              {
                return false;
              }
            }
            else
            {
              const auto *Value = Module.Functions.at(&Function);
              if (!State.Modules->ActiveBodies.contains(Value) && !ensureModuleFunctionBody(State, *Value, Declaration.ast()))
              {
                return false;
              }
            }
            continue;
          }
        }
        if (parser::DirectImportStmt::classof(Statement) || parser::FromImportStmt::classof(Statement))
        {
          continue;
        }
        if (!Module.ProcessedStatements.contains(Statement))
        {
          if (Module.ActiveStatement || std::any_of(Module.Functions.begin(), Module.Functions.end(), [&](const auto &Entry)
          {
            return State.Modules->ActiveBodies.contains(Entry.second);
          }))
          {
            State.report<core::DiagnosticKind::SemanticComptimeStatementDependency>(Declaration.ast().getSourceRange());
            return false;
          }
          Module.ProcessedStatements.insert(Statement);
          if (!analyzeModuleStatement(Module, *Statement))
          {
            return false;
          }
        }
      }
      ScopeValue = Module.ModuleScope;
    }
    bool Succeeded = true;
    auto *Snapshot = State.Context.scopeStore().snapshotScope(*ScopeValue, [&](Value *Target) -> Value *
    {
      auto &Comptime = State.Context.comptimeState();
      const auto Variable = Comptime.Variables.find(Target);
      if (Variable == Comptime.Variables.end() || !Variable->second.Comptime || !Variable->second.Initialized || static_cast<const PointerType &>(Target->type()).pointeeType().typeKind() == TypeKind::Pointer)
      {
        return Target;
      }
      const auto Place = Comptime.Engine.lookup(*Definition.Frame, Target);
      if (!reportExecution(State, Place.Status, Declaration.ast()))
      {
        Succeeded = false;
        return Target;
      }
      const auto Loaded = Comptime.Engine.load(Place.Place);
      Succeeded = reportExecution(State, Loaded.Status, Declaration.ast()) && Succeeded;
      return Loaded.Value ? const_cast<Constant *>(Loaded.Value) : Target;
    });
    if (!Succeeded)
    {
      return false;
    }
    Definition.ScopeValue = Snapshot;
    return Definition.ScopeValue != nullptr;
  }

  bool Analyzer::resolveGenericApplication(AnalysisState &State, const parser::GenericApplyExpr &Node, std::vector<const Value *> &Candidates, std::size_t Depth)
  {
    auto &Generics = State.Context.genericState();
    if (Generics.Depth >= std::min<std::size_t>(128, core::ConfigManager::getSize<core::ConfigKind::SemanticGenericDepthLimit>()))
    {
      State.report<core::DiagnosticKind::SemanticGenericLimit>(Node.getSourceRange());
      return false;
    }
    GenericDepthGuard DepthGuard(Generics);
    const auto &Object = unparenthesized(*Node.object());
    const Binding<Decl *> *Binding = nullptr;
    const Module *ImportedModule = nullptr;
    if (parser::NameExpr::classof(&Object))
    {
      Binding = State.Resolver.lookup<Decl *>(State.Context.namePool().find(static_cast<const parser::NameExpr &>(Object).name().Text));
    }
    else if (parser::MemberExpr::classof(&Object))
    {
      const auto &Member = static_cast<const parser::MemberExpr &>(Object);
      const auto Owner = analyzeExpr(State, *Member.object(), Depth + 1);
      if (Owner.ValueObject && Module::classof(Owner.ValueObject) && Member.access() == tokenizer::TokenKind::Dot)
      {
        ImportedModule = static_cast<const Module *>(Owner.ValueObject);
        Binding = State.Resolver.lookupMember<Decl *>(*const_cast<Module *>(ImportedModule), State.Context.namePool().find(Member.member().Text));
      }
    }
    if (!Binding)
    {
      State.report<core::DiagnosticKind::SemanticGenericArguments>(Node.getSourceRange(), "the target is not a generic function");
      return false;
    }
    std::vector<const FunctionDecl *> Declarations;
    for (const Decl *Target : Binding->targets())
    {
      if (!FunctionDecl::classof(Target))
      {
        continue;
      }
      const auto &Declaration = static_cast<const FunctionDecl &>(*Target);
      if (ImportedModule && (&Declaration.module() != ImportedModule || (State.CurrentModule != ImportedModule && Declaration.ast().visibility() == parser::DeclarationVisibility::Private)))
      {
        continue;
      }
      Declarations.push_back(&Declaration);
    }
    if (Declarations.empty())
    {
      State.report<core::DiagnosticKind::SemanticGenericArguments>(Node.getSourceRange(), "no accessible generic function was found");
      return false;
    }
    // Explicit expressions are evaluated once in source order, shared by all candidates.
    // Literal conversion is deferred until the candidate's parameter type is available.
    std::vector<ExpressionResult> Actuals;
    for (const auto &Argument : Node.arguments())
    {
      if (Argument.form() == parser::ArgumentKind::SpreadPositional)
      {
        State.report<core::DiagnosticKind::SemanticGenericArguments>(Argument.range(), "spread generic arguments are not supported");
        return false;
      }
      Actuals.push_back(analyzeGenericArgument(State, *Argument.value(), Depth + 1));
      if (!Actuals.back())
      {
        return false;
      }
    }
    for (const FunctionDecl *Declaration : Declarations)
    {
      const auto Parameters = Declaration->ast().genericParameters();
      std::vector<std::size_t> Mapping(Parameters.size(), Node.arguments().size());
      bool Matches = true;
      std::size_t Positional = 0;
      bool Named = false;
      for (std::size_t Index = 0; Index < Node.arguments().size(); ++Index)
      {
        const auto &Argument = Node.arguments()[Index];
        std::size_t Destination = Parameters.size();
        if (Argument.form() == parser::ArgumentKind::Named)
        {
          Named = true;
          for (std::size_t Parameter = 0; Parameter < Parameters.size(); ++Parameter)
          {
            if (Parameters[Parameter].name().Text == Argument.name()->Text)
            {
              Destination = Parameter;
              break;
            }
          }
        }
        else if (Argument.form() == parser::ArgumentKind::Positional && !Named)
        {
          Destination = Positional++;
        }
        if (Destination >= Parameters.size() || Mapping[Destination] != Node.arguments().size())
        {
          Matches = false;
          break;
        }
        Mapping[Destination] = Index;
      }
      for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
      {
        Matches = Matches && (Mapping[Index] != Node.arguments().size() || Parameters[Index].defaultValue());
      }
      if (!Matches)
      {
        continue;
      }
      if (Declarations.size() > 1)
      {
        bool Effects = false;
        for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
        {
          Effects = Effects || hasGenericEffects(*Parameters[Index].type()) || (Mapping[Index] == Node.arguments().size() && hasGenericEffects(*Parameters[Index].defaultValue()));
        }
        for (const auto &Parameter : Declaration->ast().parameters())
        {
          Effects = Effects || hasGenericEffects(*Parameter.type());
        }
        Effects = Effects || hasGenericEffects(*Declaration->ast().returnType());
        if (Effects)
        {
          State.report<core::DiagnosticKind::SemanticGenericArguments>(Node.getSourceRange(), "overloaded generic signatures and omitted defaults must be free of calls and updates");
          return false;
        }
      }
      if (!captureGenericDefinition(State, *Declaration))
      {
        return false;
      }
      auto &Definition = Generics.Definitions.at(Declaration);
      AnalysisState Bound(State.Context, *Definition.ScopeValue, *Definition.Input);
      Bound.CurrentModule = &Definition.Declaration->module();
      Bound.Modules = State.Modules;
      Bound.Frame = Definition.Frame;
      Bound.LexicalScope = Definition.LexicalScope;
      Bound.BlockDepth = Definition.Local ? 1 : 0;
      Bound.Builder.setInsertPoint(*Definition.Owner);
      NameResolver::ScopeGuard ParametersScope(Bound.Resolver);
      std::vector<const Value *> Values;
      for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
      {
        const auto &Parameter = Parameters[Index];
        const Type *Expected = analyzeType(Bound, *Parameter.type()->expression());
        if (!Expected)
        {
          return false;
        }
        const bool TypeArgument = Expected->typeKind() == TypeKind::Meta;
        if (!TypeArgument && Expected->typeKind() != TypeKind::Bool && !IntegerType::classof(Expected))
        {
          Bound.report<core::DiagnosticKind::SemanticInvalidGeneric>(Parameter.range(), "value parameters require bool or integer types");
          return false;
        }
        const bool Default = Mapping[Index] == Node.arguments().size();
        const parser::Expr &Expression = Default ? *Parameter.defaultValue() : *Node.arguments()[Mapping[Index]].value();
        AnalysisState &Environment = Default ? Bound : State;
        ExpressionResult Argument = Default ? analyzeGenericArgument(Bound, Expression, Depth + 1) : Actuals[Mapping[Index]];
        if (!Argument)
        {
          return false;
        }
        if (Argument.IntegerLiteral && IntegerType::classof(Expected))
        {
          const auto Bits = integerBits(Environment.Input, *Argument.IntegerLiteral, Argument.Negative, static_cast<const IntegerType &>(*Expected));
          if (!Bits)
          {
            Matches = false;
            break;
          }
          Argument.ValueObject = State.Context.constantPool().getIntegerConstant(static_cast<const IntegerType &>(*Expected), *Bits);
        }
        if (!Argument.ValueObject || (TypeArgument ? !Type::classof(Argument.ValueObject) : &Argument.ValueObject->type() != Expected))
        {
          Matches = false;
          break;
        }
        Values.push_back(Argument.ValueObject);
        if (Bound.Resolver.bind(State.Context.namePool().intern(Parameter.name().Text), *const_cast<Value *>(Argument.ValueObject)) != NameResolver::BindResult::Inserted)
        {
          return false;
        }
      }
      if (!Matches)
      {
        continue;
      }
      auto Existing = std::find_if(Definition.Instances.begin(), Definition.Instances.end(), [&](const auto &Instance)
      {
        return Instance->Arguments == Values;
      });
      if (Existing != Definition.Instances.end())
      {
        if ((*Existing)->State == GenericState::Phase::Signature)
        {
          State.report<core::DiagnosticKind::SemanticGenericCycle>(Node.getSourceRange());
          return false;
        }
        if ((*Existing)->State == GenericState::Phase::Failed)
        {
          return false;
        }
        Candidates.push_back((*Existing)->Function);
        continue;
      }
      if (Generics.InstanceCount >= core::ConfigManager::getSize<core::ConfigKind::SemanticGenericInstanceLimit>())
      {
        State.report<core::DiagnosticKind::SemanticGenericLimit>(Node.getSourceRange());
        return false;
      }
      ++Generics.InstanceCount;
      auto InstanceOwner = std::make_unique<GenericState::Instance>();
      auto &Instance = *InstanceOwner;
      Instance.Arguments = std::move(Values);
      Instance.Bindings = ParametersScope.scope();
      Definition.Instances.push_back(std::move(InstanceOwner));
      Instance.Pending = declareFunction(Bound, Declaration->ast(), true);
      Instance.Function = Instance.Pending.get();
      Instance.State = Instance.Function ? GenericState::Phase::Pending : GenericState::Phase::Failed;
      if (!Instance.Function)
      {
        return false;
      }
      Candidates.push_back(Instance.Function);
    }
    if (Candidates.empty())
    {
      State.report<core::DiagnosticKind::SemanticGenericArguments>(Node.getSourceRange(), "no generic declaration accepts these arguments");
      return false;
    }
    return true;
  }

  Analyzer::ExpressionResult Analyzer::analyzeGenericArgument(AnalysisState &State, const parser::Expr &Node, std::size_t Depth)
  {
    if (Depth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticGenericLimit>(Node.getSourceRange());
      return {};
    }
    std::function<bool(const parser::Expr &, std::size_t)> IsType;
    IsType = [&](const parser::Expr &Expression, std::size_t Level)
    {
      if (Level >= State.TypeDepthLimit)
      {
        return false;
      }
      const auto &Syntax = unparenthesized(Expression);
      if (parser::NameExpr::classof(&Syntax))
      {
        const auto Text = static_cast<const parser::NameExpr &>(Syntax).name().Text;
        const auto Name = State.Context.namePool().find(Text);
        if (const auto *Binding = State.Resolver.lookup(Name))
        {
          return Binding->targets().size() == 1 && Type::classof(Binding->targets().front());
        }
        return !State.Resolver.lookup<Decl *>(Name) && builtinType(State.Context.typePool(), Text);
      }
      if (parser::UnaryExpr::classof(&Syntax))
      {
        const auto &Unary = static_cast<const parser::UnaryExpr &>(Syntax);
        return (Unary.op() == tokenizer::TokenKind::Star || Unary.op() == tokenizer::TokenKind::Amp) && IsType(*Unary.operand(), Level + 1);
      }
      if (parser::MemberExpr::classof(&Syntax))
      {
        const auto &Member = static_cast<const parser::MemberExpr &>(Syntax);
        const auto &Owner = unparenthesized(*Member.object());
        if (Member.access() == tokenizer::TokenKind::Dot && parser::NameExpr::classof(&Owner))
        {
          const auto *Binding = State.Resolver.lookup(State.Context.namePool().find(static_cast<const parser::NameExpr &>(Owner).name().Text));
          if (Binding && Binding->targets().size() == 1 && Module::classof(Binding->targets().front()))
          {
            const auto *TypeBinding = State.Resolver.lookupMember(*Binding->targets().front(), State.Context.namePool().find(Member.member().Text));
            return TypeBinding && TypeBinding->targets().size() == 1 && Type::classof(TypeBinding->targets().front());
          }
        }
      }
      if (parser::ArrayRepeatExpr::classof(&Syntax))
      {
        return IsType(*static_cast<const parser::ArrayRepeatExpr &>(Syntax).value(), Level + 1);
      }
      return false;
    };
    if (IsType(Node, 0))
    {
      if (Node.isComptime())
      {
        State.report<core::DiagnosticKind::SemanticGenericArguments>(Node.getSourceRange(), "type arguments are already compile-time values");
        return {};
      }
      return {analyzeType(State, Node)};
    }
    const parser::Expr *Literal = &Node;
    bool Negative = false;
    while (Depth++ < State.ExpressionDepthLimit)
    {
      if (parser::ParenExpr::classof(Literal))
      {
        Literal = static_cast<const parser::ParenExpr *>(Literal)->expression();
      }
      else if (parser::UnaryExpr::classof(Literal) && (static_cast<const parser::UnaryExpr *>(Literal)->op() == tokenizer::TokenKind::Plus || static_cast<const parser::UnaryExpr *>(Literal)->op() == tokenizer::TokenKind::Minus))
      {
        const auto &Unary = static_cast<const parser::UnaryExpr &>(*Literal);
        Negative = Negative != (Unary.op() == tokenizer::TokenKind::Minus);
        Literal = Unary.operand();
      }
      else
      {
        break;
      }
    }
    if (parser::LiteralExpr::classof(Literal) && static_cast<const parser::LiteralExpr *>(Literal)->literalKind() == tokenizer::TokenKind::IntegerLiteral)
    {
      return {nullptr, static_cast<const parser::LiteralExpr *>(Literal), Negative};
    }
    if (parser::LiteralExpr::classof(Literal))
    {
      State.report<core::DiagnosticKind::SemanticGenericArguments>(Node.getSourceRange(), "value arguments currently support bool and integer constants");
      return {};
    }
    if (parser::MemberExpr::classof(&unparenthesized(Node)))
    {
      AnalysisState::EvaluationGuard Evaluation(State);
      const auto Result = analyzeExpr(State, Node);
      if (Result.ValueObject && Type::classof(Result.ValueObject))
      {
        return Result;
      }
      if (!Result)
      {
        return {};
      }
      if (Result.ValueObject && Constant::classof(Result.ValueObject))
      {
        return Result;
      }
      State.report<core::DiagnosticKind::SemanticGenericArguments>(Node.getSourceRange(), "generic arguments must be types or compile-time constants");
      return {};
    }
    return evaluateComptime(State, Node);
  }

  Analyzer::ExpressionResult Analyzer::analyzeGenericApplyExpr(AnalysisState &State, const parser::GenericApplyExpr &Node, std::size_t Depth)
  {
    std::vector<const Value *> Candidates;
    if (!resolveGenericApplication(State, Node, Candidates, Depth))
    {
      return {};
    }
    if (Candidates.size() != 1)
    {
      State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
      return {};
    }
    const auto &Function = static_cast<const ir::Function &>(*Candidates.front());
    if (!State.Evaluating && !State.ComptimeFunction && State.Context.comptimeState().Functions.at(&Function).Comptime)
    {
      State.report<core::DiagnosticKind::SemanticComptimeFunctionAtRuntime>(Node.getSourceRange());
      return {};
    }
    return completeGenericFunction(State, Function, Node) ? ExpressionResult{&Function} : ExpressionResult{};
  }

  bool Analyzer::completeGenericFunction(AnalysisState &State, const Function &FunctionValue, const parser::ASTNodeBase &Use)
  {
    auto &Generics = State.Context.genericState();
    GenericState::Definition *Definition = nullptr;
    GenericState::Instance *Instance = nullptr;
    for (auto &[Declaration, Candidate] : Generics.Definitions)
    {
      for (auto &Value : Candidate.Instances)
      {
        if (Value->Function == &FunctionValue)
        {
          Definition = &Candidate;
          Instance = Value.get();
          break;
        }
      }
      if (Instance)
      {
        break;
      }
    }
    if (!Instance)
    {
      return true;
    }
    if (State.Modules && State.CurrentFunction)
    {
      State.Modules->Dependencies[State.CurrentFunction].insert(&FunctionValue);
    }
    if (State.CurrentModule != &Definition->Declaration->module())
    {
      State.Context.recordModuleImport(*State.CurrentModule, FunctionValue);
    }
    if (Instance->State == GenericState::Phase::Ready || Instance->State == GenericState::Phase::Checking)
    {
      if (Instance->State == GenericState::Phase::Checking && State.Evaluating)
      {
        State.report<core::DiagnosticKind::SemanticComptimeBodyDependency>(Use.getSourceRange(), State.Context.namePool().text(FunctionValue.name()));
        return false;
      }
      return true;
    }
    if (Instance->State == GenericState::Phase::Failed || !Definition->Owner)
    {
      return false;
    }
    if (Generics.Depth >= std::min<std::size_t>(128, core::ConfigManager::getSize<core::ConfigKind::SemanticGenericDepthLimit>()))
    {
      State.report<core::DiagnosticKind::SemanticGenericLimit>(Use.getSourceRange());
      return false;
    }
    GenericDepthGuard DepthGuard(Generics);
    AnalysisState Bound(State.Context, *Instance->Bindings, *Definition->Input);
    Bound.CurrentModule = &Definition->Declaration->module();
    Bound.Modules = State.Modules;
    Bound.Frame = Definition->Frame;
    Bound.Builder.setInsertPoint(*Definition->Owner);
    Instance->State = GenericState::Phase::Failed;
    if (!Bound.Builder.appendValue(*Definition->Owner, std::move(Instance->Pending)))
    {
      return false;
    }
    const auto &Node = Definition->Declaration->ast();
    auto Identity = functionRecord(FunctionValue);
    auto Parts = Identity ? abi::childRecords(*Identity) : std::nullopt;
    auto Declaration = Parts ? abi::childRecords((*Parts)[0]) : std::nullopt;
    if (!Declaration)
    {
      Bound.report<core::DiagnosticKind::SemanticInvalidGeneric>(Node.getSourceRange(), "the instance has no supported linkage identity");
      Instance->State = GenericState::Phase::Failed;
      return false;
    }
    const auto Parameters = Node.genericParameters();
    std::function<std::optional<abi::Record>(const parser::Expr &, const abi::Record &, std::size_t)> Pattern;
    Pattern = [&](const parser::Expr &Expression, const abi::Record &Closed, std::size_t Depth) -> std::optional<abi::Record>
    {
      if (Depth >= Bound.TypeDepthLimit)
      {
        return std::nullopt;
      }
      const auto &Syntax = unparenthesized(Expression);
      if (parser::NameExpr::classof(&Syntax))
      {
        const auto Text = static_cast<const parser::NameExpr &>(Syntax).name().Text;
        for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
        {
          if (Text == Parameters[Index].name().Text)
          {
            return abi::record('g', {{'D', "0"}, {'D', std::to_string(Index)}});
          }
        }
      }
      if (parser::UnaryExpr::classof(&Syntax))
      {
        const auto &Unary = static_cast<const parser::UnaryExpr &>(Syntax);
        const auto Children = abi::childRecords(Closed, 1);
        const auto Child = Children && Children->size() == 1 ? Pattern(*Unary.operand(), Children->front(), Depth + 1) : std::nullopt;
        if (Child && (Unary.op() == tokenizer::TokenKind::Star || Unary.op() == tokenizer::TokenKind::Amp))
        {
          return abi::Record{Unary.op() == tokenizer::TokenKind::Star ? 'p' : 'r', "w" + abi::encodeRecord(*Child)};
        }
        return std::nullopt;
      }
      if (parser::ArrayRepeatExpr::classof(&Syntax))
      {
        const auto &Array = static_cast<const parser::ArrayRepeatExpr &>(Syntax);
        const auto Children = abi::childRecords(Closed);
        if (!Children || Children->size() != 2)
        {
          return std::nullopt;
        }
        const auto Element = Pattern(*Array.value(), Children->back(), Depth + 1);
        const auto &Count = unparenthesized(*Array.count());
        std::optional<abi::Record> Length;
        if (parser::NameExpr::classof(&Count))
        {
          for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
          {
            if (static_cast<const parser::NameExpr &>(Count).name().Text == Parameters[Index].name().Text)
            {
              Length = abi::record('g', {{'D', "0"}, {'D', std::to_string(Index)}});
            }
          }
        }
        if (!Length)
        {
          Length = Children->front();
        }
        return Element && Length ? std::optional<abi::Record>(abi::record('a', {*Length, *Element})) : std::nullopt;
      }
      return Closed;
    };
    std::vector<abi::Record> Schema;
    std::vector<abi::Record> Arguments;
    std::vector<abi::Record> Signature;
    std::vector<const ir::Type *> ArgumentTypes;
    for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
    {
      const Value &Argument = *Instance->Arguments[Index];
      ArgumentTypes.push_back(Type::classof(&Argument) ? static_cast<const ir::Type *>(&Argument) : &Argument.type());
      const auto Type = typeRecord(Type::classof(&Argument) ? static_cast<const ir::Type &>(Argument) : Argument.type());
      if (!Type)
      {
        Bound.report<core::DiagnosticKind::SemanticGenericArguments>(Parameters[Index].range(), "argument has no stable type identity");
        Instance->State = GenericState::Phase::Failed;
        return false;
      }
      if (ir::Type::classof(&Argument))
      {
        Schema.push_back({'T', {}});
        Arguments.push_back(abi::record('T', {*Type}));
      }
      else
      {
        const auto ParameterType = Pattern(*Parameters[Index].type()->expression(), *Type, 0);
        if (!ParameterType)
        {
          return false;
        }
        Schema.push_back(abi::record('V', {*ParameterType}));
        Arguments.push_back(abi::record('V', {*Type, {'B', constantBits(Argument)}}));
      }
    }
    for (std::size_t Index = 0; Index < Node.parameters().size(); ++Index)
    {
      const auto Closed = typeRecord(*FunctionValue.functionType().parameterTypes()[Index]);
      const auto Type = Closed ? Pattern(*Node.parameters()[Index].type()->expression(), *Closed, 0) : std::nullopt;
      if (!Type)
      {
        return false;
      }
      Signature.push_back(*Type);
    }
    (*Declaration)[5] = abi::record('G', Schema);
    (*Declaration)[6] = abi::record('H', {{'A', "n"}, abi::record('L', Signature)});
    (*Parts)[0] = abi::record('R', *Declaration);
    (*Parts)[1] = abi::record('X', Arguments);
    const auto Symbol = abi::mangle(abi::record('F', *Parts));
    if (!Symbol || !Bound.Builder.setFunctionGenericIdentity(*Instance->Function, Symbol.Name, ArgumentTypes))
    {
      Bound.report<core::DiagnosticKind::SemanticInvalidGeneric>(Node.getSourceRange(), "generic signature cannot be represented by the linkage format");
      Instance->State = GenericState::Phase::Failed;
      return false;
    }
    Instance->State = GenericState::Phase::Checking;
    const bool Succeeded = analyzeFunctionBody(Bound, Node, *Instance->Function);
    Instance->State = Succeeded ? GenericState::Phase::Ready : GenericState::Phase::Failed;
    return Succeeded;
  }
} // namespace ink::semantic
