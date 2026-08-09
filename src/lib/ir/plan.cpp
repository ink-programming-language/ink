#include "ink/ir/plan.h"

#include <cstddef>

namespace ink::ir
{
  namespace
  {
    constexpr IrPlanOpcodeMetadata PlanOpcodeMetadata[] = {
        {IrPlanOpcode::Unknown, "unknown", 0, 0, 0, IrStage::None},
#define INK_IR_PLAN_OPCODE(Name, Mnemonic, MinimumInputs, MaximumInputs, ResultCount, Stages) {IrPlanOpcode::Name, Mnemonic, MinimumInputs, MaximumInputs, ResultCount, Stages},
#include "ink/ir/generated/plan_opcode.def"
#undef INK_IR_PLAN_OPCODE
    };
  }

  const IrPlanOpcodeMetadata *irPlanOpcodeMetadata(IrPlanOpcode Opcode) noexcept
  {
    const auto Index = static_cast<std::size_t>(Opcode);
    if (Index >= sizeof(PlanOpcodeMetadata) / sizeof(PlanOpcodeMetadata[0]))
    {
      return nullptr;
    }
    return &PlanOpcodeMetadata[Index];
  }

  const char *irPlanOpcodeName(IrPlanOpcode Opcode) noexcept
  {
    const auto *Metadata = irPlanOpcodeMetadata(Opcode);
    return Metadata == nullptr ? "unknown" : Metadata->Mnemonic;
  }

  bool isIrPlanOpcode(IrPlanOpcode Opcode) noexcept
  {
    return Opcode != IrPlanOpcode::Unknown && irPlanOpcodeMetadata(Opcode) != nullptr;
  }
} // namespace ink::ir
