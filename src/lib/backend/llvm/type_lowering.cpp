#include "lowering_context.h"

#include "ink/ir/constant/class_constant.h"
#include "ink/ir/type/class_type.h"
#include "ink/ir/linkage.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>

#include <limits>

namespace ink::backend::llvm
{
  ::llvm::Type *LoweringContext::lowerType(const ir::Type &Type)
  {
    if (const auto Found = Types.find(&Type); Found != Types.end())
    {
      return Found->second;
    }
    if (!Defining.insert(&Type).second)
    {
      fail("AOT rejects a recursive class stored by value");
      return nullptr;
    }
    ::llvm::Type *Result = nullptr;
    switch (Type.typeKind())
    {
      case ir::TypeKind::Void:
        Result = ::llvm::Type::getVoidTy(Context);
        break;
      case ir::TypeKind::Bool:
        Result = ::llvm::Type::getInt1Ty(Context);
        break;
      case ir::TypeKind::Integer:
        Result = ::llvm::IntegerType::get(Context, static_cast<const ir::IntegerType &>(Type).bitWidth());
        break;
      case ir::TypeKind::Float:
        switch (static_cast<const ir::FloatType &>(Type).bitWidth())
        {
          case 16:
            Result = ::llvm::Type::getHalfTy(Context);
            break;
          case 32:
            Result = ::llvm::Type::getFloatTy(Context);
            break;
          case 64:
            Result = ::llvm::Type::getDoubleTy(Context);
            break;
        }
        break;
      case ir::TypeKind::Pointer:
      case ir::TypeKind::Reference:
      case ir::TypeKind::Function:
        Result = PointerType;
        break;
      case ir::TypeKind::Slice:
        Result = ::llvm::StructType::get(Context, {PointerType, SizeType});
        break;
      case ir::TypeKind::Array:
      {
        const auto &Array = static_cast<const ir::ArrayType &>(Type);
        if (::llvm::Type *Element = lowerType(Array.elementType()); Element && !Element->isVoidTy() && layout(Type))
        {
          Result = ::llvm::ArrayType::get(Element, Array.elementCount());
        }
        break;
      }
      case ir::TypeKind::Class:
      {
        const auto &Class = static_cast<const ir::ClassType &>(Type);
        if (!Class.isComplete() || !layout(Class) || !recordClassABI(Class))
        {
          fail("AOT requires a complete class layout");
          break;
        }
        std::vector<::llvm::Type *> Fields;
        for (const auto &Field : Class.fields())
        {
          ::llvm::Type *FieldType = Field.FieldType ? lowerType(*Field.FieldType) : nullptr;
          if (!FieldType || FieldType->isVoidTy())
          {
            return nullptr;
          }
          Fields.push_back(FieldType);
        }
        // SSA aggregates are logical values. Memory operations use the shared explicit field offsets.
        auto *Existing = ::llvm::StructType::getTypeByName(Context, Class.identity());
        if (Existing && Existing->elements() != ::llvm::ArrayRef<::llvm::Type *>(Fields))
        {
          fail("Conflicting LLVM bodies for the same nominal Ink type");
          return nullptr;
        }
        Result = Existing ? Existing : ::llvm::StructType::create(Context, Fields, Class.identity());
        break;
      }
      default:
        break;
    }
    Defining.erase(&Type);
    if (!Result)
    {
      fail("AOT encountered an unsupported runtime type");
      return nullptr;
    }
    Types.emplace(&Type, Result);
    return Result;
  }

  bool LoweringContext::nativeType(const ir::Type &Type, bool Return)
  {
    switch (Type.typeKind())
    {
      case ir::TypeKind::Void:
        return Return;
      case ir::TypeKind::Bool:
        return true;
      case ir::TypeKind::Integer:
      {
        const auto Width = static_cast<const ir::IntegerType &>(Type).bitWidth();
        return Width == 8 || Width == 16 || Width == 32 || Width == 64;
      }
      case ir::TypeKind::Float:
      {
        const auto Width = static_cast<const ir::FloatType &>(Type).bitWidth();
        return Width == 32 || Width == 64;
      }
      case ir::TypeKind::Pointer:
        return true;
      default:
        return false;
    }
  }

