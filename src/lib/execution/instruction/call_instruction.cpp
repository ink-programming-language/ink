#include "engine/function_executor.h"

#include "ink/execution/module/module_loader.h"

#include <memory>
#include <vector>

namespace ink::execution
{
  FunctionExecutor::InstructionFlow FunctionExecutor::executeCallInstruction(const ir::CallInstruction &Call, FunctionExecutionState &State)
  {
    const ir::Module &CallerDefinition = State.Module.definition();
    if (!Call.Callee.valid() || Call.Callee.value() >= CallerDefinition.Functions.size())
    {
      addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(State.Module.name(), Call.Callee.valid() ? Call.Callee.value() : ir::InvalidId);
      return InstructionFlow::Failed;
    }
    if ((CallerDefinition.Initializer.has_value() && *CallerDefinition.Initializer == Call.Callee) || (CallerDefinition.Finalizer.has_value() && *CallerDefinition.Finalizer == Call.Callee))
    {
      addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(State.Module.name(), Call.Callee.value());
      return InstructionFlow::Failed;
    }

    const ir::Function &DeclaredCallee = CallerDefinition.Functions[Call.Callee.value()];
    if (Call.ResultType != DeclaredCallee.ResultType || Call.Arguments.size() != DeclaredCallee.ParameterTypes.size())
    {
      addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(State.Module.name(), Call.Callee.value());
      return InstructionFlow::Failed;
    }

    ModuleInstance *CalleeModule = &State.Module;
    ir::FunctionId CalleeFunction = Call.Callee;
    const ir::Function *Callee = &DeclaredCallee;
    for (std::size_t ImportDepth = 0; Callee->Kind == ir::FunctionKind::Imported; ++ImportDepth)
    {
      if (ImportDepth >= MaximumModuleImportDepth)
      {
        addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(CalleeModule->name(), CalleeFunction.value());
        return InstructionFlow::Failed;
      }
      if (!Runtime.resolveImportedFunction(*CalleeModule, CalleeFunction, CalleeModule, CalleeFunction, Diagnostics))
      {
        return InstructionFlow::Failed;
      }
      const ir::Module &CalleeDefinition = CalleeModule->definition();
      if (!CalleeFunction.valid() || CalleeFunction.value() >= CalleeDefinition.Functions.size())
      {
        addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(CalleeModule->name(), CalleeFunction.valid() ? CalleeFunction.value() : ir::InvalidId);
        return InstructionFlow::Failed;
      }
      Callee = &CalleeDefinition.Functions[CalleeFunction.value()];
    }
    if (Call.ResultType != Callee->ResultType || Call.Arguments.size() != Callee->ParameterTypes.size())
    {
      addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(CalleeModule->name(), CalleeFunction.value());
      return InstructionFlow::Failed;
    }
    if (Callee->Kind == ir::FunctionKind::External && !Context.compilationContext().targetContext().isNativeAbiCompatible())
    {
      addFailure<core::DiagnosticKind::ExternalFunctionTargetUnsupported>(Callee->Name);
      return InstructionFlow::Failed;
    }

    std::vector<RuntimeValueRef> CallArguments;
    CallArguments.reserve(Call.Arguments.size());
    for (std::size_t ArgumentIndex = 0; ArgumentIndex < Call.Arguments.size(); ++ArgumentIndex)
    {
      const ir::ValueHandle &Argument = Call.Arguments[ArgumentIndex];
      if (!Argument || &Argument->type() != Callee->ParameterTypes[ArgumentIndex])
      {
        addFailure<core::DiagnosticKind::ModuleFunctionReferenceInvalid>(CalleeModule->name(), CalleeFunction.value());
        return InstructionFlow::Failed;
      }
      RuntimeValueRef ArgumentValue = evaluateValue(*Argument, State.Module, State.Frame, State.FunctionValue.Name);
      if (ArgumentValue == nullptr || &ArgumentValue->type() != Callee->ParameterTypes[ArgumentIndex])
      {
        return InstructionFlow::Failed;
      }
      CallArguments.push_back(ArgumentValue);
    }

    RuntimeValueRef CallResult = nullptr;
    if (!executeFunction(*CalleeModule, CalleeFunction, CallArguments, State.Depth + 1, CallResult))
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
