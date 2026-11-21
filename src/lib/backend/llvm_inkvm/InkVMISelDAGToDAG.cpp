#include "InkVM.h"
#include "InkVMISelLowering.h"
#include "InkVMTargetMachine.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/CodeGen/SelectionDAGISel.h>
#include <llvm/CodeGen/SelectionDAGNodes.h>
#include <llvm/Pass.h>

#include <memory>

#define DEBUG_TYPE "inkvm-isel"
#define PASS_NAME "InkVM DAG->DAG Pattern Instruction Selection"

using namespace llvm;

namespace
{
    class InkVMDAGToDAGISel : public SelectionDAGISel
    {
        public:
            InkVMDAGToDAGISel() = delete;

            InkVMDAGToDAGISel(InkVMTargetMachine& TM, CodeGenOptLevel OptLevel)
                : SelectionDAGISel(TM, OptLevel)
            {
            }

        private:
#include "InkVMGenDAGISel.inc"

            bool SelectAddrRI(SDValue Addr, SDValue& Base, SDValue& Disp)
            {
                const SDLoc DL(Addr);

                if (const auto* FIN = dyn_cast<FrameIndexSDNode>(Addr))
                {
                    Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i64);
                    Disp = CurDAG->getTargetConstant(0, DL, MVT::i64);
                    return true;
                }

                if (Addr.getOpcode() == ISD::ADD || (Addr.getOpcode() == ISD::OR && CurDAG->haveNoCommonBitsSet(Addr.getOperand(0), Addr.getOperand(1))))
                {
                    if (const auto* CN = dyn_cast<ConstantSDNode>(Addr.getOperand(1)))
                    {
                        const SDValue LHS = Addr.getOperand(0);
                        if (const auto* FIN = dyn_cast<FrameIndexSDNode>(LHS))
                        {
                            Base = CurDAG->getTargetFrameIndex(FIN->getIndex(), MVT::i64);
                        }
                        else
                        {
                            Base = LHS;
                        }
                        Disp = CurDAG->getTargetConstant(CN->getSExtValue(), DL, MVT::i64);
                        return true;
                    }
                }

                Base = Addr;
                Disp = CurDAG->getTargetConstant(0, DL, MVT::i64);
                return true;
            }

            void Select(SDNode* Node) override
            {
                if (Node->isMachineOpcode())
                {
                    Node->setNodeId(-1);
                    return;
                }

                if (Node->getOpcode() == ISD::FrameIndex)
                {
                    const SDLoc DL(Node);
                    const int FI = cast<FrameIndexSDNode>(Node)->getIndex();
                    const SDValue TFI = CurDAG->getTargetFrameIndex(FI, MVT::i64);
                    const SDValue ZeroDisp = CurDAG->getTargetConstant(0, DL, MVT::i64);
                    ReplaceNode(Node, CurDAG->getMachineNode(InkVM::ADD64rri, DL, MVT::i64, TFI, ZeroDisp));
                    return;
                }

                SelectCode(Node);
            }
    };

    class InkVMDAGToDAGISelLegacy : public SelectionDAGISelLegacy
    {
        public:
            static char ID;

            InkVMDAGToDAGISelLegacy(InkVMTargetMachine& TM, CodeGenOptLevel OptLevel)
                : SelectionDAGISelLegacy(ID, std::make_unique<InkVMDAGToDAGISel>(TM, OptLevel))
            {
            }
    };
} // namespace

char InkVMDAGToDAGISelLegacy::ID = 0;

INITIALIZE_PASS(InkVMDAGToDAGISelLegacy, DEBUG_TYPE, PASS_NAME, false, false)

namespace llvm
{
    FunctionPass* createInkVMISelDag(InkVMTargetMachine& TM, CodeGenOptLevel OptLevel)
    {
        return new InkVMDAGToDAGISelLegacy(TM, OptLevel);
    }
} // namespace llvm
