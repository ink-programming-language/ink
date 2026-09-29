#ifndef INK_SEMANTIC_NAME_RESOLVE_NAME_RESOLVER_H
#define INK_SEMANTIC_NAME_RESOLVE_NAME_RESOLVER_H

#include "ink/semantic/name_resolve/scope_store.h"

namespace ink::semantic
{
  class SemanticContext;

  // Borrows context-owned scopes and bindings, retaining only an independent current scope.
  // Context must outlive the resolver. Names use its name pool; compact ir::Name indices cannot prove provenance.
  class NameResolver final
  {
    public:
      enum class BindResult
      {
        Inserted,
        AlreadyBound,
        Conflict,
        InvalidName,
        ForeignValue,
        ForeignDecl,
        InvalidDecl,
      };

      // Enters a lexical/member scope and restores the original scope on destruction.
      // The resolver and its context must outlive the guard; scopes and bindings remain stored.
      class ScopeGuard final
      {
        public:
          explicit ScopeGuard(NameResolver &Resolver);
          ScopeGuard(NameResolver &Resolver, ir::Value &Owner);
          ~ScopeGuard() noexcept;
          ScopeGuard(const ScopeGuard &) = delete;
          ScopeGuard &operator=(const ScopeGuard &) = delete;
          ScopeGuard(ScopeGuard &&) = delete;
          ScopeGuard &operator=(ScopeGuard &&) = delete;

          // Null when member scope creation failed; a failed guard does not change resolver state.
          Scope *scope() const noexcept
          {
            return EnteredScope;
          }

        private:
          NameResolver &Resolver;
          Scope &SavedScope;
          Scope *EnteredScope;
      };

      explicit NameResolver(SemanticContext &Context) noexcept;
      // Resumes lookup in an existing scope, using that scope's store and context.
      explicit NameResolver(Scope &InitialScope) noexcept;
      NameResolver(const NameResolver &) = delete;
      NameResolver &operator=(const NameResolver &) = delete;
      NameResolver(NameResolver &&) = delete;
      NameResolver &operator=(NameResolver &&) = delete;

      Scope &rootScope() noexcept
      {
        return Store.rootScope();
      }

      const Scope &rootScope() const noexcept
      {
        return Store.rootScope();
      }

      Scope &currentScope() noexcept
      {
        return *CurrentScope;
      }

      const Scope &currentScope() const noexcept
      {
        return *CurrentScope;
      }

      // Creates and enters a child of the current scope.
      // Scopes survive resolver destruction. Bindings remain until their last target is destroyed.
      Scope &enterScope();

      // Creates and enters Owner's member scope under the current scope.
      // Foreign owners or owners already associated with a scope return null without changing state.
      Scope *enterScope(ir::Value &Owner);

      // Restores the parent without destroying the exited scope; false at the root.
      bool exitScope() noexcept;

      // Binds in the current scope. Names may alias their values.
      // Only Function values and generic FunctionDecls may share a name. Rebinding the same target is a
      // no-op; signature legality and overload selection belong to later analysis.
      BindResult bind(ir::Name BoundName, ir::Value &ValueObject);
      // Accepts only local generic FunctionDecl/ClassDecl definitions, not ModuleDecl.
      // The first successful binding records the definition scope; later aliases preserve it.
      BindResult bind(ir::Name BoundName, ir::Decl &Declaration);

      Scope *definitionScope(const ir::Decl &Declaration) noexcept;
      const Scope *definitionScope(const ir::Decl &Declaration) const noexcept;

      // Searches the current scope and its parents; lookupLocal searches only the current scope.
      // The nearest scope containing either target category hides both outer categories.
      // Select ir::Decl * explicitly for generic candidates; the default preserves value lookup.
      // Mixed function overloads are retrieved as two typed lists from that same scope.
      // Invalid names and misses return null without interning.
      template <BindingTarget T = ir::Value *>
      const Binding<T> *lookup(ir::Name BoundName) const noexcept
      {
        const Scope *Found = findScope(BoundName);
        return Found ? lookupInScope<T>(*Found, BoundName) : nullptr;
      }

      template <BindingTarget T = ir::Value *>
      const Binding<T> *lookupLocal(ir::Name BoundName) const noexcept
      {
        return lookupInScope<T>(*CurrentScope, BoundName);
      }

      // Searches only Owner's member scope, without changing the current scope or searching lexical parents.
      // Missing scopes, invalid names and misses return null; aliases of the same owner share one scope.
      template <BindingTarget T = ir::Value *>
      const Binding<T> *lookupMember(ir::Value &Owner, ir::Name MemberName) const noexcept
      {
        const Scope *Members = Store.memberScope(Owner);
        return Members ? lookupInScope<T>(*Members, MemberName) : nullptr;
      }

    private:
      const Scope *findScope(ir::Name BoundName) const noexcept;

      template <BindingTarget T>
      static BindResult bindInTable(BindingTable<T> &Bindings, ir::Name BoundName, T Target, bool OverloadSet);

      template <BindingTarget T>
      static const Binding<T> *lookupInTable(const BindingTable<T> &Bindings, ir::Name BoundName) noexcept
      {
        const auto Found = Bindings.find(BoundName);
        return Found == Bindings.end() ? nullptr : &Found->second;
      }

      template <BindingTarget T>
      static const Binding<T> *lookupInScope(const Scope &ScopeValue, ir::Name BoundName) noexcept
      {
        if constexpr (std::is_same_v<T, ir::Value *>)
        {
          return lookupInTable(ScopeValue.ValueBindings, BoundName);
        }
        else
        {
          return lookupInTable(ScopeValue.DeclBindings, BoundName);
        }
      }

      ScopeStore &Store;
      Scope *CurrentScope;
  };
} // namespace ink::semantic

#endif
