#include "ink/backend/llvm_aot.h"

#include "ink/ir/verifier.h"

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/IR/Attributes.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/FileUtilities.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <variant>
#include <vector>

namespace ink::backend
{
  const char *backendErrorCodeName(BackendErrorCode Code) noexcept
  {
    switch (Code)
    {
    case BackendErrorCode::InvalidTarget:
      return "InvalidTarget";
    case BackendErrorCode::NonNativeTarget:
      return "NonNativeTarget";
    case BackendErrorCode::TargetMachineUnavailable:
      return "TargetMachineUnavailable";
    case BackendErrorCode::TargetMismatch:
      return "TargetMismatch";
    case BackendErrorCode::InvalidIr:
      return "InvalidIr";
    case BackendErrorCode::UnsupportedType:
      return "UnsupportedType";
    case BackendErrorCode::UnsupportedOpcode:
      return "UnsupportedOpcode";
    case BackendErrorCode::LlvmVerificationFailed:
      return "LlvmVerificationFailed";
    case BackendErrorCode::OutputAlreadyExists:
      return "OutputAlreadyExists";
    case BackendErrorCode::ObjectEmissionFailed:
      return "ObjectEmissionFailed";
    case BackendErrorCode::IoError:
      return "IoError";
    }
    return "Unknown";
  }

  namespace
  {
    BackendError makeError(BackendErrorCode Code, std::string Message, ir::IrFunctionId Function = {}, ir::IrBlockId Block = {}, ir::IrOperationId Operation = {})
    {
      return {Code, std::move(Message), Function, Block, Operation};
    }

    struct TargetState
    {
      std::unique_ptr<llvm::TargetMachine> Machine;
      std::string Triple;
    };

    struct LoweredModule
    {
      std::unique_ptr<llvm::LLVMContext> Context;
      std::unique_ptr<llvm::TargetMachine> Machine;
      std::unique_ptr<llvm::Module> Module;
    };

    BackendResult<void> initializeNativeBackend()
    {
      static std::once_flag Once;
      static std::optional<BackendError> Error;
      std::call_once(Once, []()
      {
        if (llvm::InitializeNativeTarget())
        {
          Error = makeError(BackendErrorCode::TargetMachineUnavailable, "LLVM failed to initialize the native target");
          return;
        }
        if (llvm::InitializeNativeTargetAsmPrinter())
        {
          Error = makeError(BackendErrorCode::TargetMachineUnavailable, "LLVM failed to initialize the native assembly printer");
        }
      });
      return Error ? BackendResult<void>::failure(*Error) : BackendResult<void>::success();
    }

