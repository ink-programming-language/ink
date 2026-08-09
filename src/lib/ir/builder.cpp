#include "ink/ir/builder.h"

#include "ir_storage.h"

#include <algorithm>
#include <limits>
#include <stdexcept>
#include <utility>

namespace ink::ir
{
  namespace
  {
    template <typename Id, typename Entry>
    Id appendEntry(std::vector<Entry> &Entries, Entry EntryValue)
    {
      if (Entries.size() >= std::numeric_limits<typename Id::ValueType>::max())
      {
        throw std::length_error("InkIR table exceeds the typed ID range");
      }
      const auto IdValue = Id::fromValue(static_cast<typename Id::ValueType>(Entries.size()));
      Entries.push_back(std::move(EntryValue));
      return IdValue;
    }

    IrTableRange appendValues(std::vector<IrValueId> &Destination, const std::vector<IrValueId> &Values)
    {
      const auto First = static_cast<std::uint32_t>(Destination.size());
      Destination.insert(Destination.end(), Values.begin(), Values.end());
      return {First, static_cast<std::uint32_t>(Values.size())};
    }

    bool isIntegerBinaryOpcode(IrOpcode Opcode)
    {
      switch (Opcode)
      {
      case IrOpcode::IntAdd:
      case IrOpcode::IntSub:
      case IrOpcode::IntMul:
      case IrOpcode::IntSignedDiv:
      case IrOpcode::IntUnsignedDiv:
      case IrOpcode::IntSignedRem:
      case IrOpcode::IntUnsignedRem:
      case IrOpcode::IntAnd:
      case IrOpcode::IntOr:
      case IrOpcode::IntXor:
        return true;
      default:
        return false;
      }
    }
  }

  struct IrBuilder::Impl
  {
    std::shared_ptr<detail::IrModuleStorage> Storage = std::make_shared<detail::IrModuleStorage>();
    std::vector<std::vector<IrBlockId>> FunctionBlocks;
    std::vector<std::vector<IrOperationId>> BlockOperations;
    bool Finished = false;

    void requireActive() const
    {
      if (Finished)
      {
        throw std::logic_error("InkIR builder has already been finished");
      }
    }

    IrTypeId valueType(IrValueId Value) const
    {
      if (!Value.isValid() || Value.value() >= Storage->Values.size())
      {
        return {};
      }
      return Storage->Values[Value.value()].Type;
    }

    const IrType *findType(IrTypeId Type) const
    {
      if (!Type.isValid() || Type.value() >= Storage->Types.size())
      {
        return nullptr;
      }
      return &Storage->Types[Type.value()];
    }
  };

  IrBuilder::IrBuilder() : Implementation(std::make_unique<Impl>())
  {
  }

  IrBuilder::~IrBuilder() = default;
  IrBuilder::IrBuilder(IrBuilder &&Other) noexcept = default;
  IrBuilder &IrBuilder::operator=(IrBuilder &&Other) noexcept = default;

  IrTypeId IrBuilder::unitType()
  {
    Implementation->requireActive();
    for (std::size_t Index = 0; Index < Implementation->Storage->Types.size(); ++Index)
    {
      if (Implementation->Storage->Types[Index].Kind == IrTypeKind::Unit)
      {
        return IrTypeId::fromValue(static_cast<std::uint32_t>(Index));
      }
    }
    IrType Type;
    Type.Kind = IrTypeKind::Unit;
    return appendEntry<IrTypeId>(Implementation->Storage->Types, Type);
  }

  IrTypeId IrBuilder::boolType()
  {
    Implementation->requireActive();
    for (std::size_t Index = 0; Index < Implementation->Storage->Types.size(); ++Index)
    {
      if (Implementation->Storage->Types[Index].Kind == IrTypeKind::Bool)
      {
        return IrTypeId::fromValue(static_cast<std::uint32_t>(Index));
      }
    }
    IrType Type;
    Type.Kind = IrTypeKind::Bool;
    return appendEntry<IrTypeId>(Implementation->Storage->Types, Type);
  }

