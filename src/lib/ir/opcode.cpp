#include "ink/ir/opcode.h"

#include <cstddef>

namespace ink::ir
{
  namespace
  {
    constexpr IrOpcodeMetadata OpcodeMetadata[] = {
        {IrOpcode::Unknown, "unknown", IrPayloadKind::None, 0, 0, 0, 0, 0, false, IrEffect::None, IrStage::None},
#define INK_IR_OPCODE(Name, Mnemonic, Payload, MinimumOperands, MaximumOperands, MinimumResults, MaximumResults, Successors, Terminator, Effects, Stages) {IrOpcode::Name, Mnemonic, IrPayloadKind::Payload, MinimumOperands, MaximumOperands, MinimumResults, MaximumResults, Successors, Terminator, Effects, Stages},
#include "ink/ir/generated/opcode.def"
#undef INK_IR_OPCODE
    };
  }

  const IrOpcodeMetadata *irOpcodeMetadata(IrOpcode Opcode) noexcept
  {
    const auto Index = static_cast<std::size_t>(Opcode);
    if (Index >= sizeof(OpcodeMetadata) / sizeof(OpcodeMetadata[0]))
    {
      return nullptr;
    }
    return &OpcodeMetadata[Index];
  }

  const char *irOpcodeName(IrOpcode Opcode) noexcept
  {
    const auto *Metadata = irOpcodeMetadata(Opcode);
    return Metadata == nullptr ? "unknown" : Metadata->Mnemonic;
  }

  bool isIrOpcode(IrOpcode Opcode) noexcept
  {
    return Opcode != IrOpcode::Unknown && irOpcodeMetadata(Opcode) != nullptr;
  }
} // namespace ink::ir
