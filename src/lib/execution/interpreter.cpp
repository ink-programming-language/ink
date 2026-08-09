#include "ink/execution/interpreter.h"

#include "ink/ir/opcode.h"
#include "ink/ir/operation.h"
#include "ink/ir/type.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <utility>

namespace ink::execution
{
  namespace
  {
    constexpr std::uint64_t AbstractFrameBytes = 64;
    constexpr std::uint64_t AbstractValueBytes = 32;
    constexpr std::uint64_t AbstractStorageBytes = 16;
    constexpr std::uint32_t InvalidValueSlot = std::numeric_limits<std::uint32_t>::max();

    enum class MachineValueKind : std::uint8_t
    {
      Scalar,
      Place,
    };

    struct MachineValue
    {
      MachineValueKind Kind = MachineValueKind::Scalar;
      ir::IrTypeId Type;
      std::uint64_t Bits = 0;
      std::uint64_t Activation = 0;
      std::uint32_t Storage = 0;
    };

    struct StorageSlot
    {
      ir::IrTypeId Type;
      std::optional<MachineValue> Value;
    };

    struct ActivationFrame
    {
      ir::IrFunctionId Function;
      ir::IrBlockId Block;
      std::uint32_t OperationCursor = 0;
      std::vector<std::optional<MachineValue>> Values;
      std::vector<StorageSlot> Storage;
      std::optional<ir::IrOperationId> CallerOperation;
      std::uint64_t Activation = 0;
      std::uint64_t StackBytes = 0;
    };

    bool checkedAdd(std::uint64_t Left, std::uint64_t Right, std::uint64_t &Result) noexcept
    {
      if (Right > std::numeric_limits<std::uint64_t>::max() - Left)
      {
        return false;
      }
      Result = Left + Right;
      return true;
    }

    bool checkedMultiply(std::uint64_t Left, std::uint64_t Right, std::uint64_t &Result) noexcept
    {
      if (Left != 0 && Right > std::numeric_limits<std::uint64_t>::max() / Left)
      {
        return false;
      }
      Result = Left * Right;
      return true;
    }

    std::uint64_t bitMask(std::uint16_t BitWidth) noexcept
    {
      return BitWidth == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << BitWidth) - 1;
    }

    class ClosedInterpreter
    {
    public:
      ClosedInterpreter(const ir::VerifiedClosedModule &ClosedModule, RuntimeWorld &RuntimeWorldValue, ExecutionLimits ExecutionLimitsValue) : Closed(ClosedModule), Module(ClosedModule.module()), World(RuntimeWorldValue), Limits(ExecutionLimitsValue)
      {
      }

      ExecutionResult run(ir::IrFunctionId Entry, const std::vector<RuntimeValue> &Arguments)
      {
        if (Closed.targetKey() != World.targetKey())
        {
          return invariantFailure("RuntimeWorld target key does not match the verified Closed module");
        }
        if (const std::optional<ExecutionResult> Failure = preflight())
        {
          return *Failure;
        }
        if (!Module.contains(Entry))
        {
          return invariantFailure("entry function does not belong to the verified Closed module");
        }
        std::vector<MachineValue> EntryArguments;
        EntryArguments.reserve(Arguments.size());
        for (const RuntimeValue Argument : Arguments)
        {
          MachineValue Value;
          if (!normalizeExternalValue(Argument, Value))
          {
            return invariantFailure("entry argument is not a canonical scalar value of its declared InkIR type");
          }
          EntryArguments.push_back(Value);
        }

        if (const std::optional<ExecutionResult> Failure = pushFrame(Entry, EntryArguments, std::nullopt))
        {
          return *Failure;
        }

        while (!Frames.empty())
        {
          ActivationFrame &Frame = Frames.back();
          const ir::IrBlock &Block = Module.block(Frame.Block);
          if (Frame.OperationCursor >= Block.Operations.Count)
          {
            return invariantFailure("execution reached the end of an InkIR block without a terminator");
          }
          const ir::IrOperationId OperationId = Module.blockOperation(Block.Operations.First + Frame.OperationCursor);
          const ir::IrOperation &Operation = Module.operation(OperationId);
          if (Statistics.FuelConsumed >= Limits.Fuel)
          {
            return limitExceeded(ExecutionLimitKind::Fuel, "execution fuel was exhausted", Operation.Origin);
          }
          ++Statistics.FuelConsumed;
          ++Frame.OperationCursor;
          if (const std::optional<ExecutionResult> Result = execute(OperationId, Operation))
          {
            return *Result;
          }
        }
        return invariantFailure("execution ended without returning, trapping, or reporting a resource limit");
      }

