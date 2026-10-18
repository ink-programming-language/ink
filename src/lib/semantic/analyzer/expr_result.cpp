#include "ink/semantic/analyzer/expr_result.h"

#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

namespace ink::semantic
{
  ExprResult ExprResult::fromValue(const ir::Value *Value) noexcept
  {
    if (!Value)
    {
      return {};
    }
    if (ir::Type::classof(Value))
    {
      return {.Kind = ExprResultKind::Type, .Value = Value, .Type = Value->type()};
    }
    if (ir::Module::classof(Value))
    {
      return {.Kind = ExprResultKind::Module, .Value = Value, .Type = Value->type()};
    }
    if (ir::GlobalVariable::classof(Value))
    {
      return lvalue(*Value, *Value->type());
    }
    return {.Kind = ExprResultKind::RValue, .Value = Value, .Type = Value->type()};
  }

  ExprResult ExprResult::lvalue(const ir::Value &Storage, const ir::Type &Type, const Binding *Variable) noexcept
  {
    return {.Kind = ExprResultKind::LValue, .Value = &Storage, .Type = &Type, .Variable = Variable};
  }
} // namespace ink::semantic
