#include "ink/execution/execution_engine.h"

#include "ink/ir/context.h"
#include "ink/ir/verifier.h"
#include "runtime.h"

#include <ffi.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    constexpr std::size_t MaximumCallDepth = 1024;

    ffi_type *nativeType(ir::TypeKind Type)
    {
      switch (Type)
      {
      case ir::TypeKind::Void:
        return &ffi_type_void;
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
        return &ffi_type_uint8;
      case ir::TypeKind::I32:
        return &ffi_type_sint32;
      case ir::TypeKind::PointerSize:
        return sizeof(std::size_t) == sizeof(std::uint64_t) ? &ffi_type_uint64 : &ffi_type_uint32;
      case ir::TypeKind::ConstBytePointer:
        return &ffi_type_pointer;
      }
      return nullptr;
    }

    struct NativeStorage
    {
      std::uint8_t Byte = 0;
      std::int32_t I32 = 0;
      std::size_t PointerSize = 0;
      const void *Pointer = nullptr;
    };

    void *storeNativeValue(const RuntimeValue &Value, NativeStorage &Storage)
    {
      const std::optional<std::uint64_t> Integer = Value.integer();
      switch (Value.type())
      {
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
        if (!Integer.has_value())
        {
          return nullptr;
        }
        Storage.Byte = static_cast<std::uint8_t>(*Integer);
        return &Storage.Byte;
      case ir::TypeKind::I32:
        if (!Integer.has_value())
        {
          return nullptr;
        }
        Storage.I32 = static_cast<std::int32_t>(*Integer);
        return &Storage.I32;
      case ir::TypeKind::PointerSize:
        if (!Integer.has_value())
        {
          return nullptr;
        }
        Storage.PointerSize = static_cast<std::size_t>(*Integer);
        return &Storage.PointerSize;
      case ir::TypeKind::ConstBytePointer:
        Storage.Pointer = Value.pointer();
        return &Storage.Pointer;
      case ir::TypeKind::Void:
        return nullptr;
      }
      return nullptr;
    }

    RuntimeValue loadNativeValue(ir::TypeKind Type, const NativeStorage &Storage)
    {
      switch (Type)
      {
      case ir::TypeKind::Void:
        return RuntimeValue::voidValue();
      case ir::TypeKind::Bool:
      case ir::TypeKind::Byte:
        return RuntimeValue::integerValue(Type, Storage.Byte);
      case ir::TypeKind::I32:
        return RuntimeValue::integerValue(Type, static_cast<std::uint64_t>(static_cast<std::int64_t>(Storage.I32)));
      case ir::TypeKind::PointerSize:
        return RuntimeValue::integerValue(Type, Storage.PointerSize);
      case ir::TypeKind::ConstBytePointer:
        return RuntimeValue::pointerValue(Type, Storage.Pointer);
      }
      return RuntimeValue::voidValue();
    }
  } // namespace

  class ExecutionEngine::Impl
  {
  public:
    Impl(ExecutionContext &Context, const ir::Module &ModuleValue) : Context(Context), ModuleValue(ModuleValue), PreparedFunctions(ModuleValue.Functions.size())
    {
    }

    InitializationResult initialize()
    {
      InitializationResult Result;
      if (Initialized)
      {
        Result.Succeeded = true;
        return Result;
      }

      ir::IRContext IRContext(Context.compilationContext());
      ir::VerificationResult Verification = ir::verify(IRContext, ModuleValue);
      if (!Verification.succeeded())
      {
        Result.Diagnostics = Verification.diagnostics();
        return Result;
      }
      if (!registerRuntimeSymbols(Symbols))
      {
        addFailure(Result.Diagnostics, "could not register the native Ink runtime symbols");
        return Result;
      }

      for (std::size_t FunctionIndex = 0; FunctionIndex < ModuleValue.Functions.size(); ++FunctionIndex)
      {
        const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
        if (FunctionValue.Kind != ir::FunctionKind::External)
        {
          continue;
        }
        if (!Symbols.resolveAndRegister(FunctionValue.Name))
        {
          addFailure(Result.Diagnostics, "could not resolve external function @" + FunctionValue.Name);
          continue;
        }

        auto Prepared = std::make_unique<PreparedFunction>();
        Prepared->Address = Symbols.findAddress(FunctionValue.Name);
        Prepared->ArgumentTypes.reserve(FunctionValue.ParameterTypes.size());
        for (const ir::TypeKind ParameterType : FunctionValue.ParameterTypes)
        {
          Prepared->ArgumentTypes.push_back(nativeType(ParameterType));
        }
        ffi_type *ResultType = nativeType(FunctionValue.ResultType);
        if (ResultType == nullptr || ffi_prep_cif(&Prepared->Interface, FFI_DEFAULT_ABI, static_cast<unsigned int>(Prepared->ArgumentTypes.size()), ResultType, Prepared->ArgumentTypes.data()) != FFI_OK)
        {
          addFailure(Result.Diagnostics, "libffi could not prepare the signature of external function @" + FunctionValue.Name);
          continue;
        }
        PreparedFunctions[FunctionIndex] = std::move(Prepared);
      }

      if (!Result.Diagnostics.empty())
      {
        PreparedFunctions.clear();
        PreparedFunctions.resize(ModuleValue.Functions.size());
        return Result;
      }
      Initialized = true;
      Result.Succeeded = true;
      return Result;
    }

    ExecutionResult execute(std::string_view EntryName, const std::vector<RuntimeValue> &Arguments)
    {
      ExecutionResult Result;
      InitializationResult Initialization = initialize();
      if (!Initialization.succeeded())
      {
        Result.Diagnostics = Initialization.diagnostics();
        return Result;
      }

      std::size_t EntryIndex = ModuleValue.Functions.size();
      for (std::size_t FunctionIndex = 0; FunctionIndex < ModuleValue.Functions.size(); ++FunctionIndex)
      {
        if (ModuleValue.Functions[FunctionIndex].Name == EntryName)
        {
          EntryIndex = FunctionIndex;
          break;
        }
      }
      if (EntryIndex == ModuleValue.Functions.size())
      {
        addFailure(Result.Diagnostics, "entry function @" + std::string(EntryName) + " does not exist");
        return Result;
      }
      const ir::Function &Entry = ModuleValue.Functions[EntryIndex];
      if (Entry.Kind != ir::FunctionKind::Definition)
      {
        addFailure(Result.Diagnostics, "entry function @" + Entry.Name + " must be defined in InkIR");
        return Result;
      }
      if (Entry.ParameterTypes.size() != Arguments.size())
      {
        addFailure(Result.Diagnostics, "entry function @" + Entry.Name + " received the wrong number of arguments");
        return Result;
      }
      for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
      {
        if (Arguments[ArgumentIndex].type() != Entry.ParameterTypes[ArgumentIndex])
        {
          addFailure(Result.Diagnostics, "argument " + std::to_string(ArgumentIndex) + " of entry function @" + Entry.Name + " has the wrong type");
          return Result;
        }
      }

      RuntimeValue ReturnValue = RuntimeValue::voidValue();
      if (executeFunction(EntryIndex, Arguments, 0, ReturnValue, Result.Diagnostics))
      {
        Result.ReturnValue = ReturnValue;
      }
      return Result;
    }

    NativeSymbolRegistry Symbols;

  private:
    struct PreparedFunction
    {
      ffi_cif Interface{};
      std::vector<ffi_type *> ArgumentTypes;
      NativeFunctionAddress Address = nullptr;
    };

    void addFailure(std::vector<core::Diagnostic> &Diagnostics, std::string Message)
    {
      core::Diagnostic DiagnosticEntry = core::DiagnosticBuilder(core::DiagnosticKind::ExecutionFailed, {}).argument(core::DiagnosticArgumentName::Detail, std::move(Message)).build();
      Context.diagnosticEngine().report(DiagnosticEntry);
      Diagnostics.push_back(std::move(DiagnosticEntry));
    }

    bool evaluateValue(const ir::Value &Value, const std::unordered_map<std::size_t, RuntimeValue> &Frame, RuntimeValue &Result, std::vector<core::Diagnostic> &Diagnostics)
    {
      if (Value.kind() == ir::ValueKind::IntegerConstant)
      {
        const std::int64_t Integer = static_cast<const ir::IntegerConstant &>(Value).value();
        Result = RuntimeValue::integerValue(Value.type(), static_cast<std::uint64_t>(Integer));
        return true;
      }
      if (Value.kind() == ir::ValueKind::ValueOperand)
      {
        const ir::ValueId Id = static_cast<const ir::ValueOperand &>(Value).id();
        const auto Stored = Frame.find(Id.value());
        if (Stored == Frame.end())
        {
          addFailure(Diagnostics, "SSA value %" + std::to_string(Id.value()) + " is unavailable in the current execution frame");
          return false;
        }
        Result = Stored->second;
        return true;
      }
      if (Value.kind() == ir::ValueKind::GlobalAddressOperand)
      {
        const ir::GlobalAddressOperand &Address = static_cast<const ir::GlobalAddressOperand &>(Value);
        const std::string &Data = ModuleValue.ByteConstants[Address.global().value()].Data;
        Result = RuntimeValue::pointerValue(Value.type(), Data.data() + Address.byteOffset());
        return true;
      }
      addFailure(Diagnostics, "encountered an unsupported runtime value kind");
      return false;
    }

    bool callExternal(std::size_t FunctionIndex, const std::vector<RuntimeValue> &Arguments, RuntimeValue &Result, std::vector<core::Diagnostic> &Diagnostics)
    {
      const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
      const std::unique_ptr<PreparedFunction> &Prepared = PreparedFunctions[FunctionIndex];
      if (!Prepared)
      {
        addFailure(Diagnostics, "external function @" + FunctionValue.Name + " was not prepared during initialization");
        return false;
      }

      std::vector<NativeStorage> ArgumentStorage(Arguments.size());
      std::vector<void *> NativeArguments(Arguments.size());
      for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
      {
        NativeArguments[ArgumentIndex] = storeNativeValue(Arguments[ArgumentIndex], ArgumentStorage[ArgumentIndex]);
        if (NativeArguments[ArgumentIndex] == nullptr)
        {
          addFailure(Diagnostics, "could not marshal argument " + std::to_string(ArgumentIndex) + " of external function @" + FunctionValue.Name);
          return false;
        }
      }

      NativeStorage ReturnStorage;
      void *ReturnAddress = FunctionValue.ResultType == ir::TypeKind::Void ? nullptr : storeNativeValue(RuntimeValue::integerValue(FunctionValue.ResultType, 0), ReturnStorage);
      if (FunctionValue.ResultType == ir::TypeKind::ConstBytePointer)
      {
        ReturnAddress = &ReturnStorage.Pointer;
      }
      ffi_call(&Prepared->Interface, FFI_FN(Prepared->Address), ReturnAddress, NativeArguments.data());
      Result = loadNativeValue(FunctionValue.ResultType, ReturnStorage);
      return true;
    }

    bool executeFunction(std::size_t FunctionIndex, const std::vector<RuntimeValue> &Arguments, std::size_t Depth, RuntimeValue &Result, std::vector<core::Diagnostic> &Diagnostics)
    {
      const ir::Function &FunctionValue = ModuleValue.Functions[FunctionIndex];
      if (Depth >= MaximumCallDepth)
      {
        addFailure(Diagnostics, "maximum InkIR call depth exceeded in function @" + FunctionValue.Name);
        return false;
      }
      if (FunctionValue.Kind == ir::FunctionKind::External)
      {
        return callExternal(FunctionIndex, Arguments, Result, Diagnostics);
      }
      if (FunctionValue.Blocks.size() != 1)
      {
        addFailure(Diagnostics, "function @" + FunctionValue.Name + " cannot execute until InkIR has control-flow instructions because it contains multiple basic blocks");
        return false;
      }

      std::unordered_map<std::size_t, RuntimeValue> Frame;
      for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
      {
        Frame.emplace(ArgumentIndex, Arguments[ArgumentIndex]);
      }

      for (const std::unique_ptr<ir::Instruction> &InstructionPointer : FunctionValue.Blocks[0].Instructions)
      {
        if (InstructionPointer->kind() == ir::InstructionKind::Call)
        {
          const ir::CallInstruction &Call = static_cast<const ir::CallInstruction &>(*InstructionPointer);
          std::vector<RuntimeValue> CallArguments;
          CallArguments.reserve(Call.Arguments.size());
          for (const std::unique_ptr<ir::Value> &Argument : Call.Arguments)
          {
            RuntimeValue ArgumentValue = RuntimeValue::voidValue();
            if (!evaluateValue(*Argument, Frame, ArgumentValue, Diagnostics))
            {
              return false;
            }
            CallArguments.push_back(ArgumentValue);
          }

          RuntimeValue CallResult = RuntimeValue::voidValue();
          if (!executeFunction(Call.Callee.value(), CallArguments, Depth + 1, CallResult, Diagnostics))
          {
            return false;
          }
          if (Call.Result.has_value())
          {
            Frame.emplace(Call.Result->value(), CallResult);
          }
          continue;
        }

        if (InstructionPointer->kind() == ir::InstructionKind::Return)
        {
          const ir::ReturnInstruction &Return = static_cast<const ir::ReturnInstruction &>(*InstructionPointer);
          if (!Return.ReturnValue)
          {
            Result = RuntimeValue::voidValue();
            return true;
          }
          return evaluateValue(*Return.ReturnValue, Frame, Result, Diagnostics);
        }

        addFailure(Diagnostics, "function @" + FunctionValue.Name + " contains an unsupported instruction");
        return false;
      }

      addFailure(Diagnostics, "function @" + FunctionValue.Name + " finished without returning");
      return false;
    }

    ExecutionContext &Context;
    const ir::Module &ModuleValue;
    std::vector<std::unique_ptr<PreparedFunction>> PreparedFunctions;
    bool Initialized = false;
  };

  RuntimeValue RuntimeValue::voidValue() noexcept
  {
    return {};
  }

  RuntimeValue RuntimeValue::integerValue(ir::TypeKind Type, std::uint64_t Value) noexcept
  {
    RuntimeValue Result;
    Result.Type = Type;
    Result.Integer = Value;
    return Result;
  }

  RuntimeValue RuntimeValue::pointerValue(ir::TypeKind Type, const void *Value) noexcept
  {
    RuntimeValue Result;
    Result.Type = Type;
    Result.Pointer = Value;
    return Result;
  }

  ir::TypeKind RuntimeValue::type() const noexcept
  {
    return Type;
  }

  std::optional<std::uint64_t> RuntimeValue::integer() const noexcept
  {
    if (Type == ir::TypeKind::Bool || Type == ir::TypeKind::Byte || Type == ir::TypeKind::I32 || Type == ir::TypeKind::PointerSize)
    {
      return Integer;
    }
    return std::nullopt;
  }

  const void *RuntimeValue::pointer() const noexcept
  {
    return Type == ir::TypeKind::ConstBytePointer ? Pointer : nullptr;
  }

  bool InitializationResult::succeeded() const noexcept
  {
    return Succeeded && Diagnostics.empty();
  }

  const std::vector<core::Diagnostic> &InitializationResult::diagnostics() const noexcept
  {
    return Diagnostics;
  }

  bool ExecutionResult::succeeded() const noexcept
  {
    return ReturnValue.has_value() && Diagnostics.empty();
  }

  const std::optional<RuntimeValue> &ExecutionResult::returnValue() const noexcept
  {
    return ReturnValue;
  }

  const std::vector<core::Diagnostic> &ExecutionResult::diagnostics() const noexcept
  {
    return Diagnostics;
  }

  ExecutionEngine::ExecutionEngine(ExecutionContext &Context, const ir::Module &ModuleValue) : Implementation(std::make_unique<Impl>(Context, ModuleValue))
  {
  }

  ExecutionEngine::~ExecutionEngine() = default;

  NativeSymbolRegistry &ExecutionEngine::nativeSymbols() noexcept
  {
    return Implementation->Symbols;
  }

  const NativeSymbolRegistry &ExecutionEngine::nativeSymbols() const noexcept
  {
    return Implementation->Symbols;
  }

  InitializationResult ExecutionEngine::initialize()
  {
    return Implementation->initialize();
  }

  ExecutionResult ExecutionEngine::execute(std::string_view EntryName, const std::vector<RuntimeValue> &Arguments)
  {
    return Implementation->execute(EntryName, Arguments);
  }
} // namespace ink::execution
