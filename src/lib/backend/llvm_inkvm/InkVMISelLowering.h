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
            CALL,
            WRAPPER,
            SELECT_CC,
            FP_CONST,
            BR_NZ,
        };
    } // namespace InkVMISD

    class InkVMSubtarget;

    class InkVMTargetLowering : public TargetLowering
    {
        public:
            explicit InkVMTargetLowering(const TargetMachine& TM, const InkVMSubtarget& Subtarget);

            const char* getTargetNodeName(unsigned Opcode) const override;

            MVT getScalarShiftAmountTy(const DataLayout& DL, EVT ValueType) const override;

            bool areJTsAllowed(const Function* Fn) const override;

            bool isFPImmLegal(const APFloat& Imm, EVT VT, bool ForCodeSize = false) const override;

            SDValue LowerOperation(SDValue Op, SelectionDAG& DAG) const override;

            MachineBasicBlock* EmitInstrWithCustomInserter(MachineInstr& MI, MachineBasicBlock* BB) const override;

            SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::InputArg>& Ins, const SDLoc& DL, SelectionDAG& DAG, SmallVectorImpl<SDValue>& InVals) const override;

            SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg, const SmallVectorImpl<ISD::OutputArg>& Outs, const SmallVectorImpl<SDValue>& OutVals, const SDLoc& DL, SelectionDAG& DAG) const override;

            SDValue LowerCall(CallLoweringInfo& CLI, SmallVectorImpl<SDValue>& InVals) const override;

        private:
            SDValue lowerGlobalAddress(SDValue Op, SelectionDAG& DAG) const;
            SDValue lowerExternalSymbol(SDValue Op, SelectionDAG& DAG) const;
            SDValue lowerConstantFP(SDValue Op, SelectionDAG& DAG) const;
            SDValue lowerBrCC(SDValue Op, SelectionDAG& DAG) const;
            SDValue lowerSelectCC(SDValue Op, SelectionDAG& DAG) const;
    };
} // namespace llvm
