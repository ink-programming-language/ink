#include "function_executor.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    constexpr std::size_t MaximumCallDepth = 256;
  } // namespace

  FunctionExecutor::FunctionExecutor(ExecutionContext &Context, const ir::Module &ModuleValue, ExternalFunctionInvoker &ExternalInvoker, std::vector<core::Diagnostic> &Diagnostics) : Context(Context), ModuleValue(ModuleValue), ExternalInvoker(ExternalInvoker), Diagnostics(Diagnostics)
  {
  }

  bool FunctionExecutor::execute(std::size_t FunctionIndex, const std::vector<RuntimeValueRef> &Arguments, RuntimeValueRef &Result)
  {
    return executeFunction(FunctionIndex, Arguments, 0, Result);
  }

  RuntimeValueRef FunctionExecutor::importValue(RuntimeValueRef Value)
  {
    if (Value == nullptr || Values.owns(Value))
    {
      return Value;
    }
    const auto Existing = ImportedValues.find(Value);
    if (Existing != ImportedValues.end())
    {
      return Existing->second;
    }
    RuntimeValueRef Imported = Values.clone(*Value);
    if (Imported != nullptr)
    {
      ImportedValues.emplace(Value, Imported);
    }
    return Imported;
  }

  RuntimeValueRef FunctionExecutor::zeroValue(const ir::Type &TypeValue)
  {
    switch (TypeValue.kind())
    {
    case ir::TypeKind::Void:
      return Values.voidValue(TypeValue);
    case ir::TypeKind::Bool:
    case ir::TypeKind::Byte:
    case ir::TypeKind::I32:
    case ir::TypeKind::PointerSize:
      return Values.integerValue(TypeValue, 0);
    case ir::TypeKind::BytePointer:
      return Values.mutablePointerValue(TypeValue, nullptr);
    case ir::TypeKind::ConstBytePointer:
      return Values.pointerValue(TypeValue, nullptr);
    case ir::TypeKind::Struct:
    {
      const ir::StructType &Struct = static_cast<const ir::StructType &>(TypeValue);
      std::vector<RuntimeValueRef> Fields;
      Fields.reserve(Struct.fieldTypes().size());
      for (const ir::Type *FieldType : Struct.fieldTypes())
      {
        RuntimeValueRef Field = zeroValue(*FieldType);
        if (Field == nullptr)
        {
          return nullptr;
        }
        Fields.push_back(Field);
      }
      return Values.aggregateValue(Struct, std::move(Fields));
    }
    case ir::TypeKind::Count:
      return nullptr;
    }
    return nullptr;
  }

  RuntimeValueRef FunctionExecutor::evaluateValue(const ir::Value &Value, const ExecutionFrame &Frame)
  {
    if (Value.kind() == ir::ValueKind::IntegerConstant)
    {
      const std::int64_t Integer = static_cast<const ir::IntegerConstant &>(Value).value();
      return Values.integerValue(Value.type(), static_cast<std::uint64_t>(Integer));
    }
    if (Value.kind() == ir::ValueKind::ValueOperand)
    {
      const ir::ValueId Id = static_cast<const ir::ValueOperand &>(Value).id();
      RuntimeValueRef Stored = Frame.find(Id);
      if (Stored == nullptr)
      {
        addFailure<core::DiagnosticKind::SsaValueUnavailableDuringExecution>(Id.value());
        return nullptr;
      }
      return Stored;
    }
    if (Value.kind() == ir::ValueKind::GlobalAddressOperand)
    {
      const ir::GlobalAddressOperand &Address = static_cast<const ir::GlobalAddressOperand &>(Value);
      const std::string &Data = ModuleValue.ByteConstants[Address.global().value()].Data;
      return Values.pointerValue(Value.type(), Data.data() + Address.byteOffset());
    }
    if (Value.kind() == ir::ValueKind::ZeroInitializer)
    {
      RuntimeValueRef Result = zeroValue(Value.type());
      if (Result == nullptr)
      {
        addFailure<core::DiagnosticKind::ZeroInitializerConstructionFailed>(ir::typeKindName(Value.type().kind()));
      }
      return Result;
    }
    addFailure<core::DiagnosticKind::UnsupportedRuntimeValueKind>();
    return nullptr;
  }

  bool FunctionExecutor::executeFunction(std::size_t FunctionIndex, const std::vector<RuntimeValueRef> &Arguments, std::size_t Depth, RuntimeValueRef &Result)
  {
    const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
    if (Depth >= MaximumCallDepth)
    {
      addFailure<core::DiagnosticKind::CallDepthLimitExceeded>(FunctionValue.Name, MaximumCallDepth);
      return false;
    }
    if (FunctionValue.Kind == ir::FunctionKind::External)
    {
      return ExternalInvoker.invokeExternal(FunctionIndex, Arguments, Values, Result, Diagnostics);
    }
    if (FunctionValue.Blocks.size() != 1)
    {
      addFailure<core::DiagnosticKind::MultipleBasicBlocksUnsupported>(FunctionValue.Name, FunctionValue.Blocks.size());
      return false;
    }

    FunctionExecutionState State(FunctionValue, Arguments, Depth);
    for (const std::unique_ptr<ir::Instruction> &InstructionPointer : FunctionValue.Blocks[0].Instructions)
    {
      const InstructionFlow Flow = executeInstruction(*InstructionPointer, State);
      if (Flow == InstructionFlow::Failed)
      {
        return false;
      }
      if (Flow == InstructionFlow::Return)
      {
        Result = State.ReturnValue;
        return true;
      }
    }

    addFailure<core::DiagnosticKind::FunctionMissingReturnDuringExecution>(FunctionValue.Name);
    return false;
  }
} // namespace ink::execution
