#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_machine.h"
#include "ink/execution/engine/execution_linker.h"
#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/execution/ffi/ffi_argument.h"

#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/alloca_instruction.h"

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::resolveValue(const ir::Value &Value, ExecutionFrame &Frame)
  {
    if (consumeStep() != ExecutionStatus::Success)
    {
      return {LastStatus};
    }
    if (Frame.Owner != this || !Frame.Active)
    {
      return {LastStatus = ExecutionStatus::InvalidFrame};
    }
    if (&Value.context() != &Context)
    {
      return {LastStatus = ExecutionStatus::ForeignContext};
    }
    if (ir::Constant::classof(&Value))
    {
      const auto &Constant = static_cast<const ir::Constant &>(Value);
      if (!Context.constantPool().owns(Constant))
      {
        return {LastStatus = ExecutionStatus::ForeignContext};
      }
      return {ExecutionStatus::Success, Heap.fromConstant(Constant)};
    }
    if (ir::Function::classof(&Value))
    {
      return makeFunctionValue(static_cast<const ir::Function &>(Value));
    }
    // Semantic evaluation binds compile-time variables to places. Function SSA
    // values belong to the machine's private invocation slots, not these frames.
    for (ExecutionFrame *Current = &Frame; Current; Current = Current->Parent)
    {
      const auto Binding = Current->Bindings.find(&Value);
      if (Binding != Current->Bindings.end())
      {
        if (ir::AllocaInstruction::classof(&Value))
        {
          return {ExecutionStatus::Success, Heap.pointer(Value.type(), ExecutionPointer::fromPlace(Binding->second))};
        }
        return loadValue(Binding->second);
      }
    }
    return {LastStatus = ExecutionStatus::RuntimeValue};
  }

  ExecutionValueResult ExecutionEngine::execute(const ir::Function &Function, std::span<const ExecutionValueRef> Arguments)
  {
    if (enterEvaluation() != ExecutionStatus::Success)
    {
      return {LastStatus};
    }
    ExecutionValueResult Result;
    auto Execute = [&]() -> ExecutionValueResult
    {
      if (&Function.context() != &Context)
      {
        return {ExecutionStatus::ForeignContext};
      }
      if (Function.parameters().size() != Arguments.size())
      {
        return {ExecutionStatus::InvalidArguments};
      }
      const bool External = Function.isNativeImport();
      const bool CAbi = Function.languageLinkage() == ir::LanguageLinkage::C;
      bool HasStrings = false;
      for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
      {
        if (!Arguments[Index].type())
        {
          return {ExecutionStatus::InvalidArguments};
        }
        const ExecutionStatus Status = validateValue(Arguments[Index]);
        if (Status != ExecutionStatus::Success)
        {
          return {Status};
        }
        HasStrings = HasStrings || Arguments[Index].kind() == ExecutionValueKind::String;
        if (!External && Arguments[Index].type() != &Function.parameters()[Index]->type() && !(CAbi && Arguments[Index].kind() == ExecutionValueKind::String))
        {
          return {ExecutionStatus::TypeMismatch};
        }
      }
      if (Linker->synchronize(Context.revision()))
      {
        Machine->clearCode();
      }
      SemanticValueBridge &Bridge = Heap.bridge();
      const FunctionId Target = Bridge.lowerFunction(Function);
      const RuntimeFunctionDescriptor *Descriptor = Bridge.functionDescriptor(Target);
      if (!Descriptor)
      {
        return {ExecutionStatus::InvalidArguments};
      }
      const bool VmCAbi = CAbi && !Descriptor->External;
      const RuntimeTypeId ReturnType = Bridge.lowerType(Function.functionType().returnType());
      std::vector<FfiArgument> ConvertedStrings(VmCAbi && HasStrings ? Arguments.size() : 0);
      std::vector<RuntimeValue> Values;
      Values.reserve(Arguments.size());
      for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
      {
        const ExecutionValueRef &Argument = Arguments[Index];
        if (VmCAbi && Argument.kind() == ExecutionValueKind::String)
        {
          const ir::Type &ParameterType = Function.parameters()[Index]->type();
          const ExecutionStatus Status = ConvertedStrings[Index].prepare(Heap, ParameterType, Argument);
          if (Status != ExecutionStatus::Success)
          {
            return {Status};
          }
          Values.push_back(RuntimeValue::fromPointer(ExecutionPointer::fromBuffer(ConvertedStrings[Index].bufferRef()), Bridge.lowerType(ParameterType)));
          continue;
        }
        RuntimeValueResult Value = Bridge.lowerValue(Argument);
        if (!Value)
        {
          return {Value.Status};
        }
        Values.push_back(std::move(Value.Value));
      }
      RuntimeValueResult Executed = Machine->execute(Target, Values);
      ExecutionValueResult Raised = Executed ? Bridge.raiseValue(Heap, Executed.Value, ReturnType) : ExecutionValueResult{Executed.Status};
      if (Raised && Executed.Value.kind() == RuntimeKind::Pointer && Executed.Value.pointer().kind() == ExecutionPointer::Kind::Buffer)
      {
        for (FfiArgument &Argument : ConvertedStrings)
        {
          if (Argument.bufferRef() == Executed.Value.pointer().bufferRef())
          {
            Argument.promoteBuffer();
          }
        }
      }
      return Raised;
    };
    Result = Execute();
    leaveEvaluation();
    Result.Status = finishStatus(Result.Status);
    if (!Result)
    {
      Result.Value = {};
    }
    return Result;
  }
} // namespace ink::execution