    BackendResult<TargetState> createTargetState(const target::TargetKey &Key)
    {
      if (!Key.isValid() || Key.Cpu.empty())
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::InvalidTarget, "AOT emission requires a valid target key with an explicit CPU"));
      }
      BackendResult<void> Initialization = initializeNativeBackend();
      if (!Initialization)
      {
        return BackendResult<TargetState>::failure(Initialization.error());
      }
      const std::string RequestedTriple = llvm::Triple::normalize(Key.Triple);
      const std::string HostTriple = llvm::Triple::normalize(llvm::sys::getDefaultTargetTriple());
      if (RequestedTriple != HostTriple)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::NonNativeTarget, "requested target triple '" + Key.Triple + "' does not match native triple '" + HostTriple + "'"));
      }
      std::string LookupError;
      const llvm::Target *Target = llvm::TargetRegistry::lookupTarget(llvm::Triple(RequestedTriple), LookupError);
      if (!Target)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::TargetMachineUnavailable, "LLVM could not find the native target: " + LookupError));
      }
      std::unique_ptr<llvm::MCSubtargetInfo> Metadata(Target->createMCSubtargetInfo(llvm::Triple(RequestedTriple), "generic", ""));
      if (!Metadata)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::TargetMachineUnavailable, "LLVM could not create native subtarget metadata for target-key validation"));
      }
      if (!Metadata->isCPUStringValid(Key.Cpu))
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::InvalidTarget, "target key names an unknown native CPU: " + Key.Cpu));
      }
      const llvm::ArrayRef<llvm::SubtargetFeatureKV> KnownFeatures = Metadata->getAllProcessorFeatures();
      std::size_t FeatureStart = 0;
      while (FeatureStart < Key.Features.size())
      {
        const std::size_t FeatureEnd = Key.Features.find(',', FeatureStart);
        const std::size_t FeatureLength = FeatureEnd == std::string::npos ? Key.Features.size() - FeatureStart : FeatureEnd - FeatureStart;
        const std::string_view Feature(Key.Features.data() + FeatureStart, FeatureLength);
        if (Feature.size() < 2 || (Feature.front() != '+' && Feature.front() != '-'))
        {
          return BackendResult<TargetState>::failure(makeError(BackendErrorCode::InvalidTarget, "target feature entries must use nonempty +feature or -feature syntax"));
        }
        const std::string_view FeatureName = Feature.substr(1);
        const bool Known = std::any_of(KnownFeatures.begin(), KnownFeatures.end(), [FeatureName](const llvm::SubtargetFeatureKV &Candidate)
        {
          return FeatureName == Candidate.Key;
        });
        if (!Known)
        {
          return BackendResult<TargetState>::failure(makeError(BackendErrorCode::InvalidTarget, "target key names an unknown native feature: " + std::string(FeatureName)));
        }
        if (FeatureEnd == std::string::npos)
        {
          FeatureStart = Key.Features.size();
        }
        else
        {
          FeatureStart = FeatureEnd + 1;
          if (FeatureStart == Key.Features.size())
          {
            return BackendResult<TargetState>::failure(makeError(BackendErrorCode::InvalidTarget, "target feature string must not end with an empty entry"));
          }
        }
      }
      std::unique_ptr<llvm::MCSubtargetInfo> RequestedSubtarget(Target->createMCSubtargetInfo(llvm::Triple(RequestedTriple), Key.Cpu, Key.Features));
      if (!RequestedSubtarget)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::InvalidTarget, "LLVM rejected the validated CPU and feature combination"));
      }
      static_cast<void>(RequestedSubtarget);
      llvm::TargetOptions Options;
      std::unique_ptr<llvm::TargetMachine> Machine(Target->createTargetMachine(llvm::Triple(RequestedTriple), Key.Cpu, Key.Features, Options, std::nullopt, std::nullopt, llvm::CodeGenOptLevel::None));
      if (!Machine)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::TargetMachineUnavailable, "LLVM could not create a target machine for the complete target key"));
      }
      if (Machine->getTargetTriple().normalize() != RequestedTriple)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::TargetMismatch, "LLVM target machine changed the requested target triple"));
      }
      if (Machine->getTargetCPU() != Key.Cpu)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::TargetMismatch, "LLVM target machine changed the requested CPU"));
      }
      if (Machine->getTargetFeatureString() != Key.Features)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::TargetMismatch, "LLVM target machine changed the requested feature string"));
      }
      const llvm::DataLayout Layout = Machine->createDataLayout();
      if (Layout.getPointerSizeInBits(0) != Key.PointerBitWidth)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::TargetMismatch, "LLVM data layout pointer width does not match the target key"));
      }
      const bool ExpectedLittleEndian = Key.Endianness == target::TargetEndianness::Little;
      if (Layout.isLittleEndian() != ExpectedLittleEndian)
      {
        return BackendResult<TargetState>::failure(makeError(BackendErrorCode::TargetMismatch, "LLVM data layout endianness does not match the target key"));
      }
      return BackendResult<TargetState>::success({std::move(Machine), RequestedTriple});
    }

    bool isSupportedOpcode(ir::IrOpcode Opcode) noexcept
    {
      switch (Opcode)
      {
      case ir::IrOpcode::ConstInt:
      case ir::IrOpcode::ConstBool:
      case ir::IrOpcode::IntAdd:
      case ir::IrOpcode::IntSub:
      case ir::IrOpcode::IntMul:
      case ir::IrOpcode::IntNeg:
      case ir::IrOpcode::IntAnd:
      case ir::IrOpcode::IntOr:
      case ir::IrOpcode::IntXor:
      case ir::IrOpcode::IntCompare:
      case ir::IrOpcode::BoolNot:
      case ir::IrOpcode::BoolAnd:
      case ir::IrOpcode::BoolOr:
      case ir::IrOpcode::CastInt:
      case ir::IrOpcode::Alloca:
      case ir::IrOpcode::Load:
      case ir::IrOpcode::Store:
      case ir::IrOpcode::DirectCall:
      case ir::IrOpcode::Branch:
      case ir::IrOpcode::CondBranch:
      case ir::IrOpcode::Return:
      case ir::IrOpcode::Unreachable:
      case ir::IrOpcode::Trap:
        return true;
      case ir::IrOpcode::IntSignedDiv:
      case ir::IrOpcode::IntUnsignedDiv:
      case ir::IrOpcode::IntSignedRem:
      case ir::IrOpcode::IntUnsignedRem:
      case ir::IrOpcode::Unknown:
        return false;
      }
      return false;
    }

    class ModuleLowerer
    {
    public:
      ModuleLowerer(const ir::VerifiedClosedModule &ClosedModule, TargetState Target, const AotRuntimeSupport &RuntimeSupportValue) : InkModule(ClosedModule.module()), RuntimeSupport(RuntimeSupportValue), WindowsTarget(llvm::Triple(Target.Triple).isOSWindows()), Context(std::make_unique<llvm::LLVMContext>()), Machine(std::move(Target.Machine)), LlvmModule(std::make_unique<llvm::Module>("ink", *Context)), Types(InkModule.typeCount(), nullptr), TypeStates(InkModule.typeCount(), 0), Functions(InkModule.functionCount(), nullptr), Blocks(InkModule.blockCount(), nullptr), Prologues(InkModule.functionCount(), nullptr), Values(InkModule.valueCount(), nullptr)
      {
        LlvmModule->setTargetTriple(llvm::Triple(Target.Triple));
        LlvmModule->setDataLayout(Machine->createDataLayout());
      }

      BackendResult<LoweredModule> run()
      {
        const std::vector<ir::IrVerificationError> InkErrors = ir::verifyModule(InkModule, ir::IrStage::Closed);
        if (!InkErrors.empty())
        {
          return BackendResult<LoweredModule>::failure(makeError(BackendErrorCode::InvalidIr, "VerifiedClosedModule no longer satisfies the closed InkIR verifier"));
        }
        if (InkModule.planNodeCount() != 0)
        {
          return BackendResult<LoweredModule>::failure(makeError(BackendErrorCode::InvalidIr, "closed InkIR still contains elaboration plan nodes"));
        }
        if (!checkOpcodes() || !lowerTypes() || !createFunctions() || !lowerStaticOutputFunctions() || !createBlocksAndPhis() || !lowerFunctionBodies())
        {
          return BackendResult<LoweredModule>::failure(*Failure);
        }
        std::string VerificationText;
        llvm::raw_string_ostream VerificationStream(VerificationText);
        if (llvm::verifyModule(*LlvmModule, &VerificationStream))
        {
          VerificationStream.flush();
          return BackendResult<LoweredModule>::failure(makeError(BackendErrorCode::LlvmVerificationFailed, "LLVM verifier rejected lowered InkIR: " + VerificationText));
        }
        return BackendResult<LoweredModule>::success({std::move(Context), std::move(Machine), std::move(LlvmModule)});
      }

    private:
      void fail(BackendErrorCode Code, std::string Message, ir::IrFunctionId Function = {}, ir::IrBlockId Block = {}, ir::IrOperationId Operation = {})
      {
        if (!Failure)
        {
          Failure = makeError(Code, std::move(Message), Function, Block, Operation);
        }
      }

      void failOperation(BackendErrorCode Code, std::string Message, ir::IrOperationId Id)
      {
        const ir::IrOperation &Operation = InkModule.operation(Id);
        fail(Code, std::move(Message), Operation.OwnerFunction, Operation.OwnerBlock, Id);
      }

      bool checkOpcodes()
      {
        for (std::size_t Index = 0; Index < InkModule.operationCount(); ++Index)
        {
          const ir::IrOperationId Id = ir::IrOperationId::fromValue(static_cast<std::uint32_t>(Index));
          const ir::IrOpcode Opcode = InkModule.operation(Id).Opcode;
          if (Opcode == ir::IrOpcode::IntSignedDiv || Opcode == ir::IrOpcode::IntUnsignedDiv || Opcode == ir::IrOpcode::IntSignedRem || Opcode == ir::IrOpcode::IntUnsignedRem)
          {
            failOperation(BackendErrorCode::UnsupportedOpcode, "PDB integer division and remainder are unsupported until target trap and overflow behavior is preserved exactly", Id);
            return false;
          }
          if (!isSupportedOpcode(Opcode))
          {
            failOperation(BackendErrorCode::UnsupportedOpcode, std::string("unsupported closed InkIR opcode: ") + ir::irOpcodeName(Opcode), Id);
            return false;
          }
        }
        return true;
      }

      llvm::Type *lowerType(ir::IrTypeId Id)
      {
        if (!InkModule.contains(Id))
        {
          fail(BackendErrorCode::UnsupportedType, "InkIR type ID is outside the module type table");
          return nullptr;
        }
        if (TypeStates[Id.value()] == 2)
        {
          return Types[Id.value()];
        }
        if (TypeStates[Id.value()] == 1)
        {
          fail(BackendErrorCode::UnsupportedType, "recursive first-slice InkIR type cannot be represented by the LLVM backend");
          return nullptr;
        }
        TypeStates[Id.value()] = 1;
        const ir::IrType &Type = InkModule.type(Id);
        llvm::Type *Result = nullptr;
        switch (Type.Kind)
        {
        case ir::IrTypeKind::Unit:
          Result = llvm::StructType::get(*Context);
          break;
        case ir::IrTypeKind::Never:
          Result = llvm::Type::getVoidTy(*Context);
          break;
        case ir::IrTypeKind::Bool:
          Result = llvm::Type::getInt1Ty(*Context);
          break;
        case ir::IrTypeKind::Integer:
          if (Type.BitWidth != 32 && Type.BitWidth != 64)
          {
            fail(BackendErrorCode::UnsupportedType, "LLVM AOT first slice supports only 32-bit and 64-bit integers");
          }
          else
          {
            Result = llvm::IntegerType::get(*Context, Type.BitWidth);
          }
          break;
        case ir::IrTypeKind::Place:
          Result = lowerValueType(Type.ElementType) ? llvm::PointerType::get(*Context, 0) : nullptr;
          break;
        case ir::IrTypeKind::Function:
          Result = lowerFunctionType(Type);
          break;
        case ir::IrTypeKind::Unknown:
          fail(BackendErrorCode::UnsupportedType, "unknown InkIR type cannot be lowered to LLVM");
          break;
        }
        if (!Result)
        {
          return nullptr;
        }
        Types[Id.value()] = Result;
        TypeStates[Id.value()] = 2;
        return Result;
      }

      llvm::Type *lowerValueType(ir::IrTypeId Id)
      {
        if (!InkModule.contains(Id))
        {
          fail(BackendErrorCode::UnsupportedType, "InkIR value references an invalid type");
          return nullptr;
        }
        const ir::IrTypeKind Kind = InkModule.type(Id).Kind;
        if (Kind == ir::IrTypeKind::Never || Kind == ir::IrTypeKind::Function || Kind == ir::IrTypeKind::Unknown)
        {
          fail(BackendErrorCode::UnsupportedType, "InkIR never, function, or unknown type cannot be materialized as an LLVM SSA value");
          return nullptr;
        }
        return lowerType(Id);
      }

      llvm::FunctionType *lowerFunctionType(const ir::IrType &Type)
      {
        std::vector<llvm::Type *> Parameters;
        Parameters.reserve(Type.Parameters.Count);
        for (std::uint32_t Index = Type.Parameters.First; Index < Type.Parameters.end(); ++Index)
        {
          llvm::Type *Parameter = lowerValueType(InkModule.typeReference(Index));
          if (!Parameter)
          {
            return nullptr;
          }
          Parameters.push_back(Parameter);
        }
        llvm::Type *Result = llvm::Type::getVoidTy(*Context);
        if (Type.Result && InkModule.type(*Type.Result).Kind != ir::IrTypeKind::Never)
        {
          Result = lowerValueType(*Type.Result);
          if (!Result)
          {
            return nullptr;
          }
        }
        return llvm::FunctionType::get(Result, Parameters, false);
      }

      bool lowerTypes()
      {
        for (std::size_t Index = 0; Index < InkModule.typeCount(); ++Index)
        {
          if (!lowerType(ir::IrTypeId::fromValue(static_cast<std::uint32_t>(Index))))
          {
            return false;
          }
        }
        return true;
      }

      bool createFunctions()
      {
        for (std::size_t Index = 0; Index < InkModule.functionCount(); ++Index)
        {
          const ir::IrFunctionId Id = ir::IrFunctionId::fromValue(static_cast<std::uint32_t>(Index));
          const ir::IrFunction &Function = InkModule.function(Id);
          llvm::FunctionType *Signature = llvm::dyn_cast<llvm::FunctionType>(lowerType(Function.Signature));
          if (!Signature)
          {
            fail(BackendErrorCode::UnsupportedType, "InkIR function signature did not lower to an LLVM function type", Id);
            return false;
          }
          llvm::Function *LlvmFunction = llvm::Function::Create(Signature, llvm::GlobalValue::ExternalLinkage, Function.Name, *LlvmModule);
          Functions[Index] = LlvmFunction;
          std::size_t ParameterIndex = 0;
          for (llvm::Argument &Argument : LlvmFunction->args())
          {
            Argument.setName("arg" + std::to_string(ParameterIndex));
            ++ParameterIndex;
          }
          const ir::IrType &IrSignature = InkModule.type(Function.Signature);
          if (IrSignature.Result && InkModule.type(*IrSignature.Result).Kind == ir::IrTypeKind::Never)
          {
            LlvmFunction->addFnAttr(llvm::Attribute::NoReturn);
          }
        }
        return true;
      }

      bool lowerStaticOutputFunctions()
      {
        std::size_t OutputIndex = 0;
        for (const StaticOutputFunction &Output : RuntimeSupport.StaticOutputFunctions)
        {
          std::optional<ir::IrFunctionId> FunctionId;
          for (std::size_t Index = 0; Index < InkModule.functionCount(); ++Index)
          {
            const ir::IrFunctionId Candidate = ir::IrFunctionId::fromValue(static_cast<std::uint32_t>(Index));
            if (InkModule.function(Candidate).Name == Output.FunctionName)
            {
              FunctionId = Candidate;
              break;
            }
          }
          if (!FunctionId)
          {
            fail(BackendErrorCode::InvalidIr, "AOT runtime output support names a function that is absent from InkIR: " + Output.FunctionName);
            return false;
          }
          const ir::IrFunction &InkFunction = InkModule.function(*FunctionId);
          llvm::Function *Function = Functions[FunctionId->value()];
          const ir::IrType &Signature = InkModule.type(InkFunction.Signature);
          if (InkFunction.Kind != ir::IrFunctionKind::External || Signature.Parameters.Count != 0 || !Signature.Result || InkModule.type(*Signature.Result).Kind != ir::IrTypeKind::Integer || InkModule.type(*Signature.Result).BitWidth != 32 || !Function->arg_empty() || !Function->getReturnType()->isIntegerTy(32) || !Function->empty())
          {
            fail(BackendErrorCode::InvalidIr, "AOT runtime output support requires a unique external 'func () -> i32' declaration", *FunctionId);
            return false;
          }
          llvm::BasicBlock *Entry = llvm::BasicBlock::Create(*Context, "entry", Function);
          llvm::IRBuilder<> Builder(Entry);
          if (Output.Bytes.empty())
          {
            Builder.CreateRet(Builder.getInt32(0));
            ++OutputIndex;
            continue;
          }
          llvm::Constant *Data = llvm::ConstantDataArray::getString(*Context, Output.Bytes, false);
          llvm::GlobalVariable *Global = new llvm::GlobalVariable(*LlvmModule, Data->getType(), true, llvm::GlobalValue::PrivateLinkage, Data, "ink.stdout." + std::to_string(OutputIndex));
          llvm::Value *DataPointer = Builder.CreateInBoundsGEP(Data->getType(), Global, {Builder.getInt32(0), Builder.getInt32(0)}, "data");
          if (WindowsTarget)
          {
            if (Output.Bytes.size() > std::numeric_limits<std::uint32_t>::max())
            {
              fail(BackendErrorCode::UnsupportedType, "a single Windows standard-output operation cannot exceed 4 GiB", *FunctionId);
              return false;
            }
            llvm::PointerType *Pointer = llvm::PointerType::get(*Context, 0);
            llvm::FunctionType *GetStdHandleType = llvm::FunctionType::get(Pointer, {Builder.getInt32Ty()}, false);
            llvm::FunctionType *WriteFileType = llvm::FunctionType::get(Builder.getInt32Ty(), {Pointer, Pointer, Builder.getInt32Ty(), Pointer, Pointer}, false);
            llvm::FunctionCallee GetStdHandle = LlvmModule->getOrInsertFunction("GetStdHandle", GetStdHandleType);
            llvm::FunctionCallee WriteFile = LlvmModule->getOrInsertFunction("WriteFile", WriteFileType);
            llvm::Value *Handle = Builder.CreateCall(GetStdHandle, {Builder.getInt32(static_cast<std::uint32_t>(-11))}, "stdout");
            llvm::Value *Transferred = Builder.CreateAlloca(Builder.getInt32Ty(), nullptr, "transferred");
            llvm::Value *Null = llvm::ConstantPointerNull::get(Pointer);
            llvm::Value *Succeeded = Builder.CreateCall(WriteFile, {Handle, DataPointer, Builder.getInt32(static_cast<std::uint32_t>(Output.Bytes.size())), Transferred, Null}, "succeeded");
            llvm::Value *Written = Builder.CreateLoad(Builder.getInt32Ty(), Transferred, "written");
            Builder.CreateRet(Builder.CreateSelect(Builder.CreateICmpNE(Succeeded, Builder.getInt32(0)), Written, Builder.getInt32(-1)));
          }
          else
          {
            llvm::IntegerType *SizeType = llvm::IntegerType::get(*Context, LlvmModule->getDataLayout().getPointerSizeInBits(0));
            llvm::FunctionType *WriteType = llvm::FunctionType::get(SizeType, {Builder.getInt32Ty(), llvm::PointerType::get(*Context, 0), SizeType}, false);
            llvm::FunctionCallee Write = LlvmModule->getOrInsertFunction("write", WriteType);
            llvm::Value *Written = Builder.CreateCall(Write, {Builder.getInt32(1), DataPointer, llvm::ConstantInt::get(SizeType, Output.Bytes.size())}, "written");
            Builder.CreateRet(Builder.CreateIntCast(Written, Builder.getInt32Ty(), true));
          }
          ++OutputIndex;
        }
        return true;
      }

      bool createBlocksAndPhis()
      {
        for (std::size_t FunctionIndex = 0; FunctionIndex < InkModule.functionCount(); ++FunctionIndex)
        {
          const ir::IrFunctionId FunctionId = ir::IrFunctionId::fromValue(static_cast<std::uint32_t>(FunctionIndex));
          const ir::IrFunction &Function = InkModule.function(FunctionId);
          if (Function.Kind == ir::IrFunctionKind::External)
          {
            continue;
          }
          llvm::Function *LlvmFunction = Functions[FunctionIndex];
          llvm::BasicBlock *Prologue = llvm::BasicBlock::Create(*Context, "entry", LlvmFunction);
          Prologues[FunctionIndex] = Prologue;
          for (std::uint32_t Index = Function.Blocks.First; Index < Function.Blocks.end(); ++Index)
          {
            const ir::IrBlockId BlockId = InkModule.functionBlock(Index);
            Blocks[BlockId.value()] = llvm::BasicBlock::Create(*Context, "bb" + std::to_string(BlockId.value()), LlvmFunction);
          }
          for (std::uint32_t Index = Function.Blocks.First; Index < Function.Blocks.end(); ++Index)
          {
            const ir::IrBlockId BlockId = InkModule.functionBlock(Index);
            const ir::IrBlock &Block = InkModule.block(BlockId);
            for (std::uint32_t ArgumentIndex = 0; ArgumentIndex < Block.Arguments.Count; ++ArgumentIndex)
            {
              const ir::IrValueId ValueId = ir::IrValueId::fromValue(Block.Arguments.First + ArgumentIndex);
              llvm::Type *ArgumentType = lowerValueType(InkModule.value(ValueId).Type);
              if (!ArgumentType)
              {
                return false;
              }
              Values[ValueId.value()] = llvm::PHINode::Create(ArgumentType, 0, "v" + std::to_string(ValueId.value()), Blocks[BlockId.value()]);
            }
          }
          llvm::IRBuilder<> Builder(Prologue);
          Builder.CreateBr(Blocks[Function.EntryBlock.value()]);
          const ir::IrBlock &Entry = InkModule.block(Function.EntryBlock);
          std::size_t ArgumentIndex = 0;
          for (llvm::Argument &Argument : LlvmFunction->args())
          {
            const ir::IrValueId ValueId = ir::IrValueId::fromValue(Entry.Arguments.First + static_cast<std::uint32_t>(ArgumentIndex));
            llvm::cast<llvm::PHINode>(Values[ValueId.value()])->addIncoming(&Argument, Prologue);
            ++ArgumentIndex;
          }
        }
        return true;
      }

      std::vector<ir::IrBlockId> blockOrder(const ir::IrFunction &Function) const
      {
        std::vector<ir::IrBlockId> Result;
        std::vector<bool> Visited(InkModule.blockCount(), false);
        std::queue<ir::IrBlockId> Work;
        Work.push(Function.EntryBlock);
        Visited[Function.EntryBlock.value()] = true;
        while (!Work.empty())
        {
          const ir::IrBlockId BlockId = Work.front();
          Work.pop();
          Result.push_back(BlockId);
          const ir::IrBlock &Block = InkModule.block(BlockId);
          for (std::uint32_t OperationIndex = Block.Operations.First; OperationIndex < Block.Operations.end(); ++OperationIndex)
          {
            const ir::IrOperation &Operation = InkModule.operation(InkModule.blockOperation(OperationIndex));
            for (std::uint32_t SuccessorIndex = Operation.Successors.First; SuccessorIndex < Operation.Successors.end(); ++SuccessorIndex)
            {
              const ir::IrBlockId Target = InkModule.operationSuccessor(SuccessorIndex).Block;
              if (!Visited[Target.value()])
              {
                Visited[Target.value()] = true;
                Work.push(Target);
              }
            }
          }
        }
        for (std::uint32_t Index = Function.Blocks.First; Index < Function.Blocks.end(); ++Index)
        {
          const ir::IrBlockId BlockId = InkModule.functionBlock(Index);
          if (!Visited[BlockId.value()])
          {
            Result.push_back(BlockId);
          }
        }
        return Result;
      }

      llvm::Value *value(ir::IrValueId Id, ir::IrOperationId User)
      {
        if (!InkModule.contains(Id) || !Values[Id.value()])
        {
          failOperation(BackendErrorCode::InvalidIr, "InkIR operand was not available when its use was lowered", User);
          return nullptr;
        }
        return Values[Id.value()];
      }

      llvm::Value *operand(const ir::IrOperation &Operation, std::uint32_t Index, ir::IrOperationId Id)
      {
        return value(InkModule.operationOperand(Operation.Operands.First + Index), Id);
      }

      ir::IrValueId resultId(const ir::IrOperation &Operation, std::uint32_t Index) const
      {
        return InkModule.operationResult(Operation.Results.First + Index);
      }

      void bindResult(const ir::IrOperation &Operation, llvm::Value *Value)
      {
        const ir::IrValueId Id = resultId(Operation, 0);
        Values[Id.value()] = Value;
        if (!llvm::isa<llvm::Constant>(Value) && Value->getName().empty())
        {
          Value->setName("v" + std::to_string(Id.value()));
        }
      }

      bool addSuccessorIncoming(const ir::IrSuccessor &Successor, llvm::BasicBlock *Predecessor, ir::IrOperationId OperationId)
      {
        const ir::IrBlock &Target = InkModule.block(Successor.Block);
        for (std::uint32_t Index = 0; Index < Successor.Arguments.Count; ++Index)
        {
          llvm::Value *Incoming = value(InkModule.successorArgument(Successor.Arguments.First + Index), OperationId);
          if (!Incoming)
          {
            return false;
          }
          const ir::IrValueId TargetArgument = ir::IrValueId::fromValue(Target.Arguments.First + Index);
          llvm::PHINode *Phi = llvm::dyn_cast_or_null<llvm::PHINode>(Values[TargetArgument.value()]);
          if (!Phi)
          {
            failOperation(BackendErrorCode::InvalidIr, "branch target argument is not represented by an LLVM PHI", OperationId);
            return false;
          }
          Phi->addIncoming(Incoming, Predecessor);
        }
        return true;
      }

      llvm::BasicBlock *createConditionalEdgeBlock(const ir::IrOperation &Operation, ir::IrOperationId OperationId, std::uint32_t SuccessorIndex)
      {
        const std::string Name = "edge" + std::to_string(OperationId.value()) + "_" + std::to_string(SuccessorIndex);
        return llvm::BasicBlock::Create(*Context, Name, Functions[Operation.OwnerFunction.value()]);
      }

      llvm::CmpInst::Predicate comparisonPredicate(ir::IrComparePredicate Predicate) const
      {
        switch (Predicate)
        {
        case ir::IrComparePredicate::Equal:
          return llvm::CmpInst::ICMP_EQ;
        case ir::IrComparePredicate::NotEqual:
          return llvm::CmpInst::ICMP_NE;
        case ir::IrComparePredicate::SignedLess:
          return llvm::CmpInst::ICMP_SLT;
        case ir::IrComparePredicate::SignedLessEqual:
          return llvm::CmpInst::ICMP_SLE;
        case ir::IrComparePredicate::SignedGreater:
          return llvm::CmpInst::ICMP_SGT;
        case ir::IrComparePredicate::SignedGreaterEqual:
          return llvm::CmpInst::ICMP_SGE;
        case ir::IrComparePredicate::UnsignedLess:
          return llvm::CmpInst::ICMP_ULT;
        case ir::IrComparePredicate::UnsignedLessEqual:
          return llvm::CmpInst::ICMP_ULE;
        case ir::IrComparePredicate::UnsignedGreater:
          return llvm::CmpInst::ICMP_UGT;
        case ir::IrComparePredicate::UnsignedGreaterEqual:
          return llvm::CmpInst::ICMP_UGE;
        }
        return llvm::CmpInst::BAD_ICMP_PREDICATE;
      }

      bool lowerOperation(ir::IrOperationId Id, llvm::IRBuilder<> &Builder)
      {
        const ir::IrOperation &Operation = InkModule.operation(Id);
        switch (Operation.Opcode)
        {
        case ir::IrOpcode::ConstInt:
        case ir::IrOpcode::ConstBool:
        {
          const ir::IrConstantPayload *Payload = std::get_if<ir::IrConstantPayload>(&Operation.Payload);
          if (!Payload || !InkModule.contains(Payload->Constant))
          {
            failOperation(BackendErrorCode::InvalidIr, "constant operation carries an invalid constant payload", Id);
            return false;
          }
          const ir::IrConstant &Constant = InkModule.constant(Payload->Constant);
          llvm::IntegerType *Type = llvm::dyn_cast<llvm::IntegerType>(lowerValueType(Constant.Type));
          if (!Type)
          {
            failOperation(BackendErrorCode::UnsupportedType, "integer or boolean constant did not lower to an LLVM integer type", Id);
            return false;
          }
          bindResult(Operation, llvm::ConstantInt::get(Type, Constant.Bits));
          return true;
        }
        case ir::IrOpcode::IntAdd:
        case ir::IrOpcode::IntSub:
        case ir::IrOpcode::IntMul:
        case ir::IrOpcode::IntAnd:
        case ir::IrOpcode::IntOr:
        case ir::IrOpcode::IntXor:
        case ir::IrOpcode::BoolAnd:
        case ir::IrOpcode::BoolOr:
        {
          llvm::Value *Left = operand(Operation, 0, Id);
          llvm::Value *Right = operand(Operation, 1, Id);
          if (!Left || !Right)
          {
            return false;
          }
          llvm::Value *Result = nullptr;
          switch (Operation.Opcode)
          {
          case ir::IrOpcode::IntAdd:
            Result = Builder.CreateAdd(Left, Right);
            break;
          case ir::IrOpcode::IntSub:
            Result = Builder.CreateSub(Left, Right);
            break;
          case ir::IrOpcode::IntMul:
            Result = Builder.CreateMul(Left, Right);
            break;
          case ir::IrOpcode::IntAnd:
          case ir::IrOpcode::BoolAnd:
            Result = Builder.CreateAnd(Left, Right);
            break;
          case ir::IrOpcode::IntOr:
          case ir::IrOpcode::BoolOr:
            Result = Builder.CreateOr(Left, Right);
            break;
          case ir::IrOpcode::IntXor:
            Result = Builder.CreateXor(Left, Right);
            break;
          default:
            break;
          }
          bindResult(Operation, Result);
          return true;
        }
        case ir::IrOpcode::IntNeg:
        {
          llvm::Value *Operand = operand(Operation, 0, Id);
          if (!Operand)
          {
            return false;
          }
          bindResult(Operation, Builder.CreateNeg(Operand));
          return true;
        }
        case ir::IrOpcode::BoolNot:
        {
          llvm::Value *Operand = operand(Operation, 0, Id);
          if (!Operand)
          {
            return false;
          }
          bindResult(Operation, Builder.CreateNot(Operand));
          return true;
        }
        case ir::IrOpcode::IntCompare:
        {
          const ir::IrComparePayload *Payload = std::get_if<ir::IrComparePayload>(&Operation.Payload);
          llvm::Value *Left = operand(Operation, 0, Id);
          llvm::Value *Right = operand(Operation, 1, Id);
          if (!Payload || !Left || !Right)
          {
            failOperation(BackendErrorCode::InvalidIr, "integer comparison carries invalid operands or payload", Id);
            return false;
          }
          bindResult(Operation, Builder.CreateICmp(comparisonPredicate(Payload->Predicate), Left, Right));
          return true;
        }
        case ir::IrOpcode::CastInt:
        {
          const ir::IrTypePayload *Payload = std::get_if<ir::IrTypePayload>(&Operation.Payload);
          llvm::Value *Operand = operand(Operation, 0, Id);
          if (!Payload || !Operand)
          {
            failOperation(BackendErrorCode::InvalidIr, "integer cast carries invalid operand or type payload", Id);
            return false;
          }
          llvm::Type *Destination = lowerValueType(Payload->Type);
          const ir::IrValueId SourceId = InkModule.operationOperand(Operation.Operands.First);
          const ir::IrType &SourceType = InkModule.type(InkModule.value(SourceId).Type);
          if (!Destination || SourceType.Kind != ir::IrTypeKind::Integer)
          {
            failOperation(BackendErrorCode::UnsupportedType, "integer cast source or destination type is unsupported", Id);
            return false;
          }
          bindResult(Operation, Builder.CreateIntCast(Operand, Destination, SourceType.Signedness == ir::IrSignedness::Signed));
          return true;
        }
        case ir::IrOpcode::Alloca:
        {
          const ir::IrTypePayload *Payload = std::get_if<ir::IrTypePayload>(&Operation.Payload);
          llvm::Type *Element = Payload ? lowerValueType(Payload->Type) : nullptr;
          if (!Element)
          {
            failOperation(BackendErrorCode::UnsupportedType, "alloca element type is unsupported", Id);
            return false;
          }
          bindResult(Operation, Builder.CreateAlloca(Element));
          return true;
        }
        case ir::IrOpcode::Load:
        {
          llvm::Value *Place = operand(Operation, 0, Id);
          llvm::Type *ResultType = lowerValueType(InkModule.value(resultId(Operation, 0)).Type);
          if (!Place || !ResultType)
          {
            return false;
          }
          bindResult(Operation, Builder.CreateLoad(ResultType, Place));
          return true;
        }
        case ir::IrOpcode::Store:
        {
          llvm::Value *Place = operand(Operation, 0, Id);
          llvm::Value *StoredValue = operand(Operation, 1, Id);
          if (!Place || !StoredValue)
          {
            return false;
          }
          Builder.CreateStore(StoredValue, Place);
          return true;
        }
        case ir::IrOpcode::DirectCall:
        {
          const ir::IrDirectCallPayload *Payload = std::get_if<ir::IrDirectCallPayload>(&Operation.Payload);
          if (!Payload || !InkModule.contains(Payload->Callee))
          {
            failOperation(BackendErrorCode::InvalidIr, "direct call carries an invalid callee", Id);
            return false;
          }
          std::vector<llvm::Value *> Arguments;
          Arguments.reserve(Operation.Operands.Count);
          for (std::uint32_t Index = 0; Index < Operation.Operands.Count; ++Index)
          {
            llvm::Value *Argument = operand(Operation, Index, Id);
            if (!Argument)
            {
              return false;
            }
            Arguments.push_back(Argument);
          }
          llvm::CallInst *Call = Builder.CreateCall(Functions[Payload->Callee.value()], Arguments);
          if (Operation.Results.Count == 1)
          {
            bindResult(Operation, Call);
          }
          return true;
        }
        case ir::IrOpcode::Branch:
        {
          const ir::IrSuccessor &Successor = InkModule.operationSuccessor(Operation.Successors.First);
          if (!addSuccessorIncoming(Successor, Builder.GetInsertBlock(), Id))
          {
            return false;
          }
          Builder.CreateBr(Blocks[Successor.Block.value()]);
          return true;
        }
        case ir::IrOpcode::CondBranch:
        {
          llvm::Value *Condition = operand(Operation, 0, Id);
          const ir::IrSuccessor &TrueSuccessor = InkModule.operationSuccessor(Operation.Successors.First);
          const ir::IrSuccessor &FalseSuccessor = InkModule.operationSuccessor(Operation.Successors.First + 1);
          llvm::BasicBlock *TrueEdge = createConditionalEdgeBlock(Operation, Id, 0);
          llvm::BasicBlock *FalseEdge = createConditionalEdgeBlock(Operation, Id, 1);
          if (!Condition || !addSuccessorIncoming(TrueSuccessor, TrueEdge, Id) || !addSuccessorIncoming(FalseSuccessor, FalseEdge, Id))
          {
            return false;
          }
          Builder.CreateCondBr(Condition, TrueEdge, FalseEdge);
          llvm::IRBuilder<> TrueEdgeBuilder(TrueEdge);
          TrueEdgeBuilder.CreateBr(Blocks[TrueSuccessor.Block.value()]);
          llvm::IRBuilder<> FalseEdgeBuilder(FalseEdge);
          FalseEdgeBuilder.CreateBr(Blocks[FalseSuccessor.Block.value()]);
          return true;
        }
        case ir::IrOpcode::Return:
          if (Operation.Operands.Count == 0)
          {
            Builder.CreateRetVoid();
          }
          else
          {
            llvm::Value *Value = operand(Operation, 0, Id);
            if (!Value)
            {
              return false;
            }
            Builder.CreateRet(Value);
          }
          return true;
        case ir::IrOpcode::Unreachable:
          Builder.CreateUnreachable();
          return true;
        case ir::IrOpcode::Trap:
        {
          llvm::Function *Trap = llvm::Intrinsic::getOrInsertDeclaration(LlvmModule.get(), llvm::Intrinsic::trap);
          Builder.CreateCall(Trap);
          Builder.CreateUnreachable();
          return true;
        }
        case ir::IrOpcode::IntSignedDiv:
        case ir::IrOpcode::IntUnsignedDiv:
        case ir::IrOpcode::IntSignedRem:
        case ir::IrOpcode::IntUnsignedRem:
        case ir::IrOpcode::Unknown:
          failOperation(BackendErrorCode::UnsupportedOpcode, std::string("unsupported closed InkIR opcode: ") + ir::irOpcodeName(Operation.Opcode), Id);
          return false;
        }
        failOperation(BackendErrorCode::UnsupportedOpcode, std::string("unsupported closed InkIR opcode: ") + ir::irOpcodeName(Operation.Opcode), Id);
        return false;
      }

      bool lowerFunctionBodies()
      {
        for (std::size_t FunctionIndex = 0; FunctionIndex < InkModule.functionCount(); ++FunctionIndex)
        {
          const ir::IrFunctionId FunctionId = ir::IrFunctionId::fromValue(static_cast<std::uint32_t>(FunctionIndex));
          const ir::IrFunction &Function = InkModule.function(FunctionId);
          if (Function.Kind == ir::IrFunctionKind::External)
          {
            continue;
          }
          for (const ir::IrBlockId BlockId : blockOrder(Function))
          {
            llvm::IRBuilder<> Builder(Blocks[BlockId.value()]);
            const ir::IrBlock &Block = InkModule.block(BlockId);
            for (std::uint32_t OperationIndex = Block.Operations.First; OperationIndex < Block.Operations.end(); ++OperationIndex)
            {
              if (!lowerOperation(InkModule.blockOperation(OperationIndex), Builder))
              {
                return false;
              }
            }
          }
        }
        return true;
      }

      const ir::IrModule &InkModule;
      const AotRuntimeSupport &RuntimeSupport;
      bool WindowsTarget = false;
      std::unique_ptr<llvm::LLVMContext> Context;
      std::unique_ptr<llvm::TargetMachine> Machine;
      std::unique_ptr<llvm::Module> LlvmModule;
      std::vector<llvm::Type *> Types;
      std::vector<unsigned char> TypeStates;
      std::vector<llvm::Function *> Functions;
      std::vector<llvm::BasicBlock *> Blocks;
      std::vector<llvm::BasicBlock *> Prologues;
      std::vector<llvm::Value *> Values;
      std::optional<BackendError> Failure;
    };

    BackendResult<LoweredModule> lowerModule(const ir::VerifiedClosedModule &Module, const AotRuntimeSupport &RuntimeSupport)
    {
      BackendResult<TargetState> Target = createTargetState(Module.targetKey());
      if (!Target)
      {
        return BackendResult<LoweredModule>::failure(Target.error());
      }
      return ModuleLowerer(Module, Target.takeValue(), RuntimeSupport).run();
    }
  } // namespace

  BackendResult<std::string> emitLlvmText(const ir::VerifiedClosedModule &Module, const AotRuntimeSupport &RuntimeSupport)
  {
    BackendResult<LoweredModule> Lowered = lowerModule(Module, RuntimeSupport);
    if (!Lowered)
    {
      return BackendResult<std::string>::failure(Lowered.error());
    }
    LoweredModule Value = Lowered.takeValue();
    std::string Text;
    llvm::raw_string_ostream Stream(Text);
    Value.Module->print(Stream, nullptr);
    Stream.flush();
    return BackendResult<std::string>::success(std::move(Text));
  }

  BackendResult<void> emitObject(const ir::VerifiedClosedModule &Module, const std::filesystem::path &OutputPath, const AotRuntimeSupport &RuntimeSupport)
  {
    if (OutputPath.empty())
    {
      return BackendResult<void>::failure(makeError(BackendErrorCode::IoError, "object output path is empty"));
    }
    std::error_code FileSystemError;
    const bool OutputExists = std::filesystem::exists(OutputPath, FileSystemError);
    if (FileSystemError)
    {
      return BackendResult<void>::failure(makeError(BackendErrorCode::IoError, "could not inspect object output path: " + FileSystemError.message()));
    }
    if (OutputExists)
    {
      return BackendResult<void>::failure(makeError(BackendErrorCode::OutputAlreadyExists, "object output path already exists"));
    }
    BackendResult<LoweredModule> Lowered = lowerModule(Module, RuntimeSupport);
    if (!Lowered)
    {
      return BackendResult<void>::failure(Lowered.error());
    }
    LoweredModule Value = Lowered.takeValue();
    const std::string Output = OutputPath.u8string();
    llvm::SmallString<256> TemporaryPath;
    int Descriptor = -1;
    const std::error_code CreateError = llvm::sys::fs::createUniqueFile(Output + ".tmp-%%%%%%", Descriptor, TemporaryPath);
    if (CreateError)
    {
      return BackendResult<void>::failure(makeError(BackendErrorCode::IoError, "could not create temporary object file: " + CreateError.message()));
    }
    llvm::FileRemover RemoveTemporary(TemporaryPath);
    llvm::raw_fd_ostream Stream(Descriptor, true);
    llvm::legacy::PassManager Passes;
    if (Value.Machine->addPassesToEmitFile(Passes, Stream, nullptr, llvm::CodeGenFileType::ObjectFile, false))
    {
      Stream.close();
      if (Stream.has_error())
      {
        Stream.clear_error();
      }
      return BackendResult<void>::failure(makeError(BackendErrorCode::ObjectEmissionFailed, "LLVM target machine cannot emit an object file"));
    }
    Passes.run(*Value.Module);
    Stream.flush();
    Stream.close();
    if (Stream.has_error())
    {
      Stream.clear_error();
      return BackendResult<void>::failure(makeError(BackendErrorCode::IoError, "LLVM failed while writing the temporary object file"));
    }
    const std::error_code LinkError = llvm::sys::fs::create_hard_link(TemporaryPath, Output);
    if (LinkError)
    {
      const BackendErrorCode Code = LinkError == std::errc::file_exists ? BackendErrorCode::OutputAlreadyExists : BackendErrorCode::IoError;
      return BackendResult<void>::failure(makeError(Code, "could not atomically publish emitted object file without replacement: " + LinkError.message()));
    }
    const std::error_code RemoveError = llvm::sys::fs::remove(TemporaryPath);
    if (!RemoveError)
    {
      RemoveTemporary.releaseFile();
    }
    return BackendResult<void>::success();
  }
} // namespace ink::backend
