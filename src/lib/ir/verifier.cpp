#include "ink/ir/verifier.h"

#include "ir_storage.h"

#include <algorithm>
#include <deque>
#include <stdexcept>
#include <utility>
#include <variant>

namespace ink::ir
{
  class IrVerifier
  {
  public:
    static const detail::IrModuleStorage &storage(const IrModule &Module)
    {
      return *Module.Storage;
    }

    static VerifiedStagedModule staged(IrModule Module)
    {
      return VerifiedStagedModule(std::move(Module));
    }

    static VerifiedClosedModule closed(IrModule Module, target::TargetKey TargetKey)
    {
      return VerifiedClosedModule(std::move(Module), std::move(TargetKey));
    }

    static IrModule applyForceValueResolutions(const IrModule &Module, const std::vector<IrForceValueResolution> &Resolutions)
    {
      auto Storage = std::make_shared<detail::IrModuleStorage>(*Module.Storage);
      const auto findOrAddConstant = [&](const IrForceValueResolution &Resolution)
      {
        for (std::size_t Index = 0; Index < Storage->Constants.size(); ++Index)
        {
          const IrConstant &Constant = Storage->Constants[Index];
          if (Constant.Type == Resolution.Type && Constant.Kind == Resolution.Kind && Constant.Bits == Resolution.Bits)
          {
            return IrConstantId::fromValue(static_cast<std::uint32_t>(Index));
          }
        }
        const IrConstantId Constant = IrConstantId::fromValue(static_cast<std::uint32_t>(Storage->Constants.size()));
        Storage->Constants.push_back({Resolution.Kind, Resolution.Type, Resolution.Bits});
        return Constant;
      };

      for (std::size_t PlanIndex = 0; PlanIndex < Storage->PlanNodes.size(); ++PlanIndex)
      {
        const IrPlanNode &Plan = Storage->PlanNodes[PlanIndex];
        const IrForceValueResolution &Resolution = Resolutions[PlanIndex];
        const IrConstantId Constant = findOrAddConstant(Resolution);
        bool FoundDefinition = false;
        const IrValue &Output = Storage->Values[Plan.Output.value()];
        const IrBlock &OutputBlock = Storage->Blocks[Output.OwnerBlock.value()];
        for (std::uint32_t BlockOperationIndex = OutputBlock.Operations.First; BlockOperationIndex < OutputBlock.Operations.end(); ++BlockOperationIndex)
        {
          IrOperation &CandidateOperation = Storage->Operations[Storage->BlockOperations[BlockOperationIndex].value()];
          for (std::uint32_t ResultIndex = CandidateOperation.Results.First; ResultIndex < CandidateOperation.Results.end(); ++ResultIndex)
          {
            if (Storage->OperationResults[ResultIndex] == Plan.Output)
            {
              CandidateOperation.Opcode = Resolution.Kind == IrConstantKind::Bool ? IrOpcode::ConstBool : IrOpcode::ConstInt;
              CandidateOperation.Payload = IrConstantPayload{Constant};
              FoundDefinition = true;
              break;
            }
          }
          if (FoundDefinition)
          {
            break;
          }
        }
        if (!FoundDefinition)
        {
          throw std::logic_error("verified force-value output has no defining operation");
        }
      }
      Storage->PlanNodes.clear();
      std::shared_ptr<const detail::IrModuleStorage> ImmutableStorage = std::move(Storage);
      return IrModule(std::move(ImmutableStorage));
    }
  };

  namespace
  {
    constexpr std::uint8_t VariadicArity = 255;

    bool rangeWithin(IrTableRange Range, std::size_t Size) noexcept
    {
      return static_cast<std::uint64_t>(Range.First) + static_cast<std::uint64_t>(Range.Count) <= Size;
    }

    bool isOptionalOriginValid(const detail::IrModuleStorage &Storage, IrOriginId Origin) noexcept
    {
      return !Origin.isValid() || Origin.value() < Storage.Origins.size();
    }

    bool isSignedPredicate(IrComparePredicate Predicate) noexcept
    {
      return Predicate == IrComparePredicate::SignedLess || Predicate == IrComparePredicate::SignedLessEqual || Predicate == IrComparePredicate::SignedGreater || Predicate == IrComparePredicate::SignedGreaterEqual;
    }

    bool isUnsignedPredicate(IrComparePredicate Predicate) noexcept
    {
      return Predicate == IrComparePredicate::UnsignedLess || Predicate == IrComparePredicate::UnsignedLessEqual || Predicate == IrComparePredicate::UnsignedGreater || Predicate == IrComparePredicate::UnsignedGreaterEqual;
    }

    bool isEqualityPredicate(IrComparePredicate Predicate) noexcept
    {
      return Predicate == IrComparePredicate::Equal || Predicate == IrComparePredicate::NotEqual;
    }

    bool isValidSignedness(IrSignedness Signedness) noexcept
    {
      switch (Signedness)
      {
      case IrSignedness::Signless:
      case IrSignedness::Signed:
      case IrSignedness::Unsigned:
        return true;
      }
      return false;
    }

    bool isValidPlaceAccess(IrPlaceAccess Access) noexcept
    {
      switch (Access)
      {
      case IrPlaceAccess::ReadOnly:
      case IrPlaceAccess::ReadWrite:
        return true;
      }
      return false;
    }

    bool isValidComparePredicate(IrComparePredicate Predicate) noexcept
    {
      switch (Predicate)
      {
      case IrComparePredicate::Equal:
      case IrComparePredicate::NotEqual:
      case IrComparePredicate::SignedLess:
      case IrComparePredicate::SignedLessEqual:
      case IrComparePredicate::SignedGreater:
      case IrComparePredicate::SignedGreaterEqual:
      case IrComparePredicate::UnsignedLess:
      case IrComparePredicate::UnsignedLessEqual:
      case IrComparePredicate::UnsignedGreater:
      case IrComparePredicate::UnsignedGreaterEqual:
        return true;
      }
      return false;
    }

    bool isValidTrapKind(IrTrapKind Kind) noexcept
    {
      switch (Kind)
      {
      case IrTrapKind::User:
      case IrTrapKind::Bounds:
      case IrTrapKind::DivisionByZero:
      case IrTrapKind::Overflow:
      case IrTrapKind::Unreachable:
        return true;
      }
      return false;
    }

    bool isIntegerBinary(IrOpcode Opcode) noexcept
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

    bool payloadMatches(IrPayloadKind Kind, const IrOperationPayload &Payload) noexcept
    {
      switch (Kind)
      {
      case IrPayloadKind::None:
        return std::holds_alternative<IrNoPayload>(Payload);
      case IrPayloadKind::Constant:
        return std::holds_alternative<IrConstantPayload>(Payload);
      case IrPayloadKind::Compare:
        return std::holds_alternative<IrComparePayload>(Payload);
      case IrPayloadKind::Type:
        return std::holds_alternative<IrTypePayload>(Payload);
      case IrPayloadKind::DirectCall:
        return std::holds_alternative<IrDirectCallPayload>(Payload);
      case IrPayloadKind::Trap:
        return std::holds_alternative<IrTrapPayload>(Payload);
      }
      return false;
    }

    class VerificationState
    {
    public:
      VerificationState(const IrModule &ModuleValue, IrStage StageValue) : Module(ModuleValue), Storage(IrVerifier::storage(ModuleValue)), Stage(StageValue), ValueDefinitions(Storage.Values.size()), OperationPositions(Storage.Operations.size())
      {
      }

      std::vector<IrVerificationError> run()
      {
        verifyTypes();
        verifyConstantsAndOrigins();
        verifyOwnershipTables();
        verifyFunctionsAndBlocks();
        verifyOperations();
        verifyPlans();
        verifyControlFlowAndDominance();
        return std::move(Errors);
      }

    private:
      struct ValueDefinition
      {
        IrBlockId Block;
        IrOperationId Operation;
        bool Defined = false;
      };

      const IrModule &Module;
      const detail::IrModuleStorage &Storage;
      IrStage Stage;
      std::vector<IrVerificationError> Errors;
      std::vector<ValueDefinition> ValueDefinitions;
      std::vector<std::uint32_t> OperationPositions;

      void error(IrVerificationErrorCode Code, std::string Message, IrFunctionId Function = {}, IrBlockId Block = {}, IrOperationId Operation = {}, IrValueId Value = {})
      {
        Errors.push_back({Code, std::move(Message), Function, Block, Operation, Value});
      }

      template <typename Id>
      bool contains(Id IdValue, std::size_t Size) const noexcept
      {
        return IdValue.isValid() && IdValue.value() < Size;
      }

      const IrType *findType(IrTypeId Type) const noexcept
      {
        if (!contains(Type, Storage.Types.size()))
        {
          return nullptr;
        }
        return &Storage.Types[Type.value()];
      }

      const IrValue *findValue(IrValueId Value) const noexcept
      {
        if (!contains(Value, Storage.Values.size()))
        {
          return nullptr;
        }
        return &Storage.Values[Value.value()];
      }

      IrValueId operand(const IrOperation &Operation, std::uint32_t Index) const
      {
        return Storage.OperationOperands[Operation.Operands.First + Index];
      }

