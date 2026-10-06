#ifndef INK_SEMANTIC_GENERIC_STATE_H
#define INK_SEMANTIC_GENERIC_STATE_H

#include "ink/ir/decl/function_decl.h"
#include "ink/ir/function/function.h"
#include "ink/ir/lifetime_observer.h"
#include "ink/semantic/name_resolve/scope.h"

#include <unordered_map>
#include <unordered_set>

namespace ink::parser
{
  class TokenBuffer;
} // namespace ink::parser

namespace ink::execution
{
  class ExecutionFrame;
} // namespace ink::execution

namespace ink::semantic
{
  // Definitions borrow syntax; substitutions own neither AST nor mutable execution frames.
  class GenericState final
  {
    public:
      enum class Phase
      {
        Signature,
        Pending,
        Checking,
        Ready,
        Failed,
      };

      struct Instance
      {
          std::vector<const ir::Value *> Arguments;
          Scope *Bindings = nullptr;
          std::unique_ptr<ir::Function> Pending;
          ir::Function *Function = nullptr;
          Phase State = Phase::Signature;
      };

      struct Definition
      {
          ir::FunctionDecl *Declaration = nullptr;
          const parser::TokenBuffer *Input = nullptr;
          Scope *ScopeValue = nullptr;
          ir::BasicBlock *Owner = nullptr;
          execution::ExecutionFrame *Frame = nullptr;
          std::vector<std::uint64_t> LexicalScope;
          std::vector<std::unique_ptr<Instance>> Instances;
          bool Local = false;
          bool Capturing = false;
      };

      explicit GenericState(ir::IRContext &Context)
          : Observer(std::make_unique<ObserverImpl>(Context, *this))
      {
      }

      std::unordered_map<const ir::FunctionDecl *, Definition> Definitions;
      std::unordered_map<const ir::Function *, std::unordered_set<const ir::Function *>> Dependencies;
      std::size_t Depth = 0;
      std::size_t InstanceCount = 0;

    private:
      void valueDestroyed(ir::Value &Target) noexcept
      {
        for (auto Iterator = Dependencies.begin(); Iterator != Dependencies.end();)
        {
          if (static_cast<const ir::Value *>(Iterator->first) == &Target)
          {
            Iterator = Dependencies.erase(Iterator);
          }
          else
          {
            std::erase_if(Iterator->second, [&](const ir::Function *Function)
            {
              return static_cast<const ir::Value *>(Function) == &Target;
            });
            ++Iterator;
          }
        }
        for (auto &[Declaration, Definition] : Definitions)
        {
          if (Definition.Owner == &Target)
          {
            Definition.Owner = nullptr;
          }
          for (auto &Instance : Definition.Instances)
          {
            if (Instance->Function == &Target)
            {
              Instance->Function = nullptr;
              Instance->State = Phase::Failed;
            }
          }
        }
      }

      void declDestroyed(ir::Decl &Target) noexcept
      {
        for (auto &[Declaration, Definition] : Definitions)
        {
          if (static_cast<const ir::Decl *>(Declaration) == &Target)
          {
            Definition.Declaration = nullptr;
            Definition.Owner = nullptr;
          }
        }
      }

      class ObserverImpl final : public ir::LifetimeObserver
      {
        public:
          ObserverImpl(ir::IRContext &Context, GenericState &State)
              : ir::LifetimeObserver(Context),
                State(State)
          {
          }

        private:
          void valueDestroyed(ir::Value &Target) noexcept override
          {
            State.valueDestroyed(Target);
          }

          void declDestroyed(ir::Decl &Target) noexcept override
          {
            State.declDestroyed(Target);
          }

          GenericState &State;
      };

      // Unsubscribe before destroying pending signature owners.
      std::unique_ptr<ObserverImpl> Observer;
  };
} // namespace ink::semantic

#endif
