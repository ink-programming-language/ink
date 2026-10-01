#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

#include <algorithm>
#include <utility>
#include <vector>

namespace ink::semantic
{
  using namespace ink::ir;

  Analyzer::ExpressionResult Analyzer::analyzeCallExpr(AnalysisState &State, const parser::CallExpr &Node, std::size_t Depth)
  {
    if (Node.optional())
    {
      reportUnsupported(State, Node);
      return {};
    }
    const parser::Expr *CalleeNode = Node.callee();
    std::size_t CalleeDepth = Depth + 1;
    while ((State.Evaluating || !CalleeNode->isComptime()) && parser::ParenExpr::classof(CalleeNode) && CalleeDepth < State.ExpressionDepthLimit)
    {
      CalleeNode = static_cast<const parser::ParenExpr &>(*CalleeNode).expression();
      ++CalleeDepth;
    }
    if (CalleeDepth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(CalleeNode->getSourceRange());
      return {};
    }
    std::vector<const Value *> Candidates;
    if ((State.Evaluating || !CalleeNode->isComptime()) && parser::NameExpr::classof(CalleeNode))
    {
      const Name Symbol = State.Context.namePool().find(static_cast<const parser::NameExpr &>(*CalleeNode).name().Text);
      if (const auto *Binding = State.Resolver.lookup(Symbol); Binding && Binding->targets().size() > 1)
      {
        Candidates.assign(Binding->targets().begin(), Binding->targets().end());
      }
    }
    if (Candidates.empty())
    {
      const ExpressionResult Callee = analyzeExpr(State, *CalleeNode, CalleeDepth);
      if (!Callee)
      {
        return {};
      }
      if (!Callee.ValueObject || !FunctionType::classof(&Callee.ValueObject->type()))
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(CalleeNode->getSourceRange(), "callable value", Callee.IntegerLiteral ? "integer literal" : (Callee.ValueObject ? describeType(Callee.ValueObject->type()) : "void"));
        return {};
      }
      Candidates.push_back(Callee.ValueObject);
    }

    std::vector<ExpressionResult> Arguments;
    bool Succeeded = true;
    for (const auto &Argument : Node.arguments())
    {
      if (Argument.form() != parser::ArgumentKind::Positional)
      {
        State.report<core::DiagnosticKind::SemanticUnsupported>(Argument.range(), "named or spread call arguments");
        return {};
      }
      const auto CandidateParameters = static_cast<const FunctionType &>(Candidates.front()->type()).parameterTypes();
      const Type *Expected = Candidates.size() == 1 && Arguments.size() < CandidateParameters.size() ? CandidateParameters[Arguments.size()] : nullptr;
      // Preserve unconstrained integer literals until overload selection. Preparing
      // a literal has no side effects and must not prematurely force it to i32.
      const parser::Expr *LiteralNode = Argument.value();
      bool Negative = false;
      std::size_t LiteralDepth = Depth + 1;
      while (!LiteralNode->isComptime() && LiteralDepth < State.ExpressionDepthLimit)
      {
        if (parser::ParenExpr::classof(LiteralNode))
        {
          LiteralNode = static_cast<const parser::ParenExpr *>(LiteralNode)->expression();
        }
        else if (parser::UnaryExpr::classof(LiteralNode) && (static_cast<const parser::UnaryExpr *>(LiteralNode)->op() == tokenizer::TokenKind::Plus || static_cast<const parser::UnaryExpr *>(LiteralNode)->op() == tokenizer::TokenKind::Minus))
        {
          const auto &Unary = static_cast<const parser::UnaryExpr &>(*LiteralNode);
          Negative = Negative != (Unary.op() == tokenizer::TokenKind::Minus);
          LiteralNode = Unary.operand();
        }
        else
        {
          break;
        }
        ++LiteralDepth;
      }
      if (State.Evaluating && !LiteralNode->isComptime() && LiteralDepth < State.ExpressionDepthLimit && parser::LiteralExpr::classof(LiteralNode) && static_cast<const parser::LiteralExpr *>(LiteralNode)->literalKind() == tokenizer::TokenKind::IntegerLiteral)
      {
        Arguments.push_back({nullptr, static_cast<const parser::LiteralExpr *>(LiteralNode), Negative});
        continue;
      }
      AnalysisState::EvaluationGuard Guard(State, State.Evaluating, Expected);
      Arguments.push_back(analyzeExpr(State, *Argument.value(), Depth + 1));
      Succeeded = static_cast<bool>(Arguments.back()) && Succeeded;
    }
    if (!Succeeded)
    {
      return {};
    }

