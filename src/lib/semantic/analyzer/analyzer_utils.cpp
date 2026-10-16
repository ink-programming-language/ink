#include "ink/semantic/analyzer/analyzer_utils.h"

#include "ink/ir/type/type_pool.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  namespace
  {
    template <auto Getter>
    const ir::Type *getBuiltinType(const ir::TypePool &Types) noexcept
    {
      return (Types.*Getter)();
    }
  } // namespace

  const ir::Type *lookupBuiltinType(const ir::TypePool &Types, std::string_view Name)
  {
    struct Entry
    {
        std::string_view Name;
        const ir::Type *(*GetType)(const ir::TypePool &) noexcept;
    };
    // TODO: Extend numeric builtins with i8/i16/u16/i128/u128/f16 once TypePool supports them.
    static constexpr Entry Entries[] = {
        {"void", getBuiltinType<&ir::TypePool::getVoidType>},
        {"bool", getBuiltinType<&ir::TypePool::getBoolType>},
        {"u8", getBuiltinType<&ir::TypePool::getUInt8Type>},
        {"i32", getBuiltinType<&ir::TypePool::getInt32Type>},
        {"u32", getBuiltinType<&ir::TypePool::getUInt32Type>},
        {"i64", getBuiltinType<&ir::TypePool::getInt64Type>},
        {"u64", getBuiltinType<&ir::TypePool::getUInt64Type>},
        {"f32", getBuiltinType<&ir::TypePool::getFloat32Type>},
        {"f64", getBuiltinType<&ir::TypePool::getFloat64Type>},
    };
    for (const auto &Candidate : Entries)
    {
      if (Candidate.Name == Name)
      {
        return Candidate.GetType(Types);
      }
    }
    return nullptr;
  }

  const ir::Type *requireType(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target)
  {
    if (!Target)
    {
      return nullptr;
    }
    if (!ir::Type::classof(Target))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Range, "a type", ir::valueKindName(Target->kind()));
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
} // namespace ink::semantic
