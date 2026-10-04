#include "ink/execution/bytecode/execution_compiler.h"
#include "ink/execution/bridge/semantic_value_bridge.h"

#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

namespace ink::execution::test
{
  namespace
  {
    std::size_t initializedCount(const ExecutableFunction &Function)
    {
      std::size_t Count = 0;
      for (const RuntimeValue &Value : Function.InitialSlots)
      {
        Count += Value.Initialized;
      }
      return Count;
    }

    struct CompilerContext
    {
        std::unique_ptr<ir::Function> function(const ir::Type &ReturnType, std::span<const ir::Type *const> Parameters = {})
        {
          return Builder.createFunction(Context.namePool().intern("Compiled"), *Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters));
        }

        bool begin(ir::Function &Function)
        {
          auto *Body = Builder.createFunctionBody(Function);
          return Body && Builder.setInsertPoint(*Body);
        }

        const ir::IntegerConstant &integer(std::uint64_t Value)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Value));
        }

        core::CompilationContext Compilation;
        ir::IRContext Context{Compilation};
        SemanticValueBridge Bridge{Context};
        ir::IRBuilder Builder{Context};
        ExecutionCompiler Compiler;
        const ir::IntegerType &Int32 = *Context.typePool().getType<ir::TypeKind::Integer>(32, true);
        const ir::Type &Bool = Context.typePool().getType<ir::TypeKind::Bool>();
        const ir::Type &Void = Context.typePool().getType<ir::TypeKind::Void>();
    };
  } // namespace

  // Repeated references share one constant initializer while parameter and SSA operands become direct slots.
  TEST(ExecutionCompilerTest, AssignsParameterSlotsAndDeduplicatesConstants)
  {
    CompilerContext Test;
    const ir::Type *Parameters[] = {&Test.Int32};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Function));
    const auto &Constant = Test.integer(7);
    const auto *First = Test.Builder.createAddInstruction(*Function->parameters()[0], Constant);
    ASSERT_NE(First, nullptr);
    const auto *Second = Test.Builder.createAddInstruction(*First, Constant);
    ASSERT_NE(Second, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Second), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    const auto &Image = *Compiled.Function;
    ASSERT_EQ(Image.Code.size(), 3U);
    ASSERT_EQ(Image.InitialSlots.size(), Image.SlotTypes.size());
    ASSERT_EQ(initializedCount(Image), 1U);
    const SlotId ConstantSlot = Image.Code[0].Operands[2];
    EXPECT_EQ(Image.InitialSlots[ConstantSlot].Bits, 7U);
    EXPECT_EQ(Image.Code[0].Operands[1], 0U);
    EXPECT_EQ(Image.Code[0].Operands[2], ConstantSlot);
    EXPECT_EQ(Image.Code[1].Operands[1], Image.Code[0].Operands[0]);
    EXPECT_EQ(Image.Code[1].Operands[2], ConstantSlot);
    EXPECT_EQ(Image.Code[2].Operands[1], Image.Code[1].Operands[0]);
  }

  // Native widths select specialized signed/unsigned instructions and unusual widths retain exact-width helpers.
  TEST(ExecutionCompilerTest, SelectsIntegerWidthsAndComparisonSignedness)
  {
    CompilerContext Test;
    struct Row
    {
        std::uint32_t Width;
        bool Signed;
        BytecodeOpcode Add;
        BytecodeOpcode Compare;
    };
    constexpr Row Rows[] = {
        {1, false, BytecodeOpcode::AddWide, BytecodeOpcode::CompareWide},
        {8, true, BytecodeOpcode::AddI8, BytecodeOpcode::CompareSigned8},
        {16, false, BytecodeOpcode::AddI16, BytecodeOpcode::CompareUnsigned16},
        {32, true, BytecodeOpcode::AddI32, BytecodeOpcode::CompareSigned32},
        {64, false, BytecodeOpcode::AddI64, BytecodeOpcode::CompareUnsigned64},
        {65, true, BytecodeOpcode::AddWide, BytecodeOpcode::CompareWide},
        {128, false, BytecodeOpcode::AddWide, BytecodeOpcode::CompareWide},
    };
    for (const Row &Entry : Rows)
    {
      SCOPED_TRACE(Entry.Width);
      const auto *Type = Test.Context.typePool().getType<ir::TypeKind::Integer>(Entry.Width, Entry.Signed);
      const ir::Type *Parameters[] = {Type, Type};
      auto Function = Test.function(Test.Bool, Parameters);
      ASSERT_TRUE(Test.begin(*Function));
      const auto *Sum = Test.Builder.createAddInstruction(*Function->parameters()[0], *Function->parameters()[1]);
      ASSERT_NE(Sum, nullptr);
      const auto *Compare = Test.Builder.createCompareInstruction(core::ComparisonPredicate::Less, *Sum, *Function->parameters()[0]);
      ASSERT_NE(Compare, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Compare), nullptr);
      auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
      ASSERT_TRUE(Compiled);
      EXPECT_EQ(Compiled.Function->Code[0].Code, Entry.Add);
      EXPECT_EQ(Compiled.Function->Code[1].Code, Entry.Compare);
      EXPECT_EQ(Compiled.Function->Code[1].Operands[3], static_cast<std::uint32_t>(core::ComparisonPredicate::Less));
    }
  }

  // Forward and backward edges resolve to instruction offsets, including an empty destination that must fail instead of fall through.
  TEST(ExecutionCompilerTest, FixesBranchOffsetsAndPreservesMissingTerminators)
  {
    CompilerContext Test;
    const ir::Type *Parameters[] = {&Test.Bool};
    auto Function = Test.function(Test.Void, Parameters);
    auto *Entry = Test.Builder.createBasicBlock(*Function);
    auto *Loop = Test.Builder.createBasicBlock(*Function);
    auto *Empty = Test.Builder.createBasicBlock(*Function);
    auto *Exit = Test.Builder.createBasicBlock(*Function);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Entry));
    ASSERT_NE(Test.Builder.createBranchInstruction(*Loop), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Loop));
    ASSERT_NE(Test.Builder.createConditionalBranchInstruction(*Function->parameters()[0], *Entry, *Empty), nullptr);
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Exit));
    ASSERT_NE(Test.Builder.createReturnInstruction(), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    const auto &Code = Compiled.Function->Code;
    ASSERT_EQ(Code.size(), 4U);
    EXPECT_EQ(Code[0].Code, BytecodeOpcode::Jump);
    EXPECT_EQ(Code[0].Operands[1], 1U);
    EXPECT_EQ(Code[1].Code, BytecodeOpcode::JumpIf);
    EXPECT_EQ(Code[1].Operands[2], 0U);
    EXPECT_EQ(Code[1].Operands[3], 2U);
    EXPECT_EQ(Code[2].Code, BytecodeOpcode::Failure);
    EXPECT_EQ(Code[2].Operands[1], static_cast<std::uint32_t>(ExecutionStatus::MissingBody));
    EXPECT_EQ(Code[3].Code, BytecodeOpcode::ReturnVoid);
  }

  // Call sites preserve normalized argument order and distinguish an already resolved function from a function-valued parameter.
  TEST(ExecutionCompilerTest, ResolvesDirectAndIndirectCallSites)
  {
    CompilerContext Test;
    const ir::Type *CalleeParameters[] = {&Test.Int32};
    auto Callee = Test.function(Test.Int32, CalleeParameters);
    const ir::Type *Parameters[] = {&Callee->type(), &Test.Int32};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *DirectArguments[] = {Function->parameters()[1].get()};
    auto *Direct = Test.Builder.createCallInstruction(*Callee, DirectArguments);
    ASSERT_NE(Direct, nullptr);
    const ir::Value *IndirectArguments[] = {Direct};
    auto *Indirect = Test.Builder.createCallInstruction(*Function->parameters()[0], IndirectArguments);
    ASSERT_NE(Indirect, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Indirect), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    const auto &Image = *Compiled.Function;
    ASSERT_EQ(Image.Calls.size(), 2U);
    EXPECT_EQ(Image.Code[0].Code, BytecodeOpcode::CallDirect);
    EXPECT_EQ(Image.Calls[0].Target, Test.Bridge.lowerFunction(*Callee));
    EXPECT_EQ(Image.Calls[0].CalleeSlot, InvalidSlot);
    ASSERT_EQ(Image.Calls[0].Arguments.size(), 1U);
    EXPECT_EQ(Image.Calls[0].Arguments[0], 1U);
    EXPECT_EQ(Image.Code[1].Code, BytecodeOpcode::CallIndirect);
    EXPECT_EQ(Image.Calls[1].Target, InvalidFunction);
    EXPECT_EQ(Image.Calls[1].CalleeSlot, 0U);
    ASSERT_EQ(Image.Calls[1].Arguments.size(), 1U);
    EXPECT_EQ(Image.Calls[1].Arguments[0], Image.Code[0].Operands[0]);
  }

  // An alloca used exclusively by direct loads and stores can carry an internal cell handle without exposing a pointer value.
  TEST(ExecutionCompilerTest, UsesLocalMemoryInstructionsOnlyForUnobservedAddresses)
  {
    CompilerContext Test;
    auto Function = Test.function(Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Local = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Local, nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Local, Test.integer(19)), nullptr);
    auto *Loaded = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(Loaded, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Loaded), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    EXPECT_EQ(Compiled.Function->Code[0].Code, BytecodeOpcode::AllocaLocal);
    EXPECT_EQ(Compiled.Function->Code[1].Code, BytecodeOpcode::StoreLocal);
    EXPECT_EQ(Compiled.Function->Code[2].Code, BytecodeOpcode::LoadLocal);
  }

  // Passing, returning, or storing an address makes its allocation observable and keeps the managed pointer path.
  TEST(ExecutionCompilerTest, PreservesEscapingAllocationIdentities)
  {
    CompilerContext Test;
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.Int32, ir::AccessKind::ReadWrite);
    const ir::Type *Parameters[] = {Pointer};
    auto Callee = Test.function(Test.Void, Parameters);
    auto Function = Test.function(*Pointer);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Passed = Test.Builder.createAllocaInstruction(Test.Int32);
    auto *Stored = Test.Builder.createAllocaInstruction(Test.Int32);
    auto *Returned = Test.Builder.createAllocaInstruction(Test.Int32);
    auto *PointerCell = Test.Builder.createAllocaInstruction(*Pointer);
    ASSERT_NE(Passed, nullptr);
    ASSERT_NE(Stored, nullptr);
    ASSERT_NE(Returned, nullptr);
    ASSERT_NE(PointerCell, nullptr);
    const ir::Value *Arguments[] = {Passed};
    ASSERT_NE(Test.Builder.createCallInstruction(*Callee, Arguments), nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*PointerCell, *Stored), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Returned), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    EXPECT_EQ(Compiled.Function->Code[0].Code, BytecodeOpcode::Alloca);
    EXPECT_EQ(Compiled.Function->Code[1].Code, BytecodeOpcode::Alloca);
    EXPECT_EQ(Compiled.Function->Code[2].Code, BytecodeOpcode::Alloca);
    EXPECT_EQ(Compiled.Function->Code[3].Code, BytecodeOpcode::AllocaLocal);
    EXPECT_EQ(Compiled.Function->Code[5].Code, BytecodeOpcode::StoreLocal);
  }

  // A nested function's direct address use is not a private local use in the enclosing function activation.
  TEST(ExecutionCompilerTest, DetectsAddressReferencesInsideNestedFunctions)
  {
    CompilerContext Test;
    auto Function = Test.function(Test.Void);
    ASSERT_TRUE(Test.begin(*Function));
    auto *OuterBlock = Test.Builder.insertBlock();
    auto *Local = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Local, nullptr);
    auto Nested = Test.function(Test.Int32);
    ASSERT_TRUE(Test.begin(*Nested));
    auto *Loaded = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(Loaded, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Loaded), nullptr);
    ASSERT_TRUE(Test.Builder.appendValue(*OuterBlock, std::move(Nested)));
    ASSERT_TRUE(Test.Builder.setInsertPoint(*OuterBlock));
    ASSERT_NE(Test.Builder.createReturnInstruction(), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    EXPECT_EQ(Compiled.Function->Code[0].Code, BytecodeOpcode::Alloca);
    EXPECT_EQ(Compiled.Function->Code[1].Code, BytecodeOpcode::Function);
  }

  // Deep lexical function trees are scanned with a worklist while still detecting an address use at the deepest level.
  TEST(ExecutionCompilerTest, ScansDeepFunctionTreesWithoutRecursiveTraversal)
  {
    CompilerContext Test;
    auto Function = Test.function(Test.Void);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Local = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Local, nullptr);
    struct NestedOwners
    {
        ~NestedOwners()
        {
          // Tear down the test tree iteratively as IR ownership destruction is recursive.
          for (std::size_t Index = Functions.size(); Index > 1; --Index)
          {
            auto Detached = Builder.removeValue(*Functions[Index - 2]->entryBlock(), *Functions[Index - 1]);
          }
        }

        ir::IRBuilder &Builder;
        std::vector<ir::Function *> Functions;
    };
    NestedOwners Owners{Test.Builder, {Function.get()}};
    constexpr std::size_t Depth = 4096;
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      auto Nested = Test.function(Test.Void);
      ASSERT_NE(Nested, nullptr);
      auto *Next = Nested.get();
      ASSERT_TRUE(Test.begin(*Next));
      ASSERT_TRUE(Test.Builder.appendValue(*Owners.Functions.back()->entryBlock(), std::move(Nested)));
      Owners.Functions.push_back(Next);
    }
    ASSERT_NE(Test.Builder.createLoadInstruction(*Local), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    EXPECT_EQ(Compiled.Function->Code[0].Code, BytecodeOpcode::Alloca);
  }

  // Unexecuted SSA definitions retain an uninitialized slot rather than being evaluated during compilation.
  TEST(ExecutionCompilerTest, LeavesUnavailableOperandsUninitialized)
  {
    CompilerContext Test;
    auto Function = Test.function(Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    auto Detached = Test.Builder.createDetachedAddInstruction(Test.integer(1), Test.integer(2));
    ASSERT_NE(Detached, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Detached.get()), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    ASSERT_EQ(Compiled.Function->Code.size(), 1U);
    EXPECT_EQ(Compiled.Function->Code[0].Code, BytecodeOpcode::Return);
    EXPECT_EQ(initializedCount(*Compiled.Function), 0U);
    EXPECT_LT(Compiled.Function->Code[0].Operands[1], Compiled.Function->SlotTypes.size());
  }

  // CString creation remains executable storage allocation while the image owns its immutable literal bytes.
  TEST(ExecutionCompilerTest, RetainsCStringAllocationAsAnInstruction)
  {
    CompilerContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *Slice = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const auto *String = Test.Context.constantPool().getStringConstant(*Slice, "payload");
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Byte, ir::AccessKind::ReadWrite);
    auto Function = Test.function(*Pointer);
    ASSERT_TRUE(Test.begin(*Function));
    auto *CString = Test.Builder.createCStringInstruction(*String);
    ASSERT_NE(CString, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(CString), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    EXPECT_EQ(std::string_view(Compiled.Function->ConstantData.data(), Compiled.Function->ConstantData.size()), "payload");
    EXPECT_EQ(Compiled.Function->Code[0].Operands[1], 0U);
    EXPECT_EQ(Compiled.Function->Code[0].Operands[2], 7U);
    EXPECT_EQ(Compiled.Function->Code[0].Code, BytecodeOpcode::CString);
    EXPECT_EQ(initializedCount(*Compiled.Function), 0U);
  }

  // Verification rejects corrupted operand indices, wrong specialized widths, invalid opcodes, and targets outside the code image.
  TEST(ExecutionCompilerTest, RejectsMalformedOperandsAndSpecializations)
  {
    CompilerContext Test;
    const ir::Type *Parameters[] = {&Test.Int32, &Test.Int32};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Sum = Test.Builder.createAddInstruction(*Function->parameters()[0], *Function->parameters()[1]);
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    auto &Image = *Compiled.Function;
    const auto Original = Image.Code[0];
    Image.Code[0].Operands[1] = InvalidSlot;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::InvalidArguments);
    Image.Code[0] = Original;
    Image.Code[0].Code = BytecodeOpcode::AddI64;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::TypeMismatch);
    Image.Code[0].Code = BytecodeOpcode::Count;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::UnsupportedOperation);
    Image.Code[0] = {BytecodeOpcode::Jump, {0, static_cast<std::uint32_t>(Image.Code.size())}};
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::InvalidArguments);
    Image.Code[0] = Original;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::Success);
  }

  // Corrupt bytecode cannot reinterpret a managed pointer as an internal local-cell handle or expose a local handle as a pointer.
  TEST(ExecutionCompilerTest, RejectsInvalidLocalMemoryProvenance)
  {
    CompilerContext Test;
    auto Function = Test.function(Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Local = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Local, nullptr);
    auto *Loaded = Test.Builder.createLoadInstruction(*Local);
    ASSERT_NE(Loaded, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Loaded), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    auto &Image = *Compiled.Function;
    Image.Code[0].Code = BytecodeOpcode::Alloca;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::InvalidArguments);
    Image.Code[0].Code = BytecodeOpcode::AllocaLocal;
    Image.Code[1].Code = BytecodeOpcode::Load;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::InvalidArguments);
    Image.Code[1].Code = BytecodeOpcode::LoadLocal;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::Success);
  }

  // Encoded initializer types and call argument slots must match the verified runtime signature exactly.
  TEST(ExecutionCompilerTest, RejectsMismatchedInitializersAndInvalidCallArguments)
  {
    CompilerContext Test;
    const ir::Type *Parameters[] = {&Test.Int32};
    auto Callee = Test.function(Test.Int32, Parameters);
    auto Function = Test.function(Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *Arguments[] = {&Test.integer(3)};
    auto *Call = Test.Builder.createCallInstruction(*Callee, Arguments);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    auto &Image = *Compiled.Function;
    ASSERT_EQ(initializedCount(Image), 1U);
    const SlotId ConstantSlot = Image.Calls[0].Arguments[0];
    const RuntimeValue Original = Image.InitialSlots[ConstantSlot];
    Image.InitialSlots[ConstantSlot].Type = Test.Bridge.lowerType(Test.Bool);
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::TypeMismatch);
    Image.InitialSlots[ConstantSlot] = Original;
    Image.Calls[0].Arguments[0] = InvalidSlot;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::TypeMismatch);
  }

  // Lowered layouts, scalar initializers and string bytes stay valid after every source IR owner is destroyed.
  TEST(ExecutionCompilerTest, OwnsRuntimeDataBeyondIRLifetime)
  {
    std::unique_ptr<ExecutableFunction> Image;
    {
      CompilerContext Test;
      const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
      const auto *Slice = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
      const auto *String = Test.Context.constantPool().getStringConstant(*Slice, "owned bytes");
      auto Function = Test.function(Test.Int32);
      ASSERT_TRUE(Test.begin(*Function));
      ASSERT_NE(Test.Builder.createCStringInstruction(*String), nullptr);
      auto *Sum = Test.Builder.createAddInstruction(Test.integer(17), Test.integer(25));
      ASSERT_NE(Sum, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
      auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
      ASSERT_TRUE(Compiled);
      Image = std::move(Compiled.Function);
    }
    EXPECT_EQ(ExecutionCompiler{}.verify(*Image), ExecutionStatus::Success);
    EXPECT_EQ(std::string_view(Image->ConstantData.data(), Image->ConstantData.size()), "owned bytes");
    ASSERT_EQ(Image->Code[1].Code, BytecodeOpcode::AddI32);
    EXPECT_EQ(Image->InitialSlots[Image->Code[1].Operands[1]].Bits, 17U);
    EXPECT_EQ(Image->InitialSlots[Image->Code[1].Operands[2]].Bits, 25U);
    const auto *Layout = Image->Layouts->get(Image->SlotTypes[Image->Code[1].Operands[0]]);
    ASSERT_NE(Layout, nullptr);
    EXPECT_EQ(Layout->Kind, RuntimeKind::Integer);
    EXPECT_EQ(Layout->bitWidth(), 32U);
  }

  // Escaping integer addresses select width-specific loads and stores; unusual widths retain the generic payload path.
  TEST(ExecutionCompilerTest, SpecializesManagedIntegerMemoryAccesses)
  {
    CompilerContext Test;
    struct Row
    {
        std::uint32_t Width;
        BytecodeOpcode Load;
        BytecodeOpcode Store;
    };
    constexpr Row Rows[] = {
        {8, BytecodeOpcode::LoadI8, BytecodeOpcode::StoreI8},
        {16, BytecodeOpcode::LoadI16, BytecodeOpcode::StoreI16},
        {32, BytecodeOpcode::LoadI32, BytecodeOpcode::StoreI32},
        {64, BytecodeOpcode::LoadI64, BytecodeOpcode::StoreI64},
        {65, BytecodeOpcode::Load, BytecodeOpcode::Store},
    };
    for (const Row &Entry : Rows)
    {
      SCOPED_TRACE(Entry.Width);
      const auto *Integer = Test.Context.typePool().getType<ir::TypeKind::Integer>(Entry.Width, false);
      const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Integer, ir::AccessKind::ReadWrite);
      const ir::Type *Parameters[] = {Pointer, Integer};
      auto Function = Test.function(*Integer, Parameters);
      ASSERT_TRUE(Test.begin(*Function));
      ASSERT_NE(Test.Builder.createStoreInstruction(*Function->parameters()[0], *Function->parameters()[1]), nullptr);
      auto *Loaded = Test.Builder.createLoadInstruction(*Function->parameters()[0]);
      ASSERT_NE(Loaded, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Loaded), nullptr);
      auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
      ASSERT_TRUE(Compiled);
      EXPECT_EQ(Compiled.Function->Code[0].Code, Entry.Store);
      EXPECT_EQ(Compiled.Function->Code[1].Code, Entry.Load);
      Compiled.Function->Code[1].Code = Entry.Width == 64 ? BytecodeOpcode::LoadI8 : BytecodeOpcode::LoadI64;
      EXPECT_EQ(Test.Compiler.verify(*Compiled.Function), ExecutionStatus::TypeMismatch);
    }
  }

  // Independent private allocations receive compact frame indexes, and corrupted indexes cannot alias their storage.
  TEST(ExecutionCompilerTest, AssignsDistinctLocalFrameCells)
  {
    CompilerContext Test;
    auto Function = Test.function(Test.Void);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createAllocaInstruction(Test.Int32), nullptr);
    ASSERT_NE(Test.Builder.createAllocaInstruction(Test.Bool), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    auto &Image = *Compiled.Function;
    ASSERT_EQ(Image.LocalStorageCount, 2U);
    EXPECT_EQ(Image.Code[0].Operands[2], 0U);
    EXPECT_EQ(Image.Code[1].Operands[2], 1U);
    Image.Code[1].Operands[2] = 0;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::InvalidArguments);
    Image.Code[1].Operands[2] = 2;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::InvalidArguments);
    Image.Code[1].Operands[2] = 1;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::Success);
  }

  // Equal storage sizes do not make boolean and byte allocation layouts interchangeable.
  TEST(ExecutionCompilerTest, PreservesTypeIdentitySeparatelyFromStorageSize)
  {
    CompilerContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    auto Function = Test.function(Test.Void);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createAllocaInstruction(*Byte), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    auto &Image = *Compiled.Function;
    const RuntimeTypeId BoolType = Test.Bridge.lowerType(Test.Bool);
    ASSERT_EQ(Image.Layouts->get(Image.Code[0].Operands[1])->Size, Image.Layouts->get(BoolType)->Size);
    Image.Code[0].Operands[1] = BoolType;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::TypeMismatch);
  }

  // CString byte ranges are verified as a pair and embedded terminators cannot silently truncate the requested literal.
  TEST(ExecutionCompilerTest, RejectsCorruptedConstantByteRanges)
  {
    CompilerContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *Slice = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const auto *String = Test.Context.constantPool().getStringConstant(*Slice, "literal");
    auto Function = Test.function(Test.Void);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createCStringInstruction(*String), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    auto &Image = *Compiled.Function;
    Image.Code[0].Operands[1] = 1;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::InvalidArguments);
    Image.Code[0].Operands[1] = 0;
    Image.ConstantData[3] = '\0';
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::InvalidArguments);
  }

  // Function-valued operands retain signature identity even when two callable representations have the same size.
  TEST(ExecutionCompilerTest, RejectsMismatchedIndirectCallSignatures)
  {
    CompilerContext Test;
    const ir::Type *IntegerParameters[] = {&Test.Int32};
    auto Callee = Test.function(Test.Int32, IntegerParameters);
    const ir::Type *BooleanParameters[] = {&Test.Bool};
    auto Other = Test.function(Test.Int32, BooleanParameters);
    const ir::Type *Parameters[] = {&Callee->type(), &Test.Int32};
    auto Function = Test.function(Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *Arguments[] = {Function->parameters()[1].get()};
    auto *Call = Test.Builder.createCallInstruction(*Function->parameters()[0], Arguments);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
    auto Compiled = Test.Compiler.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled);
    auto &Image = *Compiled.Function;
    const RuntimeTypeId OtherSignature = Test.Bridge.lowerType(Other->type());
    ASSERT_EQ(Image.Layouts->get(Image.Calls[0].Signature)->Size, Image.Layouts->get(OtherSignature)->Size);
    Image.Calls[0].Signature = OtherSignature;
    EXPECT_EQ(Test.Compiler.verify(Image), ExecutionStatus::TypeMismatch);
  }
} // namespace ink::execution::test
