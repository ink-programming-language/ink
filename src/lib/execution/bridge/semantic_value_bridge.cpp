#include "ink/execution/bridge/semantic_value_bridge.h"

#include "ink/execution/memory/execution_heap.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

#include <utility>

namespace ink::execution
{
  SemanticValueBridge::SemanticValueBridge(ir::IRContext &Context, bool ResolveNativeImports)
      : Context(Context),
        Types(std::make_shared<RuntimeTypeTable>()),
        ResolveNativeImports(ResolveNativeImports)
  {
  }

  std::shared_ptr<RuntimeTypeTable> SemanticValueBridge::types() const noexcept
  {
    return Types;
  }

  bool SemanticValueBridge::exchangeNativeImportResolution(bool Resolve) noexcept
  {
    return std::exchange(ResolveNativeImports, Resolve);
  }

  RuntimeTypeId SemanticValueBridge::lowerType(const ir::Type &Type)
  {
    if (&Type.context() != &Context)
    {
      return InvalidRuntimeType;
    }
    if (const auto Found = TypeIds.find(&Type); Found != TypeIds.end())
    {
      return Found->second;
    }
    StorageLayout Layout;
    const auto &Target = Context.compilationContext().targetContext();
    switch (Type.typeKind())
    {
    case ir::TypeKind::Void:
      Layout.Kind = RuntimeKind::Void;
      break;
    case ir::TypeKind::Bool:
      Layout.Kind = RuntimeKind::Boolean;
      Layout.BitWidth = 1;
      Layout.Size = sizeof(bool);
      Layout.Alignment = alignof(bool);
      Layout.Native = Target.isNativeAbiCompatible();
      break;
    case ir::TypeKind::Integer:
    {
      const auto &Integer = static_cast<const ir::IntegerType &>(Type);
      Layout.Kind = RuntimeKind::Integer;
      Layout.BitWidth = Integer.bitWidth();
      Layout.Signed = Integer.isSigned();
      Layout.Size = (static_cast<std::size_t>(Layout.BitWidth) + 7) / 8;
      Layout.Native = Target.isNativeAbiCompatible() && (Layout.BitWidth == 8 || Layout.BitWidth == 16 || Layout.BitWidth == 32 || Layout.BitWidth == 64);
      if (Layout.Native)
      {
        switch (Layout.BitWidth)
        {
        case 8:
          Layout.Alignment = alignof(std::uint8_t);
          break;
        case 16:
          Layout.Alignment = alignof(std::uint16_t);
          break;
        case 32:
          Layout.Alignment = alignof(std::uint32_t);
          break;
        case 64:
          Layout.Alignment = alignof(std::uint64_t);
          break;
        }
      }
      break;
    }
    case ir::TypeKind::Float:
      Layout.Kind = RuntimeKind::Float;
      Layout.BitWidth = static_cast<const ir::FloatType &>(Type).bitWidth();
      Layout.Size = Layout.BitWidth / 8;
      Layout.Native = Target.isNativeAbiCompatible() && (Layout.BitWidth == 32 || Layout.BitWidth == 64);
      Layout.Alignment = Layout.BitWidth == 32 ? alignof(float) : Layout.BitWidth == 64 ? alignof(double) : 1;
      break;
    case ir::TypeKind::Pointer:
    {
      const auto &Pointer = static_cast<const ir::PointerType &>(Type);
      Layout.Kind = RuntimeKind::Pointer;
      Layout.Pointee = lowerType(Pointer.pointeeType());
      Layout.Writable = Pointer.access() == ir::AccessKind::ReadWrite;
      Layout.Size = Target.pointerByteWidth();
      Layout.Alignment = Layout.Size;
      break;
    }
    case ir::TypeKind::Slice:
    {
      const auto &Slice = static_cast<const ir::SliceType &>(Type);
      const ir::Type &Element = Slice.elementType();
      if (Slice.access() == ir::AccessKind::ReadOnly && Element.typeKind() == ir::TypeKind::Integer && static_cast<const ir::IntegerType &>(Element).bitWidth() == 8 && !static_cast<const ir::IntegerType &>(Element).isSigned())
      {
        Layout.Kind = RuntimeKind::String;
        Layout.Size = Target.pointerByteWidth() * 2;
        Layout.Alignment = Target.pointerByteWidth();
      }
      break;
    }
    case ir::TypeKind::Function:
    {
      const auto &Function = static_cast<const ir::FunctionType &>(Type);
      Layout.Kind = RuntimeKind::Function;
      Layout.ReturnType = lowerType(Function.returnType());
      for (const ir::Type *Parameter : Function.parameterTypes())
      {
        Layout.Parameters.push_back(lowerType(*Parameter));
      }
      Layout.Size = sizeof(FunctionId);
      Layout.Alignment = alignof(FunctionId);
      break;
    }
    default:
      break;
    }
    const RuntimeTypeId Id = Types->append(std::move(Layout));
    if (Id == InvalidRuntimeType)
    {
      return Id;
    }
    TypeIds.emplace(&Type, Id);
    if (SourceTypes.size() <= Id)
    {
      SourceTypes.resize(static_cast<std::size_t>(Id) + 1, nullptr);
    }
    SourceTypes[Id] = &Type;
    return Id;
  }

