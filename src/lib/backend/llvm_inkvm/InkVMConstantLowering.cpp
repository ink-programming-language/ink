#include "InkVMConstantLowering.h"

#include "InkVMMachineModuleEmitter.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalAlias.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instruction.h>
#include <llvm/IR/Operator.h>
#include <llvm/Support/Casting.h>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// Propagates a failed lowering sub-step (returns false; error already recorded on ModuleBuilder).
#define RETURN_IF_FAILED(Expression) \
    do                               \
    {                                \
        if (!(Expression))           \
        {                            \
            return false;            \
        }                            \
    } while (false)

// Records an emission error and returns failure (false) when Condition holds.
#define ERROR_IF(Condition, Message) \
    do                               \
    {                                \
        if (Condition)               \
        {                            \
            return error(Message);   \
        }                            \
    } while (false)

namespace llvm
{
    namespace
    {
        constexpr unsigned MaxConstantExprDepth = 32;

        std::string_view toStringView(StringRef Text) noexcept
        {
            return std::string_view(Text.data(), Text.size());
        }

        void appendAPIntLittleEndian(const APInt& Value, std::size_t ByteCount, std::vector<std::uint8_t>& OutBytes)
        {
            const unsigned BitWidth = Value.getBitWidth();
            for (std::size_t ByteIndex = 0; ByteIndex < ByteCount; ++ByteIndex)
            {
                const unsigned BitOffset = static_cast<unsigned>(ByteIndex * 8);
                if (BitOffset >= BitWidth)
                {
                    OutBytes.push_back(0);
                    continue;
                }
                const unsigned RemainingBits = std::min<unsigned>(8, BitWidth - BitOffset);
                const std::uint64_t ByteBits = Value.extractBitsAsZExtValue(RemainingBits, BitOffset);
                OutBytes.push_back(static_cast<std::uint8_t>(ByteBits));
            }
        }

        struct EvaluatedConstant64
        {
                const GlobalValue* Base = nullptr;
                std::int64_t Offset = 0;
                std::uint64_t RawBits = 0;
        };

        constexpr std::optional<std::int64_t> checkedSignedAdd(std::int64_t LHS, std::int64_t RHS) noexcept
        {
            if (RHS > 0 && LHS > std::numeric_limits<std::int64_t>::max() - RHS)
            {
                return std::nullopt;
            }
            if (RHS < 0 && LHS < std::numeric_limits<std::int64_t>::min() - RHS)
            {
                return std::nullopt;
            }
            return LHS + RHS;
        }

        constexpr std::optional<std::int64_t> checkedSignedSub(std::int64_t LHS, std::int64_t RHS) noexcept
        {
            if (RHS < 0 && LHS > std::numeric_limits<std::int64_t>::max() + RHS)
            {
                return std::nullopt;
            }
            if (RHS > 0 && LHS < std::numeric_limits<std::int64_t>::min() + RHS)
            {
                return std::nullopt;
            }
            return LHS - RHS;
        }

        class ConstantDataLowerer final
        {
            public:
                ConstantDataLowerer(const DataLayout& DL, InkVMMachineModuleBuilder& ModuleBuilder, std::vector<std::uint8_t>& OutBytes, std::vector<ink::vm::MachineGlobalRelocation>& OutRelocations) noexcept
                    : DL(DL),
                      ModuleBuilder(ModuleBuilder),
                      OutBytes(OutBytes),
                      OutRelocations(OutRelocations)
                {
                }

                bool lower(const Constant* ConstVal)
                {
                    return serializeConstant(ConstVal);
                }

            private:
                bool error(std::string Message)
                {
                    ModuleBuilder.fail(std::move(Message));
                    return false;
                }

                void padBytesTo(std::size_t TargetSize)
                {
                    if (OutBytes.size() < TargetSize)
                    {
                        OutBytes.resize(TargetSize, 0);
                    }
                }

