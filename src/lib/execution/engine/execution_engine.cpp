#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_machine.h"
#include "ink/execution/engine/execution_linker.h"
#include "ink/execution/bridge/semantic_value_bridge.h"

#include "ink/ir/context.h"

namespace ink::execution
{
  ExecutionEngine::ExecutionEngine(ir::IRContext &Context, ExecutionLimits Limits)
      : Context(Context),
        Limits(Limits),
        Heap(Context, Limits.MaxObjects, Limits.MaxStorageBytes),
        Linker(std::make_unique<ExecutionLinker>(Heap.bridge())),
        Machine(std::make_unique<ExecutionMachine>(*this, *Linker))
  {
  }

  ExecutionEngine::~ExecutionEngine() = default;

  ir::IRContext &ExecutionEngine::context() noexcept
  {
    return Context;
  }

  const ir::IRContext &ExecutionEngine::context() const noexcept
  {
    return Context;
  }

  ExecutionHeap &ExecutionEngine::heap() noexcept
  {
    return Heap;
  }

  const ExecutionHeap &ExecutionEngine::heap() const noexcept
  {
    return Heap;
  }

  ExecutionStatus ExecutionEngine::lastStatus() const noexcept
  {
    return LastStatus;
  }

  ExecutionStatus ExecutionEngine::finishStatus(ExecutionStatus Status) noexcept
  {
    if (StopStatus != ExecutionStatus::Success)
    {
      Status = StopStatus;
    }
    else if (Status == ExecutionStatus::Cancelled || Status == ExecutionStatus::BudgetExceeded)
    {
      StopStatus = Status;
    }
    return LastStatus = Status;
  }

  void ExecutionEngine::clearNativeSymbolCache() noexcept
  {
    Machine->clearNativeSymbols();
    NativeSymbols.clear();
  }

  ExecutionStatus ExecutionEngine::consumeStep()
  {
    return consumeSteps(1);
  }

  ExecutionStatus ExecutionEngine::consumeSteps(std::size_t Count)
  {
    if (StopStatus != ExecutionStatus::Success)
    {
      return LastStatus = StopStatus;
    }
    if (Count > Limits.MaxSteps - Steps)
    {
      StopStatus = ExecutionStatus::BudgetExceeded;
      return LastStatus = ExecutionStatus::BudgetExceeded;
    }
    Steps += Count;
    return LastStatus = ExecutionStatus::Success;
  }

  void ExecutionEngine::cancel() noexcept
  {
    StopStatus = ExecutionStatus::Cancelled;
    LastStatus = ExecutionStatus::Cancelled;
  }

  bool ExecutionEngine::beginInvocation() noexcept
  {
    if (ActiveCalls != 0 || EvaluationDepth != 0 || !Frames.empty())
    {
      return false;
    }
    Steps = 0;
    StopStatus = ExecutionStatus::Success;
    LastStatus = ExecutionStatus::Success;
    Heap.memoryManager().resetAllocationBudget();
    return true;
  }

  ExecutionStatus ExecutionEngine::enterEvaluation()
  {
    if (consumeStep() != ExecutionStatus::Success)
    {
      return LastStatus;
    }
    if (EvaluationDepth >= Limits.MaxEvaluationDepth)
    {
      StopStatus = ExecutionStatus::BudgetExceeded;
      return LastStatus = ExecutionStatus::BudgetExceeded;
    }
    ++EvaluationDepth;
    return ExecutionStatus::Success;
  }

  void ExecutionEngine::leaveEvaluation() noexcept
  {
    if (EvaluationDepth)
    {
      --EvaluationDepth;
    }
  }

  ExecutionFrame *ExecutionEngine::createFrame(ExecutionFrameKind Kind, ExecutionFrame *Parent)
  {
    if (consumeStep() != ExecutionStatus::Success)
    {
      return nullptr;
    }
    if ((Parent && (Parent->Owner != this || !Parent->Active)) || (Kind == ExecutionFrameKind::Module && Parent))
    {
      LastStatus = ExecutionStatus::InvalidFrame;
      return nullptr;
    }
    if (Kind == ExecutionFrameKind::Call && ActiveCalls >= Limits.MaxCallDepth)
    {
      StopStatus = ExecutionStatus::BudgetExceeded;
      LastStatus = ExecutionStatus::BudgetExceeded;
      return nullptr;
    }
    auto Frame = std::unique_ptr<ExecutionFrame>(new ExecutionFrame(*this, Kind, Parent));
    ExecutionFrame *Result = Frame.get();
    Frames.push_back(std::move(Frame));
    if (Parent)
    {
      Parent->Children.push_back(Result);
    }
    if (Kind == ExecutionFrameKind::Call)
    {
      ++ActiveCalls;
    }
    return Result;
  }

  ExecutionStatus ExecutionEngine::endFrame(ExecutionFrame &Frame)
  {
    if (Frame.Owner != this || !Frame.Active)
    {
      return LastStatus = ExecutionStatus::InvalidFrame;
    }
    if (Frame.Kind == ExecutionFrameKind::Module)
    {
      return LastStatus = ExecutionStatus::Success;
    }
    // Closing a lexical environment also ends every environment that borrows it.
    // Cleanup remains available after cancellation or budget exhaustion.
    std::vector<ExecutionFrame *> Pending = {&Frame};
    while (!Pending.empty())
    {
      ExecutionFrame *Candidate = Pending.back();
      Pending.pop_back();
      if (!Candidate->Active)
      {
        continue;
      }
      Candidate->Active = false;
      Candidate->Bindings.clear();
      for (const ExecutionStorageRef &Storage : Candidate->Storage)
      {
        Heap.release(Storage);
      }
      Candidate->Storage.clear();
      if (Candidate->Kind == ExecutionFrameKind::Call)
      {
        --ActiveCalls;
      }
      Pending.insert(Pending.end(), Candidate->Children.begin(), Candidate->Children.end());
      Candidate->Children.clear();
    }
    return LastStatus = ExecutionStatus::Success;
  }