    private:
      std::optional<ExecutionResult> preflight()
      {
        FunctionValueCounts.assign(Module.functionCount(), 0);
        ValueSlots.assign(Module.valueCount(), InvalidValueSlot);
        for (std::uint32_t ValueIndex = 0; ValueIndex < Module.valueCount(); ++ValueIndex)
        {
          const ir::IrValue &Value = Module.value(ir::IrValueId::fromValue(ValueIndex));
          if (!Module.contains(Value.OwnerFunction))
          {
            return invariantFailure("Closed interpreter encountered a value without a valid owning function");
          }
          ValueSlots[ValueIndex] = FunctionValueCounts[Value.OwnerFunction.value()]++;
        }

        for (std::uint32_t TypeIndex = 0; TypeIndex < Module.typeCount(); ++TypeIndex)
        {
          const ir::IrType &Type = Module.type(ir::IrTypeId::fromValue(TypeIndex));
          switch (Type.Kind)
          {
          case ir::IrTypeKind::Unit:
          case ir::IrTypeKind::Bool:
          case ir::IrTypeKind::Never:
          case ir::IrTypeKind::Place:
          case ir::IrTypeKind::Function:
            break;
          case ir::IrTypeKind::Integer:
            if (Type.BitWidth == 0 || Type.BitWidth > 64 || Type.Signedness == ir::IrSignedness::Signless)
            {
              return invariantFailure("Closed interpreter only supports signed or unsigned integers with widths from 1 through 64 bits");
            }
            break;
          default:
            return invariantFailure("Closed interpreter encountered an unsupported InkIR type");
          }
        }

        for (std::uint32_t FunctionIndex = 0; FunctionIndex < Module.functionCount(); ++FunctionIndex)
        {
          const ir::IrFunction &Function = Module.function(ir::IrFunctionId::fromValue(FunctionIndex));
          if (Function.Kind != ir::IrFunctionKind::Definition && Function.Kind != ir::IrFunctionKind::External)
          {
            return invariantFailure("Closed interpreter encountered a function with an invalid function kind");
          }
          const ir::IrType &Signature = Module.type(Function.Signature);
          if (Signature.Kind != ir::IrTypeKind::Function)
          {
            return invariantFailure("Closed interpreter encountered a function with a non-function signature");
          }
          if (Signature.Result && Module.type(*Signature.Result).Kind == ir::IrTypeKind::Place)
          {
            return invariantFailure("Closed interpreter does not support a place as a logical function result");
          }
        }

        for (std::uint32_t OperationIndex = 0; OperationIndex < Module.operationCount(); ++OperationIndex)
        {
          const ir::IrOperation &Operation = Module.operation(ir::IrOperationId::fromValue(OperationIndex));
          switch (Operation.Opcode)
          {
          case ir::IrOpcode::ConstInt:
          case ir::IrOpcode::ConstBool:
          case ir::IrOpcode::IntAdd:
          case ir::IrOpcode::IntSub:
          case ir::IrOpcode::IntMul:
          case ir::IrOpcode::IntNeg:
          case ir::IrOpcode::IntAnd:
          case ir::IrOpcode::IntOr:
          case ir::IrOpcode::IntXor:
          case ir::IrOpcode::IntCompare:
          case ir::IrOpcode::CastInt:
          case ir::IrOpcode::BoolNot:
          case ir::IrOpcode::BoolAnd:
          case ir::IrOpcode::BoolOr:
          case ir::IrOpcode::Alloca:
          case ir::IrOpcode::Load:
          case ir::IrOpcode::Store:
          case ir::IrOpcode::DirectCall:
          case ir::IrOpcode::Branch:
          case ir::IrOpcode::CondBranch:
          case ir::IrOpcode::Return:
          case ir::IrOpcode::Unreachable:
          case ir::IrOpcode::Trap:
            break;
          case ir::IrOpcode::IntSignedDiv:
          case ir::IrOpcode::IntUnsignedDiv:
          case ir::IrOpcode::IntSignedRem:
          case ir::IrOpcode::IntUnsignedRem:
            return unsupportedExecution(std::string("Closed interpreter has no target PDB rule handler for opcode ") + ir::irOpcodeName(Operation.Opcode), Operation.Origin);
          default:
            return unsupportedExecution(std::string("Closed interpreter has no lowering for opcode ") + ir::irOpcodeName(Operation.Opcode), Operation.Origin);
          }
        }
        return std::nullopt;
      }

      bool normalizeExternalValue(RuntimeValue External, MachineValue &Value) const
      {
        if (!Module.contains(External.type()))
        {
          return false;
        }
        const ir::IrType &Type = Module.type(External.type());
        Value.Kind = MachineValueKind::Scalar;
        Value.Type = External.type();
        switch (Type.Kind)
        {
        case ir::IrTypeKind::Integer:
          Value.Bits = External.bits() & bitMask(Type.BitWidth);
          return Value.Bits == External.bits();
        case ir::IrTypeKind::Bool:
          Value.Bits = External.bits();
          return Value.Bits <= 1;
        case ir::IrTypeKind::Unit:
          Value.Bits = 0;
          return External.bits() == 0;
        default:
          return false;
        }
      }

      std::uint64_t functionStackBytes(ir::IrFunctionId Function, bool &Valid) const
      {
        const std::uint64_t ValueCount = FunctionValueCounts[Function.value()];
        std::uint64_t ValueBytes = 0;
        std::uint64_t Total = 0;
        Valid = checkedMultiply(ValueCount, AbstractValueBytes, ValueBytes) && checkedAdd(AbstractFrameBytes, ValueBytes, Total);
        return Total;
      }