                bool adjustSymbolicOffset(EvaluatedConstant64& Value, std::int64_t Delta, bool Subtract)
                {
                    const auto Combined = Subtract ? checkedSignedSub(Value.Offset, Delta) : checkedSignedAdd(Value.Offset, Delta);
                    ERROR_IF(!Combined, "InkVM global relocation addend overflows signed 64-bit range");
                    Value.Offset = *Combined;
                    return true;
                }

                bool evaluateGlobalValue(const GlobalValue& GV, EvaluatedConstant64& OutValue)
                {
                    if (const auto* Func = dyn_cast<Function>(&GV))
                    {
                        ERROR_IF(Func->isIntrinsic(), "InkVM global relocation cannot reference an LLVM intrinsic");
                    }
                    else if (const auto* GlobalVar = dyn_cast<GlobalVariable>(&GV))
                    {
                        ERROR_IF(isInkVMMetadataGlobal(*GlobalVar), "InkVM global relocation cannot reference an LLVM metadata global");
                    }
                    else
                    {
                        return error("InkVM global relocation only supports global variables and functions");
                    }
                    OutValue = EvaluatedConstant64{&GV, 0, 0};
                    return true;
                }

                bool evaluateGEP(const GEPOperator& GEP, EvaluatedConstant64& OutValue, unsigned Depth)
                {
                    const unsigned IndexBits = DL.getIndexTypeSizeInBits(GEP.getType());
                    ERROR_IF(IndexBits == 0 || IndexBits > 64, "InkVM global GEP index width exceeds 64 bits");
                    APInt Delta(IndexBits, 0);
                    ERROR_IF(!GEP.accumulateConstantOffset(DL, Delta), "InkVM global GEP initializer must have a constant byte offset");

                    EvaluatedConstant64 Base;
                    RETURN_IF_FAILED(evaluateScalarConstant64(cast<Constant>(GEP.getPointerOperand()), Base, Depth + 1));

                    const std::int64_t DeltaSigned = Delta.getSExtValue();
                    if (Base.Base == nullptr)
                    {
                        Base.RawBits += static_cast<std::uint64_t>(DeltaSigned);
                        OutValue = Base;
                        return true;
                    }
                    RETURN_IF_FAILED(adjustSymbolicOffset(Base, DeltaSigned, false));
                    OutValue = Base;
                    return true;
                }

                bool evaluateAdd(const ConstantExpr& CE, EvaluatedConstant64& OutValue, unsigned Depth)
                {
                    EvaluatedConstant64 LHS;
                    RETURN_IF_FAILED(evaluateScalarConstant64(CE.getOperand(0), LHS, Depth + 1));
                    EvaluatedConstant64 RHS;
                    RETURN_IF_FAILED(evaluateScalarConstant64(CE.getOperand(1), RHS, Depth + 1));

                    if (LHS.Base == nullptr && RHS.Base == nullptr)
                    {
                        OutValue = EvaluatedConstant64{nullptr, 0, LHS.RawBits + RHS.RawBits};
                        return true;
                    }
                    ERROR_IF(LHS.Base != nullptr && RHS.Base != nullptr, "InkVM global initializer cannot add two symbol addresses");

                    EvaluatedConstant64 Symbolic = LHS.Base != nullptr ? LHS : RHS;
                    const std::int64_t Delta = std::bit_cast<std::int64_t>(LHS.Base != nullptr ? RHS.RawBits : LHS.RawBits);
                    RETURN_IF_FAILED(adjustSymbolicOffset(Symbolic, Delta, false));
                    OutValue = Symbolic;
                    return true;
                }

