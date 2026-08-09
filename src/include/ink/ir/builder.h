#ifndef INK_IR_BUILDER_H
#define INK_IR_BUILDER_H

#include "ink/ir/module.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ink::ir
{
  struct IrSuccessorSpec
  {
    IrBlockId Block;
    std::vector<IrValueId> Arguments;
  };

  struct IrOperationSpec
  {
    IrOpcode Opcode = IrOpcode::Unknown;
    IrOriginId Origin;
    std::vector<IrValueId> Operands;
    std::vector<IrTypeId> ResultTypes;
    std::vector<IrSuccessorSpec> Successors;
    IrOperationPayload Payload = IrNoPayload{};
  };

  struct IrBuiltBlock
  {
    IrBlockId Block;
    std::vector<IrValueId> Arguments;
  };

  struct IrBuiltOperation
  {
    IrOperationId Operation;
    std::vector<IrValueId> Results;
  };

  class IrBuilder
  {
  public:
    IrBuilder();
    ~IrBuilder();
    IrBuilder(IrBuilder &&Other) noexcept;
    IrBuilder &operator=(IrBuilder &&Other) noexcept;
    IrBuilder(const IrBuilder &) = delete;
    IrBuilder &operator=(const IrBuilder &) = delete;

    IrTypeId unitType();
    IrTypeId neverType();
    IrTypeId boolType();
    IrTypeId integerType(std::uint16_t BitWidth, IrSignedness Signedness);
    IrTypeId placeType(IrTypeId ElementType, IrPlaceAccess Access);
    IrTypeId functionType(const std::vector<IrTypeId> &Parameters, std::optional<IrTypeId> Result);

    IrConstantId integerConstant(IrTypeId Type, std::uint64_t Bits);
    IrConstantId boolConstant(bool Value);
    IrOriginId addOrigin(core::SourceFileId File, core::SourceRange Range);

    IrFunctionId addFunction(std::string Name, IrTypeId Signature, IrFunctionKind Kind, IrOriginId Origin = {});
    IrBuiltBlock addBlock(IrFunctionId Function, const std::vector<IrTypeId> &ArgumentTypes = {}, IrOriginId Origin = {});
    void setEntryBlock(IrFunctionId Function, IrBlockId Block);

    IrBuiltOperation appendOperation(IrBlockId Block, IrOperationSpec Spec);
    IrValueId createIntegerConstant(IrBlockId Block, IrConstantId Constant, IrOriginId Origin = {});
    IrValueId createBoolConstant(IrBlockId Block, IrConstantId Constant, IrOriginId Origin = {});
    IrValueId createIntegerBinary(IrBlockId Block, IrOpcode Opcode, IrValueId Left, IrValueId Right, IrOriginId Origin = {});
    IrValueId createIntegerNegate(IrBlockId Block, IrValueId Operand, IrOriginId Origin = {});
    IrValueId createIntegerCompare(IrBlockId Block, IrComparePredicate Predicate, IrValueId Left, IrValueId Right, IrOriginId Origin = {});
    IrValueId createIntegerCast(IrBlockId Block, IrValueId Operand, IrTypeId ResultType, IrOriginId Origin = {});
    IrValueId createBoolUnary(IrBlockId Block, IrOpcode Opcode, IrValueId Operand, IrOriginId Origin = {});
    IrValueId createBoolBinary(IrBlockId Block, IrOpcode Opcode, IrValueId Left, IrValueId Right, IrOriginId Origin = {});
    IrValueId createAlloca(IrBlockId Block, IrTypeId ElementType, IrPlaceAccess Access, IrOriginId Origin = {});
    IrValueId createLoad(IrBlockId Block, IrValueId Place, IrOriginId Origin = {});
    IrOperationId createStore(IrBlockId Block, IrValueId Place, IrValueId Value, IrOriginId Origin = {});
    IrBuiltOperation createDirectCall(IrBlockId Block, IrFunctionId Callee, const std::vector<IrValueId> &Arguments, IrOriginId Origin = {});
    IrOperationId createBranch(IrBlockId Block, IrBlockId Target, const std::vector<IrValueId> &Arguments = {}, IrOriginId Origin = {});
    IrOperationId createConditionalBranch(IrBlockId Block, IrValueId Condition, IrBlockId TrueTarget, const std::vector<IrValueId> &TrueArguments, IrBlockId FalseTarget, const std::vector<IrValueId> &FalseArguments, IrOriginId Origin = {});
    IrOperationId createReturn(IrBlockId Block, std::optional<IrValueId> Value = std::nullopt, IrOriginId Origin = {});
    IrOperationId createUnreachable(IrBlockId Block, IrOriginId Origin = {});
    IrOperationId createTrap(IrBlockId Block, IrTrapKind Kind, IrOriginId Origin = {});
    IrPlanNodeId addForceValuePlan(IrValueId Input, IrValueId Output, IrOriginId Origin = {});

    UnverifiedStagedModule finish();

  private:
    struct Impl;
    std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::ir

#endif
