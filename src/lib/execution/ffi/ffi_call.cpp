#include "ink/execution/ffi/ffi_call.h"

#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/execution/ffi/native_call_cache.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

#include <vector>

namespace ink::execution
{
  namespace
  {
    template <typename Invoke>
    ExecutionValueResult invokeSemanticCall(ExecutionHeap &Heap, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments, Invoke &&Call)
    {
      if (&Function.context() != &Heap.context())
      {
        return {ExecutionStatus::ForeignContext};
      }
      SemanticValueBridge &Bridge = Heap.bridge();
      const FunctionId Id = Bridge.lowerFunction(Function);
      const RuntimeFunctionDescriptor *Descriptor = Bridge.functionDescriptor(Id);
      if (!Descriptor)
      {
        return {ExecutionStatus::UnsupportedExternalSignature};
      }
      if (!Descriptor->NativeAbi)
      {
        return {ExecutionStatus::HostAbiMismatch};
      }
      if (!Descriptor->Supported)
      {
        return {ExecutionStatus::UnsupportedExternalSignature};
      }
      const TypeDesc *Signature = Bridge.types()->get(Descriptor->Signature);
      if (!Signature || Signature->functionDesc().Parameters.size() != Arguments.size())
      {
        return {ExecutionStatus::InvalidArguments};
      }
      std::vector<RuntimeValue> Values;
      Values.reserve(Arguments.size());
      for (const ExecutionValueRef &Argument : Arguments)
      {
        if (!Argument.type())
        {
          return {ExecutionStatus::InvalidArguments};
        }
        RuntimeValueResult Converted = Bridge.lowerValue(Argument);
        if (!Converted)
        {
          return {Converted.Status};
        }
        Values.push_back(std::move(Converted.Value));
      }
      const RuntimeTypeId ReturnType = Signature->functionDesc().ReturnType;
      const RuntimeValueResult Result = Call(Heap.memoryManager(), *Bridge.types(), *Descriptor, Values);
      return Result ? Bridge.raiseValue(Heap, Result.Value, ReturnType) : ExecutionValueResult{Result.Status};
    }
  } // namespace

  ExecutionValueResult callWithLibffi(ExecutionHeap &Heap, NativeSymbol Symbol, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments)
  {
    return invokeSemanticCall(Heap, Function, Arguments, [&](ExecutionMemoryManager &Memory, const RuntimeTypeTable &Types, const RuntimeFunctionDescriptor &Descriptor, std::span<const RuntimeValue> Values)
    {
      return callWithLibffi(Memory, Types, Symbol, Descriptor, Values);
    });
  }

  ExecutionValueResult NativeCallCache::invoke(ExecutionHeap &Heap, NativeSymbolCache &Symbols, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments)
  {
    return invokeSemanticCall(Heap, Function, Arguments, [&](ExecutionMemoryManager &Memory, const RuntimeTypeTable &Types, const RuntimeFunctionDescriptor &Descriptor, std::span<const RuntimeValue> Values)
    {
      return invoke(Memory, Types, Symbols, Descriptor, Values);
    });
  }
} // namespace ink::execution
