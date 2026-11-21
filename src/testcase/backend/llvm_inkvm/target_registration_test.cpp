#include "ink/backend/llvm_inkvm/inkvm_target.h"
#include "ink/backend/vm/instruction/register.h"

#include <gtest/gtest.h>
#include <llvm/ADT/BitVector.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/CodeGen/TargetFrameLowering.h>
#include <llvm/CodeGen/TargetInstrInfo.h>
#include <llvm/CodeGen/TargetRegisterInfo.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Triple.h>

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::backend::llvm_inkvm
{
    namespace
    {
        struct ExpectedRegisterSpec
        {
                vm::RegisterId Id;
                std::string_view Spelling;
                bool Reserved;
        };

        constexpr std::array<ExpectedRegisterSpec, 14> ExpectedRegisters = {{
            {vm::RegisterId::BP, "bp", true},
            {vm::RegisterId::SP, "sp", true},
            {vm::RegisterId::PC, "pc", true},
            {vm::RegisterId::ZERO, "zero", true},
            {vm::RegisterId::R0, "r0", false},
            {vm::RegisterId::R1, "r1", false},
            {vm::RegisterId::R2, "r2", false},
            {vm::RegisterId::R3, "r3", false},
            {vm::RegisterId::R4, "r4", false},
            {vm::RegisterId::R5, "r5", false},
            {vm::RegisterId::R6, "r6", false},
            {vm::RegisterId::R7, "r7", false},
            {vm::RegisterId::R8, "r8", false},
            {vm::RegisterId::R9, "r9", false},
        }};

        llvm::MCRegister findRegisterByEncoding(const llvm::MCRegisterInfo& RegInfo, std::uint8_t Encoding)
        {
            for (unsigned Reg = 1; Reg < RegInfo.getNumRegs(); ++Reg)
            {
                if (RegInfo.getEncodingValue(Reg) == Encoding)
                {
                    return Reg;
                }
            }
            return llvm::MCRegister::NoRegister;
        }
    } // namespace

    // Verifies that initializeInkVMTarget registers the out-of-tree "inkvm" target in LLVM's TargetRegistry.
    TEST(InkVMTargetRegistrationTest, RegistersTargetInTargetRegistry)
    {
        initializeInkVMTarget();

        llvm::Triple TargetTriple(llvm::StringRef(InkVMDefaultTriple.data(), InkVMDefaultTriple.size()));
        std::string ErrorMessage;
        const llvm::Target* Target = llvm::TargetRegistry::lookupTarget(llvm::StringRef(InkVMTargetName.data(), InkVMTargetName.size()), TargetTriple, ErrorMessage);

        ASSERT_NE(Target, nullptr) << ErrorMessage;
        EXPECT_TRUE(ErrorMessage.empty());
        EXPECT_EQ(std::string_view(Target->getName()), InkVMTargetName);
        EXPECT_EQ(std::string_view(Target->getBackendName()), "InkVM");
    }

    // Verifies that InkVMTargetMachine exposes the expected 64-bit little-endian DataLayout and MCAsmInfo settings.
    TEST(InkVMTargetRegistrationTest, CreatesTargetMachineWithExpectedDataLayoutAndAsmInfo)
    {
        std::string ErrorMessage;
        const auto TM = createInkVMTargetMachine(llvm::CodeGenOptLevel::Default, ErrorMessage);

        ASSERT_NE(TM, nullptr) << ErrorMessage;
        const llvm::DataLayout DL = TM->createDataLayout();
        EXPECT_TRUE(DL.isLittleEndian());
        EXPECT_EQ(DL.getPointerSize(), 8u);
        EXPECT_EQ(DL.getPointerSizeInBits(), 64u);
        EXPECT_EQ(DL.getStackAlignment().value(), 8u);

        const llvm::MCAsmInfo* MAI = TM->getMCAsmInfo();
        ASSERT_NE(MAI, nullptr);
        EXPECT_EQ(MAI->getCodePointerSize(), 8u);
        EXPECT_EQ(MAI->getMinInstAlignment(), 8u);
        EXPECT_EQ(MAI->getCommentString(), ";");
        EXPECT_EQ(MAI->getExceptionHandlingType(), llvm::ExceptionHandling::None);
    }

    // Verifies that TableGen register definitions match Ink VM register.def encodings and ABI reservation rules.
    TEST(InkVMTargetRegistrationTest, RegisterEncodingsAndReservationMatchInkVMABI)
    {
        std::string ErrorMessage;
        const auto TM = createInkVMTargetMachine(llvm::CodeGenOptLevel::Default, ErrorMessage);
        ASSERT_NE(TM, nullptr) << ErrorMessage;

        const llvm::MCRegisterInfo* MRI = TM->getMCRegisterInfo();
        ASSERT_NE(MRI, nullptr);
        // 14 architectural registers plus LLVM's synthetic NoRegister (index 0).
        EXPECT_EQ(MRI->getNumRegs(), 15u);

        llvm::LLVMContext Context;
        llvm::Module Mod("inkvm_reg_test", Context);
        Mod.setDataLayout(TM->createDataLayout());
        auto* FnType = llvm::FunctionType::get(llvm::Type::getVoidTy(Context), false);
        auto* Func = llvm::Function::Create(FnType, llvm::Function::ExternalLinkage, "empty", Mod);
        const llvm::TargetSubtargetInfo* STI = TM->getSubtargetImpl(*Func);
        ASSERT_NE(STI, nullptr);

        const llvm::TargetRegisterInfo* TRI = STI->getRegisterInfo();
        ASSERT_NE(TRI, nullptr);
        const llvm::MCPhysReg* CalleeSaved = TRI->getCalleeSavedRegs(nullptr);
        ASSERT_NE(CalleeSaved, nullptr);
        EXPECT_EQ(CalleeSaved[0], 0u);

        llvm::MachineModuleInfo MMI(TM.get());
        llvm::MachineFunction& MF = MMI.getOrCreateMachineFunction(*Func);
        const llvm::BitVector Reserved = TRI->getReservedRegs(MF);

        for (const ExpectedRegisterSpec& Expected : ExpectedRegisters)
        {
            const auto Encoding = static_cast<std::uint8_t>(Expected.Id);
            const llvm::MCRegister Reg = findRegisterByEncoding(*MRI, Encoding);
            ASSERT_TRUE(Reg.isValid()) << "Missing register with encoding " << static_cast<unsigned>(Encoding);
            EXPECT_EQ(Reserved.test(Reg.id()), Expected.Reserved) << "Unexpected reservation state for " << Expected.Spelling;
        }

        const llvm::MCRegister BPReg = findRegisterByEncoding(*MRI, static_cast<std::uint8_t>(vm::RegisterId::BP));
        EXPECT_EQ(TRI->getFrameRegister(MF), llvm::Register(BPReg));
    }

    // Verifies that InkVMFrameLowering matches the upward-growing stack and 16-byte BP save-area ABI, and RET exists.
    TEST(InkVMTargetRegistrationTest, FrameLoweringAndMinimalInstructionMatchInkVMDesign)
    {
        std::string ErrorMessage;
        const auto TM = createInkVMTargetMachine(llvm::CodeGenOptLevel::Default, ErrorMessage);
        ASSERT_NE(TM, nullptr) << ErrorMessage;

        llvm::LLVMContext Context;
        llvm::Module Mod("inkvm_frame_test", Context);
        Mod.setDataLayout(TM->createDataLayout());
        auto* FnType = llvm::FunctionType::get(llvm::Type::getVoidTy(Context), false);
        auto* Func = llvm::Function::Create(FnType, llvm::Function::ExternalLinkage, "empty", Mod);
        const llvm::TargetSubtargetInfo* STI = TM->getSubtargetImpl(*Func);
        ASSERT_NE(STI, nullptr);

        const llvm::TargetFrameLowering* TFI = STI->getFrameLowering();
        ASSERT_NE(TFI, nullptr);
        EXPECT_EQ(TFI->getStackGrowthDirection(), llvm::TargetFrameLowering::StackGrowsUp);
        EXPECT_EQ(TFI->getStackAlign().value(), 8u);
        EXPECT_EQ(TFI->getOffsetOfLocalArea(), 16);

        const llvm::MCInstrInfo* MII = TM->getMCInstrInfo();
        ASSERT_NE(MII, nullptr);
        bool FoundRet = false;
        for (unsigned Opcode = 0; Opcode < MII->getNumOpcodes(); ++Opcode)
        {
            if (MII->getName(Opcode) == "RET")
            {
                FoundRet = true;
                EXPECT_TRUE(MII->get(Opcode).isReturn());
                EXPECT_TRUE(MII->get(Opcode).isTerminator());
                EXPECT_EQ(MII->get(Opcode).getSize(), 8u);
                break;
            }
        }
        EXPECT_TRUE(FoundRet);
    }
} // namespace ink::backend::llvm_inkvm
