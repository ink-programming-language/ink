#include "ink/sema/name_resolver.h"

#include "sema_internal.h"

#include <optional>
#include <string>
#include <vector>

namespace ink::sema
{
  namespace
  {
    bool isVisibleAt(const SemanticModel &Model, const Symbol &Symbol, core::SourceRange ReferenceRange)
    {
      if (Symbol.Kind != SymbolKind::Binding)
      {
        return true;
      }
      return Model.astContext().declaration(Symbol.Declaration).Header.Range.End <= ReferenceRange.Start;
    }

    std::vector<SymbolId> lookupVisibleLexical(const SemanticModel &Model, ScopeId Scope, core::InternedStringId Name, core::SourceRange ReferenceRange)
    {
      while (Model.contains(Scope))
      {
        std::vector<SymbolId> Matches;
        for (const SymbolId Id : lookupLocal(Model, Scope, Name))
        {
          if (isVisibleAt(Model, Model.symbol(Id), ReferenceRange))
          {
            Matches.push_back(Id);
          }
        }
        if (!Matches.empty())
        {
          return Matches;
        }
        const std::optional<ScopeId> Parent = Model.scope(Scope).Parent;
        if (!Parent)
        {
          break;
        }
        Scope = *Parent;
      }
      return {};
    }
  } // namespace

  NameResolver::NameResolver(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics) noexcept : Model(Model), Diagnostics(Diagnostics)
  {
  }

  void NameResolver::run()
  {
    for (std::uint32_t Index = Model.astFile().expressions().Begin.value(); Index < Model.astFile().expressions().End.value(); ++Index)
    {
      const ast::AstExprId Id = ast::AstExprId::fromValue(Index);
      const ast::Expression &Node = Model.astContext().expression(Id);
      if (Node.Kind != ast::AstKind::NameExpression)
      {
        continue;
      }
      const ast::NamePayload *Payload = std::get_if<ast::NamePayload>(&Node.Payload);
      if (!Payload || !Model.strings().contains(Payload->Name))
      {
        SemanticModelAccess::setResolvedName(Model, Id, {ResolvedNameStatus::Unresolved, {}});
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnresolvedName, Node.Header.Range);
        continue;
      }
      const ScopeId Scope = Model.expressionScope(Id);
      const std::vector<SymbolId> Matches = lookupVisibleLexical(Model, Scope, Payload->Name, Node.Header.Range);
      if (Matches.empty())
      {
        SemanticModelAccess::setResolvedName(Model, Id, {ResolvedNameStatus::Unresolved, {}});
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::UnresolvedName, Node.Header.Range, std::nullopt, std::string(Model.strings().string(Payload->Name)));
      }
      else if (Matches.size() != 1)
      {
        SemanticModelAccess::setResolvedName(Model, Id, {ResolvedNameStatus::Ambiguous, {}});
        emitDiagnostic(Model, Diagnostics, core::DiagnosticKind::AmbiguousName, Node.Header.Range, std::nullopt, std::string(Model.strings().string(Payload->Name)));
      }
      else
      {
        SemanticModelAccess::setResolvedName(Model, Id, {ResolvedNameStatus::Resolved, Matches.front()});
      }
    }
  }
} // namespace ink::sema
