#ifndef INK_LIB_IR_IR_STORAGE_H
#define INK_LIB_IR_IR_STORAGE_H

#include "ink/ir/module.h"

#include <vector>

namespace ink::ir::detail
{
  struct IrModuleStorage
  {
    std::vector<IrType> Types;
    std::vector<IrConstant> Constants;
    std::vector<IrOrigin> Origins;
    std::vector<IrFunction> Functions;
    std::vector<IrBlock> Blocks;
    std::vector<IrOperation> Operations;
    std::vector<IrValue> Values;
    std::vector<IrPlanNode> PlanNodes;
    std::vector<IrTypeId> TypeReferences;
    std::vector<IrBlockId> FunctionBlocks;
    std::vector<IrOperationId> BlockOperations;
    std::vector<IrValueId> OperationOperands;
    std::vector<IrValueId> OperationResults;
    std::vector<IrSuccessor> OperationSuccessors;
    std::vector<IrValueId> SuccessorArguments;
  };
} // namespace ink::ir::detail

#endif
