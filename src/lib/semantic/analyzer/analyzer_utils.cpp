#include "ink/semantic/analyzer/analyzer_utils.h"

#include "ink/ir/module/global_variable.h"
#include "ink/semantic/context.h"

#include <string>

namespace ink::semantic
{
  namespace
  {
    template <auto Getter>
    const ir::Value *getBuiltinType(SemanticContext &Context) noexcept
    {
      return (Context.typePool().*Getter)();
    }

    template <bool Value>
    const ir::Value *getBuiltinBool(SemanticContext &Context) noexcept
    {
      return Context.irContext().constantPool().getBoolConstant(Value);
    }
  } // namespace

  const ir::Value *lookupBuiltinValue(SemanticContext &Context, std::string_view Name)
  {
    struct Entry
    {
        std::string_view Name;
        const ir::Value *(*GetValue)(SemanticContext &) noexcept;
    };
    // TODO: Extend numeric builtins with i128/u128/f16 once TypePool supports them.
    static constexpr Entry Entries[] = {
        {"type", getBuiltinType<&ir::TypePool::getMetaType>},
        {"void", getBuiltinType<&ir::TypePool::getVoidType>},
        {"bool", getBuiltinType<&ir::TypePool::getBoolType>},
        {"i8", getBuiltinType<&ir::TypePool::getInt8Type>},
        {"u8", getBuiltinType<&ir::TypePool::getUInt8Type>},
        {"i16", getBuiltinType<&ir::TypePool::getInt16Type>},
        {"u16", getBuiltinType<&ir::TypePool::getUInt16Type>},
        {"i32", getBuiltinType<&ir::TypePool::getInt32Type>},
        {"u32", getBuiltinType<&ir::TypePool::getUInt32Type>},
        {"i64", getBuiltinType<&ir::TypePool::getInt64Type>},
        {"u64", getBuiltinType<&ir::TypePool::getUInt64Type>},
        {"f32", getBuiltinType<&ir::TypePool::getFloat32Type>},
        {"f64", getBuiltinType<&ir::TypePool::getFloat64Type>},
        {"true", getBuiltinBool<true>},
        {"false", getBuiltinBool<false>},
    };
    for (const auto &Candidate : Entries)
    {
      RETURN_IF(Candidate.Name == Name, Candidate.GetValue(Context));
    }
    return nullptr;
  }

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
