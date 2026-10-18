#include "ink/semantic/analyzer/analyzer_utils.h"

#include "ink/ir/module/global_variable.h"
#include "ink/semantic/context.h"

#include <string>

namespace ink::semantic
{
  const ir::Type *requireType(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target)
  {
    RETURN_IF_NULL(Target, nullptr);
    RETURN_REPORT_IF_FALSE(Context, ir::Type::classof(Target), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Range, "a type", ir::GlobalVariable::classof(Target) ? "variable storage" : ir::valueKindName(Target->kind()));
    const auto *Type = static_cast<const ir::Type *>(Target);
    RETURN_REPORT_IF_FALSE(Context, Context.irContext().containsType(Type), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Range, "a type in the current IR context", "a foreign or unregistered type");
    return Type;
  }

  const ir::Value *requireRuntimeValue(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target)
  {
    RETURN_IF_NULL(Target, nullptr);
    const ir::Type *Type = Target->type();
    RETURN_REPORT_IF(Context, !Context.irContext().containsType(Type) || (!Type->hasLayout() && !ir::NullPointerConstant::classof(Target)), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Range, "a runtime value or variable storage", ir::Type::classof(Target) ? static_cast<const ir::Type *>(Target)->toString() : std::string(ir::valueKindName(Target->kind())));
    return Target;
  }
} // namespace ink::semantic