                bool evaluateSub(const ConstantExpr& CE, EvaluatedConstant64& OutValue, unsigned Depth)
                {
                    EvaluatedConstant64 LHS;
                    RETURN_IF_FAILED(evaluateScalarConstant64(CE.getOperand(0), LHS, Depth + 1));
                    EvaluatedConstant64 RHS;
                    RETURN_IF_FAILED(evaluateScalarConstant64(CE.getOperand(1), RHS, Depth + 1));

                    if (LHS.Base == nullptr && RHS.Base == nullptr)
                    {
                        OutValue = EvaluatedConstant64{nullptr, 0, LHS.RawBits - RHS.RawBits};
                        return true;
                    }
                    ERROR_IF(LHS.Base == nullptr || RHS.Base != nullptr, "InkVM global initializer does not support subtracting a symbol address");

                    const std::int64_t Delta = std::bit_cast<std::int64_t>(RHS.RawBits);
                    RETURN_IF_FAILED(adjustSymbolicOffset(LHS, Delta, true));
                    OutValue = LHS;
                    return true;
                }

                bool evaluateConstantExpr(const ConstantExpr& CE, EvaluatedConstant64& OutValue, unsigned Depth)
                {
                    switch (CE.getOpcode())
                    {
                    case Instruction::BitCast:
                    case Instruction::AddrSpaceCast:
                    case Instruction::PtrToInt:
                        return evaluateScalarConstant64(CE.getOperand(0), OutValue, Depth + 1);
                    case Instruction::IntToPtr:
                        if (const auto* IntOp = dyn_cast<ConstantInt>(CE.getOperand(0)); IntOp != nullptr && IntOp->getBitWidth() <= 64)
                        {
                            OutValue = EvaluatedConstant64{nullptr, 0, IntOp->getZExtValue()};
                            return true;
                        }
                        return evaluateScalarConstant64(CE.getOperand(0), OutValue, Depth + 1);
                    case Instruction::GetElementPtr:
                        return evaluateGEP(*cast<GEPOperator>(&CE), OutValue, Depth);
                    case Instruction::Add:
                        return evaluateAdd(CE, OutValue, Depth);
                    case Instruction::Sub:
                        return evaluateSub(CE, OutValue, Depth);
                    default:
                        return error("InkVM global constant expression kind is not supported");
                    }
                }

                bool evaluateScalarConstant64(const Constant* ConstVal, EvaluatedConstant64& OutValue, unsigned Depth = 0)
                {
                    ERROR_IF(Depth > MaxConstantExprDepth, "InkVM global constant expression exceeds maximum nesting depth");
                    ERROR_IF(ConstVal == nullptr || !ConstVal->getType()->isSized(), "InkVM global initializer must have a sized type");
                    const TypeSize RawStoreSize = DL.getTypeStoreSize(ConstVal->getType());
                    ERROR_IF(RawStoreSize.isScalable() || RawStoreSize.getFixedValue() != ink::vm::MachineGlobalRelocation::ByteSize, "InkVM global symbol relocations require an 8-byte pointer or 64-bit integer slot");

                    if (isa<ConstantPointerNull>(ConstVal) || isa<UndefValue>(ConstVal) || isa<ConstantAggregateZero>(ConstVal))
                    {
                        OutValue = EvaluatedConstant64{nullptr, 0, 0};
                        return true;
                    }
                    if (const auto* IntConst = dyn_cast<ConstantInt>(ConstVal))
                    {
                        ERROR_IF(IntConst->getBitWidth() > 64, "InkVM global constant expression exceeds 64-bit integer width");
                        OutValue = EvaluatedConstant64{nullptr, 0, IntConst->getZExtValue()};
                        return true;
                    }
                    if (const auto* Alias = dyn_cast<GlobalAlias>(ConstVal))
                    {
                        const Constant* Aliasee = Alias->getAliasee();
                        ERROR_IF(Aliasee == nullptr, "InkVM global alias initializer has no target");
                        return evaluateScalarConstant64(Aliasee, OutValue, Depth + 1);
                    }
                    if (const auto* GV = dyn_cast<GlobalValue>(ConstVal))
                    {
                        return evaluateGlobalValue(*GV, OutValue);
                    }
                    if (const auto* CE = dyn_cast<ConstantExpr>(ConstVal))
                    {
                        return evaluateConstantExpr(*CE, OutValue, Depth);
                    }
                    return error("InkVM global initializer kind is not supported");
                }

