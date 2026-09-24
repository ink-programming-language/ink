#ifndef INK_SEMANTIC_MODEL_COREDEFINES_H
#define INK_SEMANTIC_MODEL_COREDEFINES_H

#include <cstdint>

namespace ink::semantic
{
  enum class ValueKind : std::uint8_t
  {
#define INK_SEMANTIC_VALUE(Name) Name,
#include "ink/semantic/model/Values.def"
  };

  enum class TypeKind : std::uint8_t
  {
#define INK_SEMANTIC_TYPE(Name, Base) Name,
#include "ink/semantic/model/type/Types.def"
#undef INK_SEMANTIC_TYPE
  };

  // Access through a pointer/reference/slice, independent of binding mutability.
  enum class AccessKind : std::uint8_t
  {
    ReadOnly,
    ReadWrite,
  };

  // Binding mutability is independent of access through a value and compile-time evaluation.
  enum class BindingMutability : std::uint8_t
  {
    Mutable,
    Immutable,
  };

  // Declaration/member visibility, independent of data access and binding mutability.
  enum class VisibilityKind : std::uint8_t
  {
    Public,
    Private,
  };
} // namespace ink::semantic

#endif
