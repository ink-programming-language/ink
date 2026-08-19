#include "ink/backend/llvm_backend.h"
#include "ink/core/context.h"
#include "ink/ir/analysis/verifier.h"
#include "ink/ir/builder.h"

#include <gtest/gtest.h>

#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/IntrinsicInst.h>
#include <llvm/IR/Intrinsics.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace ink::backend::llvm
{
  namespace
  {
    struct LLVMBackendTestContext
    {
        LLVMBackendTestContext()
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        ~LLVMBackendTestContext()
        {
          Compilation.diagnosticEngine().removeConsumer(Diagnostics);
        }

        core::CompilationContext Compilation;
        ir::IRContext IR{Compilation};
        ::llvm::LLVMContext LLVM;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    bool hasDiagnostic(const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind)
    {
      return std::any_of(Diagnostics.begin(), Diagnostics.end(), [Kind](const core::Diagnostic &Diagnostic)
                         {
                           return Diagnostic.Kind == Kind;
                         });
    }

    std::string firstDiagnosticMessage(const std::vector<core::Diagnostic> &Diagnostics)
    {
      return Diagnostics.empty() ? std::string{} : core::DiagnosticFormatter().format(Diagnostics.front()).Message;
    }

    void expectValidLLVMModule(const ::llvm::Module &ModuleValue)
    {
      std::string VerificationMessage;
      ::llvm::raw_string_ostream VerificationStream(VerificationMessage);
      const bool Broken = ::llvm::verifyModule(ModuleValue, &VerificationStream);
      VerificationStream.flush();
      EXPECT_FALSE(Broken) << VerificationMessage;
    }

    void expectLifecycleEntry(const ::llvm::GlobalVariable &Global, const ::llvm::Function &ExpectedFunction)
    {
      const auto *Entries = ::llvm::dyn_cast_or_null<::llvm::ConstantArray>(Global.getInitializer());
      ASSERT_NE(Entries, nullptr);
      ASSERT_EQ(Entries->getNumOperands(), 1U);
      const auto *Entry = ::llvm::dyn_cast_or_null<::llvm::ConstantStruct>(Entries->getAggregateElement(0U));
      ASSERT_NE(Entry, nullptr);
      ASSERT_EQ(Entry->getNumOperands(), 3U);
      const auto *Priority = ::llvm::dyn_cast<::llvm::ConstantInt>(Entry->getOperand(0));
      const auto *Data = ::llvm::dyn_cast<::llvm::ConstantPointerNull>(Entry->getOperand(2));
      ASSERT_NE(Priority, nullptr);
      ASSERT_NE(Data, nullptr);
      EXPECT_EQ(Priority->getZExtValue(), 65535U);
      EXPECT_EQ(Entry->getOperand(1), &ExpectedFunction);
    }

    void expectBackendFailure(const LoweringResult &Result, const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind, std::string_view MessageFragment)
    {
      EXPECT_FALSE(Result.succeeded());
      EXPECT_EQ(Result.module(), nullptr);
      EXPECT_TRUE(hasDiagnostic(Diagnostics, Kind));
      EXPECT_NE(firstDiagnosticMessage(Diagnostics).find(MessageFragment), std::string::npos);
    }

    // Verifies that LoweringResult transfers module ownership through move construction, move assignment, and takeModule while leaving moved-from and consumed results unsuccessful.
    TEST(LLVMBackendResultTest, TransfersModuleOwnershipAcrossMovesAndTake)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder FirstBuilder(Context.IR);
      FirstBuilder.setModuleName("application.first");
      ir::Module FirstModule = FirstBuilder.takeModule();
      LoweringResult First = lowerToLLVMIR(Context.LLVM, FirstModule);
      ASSERT_TRUE(First.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      ::llvm::Module *FirstPointer = First.module();
      ASSERT_NE(FirstPointer, nullptr);

      LoweringResult Moved(std::move(First));
      EXPECT_FALSE(First.succeeded());
      EXPECT_EQ(First.module(), nullptr);
      ASSERT_TRUE(Moved.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      EXPECT_EQ(Moved.module(), FirstPointer);

      ir::IRBuilder SecondBuilder(Context.IR);
      SecondBuilder.setModuleName("application.second");
      ir::Module SecondModule = SecondBuilder.takeModule();
      LoweringResult Assigned = lowerToLLVMIR(Context.LLVM, SecondModule);
      ASSERT_TRUE(Assigned.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      ASSERT_NE(Assigned.module(), nullptr);

      Assigned = std::move(Moved);
      EXPECT_FALSE(Moved.succeeded());
      EXPECT_EQ(Moved.module(), nullptr);
      ASSERT_TRUE(Assigned.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      EXPECT_EQ(Assigned.module(), FirstPointer);

      std::unique_ptr<::llvm::Module> Taken = Assigned.takeModule();
      ASSERT_NE(Taken, nullptr);
      EXPECT_EQ(Taken.get(), FirstPointer);
      EXPECT_EQ(Taken->getModuleIdentifier(), "application.first");
      EXPECT_FALSE(Assigned.succeeded());
      EXPECT_EQ(Assigned.module(), nullptr);
      EXPECT_TRUE(Context.Diagnostics.diagnostics().empty());
      expectValidLLVMModule(*Taken);
    }

    // Verifies that alloca, slice projections, scalar GEP, load, store, and lifetime.end lower together into verifier-clean LLVM IR.
    TEST(LLVMMemoryLoweringTest, LowersAllocationSlicesAndMemoryAccess)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerSizeType = Context.IR.getType(ir::TypeKind::PointerSize);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::FunctionId Main = Builder.createFunction("main", I32Type);
      const std::optional<ir::BlockId> Entry = Builder.createBlock(Main, "entry");
      ASSERT_TRUE(Entry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Main, *Entry));
      ir::AllocaInstruction *Storage = Builder.createAlloca(ByteSliceType, Builder.getIntegerConstant(PointerSizeType, 8));
      ASSERT_NE(Storage, nullptr);
      ir::SliceDataInstruction *Data = Builder.createSliceData(Builder.createValueOperand(ByteSliceType, Storage->Result));
      ASSERT_NE(Data, nullptr);
      ir::SliceLengthInstruction *Length = Builder.createSliceLength(Builder.createValueOperand(ByteSliceType, Storage->Result));
      ASSERT_NE(Length, nullptr);
      ir::GetElementPointerInstruction *Element = Builder.createGetElementPointer(I32Type, Builder.createValueOperand(BytePointerType, Data->Result), Builder.getIntegerConstant(PointerSizeType, 0));
      ASSERT_NE(Element, nullptr);
      ASSERT_NE(Builder.createStore(Builder.getIntegerConstant(I32Type, 42), Builder.createValueOperand(BytePointerType, Element->Result)), nullptr);
      ir::LoadInstruction *Loaded = Builder.createLoad(I32Type, Builder.createValueOperand(BytePointerType, Element->Result));
      ASSERT_NE(Loaded, nullptr);
      ASSERT_NE(Builder.createLifetimeEnd(Builder.createValueOperand(ByteSliceType, Storage->Result)), nullptr);
      ASSERT_NE(Builder.createReturn(Builder.createValueOperand(I32Type, Loaded->Result)), nullptr);
      EXPECT_TRUE(Length->Result.valid());

      ir::Module ModuleValue = Builder.takeModule();
      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      ASSERT_TRUE(Result.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      ASSERT_NE(Result.module(), nullptr);
      expectValidLLVMModule(*Result.module());
      const ::llvm::Function *MainFunction = Result.module()->getFunction("main");
      ASSERT_NE(MainFunction, nullptr);
      bool SawAlloca = false;
      bool SawGetElementPointer = false;
      bool SawLoad = false;
      bool SawStore = false;
      bool SawLifetimeEnd = false;
      std::size_t NamedSliceProjections = 0;
      for (const ::llvm::BasicBlock &Block : *MainFunction)
      {
        for (const ::llvm::Instruction &Instruction : Block)
        {
          SawAlloca = SawAlloca || ::llvm::isa<::llvm::AllocaInst>(Instruction);
          SawGetElementPointer = SawGetElementPointer || ::llvm::isa<::llvm::GetElementPtrInst>(Instruction);
          SawLoad = SawLoad || ::llvm::isa<::llvm::LoadInst>(Instruction);
          SawStore = SawStore || ::llvm::isa<::llvm::StoreInst>(Instruction);
          if (const auto *Intrinsic = ::llvm::dyn_cast<::llvm::IntrinsicInst>(&Instruction))
          {
            SawLifetimeEnd = SawLifetimeEnd || Intrinsic->getIntrinsicID() == ::llvm::Intrinsic::lifetime_end;
          }
          if (::llvm::isa<::llvm::ExtractValueInst>(Instruction) && (Instruction.getName() == "v1" || Instruction.getName() == "v2"))
          {
            ++NamedSliceProjections;
          }
        }
      }
      EXPECT_TRUE(SawAlloca);
      EXPECT_TRUE(SawGetElementPointer);
      EXPECT_TRUE(SawLoad);
      EXPECT_TRUE(SawStore);
      EXPECT_TRUE(SawLifetimeEnd);
      EXPECT_EQ(NamedSliceProjections, 2U);
    }

    // Verifies that a dynamically sized InkIR alloca zero-initializes every allocated byte before exposing the resulting slice.
    TEST(LLVMMemoryLoweringTest, ZeroInitializesDynamicAllocation)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      const ir::Type &VoidType = Context.IR.getType(ir::TypeKind::Void);
      const ir::Type &PointerSizeType = Context.IR.getType(ir::TypeKind::PointerSize);
      const ir::Type &ByteSliceType = Context.IR.getType(ir::TypeKind::ByteSlice);
      const ir::FunctionId Allocate = Builder.createFunction("allocate_zeroed", VoidType, {ir::Parameter(&PointerSizeType)});
      const std::optional<ir::BlockId> Entry = Builder.createBlock(Allocate, "entry");
      ASSERT_TRUE(Entry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Allocate, *Entry));
      ir::AllocaInstruction *Allocation = Builder.createAlloca(ByteSliceType, Builder.createValueOperand(PointerSizeType, ir::ValueId{0}));
      ASSERT_NE(Allocation, nullptr);
      ASSERT_NE(Builder.createReturn(), nullptr);

      ir::Module ModuleValue = Builder.takeModule();
      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      ASSERT_TRUE(Result.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      ASSERT_NE(Result.module(), nullptr);
      expectValidLLVMModule(*Result.module());
      ::llvm::Function *FunctionValue = Result.module()->getFunction("allocate_zeroed");
      ASSERT_NE(FunctionValue, nullptr);
      ::llvm::AllocaInst *Storage = nullptr;
      ::llvm::MemSetInst *Initialization = nullptr;
      for (::llvm::Instruction &InstructionValue : FunctionValue->getEntryBlock())
      {
        if (Storage == nullptr)
        {
          Storage = ::llvm::dyn_cast<::llvm::AllocaInst>(&InstructionValue);
        }
        if (Initialization == nullptr)
        {
          Initialization = ::llvm::dyn_cast<::llvm::MemSetInst>(&InstructionValue);
        }
      }
      ASSERT_NE(Storage, nullptr);
      ASSERT_NE(Initialization, nullptr);
      EXPECT_EQ(Initialization->getDest(), Storage);
      EXPECT_EQ(Initialization->getLength(), FunctionValue->getArg(0));
      const auto *Fill = ::llvm::dyn_cast<::llvm::ConstantInt>(Initialization->getValue());
      ASSERT_NE(Fill, nullptr);
      EXPECT_TRUE(Fill->isZero());
      EXPECT_TRUE(Storage->comesBefore(Initialization));
    }

    // Verifies that nested struct memory operations touch only leaf fields so stores preserve padding and loads rebuild both aggregate levels.
    TEST(LLVMMemoryLoweringTest, PreservesNestedStructPaddingWithLeafMemoryOperations)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      const ir::StructType &InnerType = Builder.createStructType("MemoryInner", {ir::StructField("tag", &ByteType), ir::StructField("value", &I32Type)});
      const ir::StructType &OuterType = Builder.createStructType("MemoryOuter", {ir::StructField("prefix", &ByteType), ir::StructField("payload", &InnerType), ir::StructField("suffix", &ByteType)});
      const ir::FunctionId RoundTrip = Builder.createFunction("round_trip_nested_struct", OuterType, {ir::Parameter(&OuterType), ir::Parameter(&BytePointerType)});
      const std::optional<ir::BlockId> Entry = Builder.createBlock(RoundTrip, "entry");
      ASSERT_TRUE(Entry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(RoundTrip, *Entry));
      ASSERT_NE(Builder.createStore(Builder.createValueOperand(OuterType, ir::ValueId{0}), Builder.createValueOperand(BytePointerType, ir::ValueId{1})), nullptr);
      ir::LoadInstruction *Loaded = Builder.createLoad(OuterType, Builder.createValueOperand(BytePointerType, ir::ValueId{1}));
      ASSERT_NE(Loaded, nullptr);
      ASSERT_NE(Builder.createReturn(Builder.createValueOperand(OuterType, Loaded->Result)), nullptr);

      ir::Module ModuleValue = Builder.takeModule();
      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      ASSERT_TRUE(Result.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      ASSERT_NE(Result.module(), nullptr);
      expectValidLLVMModule(*Result.module());
      const ::llvm::Function *FunctionValue = Result.module()->getFunction("round_trip_nested_struct");
      ASSERT_NE(FunctionValue, nullptr);
      std::size_t LeafStores = 0;
      std::size_t LeafLoads = 0;
      std::size_t ExtractValues = 0;
      std::size_t InsertValues = 0;
      bool SawAggregateStore = false;
      bool SawAggregateLoad = false;
      const ::llvm::ReturnInst *Return = nullptr;
      for (const ::llvm::Instruction &InstructionValue : FunctionValue->getEntryBlock())
      {
        if (const auto *Store = ::llvm::dyn_cast<::llvm::StoreInst>(&InstructionValue))
        {
          ++LeafStores;
          SawAggregateStore = SawAggregateStore || Store->getValueOperand()->getType()->isStructTy();
        }
        if (const auto *Load = ::llvm::dyn_cast<::llvm::LoadInst>(&InstructionValue))
        {
          ++LeafLoads;
          SawAggregateLoad = SawAggregateLoad || Load->getType()->isStructTy();
        }
        ExtractValues += ::llvm::isa<::llvm::ExtractValueInst>(InstructionValue) ? 1U : 0U;
        InsertValues += ::llvm::isa<::llvm::InsertValueInst>(InstructionValue) ? 1U : 0U;
        if (const auto *Candidate = ::llvm::dyn_cast<::llvm::ReturnInst>(&InstructionValue))
        {
          Return = Candidate;
        }
      }
      EXPECT_EQ(LeafStores, 4U);
      EXPECT_EQ(LeafLoads, 4U);
      EXPECT_EQ(ExtractValues, 5U);
      EXPECT_EQ(InsertValues, 5U);
      EXPECT_FALSE(SawAggregateStore);
      EXPECT_FALSE(SawAggregateLoad);
      ASSERT_NE(Return, nullptr);
      EXPECT_TRUE(::llvm::isa<::llvm::InsertValueInst>(Return->getReturnValue()));
    }

    // Verifies that nested GEP field paths use physical indices produced for explicit-offset custom struct layouts.
    TEST(LLVMMemoryLoweringTest, LowersNestedCustomLayoutStructGetElementPointer)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      const ir::Type &VoidType = Context.IR.getType(ir::TypeKind::Void);
      const ir::Type &ByteType = Context.IR.getType(ir::TypeKind::Byte);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerSizeType = Context.IR.getType(ir::TypeKind::PointerSize);
      const ir::Type &BytePointerType = Context.IR.getType(ir::TypeKind::BytePointer);
      ir::FieldLayoutConstraints InnerValueConstraints;
      InnerValueConstraints.ExplicitAlignment = 8;
      InnerValueConstraints.ExplicitOffset = 8;
      std::vector<ir::StructField> InnerFields;
      InnerFields.emplace_back("tag", &ByteType);
      InnerFields.emplace_back("value", &I32Type, std::vector<ir::Attribute>{}, InnerValueConstraints);
      const ir::StructType &InnerType = Builder.createStructType("Inner", std::move(InnerFields));
      const ir::StructType &OuterType = Builder.createStructType("Outer", {ir::StructField("prefix", &ByteType), ir::StructField("payload", &InnerType)});
      const ir::FunctionId Locate = Builder.createFunction("locate", VoidType, {ir::Parameter(&BytePointerType), ir::Parameter(&PointerSizeType)});
      const std::optional<ir::BlockId> Entry = Builder.createBlock(Locate, "entry");
      ASSERT_TRUE(Entry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Locate, *Entry));
      std::vector<ir::ValueHandle> FieldIndices;
      FieldIndices.emplace_back(Builder.getIntegerConstant(I32Type, 1));
      FieldIndices.emplace_back(Builder.getIntegerConstant(I32Type, 1));
      ASSERT_NE(Builder.createGetElementPointer(OuterType, Builder.createValueOperand(BytePointerType, ir::ValueId{0}), Builder.createValueOperand(PointerSizeType, ir::ValueId{1}), std::move(FieldIndices)), nullptr);
      ASSERT_NE(Builder.createReturn(), nullptr);

      ir::Module ModuleValue = Builder.takeModule();
      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      ASSERT_TRUE(Result.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      ASSERT_NE(Result.module(), nullptr);
      expectValidLLVMModule(*Result.module());
      const ::llvm::Function *LocateFunction = Result.module()->getFunction("locate");
      ASSERT_NE(LocateFunction, nullptr);
      const ::llvm::GetElementPtrInst *Pointer = nullptr;
      for (const ::llvm::Instruction &Instruction : LocateFunction->getEntryBlock())
      {
        if (const auto *Candidate = ::llvm::dyn_cast<::llvm::GetElementPtrInst>(&Instruction))
        {
          Pointer = Candidate;
          break;
        }
      }
      ASSERT_NE(Pointer, nullptr);
      EXPECT_EQ(Pointer->getNumIndices(), 4U);
      auto Index = Pointer->idx_begin();
      ++Index;
      const auto *OuterField = ::llvm::dyn_cast<::llvm::ConstantInt>(Index->get());
      ++Index;
      const auto *InnerPayload = ::llvm::dyn_cast<::llvm::ConstantInt>(Index->get());
      ++Index;
      const auto *InnerField = ::llvm::dyn_cast<::llvm::ConstantInt>(Index->get());
      ASSERT_NE(OuterField, nullptr);
      ASSERT_NE(InnerPayload, nullptr);
      ASSERT_NE(InnerField, nullptr);
      EXPECT_EQ(OuterField->getZExtValue(), 1U);
      EXPECT_EQ(InnerPayload->getZExtValue(), 0U);
      EXPECT_EQ(InnerField->getZExtValue(), 2U);
      const auto *LoweredOuter = ::llvm::dyn_cast<::llvm::StructType>(Pointer->getSourceElementType());
      ASSERT_NE(LoweredOuter, nullptr);
      EXPECT_FALSE(LoweredOuter->isPacked());
      EXPECT_EQ(LoweredOuter->getNumElements(), 2U);
      const auto *LoweredInner = ::llvm::dyn_cast<::llvm::StructType>(LoweredOuter->getElementType(1));
      ASSERT_NE(LoweredInner, nullptr);
      EXPECT_FALSE(LoweredInner->isPacked());
      ASSERT_EQ(LoweredInner->getNumElements(), 2U);
      const auto *LoweredInnerPayload = ::llvm::dyn_cast<::llvm::StructType>(LoweredInner->getElementType(0));
      ASSERT_NE(LoweredInnerPayload, nullptr);
      EXPECT_TRUE(LoweredInnerPayload->isPacked());
      EXPECT_EQ(LoweredInnerPayload->getNumElements(), 4U);
    }

    // Verifies that InkIR initializer and finalizer functions become verifier-clean LLVM global constructor and destructor entries.
    TEST(LLVMLifecycleLoweringTest, EmitsGlobalConstructorsAndDestructors)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      const ir::Type &VoidType = Context.IR.getType(ir::TypeKind::Void);
      const ir::FunctionId Initialize = Builder.createFunction("initialize", VoidType);
      const std::optional<ir::BlockId> InitializeEntry = Builder.createBlock(Initialize, "entry");
      ASSERT_TRUE(InitializeEntry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Initialize, *InitializeEntry));
      ASSERT_NE(Builder.createReturn(), nullptr);
      const ir::FunctionId Finalize = Builder.createFunction("finalize", VoidType);
      const std::optional<ir::BlockId> FinalizeEntry = Builder.createBlock(Finalize, "entry");
      ASSERT_TRUE(FinalizeEntry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Finalize, *FinalizeEntry));
      ASSERT_NE(Builder.createReturn(), nullptr);
      ASSERT_TRUE(Builder.setInitializer(Initialize));
      ASSERT_TRUE(Builder.setFinalizer(Finalize));

      ir::Module ModuleValue = Builder.takeModule();
      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      ASSERT_TRUE(Result.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      ASSERT_NE(Result.module(), nullptr);
      expectValidLLVMModule(*Result.module());
      const ::llvm::GlobalVariable *Constructors = Result.module()->getNamedGlobal("llvm.global_ctors");
      const ::llvm::GlobalVariable *Destructors = Result.module()->getNamedGlobal("llvm.global_dtors");
      ASSERT_NE(Constructors, nullptr);
      ASSERT_NE(Destructors, nullptr);
      EXPECT_TRUE(Constructors->hasAppendingLinkage());
      EXPECT_TRUE(Destructors->hasAppendingLinkage());
      EXPECT_TRUE(Constructors->hasInitializer());
      EXPECT_TRUE(Destructors->hasInitializer());
      const ::llvm::Function *InitializeFunction = Result.module()->getFunction("initialize");
      const ::llvm::Function *FinalizeFunction = Result.module()->getFunction("finalize");
      ASSERT_NE(InitializeFunction, nullptr);
      ASSERT_NE(FinalizeFunction, nullptr);
      expectLifecycleEntry(*Constructors, *InitializeFunction);
      expectLifecycleEntry(*Destructors, *FinalizeFunction);
    }

    // Verifies that invalid InkIR stops before lowering and preserves the verifier's original diagnostic without wrapping it as a backend failure.
    TEST(LLVMFailureLoweringTest, PropagatesInkIRVerifierDiagnostics)
    {
      LLVMBackendTestContext Context;
      core::CollectingDiagnosticConsumer Consumer;
      Context.Compilation.diagnosticEngine().addConsumer(Consumer);
      ir::IRBuilder Builder(Context.IR);
      Builder.createFunction("missing_body", Context.IR.getType(ir::TypeKind::Void));
      ir::Module ModuleValue = Builder.takeModule();

      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);
      Context.Compilation.diagnosticEngine().removeConsumer(Consumer);

      EXPECT_FALSE(Result.succeeded());
      EXPECT_EQ(Result.module(), nullptr);
      EXPECT_TRUE(hasDiagnostic(Consumer.diagnostics(), core::DiagnosticKind::IrDefinedFunctionHasNoBasicBlocks));
    }

    // Verifies that a verifier-valid imported global is rejected through DiagnosticEngine with structured symbol information.
    TEST(LLVMFailureLoweringTest, RejectsImportedGlobal)
    {
      LLVMBackendTestContext Context;
      core::CollectingDiagnosticConsumer Consumer;
      ir::IRBuilder Builder(Context.IR);
      Builder.setModuleName("application.main");
      ir::GlobalVariable Imported;
      Imported.Name = "counter";
      Imported.ValueType = &Context.IR.getType(ir::TypeKind::I32);
      Imported.Kind = ir::GlobalVariableKind::Imported;
      Imported.Import = ir::ImportInfo{"library.data", "counter"};
      Builder.addGlobal(std::move(Imported));
      ir::Module ModuleValue = Builder.takeModule();
      ASSERT_TRUE(ir::verify(ModuleValue).succeeded());

      Context.Compilation.diagnosticEngine().addConsumer(Consumer);
      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);
      Context.Compilation.diagnosticEngine().removeConsumer(Consumer);

      expectBackendFailure(Result, Consumer.diagnostics(), core::DiagnosticKind::LLVMImportedGlobalUnsupported, "imported global @counter");
      ASSERT_EQ(Consumer.diagnostics().size(), 1U);
      ASSERT_EQ(Consumer.diagnostics().front().Arguments.size(), 1U);
      EXPECT_EQ(Consumer.diagnostics().front().Arguments.front().Name, core::DiagnosticArgumentName::SymbolName);
      const std::string *SymbolName = std::get_if<std::string>(&Consumer.diagnostics().front().Arguments.front().Value);
      ASSERT_NE(SymbolName, nullptr);
      EXPECT_EQ(*SymbolName, "counter");
    }

    // Verifies that a verifier-valid imported function is rejected explicitly because LLVM lowering requires Closed InkIR.
    TEST(LLVMFailureLoweringTest, RejectsImportedFunction)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      Builder.setModuleName("application.main");
      ir::Function Imported(Context.IR.getType(ir::TypeKind::Void));
      Imported.Name = "hook";
      Imported.Kind = ir::FunctionKind::Imported;
      Imported.Import = ir::ImportInfo{"library.runtime", "hook"};
      Builder.addFunction(std::move(Imported));
      ir::Module ModuleValue = Builder.takeModule();
      ASSERT_TRUE(ir::verify(ModuleValue).succeeded());

      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      expectBackendFailure(Result, Context.Diagnostics.diagnostics(), core::DiagnosticKind::LLVMImportedFunctionUnsupported, "imported function @hook");
    }

    // Verifies that a verifier-valid runtime import instruction is rejected explicitly instead of being silently discarded.
    TEST(LLVMFailureLoweringTest, RejectsRuntimeImportInstruction)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      Builder.setModuleName("application.main");
      const ir::FunctionId Main = Builder.createFunction("main", Context.IR.getType(ir::TypeKind::Void));
      const std::optional<ir::BlockId> Entry = Builder.createBlock(Main, "entry");
      ASSERT_TRUE(Entry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Main, *Entry));
      ASSERT_NE(Builder.createImport("library.runtime"), nullptr);
      ASSERT_NE(Builder.createReturn(), nullptr);
      ir::Module ModuleValue = Builder.takeModule();
      ASSERT_TRUE(ir::verify(ModuleValue).succeeded());

      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      expectBackendFailure(Result, Context.Diagnostics.diagnostics(), core::DiagnosticKind::LLVMRuntimeModuleImportUnsupported, "runtime import of module library.runtime");
    }

    // Verifies that an InkIR alignment wider than LLVM can encode becomes a backend diagnostic instead of reaching an LLVM alignment assertion.
    TEST(LLVMFailureLoweringTest, RejectsUnrepresentableStructAlignment)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      ir::StructLayoutConstraints Constraints;
      Constraints.ExplicitAlignment = 536870912U;
      Builder.createStructType("Overaligned", {ir::StructField("value", &Context.IR.getType(ir::TypeKind::Byte))}, Constraints);
      ir::Module ModuleValue = Builder.takeModule();
      ASSERT_TRUE(ir::verify(ModuleValue).succeeded());

      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      expectBackendFailure(Result, Context.Diagnostics.diagnostics(), core::DiagnosticKind::LLVMStructAlignmentUnrepresentable, "struct alignment 536870912 cannot be represented by LLVM");
    }

    // Verifies that currently unsupported function and field metadata attributes are ignored while their InkIR owners still lower successfully.
    TEST(LLVMAttributeLoweringTest, IgnoresAttributes)
    {
      LLVMBackendTestContext Context;
      ir::IRBuilder Builder(Context.IR);
      const ir::Type &VoidType = Context.IR.getType(ir::TypeKind::Void);
      const ir::Type &I32Type = Context.IR.getType(ir::TypeKind::I32);
      const ir::IntegerConstant &Version = Builder.getIntegerConstant(I32Type, 1);
      std::vector<ir::AttributeArgument> FieldArguments;
      FieldArguments.emplace_back("Version", Version);
      std::vector<ir::Attribute> FieldAttributes;
      FieldAttributes.emplace_back(ir::AttributeKind::Reflect, std::move(FieldArguments));
      FieldAttributes.emplace_back(ir::AttributeKind::Stored);
      Builder.createStructType("Attributed", {ir::StructField("value", &I32Type, std::move(FieldAttributes))});
      std::vector<ir::AttributeArgument> FunctionArguments;
      FunctionArguments.emplace_back("Version", Version);
      std::vector<ir::Attribute> FunctionAttributes;
      FunctionAttributes.emplace_back(ir::AttributeKind::Reflect, std::move(FunctionArguments));
      FunctionAttributes.emplace_back(ir::AttributeKind::Serialize);
      const ir::FunctionId Annotated = Builder.createFunction("annotated", VoidType, {}, std::move(FunctionAttributes));
      const std::optional<ir::BlockId> Entry = Builder.createBlock(Annotated, "entry");
      ASSERT_TRUE(Entry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Annotated, *Entry));
      ASSERT_NE(Builder.createReturn(), nullptr);

      ir::Module ModuleValue = Builder.takeModule();
      LoweringResult Result = lowerToLLVMIR(Context.LLVM, ModuleValue);

      ASSERT_TRUE(Result.succeeded()) << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
      ASSERT_NE(Result.module(), nullptr);
      expectValidLLVMModule(*Result.module());
      const ::llvm::Function *FunctionValue = Result.module()->getFunction("annotated");
      ASSERT_NE(FunctionValue, nullptr);
      EXPECT_TRUE(FunctionValue->getAttributes().isEmpty());
    }
  } // namespace
} // namespace ink::backend::llvm
