#include "ink/execution/value/execution_value.h"

#include "ink/execution/value/execution_bool_value.h"
#include "ink/execution/value/execution_float_value.h"
#include "ink/execution/value/execution_function_value.h"
#include "ink/execution/value/execution_integer_value.h"
#include "ink/execution/value/execution_pointer_value.h"
#include "ink/execution/value/execution_string_value.h"

#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/slice_type.h"

#include <cassert>
#include <utility>

namespace ink::execution
{
  namespace
  {
    bool isStringType(const ir::Type &Type) noexcept
    {
      if (Type.typeKind() != ir::TypeKind::Slice)
      {
        return false;
      }
      const auto &Slice = static_cast<const ir::SliceType &>(Type);
      if (Slice.access() != ir::AccessKind::ReadOnly || Slice.elementType().typeKind() != ir::TypeKind::Integer)
      {
        return false;
      }
      const auto &Element = static_cast<const ir::IntegerType &>(Slice.elementType());
      return Element.bitWidth() == 8 && !Element.isSigned();
    }
  } // namespace

  ExecutionValue::ExecutionValue(ExecutionObjectKind Kind, const ir::Type &Type) noexcept
      : ExecutionObject(Kind),
        ValueType(Type)
  {
  }

  ExecutionValue::~ExecutionValue() = default;

  ExecutionValueKind ExecutionValue::kind() const noexcept
  {
    switch (objectKind())
    {
    case ExecutionObjectKind::VoidValue:
      return ExecutionValueKind::Void;
    case ExecutionObjectKind::BooleanValue:
      return ExecutionValueKind::Boolean;
    case ExecutionObjectKind::IntegerValue:
      return ExecutionValueKind::Integer;
    case ExecutionObjectKind::FloatValue:
      return ExecutionValueKind::Float;
    case ExecutionObjectKind::StringValue:
      return ExecutionValueKind::String;
    case ExecutionObjectKind::PointerValue:
      return ExecutionValueKind::Pointer;
    case ExecutionObjectKind::FunctionValue:
      return ExecutionValueKind::Function;
    case ExecutionObjectKind::Cell:
    case ExecutionObjectKind::Buffer:
      return ExecutionValueKind::Invalid;
    }
    return ExecutionValueKind::Invalid;
  }

  bool ExecutionValue::valid() const noexcept
  {
    switch (kind())
    {
    case ExecutionValueKind::Invalid:
      return false;
    case ExecutionValueKind::Void:
      return ValueType.typeKind() == ir::TypeKind::Void;
    case ExecutionValueKind::Boolean:
      return ValueType.typeKind() == ir::TypeKind::Bool;
    case ExecutionValueKind::Integer:
    {
      const ExecutionInteger &Value = static_cast<const ExecutionIntegerValue &>(*this).value();
      return ValueType.typeKind() == ir::TypeKind::Integer && Value.valid() && Value.bitWidth() == static_cast<const ir::IntegerType &>(ValueType).bitWidth();
    }
    case ExecutionValueKind::Float:
    {
      const ir::FloatBits Value = static_cast<const ExecutionFloatValue &>(*this).value();
      return ValueType.typeKind() == ir::TypeKind::Float && Value.valid() && Value.bitWidth() == static_cast<const ir::FloatType &>(ValueType).bitWidth();
    }
    case ExecutionValueKind::String:
      return isStringType(ValueType);
    case ExecutionValueKind::Pointer:
      return ValueType.typeKind() == ir::TypeKind::Pointer && static_cast<const ExecutionPointerValue &>(*this).value().valid();
    case ExecutionValueKind::Function:
      return ValueType.typeKind() == ir::TypeKind::Function && &static_cast<const ExecutionFunctionValue &>(*this).value().type() == &ValueType;
    }
    return false;
  }

  const ir::Constant *ExecutionValue::toConstant(ir::IRContext &Context) const
  {
    if (!valid() || &ValueType.context() != &Context)
    {
      return nullptr;
    }
    switch (kind())
    {
    case ExecutionValueKind::Boolean:
      return &Context.constantPool().getBoolConstant(static_cast<const ExecutionBoolValue &>(*this).value());
    case ExecutionValueKind::Integer:
      return Context.constantPool().getIntegerConstant(static_cast<const ir::IntegerType &>(ValueType), static_cast<const ExecutionIntegerValue &>(*this).value().bits());
    case ExecutionValueKind::Float:
      return Context.constantPool().getFloatConstant(static_cast<const ir::FloatType &>(ValueType), static_cast<const ExecutionFloatValue &>(*this).value());
    case ExecutionValueKind::String:
      return Context.constantPool().getStringConstant(static_cast<const ir::SliceType &>(ValueType), static_cast<const ExecutionStringValue &>(*this).value());
    default:
      return nullptr;
    }
  }

  ExecutionValueRef::ExecutionValueRef(std::shared_ptr<const ExecutionValue> Value) noexcept
      : Value(std::move(Value))
  {
  }

  const ExecutionValue *ExecutionValueRef::operator->() const noexcept
  {
    assert(Value && "cannot access an empty execution value reference");
    return Value.get();
  }

  bool ExecutionValueRef::boolean() const noexcept
  {
    assert(kind() == ExecutionValueKind::Boolean && "execution value is not a boolean");
    return static_cast<const ExecutionBoolValue &>(*Value).value();
  }

  const ExecutionInteger &ExecutionValueRef::integer() const noexcept
  {
    assert(kind() == ExecutionValueKind::Integer && "execution value is not an integer");
    return static_cast<const ExecutionIntegerValue &>(*Value).value();
  }

  ir::FloatBits ExecutionValueRef::floating() const noexcept
  {
    assert(kind() == ExecutionValueKind::Float && "execution value is not a float");
    return static_cast<const ExecutionFloatValue &>(*Value).value();
  }

  std::string_view ExecutionValueRef::string() const noexcept
  {
    assert(kind() == ExecutionValueKind::String && "execution value is not a string");
    return static_cast<const ExecutionStringValue &>(*Value).value();
  }

  const ExecutionPointer &ExecutionValueRef::pointer() const noexcept
  {
    assert(kind() == ExecutionValueKind::Pointer && "execution value is not a pointer");
    return static_cast<const ExecutionPointerValue &>(*Value).value();
  }

  const ir::Function *ExecutionValueRef::function() const noexcept
  {
    assert(kind() == ExecutionValueKind::Function && "execution value is not a function");
    return &static_cast<const ExecutionFunctionValue &>(*Value).value();
  }

  ExecutionValueResult::ExecutionValueResult(ExecutionStatus Status) noexcept
      : Status(Status == ExecutionStatus::Success ? ExecutionStatus::InvalidArguments : Status)
  {
  }

  ExecutionValueResult::ExecutionValueResult(ExecutionStatus Status, ExecutionValueRef Value) noexcept
      : Status(Status == ExecutionStatus::Success && !Value ? ExecutionStatus::InvalidArguments : Status),
        Value(Status == ExecutionStatus::Success ? std::move(Value) : ExecutionValueRef{})
  {
  }
} // namespace ink::execution
