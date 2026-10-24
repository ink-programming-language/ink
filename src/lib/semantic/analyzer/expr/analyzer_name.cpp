#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/name_expr.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  namespace
  {
    bool isValidBindingValue(AnalyzerState &State, const ir::Value &Value) noexcept
    {
      const auto &Context = State.semanticContext().irContext();
      RETURN_IF(ir::Type::classof(&Value), Context.containsType(static_cast<const ir::Type *>(&Value)));
      RETURN_IF(ir::Module::classof(&Value), &static_cast<const ir::Module &>(Value).irContext() == &Context);
      const ir::Type *Type = Value.type();
      RETURN_IF(!Context.containsType(Type) || (!Type->hasLayout() && !ir::NullPointerConstant::classof(&Value)), false);
      RETURN_IF(ir::Constant::classof(&Value), Context.constantPool().owns(static_cast<const ir::Constant *>(&Value)));
      RETURN_IF(ir::GlobalVariable::classof(&Value) || ir::Function::classof(&Value), true);
      RETURN_IF(ir::Parameter::classof(&Value), static_cast<const ir::Parameter &>(Value).function() == State.irBuilder().insertFunction());
      if (ir::Instruction::classof(&Value))
      {
        const auto &Instruction = static_cast<const ir::Instruction &>(Value);
        const ir::BasicBlock *Parent = Instruction.parentBlock();
        const ir::Function *Function = State.irBuilder().insertFunction();
        return !Instruction.isTerminator() && !ir::StoreInstruction::classof(&Instruction) && Function && Parent && Parent->function() == Function;
      }
      return false;
    }
  } // namespace

  const ir::Value *Analyzer::analyzeBinding(AnalyzerState &State, const AnalysisSite &Site, const Binding &Found, bool IsCallCallee)
  {
    const bool HasSingleTarget = Found.targets().size() == 1;
    // TODO: Select an overload using the call arguments or an expected function signature.
    CHECK_ICE(Site, HasSingleTarget || !IsCallCallee, core::DiagnosticKind::SemanticUnsupported, "function overload selection");
    RETURN_REPORT_IF_FALSE(Site, HasSingleTarget, core::DiagnosticKind::SemanticAmbiguousName, nullptr);
    const ir::Value *Value = Found.targets().front();
    // Bound objects are compiler-produced IR; invalid ownership or storage is an internal model violation.
    CHECK_ICE(Site, Value && isValidBindingValue(State, *Value), core::DiagnosticKind::SemanticConstructionFailed);
    // TODO: Materialize variadic parameter packs before using them as expression values.
    CHECK_ICE(Site, !ir::Parameter::classof(Value) || !static_cast<const ir::Parameter *>(Value)->isVariadic(), core::DiagnosticKind::SemanticUnsupported, "variadic parameter values");
    return Value;
  }

  const ir::Value *Analyzer::analyzeNameExpr(AnalyzerState &State, const frontend::parser::NameExpr &Node, bool IsCallCallee)
  {
    auto &Context = State.semanticContext();
    const auto Name = Node.name();
    if (const Binding *Found = State.nameResolver().lookup(core::NamePool::instance().find(Name.Text)))
    {
      return analyzeBinding(State, State.at(Node), *Found, IsCallCallee);
    }
    if (const ir::Value *Builtin = lookupBuiltinValue(Context, Name.Text))
    {
      return Builtin;
    }
    State.at(Name.Range).report<core::DiagnosticKind::SemanticUnknownName>(Name.Text);
    return nullptr;
  }
} // namespace ink::semantic
