#ifndef INK_IR_MODULE_H
#define INK_IR_MODULE_H

#include "ink/core/source_file_id.h"
#include "ink/core/source_range.h"
#include "ink/ir/operation.h"
#include "ink/ir/plan.h"
#include "ink/ir/type.h"
#include "ink/target/target_key.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ink::ir
{
  enum class IrValueDefinitionKind : std::uint8_t
  {
    Unknown,
    BlockArgument,
    OperationResult,
  };

  struct IrOrigin
  {
    core::SourceFileId File;
    core::SourceRange Range;
  };

  struct IrValue
  {
    IrTypeId Type;
    IrFunctionId OwnerFunction;
    IrBlockId OwnerBlock;
    IrOriginId Origin;
    IrValueDefinitionKind DefinitionKind = IrValueDefinitionKind::Unknown;
    std::uint32_t DefinitionIndex = 0;
  };

  struct IrBlock
  {
    IrFunctionId OwnerFunction;
    IrOriginId Origin;
    IrTableRange Arguments;
    IrTableRange Operations;
  };

  enum class IrFunctionKind : std::uint8_t
  {
    Definition,
    External,
  };

  struct IrFunction
  {
    std::string Name;
    IrFunctionKind Kind = IrFunctionKind::Definition;
    IrTypeId Signature;
    IrOriginId Origin;
    IrBlockId EntryBlock;
    IrTableRange Blocks;
  };

  namespace detail
  {
    struct IrModuleStorage;
  }

  class IrModule
  {
  public:
    IrModule() noexcept = default;

    bool isValid() const noexcept;

    std::size_t typeCount() const noexcept;
    std::size_t constantCount() const noexcept;
    std::size_t originCount() const noexcept;
    std::size_t functionCount() const noexcept;
    std::size_t blockCount() const noexcept;
    std::size_t operationCount() const noexcept;
    std::size_t valueCount() const noexcept;
    std::size_t planNodeCount() const noexcept;

    bool contains(IrTypeId Id) const noexcept;
    bool contains(IrConstantId Id) const noexcept;
    bool contains(IrOriginId Id) const noexcept;
    bool contains(IrFunctionId Id) const noexcept;
    bool contains(IrBlockId Id) const noexcept;
    bool contains(IrOperationId Id) const noexcept;
    bool contains(IrValueId Id) const noexcept;
    bool contains(IrPlanNodeId Id) const noexcept;

    const IrType &type(IrTypeId Id) const;
    const IrConstant &constant(IrConstantId Id) const;
    const IrOrigin &origin(IrOriginId Id) const;
    const IrFunction &function(IrFunctionId Id) const;
    const IrBlock &block(IrBlockId Id) const;
    const IrOperation &operation(IrOperationId Id) const;
    const IrValue &value(IrValueId Id) const;
    const IrPlanNode &planNode(IrPlanNodeId Id) const;

    IrTypeId typeReference(std::uint32_t Index) const;
    IrBlockId functionBlock(std::uint32_t Index) const;
    IrOperationId blockOperation(std::uint32_t Index) const;
    IrValueId operationOperand(std::uint32_t Index) const;
    IrValueId operationResult(std::uint32_t Index) const;
    const IrSuccessor &operationSuccessor(std::uint32_t Index) const;
    IrValueId successorArgument(std::uint32_t Index) const;

  private:
    explicit IrModule(std::shared_ptr<const detail::IrModuleStorage> Storage) noexcept;

    std::shared_ptr<const detail::IrModuleStorage> Storage;

    friend class IrBuilder;
    friend class IrVerifier;
  };

  class UnverifiedStagedModule
  {
  public:
    const IrModule &module() const noexcept;

  private:
    explicit UnverifiedStagedModule(IrModule Module) noexcept;

    IrModule Module;

    friend class IrBuilder;
    friend class IrVerifier;
  };

  class VerifiedStagedModule
  {
  public:
    const IrModule &module() const noexcept;

  private:
    explicit VerifiedStagedModule(IrModule Module) noexcept;

    IrModule Module;

    friend class IrVerifier;
  };

  class VerifiedClosedModule
  {
  public:
    const IrModule &module() const noexcept;
    const target::TargetKey &targetKey() const noexcept;

  private:
    VerifiedClosedModule(IrModule Module, target::TargetKey TargetKey) noexcept;

    IrModule Module;
    target::TargetKey Target;

    friend class IrVerifier;
  };
} // namespace ink::ir

#endif
