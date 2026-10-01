#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/ffi/external_function.h"

#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

namespace ink::execution
{
  namespace
  {
    class CallFrameGuard final
    {
      public:
        CallFrameGuard(ExecutionEngine &Engine, ExecutionFrame &Frame) noexcept
            : Engine(Engine),
              Frame(Frame)
        {
        }

        ~CallFrameGuard()
        {
          if (Frame.active())
          {
            Engine.endFrame(Frame);
          }
        }

        CallFrameGuard(const CallFrameGuard &) = delete;
        CallFrameGuard &operator=(const CallFrameGuard &) = delete;

      private:
        ExecutionEngine &Engine;
        ExecutionFrame &Frame;
    };
  } // namespace

  ExecutionEngine::ExecutionEngine(ir::IRContext &Context, ExecutionLimits Limits)
      : Context(Context),
        Limits(Limits),
        Heap(Context, Limits.MaxObjects)
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

  void ExecutionEngine::clearNativeSymbolCache() noexcept
  {
    NativeSymbols.clear();
  }

  ExecutionStatus ExecutionEngine::consumeStep()
  {
    if (StopStatus != ExecutionStatus::Success)
    {
      return LastStatus = StopStatus;
    }
    if (Steps >= Limits.MaxSteps)
    {
      StopStatus = ExecutionStatus::BudgetExceeded;
      return LastStatus = ExecutionStatus::BudgetExceeded;
    }
    ++Steps;
    return LastStatus = ExecutionStatus::Success;
  }

