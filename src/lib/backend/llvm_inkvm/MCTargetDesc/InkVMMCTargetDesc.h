#pragma once

#include <cstdint>

namespace llvm
{
    class Target;
} // namespace llvm

#define GET_REGINFO_ENUM
#include "InkVMGenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "InkVMGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "InkVMGenSubtargetInfo.inc"
