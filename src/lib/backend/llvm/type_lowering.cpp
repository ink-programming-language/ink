#include "lowering_context.h"

#include "ink/ir/analysis/type_layout.h"

#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

namespace ink::backend::llvm
{
  namespace
  {
    std::uint64_t fixedSize(::llvm::TypeSize Size)
    {
      return Size.getFixedValue();
    }
  } // namespace

  bool LoweringContext::declareStructTypes()
  {
    for (const ir::StructType *TypeValue : SourceModule.StructTypes)
    {
      if (TypeValue == nullptr)
      {
        addFailure<core::DiagnosticKind::LLVMNullStructTypeDeclaration>();
        return false;
      }
      StructLowering Lowering;
      Lowering.Type = ::llvm::StructType::create(Context, TypeValue->name().str());
      const auto Inserted = StructTypes.emplace(TypeValue, std::move(Lowering));
      Types.emplace(TypeValue, Inserted.first->second.Type);
    }
    for (const ir::StructType *TypeValue : SourceModule.StructTypes)
    {
      if (!defineStructType(*TypeValue))
      {
        return false;
      }
    }
    return true;
  }

  bool LoweringContext::defineStructType(const ir::StructType &TypeValue)
  {
    auto StructIterator = StructTypes.find(&TypeValue);
    if (StructIterator == StructTypes.end())
    {
      addFailure<core::DiagnosticKind::LLVMUndeclaredStructType>(TypeValue.name());
      return false;
    }
    StructLowering &Lowering = StructIterator->second;
    if (Lowering.Defined)
    {
      return true;
    }
    if (Lowering.Defining)
    {
      addFailure<core::DiagnosticKind::LLVMRecursiveByValueStructType>(TypeValue.name());
      return false;
    }
    Lowering.Defining = true;

    std::vector<::llvm::Type *> LogicalFields;
    LogicalFields.reserve(TypeValue.fieldCount());
    for (std::size_t FieldIndex = 0; FieldIndex < TypeValue.fieldCount(); ++FieldIndex)
    {
      const ir::StructField &Field = TypeValue.fields()[FieldIndex];
      if (Field.type() == nullptr)
      {
        addFailure<core::DiagnosticKind::LLVMStructFieldMissingType>(TypeValue.name(), FieldIndex);
        return false;
      }
      ::llvm::Type *FieldType = lowerType(*Field.type());
      if (FieldType == nullptr)
      {
        return false;
      }
      LogicalFields.push_back(FieldType);
    }

    const std::optional<ir::TypeLayout> InkLayout = ir::computeTypeLayout(TypeValue, SourceModule.context().compilationContext().targetContext());
    if (!InkLayout.has_value())
    {
      addFailure<core::DiagnosticKind::LLVMStructLayoutUnavailable>(TypeValue.name());
      return false;
    }
    ::llvm::StructType *NaturalType = ::llvm::StructType::get(Context, LogicalFields, false);
    const ::llvm::StructLayout *NaturalLayout = TargetModule->getDataLayout().getStructLayout(NaturalType);
    bool MatchesNaturalLayout = fixedSize(NaturalLayout->getSizeInBytes()) == InkLayout->StrideSize && NaturalLayout->getAlignment().value() == InkLayout->Alignment;
    for (std::size_t FieldIndex = 0; MatchesNaturalLayout && FieldIndex < InkLayout->FieldOffsets.size(); ++FieldIndex)
    {
      MatchesNaturalLayout = fixedSize(NaturalLayout->getElementOffset(static_cast<unsigned>(FieldIndex))) == InkLayout->FieldOffsets[FieldIndex];
    }

    if (MatchesNaturalLayout)
    {
      Lowering.FieldIndices.reserve(LogicalFields.size());
      for (std::size_t FieldIndex = 0; FieldIndex < LogicalFields.size(); ++FieldIndex)
      {
        Lowering.FieldIndices.push_back({static_cast<unsigned>(FieldIndex)});
      }
      Lowering.Type->setBody(LogicalFields, false);
      Lowering.Defining = false;
      Lowering.Defined = true;
      return true;
    }

    std::vector<::llvm::Type *> PhysicalFields;
    std::vector<unsigned> PhysicalFieldIndices;
    std::uint64_t CurrentOffset = 0;
    PhysicalFieldIndices.reserve(LogicalFields.size());
    for (std::size_t FieldIndex = 0; FieldIndex < LogicalFields.size(); ++FieldIndex)
    {
      const std::uint64_t DesiredOffset = InkLayout->FieldOffsets[FieldIndex];
      if (CurrentOffset > DesiredOffset)
      {
        addFailure<core::DiagnosticKind::LLVMStructLayoutUnrepresentable>(TypeValue.name());
        return false;
      }
      if (CurrentOffset < DesiredOffset)
      {
        PhysicalFields.push_back(::llvm::ArrayType::get(::llvm::Type::getInt8Ty(Context), DesiredOffset - CurrentOffset));
        CurrentOffset = DesiredOffset;
      }
      PhysicalFieldIndices.push_back(static_cast<unsigned>(PhysicalFields.size()));
      PhysicalFields.push_back(LogicalFields[FieldIndex]);
      CurrentOffset += fixedSize(TargetModule->getDataLayout().getTypeAllocSize(LogicalFields[FieldIndex]));
    }
    if (CurrentOffset > InkLayout->StrideSize)
    {
      addFailure<core::DiagnosticKind::LLVMStructStrideUnrepresentable>(TypeValue.name());
      return false;
    }
    if (CurrentOffset < InkLayout->StrideSize)
    {
      PhysicalFields.push_back(::llvm::ArrayType::get(::llvm::Type::getInt8Ty(Context), InkLayout->StrideSize - CurrentOffset));
    }

    Lowering.FieldIndices.reserve(PhysicalFieldIndices.size());
    if (InkLayout->Alignment == 1)
    {
      Lowering.Type->setBody(PhysicalFields, true);
      for (const unsigned FieldIndex : PhysicalFieldIndices)
      {
        Lowering.FieldIndices.push_back({FieldIndex});
      }
    }
    else
    {
      ::llvm::Type *AlignmentCarrier = alignmentCarrierType(InkLayout->Alignment);
      if (AlignmentCarrier == nullptr)
      {
        return false;
      }
      Lowering.PayloadType = ::llvm::StructType::get(Context, PhysicalFields, true);
      ::llvm::ArrayType *AlignmentMarker = ::llvm::ArrayType::get(AlignmentCarrier, 0);
      Lowering.Type->setBody({Lowering.PayloadType, AlignmentMarker}, false);
      for (const unsigned FieldIndex : PhysicalFieldIndices)
      {
        Lowering.FieldIndices.push_back({0, FieldIndex});
      }
    }

    const ::llvm::StructLayout *RepresentedLayout = TargetModule->getDataLayout().getStructLayout(Lowering.Type);
    if (fixedSize(RepresentedLayout->getSizeInBytes()) != InkLayout->StrideSize || RepresentedLayout->getAlignment().value() != InkLayout->Alignment)
    {
      addFailure<core::DiagnosticKind::LLVMStructAlignmentUnpreserved>(TypeValue.name());
      return false;
    }
    Lowering.Defining = false;
    Lowering.Defined = true;
    return true;
  }

