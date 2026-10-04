#include "ink/execution/value/execution_value.h"
#include "ink/execution/value/execution_array_value.h"
#include "ink/execution/value/execution_class_value.h"
#include "ink/ir/constant/class_constant.h"

#include "ink/execution/value/execution_bool_value.h"
#include "ink/execution/value/execution_float_value.h"
#include "ink/execution/value/execution_function_value.h"
#include "ink/execution/value/execution_integer_value.h"
#include "ink/execution/value/execution_pointer_value.h"
#include "ink/execution/value/execution_string_value.h"

#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/slice_type.h"
#include "ink/ir/constant/array_constant.h"

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

  RuntimeKind ExecutionValue::kind() const noexcept
  {
    switch (objectKind())
    {
    case ExecutionObjectKind::VoidValue:
      return RuntimeKind::Void;
    case ExecutionObjectKind::BooleanValue:
      return RuntimeKind::Boolean;
    case ExecutionObjectKind::IntegerValue:
      return RuntimeKind::Integer;
    case ExecutionObjectKind::FloatValue:
      return RuntimeKind::Float;
    case ExecutionObjectKind::StringValue:
      return RuntimeKind::String;
    case ExecutionObjectKind::PointerValue:
      return RuntimeKind::Pointer;
    case ExecutionObjectKind::FunctionValue:
      return RuntimeKind::Function;
    case ExecutionObjectKind::ArrayValue:
      return RuntimeKind::Array;
    case ExecutionObjectKind::ClassValue:
      return RuntimeKind::Class;
    case ExecutionObjectKind::Cell:
    case ExecutionObjectKind::Buffer:
      return RuntimeKind::Invalid;
    }
    return RuntimeKind::Invalid;
  }

  bool ExecutionValue::valid() const noexcept
  {
    switch (kind())
    {
    case RuntimeKind::Invalid:
      return false;
    case RuntimeKind::Void:
      return ValueType.typeKind() == ir::TypeKind::Void;
    case RuntimeKind::Boolean:
      return ValueType.typeKind() == ir::TypeKind::Bool;
    case RuntimeKind::Integer:
    {
      const ExecutionInteger &Value = static_cast<const ExecutionIntegerValue &>(*this).value();
      return ValueType.typeKind() == ir::TypeKind::Integer && Value.valid() && Value.bitWidth() == static_cast<const ir::IntegerType &>(ValueType).bitWidth();
    }
    case RuntimeKind::Float:
    {
      const ir::FloatBits Value = static_cast<const ExecutionFloatValue &>(*this).value();
      return ValueType.typeKind() == ir::TypeKind::Float && Value.valid() && Value.bitWidth() == static_cast<const ir::FloatType &>(ValueType).bitWidth();
    }
    case RuntimeKind::String:
      return isStringType(ValueType);
    case RuntimeKind::Array:
    {
      if (!ir::ArrayType::classof(&ValueType))
      {
        return false;
      }
      const auto &Type = static_cast<const ir::ArrayType &>(ValueType);
      const auto Elements = static_cast<const ExecutionArrayValue &>(*this).value();
      if (Elements.size() != Type.elementCount())
      {
        return false;
      }
      for (const auto &Element : Elements)
      {
        if (!Element.valid() || Element.type() != &Type.elementType())
        {
          return false;
        }
      }
      return true;
    }
    case RuntimeKind::Class:
    {
      if (!ir::ClassType::classof(&ValueType))
      {
        return false;
      }
      const auto &Type = static_cast<const ir::ClassType &>(ValueType);
      const auto Fields = static_cast<const ExecutionClassValue &>(*this).value();
      if (!Type.isComplete() || Fields.size() != Type.fields().size())
      {
        return false;
      }
      for (std::size_t Index = 0; Index < Fields.size(); ++Index)
      {
        if (!Fields[Index].valid() || Fields[Index].type() != Type.fields()[Index].FieldType)
        {
          return false;
        }
      }
      return true;
    }
    case RuntimeKind::Pointer:
      return ValueType.typeKind() == ir::TypeKind::Pointer && static_cast<const ExecutionPointerValue &>(*this).value().valid();
    case RuntimeKind::Function:
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
    case RuntimeKind::Boolean:
      return &Context.constantPool().getBoolConstant(static_cast<const ExecutionBoolValue &>(*this).value());
    case RuntimeKind::Integer:
      return Context.constantPool().getIntegerConstant(static_cast<const ir::IntegerType &>(ValueType), static_cast<const ExecutionIntegerValue &>(*this).value().bits());
    case RuntimeKind::Float:
      return Context.constantPool().getFloatConstant(static_cast<const ir::FloatType &>(ValueType), static_cast<const ExecutionFloatValue &>(*this).value());
    case RuntimeKind::String:
      return Context.constantPool().getStringConstant(static_cast<const ir::SliceType &>(ValueType), static_cast<const ExecutionStringValue &>(*this).value());
    case RuntimeKind::Array:
    {
      std::vector<const ir::Constant *> Elements;
      for (const auto &Element : static_cast<const ExecutionArrayValue &>(*this).value())
      {
        const ir::Constant *Constant = Element.toConstant(Context);
        if (!Constant)
        {
          return nullptr;
        }
        Elements.push_back(Constant);
      }
      return Context.constantPool().getArrayConstant(static_cast<const ir::ArrayType &>(ValueType), Elements);
    }
    case RuntimeKind::Class:
    {
      std::vector<const ir::Constant *> Fields;
      for (const auto &Field : static_cast<const ExecutionClassValue &>(*this).value())
      {
        const ir::Constant *Constant = Field.toConstant(Context);
        if (!Constant)
        {
          return nullptr;
        }
        Fields.push_back(Constant);
      }
      return Context.constantPool().getClassConstant(static_cast<const ir::ClassType &>(ValueType), Fields);
    }
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
    assert(kind() == RuntimeKind::Boolean && "execution value is not a boolean");
    return static_cast<const ExecutionBoolValue &>(*Value).value();
  }

  const ExecutionInteger &ExecutionValueRef::integer() const noexcept
  {
    assert(kind() == RuntimeKind::Integer && "execution value is not an integer");
    return static_cast<const ExecutionIntegerValue &>(*Value).value();
  }

  ir::FloatBits ExecutionValueRef::floating() const noexcept
  {
    assert(kind() == RuntimeKind::Float && "execution value is not a float");
    return static_cast<const ExecutionFloatValue &>(*Value).value();
  }

  std::string_view ExecutionValueRef::string() const noexcept
  {
    assert(kind() == RuntimeKind::String && "execution value is not a string");
    return static_cast<const ExecutionStringValue &>(*Value).value();
  }

  const ExecutionPointer &ExecutionValueRef::pointer() const noexcept
  {
    assert(kind() == RuntimeKind::Pointer && "execution value is not a pointer");
    return static_cast<const ExecutionPointerValue &>(*Value).value();
  }

  std::span<const ExecutionValueRef> ExecutionValueRef::array() const noexcept
  {
    assert(kind() == RuntimeKind::Array && "execution value is not an array");
    return static_cast<const ExecutionArrayValue &>(*Value).value();
  }

  std::span<const ExecutionValueRef> ExecutionValueRef::fields() const noexcept
  {
    assert(kind() == RuntimeKind::Class && "execution value is not a class");
    return static_cast<const ExecutionClassValue &>(*Value).value();
  }

  const ir::Function *ExecutionValueRef::function() const noexcept
  {
    assert(kind() == RuntimeKind::Function && "execution value is not a function");
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