      std::optional<ExecutionResult> pushFrame(ir::IrFunctionId FunctionId, const std::vector<MachineValue> &Arguments, std::optional<ir::IrOperationId> CallerOperation)
      {
        const ir::IrFunction &Function = Module.function(FunctionId);
        if (Function.Kind == ir::IrFunctionKind::External)
        {
          return invokeExternalFunction(FunctionId, Arguments, CallerOperation);
        }
        if (Function.Kind != ir::IrFunctionKind::Definition || !Module.contains(Function.EntryBlock))
        {
          return invariantFailure("cannot enter an undefined or malformed InkIR function", operationOrigin(CallerOperation));
        }

        const std::uint64_t NewDepth = static_cast<std::uint64_t>(Frames.size()) + 1;
        if (NewDepth > Limits.MaxCallDepth)
        {
          return limitExceeded(ExecutionLimitKind::CallDepth, "maximum call depth was exceeded", operationOrigin(CallerOperation));
        }
        bool ValidStackCost = false;
        const std::uint64_t FrameBytes = functionStackBytes(FunctionId, ValidStackCost);
        std::uint64_t NewStackBytes = 0;
        if (!ValidStackCost || !checkedAdd(CurrentStackBytes, FrameBytes, NewStackBytes) || NewStackBytes > Limits.MaxStackBytes)
        {
          return limitExceeded(ExecutionLimitKind::Stack, "abstract interpreter stack budget was exceeded", operationOrigin(CallerOperation));
        }

        const ir::IrType &Signature = Module.type(Function.Signature);
        if (Signature.Parameters.Count != Arguments.size())
        {
          return invariantFailure("call argument count does not match the callee signature", operationOrigin(CallerOperation));
        }
        for (std::uint32_t Index = 0; Index < Signature.Parameters.Count; ++Index)
        {
          if (Module.typeReference(Signature.Parameters.First + Index) != Arguments[Index].Type)
          {
            return invariantFailure("call argument type does not match the callee signature", operationOrigin(CallerOperation));
          }
        }

        ActivationFrame Frame;
        Frame.Function = FunctionId;
        Frame.Values.resize(FunctionValueCounts[FunctionId.value()]);
        Frame.CallerOperation = CallerOperation;
        if (NextActivation == 0)
        {
          return invariantFailure("interpreter activation identity space was exhausted", operationOrigin(CallerOperation));
        }
        Frame.Activation = NextActivation++;
        Frame.StackBytes = FrameBytes;
        if (!enterBlock(Frame, Function.EntryBlock, Arguments))
        {
          return invariantFailure("callee entry block arguments do not match its function signature", operationOrigin(CallerOperation));
        }
        Frames.push_back(std::move(Frame));
        CurrentStackBytes = NewStackBytes;
        Statistics.MaximumCallDepth = std::max(Statistics.MaximumCallDepth, static_cast<std::uint32_t>(NewDepth));
        Statistics.PeakStackBytes = std::max(Statistics.PeakStackBytes, CurrentStackBytes);
        return std::nullopt;
      }

      std::optional<ExecutionResult> invokeExternalFunction(ir::IrFunctionId FunctionId, const std::vector<MachineValue> &Arguments, std::optional<ir::IrOperationId> CallerOperation)
      {
        const ir::IrFunction &Function = Module.function(FunctionId);
        const ExternalFunctionHandler *Handler = World.externalFunction(Function.Name);
        if (Handler == nullptr)
        {
          return unsupportedExecution(CallerOperation ? "RuntimeWorld has no handler for an external function" : "RuntimeWorld has no handler for an external entry function", operationOrigin(CallerOperation));
        }
        const ir::IrType &Signature = Module.type(Function.Signature);
        if (Signature.Parameters.Count != Arguments.size())
        {
          return invariantFailure("external call argument count does not match the callee signature", operationOrigin(CallerOperation));
        }
        std::vector<RuntimeValue> ExternalArguments;
        ExternalArguments.reserve(Arguments.size());
        for (std::uint32_t Index = 0; Index < Signature.Parameters.Count; ++Index)
        {
          if (Arguments[Index].Kind != MachineValueKind::Scalar || Module.typeReference(Signature.Parameters.First + Index) != Arguments[Index].Type)
          {
            return invariantFailure("external call argument is not a matching scalar value", operationOrigin(CallerOperation));
          }
          ExternalArguments.push_back(RuntimeValue::fromBits(Arguments[Index].Type, Arguments[Index].Bits));
        }
        const std::optional<RuntimeValue> ExternalResult = (*Handler)(ExternalArguments);
        std::optional<MachineValue> ReturnedValue;
        if (!Signature.Result)
        {
          if (ExternalResult)
          {
            return invariantFailure("void external function returned a value", operationOrigin(CallerOperation));
          }
        }
        else if (Module.type(*Signature.Result).Kind == ir::IrTypeKind::Never)
        {
          return invariantFailure("never-returning external function returned normally", operationOrigin(CallerOperation));
        }
        else
        {
          MachineValue Value;
          if (!ExternalResult || !normalizeExternalValue(*ExternalResult, Value) || Value.Type != *Signature.Result)
          {
            return invariantFailure("external function result does not match the callee signature", operationOrigin(CallerOperation));
          }
          ReturnedValue = Value;
        }
        if (!CallerOperation)
        {
          ExecutionResult Result;
          Result.Status = ExecutionStatus::Returned;
          Result.Statistics = Statistics;
          if (ReturnedValue)
          {
            Result.Value = RuntimeValue::fromBits(ReturnedValue->Type, ReturnedValue->Bits);
          }
          return Result;
        }
        if (!Module.contains(*CallerOperation))
        {
          return invariantFailure("external function returned without a valid caller continuation");
        }
        const ir::IrOperation &Call = Module.operation(*CallerOperation);
        if (ReturnedValue)
        {
          if (Call.Results.Count != 1)
          {
            return invariantFailure("external function returned a value to a call without a result", Call.Origin);
          }
          return bindResult(Module.operationResult(Call.Results.First), *ReturnedValue, Call.Origin);
        }
        if (Call.Results.Count != 0)
        {
          return invariantFailure("void external function returned to a call that expects a result", Call.Origin);
        }
        return std::nullopt;
      }

