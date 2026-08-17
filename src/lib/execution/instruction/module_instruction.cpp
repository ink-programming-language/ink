#include "engine/function_executor.h"

#include "ink/execution/module/module_loader.h"
#include "ink/ir/model/operand.h"

namespace ink::execution
{
  bool FunctionExecutor::validateGlobalVariableAccessType(const ir::Value &Pointer, ModuleInstance &Module, const ir::Type &AccessType)
  {
    if (Pointer.kind() != ir::ValueKind::GlobalVariableAddressOperand)
    {
      return true;
    }
    const ir::GlobalId Reference = static_cast<const ir::GlobalVariableAddressOperand &>(Pointer).global();
    const ir::Module &Definition = Module.definition();
    if (!Reference.valid() || Reference.value() >= Definition.Globals.size() || Definition.Globals[Reference.value()].ValueType != &AccessType)
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Module.name(), Reference.valid() ? Reference.value() : ir::InvalidId);
      return false;
    }
    return true;
  }

  RuntimeValueRef FunctionExecutor::evaluateGlobalVariableAddress(const ir::GlobalVariableAddressOperand &Address, ModuleInstance &Module, const ir::Name &)
  {
    const ir::GlobalId Reference = Address.global();
    const ir::Module &Definition = Module.definition();
    if (!Reference.valid() || Reference.value() >= Definition.Globals.size())
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Module.name(), Reference.valid() ? Reference.value() : ir::InvalidId);
      return nullptr;
    }

    const ir::GlobalVariable &DeclaredGlobal = Definition.Globals[Reference.value()];
    ModuleInstance *Target = &Module;
    ir::GlobalId TargetGlobal = Reference;
    const ir::GlobalVariable *Global = &DeclaredGlobal;
    for (std::size_t ImportDepth = 0; Global->Kind == ir::GlobalVariableKind::Imported; ++ImportDepth)
    {
      if (ImportDepth >= MaximumModuleImportDepth)
      {
        addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->name(), TargetGlobal.value());
        return nullptr;
      }
      if (!Runtime.resolveImportedGlobal(*Target, TargetGlobal, Target, TargetGlobal, Diagnostics))
      {
        return nullptr;
      }
      const ir::Module &TargetDefinition = Target->definition();
      if (!TargetGlobal.valid() || TargetGlobal.value() >= TargetDefinition.Globals.size())
      {
        addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->name(), TargetGlobal.valid() ? TargetGlobal.value() : ir::InvalidId);
        return nullptr;
      }
      Global = &TargetDefinition.Globals[TargetGlobal.value()];
    }
    if (Global->Kind != ir::GlobalVariableKind::Definition || Global->ValueType != DeclaredGlobal.ValueType)
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->name(), TargetGlobal.value());
      return nullptr;
    }

    RuntimeValueRef StoredAddress = nullptr;
    if (Address.type().kind() == ir::TypeKind::ConstBytePointer)
    {
      StoredAddress = Target->globalAddress(TargetGlobal);
    }
    else if (Address.type().kind() == ir::TypeKind::BytePointer && ((DeclaredGlobal.Mutable && Global->Mutable) || (DeclaredGlobal.Kind == ir::GlobalVariableKind::Definition && Target == &Module && Module.state() == ModuleState::Initializing)))
    {
      StoredAddress = Target->mutableGlobalAddress(TargetGlobal);
    }
    if (StoredAddress == nullptr || &StoredAddress->type() != &Address.type())
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->name(), TargetGlobal.value());
      return nullptr;
    }
    RuntimeValueRef Result = importValue(StoredAddress);
    if (Result == nullptr)
    {
      addFailure<core::DiagnosticKind::ModuleGlobalReferenceInvalid>(Target->name(), TargetGlobal.value());
    }
    return Result;
  }

  FunctionExecutor::InstructionFlow FunctionExecutor::executeImportInstruction(const ir::ImportInstruction &Import, FunctionExecutionState &State)
  {
    return Runtime.importModule(State.Module, Import.Module, Diagnostics) ? InstructionFlow::Continue : InstructionFlow::Failed;
  }
} // namespace ink::execution
