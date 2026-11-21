#pragma once

#include <llvm/CodeGen/ISDOpcodes.h>
#include <llvm/CodeGen/TargetLowering.h>

namespace llvm
{
    namespace InkVMISD
    {
        enum NodeType : unsigned
        {
            FIRST_NUMBER = ISD::BUILTIN_OP_END,
            RET_GLUE,
        };
    } // namespace InkVMISD

    class InkVMSubtarget;

    class InkVMTargetLowering : public TargetLowering
    {
        public:
            explicit InkVMTargetLowering(const TargetMachine& TM, const InkVMSubtarget& Subtarget);

            const char* getTargetNodeName(unsigned Opcode) const override;

            SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::InputArg>& Ins, const SDLoc& DL, SelectionDAG& DAG, SmallVectorImpl<SDValue>& InVals) const override;

            SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::OutputArg>& Outs, const SmallVectorImpl<SDValue>& OutVals, const SDLoc& DL, SelectionDAG& DAG) const override;
    };
} // namespace llvm