  void ExecutionEngine::cancel() noexcept
  {
    StopStatus = ExecutionStatus::Cancelled;
    LastStatus = ExecutionStatus::Cancelled;
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
      Candidate->Events.clear();
      Candidate->Values.clear();
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

  ExecutionPlaceResult ExecutionEngine::bindRuntime(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable)
  {
    return allocateObject(Frame, Binding, Type, Writable, nullptr, true);
  }

  ExecutionPlaceResult ExecutionEngine::allocateValue(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable, const ExecutionValueRef *Initial)
  {
    return allocateObject(Frame, Binding, Type, Writable, Initial, false);
  }

  ExecutionPlaceResult ExecutionEngine::allocateObject(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable, const ExecutionValueRef *Initial, bool Runtime)
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
    if (Initial && validateValue(*Initial) != ExecutionStatus::Success)
    {
      return {LastStatus = validateValue(*Initial)};
    }
    if (Initial && Initial->type() != &Type)
    {
      return {LastStatus = ExecutionStatus::TypeMismatch};
    }
    if (Frame.Bindings.contains(Binding))
    {
      return {LastStatus = ExecutionStatus::DuplicateBinding};
    }
    const ExecutionPlaceResult Result = Heap.allocateCell(Type, Writable, Initial ? *Initial : ExecutionValueRef{}, Runtime);
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

  ExecutionResult ExecutionEngine::executeOnce(ExecutionFrame &Frame, const void *EventKey, const std::function<ExecutionResult()> &Callback, bool *ReusedResult)
  {
    if (ReusedResult)
    {
      *ReusedResult = false;
    }
    if (consumeStep() != ExecutionStatus::Success)
    {
      return {LastStatus};
    }
    if (Frame.Owner != this || !Frame.Active)
    {
      return {LastStatus = ExecutionStatus::InvalidFrame};
    }
    if (!EventKey || !Callback)
    {
      return {LastStatus = ExecutionStatus::InvalidBinding};
    }
    const auto Found = Frame.Events.find(EventKey);
    if (Found != Frame.Events.end())
    {
      if (Found->second.Running)
      {
        return {LastStatus = ExecutionStatus::RecursiveEvent};
      }
      if (ReusedResult)
      {
        *ReusedResult = true;
      }
      LastStatus = Found->second.Result.Status;
      return Found->second.Result;
    }
    Frame.Events.emplace(EventKey, ExecutionFrame::Event{});
    ExecutionResult Result = Callback();
    // A callback can grow event tables or end the frame. Reacquire by key after it
    // returns instead of retaining an iterator or a reference across reentrant work.
    if (!Frame.Active)
    {
      return {LastStatus = ExecutionStatus::InvalidFrame};
    }
    if (StopStatus != ExecutionStatus::Success)
    {
      Result = {StopStatus};
    }
    if (Result.Value && !Context.constantPool().owns(*Result.Value))
    {
      Result = {ExecutionStatus::ForeignContext};
    }
    if (Result.Status == ExecutionStatus::Cancelled || Result.Status == ExecutionStatus::BudgetExceeded)
    {
      StopStatus = Result.Status;
      Frame.Events.erase(EventKey);
    }
    else
    {
      auto &Event = Frame.Events.find(EventKey)->second;
      Event.Running = false;
      Event.Result = Result;
    }
    LastStatus = Result.Status;
    return Result;
  }

  ExecutionResult ExecutionEngine::call(const ir::Function &Function, ExecutionFrame &DefinitionFrame, std::span<const ir::Value *const> Arguments, const std::function<ExecutionResult(ExecutionFrame &)> &Body)
  {
    if (consumeStep() != ExecutionStatus::Success)
    {
      return {LastStatus};
    }
    if (&Function.context() != &Context)
    {
      return {LastStatus = ExecutionStatus::ForeignContext};
    }
    if (DefinitionFrame.Owner != this || !DefinitionFrame.Active)
    {
      return {LastStatus = ExecutionStatus::InvalidFrame};
    }
    const bool External = Function.languageLinkage() == ir::LanguageLinkage::C;
    if (!External && !Body)
    {
      return {LastStatus = ExecutionStatus::MissingBody};
    }
    const auto &Parameters = Function.parameters();
    if (Parameters.size() != Arguments.size())
    {
      return {LastStatus = ExecutionStatus::InvalidArguments};
    }
    std::vector<ExecutionValueRef> EvaluatedArguments;
    EvaluatedArguments.reserve(Arguments.size());
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      if (!Arguments[Index])
      {
        return {LastStatus = ExecutionStatus::InvalidArguments};
      }
      if (&Arguments[Index]->context() != &Context)
      {
        return {LastStatus = ExecutionStatus::ForeignContext};
      }
      if (!External && &Arguments[Index]->type() != &Parameters[Index]->type())
      {
        return {LastStatus = ExecutionStatus::TypeMismatch};
      }
      ExecutionValueResult Argument = evaluate(*Arguments[Index], DefinitionFrame);
      if (!Argument)
      {
        return {LastStatus = Argument.Status};
      }
      EvaluatedArguments.push_back(std::move(Argument.Value));
    }
    ExecutionFrame *Frame = createFrame(ExecutionFrameKind::Call, &DefinitionFrame);
    if (!Frame)
    {
      return {LastStatus};
    }
    ExecutionResult Result;
    {
      CallFrameGuard Guard(*this, *Frame);
      for (std::size_t Index = 0; !External && Index < Arguments.size(); ++Index)
      {
        const ExecutionPlaceResult Parameter = allocateValue(*Frame, Parameters[Index].get(), Parameters[Index]->type(), true, &EvaluatedArguments[Index]);
        if (!Parameter)
        {
          Result = {Parameter.Status};
          break;
        }
      }
      if (Result)
      {
        Result = External ? freeze(callExternalFunction(Heap, NativeSymbols, Function, EvaluatedArguments)) : Body(*Frame);
        if (!Frame->active() && Result.Status != ExecutionStatus::Cancelled && Result.Status != ExecutionStatus::BudgetExceeded)
        {
          Result = {ExecutionStatus::InvalidFrame};
        }
      }
    }
    // Cleanup may change LastStatus; terminal execution state takes precedence
    // over a callback's reported result and can never be swallowed by a caller.
    if (StopStatus != ExecutionStatus::Success)
    {
      Result = {StopStatus};
    }
    if (Result.Status == ExecutionStatus::Cancelled || Result.Status == ExecutionStatus::BudgetExceeded)
    {
      StopStatus = Result.Status;
    }
    if (Result)
    {
      const ir::Type &ReturnType = Function.functionType().returnType();
      if (Result.Value && !Context.constantPool().owns(*Result.Value))
      {
        Result = {ExecutionStatus::ForeignContext};
      }
      else if (ReturnType.typeKind() == ir::TypeKind::Void ? Result.Value != nullptr : (!Result.Value || &Result.Value->type() != &ReturnType))
      {
        Result = {ExecutionStatus::TypeMismatch};
      }
    }
    if (!Result)
    {
      Result.Value = nullptr;
    }
    LastStatus = Result.Status;
    return Result;
  }
} // namespace ink::execution
