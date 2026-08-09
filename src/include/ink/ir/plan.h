#ifndef INK_IR_PLAN_H
#define INK_IR_PLAN_H

#include "ink/ir/ids.h"
#include "ink/ir/opcode.h"
#include "ink/ir/type.h"

#include <cstdint>

namespace ink::ir
{
  enum class IrPlanOpcode : std::uint8_t
  {
    Unknown,
#define INK_IR_PLAN_OPCODE(Name, Mnemonic, MinimumInputs, MaximumInputs, ResultCount, Stages) Name,
#include "ink/ir/generated/plan_opcode.def"
#undef INK_IR_PLAN_OPCODE
  };

  struct IrPlanOpcodeMetadata
  {
    IrPlanOpcode Opcode = IrPlanOpcode::Unknown;
    const char *Mnemonic = "unknown";
    std::uint8_t MinimumInputs = 0;
    std::uint8_t MaximumInputs = 0;
    std::uint8_t ResultCount = 0;
    IrStage Stages = IrStage::None;
  };

  struct IrPlanNode
  {
    IrPlanOpcode Opcode = IrPlanOpcode::Unknown;
    IrValueId Input;
    IrValueId Output;
    IrTypeId ResultType;
    IrOriginId Origin;
  };

  struct IrForceValueResolution
  {
    IrPlanNodeId PlanNode;
    IrTypeId Type;
    IrConstantKind Kind = IrConstantKind::Unknown;
    std::uint64_t Bits = 0;
  };

  const IrPlanOpcodeMetadata *irPlanOpcodeMetadata(IrPlanOpcode Opcode) noexcept;
  const char *irPlanOpcodeName(IrPlanOpcode Opcode) noexcept;
  bool isIrPlanOpcode(IrPlanOpcode Opcode) noexcept;
} // namespace ink::ir

#endif
