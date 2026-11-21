#include "InkVM.h"
#include "InkVMISelLowering.h"
#include "InkVMTargetMachine.h"
#include "MCTargetDesc/InkVMMCTargetDesc.h"

#include <llvm/CodeGen/SelectionDAGISel.h>
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

            void Select(SDNode* Node) override
            {
                if (Node->isMachineOpcode())
                {
                    Node->setNodeId(-1);
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
