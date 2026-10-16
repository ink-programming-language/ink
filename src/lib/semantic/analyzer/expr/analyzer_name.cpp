#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/name_expr.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

#include <string_view>

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

    const ir::Value *lookupBuiltinValue(SemanticContext &Context, std::string_view Name)
    {
      struct Entry
      {
          std::string_view Name;
          const ir::Value *(*GetValue)(SemanticContext &) noexcept;
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
          {"true", getBuiltinBool<true>},
          {"false", getBuiltinBool<false>},
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
  } // namespace

  const ir::Value *Analyzer::analyzeBinding(AnalyzerState &State, const frontend::parser::ASTNodeBase &Node, const Binding &Found)
  {
    if (Found.targets().size() != 1)
    {
      // TODO: Select an overload using the call arguments or an expected function signature.
      State.semanticContext().report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
      return nullptr;
    }
    return Found.targets().front();
  }

  const ir::Value *Analyzer::analyzeNameExpr(AnalyzerState &State, const frontend::parser::NameExpr &Node)
  {
    auto &Context = State.semanticContext();
    const auto Name = Node.name();
    if (const Binding *Found = State.nameResolver().lookup(core::NamePool::instance().find(Name.Text)))
    {
      return analyzeBinding(State, Node, *Found);
    }
    if (const ir::Value *Builtin = lookupBuiltinValue(Context, Name.Text))
    {
      return Builtin;
    }
    Context.report<core::DiagnosticKind::SemanticUnknownName>(Name.Range, Name.Text);
    return nullptr;
  }
} // namespace ink::semantic