      IrValueId result(const IrOperation &Operation, std::uint32_t Index) const
      {
        return Storage.OperationResults[Operation.Results.First + Index];
      }

      bool hasValueType(IrValueId Value, IrTypeId Type) const noexcept
      {
        const auto *ValueRecord = findValue(Value);
        return ValueRecord != nullptr && ValueRecord->Type == Type;
      }

      bool isTypeKind(IrTypeId Type, IrTypeKind Kind) const noexcept
      {
        const auto *TypeRecord = findType(Type);
        return TypeRecord != nullptr && TypeRecord->Kind == Kind;
      }

      void claimRange(IrTableRange Range, std::size_t Size, std::vector<std::uint8_t> &Claims, const char *TableName, IrVerificationErrorCode Code, IrFunctionId Function = {}, IrBlockId Block = {}, IrOperationId Operation = {})
      {
        if (!rangeWithin(Range, Size))
        {
          error(Code, std::string(TableName) + " range is outside its owning table", Function, Block, Operation);
          return;
        }
        for (std::uint32_t Index = Range.First; Index < Range.end(); ++Index)
        {
          if (Claims[Index] != 0)
          {
            error(Code, std::string(TableName) + " entry is owned by more than one range", Function, Block, Operation);
          }
          ++Claims[Index];
        }
      }

      void requireFullyClaimed(const std::vector<std::uint8_t> &Claims, const char *TableName)
      {
        if (std::any_of(Claims.begin(), Claims.end(), [](std::uint8_t Claim) { return Claim != 1; }))
        {
          error(IrVerificationErrorCode::InvalidTableReference, std::string(TableName) + " is not owned exactly once");
        }
      }

      void verifyTypes()
      {
        std::vector<std::uint8_t> TypeReferenceClaims(Storage.TypeReferences.size());
        for (std::size_t Index = 0; Index < Storage.Types.size(); ++Index)
        {
          const auto TypeId = IrTypeId::fromValue(static_cast<std::uint32_t>(Index));
          const auto &Type = Storage.Types[Index];
          if (!isValidSignedness(Type.Signedness))
          {
            error(IrVerificationErrorCode::InvalidType, "type has an out-of-domain signedness");
          }
          if (!isValidPlaceAccess(Type.Access))
          {
            error(IrVerificationErrorCode::InvalidType, "type has an out-of-domain place access");
          }
          switch (Type.Kind)
          {
          case IrTypeKind::Unit:
          case IrTypeKind::Never:
          case IrTypeKind::Bool:
            break;
          case IrTypeKind::Integer:
            if ((Type.BitWidth != 32 && Type.BitWidth != 64) || (Type.Signedness != IrSignedness::Signed && Type.Signedness != IrSignedness::Unsigned))
            {
              error(IrVerificationErrorCode::InvalidType, "first-slice integer type must be i32, i64, u32, or u64");
            }
            break;
          case IrTypeKind::Place:
            if (!contains(Type.ElementType, Storage.Types.size()) || isTypeKind(Type.ElementType, IrTypeKind::Never) || isTypeKind(Type.ElementType, IrTypeKind::Function) || isTypeKind(Type.ElementType, IrTypeKind::Place) || !isValidPlaceAccess(Type.Access))
            {
              error(IrVerificationErrorCode::InvalidType, "place element or access is not valid for the first slice");
            }
            break;
          case IrTypeKind::Function:
            claimRange(Type.Parameters, Storage.TypeReferences.size(), TypeReferenceClaims, "function parameter type", IrVerificationErrorCode::InvalidType);
            if (rangeWithin(Type.Parameters, Storage.TypeReferences.size()))
            {
              for (std::uint32_t ParameterIndex = Type.Parameters.First; ParameterIndex < Type.Parameters.end(); ++ParameterIndex)
              {
                const auto Parameter = Storage.TypeReferences[ParameterIndex];
                if (!contains(Parameter, Storage.Types.size()) || isTypeKind(Parameter, IrTypeKind::Never) || isTypeKind(Parameter, IrTypeKind::Function) || isTypeKind(Parameter, IrTypeKind::Place))
                {
                  error(IrVerificationErrorCode::InvalidType, "function parameter has an invalid first-slice type");
                }
              }
            }
            if (Type.Result && (!contains(*Type.Result, Storage.Types.size()) || isTypeKind(*Type.Result, IrTypeKind::Function) || isTypeKind(*Type.Result, IrTypeKind::Place)))
            {
              error(IrVerificationErrorCode::InvalidType, "function result has an invalid first-slice type");
            }
            break;
          case IrTypeKind::Unknown:
            error(IrVerificationErrorCode::InvalidType, "type table contains an unknown type");
            break;
          default:
            error(IrVerificationErrorCode::InvalidType, "type table contains an out-of-domain type kind");
            break;
          }
          if (Type.Kind != IrTypeKind::Integer && Type.Signedness != IrSignedness::Signless)
          {
            error(IrVerificationErrorCode::InvalidType, "only integer types may carry signedness");
          }
          if (Type.Kind != IrTypeKind::Place && Type.Access != IrPlaceAccess::ReadOnly)
          {
            error(IrVerificationErrorCode::InvalidType, "only place types may carry non-default access");
          }
          if (Type.Kind != IrTypeKind::Place && Type.ElementType.isValid())
          {
            error(IrVerificationErrorCode::InvalidType, "non-place type carries a place element reference");
          }
          if (Type.Kind != IrTypeKind::Function && (!Type.Parameters.empty() || Type.Result))
          {
            error(IrVerificationErrorCode::InvalidType, "non-function type carries signature data");
          }
          static_cast<void>(TypeId);
        }
        requireFullyClaimed(TypeReferenceClaims, "type reference table");
      }

      void verifyConstantsAndOrigins()
      {
        for (const auto &Constant : Storage.Constants)
        {
          const auto *Type = findType(Constant.Type);
          if (Type == nullptr)
          {
            error(IrVerificationErrorCode::InvalidConstant, "constant references an unknown type");
            continue;
          }
          if (Constant.Kind == IrConstantKind::Integer)
          {
            if (Type->Kind != IrTypeKind::Integer)
            {
              error(IrVerificationErrorCode::InvalidConstant, "integer constant does not have an integer type");
            }
            else if (Type->BitWidth < 64 && (Constant.Bits >> Type->BitWidth) != 0)
            {
              error(IrVerificationErrorCode::InvalidConstant, "integer constant has bits outside its declared width");
            }
          }
          else if (Constant.Kind == IrConstantKind::Bool)
          {
            if (Type->Kind != IrTypeKind::Bool || Constant.Bits > 1)
            {
              error(IrVerificationErrorCode::InvalidConstant, "boolean constant is not one of the two boolean values");
            }
          }
          else
          {
            error(IrVerificationErrorCode::InvalidConstant, "constant table contains an unknown constant kind");
          }
        }
        for (const auto &Origin : Storage.Origins)
        {
          if (!Origin.File.isValid() || Origin.Range.Start > Origin.Range.End)
          {
            error(IrVerificationErrorCode::InvalidTableReference, "origin has an invalid source file or source range");
          }
        }
      }

