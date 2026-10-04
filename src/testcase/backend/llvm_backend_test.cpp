#include "ink/backend/llvm/llvm_backend.h"
#include "ink/ir/analysis/type_layout.h"
#include "ink/ir/ir_builder.h"
#include "ink/ir/linkage.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <gtest/gtest.h>

namespace ink::backend::llvm::test
{
  namespace
  {
    struct Program
    {
        core::CompilationContext Compilation;
        ir::IRContext Context{Compilation};
        ir::IRBuilder Builder{Context};
        ir::Module *Module = Builder.createModule(Context.namePool().intern("aot_test"));
        const ir::IntegerType *Int32 = Context.typePool().getType<ir::TypeKind::Integer>(32, true);
        ir::Function *Main = nullptr;

        Program()
        {
          auto Function = Builder.createFunction(Context.namePool().intern("main"), *Context.typePool().getType<ir::TypeKind::Function>(*Int32));
          Main = Function.get();
          ir::BasicBlock *Body = Builder.createFunctionBody(*Main);
          Builder.appendValue(Module->entryBlock(), std::move(Function));
          Builder.setInsertPoint(*Body);
        }
    };

    std::string text(const ::llvm::Module &Module)
    {
      std::string Result;
      ::llvm::raw_string_ostream Stream(Result);
      Module.print(Stream, nullptr);
      return Result;
    }
  } // namespace

  // The backend supplies a real native triple and target DataLayout, and verifies optimized generated code.
  TEST(LLVMBackendTest, LowersNativeEntryAtBothOptimizationLevels)
  {
    Program Source;
    const auto *Answer = Source.Context.constantPool().getIntegerConstant(*Source.Int32, ir::IntegerBits(32, 42));
    ASSERT_NE(Source.Builder.createReturnInstruction(Answer), nullptr);
    for (const unsigned Level : {0U, 2U})
    {
      ::llvm::LLVMContext Context;
      BackendOptions Options;
      Options.OptimizationLevel = Level;
      LoweringResult Result = lowerToLLVMIR(Context, *Source.Module, Source.Main, Options);
      ASSERT_TRUE(Result.succeeded()) << Result.error();
      EXPECT_FALSE(Result.module()->getTargetTriple().str().empty());
      EXPECT_EQ(Result.module()->getDataLayout().getPointerSize(), sizeof(void *));
      EXPECT_NE(Result.module()->getFunction("main"), nullptr);
      EXPECT_FALSE(::llvm::verifyModule(*Result.module(), nullptr));
    }
  }

  // Field addresses use shared target offsets and raw LLVM memory instructions without managed-pointer helpers.
  TEST(LLVMBackendTest, ClassFieldPointersUseExplicitLayoutAndRawAddresses)
  {
    Program Source;
    const auto *Byte = Source.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *Wide = Source.Context.typePool().getType<ir::TypeKind::Integer>(128, false);
    const auto *Class = Source.Context.typePool().createClassType(Source.Context.namePool().intern("Mixed"));
    const ir::ClassField Fields[] = {
        {Source.Context.namePool().intern("Tag"), Byte},
        {Source.Context.namePool().intern("Wide"), Wide},
        {Source.Context.namePool().intern("Last"), Source.Int32},
    };
    const auto Identity = abi::mangle(abi::record('T', {abi::record('c', {ir::declarationRecord(Source.Module->linkageIdentity(), {}, 'c', "Mixed"), {'X', {}}})}));
    ASSERT_TRUE(Identity);
    ASSERT_TRUE(Source.Builder.defineClassType(*Class, Fields, Identity.Name));
    const auto *ZeroByte = Source.Context.constantPool().getIntegerConstant(*Byte, ir::IntegerBits(8, 0));
    const auto *ZeroWide = Source.Context.constantPool().getIntegerConstant(*Wide, ir::IntegerBits(128, 0));
    const auto *Answer = Source.Context.constantPool().getIntegerConstant(*Source.Int32, ir::IntegerBits(32, 42));
    const ir::Value *Values[] = {ZeroByte, ZeroWide, Answer};
    auto *Object = Source.Builder.createClassInstruction(*Class, Values);
    ASSERT_NE(Object, nullptr);
    auto *Address = Source.Builder.createAllocaInstruction(*Class);
    ASSERT_NE(Source.Builder.createStoreInstruction(*Address, *Object), nullptr);
    auto *Field = Source.Builder.createFieldPointerInstruction(*Address, 2);
    ASSERT_NE(Field, nullptr);
    auto *Loaded = Source.Builder.createLoadInstruction(*Field);
    ASSERT_NE(Source.Builder.createReturnInstruction(Loaded), nullptr);
    ::llvm::LLVMContext Context;
    LoweringResult Result = lowerToLLVMIR(Context, *Source.Module, Source.Main);
    ASSERT_TRUE(Result.succeeded()) << Result.error();
    const std::string IR = text(*Result.module());
    EXPECT_EQ(IR.find("@ink_aot_derive"), std::string::npos);
    EXPECT_EQ(IR.find("@ink_aot_address"), std::string::npos);
    EXPECT_EQ(IR.find("@ink_aot_leave"), std::string::npos);
    const auto Layout = ir::computeTypeLayout(*Class, Source.Compilation.targetContext());
    ASSERT_TRUE(Layout.has_value());
    EXPECT_EQ(Layout->FieldOffsets[2], Source.Compilation.targetContext().integerAlignment(128) + 16U);
    EXPECT_NE(IR.find("getelementptr i8"), std::string::npos);
    EXPECT_NE(IR.find("alloca i8"), std::string::npos);
    EXPECT_FALSE(Layout->VPtrOffset);
    EXPECT_FALSE(Layout->BaseOffset);
    EXPECT_FALSE(::llvm::verifyModule(*Result.module(), nullptr));
  }

