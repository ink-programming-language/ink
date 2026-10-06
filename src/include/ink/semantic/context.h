#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/ir/context.h"
#include "ink/semantic/name_resolve/scope_store.h"
#include "ink/semantic/comptime_state.h"
#include "ink/semantic/class_state.h"
#include "ink/semantic/generic_state.h"
#include "ink/ir/function/function.h"

#include <algorithm>
#include <span>
#include <unordered_map>

namespace ink::semantic
{
  // Combines independent IR storage with frontend scopes and bindings.
  // ScopeStore unregisters its observer before IR storage is destroyed.
  class SemanticContext final
  {
    public:
      FORCE_INLINE explicit SemanticContext(core::CompilationContext &Compilation)
          : IR(Compilation),
            Comptime(IR),
            Generics(IR)
      {
        Scopes.reset(new ScopeStore(*this));
        ImportObserver.reset(new ModuleImportObserver(IR, *this));
      }

      FORCE_INLINE ~SemanticContext() = default;
      SemanticContext(const SemanticContext &) = delete;
      SemanticContext &operator=(const SemanticContext &) = delete;
      SemanticContext(SemanticContext &&) = delete;
      SemanticContext &operator=(SemanticContext &&) = delete;

      FORCE_INLINE ir::IRContext &irContext() noexcept
      {
        return IR;
      }

      FORCE_INLINE const ir::IRContext &irContext() const noexcept
      {
        return IR;
      }

      FORCE_INLINE core::CompilationContext &compilationContext() noexcept
      {
        return IR.compilationContext();
      }

      FORCE_INLINE const core::CompilationContext &compilationContext() const noexcept
      {
        return IR.compilationContext();
      }

      FORCE_INLINE ir::NamePool &namePool() noexcept
      {
        return IR.namePool();
      }

      FORCE_INLINE const ir::NamePool &namePool() const noexcept
      {
        return IR.namePool();
      }

      FORCE_INLINE ScopeStore &scopeStore() noexcept
      {
        return *Scopes;
      }

      FORCE_INLINE const ScopeStore &scopeStore() const noexcept
      {
        return *Scopes;
      }

      FORCE_INLINE ir::TypePool &typePool() noexcept
      {
        return IR.typePool();
      }

      FORCE_INLINE const ir::TypePool &typePool() const noexcept
      {
        return IR.typePool();
      }

      FORCE_INLINE ir::ConstantPool &constantPool() noexcept
      {
        return IR.constantPool();
      }

      FORCE_INLINE const ir::ConstantPool &constantPool() const noexcept
      {
        return IR.constantPool();
      }

      FORCE_INLINE const std::vector<std::unique_ptr<ir::Module>> &modules() const noexcept
      {
        return IR.modules();
      }

      ComptimeState &comptimeState() noexcept
      {
        return Comptime;
      }

      ClassState &classState() noexcept
      {
        return Classes;
      }

      GenericState &genericState() noexcept
      {
        return Generics;
      }

      std::span<const ir::Function *const> moduleImports(const ir::Module &Module) const noexcept
      {
        const auto Found = Imports.find(&Module);
        return Found == Imports.end() ? std::span<const ir::Function *const>{} : std::span<const ir::Function *const>(Found->second);
      }

    private:
      void recordModuleImport(const ir::Module &Module, const ir::Function &Function)
      {
        auto &Functions = Imports[&Module];
        if (std::find(Functions.begin(), Functions.end(), &Function) == Functions.end())
        {
          Functions.push_back(&Function);
        }
      }

      class ModuleImportObserver final : public ir::LifetimeObserver
      {
        public:
          ModuleImportObserver(ir::IRContext &Context, SemanticContext &Owner)
              : ir::LifetimeObserver(Context),
                Owner(Owner)
          {
          }

        private:
          void valueDestroyed(ir::Value &Target) noexcept override
          {
            for (auto Iterator = Owner.Imports.begin(); Iterator != Owner.Imports.end();)
            {
              if (static_cast<const ir::Value *>(Iterator->first) == &Target)
              {
                Iterator = Owner.Imports.erase(Iterator);
              }
              else
              {
                auto &Functions = Iterator->second;
                std::erase_if(Functions, [&Target](const ir::Function *Function)
                {
                  return static_cast<const ir::Value *>(Function) == &Target;
                });
                ++Iterator;
              }
            }
          }

          void declDestroyed(ir::Decl &) noexcept override
          {
          }

          SemanticContext &Owner;
      };

      ir::IRContext IR;
      ComptimeState Comptime;
      ClassState Classes;
      GenericState Generics;
      std::unique_ptr<ScopeStore> Scopes;
      std::unordered_map<const ir::Module *, std::vector<const ir::Function *>> Imports;
      std::unique_ptr<ModuleImportObserver> ImportObserver;

      friend class Analyzer;
  };
} // namespace ink::semantic

#endif
