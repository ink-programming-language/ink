#include "ink/type/type_context.h"

#include <stdexcept>
#include <utility>

namespace ink::type
{
  const char *typeKindName(TypeKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_TYPE_KIND(Name) case TypeKind::Name: return #Name;
#include "ink/type/type_kind.def"
#undef INK_TYPE_KIND
    }
    return "Unknown";
  }

  bool operator==(const FunctionType &Left, const FunctionType &Right) noexcept
  {
    return Left.Parameters == Right.Parameters && Left.Result == Right.Result;
  }

  bool operator!=(const FunctionType &Left, const FunctionType &Right) noexcept
  {
    return !(Left == Right);
  }

  bool operator==(const Type &Left, const Type &Right) noexcept
  {
    return Left.Kind == Right.Kind && Left.Payload == Right.Payload;
  }

  bool operator!=(const Type &Left, const Type &Right) noexcept
  {
    return !(Left == Right);
  }

  TypeContext::TypeContext()
  {
    ErrorType = addBuiltin(TypeKind::Error);
    UnitType = addBuiltin(TypeKind::Unit);
    BoolType = addBuiltin(TypeKind::Bool);
    I32Type = addBuiltin(TypeKind::I32);
    I64Type = addBuiltin(TypeKind::I64);
    U32Type = addBuiltin(TypeKind::U32);
    U64Type = addBuiltin(TypeKind::U64);
    VoidType = addBuiltin(TypeKind::Void);
    NeverType = addBuiltin(TypeKind::Never);
  }

  TypeId TypeContext::errorType() const noexcept
  {
    return ErrorType;
  }

  TypeId TypeContext::unitType() const noexcept
  {
    return UnitType;
  }

  TypeId TypeContext::boolType() const noexcept
  {
    return BoolType;
  }

  TypeId TypeContext::i32Type() const noexcept
  {
    return I32Type;
  }

  TypeId TypeContext::i64Type() const noexcept
  {
    return I64Type;
  }

  TypeId TypeContext::u32Type() const noexcept
  {
    return U32Type;
  }

  TypeId TypeContext::u64Type() const noexcept
  {
    return U64Type;
  }

  TypeId TypeContext::voidType() const noexcept
  {
    return VoidType;
  }

  TypeId TypeContext::neverType() const noexcept
  {
    return NeverType;
  }

  TypeId TypeContext::functionType(std::vector<TypeId> Parameters, TypeId Result)
  {
    for (const TypeId Parameter : Parameters)
    {
      if (!contains(Parameter))
      {
        throw std::invalid_argument("function parameter type does not belong to this TypeContext");
      }
    }
    if (!contains(Result))
    {
      throw std::invalid_argument("function result type does not belong to this TypeContext");
    }

    FunctionType Key{std::move(Parameters), Result};
    const auto Existing = FunctionTypes.find(Key);
    if (Existing != FunctionTypes.end())
    {
      return Existing->second;
    }
    if (Types.size() >= TypeId::InvalidValue)
    {
      throw std::length_error("TypeContext cannot represent another type ID");
    }
    const TypeId Id = TypeId::fromValue(static_cast<TypeId::ValueType>(Types.size()));
    Types.push_back({TypeKind::Function, Key});
    try
    {
      FunctionTypes.emplace(std::move(Key), Id);
    }
    catch (...)
    {
      Types.pop_back();
      throw;
    }
    return Id;
  }

  bool TypeContext::contains(TypeId Id) const noexcept
  {
    return Id.isValid() && Id.value() < Types.size();
  }

  const Type &TypeContext::type(TypeId Id) const
  {
    if (!contains(Id))
    {
      throw std::out_of_range("TypeId does not identify a type in this TypeContext");
    }
    return Types[Id.value()];
  }

  const FunctionType &TypeContext::function(TypeId Id) const
  {
    const Type &Value = type(Id);
    if (Value.Kind != TypeKind::Function)
    {
      throw std::invalid_argument("TypeId does not identify a function type");
    }
    return std::get<FunctionType>(Value.Payload);
  }

  std::size_t TypeContext::size() const noexcept
  {
    return Types.size();
  }

  std::size_t TypeContext::FunctionTypeHash::operator()(const FunctionType &Value) const noexcept
  {
    std::size_t Result = std::hash<TypeId>{}(Value.Result);
    for (const TypeId Parameter : Value.Parameters)
    {
      Result ^= std::hash<TypeId>{}(Parameter) + static_cast<std::size_t>(0x9E3779B9U) + (Result << 6U) + (Result >> 2U);
    }
    Result ^= Value.Parameters.size() + static_cast<std::size_t>(0x9E3779B9U) + (Result << 6U) + (Result >> 2U);
    return Result;
  }

  TypeId TypeContext::addBuiltin(TypeKind Kind)
  {
    if (Types.size() >= TypeId::InvalidValue)
    {
      throw std::length_error("TypeContext cannot represent another type ID");
    }
    const TypeId Id = TypeId::fromValue(static_cast<TypeId::ValueType>(Types.size()));
    Types.push_back({Kind, std::monostate{}});
    return Id;
  }
} // namespace ink::type