  // A target without a matching runtime must fail explicitly before any LLVM code is produced.
  TEST(LLVMBackendTest, RejectsCrossTargetAndInvalidOptimizationLevel)
  {
    Program Source;
    const auto *Zero = Source.Context.constantPool().getIntegerConstant(*Source.Int32, ir::IntegerBits(32, 0));
    ASSERT_NE(Source.Builder.createReturnInstruction(Zero), nullptr);
    ::llvm::LLVMContext Context;
    BackendOptions Options;
    Options.TargetTriple = "wasm32-unknown-unknown";
    auto Cross = lowerToLLVMIR(Context, *Source.Module, Source.Main, Options);
    EXPECT_FALSE(Cross.succeeded());
    EXPECT_NE(Cross.error().find("cross compilation"), std::string::npos);
    Options.TargetTriple.clear();
    Options.OptimizationLevel = 4;
    auto Invalid = lowerToLLVMIR(Context, *Source.Module, Source.Main, Options);
    EXPECT_FALSE(Invalid.succeeded());
    EXPECT_NE(Invalid.error().find("optimization level"), std::string::npos);
  }

  // Incomplete classes cannot silently become zero-sized or opaque executable objects.
  TEST(LLVMBackendTest, RejectsIncompleteClassStorage)
  {
    Program Source;
    const auto *Class = Source.Context.typePool().createClassType(Source.Context.namePool().intern("Incomplete"));
    const auto *Zero = Source.Context.constantPool().getIntegerConstant(*Source.Int32, ir::IntegerBits(32, 0));
    ASSERT_NE(Source.Builder.createReturnInstruction(Zero), nullptr);
    const ir::Type *Parameters[] = {Class};
    auto Function = Source.Builder.createFunction(Source.Context.namePool().intern("incomplete"), *Source.Context.typePool().getType<ir::TypeKind::Function>(*Source.Int32, Parameters));
    ASSERT_TRUE(Function);
    auto *Body = Source.Builder.createFunctionBody(*Function);
    ASSERT_TRUE(Source.Builder.appendValue(Source.Module->entryBlock(), std::move(Function)));
    ASSERT_TRUE(Source.Builder.setInsertPoint(*Body));
    ASSERT_NE(Source.Builder.createReturnInstruction(Zero), nullptr);
    ::llvm::LLVMContext Context;
    const auto Result = lowerToLLVMIR(Context, *Source.Module, Source.Main);
    EXPECT_FALSE(Result.succeeded());
    EXPECT_NE(Result.error().find("layout"), std::string::npos);
  }
} // namespace ink::backend::llvm::test
