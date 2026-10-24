#include "ink/semantic/analyzer/analyzer_utils.h"

#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analysis_site.h"
#include "ink/semantic/context.h"

#include <string>
#include <type_traits>

namespace ink::semantic
{
  namespace
  {
    template <auto Getter>
    const ir::Value *getBuiltinType(SemanticContext &Context) noexcept
    {
      if constexpr (std::is_invocable_v<decltype(Getter), ir::TypePool &, bool>)
      {
        return (Context.typePool().*Getter)(true);
      }
      else
      {
        return (Context.typePool().*Getter)();
      }
    }

    template <bool Value>
    const ir::Value *getBuiltinBool(SemanticContext &Context) noexcept
    {
      return Context.irContext().constantPool().getBoolConstant(Value);
    }

    const ir::Value *getBuiltinNullPointer(SemanticContext &Context) noexcept
    {
      return Context.irContext().constantPool().getNullPointerConstant();
    }

    bool isRuntimeValue(SemanticContext &Context, const ir::Value &Value)
    {
      const ir::Type *Type = Value.type();
      const bool IsDescriptor = ir::Type::classof(&Value) || ir::Module::classof(&Value);
      return !IsDescriptor && !Value.isStorage() && Context.irContext().containsType(Type) && (Type->hasLayout() || ir::NullPointerConstant::classof(&Value));
    }

    std::string runtimeValueDescription(const ir::Value &Value)
    {
      return ir::Type::classof(&Value) ? static_cast<const ir::Type &>(Value).toString() : std::string(ir::valueKindName(Value.kind()));
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
        {"nullptr", getBuiltinNullPointer},
    };
    for (const auto &Candidate : Entries)
    {
      if (Candidate.Name == Name)
      {
        return Candidate.GetValue(Context);
      }
    }
    return nullptr;
  }

  const ir::Type *requireType(const AnalysisSite &Site, const ir::Value *Target)
  {
    RETURN_IF(!Target, nullptr);
    RETURN_REPORT_IF_FALSE(Site, ir::Type::classof(Target), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type", ir::valueKindName(Target->kind()));
    const auto *Type = static_cast<const ir::Type *>(Target);
    RETURN_REPORT_IF_FALSE(Site, Site.semanticContext().irContext().containsType(Type), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type in the current IR context", "a foreign or unregistered type");
    return Type;
  }

  const ir::Value *requireRuntimeValue(const AnalysisSite &Site, const ir::Value *Target)
  {
    RETURN_IF_NULL(Target, nullptr);
    RETURN_REPORT_IF_FALSE(Site, isRuntimeValue(Site.semanticContext(), *Target), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a runtime value", runtimeValueDescription(*Target));
    return Target;
  }
} // namespace ink::semantic
