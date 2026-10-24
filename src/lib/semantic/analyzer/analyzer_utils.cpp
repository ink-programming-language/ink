#include "ink/semantic/analyzer/analyzer_utils.h"

#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analysis_site.h"
#include "ink/semantic/context.h"

#include <concepts>
#include <cstdint>
#include <string>

namespace ink::semantic
{
  namespace
  {
    template <typename TypeT>
    const ir::Value *getBuiltinType(SemanticContext &Context, std::uint32_t Width, bool Signed)
    {
      if constexpr (std::same_as<TypeT, ir::IntegerType>)
      {
        return Context.typePool().get<TypeT>(Width, Signed);
      }
      else if constexpr (std::same_as<TypeT, ir::FloatType>)
      {
        return Context.typePool().get<TypeT>(Width);
      }
      else
      {
        return Context.typePool().get<TypeT>();
      }
    }

    template <bool Value>
    const ir::Value *getBuiltinBool(SemanticContext &Context, std::uint32_t, bool) noexcept
    {
      return Context.irContext().constantPool().get<ir::BoolConstant>(Value);
    }

    const ir::Value *getBuiltinNullPointer(SemanticContext &Context, std::uint32_t, bool) noexcept
    {
      return Context.irContext().constantPool().get<ir::NullPointerConstant>();
    }

    bool isRuntimeValue(SemanticContext &Context, const ir::Value &Value)
    {
      const ir::Type *Type = Value.type();
      const bool IsDescriptor = ir::Type::classof(&Value) || ir::Module::classof(&Value);
      return !IsDescriptor && Context.irContext().containsType(Type) && (Type->hasLayout() || ir::NullPointerConstant::classof(&Value));
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
        const ir::Value *(*GetValue)(SemanticContext &, std::uint32_t, bool);
        std::uint32_t Width = 0;
        bool Signed = false;
    };
    // Numeric source names remain fixed; resolve descriptors through the current context's pool.
    static constexpr Entry Entries[] = {
        {"type", getBuiltinType<ir::MetaType>},
        {"void", getBuiltinType<ir::VoidType>},
        {"bool", getBuiltinType<ir::BoolType>},
        {"i8", getBuiltinType<ir::IntegerType>, 8, true},
        {"u8", getBuiltinType<ir::IntegerType>, 8, false},
        {"i16", getBuiltinType<ir::IntegerType>, 16, true},
        {"u16", getBuiltinType<ir::IntegerType>, 16, false},
        {"i32", getBuiltinType<ir::IntegerType>, 32, true},
        {"u32", getBuiltinType<ir::IntegerType>, 32, false},
        {"i64", getBuiltinType<ir::IntegerType>, 64, true},
        {"u64", getBuiltinType<ir::IntegerType>, 64, false},
        {"f32", getBuiltinType<ir::FloatType>, 32},
        {"f64", getBuiltinType<ir::FloatType>, 64},
        {"true", getBuiltinBool<true>},
        {"false", getBuiltinBool<false>},
        {"nullptr", getBuiltinNullPointer},
    };
    for (const auto &Candidate : Entries)
    {
      if (Candidate.Name == Name)
      {
        return Candidate.GetValue(Context, Candidate.Width, Candidate.Signed);
      }
    }
    return nullptr;
  }

  const ir::Type *requireType(const AnalysisSite &Site, const ir::Value *Target)
  {
    RETURN_IF_NOT(Target, nullptr);
    RETURN_ERROR_IF_NOT(Site, ir::Type::classof(Target), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type", ir::valueKindName(Target->kind()));
    const auto *Type = static_cast<const ir::Type *>(Target);
    RETURN_ERROR_IF_NOT(Site, Site.semanticContext().irContext().containsType(Type), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type in the current IR context", "a foreign or unregistered type");
    return Type;
  }

  const ir::Value *requireRuntimeValue(const AnalysisSite &Site, const ir::Value *Target)
  {
    RETURN_IF_NOT(Target, nullptr);
    RETURN_ERROR_IF_NOT(Site, isRuntimeValue(Site.semanticContext(), *Target), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a runtime value", runtimeValueDescription(*Target));
    return Target;
  }
} // namespace ink::semantic