  ::llvm::Type *LoweringContext::lowerType(const ir::Type &TypeValue)
  {
    const auto Existing = Types.find(&TypeValue);
    if (Existing != Types.end())
    {
      if (TypeValue.kind() == ir::TypeKind::Struct && !defineStructType(static_cast<const ir::StructType &>(TypeValue)))
      {
        return nullptr;
      }
      return Existing->second;
    }

    ::llvm::Type *Result = nullptr;
    switch (TypeValue.kind())
    {
    case ir::TypeKind::Void:
      Result = ::llvm::Type::getVoidTy(Context);
      break;
    case ir::TypeKind::Bool:
      Result = ::llvm::Type::getInt1Ty(Context);
      break;
    case ir::TypeKind::Byte:
      Result = ::llvm::Type::getInt8Ty(Context);
      break;
    case ir::TypeKind::I32:
      Result = ::llvm::Type::getInt32Ty(Context);
      break;
    case ir::TypeKind::PointerSize:
      Result = ::llvm::IntegerType::get(Context, static_cast<unsigned>(SourceModule.context().compilationContext().targetContext().pointerWidth()));
      break;
    case ir::TypeKind::BytePointer:
    case ir::TypeKind::ConstBytePointer:
      Result = ::llvm::PointerType::getUnqual(Context);
      break;
    case ir::TypeKind::ByteSlice:
    case ir::TypeKind::ConstByteSlice:
    {
      const ir::Type &PointerSizeType = SourceModule.context().getType(ir::TypeKind::PointerSize);
      ::llvm::Type *LengthType = lowerType(PointerSizeType);
      if (LengthType == nullptr)
      {
        return nullptr;
      }
      Result = ::llvm::StructType::get(Context, {::llvm::PointerType::getUnqual(Context), LengthType}, false);
      Types.emplace(&SourceModule.context().getType(ir::TypeKind::ByteSlice), Result);
      Types.emplace(&SourceModule.context().getType(ir::TypeKind::ConstByteSlice), Result);
      break;
    }
    case ir::TypeKind::F16:
      Result = ::llvm::Type::getHalfTy(Context);
      break;
    case ir::TypeKind::F32:
      Result = ::llvm::Type::getFloatTy(Context);
      break;
    case ir::TypeKind::F64:
      Result = ::llvm::Type::getDoubleTy(Context);
      break;
    case ir::TypeKind::Struct:
    {
      const ir::StructType &Struct = static_cast<const ir::StructType &>(TypeValue);
      const auto StructIterator = StructTypes.find(&Struct);
      if (StructIterator == StructTypes.end())
      {
        addFailure<core::DiagnosticKind::LLVMUndeclaredStructType>(Struct.name());
        return nullptr;
      }
      if (!defineStructType(Struct))
      {
        return nullptr;
      }
      Result = StructIterator->second.Type;
      break;
    }
    case ir::TypeKind::Count:
      addFailure<core::DiagnosticKind::LLVMUnknownTypeKind>();
      return nullptr;
    }
    Types.emplace(&TypeValue, Result);
    return Result;
  }

