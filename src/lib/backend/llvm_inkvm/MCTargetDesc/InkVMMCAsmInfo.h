#pragma once

#include <llvm/MC/MCAsmInfoELF.h>

namespace llvm
{
    class Triple;

    class InkVMMCAsmInfo : public MCAsmInfoELF
    {
        public:
            explicit InkVMMCAsmInfo(const Triple& TargetTriple);

        private:
            void anchor() override;
    };
} // namespace llvm