  IrTypeId IrBuilder::neverType()
  {
    Implementation->requireActive();
    for (std::size_t Index = 0; Index < Implementation->Storage->Types.size(); ++Index)
    {
      if (Implementation->Storage->Types[Index].Kind == IrTypeKind::Never)
      {
        return IrTypeId::fromValue(static_cast<std::uint32_t>(Index));
      }
    }
    IrType Type;
    Type.Kind = IrTypeKind::Never;
    return appendEntry<IrTypeId>(Implementation->Storage->Types, Type);
  }

  IrTypeId IrBuilder::integerType(std::uint16_t BitWidth, IrSignedness Signedness)
  {
    Implementation->requireActive();
    for (std::size_t Index = 0; Index < Implementation->Storage->Types.size(); ++Index)
    {
      const auto &Type = Implementation->Storage->Types[Index];
      if (Type.Kind == IrTypeKind::Integer && Type.BitWidth == BitWidth && Type.Signedness == Signedness)
      {
        return IrTypeId::fromValue(static_cast<std::uint32_t>(Index));
      }
    }
    IrType Type;
    Type.Kind = IrTypeKind::Integer;
    Type.BitWidth = BitWidth;
    Type.Signedness = Signedness;
    return appendEntry<IrTypeId>(Implementation->Storage->Types, Type);
  }

  IrTypeId IrBuilder::placeType(IrTypeId ElementType, IrPlaceAccess Access)
  {
    Implementation->requireActive();
    for (std::size_t Index = 0; Index < Implementation->Storage->Types.size(); ++Index)
    {
      const auto &Type = Implementation->Storage->Types[Index];
      if (Type.Kind == IrTypeKind::Place && Type.ElementType == ElementType && Type.Access == Access)
      {
        return IrTypeId::fromValue(static_cast<std::uint32_t>(Index));
      }
    }
    IrType Type;
    Type.Kind = IrTypeKind::Place;
    Type.ElementType = ElementType;
    Type.Access = Access;
    return appendEntry<IrTypeId>(Implementation->Storage->Types, Type);
  }

  IrTypeId IrBuilder::functionType(const std::vector<IrTypeId> &Parameters, std::optional<IrTypeId> Result)
  {
    Implementation->requireActive();
    for (std::size_t Index = 0; Index < Implementation->Storage->Types.size(); ++Index)
    {
      const auto &Type = Implementation->Storage->Types[Index];
      if (Type.Kind != IrTypeKind::Function || Type.Result != Result || Type.Parameters.Count != Parameters.size())
      {
        continue;
      }
      bool Matches = true;
      for (std::size_t ParameterIndex = 0; ParameterIndex < Parameters.size(); ++ParameterIndex)
      {
        if (Implementation->Storage->TypeReferences[Type.Parameters.First + ParameterIndex] != Parameters[ParameterIndex])
        {
          Matches = false;
          break;
        }
      }
      if (Matches)
      {
        return IrTypeId::fromValue(static_cast<std::uint32_t>(Index));
      }
    }
    IrType Type;
    Type.Kind = IrTypeKind::Function;
    Type.Parameters = {static_cast<std::uint32_t>(Implementation->Storage->TypeReferences.size()), static_cast<std::uint32_t>(Parameters.size())};
    Type.Result = Result;
    Implementation->Storage->TypeReferences.insert(Implementation->Storage->TypeReferences.end(), Parameters.begin(), Parameters.end());
    return appendEntry<IrTypeId>(Implementation->Storage->Types, Type);
  }

  IrConstantId IrBuilder::integerConstant(IrTypeId Type, std::uint64_t Bits)
  {
    Implementation->requireActive();
    for (std::size_t Index = 0; Index < Implementation->Storage->Constants.size(); ++Index)
    {
      const auto &Constant = Implementation->Storage->Constants[Index];
      if (Constant.Kind == IrConstantKind::Integer && Constant.Type == Type && Constant.Bits == Bits)
      {
        return IrConstantId::fromValue(static_cast<std::uint32_t>(Index));
      }
    }
    return appendEntry<IrConstantId>(Implementation->Storage->Constants, IrConstant{IrConstantKind::Integer, Type, Bits});
  }

