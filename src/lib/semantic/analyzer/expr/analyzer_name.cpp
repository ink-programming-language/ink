#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/name_expr.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

#include <string_view>

namespace ink::semantic
{
  namespace
  {
    bool isValidBindingValue(AnalyzerState &State, const ir::Value &Value) noexcept
    {
      const auto &Context = State.semanticContext().irContext();
      if (ir::Type::classof(&Value))
      {
        return Context.containsType(static_cast<const ir::Type *>(&Value));
      }
      if (ir::Module::classof(&Value))
      {
        return &static_cast<const ir::Module &>(Value).irContext() == &Context;
      }
      const ir::Type *Type = Value.type();
      if (!Context.containsType(Type) || (!Type->hasLayout() && !ir::NullPointerConstant::classof(&Value)))
      {
        return false;
      }
      if (ir::Constant::classof(&Value))
      {
        return Context.constantPool().owns(static_cast<const ir::Constant *>(&Value));
      }
      if (ir::GlobalVariable::classof(&Value) || ir::Function::classof(&Value))
      {
        return true;
      }
      if (ir::Parameter::classof(&Value))
      {
        return static_cast<const ir::Parameter &>(Value).function() == State.irBuilder().insertFunction();
      }
      if (ir::Instruction::classof(&Value))
      {
        const auto &Instruction = static_cast<const ir::Instruction &>(Value);
        const ir::BasicBlock *Parent = Instruction.parentBlock();
        const ir::Function *Function = State.irBuilder().insertFunction();
        return !Instruction.isTerminator() && Function && Parent && Parent->function() == Function;
      }
      return false;
    }

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
        if (Candidate.Name == Name)
        {
          return Candidate.GetValue(Context);
        }
      }
      return nullptr;
    }
  } // namespace

  const ir::Value *Analyzer::analyzeBinding(AnalyzerState &State, const frontend::parser::ASTNodeBase &Node, const Binding &Found, bool IsCallCallee)
  {
    if (Found.targets().size() != 1)
    {
      // TODO: Select an overload using the call arguments or an expected function signature.
      if (IsCallCallee)
      {
        reportUnsupported(State, Node.getSourceRange(), "function overload selection");
        return nullptr;
      }
      State.semanticContext().report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
      return nullptr;
    }
    const ir::Value *Value = Found.targets().front();
    // Bound objects are compiler-produced IR; invalid ownership or storage is an internal model violation.
    if (!Value || !isValidBindingValue(State, *Value))
    {
      State.semanticContext().report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    if (ir::Parameter::classof(Value) && static_cast<const ir::Parameter *>(Value)->isVariadic())
    {
      // TODO: Materialize variadic parameter packs before using them as expression values.
      reportUnsupported(State, Node.getSourceRange(), "variadic parameter values");
      return nullptr;
    }
    return Value;
  }

  const ir::Value *Analyzer::analyzeNameExpr(AnalyzerState &State, const frontend::parser::NameExpr &Node, bool IsCallCallee)
  {
    auto &Context = State.semanticContext();
    const auto Name = Node.name();
    if (const Binding *Found = State.nameResolver().lookup(core::NamePool::instance().find(Name.Text)))
    {
      return analyzeBinding(State, Node, *Found, IsCallCallee);
    }
    if (const ir::Value *Builtin = lookupBuiltinValue(Context, Name.Text))
    {
      return Builtin;
    }
    Context.report<core::DiagnosticKind::SemanticUnknownName>(Name.Range, Name.Text);
    return nullptr;
  }
} // namespace ink::semantic
