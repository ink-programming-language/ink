#include "ink/execution/bridge/semantic_value_bridge.h"
#include "ink/execution/bytecode/execution_compiler.h"
#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_linker.h"
#include "ink/execution/engine/execution_machine.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <cstdint>

#ifdef _WIN32
#define INK_IMAGE_EXPORT extern "C" __declspec(dllexport)
#else
#define INK_IMAGE_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace ink::execution::test
{
  INK_IMAGE_EXPORT std::int32_t inkTestOwnedImageStringLength(const char *Value) noexcept
  {
    std::int32_t Size = 0;
    while (Value[Size])
    {
      ++Size;
    }
    return Size;
  }

  // An external entry in an image without its type table fails explicitly before native signature access.
  TEST(ExecutionImageTest, RejectsMissingLayoutTable)
  {
    ExecutionImage Image;
    RuntimeFunctionDescriptor Descriptor;
    Descriptor.Id = 0;
    Descriptor.External = true;
    Image.Descriptors.emplace(0, Descriptor);
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionEngine Engine(Context);
    ExecutionLinker Linker(std::move(Image));
    ExecutionMachine Machine(Engine, Linker);
    EXPECT_EQ(Machine.execute(0, {}).Status, ExecutionStatus::InvalidArguments);
  }

  // Runtime entry values must contain the pointer or wide-integer payload required by their declared type ID.
  TEST(ExecutionImageTest, RejectsMalformedEntryPayloads)
  {
    for (bool Pointer : {false, true})
    {
      ExecutionImage Image;
      auto Layouts = std::make_shared<RuntimeTypeTable>();
      TypeDesc Integer;
      Integer.setKind(RuntimeKind::Integer);
      Integer.setBitWidth(128);
      const RuntimeTypeId IntegerId = Layouts->append(Integer);
      TypeDesc PointerLayout;
      PointerLayout.setKind(RuntimeKind::Pointer);
      PointerLayout.editPointer().Pointee = IntegerId;
      const RuntimeTypeId PointerId = Layouts->append(PointerLayout);
      const RuntimeTypeId Parameter = Pointer ? PointerId : IntegerId;
      TypeDesc Signature;
      Signature.setKind(RuntimeKind::Function);
      Signature.editFunction().ReturnType = Parameter;
      Signature.editFunction().Parameters = {Parameter};
      const RuntimeTypeId SignatureId = Layouts->append(Signature);
      auto Function = std::make_unique<ExecutableFunction>();
      Function->Id = 0;
      Function->Signature = SignatureId;
      Function->Layouts = Layouts;
      Function->SlotTypes = {Parameter};
      Function->InitialSlots.resize(1);
      Function->InitialSlots[0].Type = Parameter;
      Function->Code.push_back({BytecodeOpcode::Return, {0, 0}});
      Image.Layouts = Layouts;
      RuntimeFunctionDescriptor Descriptor;
      Descriptor.Id = 0;
      Descriptor.Signature = SignatureId;
      Image.Descriptors.emplace(0, Descriptor);
      Image.Functions.emplace(0, std::move(Function));
      core::CompilationContext Compilation;
      ir::IRContext Context(Compilation);
      ExecutionEngine Engine(Context);
      ExecutionLinker Linker(std::move(Image));
      ExecutionMachine Machine(Engine, Linker);
      const RuntimeValue Arguments[] = {RuntimeValue::fromBits(0, Parameter)};
      EXPECT_EQ(Machine.execute(0, Arguments).Status, ExecutionStatus::TypeMismatch);
    }
  }

  // A table key cannot redirect execution to a function with another identity and frame layout.
  TEST(ExecutionImageTest, RejectsMismatchedFunctionTableIdentity)
  {
    ExecutionImage Image;
    auto Layouts = std::make_shared<RuntimeTypeTable>();
    TypeDesc Void;
    Void.setKind(RuntimeKind::Void);
    const RuntimeTypeId VoidId = Layouts->append(Void);
    TypeDesc Signature;
    Signature.setKind(RuntimeKind::Function);
    Signature.editFunction().ReturnType = VoidId;
    const RuntimeTypeId SignatureId = Layouts->append(Signature);
    Image.Layouts = Layouts;
    RuntimeFunctionDescriptor Descriptor;
    Descriptor.Signature = SignatureId;
    Descriptor.Id = 0;
    Image.Descriptors.emplace(0, Descriptor);
    Descriptor.Id = 1;
    Image.Descriptors.emplace(1, Descriptor);
    auto Function = std::make_unique<ExecutableFunction>();
    Function->Id = 1;
    Function->Signature = SignatureId;
    Function->Layouts = Layouts;
    Function->Code.push_back({BytecodeOpcode::ReturnVoid});
    Image.Functions.emplace(0, std::move(Function));
    core::CompilationContext Compilation;
    ir::IRContext Context(Compilation);
    ExecutionEngine Engine(Context);
    ExecutionLinker Linker(std::move(Image));
    ExecutionMachine Machine(Engine, Linker);
    EXPECT_EQ(Machine.execute(0, {}).Status, ExecutionStatus::InvalidArguments);
  }

  // Linked calls, frame locals and wide constants execute after every originating IR object has been destroyed.
  TEST(ExecutionImageTest, ExecutesOwnedFunctionsAfterSemanticContextDestruction)
  {
    ExecutionImage Image;
    FunctionId Entry = InvalidFunction;
    {
      core::CompilationContext Compilation;
      ir::IRContext Context(Compilation);
      ir::IRBuilder Builder(Context);
      SemanticValueBridge Bridge(Context);
      const auto &Type = *Context.typePool().getType<ir::TypeKind::Integer>(128, false);
      const auto &Signature = *Context.typePool().getType<ir::TypeKind::Function>(Type, std::span<const ir::Type *const>{});
      auto Callee = Builder.createFunction(Context.namePool().intern("OwnedCallee"), Signature);
      auto Caller = Builder.createFunction(Context.namePool().intern("OwnedCaller"), Signature);
      ASSERT_TRUE(Builder.setInsertPoint(*Builder.createFunctionBody(*Callee)));
      const std::uint64_t Words[] = {9, 3};
      const auto *Constant = Context.constantPool().getIntegerConstant(Type, ir::IntegerBits(128, Words));
      ASSERT_NE(Constant, nullptr);
      ASSERT_NE(Builder.createReturnInstruction(Constant), nullptr);
      ASSERT_TRUE(Builder.setInsertPoint(*Builder.createFunctionBody(*Caller)));
      auto *Local = Builder.createAllocaInstruction(Type);
      auto *Call = Builder.createCallInstruction(*Callee);
      ASSERT_NE(Local, nullptr);
      ASSERT_NE(Call, nullptr);
      ASSERT_NE(Builder.createStoreInstruction(*Local, *Call), nullptr);
      auto *Loaded = Builder.createLoadInstruction(*Local);
      ASSERT_NE(Loaded, nullptr);
      auto *Sum = Builder.createAddInstruction(*Loaded, *Constant);
      ASSERT_NE(Sum, nullptr);
      ASSERT_NE(Builder.createReturnInstruction(Sum), nullptr);
      for (const ir::Function *Source : {Callee.get(), Caller.get()})
      {
        auto Compiled = ExecutionCompiler{}.compile(*Source, Bridge);
        ASSERT_TRUE(Compiled);
        const FunctionId Id = Compiled.Function->Id;
        Image.Descriptors.emplace(Id, *Bridge.functionDescriptor(Id));
        Image.Functions.emplace(Id, std::move(Compiled.Function));
      }
      Entry = Bridge.lowerFunction(*Caller);
      Image.Layouts = Bridge.types();
    }
    core::CompilationContext RuntimeCompilation;
    ir::IRContext UnrelatedContext(RuntimeCompilation);
    ExecutionEngine Engine(UnrelatedContext);
    ExecutionLinker Linker(std::move(Image));
    ExecutionMachine Machine(Engine, Linker);
    const auto Result = Machine.execute(Entry, {});
    ASSERT_TRUE(Result);
    ASSERT_EQ(Result.Value.kind(), RuntimeKind::Integer);
    const std::uint64_t Expected[] = {18, 6};
    EXPECT_EQ(Result.Value.integer().bits(), ir::IntegerBits(128, Expected));
    EXPECT_EQ(Engine.heap().allocatedStorageCount(), 1U);
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
    EXPECT_EQ(Engine.heap().liveValueCount(), 0U);
  }

#if defined(_WIN32) || defined(__linux__)
  // CString bytes, external symbol names and ABI descriptors remain usable with no source context or compiler adapter.
  TEST(ExecutionImageTest, ExecutesOwnedCStringAndNativePlanAfterSemanticContextDestruction)
  {
    ExecutionImage Image;
    FunctionId Entry = InvalidFunction;
    {
      core::CompilationContext Compilation;
      ir::IRContext Context(Compilation);
      ir::IRBuilder Builder(Context);
      SemanticValueBridge Bridge(Context);
      const auto &Int32 = *Context.typePool().getType<ir::TypeKind::Integer>(32, true);
      const auto &Byte = *Context.typePool().getType<ir::TypeKind::Integer>(8, false);
      const auto &Pointer = *Context.typePool().getType<ir::TypeKind::Pointer>(Byte, ir::AccessKind::ReadWrite);
      const ir::Type *Parameters[] = {&Pointer};
      const auto &NativeSignature = *Context.typePool().getType<ir::TypeKind::Function>(Int32, Parameters);
      auto Native = Builder.createFunction(Context.namePool().intern("inkTestOwnedImageStringLength"), NativeSignature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C, core::FunctionBinding::Import);
      const auto &EntrySignature = *Context.typePool().getType<ir::TypeKind::Function>(Int32, std::span<const ir::Type *const>{});
      auto Function = Builder.createFunction(Context.namePool().intern("OwnedBytes"), EntrySignature);
      ASSERT_TRUE(Builder.setInsertPoint(*Builder.createFunctionBody(*Function)));
      const auto &StringType = *Context.typePool().getType<ir::TypeKind::Slice>(Byte, ir::AccessKind::ReadOnly);
      const auto *Constant = Context.constantPool().getStringConstant(StringType, "owned bytes");
      ASSERT_NE(Constant, nullptr);
      auto *String = Builder.createCStringInstruction(*Constant);
      ASSERT_NE(String, nullptr);
      const ir::Value *Arguments[] = {String};
      auto *Call = Builder.createCallInstruction(*Native, Arguments);
      ASSERT_NE(Call, nullptr);
      ASSERT_NE(Builder.createReturnInstruction(Call), nullptr);
      auto Compiled = ExecutionCompiler{}.compile(*Function, Bridge);
      ASSERT_TRUE(Compiled);
      Entry = Compiled.Function->Id;
      const FunctionId NativeId = Bridge.lowerFunction(*Native);
      Image.Descriptors.emplace(Entry, *Bridge.functionDescriptor(Entry));
      Image.Descriptors.emplace(NativeId, *Bridge.functionDescriptor(NativeId));
      Image.Functions.emplace(Entry, std::move(Compiled.Function));
      Image.Layouts = Bridge.types();
    }
    core::CompilationContext RuntimeCompilation;
    ir::IRContext UnrelatedContext(RuntimeCompilation);
    ExecutionEngine Engine(UnrelatedContext);
    ExecutionLinker Linker(std::move(Image));
    ExecutionMachine Machine(Engine, Linker);
    for (unsigned Iteration = 0; Iteration < 2; ++Iteration)
    {
      const auto Result = Machine.execute(Entry, {});
      ASSERT_TRUE(Result);
      EXPECT_EQ(Result.Value.Bits, 11U);
      EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
      EXPECT_EQ(Engine.heap().liveValueCount(), 0U);
    }
    EXPECT_EQ(Engine.heap().allocatedStorageCount(), 2U);
  }
#endif
} // namespace ink::execution::test

#undef INK_IMAGE_EXPORT