  IrConstantId IrBuilder::boolConstant(bool Value)
  {
    Implementation->requireActive();
    const auto Type = boolType();
    const auto Bits = Value ? std::uint64_t{1} : std::uint64_t{0};
    for (std::size_t Index = 0; Index < Implementation->Storage->Constants.size(); ++Index)
    {
      const auto &Constant = Implementation->Storage->Constants[Index];
      if (Constant.Kind == IrConstantKind::Bool && Constant.Type == Type && Constant.Bits == Bits)
      {
        return IrConstantId::fromValue(static_cast<std::uint32_t>(Index));
      }
    }
    return appendEntry<IrConstantId>(Implementation->Storage->Constants, IrConstant{IrConstantKind::Bool, Type, Bits});
  }

  IrOriginId IrBuilder::addOrigin(core::SourceFileId File, core::SourceRange Range)
  {
    Implementation->requireActive();
    return appendEntry<IrOriginId>(Implementation->Storage->Origins, IrOrigin{File, Range});
  }

  IrFunctionId IrBuilder::addFunction(std::string Name, IrTypeId Signature, IrFunctionKind Kind, IrOriginId Origin)
  {
    Implementation->requireActive();
    IrFunction Function;
    Function.Name = std::move(Name);
    Function.Kind = Kind;
    Function.Signature = Signature;
    Function.Origin = Origin;
    const auto Id = appendEntry<IrFunctionId>(Implementation->Storage->Functions, std::move(Function));
    Implementation->FunctionBlocks.emplace_back();
    return Id;
  }

  IrBuiltBlock IrBuilder::addBlock(IrFunctionId Function, const std::vector<IrTypeId> &ArgumentTypes, IrOriginId Origin)
  {
    Implementation->requireActive();
    if (!Function.isValid() || Function.value() >= Implementation->Storage->Functions.size())
    {
      throw std::out_of_range("cannot add an InkIR block to an unknown function");
    }
    IrBlock Block;
    Block.OwnerFunction = Function;
    Block.Origin = Origin;
    Block.Arguments = {static_cast<std::uint32_t>(Implementation->Storage->Values.size()), static_cast<std::uint32_t>(ArgumentTypes.size())};
    const auto BlockId = appendEntry<IrBlockId>(Implementation->Storage->Blocks, Block);
    Implementation->BlockOperations.emplace_back();
    std::vector<IrValueId> Arguments;
    Arguments.reserve(ArgumentTypes.size());
    for (std::size_t Index = 0; Index < ArgumentTypes.size(); ++Index)
    {
      IrValue Value;
      Value.Type = ArgumentTypes[Index];
      Value.OwnerFunction = Function;
      Value.OwnerBlock = BlockId;
      Value.Origin = Origin;
      Value.DefinitionKind = IrValueDefinitionKind::BlockArgument;
      Value.DefinitionIndex = static_cast<std::uint32_t>(Index);
      Arguments.push_back(appendEntry<IrValueId>(Implementation->Storage->Values, Value));
    }
    Implementation->FunctionBlocks[Function.value()].push_back(BlockId);
    auto &FunctionRecord = Implementation->Storage->Functions[Function.value()];
    if (!FunctionRecord.EntryBlock.isValid())
    {
      FunctionRecord.EntryBlock = BlockId;
    }
    return {BlockId, std::move(Arguments)};
  }

  void IrBuilder::setEntryBlock(IrFunctionId Function, IrBlockId Block)
  {
    Implementation->requireActive();
    if (!Function.isValid() || Function.value() >= Implementation->Storage->Functions.size())
    {
      throw std::out_of_range("cannot set the entry block of an unknown InkIR function");
    }
    Implementation->Storage->Functions[Function.value()].EntryBlock = Block;
  }

