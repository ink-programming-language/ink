#ifndef INK_TYPE_TYPE_CONTEXT_H
#define INK_TYPE_TYPE_CONTEXT_H

#include "ink/type/type.h"

#include <cstddef>
#include <unordered_map>
#include <vector>

namespace ink::type
{
  class TypeContext
  {
  public:
    TypeContext();
    TypeContext(const TypeContext &) = delete;
    TypeContext &operator=(const TypeContext &) = delete;
    TypeContext(TypeContext &&) = delete;
    TypeContext &operator=(TypeContext &&) = delete;

    TypeId errorType() const noexcept;
    TypeId unitType() const noexcept;
    TypeId boolType() const noexcept;
    TypeId i32Type() const noexcept;
    TypeId i64Type() const noexcept;
    TypeId u32Type() const noexcept;
    TypeId u64Type() const noexcept;
    TypeId voidType() const noexcept;
    TypeId neverType() const noexcept;
    TypeId functionType(std::vector<TypeId> Parameters, TypeId Result);

    bool contains(TypeId Id) const noexcept;
    const Type &type(TypeId Id) const;
    const FunctionType &function(TypeId Id) const;
    std::size_t size() const noexcept;

  private:
    struct FunctionTypeHash
    {
      std::size_t operator()(const FunctionType &Value) const noexcept;
    };

    TypeId addBuiltin(TypeKind Kind);

    std::vector<Type> Types;
    std::unordered_map<FunctionType, TypeId, FunctionTypeHash> FunctionTypes;
    TypeId ErrorType;
    TypeId UnitType;
    TypeId BoolType;
    TypeId I32Type;
    TypeId I64Type;
    TypeId U32Type;
    TypeId U64Type;
    TypeId VoidType;
    TypeId NeverType;
  };
} // namespace ink::type

#endif
