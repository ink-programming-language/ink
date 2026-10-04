#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::execution::test
{
  namespace
  {
    class BytecodeExecutionContext final
    {
      public:
        explicit BytecodeExecutionContext(ExecutionLimits Limits = {})
            : Context(Compilation),
              Builder(Context),
              Engine(Context, Limits),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true))
        {
        }

        std::unique_ptr<ir::Function> function(std::string_view Name, const ir::Type &ReturnType, std::span<const ir::Type *const> Parameters = {})
        {
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(ReturnType, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern(Name), *Signature) : nullptr;
        }

        bool begin(ir::Function &Function)
        {
          auto *Entry = Builder.createFunctionBody(Function);
          return Entry && Builder.setInsertPoint(*Entry);
        }

        const ir::IntegerConstant &constant(std::uint64_t Bits)
        {
          return *Context.constantPool().getIntegerConstant(Int32, ir::IntegerBits(32, Bits));
        }

        ExecutionValueRef integer(std::uint64_t Bits)
        {
          return Engine.heap().integer(Int32, ExecutionInteger(32, Bits));
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionEngine Engine;
        const ir::IntegerType &Int32;
    };

    void expectInteger(const ExecutionValueResult &Result, std::uint64_t Expected)
    {
      ASSERT_TRUE(Result);
      ASSERT_EQ(Result.Value.kind(), RuntimeKind::Integer);
      const auto Bits = Result.Value.integer().bits();
      ASSERT_EQ(Bits.words().size(), 1U);
      EXPECT_EQ(Bits.words().front(), Expected);
    }

    bool comparisonResult(core::ComparisonPredicate Predicate, int Ordering)
    {
      switch (Predicate)
      {
      case core::ComparisonPredicate::Equal:
        return Ordering == 0;
      case core::ComparisonPredicate::NotEqual:
        return Ordering != 0;
      case core::ComparisonPredicate::Less:
        return Ordering < 0;
      case core::ComparisonPredicate::LessEqual:
        return Ordering <= 0;
      case core::ComparisonPredicate::Greater:
        return Ordering > 0;
      case core::ComparisonPredicate::GreaterEqual:
        return Ordering >= 0;
      }
      return false;
    }
  } // namespace

  // Native imports refresh after IR changes while stored resolved function values retain their concrete target identity.
  TEST(BytecodeExecutionTest, ResolvesNativeExportsAndInvalidatesTheirBinding)
  {
    BytecodeExecutionContext Test;
    auto *Module = Test.Builder.createModule(Test.Context.namePool().intern("native_provider"));
    ASSERT_NE(Module, nullptr);
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.Int32, std::span<const ir::Type *const>{});
    ASSERT_NE(Signature, nullptr);
    auto Exported = Test.Builder.createFunction(Test.Context.namePool().intern("inkExecutionPrivateNativeExport"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Export);
    auto Imported = Test.Builder.createFunction(Test.Context.namePool().intern("inkExecutionPrivateNativeExport"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Import);
    ASSERT_NE(Exported, nullptr);
    ASSERT_NE(Imported, nullptr);
    EXPECT_EQ(Test.Engine.execute(*Imported).Status, ExecutionStatus::SymbolNotFound);
    ASSERT_TRUE(Test.Builder.setFunctionVisibility(*Exported, core::VisibilityKind::Private));
    ASSERT_TRUE(Test.begin(*Exported));
    auto *Return = Test.Builder.createReturnInstruction(&Test.constant(42));
    ASSERT_NE(Return, nullptr);
    auto *Definition = Exported.get();
    ASSERT_TRUE(Test.Builder.appendValue(Module->entryBlock(), std::move(Exported)));
    expectInteger(Test.Engine.execute(*Imported), 42);
    const auto Saved = Test.Engine.heap().allocateCell(*Signature, true, Test.Engine.heap().function(*Imported));
    ASSERT_TRUE(Saved);
    ASSERT_TRUE(Test.Builder.eraseValue(*Definition->entryBlock(), *Return));
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Definition->entryBlock()));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.constant(73)), nullptr);
    expectInteger(Test.Engine.execute(*Imported), 73);
    ASSERT_TRUE(Test.Builder.setFunctionBinding(*Definition, core::FunctionBinding::Local));
    EXPECT_EQ(Test.Engine.execute(*Imported).Status, ExecutionStatus::SymbolNotFound);
    const auto Loaded = Test.Engine.heap().load(Saved.Place);
    ASSERT_TRUE(Loaded);
    ASSERT_EQ(Loaded.Value.kind(), RuntimeKind::Function);
    ASSERT_EQ(Loaded.Value.function(), Definition);
    expectInteger(Test.Engine.execute(*Loaded.Value.function()), 73);
    EXPECT_EQ(Test.Engine.heap().release(Saved.Place), ExecutionStatus::Success);
    ASSERT_TRUE(Test.Builder.setFunctionBinding(*Definition, core::FunctionBinding::Export));
    expectInteger(Test.Engine.execute(*Imported), 73);
  }

  // Each evaluated string argument to a C ABI bytecode body has an independent writable copy that is reclaimed after return.
  TEST(BytecodeExecutionTest, CopiesCAbiStringArgumentsAndReclaimsTemporaryBuffers)
  {
    BytecodeExecutionContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Byte, ir::AccessKind::ReadWrite);
    const auto *String = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const ir::Type *Parameters[] = {Pointer, Pointer};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(*Byte, Parameters);
    auto Function = Test.Builder.createFunction(Test.Context.namePool().intern("copyStrings"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    const auto *Zero = Test.Context.constantPool().getIntegerConstant(*Byte, ir::IntegerBits(8, 0));
    ASSERT_NE(Test.Builder.createStoreInstruction(*Function->parameters()[0], *Zero), nullptr);
    const auto *FirstByte = Test.Builder.createLoadInstruction(*Function->parameters()[1]);
    ASSERT_NE(FirstByte, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(FirstByte), nullptr);
    const std::string_view Text("A\0B", 3);
    const auto Value = Test.Engine.heap().string(*String, Text);
    const ExecutionValueRef Arguments[] = {Value, Value};
    for (int Call = 0; Call < 2; ++Call)
    {
      expectInteger(Test.Engine.execute(*Function, Arguments), 'A');
      EXPECT_EQ(Value.string(), Text);
      EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    }
  }

  // A C ABI bytecode definition returning one temporary string alias preserves only that buffer, matching native FFI lifetime rules.
  TEST(BytecodeExecutionTest, PreservesReturnedCAbiStringAlias)
  {
    BytecodeExecutionContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Byte, ir::AccessKind::ReadWrite);
    const auto *String = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const ir::Type *Parameters[] = {Pointer, Pointer};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(*Pointer, Parameters);
    auto Function = Test.Builder.createFunction(Test.Context.namePool().intern("returnString"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Export);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    const auto *ByteValue = Test.Context.constantPool().getIntegerConstant(*Byte, ir::IntegerBits(8, 'Z'));
    ASSERT_NE(Test.Builder.createStoreInstruction(*Function->parameters()[0], *ByteValue), nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Function->parameters()[0].get()), nullptr);
    const auto Value = Test.Engine.heap().string(*String, "AB");
    const ExecutionValueRef Arguments[] = {Value, Value};
    const auto Result = Test.Engine.execute(*Function, Arguments);
    ASSERT_TRUE(Result);
    ASSERT_EQ(Result.Value.kind(), RuntimeKind::Pointer);
    ASSERT_EQ(Result.Value.pointer().kind(), ExecutionPointer::Kind::Native);
    ASSERT_TRUE(Test.Engine.heap().owns(Test.Engine.heap().memoryManager().storageFromAddress(Result.Value.pointer().address())));
    const auto *ReturnedBuffer = Test.Engine.heap().memoryManager().storageFromAddress(Result.Value.pointer().address()).buffer();
    ASSERT_NE(ReturnedBuffer, nullptr);
    EXPECT_EQ(Value.string(), "AB");
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 1U);
    EXPECT_EQ(Test.Engine.heap().release(Test.Engine.heap().memoryManager().storageFromAddress(Result.Value.pointer().address())), ExecutionStatus::Success);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
  }

  // A later invalid C ABI argument releases string copies already prepared earlier in the argument list.
  TEST(BytecodeExecutionTest, ReclaimsCAbiStringBuffersWhenArgumentPreparationFails)
  {
    BytecodeExecutionContext Test;
    const auto *Byte = Test.Context.typePool().getType<ir::TypeKind::Integer>(8, false);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Byte, ir::AccessKind::ReadWrite);
    const auto *String = Test.Context.typePool().getType<ir::TypeKind::Slice>(*Byte, ir::AccessKind::ReadOnly);
    const ir::Type *Parameters[] = {Pointer, &Test.Int32};
    const auto *Signature = Test.Context.typePool().getType<ir::TypeKind::Function>(Test.Int32, Parameters);
    auto Function = Test.Builder.createFunction(Test.Context.namePool().intern("invalidStringArgument"), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.constant(0)), nullptr);
    const auto Value = Test.Engine.heap().string(*String, "AB");
    const ExecutionValueRef Arguments[] = {Value, Value};
    EXPECT_EQ(Test.Engine.execute(*Function, Arguments).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
  }

  // Replacing a previously executed return invalidates the cached bytecode in the same engine.
  TEST(BytecodeExecutionTest, RecompilesAfterReturnReplacement)
  {
    BytecodeExecutionContext Test;
    auto Function = Test.function("ReplaceReturn", Test.Int32);
    ASSERT_NE(Function, nullptr);
    ASSERT_TRUE(Test.begin(*Function));
    auto *Return = Test.Builder.createReturnInstruction(&Test.constant(11));
    ASSERT_NE(Return, nullptr);
    expectInteger(Test.Engine.execute(*Function), 11);
    ASSERT_TRUE(Test.Builder.eraseValue(*Function->entryBlock(), *Return));
    ASSERT_TRUE(Test.Builder.setInsertPoint(*Function->entryBlock()));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.constant(29)), nullptr);
    expectInteger(Test.Engine.execute(*Function), 29);
    expectInteger(Test.Engine.execute(*Function), 29);
  }

  // A missing body does not leave a permanent failed cache entry after the declaration gains its body.
  TEST(BytecodeExecutionTest, ExecutesBodyAddedAfterMissingBodyFailure)
  {
    BytecodeExecutionContext Test;
    auto Function = Test.function("DefineLater", Test.Int32);
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Test.Engine.execute(*Function).Status, ExecutionStatus::MissingBody);
    ASSERT_TRUE(Test.begin(*Function));
    ASSERT_NE(Test.Builder.createReturnInstruction(&Test.constant(73)), nullptr);
    expectInteger(Test.Engine.execute(*Function), 73);
  }

  // Destroying and recreating a callable never executes the old image, including allocator address reuse.
  TEST(BytecodeExecutionTest, DiscardsCodeForDestroyedFunctions)
  {
    BytecodeExecutionContext Test;
    for (std::uint64_t Expected = 1; Expected <= 32; ++Expected)
    {
      auto Function = Test.function("Recreated", Test.Int32);
      ASSERT_NE(Function, nullptr);
      ASSERT_TRUE(Test.begin(*Function));
      ASSERT_NE(Test.Builder.createReturnInstruction(&Test.constant(Expected)), nullptr);
      expectInteger(Test.Engine.execute(*Function), Expected);
      Test.Builder.clearInsertPoint();
      Function.reset();
    }
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
  }

  // One compiled indirect call site must select each invocation's function parameter and argument values.
  TEST(BytecodeExecutionTest, IndirectCallsReadCurrentFunctionParameters)
  {
    BytecodeExecutionContext Test;
    const ir::Type *TargetParameters[] = {&Test.Int32};
    auto Identity = Test.function("Identity", Test.Int32, TargetParameters);
    auto Increment = Test.function("Increment", Test.Int32, TargetParameters);
    ASSERT_NE(Identity, nullptr);
    ASSERT_NE(Increment, nullptr);
    ASSERT_TRUE(Test.begin(*Identity));
    ASSERT_NE(Test.Builder.createReturnInstruction(Identity->parameters()[0].get()), nullptr);
    ASSERT_TRUE(Test.begin(*Increment));
    auto *Sum = Test.Builder.createAddInstruction(*Increment->parameters()[0], Test.constant(1));
    ASSERT_NE(Sum, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    const ir::Type *CallerParameters[] = {&Identity->functionType(), &Test.Int32};
    auto Caller = Test.function("Invoke", Test.Int32, CallerParameters);
    ASSERT_NE(Caller, nullptr);
    ASSERT_TRUE(Test.begin(*Caller));
    const ir::Value *CallArguments[] = {Caller->parameters()[1].get()};
    auto *Call = Test.Builder.createCallInstruction(*Caller->parameters()[0], CallArguments);
    ASSERT_NE(Call, nullptr);
    ASSERT_NE(Test.Builder.createReturnInstruction(Call), nullptr);
    const ExecutionValueRef First[] = {Test.Engine.heap().function(*Identity), Test.integer(41)};
    const ExecutionValueRef Second[] = {Test.Engine.heap().function(*Increment), Test.integer(41)};
    const ExecutionValueRef Third[] = {Test.Engine.heap().function(*Identity), Test.integer(99)};
    expectInteger(Test.Engine.execute(*Caller, First), 41);
    expectInteger(Test.Engine.execute(*Caller, Second), 42);
    expectInteger(Test.Engine.execute(*Caller, Third), 99);
  }

  // Native-width signed comparisons preserve ordering across the sign bit without host signed overflow.
  TEST(BytecodeExecutionTest, SignedComparisonsRespectEveryNativeWidthBoundary)
  {
    BytecodeExecutionContext Test;
    const auto &Bool = Test.Context.typePool().getType<ir::TypeKind::Bool>();
    constexpr std::array Predicates = {
        core::ComparisonPredicate::Equal,
        core::ComparisonPredicate::NotEqual,
        core::ComparisonPredicate::Less,
        core::ComparisonPredicate::LessEqual,
        core::ComparisonPredicate::Greater,
        core::ComparisonPredicate::GreaterEqual,
    };
    for (std::uint32_t Width : {8U, 16U, 32U, 64U})
    {
      SCOPED_TRACE(Width);
      const auto *Type = Test.Context.typePool().getType<ir::TypeKind::Integer>(Width, true);
      ASSERT_NE(Type, nullptr);
      const std::uint64_t Minimum = std::uint64_t{1} << (Width - 1);
      const std::uint64_t Maximum = Minimum - 1;
      const std::uint64_t NegativeOne = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
      struct ComparisonCase
      {
          std::uint64_t Left;
          std::uint64_t Right;
          int Ordering;
      };
      const ComparisonCase Cases[] = {
          {Minimum, Maximum, -1},
          {Maximum, Minimum, 1},
          {Minimum, 0, -1},
          {NegativeOne, 0, -1},
          {0, NegativeOne, 1},
          {Minimum, Minimum, 0},
          {Maximum, Maximum, 0},
      };
      for (core::ComparisonPredicate Predicate : Predicates)
      {
        SCOPED_TRACE(static_cast<unsigned>(Predicate));
        const ir::Type *Parameters[] = {Type, Type};
        auto Function = Test.function("Compare", Bool, Parameters);
        ASSERT_NE(Function, nullptr);
        ASSERT_TRUE(Test.begin(*Function));
        auto *Compare = Test.Builder.createCompareInstruction(Predicate, *Function->parameters()[0], *Function->parameters()[1]);
        ASSERT_NE(Compare, nullptr);
        ASSERT_NE(Test.Builder.createReturnInstruction(Compare), nullptr);
        for (const auto &Case : Cases)
        {
          const ExecutionValueRef Arguments[] = {Test.Engine.heap().integer(*Type, ExecutionInteger(Width, Case.Left)), Test.Engine.heap().integer(*Type, ExecutionInteger(Width, Case.Right))};
          const auto Result = Test.Engine.execute(*Function, Arguments);
          ASSERT_TRUE(Result);
          ASSERT_EQ(Result.Value.kind(), RuntimeKind::Boolean);
          EXPECT_EQ(Result.Value.boolean(), comparisonResult(Predicate, Case.Ordering));
        }
        Test.Builder.clearInsertPoint();
      }
    }
  }

  // Deep calls suspend and resume VM frames without consuming the C++ stack, then release every local allocation.
  TEST(BytecodeExecutionTest, DeepCallChainUsesExplicitFramesAndUnwindsStorage)
  {
    constexpr std::size_t Depth = 1024;
    ExecutionLimits Limits;
    Limits.MaxSteps = 100000;
    Limits.MaxCallDepth = Depth + 16;
    Limits.MaxEvaluationDepth = Depth * 4;
    Limits.MaxObjects = Depth * 4;
    BytecodeExecutionContext Test(Limits);
    std::vector<std::unique_ptr<ir::Function>> Functions;
    Functions.reserve(Depth);
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      auto Function = Test.function("Chain" + std::to_string(Index), Test.Int32);
      ASSERT_NE(Function, nullptr);
      Functions.push_back(std::move(Function));
    }
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      ASSERT_TRUE(Test.begin(*Functions[Index]));
      auto *Slot = Test.Builder.createAllocaInstruction(Test.Int32);
      ASSERT_NE(Slot, nullptr);
      ASSERT_NE(Test.Builder.createStoreInstruction(*Slot, Test.constant(1)), nullptr);
      const ir::Value *ChildResult = &Test.constant(0);
      if (Index + 1 < Depth)
      {
        ChildResult = Test.Builder.createCallInstruction(*Functions[Index + 1]);
        ASSERT_NE(ChildResult, nullptr);
      }
      auto *Local = Test.Builder.createLoadInstruction(*Slot);
      ASSERT_NE(Local, nullptr);
      auto *Sum = Test.Builder.createAddInstruction(*ChildResult, *Local);
      ASSERT_NE(Sum, nullptr);
      ASSERT_NE(Test.Builder.createReturnInstruction(Sum), nullptr);
    }
    Test.Builder.clearInsertPoint();
    expectInteger(Test.Engine.execute(*Functions.front()), Depth);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Test.Engine.heap().liveStorageBytes(), 0U);
    EXPECT_EQ(Test.Engine.heap().liveValueCount(), 0U);
    expectInteger(Test.Engine.execute(*Functions.front()), Depth);
    EXPECT_EQ(Test.Engine.heap().liveStorageCount(), 0U);
    Limits.MaxCallDepth = 32;
    ExecutionEngine Limited(Test.Context, Limits);
    EXPECT_EQ(Limited.execute(*Functions.front()).Status, ExecutionStatus::BudgetExceeded);
    EXPECT_EQ(Limited.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Limited.heap().liveValueCount(), 0U);
  }
} // namespace ink::execution::test
