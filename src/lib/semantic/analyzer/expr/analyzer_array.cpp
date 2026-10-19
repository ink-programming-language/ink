#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/array_repeat_expr.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeArrayRepeatExpr(AnalyzerState &State, const frontend::parser::ArrayRepeatExpr &Node)
  {
    const ir::Value *Value = analyzeExpr(State, *Node.value());
    RETURN_IF(!Value, nullptr);
    auto &Context = State.semanticContext();
    RETURN_REPORT_IF(Context, ir::Module::classof(Value), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.value()->getSourceRange(), "an array element type or runtime value", ir::valueKindName(Value->kind()));
    // TODO: Construct repeated runtime arrays after value array analysis is supported.
    UNSUPPORT_IF(State, !ir::Type::classof(Value), nullptr, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    const auto *Element = static_cast<const ir::Type *>(Value);
    CHECK_HAS_LAYOUT(Context, Node.value()->getSourceRange(), Element, "an array element with runtime layout", nullptr);
    const ir::Value *CountValue = analyzeExpr(State, *Node.count(), {.DefaultIntegerLiteralType = Context.typePool().getUInt64Type()});
    RETURN_IF(!CountValue, nullptr);
    RETURN_REPORT_IF_FALSE(Context, !CountValue->isStorage() && ir::IntegerConstant::classof(CountValue), core::DiagnosticKind::SemanticInvalidArrayLength, nullptr, Node.count()->getSourceRange());
    const auto Count = static_cast<const ir::IntegerConstant &>(*CountValue).tryGetNonNegativeUInt64();
    RETURN_REPORT_IF_FALSE(Context, Count, core::DiagnosticKind::SemanticInvalidArrayLength, nullptr, Node.count()->getSourceRange());
    const ir::Type *Result = Context.typePool().getArrayType(Element, *Count);
    RETURN_REPORT_IF_FALSE(Context, Result, core::DiagnosticKind::SemanticTypeMismatch, nullptr, Node.getSourceRange(), "an array with representable size and valid alignment", "an invalid array layout");
    return Result;
  }
} // namespace ink::semantic