      void verifyOwnershipTables()
      {
        std::vector<std::uint8_t> FunctionBlockClaims(Storage.FunctionBlocks.size());
        std::vector<std::uint8_t> BlockOperationClaims(Storage.BlockOperations.size());
        std::vector<std::uint8_t> OperandClaims(Storage.OperationOperands.size());
        std::vector<std::uint8_t> ResultClaims(Storage.OperationResults.size());
        std::vector<std::uint8_t> SuccessorClaims(Storage.OperationSuccessors.size());
        std::vector<std::uint8_t> SuccessorArgumentClaims(Storage.SuccessorArguments.size());
        std::vector<std::uint8_t> BlockClaims(Storage.Blocks.size());
        std::vector<std::uint8_t> OperationClaims(Storage.Operations.size());
        std::vector<std::uint8_t> ValueClaims(Storage.Values.size());

        for (std::size_t FunctionIndex = 0; FunctionIndex < Storage.Functions.size(); ++FunctionIndex)
        {
          const auto FunctionId = IrFunctionId::fromValue(static_cast<std::uint32_t>(FunctionIndex));
          const auto &Function = Storage.Functions[FunctionIndex];
          claimRange(Function.Blocks, Storage.FunctionBlocks.size(), FunctionBlockClaims, "function block", IrVerificationErrorCode::InvalidFunction, FunctionId);
          if (!rangeWithin(Function.Blocks, Storage.FunctionBlocks.size()))
          {
            continue;
          }
          for (std::uint32_t Index = Function.Blocks.First; Index < Function.Blocks.end(); ++Index)
          {
            const auto Block = Storage.FunctionBlocks[Index];
            if (!contains(Block, Storage.Blocks.size()))
            {
              error(IrVerificationErrorCode::InvalidTableReference, "function references an unknown block", FunctionId);
              continue;
            }
            ++BlockClaims[Block.value()];
            if (Storage.Blocks[Block.value()].OwnerFunction != FunctionId)
            {
              error(IrVerificationErrorCode::InvalidOwner, "block owner does not match the function block table", FunctionId, Block);
            }
          }
        }

        for (std::size_t BlockIndex = 0; BlockIndex < Storage.Blocks.size(); ++BlockIndex)
        {
          const auto BlockId = IrBlockId::fromValue(static_cast<std::uint32_t>(BlockIndex));
          const auto &Block = Storage.Blocks[BlockIndex];
          claimRange(Block.Operations, Storage.BlockOperations.size(), BlockOperationClaims, "block operation", IrVerificationErrorCode::InvalidBlock, Block.OwnerFunction, BlockId);
          if (rangeWithin(Block.Arguments, Storage.Values.size()))
          {
            for (std::uint32_t ArgumentIndex = 0; ArgumentIndex < Block.Arguments.Count; ++ArgumentIndex)
            {
              const auto ValueId = IrValueId::fromValue(Block.Arguments.First + ArgumentIndex);
              ++ValueClaims[ValueId.value()];
              ValueDefinitions[ValueId.value()] = {BlockId, {}, true};
              const auto &Value = Storage.Values[ValueId.value()];
              if (Value.DefinitionKind != IrValueDefinitionKind::BlockArgument || Value.DefinitionIndex != ArgumentIndex || Value.OwnerBlock != BlockId || Value.OwnerFunction != Block.OwnerFunction)
              {
                error(IrVerificationErrorCode::InvalidOwner, "block argument value has inconsistent definition data", Block.OwnerFunction, BlockId, {}, ValueId);
              }
            }
          }
          else
          {
            error(IrVerificationErrorCode::InvalidTableReference, "block argument range is outside the value table", Block.OwnerFunction, BlockId);
          }
          if (!rangeWithin(Block.Operations, Storage.BlockOperations.size()))
          {
            continue;
          }
          for (std::uint32_t Position = 0; Position < Block.Operations.Count; ++Position)
          {
            const auto OperationId = Storage.BlockOperations[Block.Operations.First + Position];
            if (!contains(OperationId, Storage.Operations.size()))
            {
              error(IrVerificationErrorCode::InvalidTableReference, "block references an unknown operation", Block.OwnerFunction, BlockId);
              continue;
            }
            ++OperationClaims[OperationId.value()];
            OperationPositions[OperationId.value()] = Position;
            const auto &Operation = Storage.Operations[OperationId.value()];
            if (Operation.OwnerBlock != BlockId || Operation.OwnerFunction != Block.OwnerFunction)
            {
              error(IrVerificationErrorCode::InvalidOwner, "operation owner does not match the block operation table", Block.OwnerFunction, BlockId, OperationId);
            }
          }
        }

        for (std::size_t OperationIndex = 0; OperationIndex < Storage.Operations.size(); ++OperationIndex)
        {
          const auto OperationId = IrOperationId::fromValue(static_cast<std::uint32_t>(OperationIndex));
          const auto &Operation = Storage.Operations[OperationIndex];
          claimRange(Operation.Operands, Storage.OperationOperands.size(), OperandClaims, "operation operand", IrVerificationErrorCode::InvalidOperation, Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          claimRange(Operation.Results, Storage.OperationResults.size(), ResultClaims, "operation result", IrVerificationErrorCode::InvalidOperation, Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          claimRange(Operation.Successors, Storage.OperationSuccessors.size(), SuccessorClaims, "operation successor", IrVerificationErrorCode::InvalidOperation, Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          if (rangeWithin(Operation.Results, Storage.OperationResults.size()))
          {
            for (std::uint32_t ResultIndex = 0; ResultIndex < Operation.Results.Count; ++ResultIndex)
            {
              const auto ValueId = Storage.OperationResults[Operation.Results.First + ResultIndex];
              if (!contains(ValueId, Storage.Values.size()))
              {
                error(IrVerificationErrorCode::InvalidTableReference, "operation references an unknown result value", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
                continue;
              }
              ++ValueClaims[ValueId.value()];
              ValueDefinitions[ValueId.value()] = {Operation.OwnerBlock, OperationId, true};
              const auto &Value = Storage.Values[ValueId.value()];
              if (Value.DefinitionKind != IrValueDefinitionKind::OperationResult || Value.DefinitionIndex != ResultIndex || Value.OwnerBlock != Operation.OwnerBlock || Value.OwnerFunction != Operation.OwnerFunction)
              {
                error(IrVerificationErrorCode::InvalidOwner, "operation result value has inconsistent definition data", Operation.OwnerFunction, Operation.OwnerBlock, OperationId, ValueId);
              }
            }
          }
          if (rangeWithin(Operation.Successors, Storage.OperationSuccessors.size()))
          {
            for (std::uint32_t SuccessorIndex = Operation.Successors.First; SuccessorIndex < Operation.Successors.end(); ++SuccessorIndex)
            {
              const auto &Successor = Storage.OperationSuccessors[SuccessorIndex];
              claimRange(Successor.Arguments, Storage.SuccessorArguments.size(), SuccessorArgumentClaims, "successor argument", IrVerificationErrorCode::InvalidControlFlow, Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
          }
        }

        requireFullyClaimed(FunctionBlockClaims, "function block reference table");
        requireFullyClaimed(BlockOperationClaims, "block operation reference table");
        requireFullyClaimed(OperandClaims, "operation operand table");
        requireFullyClaimed(ResultClaims, "operation result table");
        requireFullyClaimed(SuccessorClaims, "operation successor table");
        requireFullyClaimed(SuccessorArgumentClaims, "successor argument table");
        if (std::any_of(BlockClaims.begin(), BlockClaims.end(), [](std::uint8_t Claim) { return Claim != 1; }))
        {
          error(IrVerificationErrorCode::InvalidOwner, "every block must be owned by exactly one function");
        }
        if (std::any_of(OperationClaims.begin(), OperationClaims.end(), [](std::uint8_t Claim) { return Claim != 1; }))
        {
          error(IrVerificationErrorCode::InvalidOwner, "every operation must be owned by exactly one block");
        }
        if (std::any_of(ValueClaims.begin(), ValueClaims.end(), [](std::uint8_t Claim) { return Claim != 1; }))
        {
          error(IrVerificationErrorCode::InvalidOwner, "every value must have exactly one block argument or operation result definition");
        }
        for (std::size_t ValueIndex = 0; ValueIndex < Storage.Values.size(); ++ValueIndex)
        {
          const auto ValueId = IrValueId::fromValue(static_cast<std::uint32_t>(ValueIndex));
          const auto &Value = Storage.Values[ValueIndex];
          if (!contains(Value.Type, Storage.Types.size()))
          {
            error(IrVerificationErrorCode::InvalidType, "value references an unknown type", Value.OwnerFunction, Value.OwnerBlock, {}, ValueId);
          }
          if (!contains(Value.OwnerFunction, Storage.Functions.size()) || !contains(Value.OwnerBlock, Storage.Blocks.size()) || !isOptionalOriginValid(Storage, Value.Origin))
          {
            error(IrVerificationErrorCode::InvalidOwner, "value has an invalid function, block, or origin owner", Value.OwnerFunction, Value.OwnerBlock, {}, ValueId);
          }
        }
      }

      void verifyFunctionsAndBlocks()
      {
        for (std::size_t FunctionIndex = 0; FunctionIndex < Storage.Functions.size(); ++FunctionIndex)
        {
          const auto FunctionId = IrFunctionId::fromValue(static_cast<std::uint32_t>(FunctionIndex));
          const auto &Function = Storage.Functions[FunctionIndex];
          const auto *Signature = findType(Function.Signature);
          if (Function.Name.empty() || Signature == nullptr || Signature->Kind != IrTypeKind::Function || !isOptionalOriginValid(Storage, Function.Origin))
          {
            error(IrVerificationErrorCode::InvalidFunction, "function has an invalid name, signature, or origin", FunctionId);
          }
          for (std::size_t OtherIndex = 0; OtherIndex < FunctionIndex; ++OtherIndex)
          {
            if (Storage.Functions[OtherIndex].Name == Function.Name)
            {
              error(IrVerificationErrorCode::InvalidFunction, "function names must be unique", FunctionId);
            }
          }
          switch (Function.Kind)
          {
          case IrFunctionKind::External:
            if (!Function.Blocks.empty() || Function.EntryBlock.isValid())
            {
              error(IrVerificationErrorCode::InvalidFunction, "external function must not own a body", FunctionId);
            }
            continue;
          case IrFunctionKind::Definition:
            break;
          default:
            error(IrVerificationErrorCode::InvalidFunction, "function has an out-of-domain function kind", FunctionId);
            continue;
          }
          if (Function.Blocks.empty() || !contains(Function.EntryBlock, Storage.Blocks.size()) || Storage.Blocks[Function.EntryBlock.value()].OwnerFunction != FunctionId)
          {
            error(IrVerificationErrorCode::InvalidFunction, "defined function must have an owned entry block", FunctionId);
            continue;
          }
          if (Signature != nullptr && Signature->Kind == IrTypeKind::Function)
          {
            const auto &Entry = Storage.Blocks[Function.EntryBlock.value()];
            if (Entry.Arguments.Count != Signature->Parameters.Count)
            {
              error(IrVerificationErrorCode::InvalidFunction, "entry block arguments do not match function parameters", FunctionId, Function.EntryBlock);
            }
            else if (rangeWithin(Entry.Arguments, Storage.Values.size()) && rangeWithin(Signature->Parameters, Storage.TypeReferences.size()))
            {
              for (std::uint32_t Index = 0; Index < Entry.Arguments.Count; ++Index)
              {
                if (Storage.Values[Entry.Arguments.First + Index].Type != Storage.TypeReferences[Signature->Parameters.First + Index])
                {
                  error(IrVerificationErrorCode::InvalidFunction, "entry block argument type does not match the function signature", FunctionId, Function.EntryBlock);
                }
              }
            }
          }
        }

        for (std::size_t BlockIndex = 0; BlockIndex < Storage.Blocks.size(); ++BlockIndex)
        {
          const auto BlockId = IrBlockId::fromValue(static_cast<std::uint32_t>(BlockIndex));
          const auto &Block = Storage.Blocks[BlockIndex];
          if (!contains(Block.OwnerFunction, Storage.Functions.size()) || !isOptionalOriginValid(Storage, Block.Origin))
          {
            error(IrVerificationErrorCode::InvalidBlock, "block has an invalid owner or origin", Block.OwnerFunction, BlockId);
          }
          if (rangeWithin(Block.Arguments, Storage.Values.size()))
          {
            for (std::uint32_t ArgumentIndex = Block.Arguments.First; ArgumentIndex < Block.Arguments.end(); ++ArgumentIndex)
            {
              if (isTypeKind(Storage.Values[ArgumentIndex].Type, IrTypeKind::Place))
              {
                error(IrVerificationErrorCode::InvalidType, "block arguments cannot carry place values", Block.OwnerFunction, BlockId, {}, IrValueId::fromValue(ArgumentIndex));
              }
            }
          }
          if (!rangeWithin(Block.Operations, Storage.BlockOperations.size()) || Block.Operations.empty())
          {
            error(IrVerificationErrorCode::MissingTerminator, "block has no terminating operation", Block.OwnerFunction, BlockId);
            continue;
          }
          for (std::uint32_t Position = 0; Position < Block.Operations.Count; ++Position)
          {
            const auto OperationId = Storage.BlockOperations[Block.Operations.First + Position];
            if (!contains(OperationId, Storage.Operations.size()))
            {
              continue;
            }
            const auto *Metadata = irOpcodeMetadata(Storage.Operations[OperationId.value()].Opcode);
            const bool Terminator = Metadata != nullptr && Metadata->Terminator;
            if (Terminator && Position + 1 != Block.Operations.Count)
            {
              error(IrVerificationErrorCode::OperationAfterTerminator, "terminator is followed by another operation", Block.OwnerFunction, BlockId, OperationId);
            }
            if (!Terminator && Position + 1 == Block.Operations.Count)
            {
              error(IrVerificationErrorCode::MissingTerminator, "last block operation is not a terminator", Block.OwnerFunction, BlockId, OperationId);
            }
          }
        }
      }

      void verifyOperations()
      {
        for (std::size_t OperationIndex = 0; OperationIndex < Storage.Operations.size(); ++OperationIndex)
        {
          const auto OperationId = IrOperationId::fromValue(static_cast<std::uint32_t>(OperationIndex));
          const auto &Operation = Storage.Operations[OperationIndex];
          const auto *Metadata = irOpcodeMetadata(Operation.Opcode);
          if (Metadata == nullptr || Operation.Opcode == IrOpcode::Unknown)
          {
            error(IrVerificationErrorCode::InvalidOperation, "operation has an unknown opcode", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            continue;
          }
          if (!hasStage(Metadata->Stages, Stage))
          {
            error(IrVerificationErrorCode::InvalidStage, "operation is not permitted in this IR stage", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          if (!isOptionalOriginValid(Storage, Operation.Origin))
          {
            error(IrVerificationErrorCode::InvalidTableReference, "operation references an unknown origin", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          if (!rangeWithin(Operation.Operands, Storage.OperationOperands.size()) || !rangeWithin(Operation.Results, Storage.OperationResults.size()) || !rangeWithin(Operation.Successors, Storage.OperationSuccessors.size()))
          {
            continue;
          }
          if (Operation.Operands.Count < Metadata->MinimumOperands || (Metadata->MaximumOperands != VariadicArity && Operation.Operands.Count > Metadata->MaximumOperands) || Operation.Results.Count < Metadata->MinimumResults || (Metadata->MaximumResults != VariadicArity && Operation.Results.Count > Metadata->MaximumResults) || Operation.Successors.Count != Metadata->Successors)
          {
            error(IrVerificationErrorCode::InvalidArity, "operation arity does not match generated opcode metadata", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          if (!payloadMatches(Metadata->Payload, Operation.Payload))
          {
            error(IrVerificationErrorCode::InvalidPayload, "operation payload does not match generated opcode metadata", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          for (std::uint32_t Index = 0; Index < Operation.Operands.Count; ++Index)
          {
            const auto ValueId = operand(Operation, Index);
            const auto *Value = findValue(ValueId);
            if (Value == nullptr)
            {
              error(IrVerificationErrorCode::InvalidTableReference, "operation references an unknown operand value", Operation.OwnerFunction, Operation.OwnerBlock, OperationId, ValueId);
            }
            else if (Value->OwnerFunction != Operation.OwnerFunction)
            {
              error(IrVerificationErrorCode::InvalidOwner, "operation uses a value owned by another function", Operation.OwnerFunction, Operation.OwnerBlock, OperationId, ValueId);
            }
          }
          for (std::uint32_t Index = 0; Index < Operation.Results.Count; ++Index)
          {
            const auto ValueId = result(Operation, Index);
            const auto *Value = findValue(ValueId);
            if (Value == nullptr || findType(Value->Type) == nullptr)
            {
              error(IrVerificationErrorCode::InvalidType, "operation result has an unknown type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId, ValueId);
            }
          }
          verifyOperationTypes(OperationId, Operation);
          verifySuccessors(OperationId, Operation);
        }
      }

      void verifyOperationTypes(IrOperationId OperationId, const IrOperation &Operation)
      {
        auto requireSameIntegerBinary = [&]()
        {
          if (Operation.Operands.Count != 2 || Operation.Results.Count != 1)
          {
            return;
          }
          const auto *Left = findValue(operand(Operation, 0));
          const auto *Right = findValue(operand(Operation, 1));
          const auto *Result = findValue(result(Operation, 0));
          if (Left == nullptr || Right == nullptr || Result == nullptr || Left->Type != Right->Type || Left->Type != Result->Type || !isTypeKind(Left->Type, IrTypeKind::Integer))
          {
            error(IrVerificationErrorCode::InvalidType, "integer binary operation requires one identical integer type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
        };

        if (Operation.Opcode == IrOpcode::ConstInt || Operation.Opcode == IrOpcode::ConstBool)
        {
          if (!std::holds_alternative<IrConstantPayload>(Operation.Payload) || Operation.Results.Count != 1)
          {
            return;
          }
          const auto ConstantId = std::get<IrConstantPayload>(Operation.Payload).Constant;
          if (!contains(ConstantId, Storage.Constants.size()))
          {
            error(IrVerificationErrorCode::InvalidConstant, "constant operation references an unknown constant", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            return;
          }
          const auto &Constant = Storage.Constants[ConstantId.value()];
          const auto ExpectedKind = Operation.Opcode == IrOpcode::ConstInt ? IrConstantKind::Integer : IrConstantKind::Bool;
          if (Constant.Kind != ExpectedKind || !hasValueType(result(Operation, 0), Constant.Type))
          {
            error(IrVerificationErrorCode::InvalidConstant, "constant operation kind or result type does not match its constant", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          return;
        }
        if (isIntegerBinary(Operation.Opcode))
        {
          requireSameIntegerBinary();
          if (Operation.Operands.Count == 2)
          {
            const auto *Type = findValue(operand(Operation, 0));
            const auto *TypeRecord = Type == nullptr ? nullptr : findType(Type->Type);
            if ((Operation.Opcode == IrOpcode::IntSignedDiv || Operation.Opcode == IrOpcode::IntSignedRem) && TypeRecord != nullptr && TypeRecord->Signedness != IrSignedness::Signed)
            {
              error(IrVerificationErrorCode::InvalidType, "signed division or remainder requires a signed integer type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
            if ((Operation.Opcode == IrOpcode::IntUnsignedDiv || Operation.Opcode == IrOpcode::IntUnsignedRem) && TypeRecord != nullptr && TypeRecord->Signedness != IrSignedness::Unsigned)
            {
              error(IrVerificationErrorCode::InvalidType, "unsigned division or remainder requires an unsigned integer type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::IntNeg)
        {
          if (Operation.Operands.Count == 1 && Operation.Results.Count == 1)
          {
            const auto *Input = findValue(operand(Operation, 0));
            const auto *Output = findValue(result(Operation, 0));
            if (Input == nullptr || Output == nullptr || Input->Type != Output->Type || !isTypeKind(Input->Type, IrTypeKind::Integer))
            {
              error(IrVerificationErrorCode::InvalidType, "integer negate requires an identical integer operand and result type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::IntCompare)
        {
          if (!std::holds_alternative<IrComparePayload>(Operation.Payload) || Operation.Operands.Count != 2 || Operation.Results.Count != 1)
          {
            return;
          }
          const auto *Left = findValue(operand(Operation, 0));
          const auto *Right = findValue(operand(Operation, 1));
          const auto *Output = findValue(result(Operation, 0));
          const auto Predicate = std::get<IrComparePayload>(Operation.Payload).Predicate;
          if (!isValidComparePredicate(Predicate))
          {
            error(IrVerificationErrorCode::InvalidPayload, "integer comparison has an out-of-domain predicate", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            return;
          }
          if (Left == nullptr || Right == nullptr || Output == nullptr || Left->Type != Right->Type || !isTypeKind(Output->Type, IrTypeKind::Bool))
          {
            error(IrVerificationErrorCode::InvalidType, "integer comparison requires matching operands and a bool result", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            return;
          }
          const auto *InputType = findType(Left->Type);
          if (InputType == nullptr || (InputType->Kind == IrTypeKind::Bool && !isEqualityPredicate(Predicate)) || (InputType->Kind != IrTypeKind::Bool && InputType->Kind != IrTypeKind::Integer) || (isSignedPredicate(Predicate) && InputType->Signedness != IrSignedness::Signed) || (isUnsignedPredicate(Predicate) && InputType->Signedness != IrSignedness::Unsigned))
          {
            error(IrVerificationErrorCode::InvalidType, "comparison predicate is incompatible with the operand type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::CastInt)
        {
          if (!std::holds_alternative<IrTypePayload>(Operation.Payload) || Operation.Operands.Count != 1 || Operation.Results.Count != 1)
          {
            return;
          }
          const auto Target = std::get<IrTypePayload>(Operation.Payload).Type;
          const auto *Input = findValue(operand(Operation, 0));
          const auto *Output = findValue(result(Operation, 0));
          if (Input == nullptr || Output == nullptr || !isTypeKind(Input->Type, IrTypeKind::Integer) || !isTypeKind(Target, IrTypeKind::Integer) || Output->Type != Target)
          {
            error(IrVerificationErrorCode::InvalidType, "integer cast requires integer source and exact integer destination type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::BoolNot || Operation.Opcode == IrOpcode::BoolAnd || Operation.Opcode == IrOpcode::BoolOr)
        {
          for (std::uint32_t Index = 0; Index < Operation.Operands.Count; ++Index)
          {
            const auto *Value = findValue(operand(Operation, Index));
            if (Value == nullptr || !isTypeKind(Value->Type, IrTypeKind::Bool))
            {
              error(IrVerificationErrorCode::InvalidType, "boolean operation requires bool operands", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
          }
          if (Operation.Results.Count == 1)
          {
            const auto *Value = findValue(result(Operation, 0));
            if (Value == nullptr || !isTypeKind(Value->Type, IrTypeKind::Bool))
            {
              error(IrVerificationErrorCode::InvalidType, "boolean operation requires a bool result", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::Alloca)
        {
          if (!std::holds_alternative<IrTypePayload>(Operation.Payload) || Operation.Results.Count != 1)
          {
            return;
          }
          const auto Element = std::get<IrTypePayload>(Operation.Payload).Type;
          const auto *Output = findValue(result(Operation, 0));
          const auto *Place = Output == nullptr ? nullptr : findType(Output->Type);
          if (findType(Element) == nullptr || Place == nullptr || Place->Kind != IrTypeKind::Place || Place->ElementType != Element || Place->Access != IrPlaceAccess::ReadWrite)
          {
            error(IrVerificationErrorCode::InvalidType, "alloca result must be a writable place of its payload element type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::Load)
        {
          if (Operation.Operands.Count == 1 && Operation.Results.Count == 1)
          {
            const auto *PlaceValue = findValue(operand(Operation, 0));
            const auto *Output = findValue(result(Operation, 0));
            const auto *Place = PlaceValue == nullptr ? nullptr : findType(PlaceValue->Type);
            if (Place == nullptr || Place->Kind != IrTypeKind::Place || Output == nullptr || Output->Type != Place->ElementType)
            {
              error(IrVerificationErrorCode::InvalidType, "load requires a place and returns its element type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::Store)
        {
          if (Operation.Operands.Count == 2)
          {
            const auto *PlaceValue = findValue(operand(Operation, 0));
            const auto *StoredValue = findValue(operand(Operation, 1));
            const auto *Place = PlaceValue == nullptr ? nullptr : findType(PlaceValue->Type);
            if (Place == nullptr || Place->Kind != IrTypeKind::Place || Place->Access != IrPlaceAccess::ReadWrite || StoredValue == nullptr || StoredValue->Type != Place->ElementType)
            {
              error(IrVerificationErrorCode::InvalidType, "store requires a writable place followed by its exact element value", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::DirectCall)
        {
          verifyDirectCall(OperationId, Operation);
          return;
        }
        if (Operation.Opcode == IrOpcode::CondBranch && Operation.Operands.Count == 1)
        {
          const auto *Condition = findValue(operand(Operation, 0));
          if (Condition == nullptr || !isTypeKind(Condition->Type, IrTypeKind::Bool))
          {
            error(IrVerificationErrorCode::InvalidType, "conditional branch condition must be bool", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          return;
        }
        if (Operation.Opcode == IrOpcode::Return)
        {
          verifyReturn(OperationId, Operation);
          return;
        }
        if (Operation.Opcode == IrOpcode::Trap && std::holds_alternative<IrTrapPayload>(Operation.Payload) && !isValidTrapKind(std::get<IrTrapPayload>(Operation.Payload).Kind))
        {
          error(IrVerificationErrorCode::InvalidPayload, "trap operation has an out-of-domain trap kind", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
        }
      }

      void verifyDirectCall(IrOperationId OperationId, const IrOperation &Operation)
      {
        if (!std::holds_alternative<IrDirectCallPayload>(Operation.Payload))
        {
          return;
        }
        const auto Callee = std::get<IrDirectCallPayload>(Operation.Payload).Callee;
        if (!contains(Callee, Storage.Functions.size()))
        {
          error(IrVerificationErrorCode::InvalidFunction, "direct call references an unknown callee", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          return;
        }
        const auto *Signature = findType(Storage.Functions[Callee.value()].Signature);
        if (Signature == nullptr || Signature->Kind != IrTypeKind::Function || !rangeWithin(Signature->Parameters, Storage.TypeReferences.size()))
        {
          error(IrVerificationErrorCode::InvalidFunction, "direct call callee has an invalid signature", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          return;
        }
        if (Operation.Operands.Count != Signature->Parameters.Count)
        {
          error(IrVerificationErrorCode::InvalidArity, "direct call argument count does not match the callee signature", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
        }
        else
        {
          for (std::uint32_t Index = 0; Index < Operation.Operands.Count; ++Index)
          {
            if (!hasValueType(operand(Operation, Index), Storage.TypeReferences[Signature->Parameters.First + Index]))
            {
              error(IrVerificationErrorCode::InvalidType, "direct call argument type does not match the callee signature", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            }
          }
        }
        const bool ProducesValue = Signature->Result && !isTypeKind(*Signature->Result, IrTypeKind::Never);
        if ((!ProducesValue && Operation.Results.Count != 0) || (ProducesValue && (Operation.Results.Count != 1 || !hasValueType(result(Operation, 0), *Signature->Result))))
        {
          error(IrVerificationErrorCode::InvalidType, "direct call result does not match the callee signature", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
        }
      }

      void verifyReturn(IrOperationId OperationId, const IrOperation &Operation)
      {
        if (!contains(Operation.OwnerFunction, Storage.Functions.size()))
        {
          return;
        }
        const auto *Signature = findType(Storage.Functions[Operation.OwnerFunction.value()].Signature);
        if (Signature == nullptr || Signature->Kind != IrTypeKind::Function)
        {
          return;
        }
        if (!Signature->Result)
        {
          if (Operation.Operands.Count != 0)
          {
            error(IrVerificationErrorCode::InvalidType, "void function return must not carry an operand", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
        }
        else if (isTypeKind(*Signature->Result, IrTypeKind::Never))
        {
          error(IrVerificationErrorCode::InvalidControlFlow, "never function cannot contain a return", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
        }
        else if (Operation.Operands.Count != 1 || !hasValueType(operand(Operation, 0), *Signature->Result))
        {
          error(IrVerificationErrorCode::InvalidType, "return operand does not match the function result type", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
        }
      }

      void verifySuccessors(IrOperationId OperationId, const IrOperation &Operation)
      {
        if (!rangeWithin(Operation.Successors, Storage.OperationSuccessors.size()))
        {
          return;
        }
        for (std::uint32_t Index = Operation.Successors.First; Index < Operation.Successors.end(); ++Index)
        {
          const auto &Successor = Storage.OperationSuccessors[Index];
          if (!contains(Successor.Block, Storage.Blocks.size()))
          {
            error(IrVerificationErrorCode::InvalidControlFlow, "operation successor references an unknown block", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            continue;
          }
          const auto &Target = Storage.Blocks[Successor.Block.value()];
          if (Target.OwnerFunction != Operation.OwnerFunction)
          {
            error(IrVerificationErrorCode::InvalidControlFlow, "branch successor belongs to another function", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
          }
          if (!rangeWithin(Successor.Arguments, Storage.SuccessorArguments.size()) || !rangeWithin(Target.Arguments, Storage.Values.size()) || Successor.Arguments.Count != Target.Arguments.Count)
          {
            error(IrVerificationErrorCode::InvalidControlFlow, "branch argument count does not match target block arguments", Operation.OwnerFunction, Operation.OwnerBlock, OperationId);
            continue;
          }
          for (std::uint32_t ArgumentIndex = 0; ArgumentIndex < Successor.Arguments.Count; ++ArgumentIndex)
          {
            const auto Argument = Storage.SuccessorArguments[Successor.Arguments.First + ArgumentIndex];
            const auto TargetArgument = IrValueId::fromValue(Target.Arguments.First + ArgumentIndex);
            const auto *ArgumentValue = findValue(Argument);
            const auto *TargetValue = findValue(TargetArgument);
            if (ArgumentValue == nullptr || TargetValue == nullptr || ArgumentValue->Type != TargetValue->Type)
            {
              error(IrVerificationErrorCode::InvalidType, "branch argument type does not match target block argument", Operation.OwnerFunction, Operation.OwnerBlock, OperationId, Argument);
            }
          }
        }
      }

      void verifyPlans()
      {
        if (Stage == IrStage::Closed && !Storage.PlanNodes.empty())
        {
          error(IrVerificationErrorCode::InvalidStage, "Closed InkIR must not contain elaboration plan nodes");
        }
        std::vector<std::uint8_t> OutputClaims(Storage.Values.size());
        for (std::size_t Index = 0; Index < Storage.PlanNodes.size(); ++Index)
        {
          const auto &Plan = Storage.PlanNodes[Index];
          const auto *Metadata = irPlanOpcodeMetadata(Plan.Opcode);
          if (Metadata == nullptr || Plan.Opcode == IrPlanOpcode::Unknown || !hasStage(Metadata->Stages, Stage))
          {
            error(IrVerificationErrorCode::InvalidStage, "plan node opcode is not valid in this IR stage");
            continue;
          }
          const auto *Input = findValue(Plan.Input);
          const auto *Output = findValue(Plan.Output);
          const auto *ResultType = findType(Plan.ResultType);
          if (Input == nullptr || Output == nullptr || ResultType == nullptr || Plan.Input == Plan.Output || Input->Type != Plan.ResultType || Output->Type != Plan.ResultType || Input->OwnerFunction != Output->OwnerFunction || (ResultType->Kind != IrTypeKind::Bool && ResultType->Kind != IrTypeKind::Integer) || !isOptionalOriginValid(Storage, Plan.Origin))
          {
            error(IrVerificationErrorCode::InvalidType, "stage.force_value requires matching input, output, and constant-representable result types in one function", {}, {}, {}, Plan.Input);
            continue;
          }
          if (++OutputClaims[Plan.Output.value()] != 1)
          {
            error(IrVerificationErrorCode::InvalidTableReference, "stage.force_value output is owned by more than one plan node", Output->OwnerFunction, Output->OwnerBlock, {}, Plan.Output);
          }
          if (Output->DefinitionKind != IrValueDefinitionKind::OperationResult || !ValueDefinitions[Plan.Output.value()].Defined || !contains(ValueDefinitions[Plan.Output.value()].Operation, Storage.Operations.size()))
          {
            error(IrVerificationErrorCode::InvalidOperation, "stage.force_value output must be a dedicated constant placeholder operation result", Output->OwnerFunction, Output->OwnerBlock, {}, Plan.Output);
            continue;
          }
          const IrOperationId DefinitionId = ValueDefinitions[Plan.Output.value()].Operation;
          const IrOperation &Definition = Storage.Operations[DefinitionId.value()];
          const IrOpcode ExpectedOpcode = ResultType->Kind == IrTypeKind::Bool ? IrOpcode::ConstBool : IrOpcode::ConstInt;
          if (Definition.Opcode != ExpectedOpcode || Definition.Results.Count != 1 || !rangeWithin(Definition.Results, Storage.OperationResults.size()) || result(Definition, 0) != Plan.Output || !std::holds_alternative<IrConstantPayload>(Definition.Payload))
          {
            error(IrVerificationErrorCode::InvalidOperation, "stage.force_value output must be a dedicated constant placeholder operation result", Output->OwnerFunction, Output->OwnerBlock, DefinitionId, Plan.Output);
            continue;
          }
          const IrConstantId ConstantId = std::get<IrConstantPayload>(Definition.Payload).Constant;
          if (!contains(ConstantId, Storage.Constants.size()))
          {
            error(IrVerificationErrorCode::InvalidConstant, "stage.force_value placeholder references an unknown constant", Output->OwnerFunction, Output->OwnerBlock, DefinitionId, Plan.Output);
            continue;
          }
          const IrConstant &Constant = Storage.Constants[ConstantId.value()];
          const IrConstantKind ExpectedKind = ResultType->Kind == IrTypeKind::Bool ? IrConstantKind::Bool : IrConstantKind::Integer;
          if (Constant.Type != Plan.ResultType || Constant.Kind != ExpectedKind || Constant.Bits != 0)
          {
            error(IrVerificationErrorCode::InvalidConstant, "stage.force_value output placeholder must be the typed zero constant", Output->OwnerFunction, Output->OwnerBlock, DefinitionId, Plan.Output);
          }
        }
      }

      bool callReturnsNever(const IrOperation &Operation) const
      {
        if (Operation.Opcode != IrOpcode::DirectCall || !std::holds_alternative<IrDirectCallPayload>(Operation.Payload))
        {
          return false;
        }
        const auto Callee = std::get<IrDirectCallPayload>(Operation.Payload).Callee;
        if (!contains(Callee, Storage.Functions.size()))
        {
          return false;
        }
        const auto *Signature = findType(Storage.Functions[Callee.value()].Signature);
        return Signature != nullptr && Signature->Kind == IrTypeKind::Function && Signature->Result && isTypeKind(*Signature->Result, IrTypeKind::Never);
      }

      void verifyControlFlowAndDominance()
      {
        for (std::size_t FunctionIndex = 0; FunctionIndex < Storage.Functions.size(); ++FunctionIndex)
        {
          const auto FunctionId = IrFunctionId::fromValue(static_cast<std::uint32_t>(FunctionIndex));
          const auto &Function = Storage.Functions[FunctionIndex];
          if (Function.Kind != IrFunctionKind::Definition || !rangeWithin(Function.Blocks, Storage.FunctionBlocks.size()) || !contains(Function.EntryBlock, Storage.Blocks.size()))
          {
            continue;
          }
          verifyFunctionControlFlow(FunctionId, Function);
        }
      }

      void verifyFunctionControlFlow(IrFunctionId FunctionId, const IrFunction &Function)
      {
        std::vector<IrBlockId> Blocks;
        Blocks.reserve(Function.Blocks.Count);
        std::vector<int> LocalIndex(Storage.Blocks.size(), -1);
        for (std::uint32_t Index = 0; Index < Function.Blocks.Count; ++Index)
        {
          const auto Block = Storage.FunctionBlocks[Function.Blocks.First + Index];
          if (!contains(Block, Storage.Blocks.size()))
          {
            return;
          }
          LocalIndex[Block.value()] = static_cast<int>(Blocks.size());
          Blocks.push_back(Block);
        }
        if (LocalIndex[Function.EntryBlock.value()] < 0)
        {
          return;
        }
        const auto Count = Blocks.size();
        std::vector<std::vector<std::size_t>> Predecessors(Count);
        std::vector<std::vector<std::size_t>> Successors(Count);
        for (std::size_t BlockIndex = 0; BlockIndex < Count; ++BlockIndex)
        {
          const auto &Block = Storage.Blocks[Blocks[BlockIndex].value()];
          if (!rangeWithin(Block.Operations, Storage.BlockOperations.size()) || Block.Operations.empty())
          {
            continue;
          }
          const auto TerminatorId = Storage.BlockOperations[Block.Operations.end() - 1];
          if (!contains(TerminatorId, Storage.Operations.size()))
          {
            continue;
          }
          const auto &Terminator = Storage.Operations[TerminatorId.value()];
          if (!rangeWithin(Terminator.Successors, Storage.OperationSuccessors.size()))
          {
            continue;
          }
          for (std::uint32_t Index = Terminator.Successors.First; Index < Terminator.Successors.end(); ++Index)
          {
            const auto Target = Storage.OperationSuccessors[Index].Block;
            if (contains(Target, LocalIndex.size()) && LocalIndex[Target.value()] >= 0)
            {
              const auto TargetIndex = static_cast<std::size_t>(LocalIndex[Target.value()]);
              Successors[BlockIndex].push_back(TargetIndex);
              Predecessors[TargetIndex].push_back(BlockIndex);
            }
          }
        }
        const auto EntryIndex = static_cast<std::size_t>(LocalIndex[Function.EntryBlock.value()]);
        std::vector<bool> Reachable(Count, false);
        std::deque<std::size_t> Worklist;
        Reachable[EntryIndex] = true;
        Worklist.push_back(EntryIndex);
        while (!Worklist.empty())
        {
          const auto Current = Worklist.front();
          Worklist.pop_front();
          for (const auto Target : Successors[Current])
          {
            if (!Reachable[Target])
            {
              Reachable[Target] = true;
              Worklist.push_back(Target);
            }
          }
        }
        std::vector<std::vector<bool>> Dominators(Count, std::vector<bool>(Count, true));
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          if (!Reachable[Index])
          {
            std::fill(Dominators[Index].begin(), Dominators[Index].end(), false);
            Dominators[Index][Index] = true;
          }
        }
        std::fill(Dominators[EntryIndex].begin(), Dominators[EntryIndex].end(), false);
        Dominators[EntryIndex][EntryIndex] = true;
        bool Changed = true;
        while (Changed)
        {
          Changed = false;
          for (std::size_t BlockIndex = 0; BlockIndex < Count; ++BlockIndex)
          {
            if (BlockIndex == EntryIndex || !Reachable[BlockIndex])
            {
              continue;
            }
            std::vector<bool> New(Count, true);
            bool HasReachablePredecessor = false;
            for (const auto Predecessor : Predecessors[BlockIndex])
            {
              if (!Reachable[Predecessor])
              {
                continue;
              }
              if (!HasReachablePredecessor)
              {
                New = Dominators[Predecessor];
                HasReachablePredecessor = true;
              }
              else
              {
                for (std::size_t Index = 0; Index < Count; ++Index)
                {
                  New[Index] = New[Index] && Dominators[Predecessor][Index];
                }
              }
            }
            New[BlockIndex] = true;
            if (New != Dominators[BlockIndex])
            {
              Dominators[BlockIndex] = std::move(New);
              Changed = true;
            }
          }
        }

        for (std::size_t BlockIndex = 0; BlockIndex < Count; ++BlockIndex)
        {
          const auto BlockId = Blocks[BlockIndex];
          const auto &Block = Storage.Blocks[BlockId.value()];
          if (!rangeWithin(Block.Operations, Storage.BlockOperations.size()))
          {
            continue;
          }
          for (std::uint32_t Position = 0; Position < Block.Operations.Count; ++Position)
          {
            const auto OperationId = Storage.BlockOperations[Block.Operations.First + Position];
            if (!contains(OperationId, Storage.Operations.size()))
            {
              continue;
            }
            const auto &Operation = Storage.Operations[OperationId.value()];
            if (Operation.Opcode == IrOpcode::Load)
            {
              verifyLoadInitialization(FunctionId, BlockId, BlockIndex, Position, OperationId, Operation, LocalIndex, Dominators);
            }
            for (std::uint32_t OperandIndex = 0; rangeWithin(Operation.Operands, Storage.OperationOperands.size()) && OperandIndex < Operation.Operands.Count; ++OperandIndex)
            {
              verifyUse(FunctionId, BlockId, BlockIndex, Position, OperationId, operand(Operation, OperandIndex), LocalIndex, Reachable, Dominators);
            }
            for (std::uint32_t SuccessorIndex = Operation.Successors.First; rangeWithin(Operation.Successors, Storage.OperationSuccessors.size()) && SuccessorIndex < Operation.Successors.end(); ++SuccessorIndex)
            {
              const auto &Successor = Storage.OperationSuccessors[SuccessorIndex];
              for (std::uint32_t ArgumentIndex = Successor.Arguments.First; rangeWithin(Successor.Arguments, Storage.SuccessorArguments.size()) && ArgumentIndex < Successor.Arguments.end(); ++ArgumentIndex)
              {
                verifyUse(FunctionId, BlockId, BlockIndex, Position, OperationId, Storage.SuccessorArguments[ArgumentIndex], LocalIndex, Reachable, Dominators);
              }
            }
          }
          if (Block.Operations.Count != 0)
          {
            const auto TerminatorId = Storage.BlockOperations[Block.Operations.end() - 1];
            if (contains(TerminatorId, Storage.Operations.size()) && Storage.Operations[TerminatorId.value()].Opcode == IrOpcode::Unreachable && Reachable[BlockIndex])
            {
              const bool ProvenByNeverCall = Block.Operations.Count >= 2 && contains(Storage.BlockOperations[Block.Operations.end() - 2], Storage.Operations.size()) && callReturnsNever(Storage.Operations[Storage.BlockOperations[Block.Operations.end() - 2].value()]);
              if (!ProvenByNeverCall)
              {
                error(IrVerificationErrorCode::InvalidControlFlow, "reachable cf.unreachable must immediately follow a direct call returning never", FunctionId, BlockId, TerminatorId);
              }
            }
          }
        }
      }

      void verifyLoadInitialization(IrFunctionId FunctionId, IrBlockId UseBlock, std::size_t UseBlockIndex, std::uint32_t UsePosition, IrOperationId LoadId, const IrOperation &Load, const std::vector<int> &LocalIndex, const std::vector<std::vector<bool>> &Dominators)
      {
        if (Load.Operands.Count != 1 || !rangeWithin(Load.Operands, Storage.OperationOperands.size()))
        {
          return;
        }
        const IrValueId Place = operand(Load, 0);
        if (!contains(Place, ValueDefinitions.size()) || !ValueDefinitions[Place.value()].Defined || !contains(ValueDefinitions[Place.value()].Operation, Storage.Operations.size()) || Storage.Operations[ValueDefinitions[Place.value()].Operation.value()].Opcode != IrOpcode::Alloca)
        {
          error(IrVerificationErrorCode::InvalidOperation, "load operand must be the result of an alloca", FunctionId, UseBlock, LoadId, Place);
          return;
        }
        for (std::size_t CandidateIndex = 0; CandidateIndex < Storage.Operations.size(); ++CandidateIndex)
        {
          const IrOperation &Candidate = Storage.Operations[CandidateIndex];
          if (Candidate.Opcode != IrOpcode::Store || Candidate.OwnerFunction != FunctionId || Candidate.Operands.Count != 2 || !rangeWithin(Candidate.Operands, Storage.OperationOperands.size()) || operand(Candidate, 0) != Place)
          {
            continue;
          }
          const IrOperationId CandidateId = IrOperationId::fromValue(static_cast<std::uint32_t>(CandidateIndex));
          if (Candidate.OwnerBlock == UseBlock && OperationPositions[CandidateId.value()] < UsePosition)
          {
            return;
          }
          if (Candidate.OwnerBlock != UseBlock && contains(Candidate.OwnerBlock, LocalIndex.size()) && LocalIndex[Candidate.OwnerBlock.value()] >= 0 && Dominators[UseBlockIndex][static_cast<std::size_t>(LocalIndex[Candidate.OwnerBlock.value()])])
          {
            return;
          }
        }
        error(IrVerificationErrorCode::InvalidOperation, "load requires a preceding store to the same alloca in its block or a dominating block", FunctionId, UseBlock, LoadId, Place);
      }

      void verifyUse(IrFunctionId FunctionId, IrBlockId UseBlock, std::size_t UseBlockIndex, std::uint32_t UsePosition, IrOperationId UseOperation, IrValueId Value, const std::vector<int> &LocalIndex, const std::vector<bool> &Reachable, const std::vector<std::vector<bool>> &Dominators)
      {
        if (!contains(Value, ValueDefinitions.size()) || !ValueDefinitions[Value.value()].Defined)
        {
          return;
        }
        const auto &Definition = ValueDefinitions[Value.value()];
        if (Definition.Block == UseBlock)
        {
          if (Definition.Operation.isValid() && OperationPositions[Definition.Operation.value()] >= UsePosition)
          {
            error(IrVerificationErrorCode::UseBeforeDefinition, "operation uses a value before its defining operation", FunctionId, UseBlock, UseOperation, Value);
          }
          return;
        }
        if (!contains(Definition.Block, LocalIndex.size()) || LocalIndex[Definition.Block.value()] < 0)
        {
          error(IrVerificationErrorCode::NonDominatingValue, "value definition is not in the using function CFG", FunctionId, UseBlock, UseOperation, Value);
          return;
        }
        const auto DefinitionIndex = static_cast<std::size_t>(LocalIndex[Definition.Block.value()]);
        if (!Reachable[UseBlockIndex] || !Dominators[UseBlockIndex][DefinitionIndex])
        {
          error(IrVerificationErrorCode::NonDominatingValue, "value definition does not dominate its use", FunctionId, UseBlock, UseOperation, Value);
        }
      }
    };
  }

  IrStagedVerificationResult::IrStagedVerificationResult(std::optional<VerifiedStagedModule> VerifiedValue, std::vector<IrVerificationError> ErrorValues) : Verified(std::move(VerifiedValue)), Errors(std::move(ErrorValues))
  {
  }

  bool IrStagedVerificationResult::succeeded() const noexcept
  {
    return Verified.has_value();
  }

  const std::vector<IrVerificationError> &IrStagedVerificationResult::errors() const noexcept
  {
    return Errors;
  }

  const VerifiedStagedModule &IrStagedVerificationResult::verified() const
  {
    if (!Verified)
    {
      throw std::logic_error("staged InkIR verification did not succeed");
    }
    return *Verified;
  }

  VerifiedStagedModule IrStagedVerificationResult::takeVerified()
  {
    if (!Verified)
    {
      throw std::logic_error("staged InkIR verification did not succeed");
    }
    auto Result = std::move(*Verified);
    Verified.reset();
    return Result;
  }

  IrClosedVerificationResult::IrClosedVerificationResult(std::optional<VerifiedClosedModule> VerifiedValue, std::vector<IrVerificationError> ErrorValues) : Verified(std::move(VerifiedValue)), Errors(std::move(ErrorValues))
  {
  }

  bool IrClosedVerificationResult::succeeded() const noexcept
  {
    return Verified.has_value();
  }

  const std::vector<IrVerificationError> &IrClosedVerificationResult::errors() const noexcept
  {
    return Errors;
  }

  const VerifiedClosedModule &IrClosedVerificationResult::verified() const
  {
    if (!Verified)
    {
      throw std::logic_error("closed InkIR verification did not succeed");
    }
    return *Verified;
  }

  VerifiedClosedModule IrClosedVerificationResult::takeVerified()
  {
    if (!Verified)
    {
      throw std::logic_error("closed InkIR verification did not succeed");
    }
    auto Result = std::move(*Verified);
    Verified.reset();
    return Result;
  }

  std::vector<IrVerificationError> verifyModule(const IrModule &Module, IrStage Stage)
  {
    if (!Module.isValid())
    {
      return {{IrVerificationErrorCode::InvalidTableReference, "IR module has no storage", {}, {}, {}, {}}};
    }
    if (Stage != IrStage::Staged && Stage != IrStage::Closed)
    {
      return {{IrVerificationErrorCode::InvalidStage, "IR verifier requires exactly Staged or Closed mode", {}, {}, {}, {}}};
    }
    return VerificationState(Module, Stage).run();
  }

  std::vector<IrVerificationError> verifyFunction(const IrModule &Module, IrFunctionId Function, IrStage Stage)
  {
    if (!Module.contains(Function))
    {
      return {{IrVerificationErrorCode::InvalidFunction, "function verifier received an unknown function ID", Function, {}, {}, {}}};
    }
    auto AllErrors = verifyModule(Module, Stage);
    std::vector<IrVerificationError> Result;
    for (auto &Error : AllErrors)
    {
      if (!Error.Function.isValid() || Error.Function == Function)
      {
        Result.push_back(std::move(Error));
      }
    }
    return Result;
  }

  IrStagedVerificationResult verifyStaged(const UnverifiedStagedModule &Module)
  {
    auto Errors = verifyModule(Module.module(), IrStage::Staged);
    if (!Errors.empty())
    {
      return IrStagedVerificationResult(std::nullopt, std::move(Errors));
    }
    return IrStagedVerificationResult(IrVerifier::staged(Module.module()), {});
  }

  IrClosedVerificationResult closeAndVerify(const VerifiedStagedModule &Module, target::TargetKey TargetKey, const std::vector<IrForceValueResolution> &Resolutions)
  {
    auto Errors = verifyModule(Module.module(), IrStage::Staged);
    if (!TargetKey.isValid())
    {
      Errors.push_back({IrVerificationErrorCode::InvalidStage, "closing InkIR requires a complete valid TargetKey", {}, {}, {}, {}});
    }
    const auto &Storage = IrVerifier::storage(Module.module());
    std::vector<std::uint8_t> ResolutionClaims(Storage.PlanNodes.size());
    std::vector<const IrForceValueResolution *> ResolutionByPlan(Storage.PlanNodes.size());
    for (const auto &Resolution : Resolutions)
    {
      if (!Resolution.PlanNode.isValid() || Resolution.PlanNode.value() >= Storage.PlanNodes.size())
      {
        Errors.push_back({IrVerificationErrorCode::InvalidTableReference, "force-value resolution references an unknown plan node", {}, {}, {}, {}});
        continue;
      }
      ++ResolutionClaims[Resolution.PlanNode.value()];
      if (ResolutionByPlan[Resolution.PlanNode.value()] == nullptr)
      {
        ResolutionByPlan[Resolution.PlanNode.value()] = &Resolution;
      }
      if (ResolutionClaims[Resolution.PlanNode.value()] != 1)
      {
        Errors.push_back({IrVerificationErrorCode::InvalidTableReference, "force-value plan node has more than one resolution", {}, {}, {}, {}});
      }
      const auto &Plan = Storage.PlanNodes[Resolution.PlanNode.value()];
      if (!Resolution.Type.isValid() || Resolution.Type.value() >= Storage.Types.size() || Resolution.Type != Plan.ResultType)
      {
        Errors.push_back({IrVerificationErrorCode::InvalidConstant, "force-value resolution must provide the exact plan result type", {}, {}, {}, Plan.Input});
        continue;
      }
      const IrType &Type = Storage.Types[Resolution.Type.value()];
      if (Resolution.Kind == IrConstantKind::Integer)
      {
        if (Type.Kind != IrTypeKind::Integer || (Type.BitWidth < 64 && (Resolution.Bits >> Type.BitWidth) != 0))
        {
          Errors.push_back({IrVerificationErrorCode::InvalidConstant, "force-value integer resolution is incompatible with its result type", {}, {}, {}, Plan.Input});
        }
      }
      else if (Resolution.Kind == IrConstantKind::Bool)
      {
        if (Type.Kind != IrTypeKind::Bool || Resolution.Bits > 1)
        {
          Errors.push_back({IrVerificationErrorCode::InvalidConstant, "force-value boolean resolution is incompatible with its result type", {}, {}, {}, Plan.Input});
        }
      }
      else
      {
        Errors.push_back({IrVerificationErrorCode::InvalidConstant, "force-value resolution has an unknown constant kind", {}, {}, {}, Plan.Input});
      }
    }
    if (std::any_of(ResolutionClaims.begin(), ResolutionClaims.end(), [](std::uint8_t Claim) { return Claim != 1; }))
    {
      Errors.push_back({IrVerificationErrorCode::InvalidStage, "every staged force-value plan node must be resolved before closing", {}, {}, {}, {}});
    }
    std::vector<IrForceValueResolution> OrderedResolutions;
    OrderedResolutions.reserve(Storage.PlanNodes.size());
    for (std::size_t PlanIndex = 0; PlanIndex < Storage.PlanNodes.size(); ++PlanIndex)
    {
      if (ResolutionByPlan[PlanIndex] != nullptr)
      {
        OrderedResolutions.push_back(*ResolutionByPlan[PlanIndex]);
      }
    }
    for (std::size_t PlanIndex = 0; PlanIndex < Storage.PlanNodes.size(); ++PlanIndex)
    {
      if (ResolutionByPlan[PlanIndex] == nullptr)
      {
        continue;
      }
      const IrPlanNode &Plan = Storage.PlanNodes[PlanIndex];
      const IrForceValueResolution &Resolution = *ResolutionByPlan[PlanIndex];
      for (std::size_t PreviousIndex = 0; PreviousIndex < PlanIndex; ++PreviousIndex)
      {
        if (ResolutionByPlan[PreviousIndex] == nullptr || Storage.PlanNodes[PreviousIndex].Input != Plan.Input)
        {
          continue;
        }
        const IrForceValueResolution &Previous = *ResolutionByPlan[PreviousIndex];
        if (Previous.Type != Resolution.Type || Previous.Kind != Resolution.Kind || Previous.Bits != Resolution.Bits)
        {
          Errors.push_back({IrVerificationErrorCode::InvalidConstant, "force-value plans for the same input must resolve to the same typed constant", {}, {}, {}, Plan.Input});
        }
        break;
      }
    }
    if (Storage.Constants.size() > static_cast<std::size_t>(IrConstantId::InvalidValue) || Storage.PlanNodes.size() > static_cast<std::size_t>(IrConstantId::InvalidValue) - Storage.Constants.size())
    {
      Errors.push_back({IrVerificationErrorCode::InvalidTableReference, "force-value resolution would overflow the InkIR constant table", {}, {}, {}, {}});
    }
    if (!Errors.empty())
    {
      return IrClosedVerificationResult(std::nullopt, std::move(Errors));
    }
    auto ClosedModule = IrVerifier::applyForceValueResolutions(Module.module(), OrderedResolutions);
    Errors = verifyModule(ClosedModule, IrStage::Closed);
    if (!Errors.empty())
    {
      return IrClosedVerificationResult(std::nullopt, std::move(Errors));
    }
    return IrClosedVerificationResult(IrVerifier::closed(std::move(ClosedModule), std::move(TargetKey)), {});
  }
} // namespace ink::ir