  ::llvm::Type *LoweringContext::alignmentCarrierType(std::size_t Alignment)
  {
    switch (Alignment)
    {
    case 1:
      return ::llvm::Type::getInt8Ty(Context);
    case 2:
      return ::llvm::Type::getInt16Ty(Context);
    case 4:
      return ::llvm::Type::getInt32Ty(Context);
    case 8:
      return ::llvm::Type::getInt64Ty(Context);
    default:
      break;
    }
    if (Alignment > static_cast<std::size_t>(std::numeric_limits<unsigned>::max()) / 8U)
    {
      addFailure<core::DiagnosticKind::LLVMStructAlignmentUnrepresentable>(Alignment);
      return nullptr;
    }
    ::llvm::Type *Carrier = ::llvm::FixedVectorType::get(::llvm::Type::getInt8Ty(Context), static_cast<unsigned>(Alignment));
    if (TargetModule->getDataLayout().getABITypeAlign(Carrier).value() != Alignment)
    {
      addFailure<core::DiagnosticKind::LLVMStructAlignmentMismatch>(Alignment);
      return nullptr;
    }
    return Carrier;
  }

  const std::vector<unsigned> *LoweringContext::physicalFieldIndices(const ir::StructType &TypeValue, std::size_t LogicalIndex)
  {
    const auto StructIterator = StructTypes.find(&TypeValue);
    if (StructIterator == StructTypes.end() || !StructIterator->second.Defined || LogicalIndex >= StructIterator->second.FieldIndices.size())
    {
      addFailure<core::DiagnosticKind::LLVMInvalidStructFieldIndex>(TypeValue.name(), LogicalIndex);
      return nullptr;
    }
    return &StructIterator->second.FieldIndices[LogicalIndex];
  }
} // namespace ink::backend::llvm
