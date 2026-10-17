#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/array_repeat_expr.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeArrayRepeatExpr(AnalyzerState &State, const frontend::parser::ArrayRepeatExpr &Node)
  {
    const ir::Value *Value = analyzeExpr(State, *Node.value());
    if (!Value)
    {
      return nullptr;
    }
    auto &Context = State.semanticContext();
    if (ir::Module::classof(Value))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.value()->getSourceRange(), "an array element type or runtime value", ir::valueKindName(Value->kind()));
      return nullptr;
    }
    if (!ir::Type::classof(Value))
    {
      // TODO: Construct repeated runtime arrays after value array analysis is supported.
      reportUnsupported(State, Node);
      return nullptr;
    }
    const auto *Element = static_cast<const ir::Type *>(Value);
    if (!Element->hasLayout())
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.value()->getSourceRange(), "an array element with runtime layout", Element->toString());
      return nullptr;
    }
    const ir::Value *CountValue = analyzeExpr(State, *Node.count(), {.DefaultIntegerLiteralType = Context.typePool().getUInt64Type()});
    if (!CountValue)
    {
      return nullptr;
    }
    if (!ir::IntegerConstant::classof(CountValue))
    {
      Context.report<core::DiagnosticKind::SemanticInvalidArrayLength>(Node.count()->getSourceRange());
      return nullptr;
    }
    const auto Count = static_cast<const ir::IntegerConstant &>(*CountValue).tryGetNonNegativeUInt64();
    if (!Count)
    {
      Context.report<core::DiagnosticKind::SemanticInvalidArrayLength>(Node.count()->getSourceRange());
      return nullptr;
    }
    const ir::Type *Result = Context.typePool().getArrayType(Element, *Count);
    if (!Result)
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "an array with representable size and valid alignment", "an invalid array layout");
      return nullptr;
    }
    return Result;
  }
} // namespace ink::semantic