  IrBuiltOperation IrBuilder::appendOperation(IrBlockId Block, IrOperationSpec Spec)
  {
    Implementation->requireActive();
    if (!Block.isValid() || Block.value() >= Implementation->Storage->Blocks.size())
    {
      throw std::out_of_range("cannot append an InkIR operation to an unknown block");
    }
    const auto Function = Implementation->Storage->Blocks[Block.value()].OwnerFunction;
    IrOperation Operation;
    Operation.Opcode = Spec.Opcode;
    Operation.OwnerFunction = Function;
    Operation.OwnerBlock = Block;
    Operation.Origin = Spec.Origin;
    Operation.Operands = appendValues(Implementation->Storage->OperationOperands, Spec.Operands);
    Operation.Results = {static_cast<std::uint32_t>(Implementation->Storage->OperationResults.size()), static_cast<std::uint32_t>(Spec.ResultTypes.size())};
    Operation.Successors = {static_cast<std::uint32_t>(Implementation->Storage->OperationSuccessors.size()), static_cast<std::uint32_t>(Spec.Successors.size())};
    Operation.Payload = std::move(Spec.Payload);
    const auto OperationId = appendEntry<IrOperationId>(Implementation->Storage->Operations, std::move(Operation));
    std::vector<IrValueId> Results;
    Results.reserve(Spec.ResultTypes.size());
    for (std::size_t Index = 0; Index < Spec.ResultTypes.size(); ++Index)
    {
      IrValue Value;
      Value.Type = Spec.ResultTypes[Index];
      Value.OwnerFunction = Function;
      Value.OwnerBlock = Block;
      Value.Origin = Spec.Origin;
      Value.DefinitionKind = IrValueDefinitionKind::OperationResult;
      Value.DefinitionIndex = static_cast<std::uint32_t>(Index);
      const auto ValueId = appendEntry<IrValueId>(Implementation->Storage->Values, Value);
      Results.push_back(ValueId);
      Implementation->Storage->OperationResults.push_back(ValueId);
    }
    for (const auto &SuccessorSpec : Spec.Successors)
    {
      IrSuccessor Successor;
      Successor.Block = SuccessorSpec.Block;
      Successor.Arguments = appendValues(Implementation->Storage->SuccessorArguments, SuccessorSpec.Arguments);
      Implementation->Storage->OperationSuccessors.push_back(Successor);
    }
    Implementation->BlockOperations[Block.value()].push_back(OperationId);
    return {OperationId, std::move(Results)};
  }

