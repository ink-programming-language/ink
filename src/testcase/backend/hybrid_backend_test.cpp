#include "ink/backend/llvm/llvm_backend.h"
#include "ink/execution/hybrid/archive.h"
#include "ink/ir/function/function.h"
#include "ink/ir/linkage.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <gtest/gtest.h>

namespace ink::backend::llvm::test
{
  // Optimizing a hot function must preserve its single entry and runtime guard.
  TEST(HybridBackendTest, KeepsOneFunctionAndGuardAtO0AndO2)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, "func f(Value: i32): i32 { return Value + 1; }"));
    semantic::SemanticContext Semantic(Compilation);
    const auto *Module = semantic::Analyzer{}.analyze(Semantic, Parsed, "hybrid");
    ASSERT_NE(Module, nullptr);
    const auto &Function = static_cast<const ir::Function &>(*Module->entryBlock().values().front());
    const auto Name = ir::functionSymbol(Function);
    ASSERT_TRUE(Name);
    for (const unsigned Level : {0U, 2U})
    {
      ::llvm::LLVMContext Context;
      BackendOptions Options;
      Options.HotReload = true;
      Options.OptimizationLevel = Level;
      const auto Result = lowerToLLVMIR(Context, *Module, nullptr, Options);
      ASSERT_TRUE(Result.succeeded()) << Result.error();
      auto *Native = Result.module()->getFunction(Name.Name);
      ASSERT_NE(Native, nullptr);
      EXPECT_TRUE(Native->hasFnAttribute(::llvm::Attribute::NoInline));
      std::size_t Enters = 0;
      std::size_t Invokes = 0;
      for (const auto &Block : *Native)
      {
        for (const auto &Instruction : Block)
        {
          if (const auto *Call = ::llvm::dyn_cast<::llvm::CallBase>(&Instruction); Call && Call->getCalledFunction())
          {
            Enters += Call->getCalledFunction()->getName() == "ink_hybrid_enter";
            Invokes += Call->getCalledFunction()->getName() == "ink_hybrid_invoke";
          }
        }
      }
      EXPECT_EQ(Enters, 1U);
      EXPECT_EQ(Invokes, 1U);
      auto Manifest = execution::deserializeHybridArtifact(Result.hybridManifest());
      ASSERT_TRUE(Manifest) << Manifest.Error;
      EXPECT_TRUE(Manifest.Artifact.Bytecode->Image.Functions.empty());
      EXPECT_EQ(Manifest.Artifact.Bytecode->Symbols.size(), 1U);
    }
    ::llvm::LLVMContext Context;
    const auto Ordinary = lowerToLLVMIR(Context, *Module, nullptr);
    ASSERT_TRUE(Ordinary.succeeded()) << Ordinary.error();
    EXPECT_EQ(Ordinary.module()->getFunction("ink_hybrid_enter"), nullptr);
    EXPECT_TRUE(Ordinary.hybridManifest().empty());
  }
} // namespace ink::backend::llvm::test
