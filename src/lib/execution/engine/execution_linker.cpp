#include "ink/execution/engine/execution_linker.h"

#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/execution/bytecode/execution_compiler.h"

namespace ink::execution
{
  ExecutionLinker::ExecutionLinker(SemanticValueBridge &Bridge)
      : Bridge(&Bridge)
  {
    Image.Layouts = Bridge.types();
  }

  ExecutionLinker::ExecutionLinker(ExecutionImage Image)
      : Image(std::move(Image))
  {
  }

  const RuntimeTypeTable *ExecutionLinker::layouts() const noexcept
  {
    return Image.Layouts.get();
  }

  bool ExecutionLinker::synchronize(std::uint64_t NewRevision)
  {
    if (!Bridge || Revision == NewRevision)
    {
      return false;
    }
    Image.Functions.clear();
    Image.Descriptors.clear();
    Revision = NewRevision;
    return true;
  }

  FunctionId ExecutionLinker::resolveFunction(FunctionId Function)
  {
    const auto *Source = Bridge ? Bridge->sourceFunction(Function) : nullptr;
    return Source ? Bridge->lowerFunction(*Source) : Function;
  }

  const RuntimeFunctionDescriptor *ExecutionLinker::descriptor(FunctionId Function)
  {
    const auto Found = Image.Descriptors.find(Function);
    if (Found != Image.Descriptors.end())
    {
      return Found->second.Id == Function ? &Found->second : nullptr;
    }
    if (!Bridge)
    {
      return nullptr;
    }
    const auto *Source = Bridge->sourceFunction(Function);
    if (Source)
    {
      Bridge->lowerFunction(*Source);
    }
    const RuntimeFunctionDescriptor *Descriptor = Bridge->functionDescriptor(Function);
    return Descriptor ? &Image.Descriptors.emplace(Function, *Descriptor).first->second : nullptr;
  }

  const ExecutableFunction *ExecutionLinker::prepare(FunctionId Function, ExecutionStatus &Status)
  {
    const auto Found = Image.Functions.find(Function);
    if (Found != Image.Functions.end())
    {
      Status = Found->second && Found->second->Id == Function ? validate(*Found->second) : ExecutionStatus::InvalidArguments;
      return Status == ExecutionStatus::Success ? Found->second.get() : nullptr;
    }
    const auto *Source = Bridge ? Bridge->sourceFunction(Function) : nullptr;
    if (!Source)
    {
      Status = ExecutionStatus::MissingBody;
      return nullptr;
    }
    // Registration precedes lowering, allowing recursion without recursively
    // compiling the call graph. Missing bodies fail only when actually invoked.
    ExecutionCompilationResult Compiled = ExecutionCompiler{}.compile(*Source, *Bridge);
    if (!Compiled)
    {
      Status = Compiled.Status;
      return nullptr;
    }
    Status = validate(*Compiled.Function);
    if (Status != ExecutionStatus::Success)
    {
      return nullptr;
    }
    return Image.Functions.emplace(Function, std::move(Compiled.Function)).first->second.get();
  }

  ExecutionStatus ExecutionLinker::validate(const ExecutableFunction &Function)
  {
    if (!Image.Layouts || Function.Layouts != Image.Layouts)
    {
      return ExecutionStatus::ForeignContext;
    }
    const auto *OwnDescriptor = descriptor(Function.Id);
    if (!OwnDescriptor || OwnDescriptor->Signature != Function.Signature)
    {
      return ExecutionStatus::TypeMismatch;
    }
    const ExecutionStatus Status = ExecutionCompiler{}.verify(Function);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    for (const RuntimeValue &Value : Function.InitialSlots)
    {
      if (!Value.Initialized || Image.Layouts->get(Value.Type)->Kind != RuntimeKind::Function)
      {
        continue;
      }
      const auto *Target = descriptor(static_cast<FunctionId>(Value.Bits));
      if (!Target || Target->Signature != Value.Type)
      {
        return ExecutionStatus::TypeMismatch;
      }
    }
    for (const ExecutionCallSite &Call : Function.Calls)
    {
      if (Call.Target == InvalidFunction)
      {
        continue;
      }
      const auto *Target = descriptor(Call.Target);
      if (!Target || Target->Signature != Call.Signature)
      {
        return ExecutionStatus::TypeMismatch;
      }
    }
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
