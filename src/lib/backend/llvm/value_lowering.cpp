#include "lowering_context.h"

#include "ink/ir/analysis/type_layout.h"

#include <llvm/ADT/APFloat.h>
#include <llvm/ADT/APInt.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalVariable.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ink::backend::llvm
{
  namespace
  {
    ::llvm::APFloat floatingPointValue(const ir::FloatConstant &ConstantValue)
    {
      switch (ConstantValue.format())
      {
      case ir::FloatFormat::F16:
        return ::llvm::APFloat(::llvm::APFloat::IEEEhalf(), ::llvm::APInt(16, ConstantValue.bitPattern()));
      case ir::FloatFormat::F32:
        return ::llvm::APFloat(::llvm::APFloat::IEEEsingle(), ::llvm::APInt(32, ConstantValue.bitPattern()));
      case ir::FloatFormat::F64:
        return ::llvm::APFloat(::llvm::APFloat::IEEEdouble(), ::llvm::APInt(64, ConstantValue.bitPattern()));
      }
      return ::llvm::APFloat(0.0);
    }
  } // namespace

  bool LoweringContext::lowerByteConstants()
  {
    ByteConstants.reserve(SourceModule.ByteConstants.size());
    for (const ir::ByteConstant &ConstantValue : SourceModule.ByteConstants)
    {
      ::llvm::Constant *Initializer = ::llvm::ConstantDataArray::getString(Context, ConstantValue.Data, false);
      auto *Global = new ::llvm::GlobalVariable(*TargetModule, Initializer->getType(), true, ::llvm::GlobalValue::PrivateLinkage, Initializer, ConstantValue.Name.str());
      Global->setAlignment(::llvm::Align(1));
      ByteConstants.push_back(Global);
    }
    return true;
  }

  bool LoweringContext::lowerGlobals()
  {
    Globals.reserve(SourceModule.Globals.size());
    for (const ir::GlobalVariable &GlobalValue : SourceModule.Globals)
    {
      if (GlobalValue.Kind == ir::GlobalVariableKind::Imported)
      {
        addFailure<core::DiagnosticKind::LLVMImportedGlobalUnsupported>(GlobalValue.Name);
        return false;
      }
      if (GlobalValue.ValueType == nullptr)
      {
        addFailure<core::DiagnosticKind::LLVMGlobalMissingValueType>(GlobalValue.Name);
        return false;
      }
      ::llvm::Type *ValueType = lowerType(*GlobalValue.ValueType);
      if (ValueType == nullptr)
      {
        return false;
      }
      ::llvm::Constant *Initializer = ::llvm::Constant::getNullValue(ValueType);
      // Ink immutable globals remain writable during module initialization, so LLVM constness cannot model them directly.
      auto *Global = new ::llvm::GlobalVariable(*TargetModule, ValueType, false, ::llvm::GlobalValue::ExternalLinkage, Initializer, GlobalValue.Name.str());
      const std::optional<ir::TypeLayout> Layout = ir::computeTypeLayout(*GlobalValue.ValueType, SourceModule.context().compilationContext().targetContext());
      if (Layout.has_value())
      {
        Global->setAlignment(::llvm::Align(Layout->Alignment));
      }
      Globals.push_back(Global);
    }
    return true;
  }

  ::llvm::Constant *LoweringContext::lowerConstant(const ir::Constant &ConstantValue)
  {
    ::llvm::Type *ValueType = lowerType(ConstantValue.type());
    if (ValueType == nullptr)
    {
      return nullptr;
    }
    switch (ConstantValue.kind())
    {
    case ir::ValueKind::IntegerConstant:
    {
      const ir::IntegerConstant &Integer = static_cast<const ir::IntegerConstant &>(ConstantValue);
      return ::llvm::ConstantInt::get(static_cast<::llvm::IntegerType *>(ValueType), Integer.unsignedValue());
    }
    case ir::ValueKind::FloatConstant:
      return ::llvm::ConstantFP::get(Context, floatingPointValue(static_cast<const ir::FloatConstant &>(ConstantValue)));
    case ir::ValueKind::StringConstant:
    {
      const ir::StringConstant &String = static_cast<const ir::StringConstant &>(ConstantValue);
      const auto Existing = StringConstants.find(&String);
      if (Existing != StringConstants.end())
      {
        return Existing->second;
      }
      ::llvm::Constant *Initializer = ::llvm::ConstantDataArray::getString(Context, String.data(), false);
      const std::string Name = ".ink.string." + std::to_string(StringConstants.size());
      auto *Global = new ::llvm::GlobalVariable(*TargetModule, Initializer->getType(), true, ::llvm::GlobalValue::PrivateLinkage, Initializer, Name);
      Global->setAlignment(::llvm::Align(1));
      ::llvm::Constant *Zero = ::llvm::ConstantInt::get(::llvm::Type::getInt32Ty(Context), 0);
      const std::vector<::llvm::Constant *> Indices{Zero, Zero};
      ::llvm::Constant *Data = ::llvm::ConstantExpr::getInBoundsGetElementPtr(Initializer->getType(), Global, Indices);
      ::llvm::Type *LengthType = lowerType(SourceModule.context().getType(ir::TypeKind::PointerSize));
      if (LengthType == nullptr)
      {
        return nullptr;
      }
      ::llvm::Constant *Length = ::llvm::ConstantInt::get(static_cast<::llvm::IntegerType *>(LengthType), String.data().size());
      ::llvm::Constant *Slice = ::llvm::ConstantStruct::get(static_cast<::llvm::StructType *>(ValueType), {Data, Length});
      StringConstants.emplace(&String, Slice);
      return Slice;
    }
    case ir::ValueKind::NullConstant:
      return ::llvm::ConstantPointerNull::get(static_cast<::llvm::PointerType *>(ValueType));
    case ir::ValueKind::ZeroInitializer:
      return ::llvm::Constant::getNullValue(ValueType);
    case ir::ValueKind::AggregateConstant:
    {
      const ir::AggregateConstant &Aggregate = static_cast<const ir::AggregateConstant &>(ConstantValue);
      const ir::StructType &SourceType = static_cast<const ir::StructType &>(Aggregate.type());
      auto *TargetType = static_cast<::llvm::StructType *>(ValueType);
      const auto StructIterator = StructTypes.find(&SourceType);
      if (StructIterator == StructTypes.end())
      {
        addFailure<core::DiagnosticKind::LLVMUndeclaredAggregateConstantType>(SourceType.name());
        return nullptr;
      }
      ::llvm::StructType *ElementContainerType = StructIterator->second.PayloadType == nullptr ? TargetType : StructIterator->second.PayloadType;
      std::vector<::llvm::Constant *> Elements;
      Elements.reserve(ElementContainerType->getNumElements());
      for (::llvm::Type *ElementType : ElementContainerType->elements())
      {
        Elements.push_back(::llvm::Constant::getNullValue(ElementType));
      }
      for (std::size_t ElementIndex = 0; ElementIndex < Aggregate.elements().size(); ++ElementIndex)
      {
        ::llvm::Constant *Element = lowerConstant(Aggregate.elements()[ElementIndex].get());
        if (Element == nullptr)
        {
          return nullptr;
        }
        const std::vector<unsigned> *PhysicalIndices = physicalFieldIndices(SourceType, ElementIndex);
        if (PhysicalIndices == nullptr)
        {
          return nullptr;
        }
        Elements[PhysicalIndices->back()] = Element;
      }
      ::llvm::Constant *Container = ::llvm::ConstantStruct::get(ElementContainerType, Elements);
      if (StructIterator->second.PayloadType == nullptr)
      {
        return Container;
      }
      return ::llvm::ConstantStruct::get(TargetType, {Container, ::llvm::Constant::getNullValue(TargetType->getElementType(1))});
    }
    case ir::ValueKind::ValueOperand:
    case ir::ValueKind::GlobalAddressOperand:
    case ir::ValueKind::GlobalVariableAddressOperand:
      addFailure<core::DiagnosticKind::LLVMNonConstantValueInConstantLowering>();
      return nullptr;
    }
    addFailure<core::DiagnosticKind::LLVMUnknownConstantKind>();
    return nullptr;
  }

  ::llvm::Constant *LoweringContext::lowerGlobalAddress(const ir::GlobalAddressOperand &Address)
  {
    if (!Address.byteConstant().valid() || Address.byteConstant().value() >= ByteConstants.size())
    {
      addFailure<core::DiagnosticKind::LLVMInvalidByteConstantAddress>(Address.byteConstant().value());
      return nullptr;
    }
    ::llvm::GlobalVariable *Global = ByteConstants[Address.byteConstant().value()];
    ::llvm::Constant *Zero = ::llvm::ConstantInt::get(::llvm::Type::getInt32Ty(Context), 0);
    ::llvm::Type *IndexType = lowerType(SourceModule.context().getType(ir::TypeKind::PointerSize));
    if (IndexType == nullptr)
    {
      return nullptr;
    }
    ::llvm::Constant *Offset = ::llvm::ConstantInt::get(static_cast<::llvm::IntegerType *>(IndexType), Address.byteOffset());
    const std::vector<::llvm::Constant *> Indices{Zero, Offset};
    return ::llvm::ConstantExpr::getInBoundsGetElementPtr(Global->getValueType(), Global, Indices);
  }

  ::llvm::Constant *LoweringContext::lowerGlobalVariableAddress(const ir::GlobalVariableAddressOperand &Address)
  {
    if (!Address.global().valid() || Address.global().value() >= Globals.size())
    {
      addFailure<core::DiagnosticKind::LLVMInvalidGlobalVariableAddress>(Address.global().value());
      return nullptr;
    }
    return Globals[Address.global().value()];
  }
} // namespace ink::backend::llvm
