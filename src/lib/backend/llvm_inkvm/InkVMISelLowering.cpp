#include "InkVMISelLowering.h"

#include "InkVMRegisterInfo.h"
#include "InkVMSubtarget.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/SelectionDAG.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/Alignment.h>
#include <llvm/Support/ErrorHandling.h>

#include <algorithm>
#include <cassert>
#include <cstdint>

namespace llvm
{
    namespace
    {
        std::uint64_t computeSlotBytes(Type* Ty, const DataLayout& DL)
        {
            if (Ty->isVoidTy())
            {
                return 0;
            }
            const std::uint64_t ValueBytes = DL.getTypeAllocSize(Ty).getFixedValue();
            return alignTo(std::max<std::uint64_t>(8, ValueBytes), 8);
        }

        struct FunctionCallAreaLayout
        {
                std::uint64_t ReturnBytes = 0;
                std::uint64_t CallBytes = 0;
        };

        FunctionCallAreaLayout computeCallAreaLayout(const Function& Func, const DataLayout& DL)
        {
            FunctionCallAreaLayout Layout;
            Layout.ReturnBytes = computeSlotBytes(Func.getReturnType(), DL);
            std::uint64_t ArgsBytes = 0;
            for (const Argument& Arg : Func.args())
            {
                ArgsBytes += computeSlotBytes(Arg.getType(), DL);
            }
            Layout.CallBytes = Layout.ReturnBytes + ArgsBytes;
            return Layout;
        }

        int getOrCreateFixedSlot(MachineFrameInfo& MFI, std::uint64_t Size, std::int64_t Offset, bool IsImmutable)
        {
            for (int FI = MFI.getObjectIndexBegin(); FI < 0; ++FI)
            {
                if (MFI.getObjectOffset(FI) == Offset && static_cast<std::uint64_t>(MFI.getObjectSize(FI)) == Size)
                {
                    return FI;
                }
            }
            return MFI.CreateFixedObject(Size, Offset, IsImmutable);
        }

        SDValue emitSlotLoad(SelectionDAG& DAG, const SDValue& Chain, const SDLoc& DL, const SDValue& Addr, MVT RegVT, EVT MemVT, ISD::ArgFlagsTy Flags, MachinePointerInfo PtrInfo)
        {
            if (MemVT == MVT::i64)
            {
                return DAG.getLoad(RegVT, DL, Chain, Addr, PtrInfo);
            }
            EVT LoadVT = MemVT == MVT::i1 ? EVT(MVT::i8) : MemVT;
            const ISD::LoadExtType ExtType = Flags.isSExt() ? ISD::SEXTLOAD : (Flags.isZExt() ? ISD::ZEXTLOAD : ISD::EXTLOAD);
            return DAG.getExtLoad(ExtType, DL, RegVT, Chain, Addr, PtrInfo, LoadVT);
        }
    } // namespace

    InkVMTargetLowering::InkVMTargetLowering(const TargetMachine& TM, const InkVMSubtarget& Subtarget)
        : TargetLowering(TM, Subtarget)
    {
        addRegisterClass(MVT::i64, &InkVM::GPRRegClass);
        computeRegisterProperties(Subtarget.getRegisterInfo());

        setStackPointerRegisterToSaveRestore(InkVM::SP);

        // Symbol addresses lower through InkVMISD::WRAPPER into mov64ri dst, @sym [+ offset].
        setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);
        setOperationAction(ISD::ExternalSymbol, MVT::i64, Custom);

