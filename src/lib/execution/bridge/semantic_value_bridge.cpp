#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/class_constant.h"
#include "ink/ir/analysis/type_layout.h"

#include <limits>

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
      const TypeDesc *Layout = Types->get(Found->second);
      synchronizeReflection();
      return Layout && Layout->Kind == RuntimeKind::Invalid && !DefiningTypes.contains(&Type) ? defineType(Type, Found->second) : Found->second;
    }
    const RuntimeTypeId Id = Types->reserve();
    if (Id == InvalidRuntimeType)
    {
      return Id;
    }
    TypeIds.emplace(&Type, Id);
    return defineType(Type, Id);
  }

  RuntimeTypeId SemanticValueBridge::defineType(const ir::Type &Type, RuntimeTypeId Id)
  {
    struct DefinitionScope
    {
        std::unordered_set<const ir::Type *> &Active;
        const ir::Type *Type;

        ~DefinitionScope()
        {
          Active.erase(Type);
        }
    };
    DefiningTypes.insert(&Type);
    DefinitionScope Scope{DefiningTypes, &Type};
    const auto &Target = Context.compilationContext().targetContext();
    const auto Computed = Type.typeKind() == ir::TypeKind::Void ? std::optional<ir::TypeLayout>(ir::TypeLayout{}) : ir::computeTypeLayout(Type, Target);
    if (!Computed || Computed->Size > std::numeric_limits<std::size_t>::max() || Computed->Alignment > std::numeric_limits<std::size_t>::max())
    {
      TypeIds.erase(&Type);
      return InvalidRuntimeType;
    }
    TypeDesc Layout;
    Layout.Size = static_cast<std::size_t>(Computed->Size);
    Layout.Alignment = static_cast<std::size_t>(Computed->Alignment);
    std::vector<std::pair<const ir::Type *, RuntimeTypeId>> Deferred;
    // Pointer and function layouts do not depend on the storage of their referenced
    // types. Complete these layouts before following their reserved type identities.
    const auto ReserveReference = [&](const ir::Type &Referenced)
    {
      if (const auto Found = TypeIds.find(&Referenced); Found != TypeIds.end())
      {
        return Found->second;
      }
      const RuntimeTypeId Reference = Types->reserve();
      if (Reference != InvalidRuntimeType)
      {
        TypeIds.emplace(&Referenced, Reference);
        Deferred.emplace_back(&Referenced, Reference);
      }
      return Reference;
    };
    switch (Type.typeKind())
    {
    case ir::TypeKind::Void:
      Layout.setKind(RuntimeKind::Void);
      Layout.Name = "void";
      break;
    case ir::TypeKind::Bool:
      Layout.setKind(RuntimeKind::Boolean);
      Layout.Name = "bool";
      Layout.setBitWidth(1);
      Layout.Native = Target.isNativeAbiCompatible() && Layout.Size == sizeof(bool) && Layout.Alignment == alignof(bool);
      break;
    case ir::TypeKind::Integer:
    {
      const auto &Integer = static_cast<const ir::IntegerType &>(Type);
      Layout.setKind(RuntimeKind::Integer);
      Layout.setBitWidth(Integer.bitWidth());
      Layout.editInteger().Signed = Integer.isSigned();
      Layout.Name = std::string(Integer.isSigned() ? "i" : "u") + std::to_string(Integer.bitWidth());
      Layout.Native = Target.isNativeAbiCompatible() && (Layout.bitWidth() == 8 || Layout.bitWidth() == 16 || Layout.bitWidth() == 32 || Layout.bitWidth() == 64);
      break;
    }
    case ir::TypeKind::Float:
      Layout.setKind(RuntimeKind::Float);
      Layout.setBitWidth(static_cast<const ir::FloatType &>(Type).bitWidth());
      Layout.Name = "f" + std::to_string(Layout.bitWidth());
      Layout.Native = Target.isNativeAbiCompatible() && (Layout.bitWidth() == 32 || Layout.bitWidth() == 64);
      break;
    case ir::TypeKind::Pointer:
    {
      const auto &Pointer = static_cast<const ir::PointerType &>(Type);
      Layout.setKind(RuntimeKind::Pointer);
      Layout.editPointer().Pointee = ReserveReference(Pointer.pointeeType());
      Layout.editPointer().Writable = Pointer.access() == ir::AccessKind::ReadWrite;
      if (Layout.pointerDesc().Pointee == InvalidRuntimeType)
      {
        TypeIds.erase(&Type);
        return InvalidRuntimeType;
      }
      break;
    }
    case ir::TypeKind::Array:
    {
      const auto &Array = static_cast<const ir::ArrayType &>(Type);
      Layout.setKind(RuntimeKind::Array);
      Layout.editArray().ElementType = lowerType(Array.elementType());
      const TypeDesc *Element = Types->get(Layout.arrayDesc().ElementType);
      if (!Element || Element->Kind == RuntimeKind::Invalid || Element->Kind == RuntimeKind::Void)
      {
        TypeIds.erase(&Type);
        return InvalidRuntimeType;
      }
      Layout.editArray().ElementCount = Array.elementCount();
      Layout.Native = Element->Native;
      break;
    }
    case ir::TypeKind::Class:
    {
      const auto &Class = static_cast<const ir::ClassType &>(Type);
      auto Description = std::make_shared<ClassDesc>();
      Layout.setKind(RuntimeKind::Class);
      Description->NominalIdentity = Class.identity();
      Layout.Native = true;
      for (std::size_t Index = 0; Index < Class.fields().size(); ++Index)
      {
        const RuntimeTypeId Field = lowerType(*Class.fields()[Index].FieldType);
        const TypeDesc *FieldLayout = Types->get(Field);
        if (!FieldLayout || FieldLayout->Kind == RuntimeKind::Invalid || FieldLayout->Kind == RuntimeKind::Void || Computed->FieldOffsets[Index] > std::numeric_limits<std::size_t>::max())
        {
          TypeIds.erase(&Type);
          return InvalidRuntimeType;
        }
        Description->Fields.push_back({std::string(Type.context().namePool().text(Class.fields()[Index].FieldName)), Field, static_cast<std::size_t>(Computed->FieldOffsets[Index]), Class.fields()[Index].Visibility == ir::VisibilityKind::Private ? MemberVisibility::Private : MemberVisibility::Public});
        Layout.Native = Layout.Native && FieldLayout->Native;
      }
      Layout.setDetails(std::move(Description));
      break;
    }
    case ir::TypeKind::Slice:
    {
      const auto &Slice = static_cast<const ir::SliceType &>(Type);
      const ir::Type &Element = Slice.elementType();
      if (Slice.access() == ir::AccessKind::ReadOnly && Element.typeKind() == ir::TypeKind::Integer && static_cast<const ir::IntegerType &>(Element).bitWidth() == 8 && !static_cast<const ir::IntegerType &>(Element).isSigned())
      {
        Layout.setKind(RuntimeKind::String);
        Layout.Name = "string";
      }
      break;
    }
    case ir::TypeKind::Function:
    {
      const auto &Function = static_cast<const ir::FunctionType &>(Type);
      Layout.setKind(RuntimeKind::Function);
      Layout.editFunction().ReturnType = ReserveReference(Function.returnType());
      for (const ir::Type *Parameter : Function.parameterTypes())
      {
        Layout.editFunction().Parameters.push_back(ReserveReference(*Parameter));
      }
      break;
    }
    default:
      break;
    }
    if (Layout.Kind == RuntimeKind::Invalid)
    {
      TypeIds.erase(&Type);
      return InvalidRuntimeType;
    }
    if (!Types->define(Id, std::move(Layout)))
    {
      TypeIds.erase(&Type);
      return InvalidRuntimeType;
    }
    if (SourceTypes.size() <= Id)
    {
      SourceTypes.resize(static_cast<std::size_t>(Id) + 1, nullptr);
    }
    SourceTypes[Id] = &Type;
    ReflectionTypeCount = std::numeric_limits<std::size_t>::max();
    for (const auto &[Referenced, Reference] : Deferred)
    {
      const TypeDesc *ReferencedLayout = Types->get(Reference);
      if (!ReferencedLayout || (ReferencedLayout->Kind == RuntimeKind::Invalid && defineType(*Referenced, Reference) == InvalidRuntimeType))
      {
        TypeIds.erase(&Type);
        return InvalidRuntimeType;
      }
    }
    synchronizeReflection();
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
    case ExecutionValueKind::Array:
    {
      std::vector<RuntimeValue> Elements;
      for (const auto &Element : Value.array())
      {
        RuntimeValueResult Converted = lowerValue(Element);
        if (!Converted)
        {
          return Converted;
        }
        Elements.push_back(std::move(Converted.Value));
      }
      return {ExecutionStatus::Success, RuntimeValue::fromArray(std::move(Elements), Type)};
    }
    case ExecutionValueKind::Class:
    {
      std::vector<RuntimeValue> Elements;
      for (const auto &Element : Value.fields())
      {
        RuntimeValueResult Converted = lowerValue(Element);
        if (!Converted)
        {
          return Converted;
        }
        Elements.push_back(std::move(Converted.Value));
      }
      return {ExecutionStatus::Success, RuntimeValue::fromClass(std::move(Elements), Type)};
    }
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
    case ir::ValueKind::ArrayConstant:
    {
      std::vector<RuntimeValue> Elements;
      for (const ir::Constant *Element : static_cast<const ir::ArrayConstant &>(Value).elements())
      {
        RuntimeValueResult Converted = lowerConstant(*Element);
        if (!Converted)
        {
          return Converted;
        }
        Elements.push_back(std::move(Converted.Value));
      }
      return {ExecutionStatus::Success, RuntimeValue::fromArray(std::move(Elements), Type)};
    }
    case ir::ValueKind::ClassConstant:
    {
      std::vector<RuntimeValue> Elements;
      for (const ir::Constant *Element : static_cast<const ir::ClassConstant &>(Value).fields())
      {
        RuntimeValueResult Converted = lowerConstant(*Element);
        if (!Converted)
        {
          return Converted;
        }
        Elements.push_back(std::move(Converted.Value));
      }
      return {ExecutionStatus::Success, RuntimeValue::fromClass(std::move(Elements), Type)};
    }
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
    const TypeDesc *Layout = Types->get(Type);
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
      if (Layout->bitWidth() > 64)
      {
        if (Value.kind() != RuntimeKind::Integer || Value.integer().bitWidth() != Layout->bitWidth())
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
        Result = Heap.integer(*Source, ExecutionInteger(Layout->bitWidth(), Value.Bits));
      }
      break;
    case RuntimeKind::Float:
      if (Value.Object)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      Result = Heap.floating(*Source, ir::FloatBits(Layout->bitWidth(), Value.Bits));
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
    case RuntimeKind::Array:
    {
      if (Value.kind() != RuntimeKind::Array || Value.array().size() != Layout->arrayDesc().ElementCount)
      {
        return {ExecutionStatus::TypeMismatch};
      }
      std::vector<ExecutionValueRef> Elements;
      for (const RuntimeValue &Element : Value.array())
      {
        if (Element.kind() == RuntimeKind::Pointer && Element.pointer().status() != ExecutionStatus::Success)
        {
          return {Element.pointer().status()};
        }
        ExecutionValueResult Converted = raiseValue(Heap, Element, Layout->arrayDesc().ElementType);
        if (!Converted)
        {
          return Converted;
        }
        Elements.push_back(std::move(Converted.Value));
      }
      Result = Heap.array(*Source, std::move(Elements));
      break;
    }
    case RuntimeKind::Class:
    {
      if (Value.kind() != RuntimeKind::Class || Value.fields().size() != Layout->classDesc().Fields.size())
      {
        return {ExecutionStatus::TypeMismatch};
      }
      std::vector<ExecutionValueRef> Fields;
      for (std::size_t Index = 0; Index < Value.fields().size(); ++Index)
      {
        const RuntimeValue &Field = Value.fields()[Index];
        if (Field.kind() == RuntimeKind::Pointer && Field.pointer().status() != ExecutionStatus::Success)
        {
          return {Field.pointer().status()};
        }
        ExecutionValueResult Converted = raiseValue(Heap, Field, Layout->classDesc().Fields[Index].Type);
        if (!Converted)
        {
          return Converted;
        }
        Fields.push_back(std::move(Converted.Value));
      }
      Result = Heap.classValue(*Source, std::move(Fields));
      break;
    }
    case RuntimeKind::Invalid:
      return {ExecutionStatus::UnsupportedOperation};
    }
    return Result ? ExecutionValueResult{ExecutionStatus::Success, std::move(Result)} : ExecutionValueResult{Heap.lastStatus()};
  }
} // namespace ink::execution