  IrValueId IrBuilder::createIntegerConstant(IrBlockId Block, IrConstantId Constant, IrOriginId Origin)
  {
    IrTypeId Type;
    if (Constant.isValid() && Constant.value() < Implementation->Storage->Constants.size())
    {
      Type = Implementation->Storage->Constants[Constant.value()].Type;
    }
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::ConstInt;
    Spec.Origin = Origin;
    Spec.ResultTypes = {Type};
    Spec.Payload = IrConstantPayload{Constant};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createBoolConstant(IrBlockId Block, IrConstantId Constant, IrOriginId Origin)
  {
    IrTypeId Type;
    if (Constant.isValid() && Constant.value() < Implementation->Storage->Constants.size())
    {
      Type = Implementation->Storage->Constants[Constant.value()].Type;
    }
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::ConstBool;
    Spec.Origin = Origin;
    Spec.ResultTypes = {Type};
    Spec.Payload = IrConstantPayload{Constant};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createIntegerBinary(IrBlockId Block, IrOpcode Opcode, IrValueId Left, IrValueId Right, IrOriginId Origin)
  {
    if (!isIntegerBinaryOpcode(Opcode))
    {
      throw std::invalid_argument("opcode is not an InkIR integer binary operation");
    }
    IrOperationSpec Spec;
    Spec.Opcode = Opcode;
    Spec.Origin = Origin;
    Spec.Operands = {Left, Right};
    Spec.ResultTypes = {Implementation->valueType(Left)};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createIntegerCompare(IrBlockId Block, IrComparePredicate Predicate, IrValueId Left, IrValueId Right, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::IntCompare;
    Spec.Origin = Origin;
    Spec.Operands = {Left, Right};
    Spec.ResultTypes = {boolType()};
    Spec.Payload = IrComparePayload{Predicate};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createIntegerNegate(IrBlockId Block, IrValueId Operand, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::IntNeg;
    Spec.Origin = Origin;
    Spec.Operands = {Operand};
    Spec.ResultTypes = {Implementation->valueType(Operand)};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createIntegerCast(IrBlockId Block, IrValueId Operand, IrTypeId ResultType, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::CastInt;
    Spec.Origin = Origin;
    Spec.Operands = {Operand};
    Spec.ResultTypes = {ResultType};
    Spec.Payload = IrTypePayload{ResultType};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createBoolUnary(IrBlockId Block, IrOpcode Opcode, IrValueId Operand, IrOriginId Origin)
  {
    if (Opcode != IrOpcode::BoolNot)
    {
      throw std::invalid_argument("opcode is not an InkIR boolean unary operation");
    }
    IrOperationSpec Spec;
    Spec.Opcode = Opcode;
    Spec.Origin = Origin;
    Spec.Operands = {Operand};
    Spec.ResultTypes = {boolType()};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createBoolBinary(IrBlockId Block, IrOpcode Opcode, IrValueId Left, IrValueId Right, IrOriginId Origin)
  {
    if (Opcode != IrOpcode::BoolAnd && Opcode != IrOpcode::BoolOr)
    {
      throw std::invalid_argument("opcode is not an InkIR boolean binary operation");
    }
    IrOperationSpec Spec;
    Spec.Opcode = Opcode;
    Spec.Origin = Origin;
    Spec.Operands = {Left, Right};
    Spec.ResultTypes = {boolType()};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createAlloca(IrBlockId Block, IrTypeId ElementType, IrPlaceAccess Access, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::Alloca;
    Spec.Origin = Origin;
    Spec.ResultTypes = {placeType(ElementType, Access)};
    Spec.Payload = IrTypePayload{ElementType};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrValueId IrBuilder::createLoad(IrBlockId Block, IrValueId Place, IrOriginId Origin)
  {
    IrTypeId ElementType;
    const auto *Type = Implementation->findType(Implementation->valueType(Place));
    if (Type != nullptr && Type->Kind == IrTypeKind::Place)
    {
      ElementType = Type->ElementType;
    }
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::Load;
    Spec.Origin = Origin;
    Spec.Operands = {Place};
    Spec.ResultTypes = {ElementType};
    return appendOperation(Block, std::move(Spec)).Results.front();
  }

  IrOperationId IrBuilder::createStore(IrBlockId Block, IrValueId Place, IrValueId Value, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::Store;
    Spec.Origin = Origin;
    Spec.Operands = {Place, Value};
    return appendOperation(Block, std::move(Spec)).Operation;
  }

  IrBuiltOperation IrBuilder::createDirectCall(IrBlockId Block, IrFunctionId Callee, const std::vector<IrValueId> &Arguments, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::DirectCall;
    Spec.Origin = Origin;
    Spec.Operands = Arguments;
    Spec.Payload = IrDirectCallPayload{Callee};
    if (Callee.isValid() && Callee.value() < Implementation->Storage->Functions.size())
    {
      const auto *Signature = Implementation->findType(Implementation->Storage->Functions[Callee.value()].Signature);
      if (Signature != nullptr && Signature->Kind == IrTypeKind::Function && Signature->Result && Implementation->findType(*Signature->Result) != nullptr && Implementation->findType(*Signature->Result)->Kind != IrTypeKind::Never)
      {
        Spec.ResultTypes = {*Signature->Result};
      }
    }
    return appendOperation(Block, std::move(Spec));
  }

  IrOperationId IrBuilder::createBranch(IrBlockId Block, IrBlockId Target, const std::vector<IrValueId> &Arguments, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::Branch;
    Spec.Origin = Origin;
    Spec.Successors = {{Target, Arguments}};
    return appendOperation(Block, std::move(Spec)).Operation;
  }

  IrOperationId IrBuilder::createConditionalBranch(IrBlockId Block, IrValueId Condition, IrBlockId TrueTarget, const std::vector<IrValueId> &TrueArguments, IrBlockId FalseTarget, const std::vector<IrValueId> &FalseArguments, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::CondBranch;
    Spec.Origin = Origin;
    Spec.Operands = {Condition};
    Spec.Successors = {{TrueTarget, TrueArguments}, {FalseTarget, FalseArguments}};
    return appendOperation(Block, std::move(Spec)).Operation;
  }

  IrOperationId IrBuilder::createReturn(IrBlockId Block, std::optional<IrValueId> Value, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::Return;
    Spec.Origin = Origin;
    if (Value)
    {
      Spec.Operands = {*Value};
    }
    return appendOperation(Block, std::move(Spec)).Operation;
  }

  IrOperationId IrBuilder::createTrap(IrBlockId Block, IrTrapKind Kind, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::Trap;
    Spec.Origin = Origin;
    Spec.Payload = IrTrapPayload{Kind};
    return appendOperation(Block, std::move(Spec)).Operation;
  }

  IrOperationId IrBuilder::createUnreachable(IrBlockId Block, IrOriginId Origin)
  {
    IrOperationSpec Spec;
    Spec.Opcode = IrOpcode::Unreachable;
    Spec.Origin = Origin;
    return appendOperation(Block, std::move(Spec)).Operation;
  }

  IrPlanNodeId IrBuilder::addForceValuePlan(IrValueId Input, IrValueId Output, IrOriginId Origin)
  {
    Implementation->requireActive();
    IrTypeId ResultType;
    if (Input.isValid() && Input.value() < Implementation->Storage->Values.size())
    {
      ResultType = Implementation->Storage->Values[Input.value()].Type;
    }
    return appendEntry<IrPlanNodeId>(Implementation->Storage->PlanNodes, IrPlanNode{IrPlanOpcode::ForceValue, Input, Output, ResultType, Origin});
  }

  UnverifiedStagedModule IrBuilder::finish()
  {
    Implementation->requireActive();
    for (std::size_t FunctionIndex = 0; FunctionIndex < Implementation->Storage->Functions.size(); ++FunctionIndex)
    {
      auto &Function = Implementation->Storage->Functions[FunctionIndex];
      const auto &Blocks = Implementation->FunctionBlocks[FunctionIndex];
      Function.Blocks = {static_cast<std::uint32_t>(Implementation->Storage->FunctionBlocks.size()), static_cast<std::uint32_t>(Blocks.size())};
      Implementation->Storage->FunctionBlocks.insert(Implementation->Storage->FunctionBlocks.end(), Blocks.begin(), Blocks.end());
    }
    for (std::size_t BlockIndex = 0; BlockIndex < Implementation->Storage->Blocks.size(); ++BlockIndex)
    {
      auto &Block = Implementation->Storage->Blocks[BlockIndex];
      const auto &Operations = Implementation->BlockOperations[BlockIndex];
      Block.Operations = {static_cast<std::uint32_t>(Implementation->Storage->BlockOperations.size()), static_cast<std::uint32_t>(Operations.size())};
      Implementation->Storage->BlockOperations.insert(Implementation->Storage->BlockOperations.end(), Operations.begin(), Operations.end());
    }
    Implementation->Finished = true;
    std::shared_ptr<const detail::IrModuleStorage> Storage = Implementation->Storage;
    return UnverifiedStagedModule(IrModule(std::move(Storage)));
  }
} // namespace ink::ir