                bool serializeDataSequential(const ConstantDataSequential& DataSeq)
                {
                    if (DataSeq.getElementByteSize() == 1)
                    {
                        const StringRef Raw = DataSeq.getRawDataValues();
                        OutBytes.insert(OutBytes.end(), Raw.bytes_begin(), Raw.bytes_end());
                        return true;
                    }
                    for (unsigned Index = 0; Index < DataSeq.getNumElements(); ++Index)
                    {
                        RETURN_IF_FAILED(serializeConstant(DataSeq.getElementAsConstant(Index)));
                    }
                    return true;
                }

                bool serializeArray(const ConstantArray& ArrayConst)
                {
                    const TypeSize RawElemAllocSize = DL.getTypeAllocSize(ArrayConst.getType()->getElementType());
                    ERROR_IF(RawElemAllocSize.isScalable(), "InkVM global initializer does not support scalable types");
                    const std::size_t ElemAllocSize = static_cast<std::size_t>(RawElemAllocSize.getFixedValue());
                    for (unsigned Index = 0; Index < ArrayConst.getNumOperands(); ++Index)
                    {
                        const std::size_t ElemStart = OutBytes.size();
                        RETURN_IF_FAILED(serializeConstant(ArrayConst.getOperand(Index)));
                        padBytesTo(ElemStart + ElemAllocSize);
                    }
                    return true;
                }

                bool serializeVector(const ConstantVector& VectorConst)
                {
                    for (unsigned Index = 0; Index < VectorConst.getNumOperands(); ++Index)
                    {
                        RETURN_IF_FAILED(serializeConstant(VectorConst.getOperand(Index)));
                    }
                    return true;
                }

                bool serializeStruct(const ConstantStruct& StructConst)
                {
                    const StructLayout* Layout = DL.getStructLayout(StructConst.getType());
                    const std::size_t BaseOffset = OutBytes.size();
                    for (unsigned FieldIndex = 0; FieldIndex < StructConst.getNumOperands(); ++FieldIndex)
                    {
                        padBytesTo(BaseOffset + static_cast<std::size_t>(Layout->getElementOffset(FieldIndex)));
                        RETURN_IF_FAILED(serializeConstant(StructConst.getOperand(FieldIndex)));
                    }
                    padBytesTo(BaseOffset + static_cast<std::size_t>(Layout->getSizeInBytes()));
                    return true;
                }

                bool serializeScalarAddressOrValue(const Constant* ConstVal)
                {
                    EvaluatedConstant64 Evaluated;
                    RETURN_IF_FAILED(evaluateScalarConstant64(ConstVal, Evaluated));
                    if (Evaluated.Base == nullptr)
                    {
                        appendAPIntLittleEndian(APInt(64, Evaluated.RawBits), ink::vm::MachineGlobalRelocation::ByteSize, OutBytes);
                        return true;
                    }
                    ERROR_IF(Evaluated.Base->getName().empty(), "InkVM global relocation target must have a nonempty symbol name");
                    const ink::core::Name SymbolName = ModuleBuilder.internName(toStringView(Evaluated.Base->getName()));
                    const auto FieldOffset = static_cast<std::uint64_t>(OutBytes.size());
                    OutBytes.insert(OutBytes.end(), ink::vm::MachineGlobalRelocation::ByteSize, 0);
                    OutRelocations.push_back({FieldOffset, SymbolName, Evaluated.Offset});
                    return true;
                }

