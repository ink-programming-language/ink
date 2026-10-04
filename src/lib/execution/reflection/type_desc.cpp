#include "ink/execution/reflection/type_table.h"

namespace ink::execution
{
  void TypeDesc::setKind(RuntimeKind Value)
  {
    Kind = Value;
    switch (Value)
    {
    case RuntimeKind::Integer:
      Details = std::make_shared<IntegerDesc>();
      break;
    case RuntimeKind::Float:
      Details = std::make_shared<FloatDesc>();
      break;
    case RuntimeKind::Pointer:
      Details = std::make_shared<PointerDesc>();
      break;
    case RuntimeKind::Function:
      Details = std::make_shared<FunctionDesc>();
      break;
    case RuntimeKind::Array:
      Details = std::make_shared<ArrayDesc>();
      break;
    case RuntimeKind::Class:
      Details = std::make_shared<ClassDesc>();
      break;
    default:
      Details.reset();
      break;
    }
  }

  void TypeDesc::setDetails(std::shared_ptr<TypeDetails> Value)
  {
    Kind = Value ? Value->kind() : RuntimeKind::Invalid;
    Details = std::move(Value);
  }

  bool TypeDesc::validDetails() const noexcept
  {
    switch (Kind)
    {
    case RuntimeKind::Integer:
    case RuntimeKind::Float:
    case RuntimeKind::Pointer:
    case RuntimeKind::Function:
    case RuntimeKind::Array:
    case RuntimeKind::Class:
      return Details && Details->kind() == Kind;
    default:
      return !Details;
    }
  }

  std::uint32_t TypeDesc::bitWidth() const noexcept
  {
    return Kind == RuntimeKind::Boolean ? 1 : Kind == RuntimeKind::Integer ? integerDesc().BitWidth : Kind == RuntimeKind::Float ? floatDesc().BitWidth : 0;
  }

  void TypeDesc::setBitWidth(std::uint32_t Value)
  {
    if (Kind == RuntimeKind::Integer)
    {
      editInteger().BitWidth = Value;
    }
    else if (Kind == RuntimeKind::Float)
    {
      editFloat().BitWidth = Value;
    }
    else
    {
      assert(Value == bitWidth());
    }
  }

  bool TypeDesc::isSigned() const noexcept
  {
    return Kind == RuntimeKind::Integer && integerDesc().Signed;
  }

  const FieldDesc *ClassDesc::findField(std::string_view Name) const noexcept
  {
    for (const FieldDesc &Field : Fields)
    {
      if (Field.Name == Name)
      {
        return &Field;
      }
    }
    return nullptr;
  }

  const MethodDesc *ClassDesc::findMethod(std::string_view Name, RuntimeTypeId Signature) const noexcept
  {
    const MethodDesc *Result = nullptr;
    for (const MethodDesc &Method : Methods)
    {
      if (Method.Name == Name && (Signature == InvalidRuntimeType || Method.Signature == Signature))
      {
        if (Result)
        {
          return nullptr;
        }
        Result = &Method;
      }
    }
    return Result;
  }

  const TypeDesc *RuntimeTypeTable::find(std::string_view Name) const noexcept
  {
    // Avoid allocating a temporary string during reflection queries.
    for (const auto &[Key, Type] : Names)
    {
      if (Key == Name)
      {
        return get(Type);
      }
    }
    return nullptr;
  }

  bool RuntimeTypeTable::updateClass(RuntimeTypeId Type, std::shared_ptr<ClassDesc> Description)
  {
    if (!Description || Type >= Layouts.size() || Layouts[Type].Kind != RuntimeKind::Class || Description->NominalIdentity != Layouts[Type].classDesc().NominalIdentity || Description->Fields.size() != Layouts[Type].classDesc().Fields.size())
    {
      return false;
    }
    const auto &Previous = Layouts[Type].classDesc();
    for (std::size_t Index = 0; Index < Description->Fields.size(); ++Index)
    {
      const auto &Field = Description->Fields[Index];
      if (Field.Name != Previous.Fields[Index].Name || Field.Type != Previous.Fields[Index].Type || Field.Offset != Previous.Fields[Index].Offset)
      {
        return false;
      }
      Description->Fields[Index].Layout = Previous.Fields[Index].Layout;
    }
    // Preserve the canonical descriptor address retained by cells and arrays.
    *static_cast<ClassDesc *>(Layouts[Type].Details.get()) = *Description;
    return true;
  }
} // namespace ink::execution
