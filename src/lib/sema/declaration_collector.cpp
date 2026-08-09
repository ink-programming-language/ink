#include "ink/sema/declaration_collector.h"

#include "sema_internal.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ink::sema
{
  namespace
  {
    struct WorkItem
    {
      ast::AstNodeRef Ref;
      ScopeId Scope;
    };

    std::optional<core::InternedStringId> declarationName(const SemanticModel &Model, ast::AstDeclId Id)
    {
      const ast::Declaration &Node = Model.astContext().declaration(Id);
      if (const ast::FunctionPayload *Payload = std::get_if<ast::FunctionPayload>(&Node.Payload))
      {
        return Payload->Name;
      }
      if (const ast::ParameterPayload *Payload = std::get_if<ast::ParameterPayload>(&Node.Payload))
      {
        return Payload->Name;
      }
      if (const ast::BindingPayload *Payload = std::get_if<ast::BindingPayload>(&Node.Payload))
      {
        const ast::Pattern &Pattern = Model.astContext().pattern(Payload->Pattern);
        if (const ast::BindingPatternPayload *PatternPayload = std::get_if<ast::BindingPatternPayload>(&Pattern.Payload))
        {
          return PatternPayload->Name;
        }
      }
      return std::nullopt;
    }

    std::optional<SymbolKind> declarationSymbolKind(const ast::Declaration &Node)
    {
      if (std::holds_alternative<ast::FunctionPayload>(Node.Payload))
      {
        return SymbolKind::Function;
      }
      if (std::holds_alternative<ast::ParameterPayload>(Node.Payload))
      {
        return SymbolKind::Parameter;
      }
      if (std::holds_alternative<ast::BindingPayload>(Node.Payload))
      {
        return SymbolKind::Binding;
      }
      return std::nullopt;
    }

    bool declarationMutable(const ast::Declaration &Node)
    {
      const ast::BindingPayload *Payload = std::get_if<ast::BindingPayload>(&Node.Payload);
      return Payload != nullptr && Payload->Mode == ast::AstBindingMode::Var;
    }

    void reportUnsupported(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics, ast::AstNodeRef Ref)
    {
      if (!SemanticModelAccess::markUnsupported(Model, Ref))
      {
        return;
      }
      const ast::AstNodeView View = Model.astContext().node(Ref);
      std::string Feature = ast::astKindName(View.Kind);
      if (Ref.Category == ast::AstNodeCategory::Declaration)
      {
        const ast::Declaration &Node = Model.astContext().declaration(ast::AstDeclId::fromValue(Ref.Index));
        if (const ast::UnsupportedPayload *Payload = std::get_if<ast::UnsupportedPayload>(&Node.Payload))
        {
          Feature = ast::unsupportedFeatureName(Payload->Feature);
        }
      }
      else if (Ref.Category == ast::AstNodeCategory::Pattern)
      {
        const ast::Pattern &Node = Model.astContext().pattern(ast::AstPatternId::fromValue(Ref.Index));
        if (const ast::UnsupportedPayload *Payload = std::get_if<ast::UnsupportedPayload>(&Node.Payload))
        {
          Feature = ast::unsupportedFeatureName(Payload->Feature);
        }
      }
      emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnsupportedSemanticFeature, View.Header->Range, std::nullopt, std::move(Feature));
    }

    void declareOne(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics, ast::AstDeclId Id, ScopeId Scope)
    {
      SemanticModelAccess::setDeclarationScope(Model, Id, Scope);
      if (Model.declarationSymbol(Id))
      {
        return;
      }
      const ast::Declaration &Node = Model.astContext().declaration(Id);
      const std::optional<SymbolKind> Kind = declarationSymbolKind(Node);
      const std::optional<core::InternedStringId> Name = declarationName(Model, Id);
      if (!Kind || !Name || !Model.strings().contains(*Name))
      {
        return;
      }
      const std::vector<SymbolId> Existing = lookupLocal(Model, Scope, *Name);
      if (!Existing.empty())
      {
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::RedefinedName, Node.Header.Range, std::nullopt, std::string(Model.strings().string(*Name)), Model.symbol(Existing.front()).Declaration);
      }
      const SymbolId Symbol = SemanticModelAccess::addSymbol(Model, *Kind, *Name, Id, Scope, declarationMutable(Node));
      if (const ast::BindingPayload *Payload = std::get_if<ast::BindingPayload>(&Node.Payload))
      {
        SemanticModelAccess::setPatternSymbol(Model, Payload->Pattern, Symbol);
      }
    }

    void predeclareList(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics, ast::AstNodeList List, ScopeId Scope)
    {
      for (const ast::AstNodeRef Ref : Model.astContext().list(List))
      {
        if (Ref.Category == ast::AstNodeCategory::Declaration)
        {
          declareOne(Model, Diagnostics, ast::AstDeclId::fromValue(Ref.Index), Scope);
        }
      }
    }

    void pushReverse(std::vector<WorkItem> &Work, const std::vector<ast::AstNodeRef> &Children, ScopeId Scope)
    {
      for (auto Iterator = Children.rbegin(); Iterator != Children.rend(); ++Iterator)
      {
        Work.push_back({*Iterator, Scope});
      }
    }
  } // namespace

  DeclarationCollector::DeclarationCollector(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) noexcept : Model(Model), Diagnostics(Diagnostics)
  {
  }

  void DeclarationCollector::run()
  {
    const ast::AstDeclId Root = Model.astFile().root();
    const ScopeId Global = SemanticModelAccess::addScope(Model, std::nullopt, ast::AstNodeRef::declaration(Root));
    SemanticModelAccess::setGlobalScope(Model, Global);
    SemanticModelAccess::setDeclarationOwnedScope(Model, Root, Global);
    std::vector<WorkItem> Work;
    Work.push_back({ast::AstNodeRef::declaration(Root), Global});
    while (!Work.empty())
    {
      const WorkItem Current = Work.back();
      Work.pop_back();
      const ast::AstNodeView View = Model.astContext().node(Current.Ref);
      if (Current.Ref.Category == ast::AstNodeCategory::Declaration)
      {
        const ast::AstDeclId Id = ast::AstDeclId::fromValue(Current.Ref.Index);
        const ast::Declaration &Node = Model.astContext().declaration(Id);
        SemanticModelAccess::setDeclarationScope(Model, Id, Current.Scope);
        if (Node.Kind == ast::AstKind::UnsupportedDeclaration || Node.Kind == ast::AstKind::ImportDeclaration)
        {
          reportUnsupported(Model, Diagnostics, Current.Ref);
        }
        if (const ast::SourceFilePayload *Payload = std::get_if<ast::SourceFilePayload>(&Node.Payload))
        {
          predeclareList(Model, Diagnostics, Payload->Items, Current.Scope);
          pushReverse(Work, directChildren(Model, Current.Ref), Current.Scope);
          continue;
        }
        if (const ast::FunctionPayload *Payload = std::get_if<ast::FunctionPayload>(&Node.Payload))
        {
          declareOne(Model, Diagnostics, Id, Current.Scope);
          const ScopeId FunctionScope = SemanticModelAccess::addScope(Model, Current.Scope, Current.Ref);
          SemanticModelAccess::setDeclarationOwnedScope(Model, Id, FunctionScope);
          predeclareList(Model, Diagnostics, Payload->Parameters, FunctionScope);
          std::vector<ast::AstNodeRef> OuterChildren;
          appendNodeList(Model.astContext(), Node.Header.Supplemental, OuterChildren);
          pushReverse(Work, OuterChildren, Current.Scope);
          std::vector<ast::AstNodeRef> FunctionChildren;
          appendNodeList(Model.astContext(), Payload->Parameters, FunctionChildren);
          if (Payload->ResultType)
          {
            FunctionChildren.push_back(ast::AstNodeRef::expression(*Payload->ResultType));
          }
          if (Payload->Body)
          {
            FunctionChildren.push_back(ast::AstNodeRef::statement(*Payload->Body));
          }
          pushReverse(Work, FunctionChildren, FunctionScope);
          continue;
        }
        declareOne(Model, Diagnostics, Id, Current.Scope);
        pushReverse(Work, directChildren(Model, Current.Ref), Current.Scope);
        continue;
      }
      if (Current.Ref.Category == ast::AstNodeCategory::Statement)
      {
        const ast::AstStmtId Id = ast::AstStmtId::fromValue(Current.Ref.Index);
        const ast::Statement &Node = Model.astContext().statement(Id);
        if (const ast::BlockPayload *Payload = std::get_if<ast::BlockPayload>(&Node.Payload))
        {
          const ScopeId BlockScope = SemanticModelAccess::addScope(Model, Current.Scope, Current.Ref);
          SemanticModelAccess::setStatementScope(Model, Id, BlockScope);
          predeclareList(Model, Diagnostics, Payload->Items, BlockScope);
          pushReverse(Work, directChildren(Model, Current.Ref), BlockScope);
        }
        else
        {
          SemanticModelAccess::setStatementScope(Model, Id, Current.Scope);
          pushReverse(Work, directChildren(Model, Current.Ref), Current.Scope);
        }
        continue;
      }
      if (Current.Ref.Category == ast::AstNodeCategory::Expression)
      {
        SemanticModelAccess::setExpressionScope(Model, ast::AstExprId::fromValue(Current.Ref.Index), Current.Scope);
        pushReverse(Work, directChildren(Model, Current.Ref), Current.Scope);
        continue;
      }
      if (Current.Ref.Category == ast::AstNodeCategory::Pattern)
      {
        const ast::AstPatternId Id = ast::AstPatternId::fromValue(Current.Ref.Index);
        SemanticModelAccess::setPatternScope(Model, Id, Current.Scope);
        if (View.Kind == ast::AstKind::UnsupportedPattern || View.Kind == ast::AstKind::TuplePattern || View.Kind == ast::AstKind::VariantPattern)
        {
          reportUnsupported(Model, Diagnostics, Current.Ref);
        }
        pushReverse(Work, directChildren(Model, Current.Ref), Current.Scope);
      }
    }
  }
} // namespace ink::sema
