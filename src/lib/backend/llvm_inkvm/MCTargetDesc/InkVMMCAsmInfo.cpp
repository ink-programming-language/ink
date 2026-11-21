#include "MCTargetDesc/InkVMMCAsmInfo.h"

#include <llvm/TargetParser/Triple.h>

namespace llvm
{
    void InkVMMCAsmInfo::anchor()
    {
    }

    InkVMMCAsmInfo::InkVMMCAsmInfo(const Triple& TargetTriple)
    {
        static_cast<void>(TargetTriple);
        IsLittleEndian = true;
        CodePointerSize = 8;
        CalleeSaveStackSlotSize = 8;
        MinInstAlignment = 8;
        CommentString = ";";
        SupportsDebugInformation = false;
        ExceptionsType = ExceptionHandling::None;
    }
} // namespace llvm
