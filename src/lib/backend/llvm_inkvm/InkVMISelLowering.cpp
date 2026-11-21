#include "InkVMISelLowering.h"

#include "InkVMRegisterInfo.h"
#include "InkVMSubtarget.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/CodeGen/SelectionDAG.h>
#include <llvm/Support/ErrorHandling.h>

namespace llvm
{
    InkVMTargetLowering::InkVMTargetLowering(const TargetMachine& TM, const InkVMSubtarget& Subtarget)
        : TargetLowering(TM, Subtarget)
    {
        addRegisterClass(MVT::i64, &InkVM::GPRRegClass);
        computeRegisterProperties(Subtarget.getRegisterInfo());

        setStackPointerRegisterToSaveRestore(InkVM::SP);
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

    SDValue InkVMTargetLowering::LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::InputArg>& Ins, const SDLoc& DL, SelectionDAG& DAG, SmallVectorImpl<SDValue>& InVals) const
    {
        static_cast<void>(CallConv);
        static_cast<void>(DL);
        static_cast<void>(DAG);
        static_cast<void>(InVals);

        if (IsVarArg)
        {
            report_fatal_error("InkVM variadic arguments are not implemented yet");
        }
        if (!Ins.empty())
        {
            report_fatal_error("InkVM formal arguments will be implemented in Step 4");
        }

        return Chain;
    }

    SDValue InkVMTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::OutputArg>& Outs, const SmallVectorImpl<SDValue>& OutVals, const SDLoc& DL, SelectionDAG& DAG) const
    {
        static_cast<void>(CallConv);
        static_cast<void>(IsVarArg);
        static_cast<void>(OutVals);

        if (!Outs.empty())
        {
            report_fatal_error("InkVM non-void return values will be implemented in Step 4");
        }

        return DAG.getNode(InkVMISD::RET_GLUE, DL, MVT::Other, Chain);
    }
} // namespace llvm
