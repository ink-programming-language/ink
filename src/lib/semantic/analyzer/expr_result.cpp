#include "ink/semantic/analyzer/expr_result.h"

#include "ink/core/support/core_define.h"
#include "ink/ir/type/structure_type/pointer_type.h"

namespace ink::semantic
{
  ExprResult::ExprResult(const ir::Value *Value) noexcept
      : Value(Value)
  {
  }

  ExprResult ExprResult::location(const ir::Value &Address, const ir::Value *Declaration) noexcept
  {
    check(ir::PointerType::classof(Address.type()));
    ExprResult Result(&Address);
    Result.Declaration = Declaration;
    Result.Location = true;
    return Result;
  }

  ExprResult::operator bool() const noexcept
  {
    return Value != nullptr;
  }

  const ir::Value *ExprResult::value() const noexcept
  {
    return Value;
  }

  const ir::Type *ExprResult::type() const noexcept
  {
    if (!Value)
    {
      return nullptr;
    }
    return Location ? static_cast<const ir::PointerType *>(Value->type())->pointeeType() : Value->type();
  }

  bool ExprResult::isLocation() const noexcept
  {
    return Location;
  }

  const ir::Value *ExprResult::declaration() const noexcept
  {
    return Declaration;
  }
} // namespace ink::semantic
