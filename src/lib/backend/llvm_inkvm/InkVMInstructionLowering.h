#pragma once

#include <llvm/ADT/DenseMap.h>

namespace ink::vm
{
    class MachineBasicBlock;
} // namespace ink::vm

namespace llvm
{
    class InkVMMachineModuleBuilder;
    class MachineBasicBlock;
    class MachineFunction;

    bool lowerInkVMFunctionInstructions(const MachineFunction& MF, const DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*>& BlockMap, InkVMMachineModuleBuilder& ModuleBuilder);
} // namespace llvm
