#include "InkVMMachineModuleEmitter.h"

#include "InkVM.h"
#include "InkVMConstantLowering.h"
#include "InkVMInstructionLowering.h"

#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/CodeGen/MachineBasicBlock.h>
#include <llvm/CodeGen/MachineFunction.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Module.h>
#include <llvm/Pass.h>

#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#define DEBUG_TYPE "inkvm-machine-module-emitter"
#define PASS_NAME "InkVM MachineModule Emitter"

// Propagates a failed emission sub-step (returns false; error already recorded on this builder).
#define RETURN_IF_FAILED(Expression) \
    do                               \
    {                                \
        if (!(Expression))           \
        {                            \
            return false;            \
        }                            \
    } while (false)

// Records an emission error and returns failure (false) when Condition holds.
#define ERROR_IF(Condition, Message) \
    do                               \
    {                                \
        if (Condition)               \
        {                            \
            return error(Message);   \
        }                            \
    } while (false)

using namespace llvm;

namespace
{
    std::string_view toStringView(StringRef Text) noexcept
    {
        return std::string_view(Text.data(), Text.size());
    }

    void trimTrailingZeroBytes(std::vector<std::uint8_t>& Bytes) noexcept
    {
        while (!Bytes.empty() && Bytes.back() == 0)
        {
            Bytes.pop_back();
        }
    }

    DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*> createFunctionBlocks(const MachineFunction& MF, ink::vm::MachineFunction& VMFunc, InkVMMachineModuleBuilder& ModuleBuilder)
    {
        DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*> BlockMap;
        BlockMap.reserve(MF.size());
        for (const MachineBasicBlock& MBB : MF)
        {
            ink::core::Name BlockName = ink::core::NAME_None;
            if (const BasicBlock* IRBlock = MBB.getBasicBlock(); IRBlock != nullptr && !IRBlock->getName().empty())
            {
                BlockName = ModuleBuilder.internName(toStringView(IRBlock->getName()));
            }
            ink::vm::MachineBasicBlock& VMBlock = VMFunc.createBasicBlock(BlockName);
            BlockMap.try_emplace(&MBB, &VMBlock);
        }
        return BlockMap;
    }

    class InkVMMachineModuleEmitter final : public MachineFunctionPass
    {
        public:
            static char ID;

            explicit InkVMMachineModuleEmitter(InkVMMachineModuleEmitSink& Sink)
                : MachineFunctionPass(ID),
                  Sink(Sink)
            {
            }

            StringRef getPassName() const override
            {
                return PASS_NAME;
            }

            void getAnalysisUsage(AnalysisUsage& AU) const override
            {
                AU.setPreservesAll();
                MachineFunctionPass::getAnalysisUsage(AU);
            }

            bool doInitialization(Module& M) override
            {
                Sink.Output.reset();
                Sink.ErrorMessage.clear();
                Sink.Failed = false;
                Builder.initialize(M);
                return false;
            }

            bool runOnMachineFunction(MachineFunction& MF) override
            {
                if (!Builder.hasFailed())
                {
                    Builder.emitFunction(MF);
                }
                return false;
            }

            bool doFinalization(Module& M) override
            {
                static_cast<void>(M);
                auto Result = Builder.takeModule();
                if (Result)
                {
                    Sink.Output = std::move(*Result);
                    Sink.ErrorMessage.clear();
                    Sink.Failed = false;
                }
                else
                {
                    Sink.Output.reset();
                    Sink.ErrorMessage = std::move(Result.error());
                    Sink.Failed = true;
                }
                return false;
            }

        private:
            InkVMMachineModuleEmitSink& Sink;
            InkVMMachineModuleBuilder Builder;
    };
} // namespace

char InkVMMachineModuleEmitter::ID = 0;

INITIALIZE_PASS(InkVMMachineModuleEmitter, DEBUG_TYPE, PASS_NAME, false, false)

namespace llvm
{
    void InkVMMachineModuleBuilder::fail(std::string Message)
    {
        if (!Failed)
        {
            Failed = true;
            ErrorMessage = std::move(Message);
        }
    }

    bool InkVMMachineModuleBuilder::error(std::string Message)
    {
        fail(std::move(Message));
        return false;
    }

    ink::core::Name InkVMMachineModuleBuilder::internName(std::string_view Text)
    {
        const ink::core::Name Interned = ink::core::NamePool::instance().intern(Text);
        checkMsgf(Text.empty() || Interned.valid(), "InkVM name pool capacity exhausted");
        return Interned;
    }

    bool InkVMMachineModuleBuilder::recordExternalSymbolDeclaration(ink::core::Name SymbolName)
    {
        ERROR_IF(Output == nullptr, "InkVM MachineModule emitter was not initialized");
        if (SymbolName.valid() && KnownSymbols.insert(SymbolName).second)
        {
            Output->declareSymbol(SymbolName, ink::vm::MachineSymbolKind::Function, false);
        }
        return true;
    }

    bool InkVMMachineModuleBuilder::initialize(const Module& IRModule)
    {
        Output.reset();
        KnownSymbols.clear();
        ErrorMessage.clear();
        Failed = false;

        const StringRef RawModuleName = IRModule.getName().empty() ? StringRef("inkvm") : StringRef(IRModule.getName());
        const ink::core::Name ModuleName = internName(toStringView(RawModuleName));
        Output = std::make_unique<ink::vm::MachineModule>(ModuleName);

        RETURN_IF_FAILED(emitExternalDeclarations(IRModule));
        RETURN_IF_FAILED(emitDataGlobals(IRModule));
        recordDefinedFunctionSymbols(IRModule);
        return true;
    }