  ExecutionPlaceResult ExecutionEngine::allocate(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable, const ir::Constant *Initial)
  {
    if (StopStatus != ExecutionStatus::Success)
    {
      return {LastStatus = StopStatus};
    }
    if (Initial && !Context.constantPool().owns(*Initial))
    {
      return {LastStatus = ExecutionStatus::ForeignContext};
    }
    const ExecutionValueRef InitialValue = Initial ? Heap.fromConstant(*Initial) : ExecutionValueRef{};
    return allocateValue(Frame, Binding, Type, Writable, Initial ? &InitialValue : nullptr);
  }

  ExecutionPlaceResult ExecutionEngine::allocateValue(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable, const ExecutionValueRef *Initial)
  {
    if (consumeStep() != ExecutionStatus::Success)
    {
      return {LastStatus};
    }
    if (Frame.Owner != this || !Frame.Active)
    {
      return {LastStatus = ExecutionStatus::InvalidFrame};
    }
    if (!Binding)
    {
      return {LastStatus = ExecutionStatus::InvalidBinding};
    }
    if (&Type.context() != &Context)
    {
      return {LastStatus = ExecutionStatus::ForeignContext};
    }
    if (Initial)
    {
      const ExecutionStatus InitialStatus = validateValue(*Initial);
      if (InitialStatus != ExecutionStatus::Success)
      {
        return {LastStatus = InitialStatus};
      }
    }
    if (Initial && Initial->type() != &Type)
    {
      return {LastStatus = ExecutionStatus::TypeMismatch};
    }
    if (Frame.Bindings.contains(Binding))
    {
      return {LastStatus = ExecutionStatus::DuplicateBinding};
    }
    const ExecutionPlaceResult Result = Heap.allocateCell(Type, Writable, Initial ? *Initial : ExecutionValueRef{});
    if (!Result)
    {
      if (Result.Status == ExecutionStatus::BudgetExceeded)
      {
        StopStatus = Result.Status;
      }
      return {LastStatus = Result.Status};
    }
    Frame.Bindings.emplace(Binding, Result.Place);
    Frame.Storage.push_back(Result.Place.storage());
    LastStatus = ExecutionStatus::Success;
    return Result;
  }

  ExecutionPlaceResult ExecutionEngine::lookup(const ExecutionFrame &Frame, const void *Binding)
  {
    if (consumeStep() != ExecutionStatus::Success)
    {
      return {LastStatus};
    }
    if (Frame.Owner != this || !Frame.Active)
    {
      return {LastStatus = ExecutionStatus::InvalidFrame};
    }
    if (!Binding)
    {
      return {LastStatus = ExecutionStatus::InvalidBinding};
    }
    for (const ExecutionFrame *Current = &Frame; Current; Current = Current->Parent)
    {
      const auto Found = Current->Bindings.find(Binding);
      if (Found != Current->Bindings.end())
      {
        return {ExecutionStatus::Success, Found->second};
      }
    }
    return {LastStatus = ExecutionStatus::UnknownBinding};
  }

  ExecutionStatus ExecutionEngine::validatePlace(ExecutionPlace Place) const noexcept
  {
    if (!Heap.owns(Place.storage()))
    {
      return ExecutionStatus::InvalidPlace;
    }
    return Place.status();
  }

  ExecutionResult ExecutionEngine::load(ExecutionPlace Place)
  {
    return freeze(loadValue(Place));
  }

  ExecutionValueResult ExecutionEngine::loadValue(ExecutionPlace Place)
  {
    if (consumeStep() != ExecutionStatus::Success)
    {
      return {LastStatus};
    }
    const ExecutionStatus PlaceStatus = validatePlace(Place);
    if (PlaceStatus != ExecutionStatus::Success)
    {
      return {LastStatus = PlaceStatus};
    }
    ExecutionValueResult Result = Heap.load(Place);
    LastStatus = Result.Status;
    return Result;
  }

  ExecutionStatus ExecutionEngine::store(ExecutionPlace Place, const ir::Constant &Value)
  {
    if (StopStatus != ExecutionStatus::Success)
    {
      return LastStatus = StopStatus;
    }
    if (!Context.constantPool().owns(Value))
    {
      return LastStatus = ExecutionStatus::ForeignContext;
    }
    return storeValue(Place, Heap.fromConstant(Value));
  }

  ExecutionStatus ExecutionEngine::storeValue(ExecutionPlace Place, const ExecutionValueRef &Value)
  {
    if (consumeStep() != ExecutionStatus::Success)
    {
      return LastStatus;
    }
    const ExecutionStatus PlaceStatus = validatePlace(Place);
    if (PlaceStatus != ExecutionStatus::Success)
    {
      return LastStatus = PlaceStatus;
    }
    const ExecutionStatus ValueStatus = validateValue(Value);
    if (ValueStatus != ExecutionStatus::Success)
    {
      return LastStatus = ValueStatus;
    }
    return LastStatus = Heap.store(Place, Value);
  }
} // namespace ink::execution
