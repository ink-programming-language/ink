#include "ink/ir/type.h"

namespace ink::ir
{
  Type::~Type() = default;

  const char *typeKindName(TypeKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_TYPE(Name, Spelling) \
  case TypeKind::Name:              \
    return Spelling;
#define INK_IR_DERIVED_TYPE(Name, Spelling) \
  case TypeKind::Name:                      \
    return Spelling;
#include "ink/ir/ir.def"
    case TypeKind::Count:
      return "unknown";
    }
    return "unknown";
  }

  bool isFloatingPointType(TypeKind Kind) noexcept
  {
    return Kind == TypeKind::F16 || Kind == TypeKind::F32 || Kind == TypeKind::F64;
  }

  std::size_t floatingPointBitWidth(TypeKind Kind) noexcept
  {
    switch (Kind)
    {
    case TypeKind::F16:
      return 16;
    case TypeKind::F32:
      return 32;
    case TypeKind::F64:
      return 64;
    case TypeKind::Void:
    case TypeKind::Bool:
    case TypeKind::Byte:
    case TypeKind::I32:
    case TypeKind::PointerSize:
    case TypeKind::BytePointer:
    case TypeKind::ConstBytePointer:
    case TypeKind::ByteSlice:
    case TypeKind::ConstByteSlice:
    case TypeKind::Struct:
    case TypeKind::Count:
      return 0;
    }
    return 0;
  }
} // namespace ink::ir