    bool InkVMMachineModuleBuilder::emitExternalDeclarations(const Module& IRModule)
    {
        for (const GlobalVariable& Global : IRModule.globals())
        {
            if (!Global.isDeclaration() || isInkVMMetadataGlobal(Global))
            {
                continue;
            }
            ERROR_IF(Global.getName().empty(), "InkVM external global declaration requires a nonempty name");
            const ink::core::Name GlobalName = internName(toStringView(Global.getName()));
            Output->declareSymbol(GlobalName, ink::vm::MachineSymbolKind::GlobalVariable, Global.isConstant());
            KnownSymbols.insert(GlobalName);
        }

        for (const Function& Func : IRModule.functions())
        {
            if (!Func.isDeclaration() || Func.isIntrinsic())
            {
                continue;
            }
            ERROR_IF(Func.getName().empty(), "InkVM external function declaration requires a nonempty name");
            const ink::core::Name FuncName = internName(toStringView(Func.getName()));
            Output->declareSymbol(FuncName, ink::vm::MachineSymbolKind::Function, false);
            KnownSymbols.insert(FuncName);
        }
        return true;
    }

    bool InkVMMachineModuleBuilder::emitDataGlobals(const Module& IRModule)
    {
        const DataLayout& DL = IRModule.getDataLayout();
        for (const GlobalVariable& Global : IRModule.globals())
        {
            if (Global.isDeclaration() || isInkVMMetadataGlobal(Global))
            {
                continue;
            }
            RETURN_IF_FAILED(emitDataGlobal(Global, DL));
        }
        return true;
    }

    bool InkVMMachineModuleBuilder::emitDataGlobal(const GlobalVariable& Global, const DataLayout& DL)
    {
        Type* ValueTy = Global.getValueType();
        ERROR_IF(ValueTy == nullptr || !ValueTy->isSized(), "InkVM global variable must have a sized type");
        const TypeSize RawAllocSize = DL.getTypeAllocSize(ValueTy);
        ERROR_IF(RawAllocSize.isScalable(), "InkVM global variable does not support scalable types");

        const std::uint64_t ByteSize = RawAllocSize.getFixedValue();
        const std::uint64_t Alignment = std::max<std::uint64_t>(1, Global.getAlign().value_or(DL.getABITypeAlign(ValueTy)).value());
        const ink::vm::MachineSymbolLinkage Linkage = Global.hasLocalLinkage() ? ink::vm::MachineSymbolLinkage::Local : ink::vm::MachineSymbolLinkage::External;
        ERROR_IF(Linkage == ink::vm::MachineSymbolLinkage::External && Global.getName().empty(), "InkVM external global definition requires a nonempty name");

        const ink::core::Name GlobalName = internName(toStringView(Global.getName()));
        std::vector<std::uint8_t> InitialBytes;
        std::vector<ink::vm::MachineGlobalRelocation> Relocations;
        if (Global.hasInitializer())
        {
            InitialBytes.reserve(static_cast<std::size_t>(ByteSize));
            RETURN_IF_FAILED(lowerInkVMConstantData(Global.getInitializer(), DL, *this, InitialBytes, Relocations));
            trimTrailingZeroBytes(InitialBytes);
        }

        const auto Created = Output->createGlobalVariable(ByteSize, Alignment, InitialBytes, GlobalName, Linkage, Global.isConstant(), Relocations);
        ERROR_IF(!Created, std::string(formatInkVMGlobalDataError(Created.error())));
        if (GlobalName.valid())
        {
            KnownSymbols.insert(GlobalName);
        }
        return true;
    }

    void InkVMMachineModuleBuilder::recordDefinedFunctionSymbols(const Module& IRModule)
    {
        for (const Function& Func : IRModule.functions())
        {
            if (Func.isDeclaration() || Func.getName().empty())
            {
                continue;
            }
            KnownSymbols.insert(internName(toStringView(Func.getName())));
        }
    }

    bool InkVMMachineModuleBuilder::emitFunction(const MachineFunction& MF)
    {
        if (Failed)
        {
            return false;
        }
        ERROR_IF(Output == nullptr, "InkVM MachineModule emitter was not initialized");

        const Function& IRFunc = MF.getFunction();
        const ink::vm::MachineSymbolLinkage Linkage = IRFunc.hasLocalLinkage() ? ink::vm::MachineSymbolLinkage::Local : ink::vm::MachineSymbolLinkage::External;
        ERROR_IF(Linkage == ink::vm::MachineSymbolLinkage::External && IRFunc.getName().empty(), "InkVM external function definition requires a nonempty name");

        const ink::core::Name FuncName = internName(toStringView(IRFunc.getName()));
        ink::vm::MachineFunction& VMFunc = Output->createFunction(FuncName, Linkage);
        if (FuncName.valid())
        {
            KnownSymbols.insert(FuncName);
        }

        const DenseMap<const MachineBasicBlock*, ink::vm::MachineBasicBlock*> BlockMap = createFunctionBlocks(MF, VMFunc, *this);
        return lowerInkVMFunctionInstructions(MF, BlockMap, *this);
    }

    std::expected<std::unique_ptr<ink::vm::MachineModule>, std::string> InkVMMachineModuleBuilder::takeModule()
    {
        if (Failed)
        {
            Output.reset();
            return std::unexpected(ErrorMessage.empty() ? std::string("InkVM MachineModule emission failed") : std::move(ErrorMessage));
        }
        if (Output == nullptr)
        {
            return std::unexpected("InkVM MachineModule emitter produced no module");
        }
        return std::move(Output);
    }

    FunctionPass* createInkVMMachineModuleEmitter(InkVMTargetMachine& TM, InkVMMachineModuleEmitSink& Sink)
    {
        static_cast<void>(TM);
        return new InkVMMachineModuleEmitter(Sink);
    }
} // namespace llvm

#undef ERROR_IF
#undef RETURN_IF_FAILED
