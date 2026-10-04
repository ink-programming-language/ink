#include "../lowering_context.h"
#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/execution/reflection/native_reflection.h"
#include "ink/ir/linkage.h"

#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalVariable.h>

namespace ink::backend::llvm
{
  bool LoweringContext::lowerReflection()
  {
    using namespace execution;
    SemanticValueBridge Bridge(const_cast<ir::IRContext &>(Source.context()));
    for (const ir::Function *Function : SourceFunctions)
    {
      Bridge.lowerFunction(*Function);
      if (Function->classOwner())
      {
        Bridge.lowerType(*Function->classOwner());
      }
    }
    Bridge.retainReflectionTypes();
    auto *Word = ::llvm::Type::getInt32Ty(Context);
    auto *Wide = ::llvm::Type::getInt64Ty(Context);
    const auto U32 = [Word](std::uint32_t Value)
    {
      return ::llvm::ConstantInt::get(Word, Value);
    };
    const auto Size = [&](std::uint64_t Value)
    {
      return ::llvm::ConstantInt::get(SizeType, Value);
    };
    const auto Text = [&](std::string_view Value) -> ::llvm::Constant *
    {
      auto *Bytes = ::llvm::ConstantDataArray::getString(Context, ::llvm::StringRef(Value.data(), Value.size()));
      return new ::llvm::GlobalVariable(Module, Bytes->getType(), true, ::llvm::GlobalValue::PrivateLinkage, Bytes, "reflection.name");
    };
    const auto Record = [&](::llvm::StructType *Type, ::llvm::ArrayRef<::llvm::Constant *> Values) -> ::llvm::Constant *
    {
      auto *Value = ::llvm::ConstantStruct::get(Type, Values);
      return new ::llvm::GlobalVariable(Module, Type, true, ::llvm::GlobalValue::PrivateLinkage, Value, "reflection.desc");
    };
    const auto Array = [&](::llvm::Type *Type, const std::vector<::llvm::Constant *> &Values) -> ::llvm::Constant *
    {
      auto *ArrayType = ::llvm::ArrayType::get(Type, Values.size());
      auto *Value = ::llvm::ConstantArray::get(ArrayType, Values);
      return new ::llvm::GlobalVariable(Module, ArrayType, true, ::llvm::GlobalValue::PrivateLinkage, Value, "reflection.entries");
    };
    std::unordered_map<const ir::Function *, ::llvm::Constant *> Thunks;
    const auto Thunk = [&](FunctionId Id) -> ::llvm::Constant *
    {
      const auto *Function = Bridge.sourceFunction(Id);
      if (!Function || !Functions.contains(Function))
      {
        return ::llvm::ConstantPointerNull::get(PointerType);
      }
      if (const auto Found = Thunks.find(Function); Found != Thunks.end())
      {
        return Found->second;
      }
      auto *Result = reflectionThunk(*Function);
      Thunks.emplace(Function, Result);
      return Result;
    };
    auto *IntegerType = ::llvm::StructType::get(Context, {Word, Word});
    auto *FloatType = ::llvm::StructType::get(Context, ::llvm::ArrayRef<::llvm::Type *>{Word});
    auto *PointerDescType = IntegerType;
    auto *SignatureType = ::llvm::StructType::get(Context, {Word, SizeType, PointerType});
    auto *ArrayType = ::llvm::StructType::get(Context, {Word, Wide});
    auto *FieldType = ::llvm::StructType::get(Context, {PointerType, Word, SizeType, Word, PointerType});
    auto *MethodType = ::llvm::StructType::get(Context, {PointerType, Word, Word, Word, PointerType});
    auto *ClassType = ::llvm::StructType::get(Context, {SizeType, PointerType, SizeType, PointerType});
    auto *TypeType = ::llvm::StructType::get(Context, {Word, PointerType, SizeType, SizeType, PointerType});
    auto *ModuleType = ::llvm::StructType::get(Context, {Word, PointerType, SizeType, PointerType});
    // This backend currently targets the host ABI; verify its exported C table.
    const auto &Target = Module.getDataLayout();
    if (Target.getTypeAllocSize(TypeType) != sizeof(NativeTypeDesc) || Target.getTypeAllocSize(FieldType) != sizeof(NativeFieldDesc) || Target.getTypeAllocSize(MethodType) != sizeof(NativeMethodDesc) || Target.getTypeAllocSize(ModuleType) != sizeof(NativeModuleDesc))
    {
      return fail("AOT reflection table differs from the native descriptor ABI");
    }
    std::vector<::llvm::Constant *> Types;
    for (std::size_t Index = 0; Index < Bridge.types()->size(); ++Index)
    {
      const auto &Type = *Bridge.types()->get(static_cast<RuntimeTypeId>(Index));
      ::llvm::Constant *Details = ::llvm::ConstantPointerNull::get(PointerType);
      switch (Type.Kind)
      {
      case RuntimeKind::Integer:
        Details = Record(IntegerType, {U32(Type.bitWidth()), U32(Type.isSigned())});
        break;
      case RuntimeKind::Float:
        Details = Record(FloatType, {U32(Type.bitWidth())});
        break;
      case RuntimeKind::Pointer:
        Details = Record(PointerDescType, {U32(Type.pointerDesc().Pointee), U32(Type.pointerDesc().Writable)});
        break;
      case RuntimeKind::Function:
      {
        std::vector<::llvm::Constant *> Parameters;
        for (const auto Parameter : Type.functionDesc().Parameters)
        {
          Parameters.push_back(U32(Parameter));
        }
        Details = Record(SignatureType, {U32(Type.functionDesc().ReturnType), Size(Parameters.size()), Array(Word, Parameters)});
        break;
      }
      case RuntimeKind::Array:
        Details = Record(ArrayType, {U32(Type.arrayDesc().ElementType), ::llvm::ConstantInt::get(Wide, Type.arrayDesc().ElementCount)});
        break;
      case RuntimeKind::Class:
      {
        std::vector<::llvm::Constant *> Fields;
        for (const auto &Field : Type.classDesc().Fields)
        {
          auto *Initializer = Thunk(Field.Initializer);
          if (!Initializer)
          {
            return false;
          }
          Fields.push_back(::llvm::ConstantStruct::get(FieldType, {Text(Field.Name), U32(Field.Type), Size(Field.Offset), U32(static_cast<unsigned>(Field.Visibility)), Initializer}));
        }
        std::vector<::llvm::Constant *> Methods;
        for (const auto &Method : Type.classDesc().Methods)
        {
          auto *Invoke = Thunk(Method.Function);
          if (!Invoke)
          {
            return false;
          }
          Methods.push_back(::llvm::ConstantStruct::get(MethodType, {Text(Method.Name), U32(Method.Signature), U32(static_cast<unsigned>(Method.Visibility)), U32(Method.WritableReceiver), Invoke}));
        }
        Details = Record(ClassType, {Size(Fields.size()), Array(FieldType, Fields), Size(Methods.size()), Array(MethodType, Methods)});
        break;
      }
      default:
        break;
      }
      Types.push_back(::llvm::ConstantStruct::get(TypeType, {U32(static_cast<unsigned>(Type.Kind)), Text(Type.Name), Size(Type.Size), Size(Type.Alignment), Details}));
    }
    const auto ModuleName = Source.context().namePool().text(Source.name());
    auto *Description = Record(ModuleType, {U32(1), Text(ModuleName), Size(Types.size()), Array(TypeType, Types)});
    const auto Mangled = ir::reflectionSymbol(Source);
    if (!Mangled)
    {
      return fail(Mangled.Error);
    }
    auto *Getter = ::llvm::Function::Create(::llvm::FunctionType::get(PointerType, false), ::llvm::GlobalValue::ExternalLinkage, Mangled.Name, Module);
    ::llvm::IRBuilder<> Builder(::llvm::BasicBlock::Create(Context, "entry", Getter));
    Builder.CreateRet(Description);
    return true;
  }
} // namespace ink::backend::llvm
