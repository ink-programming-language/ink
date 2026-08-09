#ifndef INK_IR_OPCODE_H
#define INK_IR_OPCODE_H

#include <cstdint>

namespace ink::ir
{
  enum class IrStage : std::uint8_t
  {
    None = 0,
    Staged = 1 << 0,
    Closed = 1 << 1,
  };

  enum class IrEffect : std::uint16_t
  {
    None = 0,
    Pure = 1 << 0,
    ReadMemory = 1 << 1,
    WriteMemory = 1 << 2,
    Allocate = 1 << 3,
    Call = 1 << 4,
    Control = 1 << 5,
    MayTrap = 1 << 6,
    TargetDependent = 1 << 7,
    PdbBoundary = 1 << 8,
    Runtime = 1 << 9,
  };

  constexpr IrStage operator|(IrStage Left, IrStage Right) noexcept
  {
    return static_cast<IrStage>(static_cast<std::uint8_t>(Left) | static_cast<std::uint8_t>(Right));
  }

  constexpr IrEffect operator|(IrEffect Left, IrEffect Right) noexcept
  {
    return static_cast<IrEffect>(static_cast<std::uint16_t>(Left) | static_cast<std::uint16_t>(Right));
  }

  constexpr bool hasStage(IrStage Stages, IrStage Stage) noexcept
  {
    return (static_cast<std::uint8_t>(Stages) & static_cast<std::uint8_t>(Stage)) != 0;
  }

  constexpr bool hasEffect(IrEffect Effects, IrEffect Effect) noexcept
  {
    return (static_cast<std::uint16_t>(Effects) & static_cast<std::uint16_t>(Effect)) != 0;
  }

  enum class IrPayloadKind : std::uint8_t
  {
    None,
    Constant,
    Compare,
    Type,
    DirectCall,
    Trap,
  };

  enum class IrOpcode : std::uint8_t
  {
    Unknown,
#define INK_IR_OPCODE(Name, Mnemonic, Payload, MinimumOperands, MaximumOperands, MinimumResults, MaximumResults, Successors, Terminator, Effects, Stages) Name,
#include "ink/ir/generated/opcode.def"
#undef INK_IR_OPCODE
  };

  struct IrOpcodeMetadata
  {
    IrOpcode Opcode = IrOpcode::Unknown;
    const char *Mnemonic = "unknown";
    IrPayloadKind Payload = IrPayloadKind::None;
    std::uint8_t MinimumOperands = 0;
    std::uint8_t MaximumOperands = 0;
    std::uint8_t MinimumResults = 0;
    std::uint8_t MaximumResults = 0;
    std::uint8_t Successors = 0;
    bool Terminator = false;
    IrEffect Effects = IrEffect::None;
    IrStage Stages = IrStage::None;
  };

  const IrOpcodeMetadata *irOpcodeMetadata(IrOpcode Opcode) noexcept;
  const char *irOpcodeName(IrOpcode Opcode) noexcept;
  bool isIrOpcode(IrOpcode Opcode) noexcept;
} // namespace ink::ir

#endif
