#ifndef INK_IR_COREDEFINES_H
#define INK_IR_COREDEFINES_H

#include <cstdint>

namespace ink::ir
{
  enum class ValueKind : std::uint8_t
  {
#define INK_IR_VALUE(Name) Name,
#include "ink/ir/Values.def"
  };

  enum class TypeKind : std::uint8_t
  {
#define INK_IR_TYPE(Name, Base) Name,
#include "ink/ir/type/Types.def"
#undef INK_IR_TYPE
  };

  // Parameter binding category, independent of the value's ValueKind.
  enum class ParameterKind : std::uint8_t
  {
    Positional,
    Named,
    Variadic,
  };

  // Runtime calling convention; target-specific ABI lowering belongs to the backend.
  enum class CallingConvention : std::uint8_t
  {
    C,
    Fast,
    Cold,
  };

  // Language linkage and symbol naming, independent of body presence and symbol visibility.
  enum class LanguageLinkage : std::uint8_t
  {
    Ink,
    C,
  };

  // Native symbol direction, independent of language linkage and source visibility.
  enum class FunctionBinding : std::uint8_t
  {
    Local,
    Import,
    Export,
  };

  // Access through a pointer/reference/slice, independent of binding mutability.
  enum class AccessKind : std::uint8_t
  {
    ReadOnly,
    ReadWrite,
  };

  // Declaration/member visibility, independent of data access and binding mutability.
  enum class VisibilityKind : std::uint8_t
  {
    Public,
    Private,
  };
} // namespace ink::ir

#endif