  const ir::Type *SemanticValueBridge::sourceType(RuntimeTypeId Type) const noexcept
  {
    return Type < SourceTypes.size() ? SourceTypes[Type] : nullptr;
  }

  void SemanticValueBridge::updateNativeExports()
  {
    if (NativeExportRevision == Context.revision())
    {
      return;
    }
    NativeExports.clear();
    std::vector<const ir::BasicBlock *> Pending;
    for (const auto &Module : Context.modules())
    {
      Pending.push_back(&Module->entryBlock());
    }
    while (!Pending.empty())
    {
      const ir::BasicBlock *Block = Pending.back();
      Pending.pop_back();
      for (const auto &Value : Block->values())
      {
        if (ir::Module::classof(Value.get()))
        {
          Pending.push_back(&static_cast<const ir::Module &>(*Value).entryBlock());
        }
        else if (ir::Function::classof(Value.get()))
        {
          const auto &Function = static_cast<const ir::Function &>(*Value);
          if (Function.isNativeExport())
          {
            const auto [Entry, Inserted] = NativeExports.emplace(Context.namePool().text(Function.name()), &Function);
            if (!Inserted)
            {
              Entry->second = nullptr;
            }
          }
        }
      }
    }
    NativeExportRevision = Context.revision();
  }

  FunctionId SemanticValueBridge::lowerFunction(const ir::Function &Function)
  {
    if (&Function.context() != &Context)
    {
      return InvalidFunction;
    }
    if (ResolveNativeImports && Function.isNativeImport())
    {
      updateNativeExports();
      const auto Exported = NativeExports.find(std::string(Context.namePool().text(Function.name())));
      if (Exported != NativeExports.end())
      {
        const ir::Function *Target = Exported->second;
        if (!Target || &Target->functionType() != &Function.functionType() || Target->languageLinkage() != Function.languageLinkage() || Target->callingConvention() != Function.callingConvention())
        {
          return InvalidFunction;
        }
        return lowerFunction(*Target);
      }
    }
    const auto Found = FunctionIds.find(&Function);
    FunctionId Id;
    if (Found != FunctionIds.end())
    {
      Id = Found->second;
    }
    else
    {
      if (Functions.size() >= InvalidFunction)
      {
        return InvalidFunction;
      }
      Id = static_cast<FunctionId>(Functions.size());
      FunctionIds.emplace(&Function, Id);
      SourceFunctions.push_back(&Function);
      Functions.emplace_back();
    }
    RuntimeFunctionDescriptor &Descriptor = Functions[Id];
    Descriptor.Id = Id;
    Descriptor.Signature = lowerType(Function.type());
    Descriptor.Symbol = Context.namePool().text(Function.name());
    Descriptor.External = Function.isNativeImport();
    Descriptor.NativeAbi = Context.compilationContext().targetContext().isNativeAbiCompatible();
    Descriptor.CAbi = Function.languageLinkage() == ir::LanguageLinkage::C;
    Descriptor.Exported = Function.isNativeExport();
    Descriptor.Supported = Descriptor.CAbi && Function.callingConvention() == ir::CallingConvention::C;
    for (const auto &Parameter : Function.parameters())
    {
      if (Parameter->parameterKind() == ir::ParameterKind::Variadic)
      {
        Descriptor.Supported = false;
      }
    }
    return Id;
  }

