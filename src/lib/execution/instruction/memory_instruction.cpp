#include "engine/function_executor.h"

#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/model/constant.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    template <core::DiagnosticKind Kind, typename... ArgumentTypes>
    void addExecutionFailure(ExecutionContext &Context, std::vector<core::Diagnostic> &Diagnostics, ArgumentTypes &&...Arguments)
    {
      core::Diagnostic DiagnosticEntry = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
      Context.diagnosticEngine().report(DiagnosticEntry);
      Diagnostics.push_back(std::move(DiagnosticEntry));
    }

    void addMemoryFailure(ExecutionContext &Context, std::vector<core::Diagnostic> &Diagnostics, RuntimeMemoryStatus Status, std::string_view Operation, const ir::Name &FunctionName, std::uint64_t Index, std::uint64_t RegionLength, std::uint64_t AccessSize = 0)
    {
      switch (Status)
      {
      case RuntimeMemoryStatus::AllocationSizeLimitExceeded:
        addExecutionFailure<core::DiagnosticKind::MemoryAllocationLimitExceeded>(Context, Diagnostics, RegionLength, MaximumRuntimeByteAllocationSize);
        return;
      case RuntimeMemoryStatus::AllocationCountLimitExceeded:
        addExecutionFailure<core::DiagnosticKind::MemoryAllocationCountLimitExceeded>(Context, Diagnostics, MaximumRuntimeByteAllocationCount);
        return;
      case RuntimeMemoryStatus::StorageLimitExceeded:
        addExecutionFailure<core::DiagnosticKind::MemoryStorageLimitExceeded>(Context, Diagnostics, RegionLength, MaximumRuntimeByteStorage);
        return;
      case RuntimeMemoryStatus::OutOfBounds:
        addExecutionFailure<core::DiagnosticKind::MemoryAccessOutOfBounds>(Context, Diagnostics, Operation, FunctionName, Index, AccessSize, RegionLength);
        return;
      case RuntimeMemoryStatus::AddressOverflow:
        addExecutionFailure<core::DiagnosticKind::MemoryAddressOverflow>(Context, Diagnostics, Operation, FunctionName);
        return;
      case RuntimeMemoryStatus::UntrackedPointer:
        addExecutionFailure<core::DiagnosticKind::MemoryAccessRequiresTrackedPointer>(Context, Diagnostics, Operation, FunctionName);
        return;
      case RuntimeMemoryStatus::InvalidRepresentation:
        addExecutionFailure<core::DiagnosticKind::MemoryInvalidRepresentation>(Context, Diagnostics, Operation, FunctionName, Index);
        return;
      case RuntimeMemoryStatus::LifetimeEnded:
        addExecutionFailure<core::DiagnosticKind::MemoryLifetimeEnded>(Context, Diagnostics, Operation, FunctionName);
        return;
      case RuntimeMemoryStatus::NotOwned:
        addExecutionFailure<core::DiagnosticKind::MemoryLifetimeNotOwned>(Context, Diagnostics, FunctionName);
        return;
      case RuntimeMemoryStatus::Ok:
      case RuntimeMemoryStatus::InvalidValue:
        addExecutionFailure<core::DiagnosticKind::InvalidRuntimeMemoryValue>(Context, Diagnostics, Operation, FunctionName);
        return;
      }
      addExecutionFailure<core::DiagnosticKind::InvalidRuntimeMemoryValue>(Context, Diagnostics, Operation, FunctionName);
    }

    std::optional<std::uint64_t> runtimeIndex(RuntimeValueRef Value) noexcept
    {
      if (Value == nullptr)
      {
        return std::nullopt;
      }
      return Value->integer();
    }
  } // namespace

  FunctionExecutor::InstructionFlow FunctionExecutor::executeAllocaInstruction(const ir::AllocaInstruction &Alloca, FunctionExecutionState &State)
  {
    RuntimeValueRef SizeValue = evaluateValue(*Alloca.Size, State.Module, State.Frame, State.FunctionValue.Name);
    const std::optional<std::uint64_t> Size = runtimeIndex(SizeValue);
    if (!Size.has_value())
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "alloca", State.FunctionValue.Name, 0, 0);
      return InstructionFlow::Failed;
    }

    RuntimeMemoryStatus Status = RuntimeMemoryStatus::Ok;
    RuntimeValueRef Result = Values.allocateByteSlice(*Alloca.ResultType, *Size, State.FrameId, Status);
    if (Status != RuntimeMemoryStatus::Ok || Result == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, Status == RuntimeMemoryStatus::Ok ? RuntimeMemoryStatus::InvalidValue : Status, "alloca", State.FunctionValue.Name, 0, *Size);
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(Alloca.Result, Result))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("alloca", State.FunctionValue.Name, Alloca.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeLoadInstruction(const ir::LoadInstruction &Load, FunctionExecutionState &State)
  {
    if (Load.Pointer == nullptr || Load.ResultType == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "load", State.FunctionValue.Name, 0, 0);
      return InstructionFlow::Failed;
    }
    if (!validateGlobalVariableAccessType(*Load.Pointer, State.Module, *Load.ResultType))
    {
      return InstructionFlow::Failed;
    }
    RuntimeValueRef Pointer = evaluateValue(*Load.Pointer, State.Module, State.Frame, State.FunctionValue.Name);
    if (Pointer == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "load", State.FunctionValue.Name, 0, Pointer == nullptr ? 0 : Pointer->byteLength().value_or(0));
      return InstructionFlow::Failed;
    }

    const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(*Load.ResultType, Context.compilationContext().targetContext());
    const std::size_t AccessSize = Layout.has_value() ? Layout->StrideSize : 0;
    const std::uint64_t ByteOffset = runtimePointerByteOffset(*Pointer).value_or(0);
    RuntimeValueRef Result = nullptr;
    const RuntimeMemoryStatus Status = Values.loadValue(*Pointer, *Load.ResultType, Result);
    if (Status != RuntimeMemoryStatus::Ok)
    {
      addMemoryFailure(Context, Diagnostics, Status, "load", State.FunctionValue.Name, ByteOffset, Pointer->byteLength().value_or(0), AccessSize);
      return InstructionFlow::Failed;
    }
    if (Result == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "load", State.FunctionValue.Name, ByteOffset, Pointer->byteLength().value_or(0));
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(Load.Result, Result))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("load", State.FunctionValue.Name, Load.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeStoreInstruction(const ir::StoreInstruction &Store, FunctionExecutionState &State)
  {
    if (Store.Pointer == nullptr || Store.StoredValue == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "store", State.FunctionValue.Name, 0, 0);
      return InstructionFlow::Failed;
    }
    if (!validateGlobalVariableAccessType(*Store.Pointer, State.Module, Store.StoredValue->type()))
    {
      return InstructionFlow::Failed;
    }
    RuntimeValueRef StoredValue = evaluateValue(*Store.StoredValue, State.Module, State.Frame, State.FunctionValue.Name);
    RuntimeValueRef Pointer = evaluateValue(*Store.Pointer, State.Module, State.Frame, State.FunctionValue.Name);
    if (Pointer == nullptr || StoredValue == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "store", State.FunctionValue.Name, 0, Pointer == nullptr ? 0 : Pointer->byteLength().value_or(0));
      return InstructionFlow::Failed;
    }

    const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(StoredValue->type(), Context.compilationContext().targetContext());
    const std::size_t AccessSize = Layout.has_value() ? Layout->StrideSize : 0;
    const std::uint64_t ByteOffset = runtimePointerByteOffset(*Pointer).value_or(0);
    const RuntimeMemoryStatus Status = Values.storeValue(*Pointer, *StoredValue);
    if (Status != RuntimeMemoryStatus::Ok)
    {
      addMemoryFailure(Context, Diagnostics, Status, "store", State.FunctionValue.Name, ByteOffset, Pointer->byteLength().value_or(0), AccessSize);
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeGetElementPointerInstruction(const ir::GetElementPointerInstruction &GetElementPointer, FunctionExecutionState &State)
  {
    if (!GetElementPointer.Pointer || !GetElementPointer.Index || GetElementPointer.ResultType == nullptr || GetElementPointer.ElementType == nullptr || GetElementPointer.Index->type().kind() != ir::TypeKind::PointerSize)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "getelementptr", State.FunctionValue.Name, 0, 0);
      return InstructionFlow::Failed;
    }
    RuntimeValueRef Pointer = evaluateValue(*GetElementPointer.Pointer, State.Module, State.Frame, State.FunctionValue.Name);
    RuntimeValueRef IndexValue = evaluateValue(*GetElementPointer.Index, State.Module, State.Frame, State.FunctionValue.Name);
    const std::optional<std::uint64_t> Index = runtimeIndex(IndexValue);
    if (Pointer == nullptr || IndexValue == nullptr || IndexValue->type().kind() != ir::TypeKind::PointerSize || !Index.has_value())
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "getelementptr", State.FunctionValue.Name, Index.value_or(0), Pointer == nullptr ? 0 : Pointer->byteLength().value_or(0));
      return InstructionFlow::Failed;
    }
    std::vector<std::uint32_t> FieldIndices;
    FieldIndices.reserve(GetElementPointer.FieldIndices.size());
    for (const std::unique_ptr<ir::Value> &FieldIndexValue : GetElementPointer.FieldIndices)
    {
      if (!FieldIndexValue || FieldIndexValue->type().kind() != ir::TypeKind::I32 || FieldIndexValue->kind() != ir::ValueKind::IntegerConstant)
      {
        addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "getelementptr", State.FunctionValue.Name, *Index, Pointer->byteLength().value_or(0));
        return InstructionFlow::Failed;
      }
      const ir::IntegerConstant &FieldIndex = static_cast<const ir::IntegerConstant &>(*FieldIndexValue);
      if (FieldIndex.isNegative() || FieldIndex.unsignedValue() > static_cast<std::uint64_t>(std::numeric_limits<std::int32_t>::max()))
      {
        addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "getelementptr", State.FunctionValue.Name, *Index, Pointer->byteLength().value_or(0));
        return InstructionFlow::Failed;
      }
      FieldIndices.push_back(static_cast<std::uint32_t>(FieldIndex.unsignedValue()));
    }

    RuntimeMemoryStatus Status = RuntimeMemoryStatus::Ok;
    RuntimeValueRef Result = Values.getElementPointer(*GetElementPointer.ResultType, *Pointer, *GetElementPointer.ElementType, *Index, FieldIndices, Status);
    if (Status != RuntimeMemoryStatus::Ok || Result == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, Status == RuntimeMemoryStatus::Ok ? RuntimeMemoryStatus::InvalidValue : Status, "getelementptr", State.FunctionValue.Name, *Index, Pointer->byteLength().value_or(0));
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(GetElementPointer.Result, Result))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("getelementptr", State.FunctionValue.Name, GetElementPointer.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeLifetimeEndInstruction(const ir::LifetimeEndInstruction &LifetimeEnd, FunctionExecutionState &State)
  {
    RuntimeValueRef Slice = evaluateValue(*LifetimeEnd.Slice, State.Module, State.Frame, State.FunctionValue.Name);
    if (Slice == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "lifetime.end", State.FunctionValue.Name, 0, 0);
      return InstructionFlow::Failed;
    }

    const RuntimeMemoryStatus Status = Values.endByteSliceLifetime(*Slice, State.FrameId);
    if (Status != RuntimeMemoryStatus::Ok)
    {
      addMemoryFailure(Context, Diagnostics, Status, "lifetime.end", State.FunctionValue.Name, 0, Slice->byteLength().value_or(0));
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeSliceDataInstruction(const ir::SliceDataInstruction &SliceData, FunctionExecutionState &State)
  {
    RuntimeValueRef Slice = evaluateValue(*SliceData.Slice, State.Module, State.Frame, State.FunctionValue.Name);
    if (Slice == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "slice.data", State.FunctionValue.Name, 0, 0);
      return InstructionFlow::Failed;
    }
    if (!Slice->memoryAlive())
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::LifetimeEnded, "slice.data", State.FunctionValue.Name, 0, Slice->byteLength().value_or(0));
      return InstructionFlow::Failed;
    }

    RuntimeValueRef Result = Values.pointerFromByteSlice(*SliceData.ResultType, *Slice);
    if (Result == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "slice.data", State.FunctionValue.Name, 0, Slice->byteLength().value_or(0));
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(SliceData.Result, Result))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("slice.data", State.FunctionValue.Name, SliceData.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeSliceLengthInstruction(const ir::SliceLengthInstruction &SliceLength, FunctionExecutionState &State)
  {
    RuntimeValueRef Slice = evaluateValue(*SliceLength.Slice, State.Module, State.Frame, State.FunctionValue.Name);
    const std::optional<std::size_t> Size = Slice == nullptr ? std::nullopt : Slice->byteLength();
    if (!Size.has_value())
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "slice.length", State.FunctionValue.Name, 0, 0);
      return InstructionFlow::Failed;
    }

    RuntimeValueRef Result = Values.integerValue(*SliceLength.ResultType, *Size);
    if (Result == nullptr)
    {
      addMemoryFailure(Context, Diagnostics, RuntimeMemoryStatus::InvalidValue, "slice.length", State.FunctionValue.Name, 0, *Size);
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(SliceLength.Result, Result))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("slice.length", State.FunctionValue.Name, SliceLength.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }
} // namespace ink::execution
