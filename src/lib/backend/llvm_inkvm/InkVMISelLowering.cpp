#include "InkVMISelLowering.h"

#include "InkVMRegisterInfo.h"
#include "InkVMSubtarget.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/SelectionDAG.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/Alignment.h>
#include <llvm/Support/ErrorHandling.h>

#include <algorithm>
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
    } // namespace

    InkVMTargetLowering::InkVMTargetLowering(const TargetMachine& TM, const InkVMSubtarget& Subtarget)
        : TargetLowering(TM, Subtarget)
    {
        addRegisterClass(MVT::i64, &InkVM::GPRRegClass);
        computeRegisterProperties(Subtarget.getRegisterInfo());

        setStackPointerRegisterToSaveRestore(InkVM::SP);

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
                report_fatal_error("InkVM Step 3 currently supports 64-bit integer/pointer arguments");
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
            const SDValue Loaded = DAG.getLoad(Arg.VT, DL, Chain, FINode, MachinePointerInfo::getFixedStack(MF, FI));
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
            report_fatal_error("InkVM Step 3 currently supports void or single 64-bit integer/pointer return values");
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
} // namespace llvm
