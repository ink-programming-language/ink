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
    const auto Site = State.at(Node);
    const ir::Value *Value = analyzeExpr(State, *Node.value());
    RETURN_IF_NOT(Value, nullptr);
    auto &Context = State.semanticContext();
    const auto ElementSite = State.at(*Node.value());
    RETURN_ERROR_IF(ElementSite, ir::Module::classof(Value), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "an array element type or runtime value", ir::valueKindName(Value->kind()));
    // TODO: Construct repeated runtime arrays after value array analysis is supported.
    CHECK_ICE(Site, ir::Type::classof(Value), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    const auto *Element = static_cast<const ir::Type *>(Value);
    CHECK_HAS_LAYOUT(ElementSite, Element, "an array element with runtime layout", nullptr);
    const ir::Value *CountValue = analyzeExpr(State, *Node.count(), {.DefaultIntegerLiteralType = Context.typePool().getUInt64Type()});
    RETURN_IF_NOT(CountValue, nullptr);
    const auto CountSite = State.at(*Node.count());
    RETURN_ERROR_IF_NOT(CountSite, !CountValue->isStorage() && ir::IntegerConstant::classof(CountValue), core::DiagnosticKind::SemanticInvalidArrayLength, nullptr);
    const auto Count = static_cast<const ir::IntegerConstant &>(*CountValue).tryGetNonNegativeUInt64();
    RETURN_ERROR_IF_NOT(CountSite, Count, core::DiagnosticKind::SemanticInvalidArrayLength, nullptr);
    const ir::Type *Result = Context.typePool().getArrayType(Element, *Count);
    RETURN_ERROR_IF_NOT(Site, Result, core::DiagnosticKind::SemanticTypeMismatch, nullptr, "an array with representable size and valid alignment", "an invalid array layout");
    return Result;
  }
} // namespace ink::semantic
