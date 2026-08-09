#include "ink/ir/module.h"

#include "ir_storage.h"

#include <stdexcept>
#include <utility>

namespace ink::ir
{
  namespace
  {
    const detail::IrModuleStorage &requireStorage(const std::shared_ptr<const detail::IrModuleStorage> &Storage)
    {
      if (!Storage)
      {
        throw std::logic_error("IR module has no storage");
      }
      return *Storage;
    }

    template <typename Id>
    bool containsId(Id IdValue, std::size_t Size) noexcept
    {
      return IdValue.isValid() && static_cast<std::size_t>(IdValue.value()) < Size;
    }
  }

  IrModule::IrModule(std::shared_ptr<const detail::IrModuleStorage> StorageValue) noexcept : Storage(std::move(StorageValue))
  {
  }

  bool IrModule::isValid() const noexcept
  {
    return static_cast<bool>(Storage);
  }

  std::size_t IrModule::typeCount() const noexcept
  {
    return Storage ? Storage->Types.size() : 0;
  }

  std::size_t IrModule::constantCount() const noexcept
  {
    return Storage ? Storage->Constants.size() : 0;
  }

  std::size_t IrModule::originCount() const noexcept
  {
    return Storage ? Storage->Origins.size() : 0;
  }

  std::size_t IrModule::functionCount() const noexcept
  {
    return Storage ? Storage->Functions.size() : 0;
  }

  std::size_t IrModule::blockCount() const noexcept
  {
    return Storage ? Storage->Blocks.size() : 0;
  }

  std::size_t IrModule::operationCount() const noexcept
  {
    return Storage ? Storage->Operations.size() : 0;
  }

  std::size_t IrModule::valueCount() const noexcept
  {
    return Storage ? Storage->Values.size() : 0;
  }

  std::size_t IrModule::planNodeCount() const noexcept
  {
    return Storage ? Storage->PlanNodes.size() : 0;
  }

  bool IrModule::contains(IrTypeId Id) const noexcept
  {
    return containsId(Id, typeCount());
  }

  bool IrModule::contains(IrConstantId Id) const noexcept
  {
    return containsId(Id, constantCount());
  }

  bool IrModule::contains(IrOriginId Id) const noexcept
  {
    return containsId(Id, originCount());
  }

  bool IrModule::contains(IrFunctionId Id) const noexcept
  {
    return containsId(Id, functionCount());
  }

  bool IrModule::contains(IrBlockId Id) const noexcept
  {
    return containsId(Id, blockCount());
  }

  bool IrModule::contains(IrOperationId Id) const noexcept
  {
    return containsId(Id, operationCount());
  }

  bool IrModule::contains(IrValueId Id) const noexcept
  {
    return containsId(Id, valueCount());
  }

  bool IrModule::contains(IrPlanNodeId Id) const noexcept
  {
    return containsId(Id, planNodeCount());
  }

  const IrType &IrModule::type(IrTypeId Id) const
  {
    return requireStorage(Storage).Types.at(Id.value());
  }

  const IrConstant &IrModule::constant(IrConstantId Id) const
  {
    return requireStorage(Storage).Constants.at(Id.value());
  }

  const IrOrigin &IrModule::origin(IrOriginId Id) const
  {
    return requireStorage(Storage).Origins.at(Id.value());
  }

  const IrFunction &IrModule::function(IrFunctionId Id) const
  {
    return requireStorage(Storage).Functions.at(Id.value());
  }

  const IrBlock &IrModule::block(IrBlockId Id) const
  {
    return requireStorage(Storage).Blocks.at(Id.value());
  }

  const IrOperation &IrModule::operation(IrOperationId Id) const
  {
    return requireStorage(Storage).Operations.at(Id.value());
  }

  const IrValue &IrModule::value(IrValueId Id) const
  {
    return requireStorage(Storage).Values.at(Id.value());
  }

  const IrPlanNode &IrModule::planNode(IrPlanNodeId Id) const
  {
    return requireStorage(Storage).PlanNodes.at(Id.value());
  }

  IrTypeId IrModule::typeReference(std::uint32_t Index) const
  {
    return requireStorage(Storage).TypeReferences.at(Index);
  }

  IrBlockId IrModule::functionBlock(std::uint32_t Index) const
  {
    return requireStorage(Storage).FunctionBlocks.at(Index);
  }

  IrOperationId IrModule::blockOperation(std::uint32_t Index) const
  {
    return requireStorage(Storage).BlockOperations.at(Index);
  }

  IrValueId IrModule::operationOperand(std::uint32_t Index) const
  {
    return requireStorage(Storage).OperationOperands.at(Index);
  }

  IrValueId IrModule::operationResult(std::uint32_t Index) const
  {
    return requireStorage(Storage).OperationResults.at(Index);
  }

  const IrSuccessor &IrModule::operationSuccessor(std::uint32_t Index) const
  {
    return requireStorage(Storage).OperationSuccessors.at(Index);
  }

  IrValueId IrModule::successorArgument(std::uint32_t Index) const
  {
    return requireStorage(Storage).SuccessorArguments.at(Index);
  }

  UnverifiedStagedModule::UnverifiedStagedModule(IrModule ModuleValue) noexcept : Module(std::move(ModuleValue))
  {
  }

  const IrModule &UnverifiedStagedModule::module() const noexcept
  {
    return Module;
  }

  VerifiedStagedModule::VerifiedStagedModule(IrModule ModuleValue) noexcept : Module(std::move(ModuleValue))
  {
  }

  const IrModule &VerifiedStagedModule::module() const noexcept
  {
    return Module;
  }

  VerifiedClosedModule::VerifiedClosedModule(IrModule ModuleValue, target::TargetKey TargetKey) noexcept : Module(std::move(ModuleValue)), Target(std::move(TargetKey))
  {
  }

  const IrModule &VerifiedClosedModule::module() const noexcept
  {
    return Module;
  }

  const target::TargetKey &VerifiedClosedModule::targetKey() const noexcept
  {
    return Target;
  }
} // namespace ink::ir
