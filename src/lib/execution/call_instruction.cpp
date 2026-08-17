#include "function_executor.h"

#include <memory>
#include <vector>

namespace ink::execution
{
  FunctionExecutor::InstructionFlow FunctionExecutor::executeCallInstruction(const ir::CallInstruction &Call, FunctionExecutionState &State)
  {
    ModuleInstance *CalleeModule = Runtime.resolveReferencedModule(State.Module, Call.Callee.Module, Diagnostics);
    if (CalleeModule == nullptr)
    {
      return InstructionFlow::Failed;
    }
    const ir::Module &CalleeDefinition = CalleeModule->definition();
    if (!Call.Callee.Function.valid() || Call.Callee.Function.value() >= CalleeDefinition.Functions.size())
    {
      addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(CalleeModule->id().value(), Call.Callee.Function.valid() ? Call.Callee.Function.value() : ir::InvalidId);
      return InstructionFlow::Failed;
    }
    if ((CalleeDefinition.Initializer.has_value() && *CalleeDefinition.Initializer == Call.Callee.Function) || (CalleeDefinition.Finalizer.has_value() && *CalleeDefinition.Finalizer == Call.Callee.Function))
    {
      addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(CalleeModule->id().value(), Call.Callee.Function.value());
      return InstructionFlow::Failed;
    }

    const ir::Function &Callee = CalleeDefinition.Functions[Call.Callee.Function.value()];
    if (Call.ResultType != Callee.ResultType || Call.Arguments.size() != Callee.ParameterTypes.size())
    {
      addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(CalleeModule->id().value(), Call.Callee.Function.value());
      return InstructionFlow::Failed;
    }
    if (Callee.Kind == ir::FunctionKind::External && !Context.compilationContext().targetContext().isNativeAbiCompatible())
    {
      addFailure<core::DiagnosticKind::ExternalFunctionTargetUnsupported>(Callee.Name);
      return InstructionFlow::Failed;
    }

    std::vector<RuntimeValueRef> CallArguments;
    CallArguments.reserve(Call.Arguments.size());
    for (std::size_t ArgumentIndex = 0; ArgumentIndex < Call.Arguments.size(); ++ArgumentIndex)
    {
      const std::unique_ptr<ir::Value> &Argument = Call.Arguments[ArgumentIndex];
      if (Argument == nullptr || &Argument->type() != Callee.ParameterTypes[ArgumentIndex])
      {
        addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(CalleeModule->id().value(), Call.Callee.Function.value());
        return InstructionFlow::Failed;
      }
      RuntimeValueRef ArgumentValue = evaluateValue(*Argument, State.Module, State.Frame, State.FunctionValue.Name);
      if (ArgumentValue == nullptr || &ArgumentValue->type() != Callee.ParameterTypes[ArgumentIndex])
      {
        return InstructionFlow::Failed;
      }
      CallArguments.push_back(ArgumentValue);
    }

    RuntimeValueRef CallResult = nullptr;
    if (!executeFunction(*CalleeModule, Call.Callee.Function, CallArguments, State.Depth + 1, CallResult))
    {
      return InstructionFlow::Failed;
    }
    if (!Call.Result.has_value())
    {
      return InstructionFlow::Continue;
    }
    if (CallResult == nullptr || &CallResult->type() != Call.ResultType)
    {
      addFailure<core::DiagnosticKind::CallResultMissing>(State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(*Call.Result, CallResult))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("call", State.FunctionValue.Name, Call.Result->value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }
} // namespace ink::execution