      bool enterBlock(ActivationFrame &Frame, ir::IrBlockId BlockId, const std::vector<MachineValue> &Arguments)
      {
        if (!Module.contains(BlockId))
        {
          return false;
        }
        const ir::IrBlock &Block = Module.block(BlockId);
        if (Block.OwnerFunction != Frame.Function || Block.Arguments.Count != Arguments.size())
        {
          return false;
        }
        for (std::uint32_t Index = 0; Index < Block.Arguments.Count; ++Index)
        {
          const ir::IrValueId ArgumentId = ir::IrValueId::fromValue(Block.Arguments.First + Index);
          if (!Module.contains(ArgumentId) || Module.value(ArgumentId).Type != Arguments[Index].Type || !valueSlot(ArgumentId, Frame.Function))
          {
            return false;
          }
        }
        Frame.Block = BlockId;
        Frame.OperationCursor = 0;
        for (std::uint32_t Index = 0; Index < Block.Arguments.Count; ++Index)
        {
          const ir::IrValueId ArgumentId = ir::IrValueId::fromValue(Block.Arguments.First + Index);
          Frame.Values[*valueSlot(ArgumentId, Frame.Function)] = Arguments[Index];
        }
        return true;
      }

      std::optional<ExecutionResult> execute(ir::IrOperationId OperationId, const ir::IrOperation &Operation)
      {
        switch (Operation.Opcode)
        {
        case ir::IrOpcode::ConstInt:
        case ir::IrOpcode::ConstBool:
          return executeConstant(Operation);
        case ir::IrOpcode::IntAdd:
        case ir::IrOpcode::IntSub:
        case ir::IrOpcode::IntMul:
        case ir::IrOpcode::IntAnd:
        case ir::IrOpcode::IntOr:
        case ir::IrOpcode::IntXor:
          return executeIntegerBinary(Operation);
        case ir::IrOpcode::IntNeg:
          return executeIntegerNegate(Operation);
        case ir::IrOpcode::IntCompare:
          return executeCompare(Operation);
        case ir::IrOpcode::CastInt:
          return executeIntegerCast(Operation);
        case ir::IrOpcode::BoolNot:
        case ir::IrOpcode::BoolAnd:
        case ir::IrOpcode::BoolOr:
          return executeBoolean(Operation);
        case ir::IrOpcode::Alloca:
          return executeAlloca(Operation);
        case ir::IrOpcode::Load:
          return executeLoad(Operation);
        case ir::IrOpcode::Store:
          return executeStore(Operation);
        case ir::IrOpcode::DirectCall:
          return executeDirectCall(OperationId, Operation);
        case ir::IrOpcode::Branch:
          return executeBranch(Operation, 0);
        case ir::IrOpcode::CondBranch:
          return executeConditionalBranch(Operation);
        case ir::IrOpcode::Return:
          return executeReturn(Operation);
        case ir::IrOpcode::Trap:
          return executeTrap(Operation);
        case ir::IrOpcode::Unreachable:
          return invariantFailure("execution reached verifier-only cf.unreachable", Operation.Origin);
        default:
          return invariantFailure("execution reached an opcode rejected by Closed interpreter preflight", Operation.Origin);
        }
      }

      std::optional<ExecutionResult> executeConstant(const ir::IrOperation &Operation)
      {
        const auto *Payload = std::get_if<ir::IrConstantPayload>(&Operation.Payload);
        if (Payload == nullptr || !Module.contains(Payload->Constant))
        {
          return invariantFailure("constant operation has an invalid payload", Operation.Origin);
        }
        const ir::IrConstant &Constant = Module.constant(Payload->Constant);
        MachineValue Value;
        Value.Kind = MachineValueKind::Scalar;
        Value.Type = Constant.Type;
        const ir::IrType &Type = Module.type(Constant.Type);
        if (Operation.Opcode == ir::IrOpcode::ConstInt && Constant.Kind == ir::IrConstantKind::Integer && Type.Kind == ir::IrTypeKind::Integer)
        {
          Value.Bits = Constant.Bits & bitMask(Type.BitWidth);
        }
        else if (Operation.Opcode == ir::IrOpcode::ConstBool && Constant.Kind == ir::IrConstantKind::Bool && Type.Kind == ir::IrTypeKind::Bool && Constant.Bits <= 1)
        {
          Value.Bits = Constant.Bits;
        }
        else
        {
          return invariantFailure("constant operation payload and result type disagree", Operation.Origin);
        }
        return bindOnlyResult(Operation, Value);
      }

      std::optional<ExecutionResult> executeIntegerBinary(const ir::IrOperation &Operation)
      {
        MachineValue Left;
        MachineValue Right;
        if (!readScalarOperand(Operation, 0, Left) || !readScalarOperand(Operation, 1, Right) || Left.Type != Right.Type)
        {
          return invariantFailure("integer binary operation has invalid operands", Operation.Origin);
        }
        const ir::IrType &Type = Module.type(Left.Type);
        if (Type.Kind != ir::IrTypeKind::Integer)
        {
          return invariantFailure("integer binary operation has a non-integer operand", Operation.Origin);
        }
        MachineValue Result;
        Result.Kind = MachineValueKind::Scalar;
        Result.Type = Left.Type;
        switch (Operation.Opcode)
        {
        case ir::IrOpcode::IntAdd:
          Result.Bits = Left.Bits + Right.Bits;
          break;
        case ir::IrOpcode::IntSub:
          Result.Bits = Left.Bits - Right.Bits;
          break;
        case ir::IrOpcode::IntMul:
          Result.Bits = Left.Bits * Right.Bits;
          break;
        case ir::IrOpcode::IntAnd:
          Result.Bits = Left.Bits & Right.Bits;
          break;
        case ir::IrOpcode::IntOr:
          Result.Bits = Left.Bits | Right.Bits;
          break;
        case ir::IrOpcode::IntXor:
          Result.Bits = Left.Bits ^ Right.Bits;
          break;
        default:
          return invariantFailure("integer binary dispatcher received an invalid opcode", Operation.Origin);
        }
        Result.Bits &= bitMask(Type.BitWidth);
        return bindOnlyResult(Operation, Result);
      }

