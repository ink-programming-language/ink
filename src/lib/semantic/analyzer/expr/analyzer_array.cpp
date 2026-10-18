#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/array_repeat_expr.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  ExprResult Analyzer::analyzeArrayRepeatExpr(AnalyzerState &State, const frontend::parser::ArrayRepeatExpr &Node)
  {
    const ExprResult Value = analyzeExpr(State, *Node.value());
    RETURN_IF(!Value, ExprResult{});
    auto &Context = State.semanticContext();
    RETURN_REPORT_IF(Context, Value.Kind == ExprResultKind::Module, core::DiagnosticKind::SemanticTypeMismatch, ExprResult{}, Node.value()->getSourceRange(), "an array element type or runtime value", ir::valueKindName(Value.Value->kind()));
    // TODO: Construct repeated runtime arrays after value array analysis is supported.
    UNSUPPORT_IF(State, Value.Kind != ExprResultKind::Type, ExprResult{}, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    const auto *Element = static_cast<const ir::Type *>(Value.Value);
    CHECK_HAS_LAYOUT(Context, Node.value()->getSourceRange(), Element, "an array element with runtime layout", ExprResult{});
    const ExprResult CountValue = analyzeExpr(State, *Node.count(), {.DefaultIntegerLiteralType = Context.typePool().getUInt64Type()});
    RETURN_IF(!CountValue, ExprResult{});
    RETURN_REPORT_IF_FALSE(Context, CountValue.Kind == ExprResultKind::RValue && ir::IntegerConstant::classof(CountValue.Value), core::DiagnosticKind::SemanticInvalidArrayLength, ExprResult{}, Node.count()->getSourceRange());
    const auto Count = static_cast<const ir::IntegerConstant &>(*CountValue.Value).tryGetNonNegativeUInt64();
    RETURN_REPORT_IF_FALSE(Context, Count, core::DiagnosticKind::SemanticInvalidArrayLength, ExprResult{}, Node.count()->getSourceRange());
    const ir::Type *Result = Context.typePool().getArrayType(Element, *Count);
    RETURN_REPORT_IF_FALSE(Context, Result, core::DiagnosticKind::SemanticTypeMismatch, ExprResult{}, Node.getSourceRange(), "an array with representable size and valid alignment", "an invalid array layout");
    return ExprResult::fromValue(Result);
  }
} // namespace ink::semantic