        // Expand operations that do not map to a single InkVM 64-bit register instruction.
        setOperationAction(ISD::SDIVREM, MVT::i64, Expand);
        setOperationAction(ISD::UDIVREM, MVT::i64, Expand);
        setOperationAction(ISD::SMUL_LOHI, MVT::i64, Expand);
        setOperationAction(ISD::UMUL_LOHI, MVT::i64, Expand);
        setOperationAction(ISD::MULHS, MVT::i64, Expand);
        setOperationAction(ISD::MULHU, MVT::i64, Expand);
        setOperationAction(ISD::ROTL, MVT::i64, Expand);
        setOperationAction(ISD::ROTR, MVT::i64, Expand);
        setOperationAction(ISD::BSWAP, MVT::i64, Expand);
        setOperationAction(ISD::BITREVERSE, MVT::i64, Expand);
        setOperationAction(ISD::CTPOP, MVT::i64, Expand);
        setOperationAction(ISD::CTLZ, MVT::i64, Expand);
        setOperationAction(ISD::CTTZ, MVT::i64, Expand);
        setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i64, Expand);
        setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i64, Expand);
        setOperationAction(ISD::SHL_PARTS, MVT::i64, Expand);
        setOperationAction(ISD::SRL_PARTS, MVT::i64, Expand);
        setOperationAction(ISD::SRA_PARTS, MVT::i64, Expand);

        // Narrow sign-extension in register expands into shl64rri + ashr64rri.
        setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1, Expand);
        setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i8, Expand);
        setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i16, Expand);
        setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i32, Expand);

        // InkVM's smallest memory access is 8 bits, so 1-bit loads/stores widen to i8.
        setLoadExtAction(ISD::EXTLOAD, MVT::i64, MVT::i1, Promote);
        setLoadExtAction(ISD::ZEXTLOAD, MVT::i64, MVT::i1, Promote);
        setLoadExtAction(ISD::SEXTLOAD, MVT::i64, MVT::i1, Promote);
        setTruncStoreAction(MVT::i64, MVT::i1, Expand);
    }

    const char* InkVMTargetLowering::getTargetNodeName(unsigned Opcode) const
    {
        switch (static_cast<InkVMISD::NodeType>(Opcode))
        {
        case InkVMISD::RET_GLUE:
            return "InkVMISD::RET_GLUE";
        case InkVMISD::CALL:
            return "InkVMISD::CALL";
        case InkVMISD::WRAPPER:
            return "InkVMISD::WRAPPER";
        default:
            return nullptr;
        }
    }

    MVT InkVMTargetLowering::getScalarShiftAmountTy(const DataLayout& DL, EVT ValueType) const
    {
        static_cast<void>(DL);
        static_cast<void>(ValueType);
        return MVT::i64;
    }

    SDValue InkVMTargetLowering::LowerOperation(SDValue Op, SelectionDAG& DAG) const
    {
        switch (Op.getOpcode())
        {
        case ISD::GlobalAddress:
            return lowerGlobalAddress(Op, DAG);
        case ISD::ExternalSymbol:
            return lowerExternalSymbol(Op, DAG);
        default:
            llvm_unreachable("Unhandled InkVM custom lowering operation");
        }
    }

    SDValue InkVMTargetLowering::lowerGlobalAddress(SDValue Op, SelectionDAG& DAG) const
    {
        const auto* GA = cast<GlobalAddressSDNode>(Op);
        const SDLoc DL(Op);
        const SDValue TargetGA = DAG.getTargetGlobalAddress(GA->getGlobal(), DL, MVT::i64, GA->getOffset());
        return DAG.getNode(InkVMISD::WRAPPER, DL, MVT::i64, TargetGA);
    }

    SDValue InkVMTargetLowering::lowerExternalSymbol(SDValue Op, SelectionDAG& DAG) const
    {
        const auto* ES = cast<ExternalSymbolSDNode>(Op);
        const SDLoc DL(Op);
        const SDValue TargetES = DAG.getTargetExternalSymbol(ES->getSymbol(), MVT::i64);
        return DAG.getNode(InkVMISD::WRAPPER, DL, MVT::i64, TargetES);
    }

    SDValue InkVMTargetLowering::LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::InputArg>& Ins, const SDLoc& DL, SelectionDAG& DAG, SmallVectorImpl<SDValue>& InVals) const
    {
        static_cast<void>(CallConv);

        if (IsVarArg)
        {
            report_fatal_error("InkVM variadic arguments are not implemented yet");
        }

        MachineFunction& MF = DAG.getMachineFunction();
        MachineFrameInfo& MFI = MF.getFrameInfo();
        const Function& Func = MF.getFunction();
        const DataLayout& LayoutInfo = DAG.getDataLayout();
        const FunctionCallAreaLayout CallArea = computeCallAreaLayout(Func, LayoutInfo);

        std::uint64_t RunningArgOffset = 0;
        for (const ISD::InputArg& Arg : Ins)
        {
            if (Arg.VT != MVT::i64)
            {
                report_fatal_error("InkVM Step 4 currently supports 64-bit scalar/promoted integer and pointer arguments");
            }
            const std::int64_t SlotOffset = -static_cast<std::int64_t>(CallArea.CallBytes) + static_cast<std::int64_t>(CallArea.ReturnBytes) + static_cast<std::int64_t>(RunningArgOffset);
            RunningArgOffset += 8;

            if (!Arg.Used)
            {
                InVals.push_back(DAG.getUNDEF(Arg.VT));
                continue;
            }

            const int FI = getOrCreateFixedSlot(MFI, 8, SlotOffset, true);
            const SDValue FINode = DAG.getFrameIndex(FI, MVT::i64);
            const SDValue Loaded = emitSlotLoad(DAG, Chain, DL, FINode, Arg.VT, Arg.ArgVT, Arg.Flags, MachinePointerInfo::getFixedStack(MF, FI));
            InVals.push_back(Loaded);
        }

        return Chain;
    }

    SDValue InkVMTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::OutputArg>& Outs, const SmallVectorImpl<SDValue>& OutVals, const SDLoc& DL, SelectionDAG& DAG) const
    {
        static_cast<void>(CallConv);
        static_cast<void>(IsVarArg);

        if (Outs.empty())
        {
            return DAG.getNode(InkVMISD::RET_GLUE, DL, MVT::Other, Chain);
        }

        if (Outs.size() != 1 || Outs[0].VT != MVT::i64)
        {
            report_fatal_error("InkVM Step 4 currently supports void or single scalar/pointer return values");
        }

        MachineFunction& MF = DAG.getMachineFunction();
        MachineFrameInfo& MFI = MF.getFrameInfo();
        const FunctionCallAreaLayout CallArea = computeCallAreaLayout(MF.getFunction(), DAG.getDataLayout());
        const std::int64_t ReturnOffset = -static_cast<std::int64_t>(CallArea.CallBytes);
        const int ReturnFI = getOrCreateFixedSlot(MFI, 8, ReturnOffset, false);
        const SDValue ReturnSlot = DAG.getFrameIndex(ReturnFI, MVT::i64);
        Chain = DAG.getStore(Chain, DL, OutVals[0], ReturnSlot, MachinePointerInfo::getFixedStack(MF, ReturnFI));

        return DAG.getNode(InkVMISD::RET_GLUE, DL, MVT::Other, Chain);
    }

    SDValue InkVMTargetLowering::LowerCall(CallLoweringInfo& CLI, SmallVectorImpl<SDValue>& InVals) const
    {
        SelectionDAG& DAG = CLI.DAG;
        const SDLoc& DL = CLI.DL;
        const SmallVectorImpl<ISD::OutputArg>& Outs = CLI.Outs;
        const SmallVectorImpl<SDValue>& OutVals = CLI.OutVals;
        const SmallVectorImpl<ISD::InputArg>& Ins = CLI.Ins;
        SDValue Chain = CLI.Chain;
        SDValue Callee = CLI.Callee;
        CLI.IsTailCall = false;

        if (CLI.IsVarArg)
        {
            report_fatal_error("InkVM variadic calls are not implemented yet");
        }
        if (Ins.size() > 1)
        {
            report_fatal_error("InkVM Step 4 currently supports void or single scalar/pointer call results");
        }

        const std::uint64_t ReturnBytes = Ins.empty() ? 0 : 8;
        const std::uint64_t ArgsBytes = static_cast<std::uint64_t>(Outs.size()) * 8;
        const std::uint64_t CallBytes = ReturnBytes + ArgsBytes;

        // 1. Reserve [CallBegin, sp) on the upward-growing stack via ADJCALLSTACKDOWN -> add64rri sp, sp, +CallBytes.
        Chain = DAG.getCALLSEQ_START(Chain, CallBytes, 0, DL);

        // 2. Write each argument into [sp - CallBytes + ReturnBytes + 8 * Index].
        const SDValue SPReg = DAG.getRegister(InkVM::SP, MVT::i64);
        SmallVector<SDValue, 8> ArgStoreChains;
        for (std::size_t Index = 0; Index < Outs.size(); ++Index)
        {
            if (Outs[Index].VT != MVT::i64)
            {
                report_fatal_error("InkVM Step 4 currently supports 64-bit scalar/promoted call arguments");
            }
            const std::int64_t OffsetFromSP = -static_cast<std::int64_t>(CallBytes) + static_cast<std::int64_t>(ReturnBytes) + static_cast<std::int64_t>(Index * 8);
            const SDValue ArgAddr = DAG.getNode(ISD::ADD, DL, MVT::i64, SPReg, DAG.getConstant(OffsetFromSP, DL, MVT::i64));
            ArgStoreChains.push_back(DAG.getStore(Chain, DL, OutVals[Index], ArgAddr, MachinePointerInfo()));
        }
        if (!ArgStoreChains.empty())
        {
            Chain = DAG.getNode(ISD::TokenFactor, DL, MVT::Other, ArgStoreChains);
        }

        // 3. Lower direct zero-offset symbol callees to target nodes so ISel picks CALLi; otherwise CALLr.
        if (const auto* GA = dyn_cast<GlobalAddressSDNode>(Callee); GA != nullptr && GA->getOffset() == 0)
        {
            Callee = DAG.getTargetGlobalAddress(GA->getGlobal(), DL, MVT::i64, 0);
        }
        else if (const auto* ES = dyn_cast<ExternalSymbolSDNode>(Callee))
        {
            Callee = DAG.getTargetExternalSymbol(ES->getSymbol(), MVT::i64);
        }

        MachineFunction& MF = DAG.getMachineFunction();
        const std::uint32_t* Mask = MF.getSubtarget<InkVMSubtarget>().getRegisterInfo()->getCallPreservedMask(MF, CLI.CallConv);
        assert(Mask != nullptr && "Missing InkVM call preserved register mask");

        SmallVector<SDValue, 4> CallOps;
        CallOps.push_back(Chain);
        CallOps.push_back(Callee);
        CallOps.push_back(DAG.getRegisterMask(Mask));

        const SDVTList CallResultTypes = DAG.getVTList(MVT::Other, MVT::Glue);
        Chain = DAG.getNode(InkVMISD::CALL, DL, CallResultTypes, CallOps);

        // 4. Load the return value from [sp - CallBytes] BEFORE ADJCALLSTACKUP reclaims the call area.
        if (!Ins.empty())
        {
            if (Ins[0].VT != MVT::i64)
            {
                report_fatal_error("InkVM Step 4 currently supports 64-bit scalar/promoted call results");
            }
            const SDValue ReturnAddr = DAG.getNode(ISD::ADD, DL, MVT::i64, SPReg, DAG.getConstant(-static_cast<std::int64_t>(CallBytes), DL, MVT::i64));
            const SDValue ReturnLoad = emitSlotLoad(DAG, Chain, DL, ReturnAddr, Ins[0].VT, Ins[0].ArgVT, Ins[0].Flags, MachinePointerInfo());
            InVals.push_back(ReturnLoad);
            Chain = ReturnLoad.getValue(1);
        }

        // 5. Reclaim the call area via ADJCALLSTACKUP -> add64rri sp, sp, -CallBytes.
        Chain = DAG.getCALLSEQ_END(Chain, CallBytes, 0, SDValue(), DL);
        return Chain;
    }
} // namespace llvm