      std::optional<ExecutionResult> executeIntegerNegate(const ir::IrOperation &Operation)
      {
        MachineValue Operand;
        if (!readScalarOperand(Operation, 0, Operand))
        {
          return invariantFailure("integer negate operation has an invalid operand", Operation.Origin);
        }
        const ir::IrType &Type = Module.type(Operand.Type);
        if (Type.Kind != ir::IrTypeKind::Integer)
        {
          return invariantFailure("integer negate operation has a non-integer operand", Operation.Origin);
        }
        Operand.Bits = (std::uint64_t{0} - Operand.Bits) & bitMask(Type.BitWidth);
        return bindOnlyResult(Operation, Operand);
      }

      std::optional<ExecutionResult> executeCompare(const ir::IrOperation &Operation)
      {
        const auto *Payload = std::get_if<ir::IrComparePayload>(&Operation.Payload);
        MachineValue Left;
        MachineValue Right;
        if (Payload == nullptr || !readScalarOperand(Operation, 0, Left) || !readScalarOperand(Operation, 1, Right) || Left.Type != Right.Type)
        {
          return invariantFailure("comparison operation has invalid operands or payload", Operation.Origin);
        }
        const ir::IrType &Type = Module.type(Left.Type);
        bool Comparison = false;
        if (Type.Kind == ir::IrTypeKind::Bool)
        {
          if (Payload->Predicate == ir::IrComparePredicate::Equal)
          {
            Comparison = Left.Bits == Right.Bits;
          }
          else if (Payload->Predicate == ir::IrComparePredicate::NotEqual)
          {
            Comparison = Left.Bits != Right.Bits;
          }
          else
          {
            return invariantFailure("boolean comparison uses a non-equality predicate", Operation.Origin);
          }
        }
        else if (Type.Kind == ir::IrTypeKind::Integer)
        {
          const bool SignedPredicate = Payload->Predicate == ir::IrComparePredicate::SignedLess || Payload->Predicate == ir::IrComparePredicate::SignedLessEqual || Payload->Predicate == ir::IrComparePredicate::SignedGreater || Payload->Predicate == ir::IrComparePredicate::SignedGreaterEqual;
          const bool UnsignedPredicate = Payload->Predicate == ir::IrComparePredicate::UnsignedLess || Payload->Predicate == ir::IrComparePredicate::UnsignedLessEqual || Payload->Predicate == ir::IrComparePredicate::UnsignedGreater || Payload->Predicate == ir::IrComparePredicate::UnsignedGreaterEqual;
          if ((SignedPredicate && Type.Signedness != ir::IrSignedness::Signed) || (UnsignedPredicate && Type.Signedness != ir::IrSignedness::Unsigned))
          {
            return invariantFailure("integer comparison predicate is incompatible with operand signedness", Operation.Origin);
          }
          const std::uint64_t SignBit = std::uint64_t{1} << (Type.BitWidth - 1);
          const std::uint64_t SignedLeft = Left.Bits ^ SignBit;
          const std::uint64_t SignedRight = Right.Bits ^ SignBit;
          switch (Payload->Predicate)
          {
          case ir::IrComparePredicate::Equal:
            Comparison = Left.Bits == Right.Bits;
            break;
          case ir::IrComparePredicate::NotEqual:
            Comparison = Left.Bits != Right.Bits;
            break;
          case ir::IrComparePredicate::SignedLess:
            Comparison = SignedLeft < SignedRight;
            break;
          case ir::IrComparePredicate::SignedLessEqual:
            Comparison = SignedLeft <= SignedRight;
            break;
          case ir::IrComparePredicate::SignedGreater:
            Comparison = SignedLeft > SignedRight;
            break;
          case ir::IrComparePredicate::SignedGreaterEqual:
            Comparison = SignedLeft >= SignedRight;
            break;
          case ir::IrComparePredicate::UnsignedLess:
            Comparison = Left.Bits < Right.Bits;
            break;
          case ir::IrComparePredicate::UnsignedLessEqual:
            Comparison = Left.Bits <= Right.Bits;
            break;
          case ir::IrComparePredicate::UnsignedGreater:
            Comparison = Left.Bits > Right.Bits;
            break;
          case ir::IrComparePredicate::UnsignedGreaterEqual:
            Comparison = Left.Bits >= Right.Bits;
            break;
          }
        }
        else
        {
          return invariantFailure("comparison operation has an unsupported operand type", Operation.Origin);
        }
        const ir::IrValueId ResultId = onlyResult(Operation);
        if (!ResultId || Module.type(Module.value(ResultId).Type).Kind != ir::IrTypeKind::Bool)
        {
          return invariantFailure("comparison operation does not produce exactly one boolean result", Operation.Origin);
        }
        return bindResult(ResultId, MachineValue{MachineValueKind::Scalar, Module.value(ResultId).Type, Comparison ? 1U : 0U, 0, 0}, Operation.Origin);
      }