  const ir::Function *SemanticValueBridge::sourceFunction(FunctionId Function) const noexcept
  {
    return Function < SourceFunctions.size() ? SourceFunctions[Function] : nullptr;
  }

  const RuntimeFunctionDescriptor *SemanticValueBridge::functionDescriptor(FunctionId Function) const noexcept
  {
    return Function < Functions.size() ? &Functions[Function] : nullptr;
  }

  RuntimeValueResult SemanticValueBridge::lowerValue(const ExecutionValueRef &Value)
  {
    if (!Value.type())
    {
      return {ExecutionStatus::TypeMismatch};
    }
    if (&Value.type()->context() != &Context)
    {
      return {ExecutionStatus::ForeignContext};
    }
    if (Value.kind() == ExecutionValueKind::Pointer && Value.pointer().status() != ExecutionStatus::Success)
    {
      return {Value.pointer().status()};
    }
    if (!Value.valid())
    {
      return {ExecutionStatus::TypeMismatch};
    }
    const RuntimeTypeId Type = lowerType(*Value.type());
    if (Type == InvalidRuntimeType)
    {
      return {ExecutionStatus::Overflow};
    }
    switch (Value.kind())
    {
    case ExecutionValueKind::Void:
      return {ExecutionStatus::Success, RuntimeValue::fromBits(0, Type)};
    case ExecutionValueKind::Boolean:
      return {ExecutionStatus::Success, RuntimeValue::fromBits(Value.boolean(), Type)};
    case ExecutionValueKind::Integer:
      return {ExecutionStatus::Success, RuntimeValue::fromInteger(Value.integer(), Type)};
    case ExecutionValueKind::Float:
      return {ExecutionStatus::Success, RuntimeValue::fromBits(Value.floating().bits(), Type)};
    case ExecutionValueKind::String:
      return {ExecutionStatus::Success, RuntimeValue::fromString(Value.string(), Type)};
    case ExecutionValueKind::Pointer:
      return {ExecutionStatus::Success, RuntimeValue::fromPointer(Value.pointer(), Type)};
    case ExecutionValueKind::Function:
    {
      const FunctionId Id = lowerFunction(*Value.function());
      return Id == InvalidFunction ? RuntimeValueResult{ExecutionStatus::Overflow} : RuntimeValueResult{ExecutionStatus::Success, RuntimeValue::fromBits(Id, Type)};
    }
    case ExecutionValueKind::Invalid:
      return {ExecutionStatus::TypeMismatch};
    }
    return {ExecutionStatus::UnsupportedOperation};
  }