                bool serializeConstant(const Constant* ConstVal)
                {
                    ERROR_IF(ConstVal == nullptr || !ConstVal->getType()->isSized(), "InkVM global initializer must have a sized type");
                    const TypeSize RawStoreSize = DL.getTypeStoreSize(ConstVal->getType());
                    ERROR_IF(RawStoreSize.isScalable(), "InkVM global initializer does not support scalable types");
                    const std::size_t StoreSize = static_cast<std::size_t>(RawStoreSize.getFixedValue());

                    if (isa<ConstantAggregateZero>(ConstVal) || isa<UndefValue>(ConstVal) || isa<ConstantPointerNull>(ConstVal))
                    {
                        OutBytes.insert(OutBytes.end(), StoreSize, 0);
                        return true;
                    }
                    if (const auto* IntConst = dyn_cast<ConstantInt>(ConstVal))
                    {
                        appendAPIntLittleEndian(IntConst->getValue(), StoreSize, OutBytes);
                        return true;
                    }
                    if (const auto* FPConst = dyn_cast<ConstantFP>(ConstVal))
                    {
                        appendAPIntLittleEndian(FPConst->getValueAPF().bitcastToAPInt(), StoreSize, OutBytes);
                        return true;
                    }
                    if (const auto* DataSeq = dyn_cast<ConstantDataSequential>(ConstVal))
                    {
                        return serializeDataSequential(*DataSeq);
                    }
                    if (const auto* ArrayConst = dyn_cast<ConstantArray>(ConstVal))
                    {
                        return serializeArray(*ArrayConst);
                    }
                    if (const auto* VectorConst = dyn_cast<ConstantVector>(ConstVal))
                    {
                        return serializeVector(*VectorConst);
                    }
                    if (const auto* StructConst = dyn_cast<ConstantStruct>(ConstVal))
                    {
                        return serializeStruct(*StructConst);
                    }
                    if (isa<GlobalValue>(ConstVal) || isa<ConstantExpr>(ConstVal))
                    {
                        return serializeScalarAddressOrValue(ConstVal);
                    }
                    return error("InkVM global initializer kind is not supported");
                }

                const DataLayout& DL;
                InkVMMachineModuleBuilder& ModuleBuilder;
                std::vector<std::uint8_t>& OutBytes;
                std::vector<ink::vm::MachineGlobalRelocation>& OutRelocations;
        };
    } // namespace

    bool isInkVMMetadataGlobal(const GlobalVariable& Global)
    {
        return Global.getSection() == "llvm.metadata" || Global.getName().starts_with("llvm.");
    }

    std::string_view formatInkVMGlobalDataError(ink::vm::MachineModuleDataError Error) noexcept
    {
        switch (Error)
        {
        case ink::vm::MachineModuleDataError::InvalidAlignment:
            return "InkVM global variable alignment must be a nonzero power of two";
        case ink::vm::MachineModuleDataError::InitializerTooLarge:
            return "InkVM global variable initializer exceeds its allocated byte size";
        case ink::vm::MachineModuleDataError::SizeOverflow:
            return "InkVM global variable initializer exceeds host storage limits";
        case ink::vm::MachineModuleDataError::InvalidRelocationSymbol:
            return "InkVM global relocation target must be a valid registered symbol name";
        case ink::vm::MachineModuleDataError::InvalidRelocationAlignment:
            return "InkVM global variable relocations require alignment >= 8 and an eight-byte aligned byte offset";
        case ink::vm::MachineModuleDataError::RelocationOutOfBounds:
            return "InkVM global relocation exceeds its allocated byte size";
        case ink::vm::MachineModuleDataError::RelocationOverlap:
            return "InkVM global relocations must not overlap";
        case ink::vm::MachineModuleDataError::RelocationNonZeroInitialBytes:
            return "InkVM global relocation field must have zero initial bytes";
        }
        return "InkVM global variable layout is invalid";
    }

    bool lowerInkVMConstantData(const Constant* Initializer, const DataLayout& DL, InkVMMachineModuleBuilder& ModuleBuilder, std::vector<std::uint8_t>& OutBytes, std::vector<ink::vm::MachineGlobalRelocation>& OutRelocations)
    {
        ConstantDataLowerer Lowerer(DL, ModuleBuilder, OutBytes, OutRelocations);
        return Lowerer.lower(Initializer);
    }
} // namespace llvm

#undef ERROR_IF
#undef RETURN_IF_FAILED
