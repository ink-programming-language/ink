#include "function_executor.h"

#include <cstddef>
#include <utility>
#include <vector>

namespace ink::execution
{
  FunctionExecutor::InstructionFlow FunctionExecutor::executeInsertValueInstruction(const ir::InsertValueInstruction &Insert, FunctionExecutionState &State)
  {
    RuntimeValueRef Aggregate = evaluateValue(*Insert.Aggregate, State.Module, State.Frame, State.FunctionValue.Name);
    RuntimeValueRef Element = evaluateValue(*Insert.Element, State.Module, State.Frame, State.FunctionValue.Name);
    if (Aggregate == nullptr || Element == nullptr)
    {
      return InstructionFlow::Failed;
    }
    Aggregate = importValue(Aggregate);
    Element = importValue(Element);
    if (Aggregate == nullptr || Element == nullptr)
    {
      addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>("insertvalue", State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }
    const ir::StructType &Struct = static_cast<const ir::StructType &>(*Insert.ResultType);
    std::vector<RuntimeValueRef> Fields;
    Fields.reserve(Struct.fieldTypes().size());
    for (std::size_t FieldIndex = 0; FieldIndex < Struct.fieldTypes().size(); ++FieldIndex)
    {
      RuntimeValueRef Field = Aggregate->field(FieldIndex);
      if (Field == nullptr)
      {
        addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>("insertvalue", State.FunctionValue.Name);
        return InstructionFlow::Failed;
      }
      Fields.push_back(FieldIndex == Insert.FieldIndex ? Element : Field);
    }
    RuntimeValueRef InsertResult = Values.aggregateValue(Struct, std::move(Fields));
    if (InsertResult == nullptr)
    {
      addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>("insertvalue", State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(Insert.Result, InsertResult))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("insertvalue", State.FunctionValue.Name, Insert.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeExtractValueInstruction(const ir::ExtractValueInstruction &Extract, FunctionExecutionState &State)
  {
    RuntimeValueRef Aggregate = evaluateValue(*Extract.Aggregate, State.Module, State.Frame, State.FunctionValue.Name);
    if (Aggregate == nullptr)
    {
      return InstructionFlow::Failed;
    }
    RuntimeValueRef Field = Aggregate->field(Extract.FieldIndex);
    if (Field == nullptr)
    {
      addFailure<core::DiagnosticKind::InvalidRuntimeAggregate>("extractvalue", State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(Extract.Result, Field))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("extractvalue", State.FunctionValue.Name, Extract.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }
} // namespace ink::execution