      std::optional<ExecutionResult> executeIntegerCast(const ir::IrOperation &Operation)
      {
        const auto *Payload = std::get_if<ir::IrTypePayload>(&Operation.Payload);
        MachineValue Operand;
        if (Payload == nullptr || !readScalarOperand(Operation, 0, Operand) || !Module.contains(Payload->Type))
        {
          return invariantFailure("integer cast has an invalid operand or destination type", Operation.Origin);
        }
        const ir::IrType &SourceType = Module.type(Operand.Type);
        const ir::IrType &DestinationType = Module.type(Payload->Type);
        if (SourceType.Kind != ir::IrTypeKind::Integer || DestinationType.Kind != ir::IrTypeKind::Integer)
        {
          return invariantFailure("integer cast source and destination must both be integers", Operation.Origin);
        }
        std::uint64_t Bits = Operand.Bits;
        if (DestinationType.BitWidth > SourceType.BitWidth && SourceType.Signedness == ir::IrSignedness::Signed)
        {
          const std::uint64_t SourceSignBit = std::uint64_t{1} << (SourceType.BitWidth - 1);
          if ((Bits & SourceSignBit) != 0)
          {
            Bits |= ~bitMask(SourceType.BitWidth);
          }
        }
        Bits &= bitMask(DestinationType.BitWidth);
        return bindOnlyResult(Operation, MachineValue{MachineValueKind::Scalar, Payload->Type, Bits, 0, 0});
      }

      std::optional<ExecutionResult> executeBoolean(const ir::IrOperation &Operation)
      {
        MachineValue Left;
        if (!readScalarOperand(Operation, 0, Left) || Module.type(Left.Type).Kind != ir::IrTypeKind::Bool || Left.Bits > 1)
        {
          return invariantFailure("boolean operation has an invalid first operand", Operation.Origin);
        }
        bool Result = false;
        if (Operation.Opcode == ir::IrOpcode::BoolNot)
        {
          Result = Left.Bits == 0;
        }
        else
        {
          MachineValue Right;
          if (!readScalarOperand(Operation, 1, Right) || Right.Type != Left.Type || Right.Bits > 1)
          {
            return invariantFailure("boolean operation has an invalid second operand", Operation.Origin);
          }
          Result = Operation.Opcode == ir::IrOpcode::BoolAnd ? Left.Bits != 0 && Right.Bits != 0 : Left.Bits != 0 || Right.Bits != 0;
        }
        return bindOnlyResult(Operation, MachineValue{MachineValueKind::Scalar, Left.Type, Result ? 1U : 0U, 0, 0});
      }

      std::optional<ExecutionResult> executeAlloca(const ir::IrOperation &Operation)
      {
        const auto *Payload = std::get_if<ir::IrTypePayload>(&Operation.Payload);
        const ir::IrValueId ResultId = onlyResult(Operation);
        if (Payload == nullptr || !Module.contains(Payload->Type) || !ResultId)
        {
          return invariantFailure("alloca operation has an invalid type or result", Operation.Origin);
        }
        const ir::IrType &PlaceType = Module.type(Module.value(ResultId).Type);
        if (PlaceType.Kind != ir::IrTypeKind::Place || PlaceType.ElementType != Payload->Type)
        {
          return invariantFailure("alloca result is not a matching place type", Operation.Origin);
        }
        const std::optional<std::uint64_t> ElementBytes = scalarStorageBytes(Payload->Type);
        std::uint64_t AllocationBytes = 0;
        std::uint64_t NewFrameBytes = 0;
        std::uint64_t NewStackBytes = 0;
        if (!ElementBytes || !checkedAdd(AbstractStorageBytes, *ElementBytes, AllocationBytes) || !checkedAdd(Frames.back().StackBytes, AllocationBytes, NewFrameBytes) || !checkedAdd(CurrentStackBytes, AllocationBytes, NewStackBytes) || NewStackBytes > Limits.MaxStackBytes)
        {
          return limitExceeded(ExecutionLimitKind::Stack, "alloca exceeded the abstract interpreter stack budget", Operation.Origin);
        }
        if (Frames.back().Storage.size() >= std::numeric_limits<std::uint32_t>::max())
        {
          return limitExceeded(ExecutionLimitKind::Stack, "alloca exceeded the interpreter storage-slot limit", Operation.Origin);
        }
        const std::uint32_t StorageIndex = static_cast<std::uint32_t>(Frames.back().Storage.size());
        Frames.back().Storage.push_back(StorageSlot{Payload->Type, std::nullopt});
        Frames.back().StackBytes = NewFrameBytes;
        CurrentStackBytes = NewStackBytes;
        Statistics.PeakStackBytes = std::max(Statistics.PeakStackBytes, CurrentStackBytes);
        return bindResult(ResultId, MachineValue{MachineValueKind::Place, Module.value(ResultId).Type, 0, Frames.back().Activation, StorageIndex}, Operation.Origin);
      }

      std::optional<ExecutionResult> executeLoad(const ir::IrOperation &Operation)
      {
        MachineValue Place;
        if (!readOperand(Operation, 0, Place) || Place.Kind != MachineValueKind::Place)
        {
          return invariantFailure("load operand is not a valid place", Operation.Origin);
        }
        StorageSlot *Slot = findStorage(Place);
        if (Slot == nullptr || !Slot->Value)
        {
          return invariantFailure("load reached uninitialized or expired storage", Operation.Origin);
        }
        return bindOnlyResult(Operation, *Slot->Value);
      }

      std::optional<ExecutionResult> executeStore(const ir::IrOperation &Operation)
      {
        MachineValue Place;
        MachineValue Value;
        if (!readOperand(Operation, 0, Place) || Place.Kind != MachineValueKind::Place || !readScalarOperand(Operation, 1, Value))
        {
          return invariantFailure("store operands are not a place and scalar value", Operation.Origin);
        }
        const ir::IrType &PlaceType = Module.type(Place.Type);
        StorageSlot *Slot = findStorage(Place);
        if (PlaceType.Kind != ir::IrTypeKind::Place || PlaceType.Access != ir::IrPlaceAccess::ReadWrite || Slot == nullptr || Slot->Type != Value.Type)
        {
          return invariantFailure("store place does not provide matching writable storage", Operation.Origin);
        }
        Slot->Value = Value;
        return std::nullopt;
      }

