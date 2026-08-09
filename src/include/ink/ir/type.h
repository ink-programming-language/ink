#ifndef INK_IR_TYPE_H
#define INK_IR_TYPE_H

#include "ink/ir/ids.h"

#include <cstdint>
#include <optional>

namespace ink::ir
{
  enum class IrTypeKind : std::uint8_t
  {
    Unknown,
    Unit,
    Never,
    Bool,
    Integer,
    Place,
    Function,
  };

  enum class IrSignedness : std::uint8_t
  {
    Signless,
    Signed,
    Unsigned,
  };

  enum class IrPlaceAccess : std::uint8_t
  {
    ReadOnly,
    ReadWrite,
  };

  struct IrType
  {
    IrTypeKind Kind = IrTypeKind::Unknown;
    std::uint16_t BitWidth = 0;
    IrSignedness Signedness = IrSignedness::Signless;
    IrTypeId ElementType;
    IrPlaceAccess Access = IrPlaceAccess::ReadOnly;
    IrTableRange Parameters;
    std::optional<IrTypeId> Result;
  };

  enum class IrConstantKind : std::uint8_t
  {
    Unknown,
    Integer,
    Bool,
  };

  struct IrConstant
  {
    IrConstantKind Kind = IrConstantKind::Unknown;
    IrTypeId Type;
    std::uint64_t Bits = 0;
  };
} // namespace ink::ir

#endif
