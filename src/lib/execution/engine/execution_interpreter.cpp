#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/ffi/external_function.h"

#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/c_string_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"

#include <utility>

namespace ink::execution
{
  ExecutionValueResult ExecutionEngine::evaluate(const ir::Value &Value, ExecutionFrame &Frame)
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
    const auto Found = Frame.Values.find(&Value);
    if (Found != Frame.Values.end())
    {
      return {ExecutionStatus::Success, Found->second};
    }
    // Semantic evaluation binds compile-time variables to places. IR instruction
    // results are available only after their ordered execution in this frame.
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
    ExecutionValueResult Result = executeInvocation(Function, Arguments);
    leaveEvaluation();
    Result.Status = finishStatus(Result.Status);
    if (!Result)
    {
      Result.Value = {};
    }
    return Result;
  }

  ExecutionValueResult ExecutionEngine::executeInvocation(const ir::Function &Function, std::span<const ExecutionValueRef> Arguments)
  {
    if (&Function.context() != &Context)
    {
      return {ExecutionStatus::ForeignContext};
    }
    const bool External = Function.languageLinkage() == ir::LanguageLinkage::C;
    const auto &Parameters = Function.parameters();
    if (Parameters.size() != Arguments.size())
    {
      return {ExecutionStatus::InvalidArguments};
    }
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
      if (!External && Arguments[Index].type() != &Parameters[Index]->type())
      {
        return {ExecutionStatus::TypeMismatch};
      }
    }
    if (!External && !Function.hasBody())
    {
      return {ExecutionStatus::MissingBody};
    }
    ExecutionFrame *Frame = createFrame(ExecutionFrameKind::Call);
    if (!Frame)
    {
      return {LastStatus};
    }
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      Frame->Values.emplace(Parameters[Index].get(), Arguments[Index]);
    }
    ExecutionValueResult Result = External ? callExternalFunction(Heap, NativeSymbols, Function, Arguments) : executeBody(Function, *Frame);
    if (Result && Result.Value.type() != &Function.functionType().returnType())
    {
      Result = {ExecutionStatus::TypeMismatch};
    }
    if (Result)
    {
      const ExecutionStatus Status = validateValue(Result.Value);
      if (Status != ExecutionStatus::Success)
      {
        Result = {Status};
      }
    }
    // Keep pointer provenance in returned values, but end function-owned storage.
    // A later access to an escaped alloca/CString reports ExpiredPlace.
    endFrame(*Frame);
    return Result;
  }

  ExecutionValueResult ExecutionEngine::executeBody(const ir::Function &Function, ExecutionFrame &Frame)
  {
    const ir::BasicBlock *Block = Function.entryBlock();
    for (const auto &Instruction : Block->values())
    {
      if (consumeStep() != ExecutionStatus::Success)
      {
        return {LastStatus};
      }
      if (ir::ReturnInstruction::classof(Instruction.get()))
      {
        return executeReturn(static_cast<const ir::ReturnInstruction &>(*Instruction), Frame);
      }
      ExecutionValueResult Result = executeInstruction(*Instruction, Frame);
      if (!Result)
      {
        return Result;
      }
      Frame.Values.emplace(Instruction.get(), std::move(Result.Value));
    }
    // Block insertion order is not control flow. Never fall through to another block.
    return {ExecutionStatus::MissingBody};
  }

  ExecutionValueResult ExecutionEngine::executeInstruction(const ir::Value &Instruction, ExecutionFrame &Frame)
  {
    switch (Instruction.kind())
    {
    case ir::ValueKind::Function:
      return makeFunctionValue(static_cast<const ir::Function &>(Instruction));
    case ir::ValueKind::AllocaInstruction:
      return executeAlloca(static_cast<const ir::AllocaInstruction &>(Instruction), Frame);
    case ir::ValueKind::LoadInstruction:
      return executeLoad(static_cast<const ir::LoadInstruction &>(Instruction), Frame);
    case ir::ValueKind::StoreInstruction:
      return executeStore(static_cast<const ir::StoreInstruction &>(Instruction), Frame);
    case ir::ValueKind::AddInstruction:
      return executeAdd(static_cast<const ir::AddInstruction &>(Instruction), Frame);
    case ir::ValueKind::CStringInstruction:
      return executeCString(static_cast<const ir::CStringInstruction &>(Instruction), Frame);
    case ir::ValueKind::CallInstruction:
      return executeCall(static_cast<const ir::CallInstruction &>(Instruction), Frame);
    default:
      return {ExecutionStatus::UnsupportedOperation};
    }
  }
} // namespace ink::execution
