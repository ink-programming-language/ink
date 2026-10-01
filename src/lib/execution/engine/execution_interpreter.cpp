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
      return {ExecutionStatus::Success, Heap.function(static_cast<const ir::Function &>(Value))};
    }
    const auto Found = Frame.Values.find(&Value);
    if (Found != Frame.Values.end())
    {
      return {ExecutionStatus::Success, Found->second};
    }
    // AST interpretation binds mutable parameter/variable identities to places.
    // IR instructions, in contrast, are read only after their ordered execution.
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
    if (Result.Status == ExecutionStatus::Cancelled || Result.Status == ExecutionStatus::BudgetExceeded)
    {
      StopStatus = Result.Status;
    }
    if (StopStatus != ExecutionStatus::Success)
    {
      Result = {StopStatus};
    }
    LastStatus = Result.Status;
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
        const ir::Value *ReturnedValue = static_cast<const ir::ReturnInstruction &>(*Instruction).returnedValue();
        return ReturnedValue ? evaluate(*ReturnedValue, Frame) : ExecutionValueResult{ExecutionStatus::Success, Heap.voidValue(Instruction->type())};
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

  ExecutionValueResult ExecutionEngine::executeCall(const ir::Value &Instruction, ExecutionFrame &Frame)
  {
    const auto &Call = static_cast<const ir::CallInstruction &>(Instruction);
    ExecutionValueResult Callee = evaluate(Call.callee(), Frame);
    if (!Callee)
    {
      return Callee;
    }
    if (Callee.Value.kind() != ExecutionValueKind::Function || !Callee.Value.valid())
    {
      return {ExecutionStatus::TypeMismatch};
    }
    std::vector<ExecutionValueRef> Arguments;
    Arguments.reserve(Call.arguments().size());
    for (const ir::Value *Argument : Call.arguments())
    {
      ExecutionValueResult Result = evaluate(*Argument, Frame);
      if (!Result)
      {
        return Result;
      }
      Arguments.push_back(std::move(Result.Value));
    }
    return execute(*Callee.Value.function(), Arguments);
  }

  ExecutionValueResult ExecutionEngine::executeInstruction(const ir::Value &Instruction, ExecutionFrame &Frame)
  {
    switch (Instruction.kind())
    {
    case ir::ValueKind::Function:
      return {ExecutionStatus::Success, Heap.function(static_cast<const ir::Function &>(Instruction))};
    case ir::ValueKind::AllocaInstruction:
    {
      const auto &Alloca = static_cast<const ir::AllocaInstruction &>(Instruction);
      const ExecutionPlaceResult Result = allocateValue(Frame, &Alloca, Alloca.allocatedType());
      return Result ? ExecutionValueResult{ExecutionStatus::Success, Heap.pointer(Alloca.type(), ExecutionPointer::fromPlace(Result.Place))} : ExecutionValueResult{Result.Status};
    }
    case ir::ValueKind::LoadInstruction:
    {
      const auto &Load = static_cast<const ir::LoadInstruction &>(Instruction);
      ExecutionValueResult Address = evaluate(Load.address(), Frame);
      return Address ? loadPointer(Address.Value) : Address;
    }
    case ir::ValueKind::StoreInstruction:
    {
      const auto &Store = static_cast<const ir::StoreInstruction &>(Instruction);
      ExecutionValueResult Address = evaluate(Store.address(), Frame);
      if (!Address)
      {
        return Address;
      }
      ExecutionValueResult Value = evaluate(Store.storedValue(), Frame);
      if (!Value)
      {
        return Value;
      }
      const ExecutionStatus Status = storePointer(Address.Value, Value.Value);
      return Status == ExecutionStatus::Success ? ExecutionValueResult{Status, Heap.voidValue(Store.type())} : ExecutionValueResult{Status};
    }
    case ir::ValueKind::AddInstruction:
    {
      const auto &Add = static_cast<const ir::AddInstruction &>(Instruction);
      ExecutionValueResult Left = evaluate(Add.left(), Frame);
      if (!Left)
      {
        return Left;
      }
      ExecutionValueResult Right = evaluate(Add.right(), Frame);
      if (!Right)
      {
        return Right;
      }
      if (Left.Value.kind() != ExecutionValueKind::Integer || Right.Value.kind() != ExecutionValueKind::Integer || Left.Value.type() != Right.Value.type())
      {
        return {ExecutionStatus::TypeMismatch};
      }
      ExecutionInteger Sum(Left.Value.integer().bitWidth());
      const ExecutionStatus Status = Left.Value.integer().add(Right.Value.integer(), Sum);
      return Status == ExecutionStatus::Success ? ExecutionValueResult{Status, Heap.integer(Add.type(), std::move(Sum))} : ExecutionValueResult{Status};
    }
    case ir::ValueKind::CStringInstruction:
    {
      const auto &CString = static_cast<const ir::CStringInstruction &>(Instruction);
      const ExecutionStorageRef Buffer = Heap.allocateBuffer(CString.source().value());
      if (!Buffer.valid())
      {
        if (Heap.lastStatus() == ExecutionStatus::BudgetExceeded)
        {
          StopStatus = Heap.lastStatus();
        }
        return {Heap.lastStatus()};
      }
      Frame.Storage.push_back(Buffer);
      return {ExecutionStatus::Success, Heap.pointer(CString.type(), ExecutionPointer::fromBuffer(Buffer))};
    }
    case ir::ValueKind::CallInstruction:
      return executeCall(Instruction, Frame);
    default:
      return {ExecutionStatus::UnsupportedOperation};
    }
  }
} // namespace ink::execution