  ::llvm::FunctionType *LoweringContext::signature(const ir::FunctionType &Type, bool Native)
  {
    if (Native && !nativeType(Type.returnType(), true))
    {
      fail("AOT C ABI supports only scalar and raw pointer results");
      return nullptr;
    }
    ::llvm::Type *Result = lowerType(Type.returnType());
    if (!Result)
    {
      return nullptr;
    }
    std::vector<::llvm::Type *> Parameters;
    for (const ir::Type *Parameter : Type.parameterTypes())
    {
      if (Native && !nativeType(*Parameter))
      {
        fail("AOT C ABI does not support class values passed by value");
        return nullptr;
      }
      ::llvm::Type *ParameterType = lowerType(*Parameter);
      if (!ParameterType || ParameterType->isVoidTy())
      {
        return nullptr;
      }
      Parameters.push_back(ParameterType);
    }
    return ::llvm::FunctionType::get(Result, Parameters, false);
  }

  ::llvm::Constant *LoweringContext::constant(const ir::Value &Value)
  {
    if (const auto Found = Constants.find(&Value); Found != Constants.end())
    {
      return Found->second;
    }
    ::llvm::Type *Type = lowerType(Value.type());
    if (!Type)
    {
      return nullptr;
    }
    ::llvm::Constant *Result = nullptr;
    switch (Value.kind())
    {
      case ir::ValueKind::IntegerConstant:
      {
        const ir::IntegerBits &Bits = static_cast<const ir::IntegerConstant &>(Value).value();
        Result = ::llvm::ConstantInt::get(Context, ::llvm::APInt(Bits.bitWidth(), ::llvm::ArrayRef<std::uint64_t>(Bits.words().data(), Bits.words().size())));
        break;
      }
      case ir::ValueKind::BoolConstant:
        Result = ::llvm::ConstantInt::get(Type, static_cast<const ir::BoolConstant &>(Value).value());
        break;
      case ir::ValueKind::FloatConstant:
      {
        const ir::FloatBits &Bits = static_cast<const ir::FloatConstant &>(Value).value();
        const ::llvm::fltSemantics &Semantics = Bits.bitWidth() == 16 ? ::llvm::APFloat::IEEEhalf() : Bits.bitWidth() == 32 ? ::llvm::APFloat::IEEEsingle() : ::llvm::APFloat::IEEEdouble();
        Result = ::llvm::ConstantFP::get(Context, ::llvm::APFloat(Semantics, ::llvm::APInt(Bits.bitWidth(), Bits.bits())));
        break;
      }
      case ir::ValueKind::StringConstant:
      {
        const auto &String = static_cast<const ir::StringConstant &>(Value);
        const std::string_view Data = String.value();
        auto *Bytes = ::llvm::ConstantDataArray::getString(Context, ::llvm::StringRef(Data.data(), Data.size()), true);
        auto *Global = new ::llvm::GlobalVariable(Module, Bytes->getType(), true, ::llvm::GlobalValue::PrivateLinkage, Bytes, "ink.string");
        Global->setAlignment(::llvm::Align(1));
        Result = ::llvm::ConstantStruct::get(static_cast<::llvm::StructType *>(Type), {Global, ::llvm::ConstantInt::get(SizeType, Data.size())});
        break;
      }
      case ir::ValueKind::ArrayConstant:
      {
        std::vector<::llvm::Constant *> Elements;
        for (const auto *Element : static_cast<const ir::ArrayConstant &>(Value).elements())
        {
          ::llvm::Constant *Item = constant(*Element);
          if (!Item)
          {
            return nullptr;
          }
          Elements.push_back(Item);
        }
        Result = ::llvm::ConstantArray::get(static_cast<::llvm::ArrayType *>(Type), Elements);
        break;
      }
      case ir::ValueKind::ClassConstant:
      {
        std::vector<::llvm::Constant *> Fields;
        for (const auto *Field : static_cast<const ir::ClassConstant &>(Value).fields())
        {
          ::llvm::Constant *Item = constant(*Field);
          if (!Item)
          {
            return nullptr;
          }
          Fields.push_back(Item);
        }
        Result = ::llvm::ConstantStruct::get(static_cast<::llvm::StructType *>(Type), Fields);
        break;
      }
      case ir::ValueKind::Function:
      {
        const auto Found = Functions.find(static_cast<const ir::Function *>(&Value));
        if (Found != Functions.end())
        {
          Result = Found->second;
        }
        break;
      }
      default:
        break;
    }
    if (!Result)
    {
      fail("AOT cannot lower this constant or function reference");
      return nullptr;
    }
    Constants.emplace(&Value, Result);
    return Result;
  }
} // namespace ink::backend::llvm