  RuntimeValueResult SemanticValueBridge::lowerConstant(const ir::Value &Value)
  {
    if (&Value.context() != &Context)
    {
      return {ExecutionStatus::ForeignContext};
    }
    if (ir::Function::classof(&Value))
    {
      const FunctionId Id = lowerFunction(static_cast<const ir::Function &>(Value));
      return Id == InvalidFunction ? RuntimeValueResult{ExecutionStatus::Overflow} : RuntimeValueResult{ExecutionStatus::Success, RuntimeValue::fromBits(Id, lowerType(Value.type()))};
    }
    if (!ir::Constant::classof(&Value))
    {
      return {ExecutionStatus::RuntimeValue};
    }
    if (!Context.constantPool().owns(static_cast<const ir::Constant &>(Value)))
    {
      return {ExecutionStatus::ForeignContext};
    }
    const RuntimeTypeId Type = lowerType(Value.type());
    if (Type == InvalidRuntimeType)
    {
      return {ExecutionStatus::Overflow};
    }
    switch (Value.kind())
    {
    case ir::ValueKind::BoolConstant:
      return {ExecutionStatus::Success, RuntimeValue::fromBits(static_cast<const ir::BoolConstant &>(Value).value(), Type)};
    case ir::ValueKind::IntegerConstant:
      return {ExecutionStatus::Success, RuntimeValue::fromInteger(ExecutionInteger(static_cast<const ir::IntegerConstant &>(Value).value()), Type)};
    case ir::ValueKind::FloatConstant:
      return {ExecutionStatus::Success, RuntimeValue::fromBits(static_cast<const ir::FloatConstant &>(Value).value().bits(), Type)};
    case ir::ValueKind::StringConstant:
      return {ExecutionStatus::Success, RuntimeValue::fromString(static_cast<const ir::StringConstant &>(Value).value(), Type)};
    default:
      return {ExecutionStatus::UnsupportedOperation};
    }
  }

  ExecutionValueResult SemanticValueBridge::raiseValue(ExecutionHeap &Heap, const RuntimeValue &Value, RuntimeTypeId Type)
  {
    if (!Value.Initialized)
    {
      return {ExecutionStatus::Uninitialized};
    }
    const ir::Type *Source = sourceType(Type);
    const StorageLayout *Layout = Types->get(Type);
    if (!Source || !Layout || Value.Type != Type)
    {
      return {ExecutionStatus::TypeMismatch};
    }
    ExecutionValueRef Result;
    switch (Layout->Kind)
    {
    case RuntimeKind::Void:
      Result = Heap.voidValue(*Source);
      break;
    case RuntimeKind::Boolean:
      if (Value.Object || Value.Bits > 1)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      Result = Heap.boolean(*Source, Value.Bits != 0);
      break;
    case RuntimeKind::Integer:
      if (Layout->BitWidth > 64)
      {
        if (Value.kind() != RuntimeKind::Integer || Value.integer().bitWidth() != Layout->BitWidth)
        {
          return {ExecutionStatus::TypeMismatch};
        }
        Result = Heap.integer(*Source, Value.integer());
      }
      else
      {
        if (Value.Object)
        {
          return {ExecutionStatus::TypeMismatch};
        }
        Result = Heap.integer(*Source, ExecutionInteger(Layout->BitWidth, Value.Bits));
      }
      break;
    case RuntimeKind::Float:
      if (Value.Object)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      Result = Heap.floating(*Source, ir::FloatBits(Layout->BitWidth, Value.Bits));
      break;
    case RuntimeKind::String:
      if (Value.kind() != RuntimeKind::String)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      Result = Heap.string(*Source, Value.string());
      break;
    case RuntimeKind::Pointer:
      if (Value.kind() != RuntimeKind::Pointer)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      Result = Heap.pointerSnapshot(*Source, Value.pointer());
      break;
    case RuntimeKind::Function:
    {
      if (Value.Object || Value.Bits >= InvalidFunction)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      const ir::Function *Function = sourceFunction(static_cast<FunctionId>(Value.Bits));
      if (!Function || &Function->type() != Source)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      Result = Heap.function(*Function);
      break;
    }
    case RuntimeKind::Invalid:
      return {ExecutionStatus::UnsupportedOperation};
    }
    return Result ? ExecutionValueResult{ExecutionStatus::Success, std::move(Result)} : ExecutionValueResult{Heap.lastStatus()};
  }
} // namespace ink::execution
