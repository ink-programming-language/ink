#include "ink/execution/ffi/external_function.h"

#include "ink/execution/ffi/ffi_call.h"
#include "ink/execution/ffi/native_symbol_cache.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

namespace ink::execution
{
  ExecutionValueResult callExternalFunction(ExecutionHeap &Heap, NativeSymbolCache &Symbols, const ir::Function &Function, std::span<const ExecutionValueRef> Arguments)
  {
    ir::IRContext &Context = Heap.context();
    if (&Function.context() != &Context)
    {
      return {ExecutionStatus::ForeignContext};
    }
    if (!Context.compilationContext().targetContext().isNativeAbiCompatible())
    {
      return {ExecutionStatus::HostAbiMismatch};
    }
    if (!Function.isNativeImport() || Function.languageLinkage() != ir::LanguageLinkage::C || Function.callingConvention() != ir::CallingConvention::C)
    {
      return {ExecutionStatus::UnsupportedExternalSignature};
    }
    if (Function.parameters().size() != Arguments.size())
    {
      return {ExecutionStatus::InvalidArguments};
    }
    for (const ExecutionValueRef &Argument : Arguments)
    {
      if (!Argument.type())
      {
        return {ExecutionStatus::InvalidArguments};
      }
      if (&Argument.type()->context() != &Context)
      {
        return {ExecutionStatus::ForeignContext};
      }
    }
    NativeSymbol Symbol = Symbols.find(Context.namePool().text(Function.name()));
    if (!Symbol)
    {
      return {ExecutionStatus::SymbolNotFound};
    }
    return callWithLibffi(Heap, Symbol, Function, Arguments);
  }
} // namespace ink::execution
