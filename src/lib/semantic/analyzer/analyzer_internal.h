#ifndef INK_LIB_SEMANTIC_ANALYZER_ANALYZER_INTERNAL_H
#define INK_LIB_SEMANTIC_ANALYZER_ANALYZER_INTERNAL_H

#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/ir/ir_builder.h"
#include "ink/semantic/name_resolve/name_resolver.h"
#include "ink/parser/token_cursor.h"

#include "ink/core/config_manager.h"
#include "ink/core/diagnostic.h"
#include "ink/core/source_id.h"

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ink::semantic
{
  std::string describeType(const ir::Type &ValueType);
  const ir::Type *builtinType(ir::TypePool &Types, std::string_view Text);
  std::optional<ir::IntegerBits> integerBits(const parser::TokenBuffer &Input, const parser::LiteralExpr &Literal, bool Negative, const ir::IntegerType &Target);
  bool acceptsCString(const ir::Value &ValueObject, const ir::Type &Target, bool CArgument);
  const parser::LiteralExpr *findDeferredIntegerLiteral(const parser::Expr &Node, std::size_t Depth, std::size_t Limit);
  bool moduleHasOtherDeclaration(const parser::ModuleAST &Root, std::string_view Name);

  // Integer literals keep their source until an expected type or overload is known.
  struct Analyzer::ExpressionResult
  {
      const ir::Value *ValueObject = nullptr;
      const parser::LiteralExpr *IntegerLiteral = nullptr;
      bool Negative = false;
      bool Void = false;
      const ir::Value *TemporaryAddress = nullptr;

      explicit operator bool() const noexcept
      {
        return ValueObject || IntegerLiteral || Void;
      }
  };

  struct Analyzer::AnalysisState
  {
      AnalysisState(SemanticContext &Context, Scope &InitialScope, const parser::TokenBuffer &Input)
          : Context(Context),
            Resolver(InitialScope),
            Builder(Context.irContext()),
            Input(Input),
            Source(Input.lexedFile().sourceId())
      {
      }

      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void report(core::SourceRange Span, ArgumentTypes &&...Arguments) const
      {
        Context.compilationContext().diagnosticEngine().report<Kind>(Source, Span, std::forward<ArgumentTypes>(Arguments)...);
      }

      SemanticContext &Context;
      ir::Module *CurrentModule = nullptr;
      ModuleGraph *Modules = nullptr;
      NameResolver Resolver;
      ir::IRBuilder Builder;
      const parser::TokenBuffer &Input;
      core::SourceId Source;
      const std::size_t TypeDepthLimit = core::ConfigManager::getSize<core::ConfigKind::SemanticTypeDepthLimit>();
      const std::size_t ExpressionDepthLimit = core::ConfigManager::getSize<core::ConfigKind::SemanticExpressionDepthLimit>();
      std::size_t BlockDepth = 0;
      std::vector<std::uint64_t> LexicalScope;
      std::uint64_t NextLexicalBlock = 0;
      ir::Function *CurrentFunction = nullptr;
      const ir::ClassType *CurrentClass = nullptr;
      const ir::ClassType *DeclaringClass = nullptr;
      bool Terminated = false;
      execution::ExecutionFrame *Frame = nullptr;
      bool Evaluating = false;
      bool ComptimeFunction = false;
      bool ClassImportsOnly = false;
      const ir::Type *ExpectedType = nullptr;
      std::size_t LoopDepth = 0;
      bool Breaking = false;
      bool Continuing = false;

      struct InitializationState
      {
          std::unordered_map<ComptimeState::Variable *, bool> Variables;
          std::vector<bool> Fields;
      };

      InitializationState initializationState()
      {
        InitializationState Result;
        for (auto &Entry : Context.comptimeState().Variables)
        {
          if (!Entry.second.Comptime && Entry.second.Function == CurrentFunction)
          {
            Result.Variables.emplace(&Entry.second, Entry.second.Initialized);
          }
        }
        Result.Fields = ConstructorFields;
        return Result;
      }

      void restoreInitialization(const InitializationState &Snapshot)
      {
        for (const auto &Entry : Snapshot.Variables)
        {
          Entry.first->Initialized = Entry.second;
        }
        ConstructorFields = Snapshot.Fields;
      }

      void mergeInitialization(const InitializationState &Incoming, const std::vector<InitializationState> &Paths)
      {
        restoreInitialization(Incoming);
        if (Paths.empty())
        {
          return;
        }
        for (const auto &Entry : Incoming.Variables)
        {
          bool Initialized = true;
          for (const auto &Path : Paths)
          {
            const auto Found = Path.Variables.find(Entry.first);
            Initialized = Initialized && Found != Path.Variables.end() && Found->second;
          }
          Entry.first->Initialized = Initialized;
        }
        for (std::size_t Index = 0; Index < ConstructorFields.size(); ++Index)
        {
          bool Initialized = true;
          for (const auto &Path : Paths)
          {
            Initialized = Initialized && Path.Fields[Index];
          }
          ConstructorFields[Index] = Initialized;
        }
      }

      struct LoopControl
      {
          ir::BasicBlock *BreakTarget = nullptr;
          ir::BasicBlock *ContinueTarget = nullptr;
          std::size_t LifetimeBegin = 0;
          std::vector<InitializationState> Breaks;
          std::vector<InitializationState> Continues;
      };

      std::vector<LoopControl> Loops;

      class LoopGuard final
      {
        public:
          explicit LoopGuard(AnalysisState &State, ir::BasicBlock *BreakTarget = nullptr, ir::BasicBlock *ContinueTarget = nullptr)
              : State(State),
                SavedDepth(State.LoopDepth)
          {
            State.Loops.push_back({BreakTarget, ContinueTarget, State.Lifetimes.size(), {}, {}});
            State.LoopDepth = BreakTarget ? 0 : SavedDepth + 1;
          }

          ~LoopGuard()
          {
            State.Loops.pop_back();
            State.LoopDepth = SavedDepth;
          }

        private:
          AnalysisState &State;
          std::size_t SavedDepth;
      };

      struct ObjectLifetime
      {
          const ir::Value *Address;
          const ir::Type *Type;
          const ir::Value *Initialized;
          const ir::Value *TemporaryValue;
          bool Comptime;
          bool Active = true;
          const ir::Value *Construction = nullptr;
      };

      std::vector<ObjectLifetime> Lifetimes;
      std::vector<bool> ConstructorFields;
      bool Constructing = false;
      bool AccessingConstructorField = false;
      bool Destroying = false;

      class TraversalGuard final
      {
        public:
          explicit TraversalGuard(AnalysisState &State)
              : Engine(State.Context.comptimeState().Engine),
                Status(State.Evaluating ? Engine.enterEvaluation() : execution::ExecutionStatus::Success),
                Active(State.Evaluating && Status == execution::ExecutionStatus::Success)
          {
          }

          ~TraversalGuard()
          {
            if (Active)
            {
              Engine.leaveEvaluation();
            }
          }

          execution::ExecutionStatus status() const noexcept
          {
            return Status;
          }

        private:
          execution::ExecutionEngine &Engine;
          execution::ExecutionStatus Status;
          bool Active;
      };

      class FrameGuard final
      {
        public:
          FrameGuard(AnalysisState &State, execution::ExecutionFrameKind Kind)
              : State(State),
                Saved(State.Frame),
                Entered(State.Context.comptimeState().Engine.createFrame(Kind, Saved))
          {
            State.Frame = Entered;
          }

          ~FrameGuard()
          {
            if (Entered)
            {
              State.Context.comptimeState().Engine.endFrame(*Entered);
            }
            State.Frame = Saved;
          }

          explicit operator bool() const noexcept
          {
            return Entered != nullptr;
          }

        private:
          AnalysisState &State;
          execution::ExecutionFrame *Saved;
          execution::ExecutionFrame *Entered;
      };

      class EvaluationGuard final
      {
        public:
          explicit EvaluationGuard(AnalysisState &State, bool Evaluating = true, const ir::Type *Expected = nullptr)
              : State(State),
                Saved(State.Evaluating),
                SavedType(State.ExpectedType)
          {
            State.Evaluating = Evaluating;
            State.ExpectedType = Expected;
          }

          ~EvaluationGuard()
          {
            State.Evaluating = Saved;
            State.ExpectedType = SavedType;
          }

        private:
          AnalysisState &State;
          bool Saved;
          const ir::Type *SavedType;
      };
  };

  struct Analyzer::ModuleAnalysis
  {
      const ModuleInput *Source = nullptr;
      ir::Module *Owner = nullptr;
      std::unique_ptr<AnalysisState> State;
      Scope *ModuleScope = nullptr;
      execution::ExecutionFrame *ModuleFrame = nullptr;
      const parser::Stmt *ActiveStatement = nullptr;
      std::unordered_map<const parser::FunctionDecl *, ir::Function *> Functions;
      std::unordered_set<const parser::Stmt *> ProcessedStatements;
  };

  struct Analyzer::ModuleGraph
  {
      explicit ModuleGraph(SemanticContext &Context)
          : Dependencies(Context.genericState().Dependencies)
      {
      }
      enum class BodyState
      {
        Pending,
        Analyzing,
        Complete,
        Failed,
      };

      struct FunctionBody
      {
          ModuleAnalysis *Module = nullptr;
          const parser::FunctionDecl *Declaration = nullptr;
          BodyState State = BodyState::Pending;
      };

      std::unordered_map<std::string, ModuleAnalysis *> Modules;
      std::unordered_map<const ir::Function *, FunctionBody> Bodies;
      std::unordered_map<const ir::Function *, std::unordered_set<const ir::Function *>> &Dependencies;
      std::unordered_set<const ir::Function *> ActiveBodies;
      std::size_t LoweringDepth = 0;
  };
} // namespace ink::semantic

#endif
