#include "engine/function_executor.h"

#include "ink/ir/model/constant.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/operand.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    constexpr std::size_t MaximumCallDepth = 256;
    constexpr std::size_t MaximumInstructionCount = 1000000;
  } // namespace

  FunctionExecutor::FunctionExecutor(ExecutionContext &Context, ModuleExecutionRuntime &Runtime, ModuleInstance &EntryModule, std::vector<core::Diagnostic> &Diagnostics)
      : Context(Context),
        Runtime(Runtime),
        EntryModule(EntryModule),
        Diagnostics(Diagnostics),
        Values(Context.compilationContext().targetContext())
  {
  }

  bool FunctionExecutor::execute(ir::FunctionId Function, const std::vector<RuntimeValueRef> &Arguments, RuntimeValueRef &Result)
  {
    return executeFunction(EntryModule, Function, Arguments, 0, Result);
  }

  RuntimeValueRef FunctionExecutor::importValue(RuntimeValueRef Value)
  {
    if (Value == nullptr || Values.owns(Value))
    {
      return Value;
    }
    const auto Existing = ImportedValues.find(Value);
    if (Existing != ImportedValues.end())
    {
      return Existing->second;
    }
    RuntimeValueRef Imported = Values.clone(*Value);
    if (Imported != nullptr)
    {
      ImportedValues.emplace(Value, Imported);
    }
    return Imported;
  }

  RuntimeValueRef FunctionExecutor::zeroValue(const ir::Type &TypeValue)
  {
    switch (TypeValue.kind())
    {
    case ir::TypeKind::Void:
      return Values.voidValue(TypeValue);
    case ir::TypeKind::Bool:
    case ir::TypeKind::Byte:
    case ir::TypeKind::I32:
    case ir::TypeKind::PointerSize:
      return Values.integerValue(TypeValue, 0);
    case ir::TypeKind::F16:
    case ir::TypeKind::F32:
    case ir::TypeKind::F64:
      return Values.floatingPointValue(TypeValue, 0);
    case ir::TypeKind::BytePointer:
      return Values.mutablePointerValue(TypeValue, nullptr);
    case ir::TypeKind::ConstBytePointer:
      return Values.pointerValue(TypeValue, nullptr);
    case ir::TypeKind::ByteSlice:
      return Values.mutableByteSliceValue(TypeValue, nullptr, 0);
    case ir::TypeKind::ConstByteSlice:
      return Values.byteSliceValue(TypeValue, nullptr, 0);
    case ir::TypeKind::Struct:
    {
      const ir::StructType &Struct = static_cast<const ir::StructType &>(TypeValue);
      std::vector<RuntimeValueRef> Fields;
      Fields.reserve(Struct.fieldTypes().size());
      for (const ir::Type *FieldType : Struct.fieldTypes())
      {
        RuntimeValueRef Field = zeroValue(*FieldType);
        if (Field == nullptr)
        {
          return nullptr;
        }
        Fields.push_back(Field);
      }
      return Values.aggregateValue(Struct, std::move(Fields));
    }
    case ir::TypeKind::Count:
      return nullptr;
    }
    return nullptr;
  }

  RuntimeValueRef FunctionExecutor::evaluateValue(const ir::Value &Value, ModuleInstance &Module, const ExecutionFrame &Frame, const ir::Name &FunctionName)
  {
    if (Value.kind() == ir::ValueKind::IntegerConstant)
    {
      const std::uint64_t Integer = static_cast<const ir::IntegerConstant &>(Value).unsignedValue();
      return Values.integerValue(Value.type(), Integer);
    }
    if (Value.kind() == ir::ValueKind::FloatConstant)
    {
      const std::uint64_t Bits = static_cast<const ir::FloatConstant &>(Value).bitPattern();
      return Values.floatingPointValue(Value.type(), Bits);
    }
    if (Value.kind() == ir::ValueKind::StringConstant)
    {
      RuntimeValueRef Result = importValue(Module.stringConstantValue(static_cast<const ir::StringConstant &>(Value)));
      if (Result == nullptr)
      {
        addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
      }
      return Result;
    }
    if (Value.kind() == ir::ValueKind::NullConstant)
    {
      if (Value.type().kind() == ir::TypeKind::BytePointer)
      {
        return Values.mutablePointerValue(Value.type(), nullptr);
      }
      if (Value.type().kind() == ir::TypeKind::ConstBytePointer)
      {
        return Values.pointerValue(Value.type(), nullptr);
      }
      addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
      return nullptr;
    }
    if (Value.kind() == ir::ValueKind::AggregateConstant)
    {
      if (Value.type().kind() != ir::TypeKind::Struct)
      {
        addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>("aggregate constant", FunctionName);
        return nullptr;
      }
      const ir::AggregateConstant &Constant = static_cast<const ir::AggregateConstant &>(Value);
      std::vector<RuntimeValueRef> Elements;
      Elements.reserve(Constant.elements().size());
      for (const auto &Element : Constant.elements())
      {
        if (!Element)
        {
          addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>("aggregate constant", FunctionName);
          return nullptr;
        }
        RuntimeValueRef RuntimeElement = evaluateValue(*Element, Module, Frame, FunctionName);
        if (RuntimeElement == nullptr)
        {
          return nullptr;
        }
        Elements.push_back(RuntimeElement);
      }
      RuntimeValueRef Result = Values.aggregateValue(static_cast<const ir::StructType &>(Value.type()), std::move(Elements));
      if (Result == nullptr)
      {
        addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>("aggregate constant", FunctionName);
      }
      return Result;
    }
    if (Value.kind() == ir::ValueKind::ValueOperand)
    {
      const ir::ValueId Id = static_cast<const ir::ValueOperand &>(Value).id();
      RuntimeValueRef Stored = Frame.find(Id);
      if (Stored == nullptr)
      {
        addFailure<core::DiagnosticKind::SsaValueUnavailableDuringExecution>(Id.value());
        return nullptr;
      }
      RuntimeValueRef Result = importValue(Stored);
      if (Result == nullptr)
      {
        addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
      }
      return Result;
    }
    if (Value.kind() == ir::ValueKind::GlobalAddressOperand)
    {
      const ir::GlobalAddressOperand &Address = static_cast<const ir::GlobalAddressOperand &>(Value);
      const ir::Module &ModuleValue = Module.definition();
      const std::size_t GlobalIndex = Address.byteConstant().value();
      if (!Address.byteConstant().valid() || GlobalIndex >= ModuleValue.ByteConstants.size())
      {
        addFailure<core::DiagnosticKind::InvalidRuntimeMemoryValue>("global address", FunctionName);
        return nullptr;
      }
      const ir::ByteConstant *Constant = &ModuleValue.ByteConstants[GlobalIndex];
      RuntimeValueRef BasePointer = nullptr;
      const auto Existing = GlobalPointers.find(Constant);
      if (Existing != GlobalPointers.end())
      {
        BasePointer = Existing->second;
      }
      else
      {
        BasePointer = importValue(Module.byteConstantAddress(Address.byteConstant()));
        if (BasePointer == nullptr)
        {
          addFailure<core::DiagnosticKind::InvalidRuntimeMemoryValue>("global address", FunctionName);
          return nullptr;
        }
        GlobalPointers.emplace(Constant, BasePointer);
      }
      if (Address.byteOffset() == 0)
      {
        return BasePointer;
      }
      RuntimeMemoryStatus Status = RuntimeMemoryStatus::Ok;
      RuntimeValueRef Result = Values.getElementPointer(Value.type(), *BasePointer, ModuleValue.context().getType(ir::TypeKind::Byte), Address.byteOffset(), Status);
      if (Result == nullptr)
      {
        if (Status == RuntimeMemoryStatus::AddressOverflow)
        {
          addFailure<core::DiagnosticKind::MemoryAddressOverflow>("global address", FunctionName);
        }
        else
        {
          addFailure<core::DiagnosticKind::InvalidRuntimeMemoryValue>("global address", FunctionName);
        }
      }
      return Result;
    }
    if (Value.kind() == ir::ValueKind::GlobalVariableAddressOperand)
    {
      return evaluateGlobalVariableAddress(static_cast<const ir::GlobalVariableAddressOperand &>(Value), Module, FunctionName);
    }
    if (Value.kind() == ir::ValueKind::ZeroInitializer)
    {
      RuntimeValueRef Result = zeroValue(Value.type());
      if (Result == nullptr)
      {
        addFailure<core::DiagnosticKind::ZeroInitializerConstructionFailed>(ir::typeKindName(Value.type().kind()));
      }
      return Result;
    }
    addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
    return nullptr;
  }

  bool FunctionExecutor::executeFunction(ModuleInstance &Module, ir::FunctionId Function, const std::vector<RuntimeValueRef> &Arguments, std::size_t Depth, RuntimeValueRef &Result)
  {
    const ir::Module &ModuleValue = Module.definition();
    if (!Function.valid() || Function.value() >= ModuleValue.Functions.size())
    {
      addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(Module.name(), Function.valid() ? Function.value() : ir::InvalidId);
      return false;
    }
    const ir::Function &FunctionValue = ModuleValue.Functions[Function.value()];
    if (Depth >= MaximumCallDepth)
    {
      addFailure<core::DiagnosticKind::CallDepthLimitExceeded>(FunctionValue.Name, MaximumCallDepth);
      return false;
    }
    if (FunctionValue.Kind == ir::FunctionKind::External)
    {
      ExternalFunctionInvoker *Invoker = Runtime.externalInvoker(Module);
      if (Invoker == nullptr)
      {
        addFailure<core::DiagnosticKind::ModuleLoadFailed>(Module.name());
        return false;
      }
      return Invoker->invokeExternal(Function.value(), Arguments, Values, Result, Diagnostics);
    }

    FunctionExecutionState State(Module, FunctionValue, Arguments, Depth, NextFrameId++);
    State.NextBlock = ir::BlockId{0};
    if (!enterBlock(State))
    {
      Values.endFrameLifetimes(State.FrameId);
      return false;
    }

    while (State.NextBlock.valid() && State.NextBlock.value() < FunctionValue.Blocks.size())
    {
      const ir::BasicBlock &Block = FunctionValue.Blocks[State.NextBlock.value()];
      bool Branched = false;
      for (const std::unique_ptr<ir::Instruction> &InstructionPointer : Block.Instructions)
      {
        if (!consumeInstructionStep(FunctionValue))
        {
          Values.endFrameLifetimes(State.FrameId);
          return false;
        }
        const InstructionFlow Flow = executeInstruction(*InstructionPointer, State);
        if (Flow == InstructionFlow::Failed)
        {
          Values.endFrameLifetimes(State.FrameId);
          return false;
        }
        if (Flow == InstructionFlow::Return)
        {
          Result = State.ReturnValue;
          Values.endFrameLifetimes(State.FrameId);
          return true;
        }
        if (Flow == InstructionFlow::Branch)
        {
          Branched = true;
          break;
        }
      }
      if (!Branched || !enterBlock(State))
      {
        Values.endFrameLifetimes(State.FrameId);
        if (!Branched)
        {
          addFailure<core::DiagnosticKind::FunctionMissingReturnDuringExecution>(FunctionValue.Name);
        }
        return false;
      }
    }

    Values.endFrameLifetimes(State.FrameId);
    addFailure<core::DiagnosticKind::FunctionMissingReturnDuringExecution>(FunctionValue.Name);
    return false;
  }

  bool FunctionExecutor::enterBlock(FunctionExecutionState &State)
  {
    if (!State.NextBlock.valid() || State.NextBlock.value() >= State.FunctionValue.Blocks.size())
    {
      addFailure<core::DiagnosticKind::InvalidBranchTargetDuringExecution>(State.FunctionValue.Name);
      return false;
    }
    const ir::BasicBlock &Block = State.FunctionValue.Blocks[State.NextBlock.value()];
    std::size_t PhiCount = 0;
    while (PhiCount < Block.Instructions.size() && Block.Instructions[PhiCount] && Block.Instructions[PhiCount]->kind() == ir::InstructionKind::Phi)
    {
      ++PhiCount;
    }
    for (std::size_t InstructionIndex = PhiCount; InstructionIndex < Block.Instructions.size(); ++InstructionIndex)
    {
      if (Block.Instructions[InstructionIndex] && Block.Instructions[InstructionIndex]->kind() == ir::InstructionKind::Phi)
      {
        addFailure<core::DiagnosticKind::PhiIncomingMismatchDuringExecution>(State.FunctionValue.Name);
        return false;
      }
    }
    if (PhiCount == 0)
    {
      return true;
    }
    if (!State.PreviousBlock.valid())
    {
      addFailure<core::DiagnosticKind::PhiIncomingMismatchDuringExecution>(State.FunctionValue.Name);
      return false;
    }

    std::vector<std::pair<ir::ValueId, RuntimeValueRef>> Bindings;
    Bindings.reserve(PhiCount);
    for (std::size_t PhiIndex = 0; PhiIndex < PhiCount; ++PhiIndex)
    {
      const ir::PhiInstruction &Phi = static_cast<const ir::PhiInstruction &>(*Block.Instructions[PhiIndex]);
      if (!Phi.Result.valid() || Phi.ResultType == nullptr || Phi.ResultType->kind() == ir::TypeKind::Void)
      {
        addFailure<core::DiagnosticKind::PhiIncomingMismatchDuringExecution>(State.FunctionValue.Name);
        return false;
      }

      const ir::PhiIncoming *SelectedIncoming = nullptr;
      for (const ir::PhiIncoming &Incoming : Phi.IncomingValues)
      {
        if (Incoming.Predecessor == State.PreviousBlock)
        {
          SelectedIncoming = &Incoming;
          break;
        }
      }
      if (SelectedIncoming == nullptr || !SelectedIncoming->Value || &SelectedIncoming->Value->type() != Phi.ResultType)
      {
        addFailure<core::DiagnosticKind::PhiIncomingMismatchDuringExecution>(State.FunctionValue.Name);
        return false;
      }

      RuntimeValueRef IncomingValue = evaluateValue(*SelectedIncoming->Value, State.Module, State.Frame, State.FunctionValue.Name);
      if (IncomingValue == nullptr)
      {
        return false;
      }
      if (&IncomingValue->type() != Phi.ResultType)
      {
        addFailure<core::DiagnosticKind::PhiIncomingMismatchDuringExecution>(State.FunctionValue.Name);
        return false;
      }
      Bindings.emplace_back(Phi.Result, IncomingValue);
    }

    for (const std::pair<ir::ValueId, RuntimeValueRef> &Binding : Bindings)
    {
      if (!State.Frame.define(Binding.first, Binding.second))
      {
        addFailure<core::DiagnosticKind::PhiIncomingMismatchDuringExecution>(State.FunctionValue.Name);
        return false;
      }
    }
    return true;
  }

  bool FunctionExecutor::consumeInstructionStep(const ir::Function &FunctionValue)
  {
    if (ExecutedInstructionCount >= MaximumInstructionCount)
    {
      addFailure<core::DiagnosticKind::ExecutionStepLimitExceeded>(FunctionValue.Name, MaximumInstructionCount);
      return false;
    }
    ++ExecutedInstructionCount;
    return true;
  }
} // namespace ink::execution
