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
#include <utility>

namespace ink::semantic
{
  std::string describeType(const ir::Type &ValueType);
  std::optional<ir::IntegerBits> integerBits(const parser::TokenBuffer &Input, const parser::LiteralExpr &Literal, bool Negative, const ir::IntegerType &Target);
  bool acceptsCString(const ir::Value &ValueObject, const ir::Type &Target, bool CArgument);

  // Integer literals keep their source until an expected type or overload is known.
  struct Analyzer::ExpressionResult
  {
      const ir::Value *ValueObject = nullptr;
      const parser::LiteralExpr *IntegerLiteral = nullptr;
      bool Negative = false;
      bool Void = false;

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
      NameResolver Resolver;
      ir::IRBuilder Builder;
      const parser::TokenBuffer &Input;
      core::SourceId Source;
      const std::size_t TypeDepthLimit = core::ConfigManager::getSize<core::ConfigKind::SemanticTypeDepthLimit>();
      const std::size_t ExpressionDepthLimit = core::ConfigManager::getSize<core::ConfigKind::SemanticExpressionDepthLimit>();
      std::size_t BlockDepth = 0;
      ir::Function *CurrentFunction = nullptr;
      bool Terminated = false;
      execution::ExecutionFrame *Frame = nullptr;
      bool Evaluating = false;
      bool ComptimeFunction = false;
      const ir::Type *ExpectedType = nullptr;
      std::size_t LoopDepth = 0;
      bool Breaking = false;
      bool Continuing = false;

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
} // namespace ink::semantic

#endif
