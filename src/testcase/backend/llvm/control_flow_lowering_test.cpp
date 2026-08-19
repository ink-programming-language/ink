#include "ink/backend/llvm/llvm_backend.h"
#include "ink/core/context.h"
#include "ink/ir/model/context.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ink::backend::llvm
{
  namespace
  {
    struct LLVMControlFlowLoweringTestContext
    {
        LLVMControlFlowLoweringTestContext()
        {
          Compilation.diagnosticEngine().addConsumer(Diagnostics);
        }

        ~LLVMControlFlowLoweringTestContext()
        {
          Compilation.diagnosticEngine().removeConsumer(Diagnostics);
        }

        core::CompilationContext Compilation;
        ir::IRContext IR{Compilation};
        ::llvm::LLVMContext LLVM;
        core::CollectingDiagnosticConsumer Diagnostics;
    };

    std::string firstDiagnosticMessage(const std::vector<core::Diagnostic> &Diagnostics)
    {
      return Diagnostics.empty() ? std::string{} : core::DiagnosticFormatter().format(Diagnostics.front()).Message;
    }

    std::unique_ptr<::llvm::Module> lowerVerifiedModule(LLVMControlFlowLoweringTestContext &Context, std::string_view Text)
    {
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      if (!Parsed.succeeded())
      {
        ADD_FAILURE() << "expected test InkIR to deserialize, but received " << Context.Diagnostics.diagnostics().size() << " diagnostics";
        return nullptr;
      }

      LoweringResult Lowered = lowerToLLVMIR(Context.LLVM, *Parsed.module());
      if (!Lowered.succeeded())
      {
        ADD_FAILURE() << "expected InkIR lowering to succeed: " << firstDiagnosticMessage(Context.Diagnostics.diagnostics());
        return nullptr;
      }

      std::string VerificationMessage;
      ::llvm::raw_string_ostream VerificationStream(VerificationMessage);
      if (::llvm::verifyModule(*Lowered.module(), &VerificationStream))
      {
        VerificationStream.flush();
        ADD_FAILURE() << "expected lowered LLVM IR to verify: " << VerificationMessage;
        return nullptr;
      }
      return Lowered.takeModule();
    }

    ::llvm::BasicBlock *findBlock(::llvm::Function &FunctionValue, const char *Name)
    {
      for (::llvm::BasicBlock &Block : FunctionValue)
      {
        if (Block.getName() == Name)
        {
          return &Block;
        }
      }
      return nullptr;
    }

    ::llvm::Instruction *findInstruction(::llvm::Function &FunctionValue, const char *Name)
    {
      for (::llvm::BasicBlock &Block : FunctionValue)
      {
        for (::llvm::Instruction &InstructionValue : Block)
        {
          if (InstructionValue.getName() == Name)
          {
            return &InstructionValue;
          }
        }
      }
      return nullptr;
    }

    // Verifies that InkIR definitions and an external declaration become LLVM definitions and a declaration while direct calls retain both target identities.
    TEST(LLVMControlFlowLoweringTest, LowersFunctionDefinitionsDeclarationAndExternalCall)
    {
      LLVMControlFlowLoweringTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "declare extern \"C\" i32 @external_transform(i32)\n"
          "define i32 @increment(i32 %0) {\n"
          "entry:\n"
          "  %1 = add i32 %0, i32 1\n"
          "  ret i32 %1\n"
          "}\n"
          "define i32 @caller(i32 %0) {\n"
          "entry:\n"
          "  %1 = call i32 @increment(i32 %0)\n"
          "  %2 = call i32 @external_transform(i32 %1)\n"
          "  ret i32 %2\n"
          "}\n";

      std::unique_ptr<::llvm::Module> ModuleValue = lowerVerifiedModule(Context, Text);

      ASSERT_NE(ModuleValue, nullptr);
      ::llvm::Function *External = ModuleValue->getFunction("external_transform");
      ::llvm::Function *Increment = ModuleValue->getFunction("increment");
      ::llvm::Function *Caller = ModuleValue->getFunction("caller");
      ASSERT_NE(External, nullptr);
      ASSERT_NE(Increment, nullptr);
      ASSERT_NE(Caller, nullptr);
      EXPECT_TRUE(External->isDeclaration());
      EXPECT_FALSE(Increment->isDeclaration());
      EXPECT_FALSE(Caller->isDeclaration());
      auto *DefinedCall = ::llvm::dyn_cast_or_null<::llvm::CallInst>(findInstruction(*Caller, "v1"));
      auto *ExternalCall = ::llvm::dyn_cast_or_null<::llvm::CallInst>(findInstruction(*Caller, "v2"));
      ASSERT_NE(DefinedCall, nullptr);
      ASSERT_NE(ExternalCall, nullptr);
      EXPECT_EQ(DefinedCall->getCalledFunction(), Increment);
      EXPECT_EQ(ExternalCall->getCalledFunction(), External);
    }

    // Verifies that i32 addition over SSA parameters lowers to a named LLVM integer add rather than losing the operation during value mapping.
    TEST(LLVMControlFlowLoweringTest, LowersIntegerAddition)
    {
      LLVMControlFlowLoweringTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @add_values(i32 %0, i32 %1) {\n"
          "entry:\n"
          "  %2 = add i32 %0, i32 %1\n"
          "  ret i32 %2\n"
          "}\n";

      std::unique_ptr<::llvm::Module> ModuleValue = lowerVerifiedModule(Context, Text);

      ASSERT_NE(ModuleValue, nullptr);
      ::llvm::Function *FunctionValue = ModuleValue->getFunction("add_values");
      ASSERT_NE(FunctionValue, nullptr);
      auto *Add = ::llvm::dyn_cast_or_null<::llvm::BinaryOperator>(findInstruction(*FunctionValue, "v2"));
      ASSERT_NE(Add, nullptr);
      EXPECT_EQ(Add->getOpcode(), ::llvm::Instruction::Add);
      EXPECT_TRUE(Add->getType()->isIntegerTy(32));
    }

    // Verifies every integer comparison predicate, including signed i32 ordering and unsigned byte and target-sized ordering.
    TEST(LLVMControlFlowLoweringTest, SelectsEveryIntegerComparisonPredicate)
    {
      LLVMControlFlowLoweringTestContext Context;
      struct ComparisonCase
      {
          const char *TypeName;
          const char *PredicateName;
          ::llvm::CmpInst::Predicate ExpectedPredicate;
          unsigned ExpectedWidth;
      };
      const unsigned PointerWidth = static_cast<unsigned>(Context.Compilation.targetContext().pointerWidth());
      const ComparisonCase Cases[] = {
          {"i32", "eq", ::llvm::CmpInst::ICMP_EQ, 32},
          {"i32", "ne", ::llvm::CmpInst::ICMP_NE, 32},
          {"i32", "lt", ::llvm::CmpInst::ICMP_SLT, 32},
          {"i32", "le", ::llvm::CmpInst::ICMP_SLE, 32},
          {"i32", "gt", ::llvm::CmpInst::ICMP_SGT, 32},
          {"i32", "ge", ::llvm::CmpInst::ICMP_SGE, 32},
          {"byte", "lt", ::llvm::CmpInst::ICMP_ULT, 8},
          {"byte", "le", ::llvm::CmpInst::ICMP_ULE, 8},
          {"byte", "gt", ::llvm::CmpInst::ICMP_UGT, 8},
          {"byte", "ge", ::llvm::CmpInst::ICMP_UGE, 8},
          {"ptrsize", "lt", ::llvm::CmpInst::ICMP_ULT, PointerWidth},
          {"ptrsize", "le", ::llvm::CmpInst::ICMP_ULE, PointerWidth},
          {"ptrsize", "gt", ::llvm::CmpInst::ICMP_UGT, PointerWidth},
          {"ptrsize", "ge", ::llvm::CmpInst::ICMP_UGE, PointerWidth},
      };

      for (const ComparisonCase &CaseValue : Cases)
      {
        const std::string Text = "inkir 1\ndefine bool @compare(" + std::string(CaseValue.TypeName) + " %0, " + CaseValue.TypeName + " %1) {\nentry:\n  %2 = icmp " + CaseValue.PredicateName + " " + CaseValue.TypeName + " %0, " + CaseValue.TypeName + " %1\n  ret bool %2\n}\n";
        std::unique_ptr<::llvm::Module> ModuleValue = lowerVerifiedModule(Context, Text);
        ASSERT_NE(ModuleValue, nullptr);
        ::llvm::Function *FunctionValue = ModuleValue->getFunction("compare");
        ASSERT_NE(FunctionValue, nullptr);
        auto *Compare = ::llvm::dyn_cast_or_null<::llvm::ICmpInst>(findInstruction(*FunctionValue, "v2"));
        ASSERT_NE(Compare, nullptr);
        EXPECT_EQ(Compare->getPredicate(), CaseValue.ExpectedPredicate);
        EXPECT_TRUE(Compare->getOperand(0)->getType()->isIntegerTy(CaseValue.ExpectedWidth));
      }
    }

    // Verifies that mutable and const opaque byte pointers lower to LLVM pointer equality and inequality without introducing ordered pointer predicates.
    TEST(LLVMControlFlowLoweringTest, LowersPointerEqualityPredicates)
    {
      LLVMControlFlowLoweringTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define bool @same_pointer(byte* %0, byte* %1) {\n"
          "entry:\n"
          "  %2 = icmp eq byte* %0, byte* %1\n"
          "  ret bool %2\n"
          "}\n"
          "define bool @different_const_pointer(const byte* %0, const byte* %1) {\n"
          "entry:\n"
          "  %2 = icmp ne const byte* %0, const byte* %1\n"
          "  ret bool %2\n"
          "}\n";
      struct ComparisonCase
      {
          const char *FunctionName;
          ::llvm::CmpInst::Predicate ExpectedPredicate;
      };
      const ComparisonCase Cases[] = {
          {"same_pointer", ::llvm::CmpInst::ICMP_EQ},
          {"different_const_pointer", ::llvm::CmpInst::ICMP_NE},
      };

      std::unique_ptr<::llvm::Module> ModuleValue = lowerVerifiedModule(Context, Text);

      ASSERT_NE(ModuleValue, nullptr);
      for (const ComparisonCase &CaseValue : Cases)
      {
        ::llvm::Function *FunctionValue = ModuleValue->getFunction(CaseValue.FunctionName);
        ASSERT_NE(FunctionValue, nullptr);
        auto *Compare = ::llvm::dyn_cast_or_null<::llvm::ICmpInst>(findInstruction(*FunctionValue, "v2"));
        ASSERT_NE(Compare, nullptr);
        EXPECT_EQ(Compare->getPredicate(), CaseValue.ExpectedPredicate);
        EXPECT_TRUE(Compare->getOperand(0)->getType()->isPointerTy());
        EXPECT_TRUE(Compare->getOperand(1)->getType()->isPointerTy());
      }
    }

    // Verifies unconditional and conditional branches, a loop-carried phi backedge, and duplicate edges from one predecessor lower to a verifier-valid LLVM CFG.
    TEST(LLVMControlFlowLoweringTest, LowersBranchesBackedgePhiAndDuplicatePredecessorEdges)
    {
      LLVMControlFlowLoweringTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @loop(bool %0) {\n"
          "entry:\n"
          "  br loop\n"
          "loop:\n"
          "  %1 = phi i32 [0, entry], [%2, body], [%2, body]\n"
          "  condbr bool %0, body, exit\n"
          "body:\n"
          "  %2 = add i32 %1, i32 1\n"
          "  condbr bool %0, loop, loop\n"
          "exit:\n"
          "  ret i32 %1\n"
          "}\n";

      std::unique_ptr<::llvm::Module> ModuleValue = lowerVerifiedModule(Context, Text);

      ASSERT_NE(ModuleValue, nullptr);
      ::llvm::Function *FunctionValue = ModuleValue->getFunction("loop");
      ASSERT_NE(FunctionValue, nullptr);
      ::llvm::BasicBlock *Entry = findBlock(*FunctionValue, "entry");
      ::llvm::BasicBlock *Loop = findBlock(*FunctionValue, "loop");
      ::llvm::BasicBlock *Body = findBlock(*FunctionValue, "body");
      ::llvm::BasicBlock *Exit = findBlock(*FunctionValue, "exit");
      ASSERT_NE(Entry, nullptr);
      ASSERT_NE(Loop, nullptr);
      ASSERT_NE(Body, nullptr);
      ASSERT_NE(Exit, nullptr);
      auto *EntryBranch = ::llvm::dyn_cast<::llvm::BranchInst>(Entry->getTerminator());
      auto *LoopBranch = ::llvm::dyn_cast<::llvm::BranchInst>(Loop->getTerminator());
      auto *BodyBranch = ::llvm::dyn_cast<::llvm::BranchInst>(Body->getTerminator());
      ASSERT_NE(EntryBranch, nullptr);
      ASSERT_NE(LoopBranch, nullptr);
      ASSERT_NE(BodyBranch, nullptr);
      EXPECT_TRUE(EntryBranch->isUnconditional());
      EXPECT_EQ(EntryBranch->getSuccessor(0), Loop);
      EXPECT_TRUE(LoopBranch->isConditional());
      EXPECT_EQ(LoopBranch->getSuccessor(0), Body);
      EXPECT_EQ(LoopBranch->getSuccessor(1), Exit);
      EXPECT_TRUE(BodyBranch->isConditional());
      EXPECT_EQ(BodyBranch->getSuccessor(0), Loop);
      EXPECT_EQ(BodyBranch->getSuccessor(1), Loop);
      auto *Phi = ::llvm::dyn_cast_or_null<::llvm::PHINode>(findInstruction(*FunctionValue, "v1"));
      auto *Add = ::llvm::dyn_cast_or_null<::llvm::BinaryOperator>(findInstruction(*FunctionValue, "v2"));
      ASSERT_NE(Phi, nullptr);
      ASSERT_NE(Add, nullptr);
      ASSERT_EQ(Phi->getNumIncomingValues(), 3U);
      EXPECT_EQ(Phi->getIncomingBlock(0), Entry);
      EXPECT_EQ(Phi->getIncomingBlock(1), Body);
      EXPECT_EQ(Phi->getIncomingBlock(2), Body);
      EXPECT_EQ(Phi->getIncomingValue(1), Add);
      EXPECT_EQ(Phi->getIncomingValue(2), Add);
    }

    // Verifies that a definition in a physically later block still lowers before its dominated use when CFG order differs from block storage order.
    TEST(LLVMControlFlowLoweringTest, LowersDominatingDefinitionFromPhysicallyLaterBlock)
    {
      LLVMControlFlowLoweringTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @out_of_order_blocks(i32 %0) {\n"
          "entry:\n"
          "  br definition\n"
          "use:\n"
          "  %1 = add i32 %2, i32 1\n"
          "  ret i32 %1\n"
          "definition:\n"
          "  %2 = add i32 %0, i32 21\n"
          "  br use\n"
          "}\n";

      std::unique_ptr<::llvm::Module> ModuleValue = lowerVerifiedModule(Context, Text);

      ASSERT_NE(ModuleValue, nullptr);
      ::llvm::Function *FunctionValue = ModuleValue->getFunction("out_of_order_blocks");
      ASSERT_NE(FunctionValue, nullptr);
      auto *Use = ::llvm::dyn_cast_or_null<::llvm::BinaryOperator>(findInstruction(*FunctionValue, "v1"));
      auto *Definition = ::llvm::dyn_cast_or_null<::llvm::BinaryOperator>(findInstruction(*FunctionValue, "v2"));
      ASSERT_NE(Use, nullptr);
      ASSERT_NE(Definition, nullptr);
      EXPECT_EQ(Use->getOperand(0), Definition);
    }

    // Verifies that struct field replacement and retrieval lower to LLVM insertvalue and extractvalue with the same logical field index and SSA chain.
    TEST(LLVMControlFlowLoweringTest, LowersInsertAndExtractValue)
    {
      LLVMControlFlowLoweringTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "%Pair = type {byte, i32}\n"
          "define i32 @replace_and_read(%Pair %0, i32 %1) {\n"
          "entry:\n"
          "  %2 = insertvalue %Pair %0, i32 %1, 1\n"
          "  %3 = extractvalue %Pair %2, 1\n"
          "  ret i32 %3\n"
          "}\n";

      std::unique_ptr<::llvm::Module> ModuleValue = lowerVerifiedModule(Context, Text);

      ASSERT_NE(ModuleValue, nullptr);
      ::llvm::Function *FunctionValue = ModuleValue->getFunction("replace_and_read");
      ASSERT_NE(FunctionValue, nullptr);
      auto *Insert = ::llvm::dyn_cast_or_null<::llvm::InsertValueInst>(findInstruction(*FunctionValue, "v2"));
      auto *Extract = ::llvm::dyn_cast_or_null<::llvm::ExtractValueInst>(findInstruction(*FunctionValue, "v3"));
      ASSERT_NE(Insert, nullptr);
      ASSERT_NE(Extract, nullptr);
      ASSERT_EQ(Insert->getNumIndices(), 1U);
      ASSERT_EQ(Extract->getNumIndices(), 1U);
      EXPECT_EQ(Insert->getIndices()[0], 1U);
      EXPECT_EQ(Extract->getIndices()[0], 1U);
      EXPECT_EQ(Extract->getAggregateOperand(), Insert);
      EXPECT_TRUE(Extract->getType()->isIntegerTy(32));
    }
  } // namespace
} // namespace ink::backend::llvm
