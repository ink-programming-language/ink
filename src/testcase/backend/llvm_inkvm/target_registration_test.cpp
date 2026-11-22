#include "ink/backend/llvm_inkvm/inkvm_target.h"
#include "ink/backend/vm/instruction/machine_instruction.h"
#include "ink/backend/vm/instruction/register.h"

#include <gtest/gtest.h>
#include <llvm/ADT/BitVector.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineModuleInfo.h>
#include <llvm/CodeGen/TargetFrameLowering.h>
#include <llvm/CodeGen/TargetInstrInfo.h>
#include <llvm/CodeGen/TargetOpcodes.h>
#include <llvm/CodeGen/TargetRegisterInfo.h>
#include <llvm/CodeGen/TargetSubtargetInfo.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCInstPrinter.h>
#include <llvm/MC/MCInstrDesc.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Triple.h>

// Include after LLVM headers because core_define.h defines a check() macro that collides with LLVM's PassManagerInternal.h.
#include "ink/backend/vm/instruction/instruction_encoding.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

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

        constexpr ExpectedRegisterSpec ExpectedRegisters[] = {
#define REGISTER(Kind, Encoding, Spelling) {vm::RegisterId::Kind, Spelling, !vm::isGeneralPurposeRegister(vm::RegisterId::Kind)},
#include "ink/backend/vm/instruction/register.def"
#undef REGISTER
        };

        struct VMOpcodeSpec
        {
                vm::Opcode OpcodeValue;
                std::string_view Spelling;
                std::size_t ByteSize;
        };

        constexpr VMOpcodeSpec VMOpcodeSpecs[] = {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) {vm::Opcode::Kind, Spelling, vm::InstructionEncoding::byteSize(vm::Opcode::Kind)},
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
        };

        std::optional<VMOpcodeSpec> findVMOpcodeSpec(std::uint16_t Encoding)
        {
            for (const VMOpcodeSpec& Spec : VMOpcodeSpecs)
            {
                if (static_cast<std::uint16_t>(Spec.OpcodeValue) == Encoding)
                {
                    return Spec;
                }
            }
            return std::nullopt;
        }

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

        std::string_view extractMnemonic(std::string_view PrintedInstruction)
        {
            const std::size_t FirstNonSpace = PrintedInstruction.find_first_not_of(" \t\r\n");
            if (FirstNonSpace == std::string_view::npos)
            {
                return {};
            }
            const std::string_view Trimmed = PrintedInstruction.substr(FirstNonSpace);
            const std::size_t FirstDelimiter = Trimmed.find_first_of(" \t\r\n");
            return FirstDelimiter == std::string_view::npos ? Trimmed : Trimmed.substr(0, FirstDelimiter);
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

    // Verifies that generated TableGen register definitions match Ink VM register.def encodings, spellings, and ABI reservation rules.
    TEST(InkVMTargetRegistrationTest, RegisterEncodingsAndReservationMatchInkVMABI)
    {
        std::string ErrorMessage;
        const auto TM = createInkVMTargetMachine(llvm::CodeGenOptLevel::Default, ErrorMessage);
        ASSERT_NE(TM, nullptr) << ErrorMessage;

        const llvm::MCAsmInfo* MAI = TM->getMCAsmInfo();
        const llvm::MCInstrInfo* MII = TM->getMCInstrInfo();
        const llvm::MCRegisterInfo* MRI = TM->getMCRegisterInfo();
        ASSERT_NE(MAI, nullptr);
        ASSERT_NE(MII, nullptr);
        ASSERT_NE(MRI, nullptr);

        std::unique_ptr<llvm::MCInstPrinter> InstPrinter(TM->getTarget().createMCInstPrinter(TM->getTargetTriple(), 0, *MAI, *MII, *MRI));
        ASSERT_NE(InstPrinter, nullptr);

        // Every register in register.def plus LLVM's synthetic NoRegister (index 0).
        EXPECT_EQ(MRI->getNumRegs(), std::size(ExpectedRegisters) + 1u);

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

            std::string PrintedRegName;
            llvm::raw_string_ostream RegOS(PrintedRegName);
            InstPrinter->printRegName(RegOS, Reg);
            EXPECT_EQ(PrintedRegName, Expected.Spelling) << "Register spelling mismatch for encoding " << static_cast<unsigned>(Encoding);
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
                EXPECT_TRUE(hasEncodedVMOpcode(MII->get(Opcode).TSFlags));
                EXPECT_EQ(decodeVMOpcodeFromTSFlags(MII->get(Opcode).TSFlags), static_cast<std::uint16_t>(vm::Opcode::Ret));
                break;
            }
        }
        EXPECT_TRUE(FoundRet);
    }

    // Verifies that every target-specific InkVM instruction matches opcode.def in wire encoding, byte size, and printed mnemonic.
    TEST(InkVMTargetRegistrationTest, InstructionMetadataMatchesInkVMOpcodeDef)
    {
        std::string ErrorMessage;
        const auto TM = createInkVMTargetMachine(llvm::CodeGenOptLevel::Default, ErrorMessage);
        ASSERT_NE(TM, nullptr) << ErrorMessage;

        const llvm::MCAsmInfo* MAI = TM->getMCAsmInfo();
        const llvm::MCInstrInfo* MII = TM->getMCInstrInfo();
        const llvm::MCRegisterInfo* MRI = TM->getMCRegisterInfo();
        const llvm::MCSubtargetInfo* STI = TM->getMCSubtargetInfo();
        ASSERT_NE(MAI, nullptr);
        ASSERT_NE(MII, nullptr);
        ASSERT_NE(MRI, nullptr);
        ASSERT_NE(STI, nullptr);

        std::unique_ptr<llvm::MCInstPrinter> InstPrinter(TM->getTarget().createMCInstPrinter(TM->getTargetTriple(), 0, *MAI, *MII, *MRI));
        ASSERT_NE(InstPrinter, nullptr);

        const llvm::MCRegister R0Reg = findRegisterByEncoding(*MRI, static_cast<std::uint8_t>(vm::RegisterId::R0));
        ASSERT_TRUE(R0Reg.isValid());

        std::size_t PseudoCount = 0;
        std::size_t RealInstructionCount = 0;
        std::unordered_map<std::string, unsigned> OpcodeByName;

        for (unsigned Opcode = 0; Opcode < MII->getNumOpcodes(); ++Opcode)
        {
            if (!llvm::isTargetSpecificOpcode(Opcode))
            {
                continue;
            }
            const llvm::StringRef Name = MII->getName(Opcode);
            OpcodeByName.emplace(Name.str(), Opcode);
            const llvm::MCInstrDesc& Desc = MII->get(Opcode);

            if (Desc.isPseudo())
            {
                ++PseudoCount;
                EXPECT_FALSE(hasEncodedVMOpcode(Desc.TSFlags)) << "Pseudo instruction " << Name.str() << " should not encode a VM opcode";
                EXPECT_EQ(Desc.getSize(), 0u) << "Pseudo instruction " << Name.str() << " should have zero byte size";
                continue;
            }

            ++RealInstructionCount;
            ASSERT_TRUE(hasEncodedVMOpcode(Desc.TSFlags)) << "Instruction " << Name.str() << " is missing generated VM opcode metadata";

            const std::uint16_t EncodedVMOpcode = decodeVMOpcodeFromTSFlags(Desc.TSFlags);
            const std::optional<VMOpcodeSpec> Spec = findVMOpcodeSpec(EncodedVMOpcode);
            ASSERT_TRUE(Spec.has_value()) << "Instruction " << Name.str() << " encodes unknown VM opcode 0x" << std::hex << EncodedVMOpcode;
            EXPECT_EQ(Desc.getSize(), Spec->ByteSize) << "Byte size mismatch for " << Name.str();

            llvm::MCInst DummyInst;
            DummyInst.setOpcode(Opcode);
            for (const llvm::MCOperandInfo& OpInfo : Desc.operands())
            {
                if (OpInfo.RegClass != -1 || OpInfo.OperandType == llvm::MCOI::OPERAND_REGISTER)
                {
                    DummyInst.addOperand(llvm::MCOperand::createReg(R0Reg));
                }
                else
                {
                    DummyInst.addOperand(llvm::MCOperand::createImm(0));
                }
            }

            std::string Printed;
            llvm::raw_string_ostream OS(Printed);
            InstPrinter->printInst(&DummyInst, 0, "", *STI, OS);
            EXPECT_EQ(extractMnemonic(Printed), Spec->Spelling) << "Mnemonic mismatch for " << Name.str() << " (printed: '" << Printed << "')";
        }

        EXPECT_GE(PseudoCount, 4u);
        EXPECT_GT(RealInstructionCount, 0u);

        const std::array<std::pair<std::string_view, vm::Opcode>, 15> SpotChecks = {{
            {"RET", vm::Opcode::Ret},
            {"JMP", vm::Opcode::Jmp},
            {"JMPR", vm::Opcode::Jmpr},
            {"CALLi", vm::Opcode::Calli},
            {"CALLr", vm::Opcode::Callr},
            {"ADD64rrr", vm::Opcode::Add64rrr},
            {"ADD64rri", vm::Opcode::Add64rri},
            {"MOV64ri", vm::Opcode::Mov64ri},
            {"MOV64r_sym", vm::Opcode::Mov64ri},
            {"MOV64ri_f64", vm::Opcode::Mov64ri},
            {"LOAD64rm", vm::Opcode::Load64rm},
            {"LOAD64rm_sym", vm::Opcode::Load64rm},
            {"LOAD64rm_f64_sym", vm::Opcode::Load64rm},
            {"FABS64rr", vm::Opcode::Fabs64rr},
            {"FCMP64OEQrrr", vm::Opcode::Fcmp64oeqrrr},
        }};

        for (const auto& [LLVMName, ExpectedVMOpcode] : SpotChecks)
        {
            const auto It = OpcodeByName.find(std::string(LLVMName));
            ASSERT_NE(It, OpcodeByName.end()) << "Missing InkVM instruction " << LLVMName;
            const llvm::MCInstrDesc& Desc = MII->get(It->second);
            EXPECT_EQ(decodeVMOpcodeFromTSFlags(Desc.TSFlags), static_cast<std::uint16_t>(ExpectedVMOpcode)) << "VM opcode mismatch for " << LLVMName;
        }
    }
} // namespace ink::backend::llvm_inkvm