      std::optional<ExecutionResult> executeDirectCall(ir::IrOperationId OperationId, const ir::IrOperation &Operation)
      {
        const auto *Payload = std::get_if<ir::IrDirectCallPayload>(&Operation.Payload);
        if (Payload == nullptr || !Module.contains(Payload->Callee))
        {
          return invariantFailure("direct call has an invalid callee", Operation.Origin);
        }
        std::vector<MachineValue> Arguments;
        Arguments.reserve(Operation.Operands.Count);
        for (std::uint32_t Index = 0; Index < Operation.Operands.Count; ++Index)
        {
          MachineValue Argument;
          if (!readOperand(Operation, Index, Argument))
          {
            return invariantFailure("direct call has an unavailable argument", Operation.Origin);
          }
          Arguments.push_back(Argument);
        }
        return pushFrame(Payload->Callee, Arguments, OperationId);
      }

      std::optional<ExecutionResult> executeBranch(const ir::IrOperation &Operation, std::uint32_t SuccessorIndex)
      {
        if (SuccessorIndex >= Operation.Successors.Count)
        {
          return invariantFailure("branch selected an invalid successor", Operation.Origin);
        }
        const ir::IrSuccessor &Successor = Module.operationSuccessor(Operation.Successors.First + SuccessorIndex);
        std::vector<MachineValue> Arguments;
        Arguments.reserve(Successor.Arguments.Count);
        for (std::uint32_t Index = 0; Index < Successor.Arguments.Count; ++Index)
        {
          const ir::IrValueId ArgumentId = Module.successorArgument(Successor.Arguments.First + Index);
          MachineValue Argument;
          if (!readValue(ArgumentId, Argument))
          {
            return invariantFailure("branch successor argument is unavailable", Operation.Origin);
          }
          Arguments.push_back(Argument);
        }
        if (!enterBlock(Frames.back(), Successor.Block, Arguments))
        {
          return invariantFailure("branch successor arguments do not match the destination block", Operation.Origin);
        }
        return std::nullopt;
      }

      std::optional<ExecutionResult> executeConditionalBranch(const ir::IrOperation &Operation)
      {
        MachineValue Condition;
        if (!readScalarOperand(Operation, 0, Condition) || Module.type(Condition.Type).Kind != ir::IrTypeKind::Bool || Condition.Bits > 1)
        {
          return invariantFailure("conditional branch condition is not a canonical boolean", Operation.Origin);
        }
        return executeBranch(Operation, Condition.Bits == 0 ? 1U : 0U);
      }

      std::optional<ExecutionResult> executeReturn(const ir::IrOperation &Operation)
      {
        const ir::IrFunctionId FunctionId = Frames.back().Function;
        const ir::IrType &Signature = Module.type(Module.function(FunctionId).Signature);
        std::optional<MachineValue> ReturnedValue;
        if (!Signature.Result)
        {
          if (Operation.Operands.Count != 0)
          {
            return invariantFailure("void function returned a value", Operation.Origin);
          }
        }
        else if (Module.type(*Signature.Result).Kind == ir::IrTypeKind::Never)
        {
          return invariantFailure("never function executed a normal return", Operation.Origin);
        }
        else
        {
          MachineValue Value;
          if (Operation.Operands.Count != 1 || !readOperand(Operation, 0, Value) || Value.Type != *Signature.Result)
          {
            return invariantFailure("function return value does not match its logical result", Operation.Origin);
          }
          ReturnedValue = Value;
        }

        const std::optional<ir::IrOperationId> CallerOperation = Frames.back().CallerOperation;
        CurrentStackBytes -= Frames.back().StackBytes;
        Frames.pop_back();
        if (Frames.empty())
        {
          if (ReturnedValue && ReturnedValue->Kind != MachineValueKind::Scalar)
          {
            return invariantFailure("entry function returned a non-scalar runtime value", Operation.Origin);
          }
          ExecutionResult Result;
          Result.Status = ExecutionStatus::Returned;
          Result.Origin = Operation.Origin;
          Result.Statistics = Statistics;
          if (ReturnedValue)
          {
            Result.Value = RuntimeValue::fromBits(ReturnedValue->Type, ReturnedValue->Bits);
          }
          return Result;
        }
        if (!CallerOperation || !Module.contains(*CallerOperation))
        {
          return invariantFailure("callee returned without a valid caller continuation", Operation.Origin);
        }
        const ir::IrOperation &Call = Module.operation(*CallerOperation);
        if (ReturnedValue)
        {
          if (Call.Results.Count != 1)
          {
            return invariantFailure("callee returned a value to a call without a result", Call.Origin);
          }
          return bindResult(Module.operationResult(Call.Results.First), *ReturnedValue, Call.Origin);
        }
        if (Call.Results.Count != 0)
        {
          return invariantFailure("void callee returned to a call that expects a result", Call.Origin);
        }
        return std::nullopt;
      }

      std::optional<ExecutionResult> executeTrap(const ir::IrOperation &Operation)
      {
        const auto *Payload = std::get_if<ir::IrTrapPayload>(&Operation.Payload);
        if (Payload == nullptr)
        {
          return invariantFailure("trap operation has an invalid payload", Operation.Origin);
        }
        ExecutionResult Result;
        Result.Status = ExecutionStatus::LanguageTrap;
        Result.Trap = Payload->Kind;
        Result.Origin = Operation.Origin;
        Result.Statistics = Statistics;
        return Result;
      }

