#include "ink/semantic/analyzer/analyzer_utils.h"

#include "ink/ir/module/global_variable.h"
#include "ink/semantic/context.h"

#include <string>

namespace ink::semantic
{
  const ir::Type *requireType(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target)
  {
    if (!Target)
    {
      return nullptr;
    }
    if (!ir::Type::classof(Target))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Range, "a type", ir::GlobalVariable::classof(Target) ? "variable storage" : ir::valueKindName(Target->kind()));
      return nullptr;
    }
    const auto *Type = static_cast<const ir::Type *>(Target);
    if (!Context.irContext().containsType(Type))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Range, "a type in the current IR context", "a foreign or unregistered type");
      return nullptr;
    }
    return Type;
  }

  const ir::Value *requireRuntimeValue(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target)
  {
    if (!Target)
    {
      return nullptr;
    }
    const ir::Type *Type = Target->type();
    if (!Context.irContext().containsType(Type) || (!Type->hasLayout() && !ir::NullPointerConstant::classof(Target)))
    {
      const auto Actual = ir::Type::classof(Target) ? static_cast<const ir::Type *>(Target)->toString() : std::string(ir::valueKindName(Target->kind()));
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Range, "a runtime value or variable storage", Actual);
      return nullptr;
    }
    return Target;
  }
} // namespace ink::semantic