    const auto IsCFunction = [](const Value &Callee)
    {
      return Function::classof(&Callee) && static_cast<const Function &>(Callee).languageLinkage() == LanguageLinkage::C;
    };
    const Value *Selected = Candidates.front();
    if (Candidates.size() > 1)
    {
      struct CandidateMatch
      {
          const Value *Callee;
          std::vector<unsigned> Ranks;
      };
      std::vector<CandidateMatch> Matches;
      for (const Value *Candidate : Candidates)
      {
        const auto Parameters = static_cast<const FunctionType &>(Candidate->type()).parameterTypes();
        if (Parameters.size() != Arguments.size())
        {
          continue;
        }
        CandidateMatch Match{Candidate, {}};
        for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
        {
          const ExpressionResult &Argument = Arguments[Index];
          const Type &Parameter = *Parameters[Index];
          if (Argument.IntegerLiteral && IntegerType::classof(&Parameter))
          {
            const auto &Integer = static_cast<const IntegerType &>(Parameter);
            if (!integerBits(State.Input, *Argument.IntegerLiteral, Argument.Negative, Integer))
            {
              break;
            }
            Match.Ranks.push_back(Integer.isSigned() && Integer.bitWidth() == 32 ? 0U : 1U);
          }
          else if (Argument.ValueObject && &Argument.ValueObject->type() == &Parameter)
          {
            Match.Ranks.push_back(0);
          }
          else if (Argument.ValueObject && acceptsCString(*Argument.ValueObject, Parameter, IsCFunction(*Candidate)))
          {
            Match.Ranks.push_back(1);
          }
          else
          {
            break;
          }
        }
        if (Match.Ranks.size() == Arguments.size())
        {
          Matches.push_back(std::move(Match));
        }
      }
      if (Matches.empty())
      {
        State.report<core::DiagnosticKind::SemanticNoMatchingOverload>(Node.getSourceRange());
        return {};
      }
      // A candidate must be no worse for every argument and better for at least one.
      Selected = nullptr;
      for (const CandidateMatch &Match : Matches)
      {
        const bool Dominated = std::any_of(Matches.begin(), Matches.end(), [&Match](const CandidateMatch &Other)
        {
          bool Better = false;
          for (std::size_t Index = 0; Index < Match.Ranks.size(); ++Index)
          {
            if (Other.Ranks[Index] > Match.Ranks[Index])
            {
              return false;
            }
            Better = Better || Other.Ranks[Index] < Match.Ranks[Index];
          }
          return Better;
        });
        if (!Dominated)
        {
          if (Selected)
          {
            State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
            return {};
          }
          Selected = Match.Callee;
        }
      }
    }
    const auto Parameters = static_cast<const FunctionType &>(Selected->type()).parameterTypes();
    if (Parameters.size() != Arguments.size())
    {
      State.report<core::DiagnosticKind::SemanticArgumentCount>(Node.getSourceRange(), Parameters.size(), Arguments.size());
      return {};
    }
    if (!State.Evaluating && !State.ComptimeFunction && Function::classof(Selected))
    {
      const auto &Functions = State.Context.comptimeState().Functions;
      const auto Definition = Functions.find(Selected);
      if (Definition != Functions.end() && Definition->second.Comptime)
      {
        State.report<core::DiagnosticKind::SemanticComptimeFunctionAtRuntime>(Node.getSourceRange());
        return {};
      }
    }
    std::vector<const Value *> Converted;
    if (State.Evaluating && !Function::classof(Selected))
    {
      reportExecution(State, execution::ExecutionStatus::UnsupportedOperation, Node);
      return {};
    }
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      const Value *Argument = convertExpression(State, Arguments[Index], *Parameters[Index], *Node.arguments()[Index].value(), IsCFunction(*Selected));
      if (!Argument)
      {
        return {};
      }
      Converted.push_back(Argument);
    }
    if (State.Evaluating)
    {
      if (!Function::classof(Selected))
      {
        reportExecution(State, execution::ExecutionStatus::UnsupportedOperation, Node);
        return {};
      }
      return callComptime(State, static_cast<const Function &>(*Selected), Converted, Node);
    }
    const Value *Result = State.Builder.createCallInstruction(*Selected, Converted);
    if (!Result)
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
    }
    return {Result};
  }
} // namespace ink::semantic