      std::optional<std::uint64_t> scalarStorageBytes(ir::IrTypeId TypeId) const
      {
        const ir::IrType &Type = Module.type(TypeId);
        switch (Type.Kind)
        {
        case ir::IrTypeKind::Integer:
          return (static_cast<std::uint64_t>(Type.BitWidth) + 7) / 8;
        case ir::IrTypeKind::Bool:
        case ir::IrTypeKind::Unit:
          return 1;
        default:
          return std::nullopt;
        }
      }

      StorageSlot *findStorage(const MachineValue &Place)
      {
        for (ActivationFrame &Frame : Frames)
        {
          if (Frame.Activation == Place.Activation && Place.Storage < Frame.Storage.size())
          {
            return &Frame.Storage[Place.Storage];
          }
        }
        return nullptr;
      }

      bool readValue(ir::IrValueId ValueId, MachineValue &Value) const
      {
        const std::optional<std::uint32_t> Slot = valueSlot(ValueId, Frames.back().Function);
        if (!Slot || !Frames.back().Values[*Slot])
        {
          return false;
        }
        Value = *Frames.back().Values[*Slot];
        return true;
      }

      std::optional<std::uint32_t> valueSlot(ir::IrValueId ValueId, ir::IrFunctionId Function) const
      {
        if (!Module.contains(ValueId) || Module.value(ValueId).OwnerFunction != Function || ValueId.value() >= ValueSlots.size() || ValueSlots[ValueId.value()] == InvalidValueSlot)
        {
          return std::nullopt;
        }
        return ValueSlots[ValueId.value()];
      }

      bool readOperand(const ir::IrOperation &Operation, std::uint32_t Index, MachineValue &Value) const
      {
        return Index < Operation.Operands.Count && readValue(Module.operationOperand(Operation.Operands.First + Index), Value);
      }

      bool readScalarOperand(const ir::IrOperation &Operation, std::uint32_t Index, MachineValue &Value) const
      {
        return readOperand(Operation, Index, Value) && Value.Kind == MachineValueKind::Scalar;
      }

      ir::IrValueId onlyResult(const ir::IrOperation &Operation) const
      {
        return Operation.Results.Count == 1 ? Module.operationResult(Operation.Results.First) : ir::IrValueId{};
      }

      std::optional<ExecutionResult> bindOnlyResult(const ir::IrOperation &Operation, const MachineValue &Value)
      {
        const ir::IrValueId ResultId = onlyResult(Operation);
        if (!ResultId)
        {
          return invariantFailure("operation does not have exactly one result", Operation.Origin);
        }
        return bindResult(ResultId, Value, Operation.Origin);
      }

      std::optional<ExecutionResult> bindResult(ir::IrValueId ResultId, const MachineValue &Value, ir::IrOriginId Origin)
      {
        const std::optional<std::uint32_t> Slot = valueSlot(ResultId, Frames.back().Function);
        if (!Slot || Module.value(ResultId).Type != Value.Type)
        {
          return invariantFailure("operation result cannot be bound to the produced runtime value", Origin);
        }
        Frames.back().Values[*Slot] = Value;
        return std::nullopt;
      }

      ir::IrOriginId operationOrigin(std::optional<ir::IrOperationId> Operation) const
      {
        return Operation && Module.contains(*Operation) ? Module.operation(*Operation).Origin : ir::IrOriginId{};
      }

      ExecutionResult invariantFailure(std::string Message, ir::IrOriginId Origin = {}) const
      {
        ExecutionResult Result;
        Result.Status = ExecutionStatus::InternalInvariantFailure;
        Result.Origin = Origin;
        Result.Message = std::move(Message);
        Result.Statistics = Statistics;
        return Result;
      }

      ExecutionResult unsupportedExecution(std::string Message, ir::IrOriginId Origin = {}) const
      {
        ExecutionResult Result;
        Result.Status = ExecutionStatus::Unsupported;
        Result.Origin = Origin;
        Result.Message = std::move(Message);
        Result.Statistics = Statistics;
        return Result;
      }

      ExecutionResult limitExceeded(ExecutionLimitKind Limit, std::string Message, ir::IrOriginId Origin = {}) const
      {
        ExecutionResult Result;
        Result.Status = ExecutionStatus::LimitExceeded;
        Result.Limit = Limit;
        Result.Origin = Origin;
        Result.Message = std::move(Message);
        Result.Statistics = Statistics;
        return Result;
      }

      const ir::VerifiedClosedModule &Closed;
      const ir::IrModule &Module;
      RuntimeWorld &World;
      ExecutionLimits Limits;
      ExecutionStatistics Statistics;
      std::vector<ActivationFrame> Frames;
      std::uint64_t CurrentStackBytes = 0;
      std::uint64_t NextActivation = 1;
      std::vector<std::uint32_t> ValueSlots;
      std::vector<std::uint32_t> FunctionValueCounts;
    };
  } // namespace

  bool ExecutionResult::returned() const noexcept
  {
    return Status == ExecutionStatus::Returned;
  }

  ExecutionResult interpret(const ir::VerifiedClosedModule &Module, ir::IrFunctionId Entry, RuntimeWorld &World, const std::vector<RuntimeValue> &Arguments, ExecutionLimits Limits)
  {
    ClosedInterpreter Interpreter(Module, World, Limits);
    return Interpreter.run(Entry, Arguments);
  }
} // namespace ink::execution
