#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/analyzer/block_depth_guard.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  namespace
  {
    bool validateResult(AnalyzerState &State, const frontend::parser::Expr &Node, const ir::Value *Value)
    {
      auto &Context = State.semanticContext();
      if (ir::Type::classof(Value))
      {
        return requireType(Context, Node.getSourceRange(), Value) != nullptr;
      }
      if (ir::Module::classof(Value))
      {
        if (&static_cast<const ir::Module *>(Value)->irContext() == &Context.irContext())
        {
          return true;
        }
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a module in the current IR context", "a foreign module");
        return false;
      }
      if (!requireRuntimeValue(Context, Node.getSourceRange(), Value))
      {
        return false;
      }
      if (ir::GlobalVariable::classof(Value))
      {
        return true;
      }
      if (ir::Constant::classof(Value))
      {
        if (Context.irContext().constantPool().owns(static_cast<const ir::Constant *>(Value)))
        {
          return true;
        }
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a constant owned by the current IR context", "a foreign or unregistered constant");
        return false;
      }
      if (ir::Function::classof(Value))
      {
        return true;
      }
      if (ir::Parameter::classof(Value))
      {
        const auto &Parameter = static_cast<const ir::Parameter &>(*Value);
        if (Parameter.function() != State.irBuilder().insertFunction())
        {
          const auto Name = frontend::parser::NameExpr::classof(&Node) ? static_cast<const frontend::parser::NameExpr &>(Node).name().Text : core::NamePool::instance().text(Parameter.name());
          Context.report<core::DiagnosticKind::SemanticInvalidCapture>(Node.getSourceRange(), Name);
          return false;
        }
        if (Parameter.isVariadic())
        {
          Context.report<core::DiagnosticKind::SemanticUnsupported>(Node.getSourceRange(), "variadic parameter values");
          return false;
        }
        return true;
      }
      if (ir::Instruction::classof(Value) && !static_cast<const ir::Instruction *>(Value)->isTerminator())
      {
        if (const ir::Function *Function = State.irBuilder().insertFunction())
        {
          for (const auto &Block : Function->blocks())
          {
            for (const auto &Instruction : Block->instructions())
            {
              if (Instruction.get() == Value)
              {
                return true;
              }
            }
          }
        }
      }
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a function, constant, variable or value in the current function", ir::valueKindName(Value->kind()));
      return false;
    }
  } // namespace

  const ir::Value *Analyzer::analyzeExpr(AnalyzerState &State, const frontend::parser::Expr &Node, ExprHints Hints)
  {
    auto &Context = State.semanticContext();
    if (State.recursionDepth() >= State.recursionDepthLimit())
    {
      Context.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return nullptr;
    }
    BlockDepthGuard DepthGuard(State.recursionDepth());
    if (Node.isComptime())
    {
      reportUnsupported(State, Node);
      return nullptr;
    }
    const ir::Type *ExpectedType = Hints.ExpectedType;
    if (ExpectedType && (!Context.irContext().containsType(ExpectedType) || !ExpectedType->hasLayout()))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "a target type with runtime layout in the current IR context", "a foreign, unregistered or non-runtime type");
      return nullptr;
    }
    if (Hints.DefaultIntegerLiteralType && !Context.irContext().containsType(Hints.DefaultIntegerLiteralType))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "an integer literal default type in the current IR context", "a foreign or unregistered integer type");
      return nullptr;
    }

    const ir::Value *Result = nullptr;
    using namespace frontend::parser;
    switch (Node.getKind())
    {
      case ASTKind::NameExpr:
        Result = analyzeNameExpr(State, static_cast<const NameExpr &>(Node));
        break;
      case ASTKind::LiteralExpr:
        Result = analyzeLiteralExpr(State, static_cast<const LiteralExpr &>(Node), Hints);
        break;
      case ASTKind::ParenExpr:
        Result = analyzeParenExpr(State, static_cast<const ParenExpr &>(Node), Hints);
        break;
      case ASTKind::MemberExpr:
        Result = analyzeMemberExpr(State, static_cast<const MemberExpr &>(Node));
        break;
      case ASTKind::UnaryExpr:
        Result = analyzeUnaryExpr(State, static_cast<const UnaryExpr &>(Node));
        break;
      case ASTKind::ArrayRepeatExpr:
        Result = analyzeArrayRepeatExpr(State, static_cast<const ArrayRepeatExpr &>(Node));
        break;
      case ASTKind::FunctionTypeExpr:
        Result = analyzeFunctionTypeExpr(State, static_cast<const FunctionTypeExpr &>(Node));
        break;
      default:
        // TODO: Restore calls, generic applications, remaining operators and type-valued compile-time evaluation.
        reportUnsupported(State, Node);
        return nullptr;
    }
    if (!Result)
    {
      return nullptr;
    }
    if (!validateResult(State, Node, Result))
    {
      return nullptr;
    }
    if (!ExpectedType)
    {
      return Result;
    }
    if (!requireRuntimeValue(Context, Node.getSourceRange(), Result))
    {
      return nullptr;
    }
    const bool Compatible = ir::Constant::classof(Result) ? static_cast<const ir::Constant *>(Result)->canInitialize(ExpectedType) : Result->type() == ExpectedType;
    if (!Compatible)
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), ExpectedType->toString(), Result->type()->toString());
      return nullptr;
    }
    return Result;
  }
} // namespace ink::semantic
