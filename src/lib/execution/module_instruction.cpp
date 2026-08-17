#include "function_executor.h"

namespace ink::execution
{
  bool FunctionExecutor::validateGlobalVariableAccessType(const ir::Value &Pointer, ModuleInstance &Module, const ir::Type &AccessType)
  {
    if (Pointer.kind() != ir::ValueKind::GlobalVariableAddressOperand)
    {
      return true;
    }
    const ir::GlobalRef Reference = static_cast<const ir::GlobalVariableAddressOperand &>(Pointer).global();
    ModuleInstance *Target = Runtime.resolveReferencedModule(Module, Reference.Module, Diagnostics);
    if (Target == nullptr)
    {
      return false;
    }
    const ir::Module &Definition = Target->definition();
    if (!Reference.Global.valid() || Reference.Global.value() >= Definition.Globals.size() || Definition.Globals[Reference.Global.value()].ValueType != &AccessType)
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->id().value(), Reference.Global.valid() ? Reference.Global.value() : ir::InvalidId);
      return false;
    }
    return true;
  }

  RuntimeValueRef FunctionExecutor::evaluateGlobalVariableAddress(const ir::GlobalVariableAddressOperand &Address, ModuleInstance &Module, const std::string &)
  {
    const ir::GlobalRef Reference = Address.global();
    ModuleInstance *Target = Runtime.resolveReferencedModule(Module, Reference.Module, Diagnostics);
    if (Target == nullptr)
    {
      return nullptr;
    }
    const ir::Module &Definition = Target->definition();
    if (!Reference.Global.valid() || Reference.Global.value() >= Definition.Globals.size())
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->id().value(), Reference.Global.valid() ? Reference.Global.value() : ir::InvalidId);
      return nullptr;
    }

    const ir::GlobalVariable &Global = Definition.Globals[Reference.Global.value()];
    RuntimeValueRef StoredAddress = nullptr;
    if (Address.type().kind() == ir::TypeKind::ConstBytePointer)
    {
      StoredAddress = Target->globalAddress(Reference.Global);
    }
    else if (Address.type().kind() == ir::TypeKind::BytePointer && (Global.Mutable || (Target == &Module && Module.state() == ModuleState::Initializing)))
    {
      StoredAddress = Target->mutableGlobalAddress(Reference.Global);
    }
    if (StoredAddress == nullptr || &StoredAddress->type() != &Address.type())
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->id().value(), Reference.Global.value());
      return nullptr;
    }
    RuntimeValueRef Result = importValue(StoredAddress);
    if (Result == nullptr)
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->id().value(), Reference.Global.value());
    }
    return Result;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeImportInstruction(const ir::ImportInstruction &Import, FunctionExecutionState &State)
  {
    return Runtime.importModule(State.Module, Import.Module, Diagnostics) ? InstructionFlow::Continue : InstructionFlow::Failed;
  }
} // namespace ink::execution
