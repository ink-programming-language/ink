#include "external_call_test_support.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/context.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

#include <bit>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#if defined(_WIN32) || defined(__linux__)
namespace ink::execution::test
{
  namespace
  {
    class ExternalContext final
    {
      public:
        explicit ExternalContext(core::TargetContext Target = core::TargetContext::native())
            : Compilation(Target),
              Context(Compilation),
              Builder(Context),
              Engine(Context),
              Int32(*Context.typePool().getType<ir::TypeKind::Integer>(32, true)),
              CountType(*Context.typePool().getType<ir::TypeKind::Integer>(WriteCountWidth, false)),
              ReturnType(*Context.typePool().getType<ir::TypeKind::Integer>(WriteReturnWidth, true))
        {
        }

        const ir::IntegerType &integerType(std::uint32_t Width, bool Signed)
        {
          return *Context.typePool().getType<ir::TypeKind::Integer>(Width, Signed);
        }

        const ir::IntegerConstant &integer(const ir::IntegerType &Type, std::uint64_t Value)
        {
          return *Context.constantPool().getIntegerConstant(Type, ir::IntegerBits(Type.bitWidth(), Value));
        }

        const ir::FloatConstant &floating(std::uint32_t Width, std::uint64_t Bits)
        {
          const auto *Type = Context.typePool().getType<ir::TypeKind::Float>(Width);
          return *Context.constantPool().getFloatConstant(*Type, ir::FloatBits(Width, Bits));
        }

        const ir::StringConstant &string(std::string_view Value)
        {
          const auto *Slice = Context.typePool().getType<ir::TypeKind::Slice>(integerType(8, false), ir::AccessKind::ReadOnly);
          return *Context.constantPool().getStringConstant(*Slice, Value);
        }

        std::unique_ptr<ir::Function> function(std::string_view Name, const ir::Type &Result, std::span<const ir::Type *const> Parameters = {})
        {
          const auto *Signature = Context.typePool().getType<ir::TypeKind::Function>(Result, Parameters);
          return Signature ? Builder.createFunction(Context.namePool().intern(Name), *Signature, {}, {}, ir::CallingConvention::C, ir::LanguageLinkage::C) : nullptr;
        }

        std::unique_ptr<ir::Function> writeFunction(bool VoidBuffer = false)
        {
          const ir::Type &Pointee = VoidBuffer ? Context.typePool().getType<ir::TypeKind::Void>() : static_cast<const ir::Type &>(integerType(8, false));
          const auto *Pointer = Context.typePool().getType<ir::TypeKind::Pointer>(Pointee, ir::AccessKind::ReadWrite);
          const ir::Type *Parameters[] = {&Int32, Pointer, &CountType};
          return function(WriteSymbol, ReturnType, Parameters);
        }

        core::CompilationContext Compilation;
        ir::IRContext Context;
        ir::IRBuilder Builder;
        ExecutionEngine Engine;
        const ir::IntegerType &Int32;
        const ir::IntegerType &CountType;
        const ir::IntegerType &ReturnType;
    };
  } // namespace

  // The exact platform symbol copies UTF-8 and embedded NUL bytes through libffi without running an Ink callback.
  TEST(ExecutionExternalCallTest, WritesExactBytesThroughTheNativeSymbol)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    ExternalContext Test;
    auto Function = Test.writeFunction();
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    std::string Payload = "\xe4\xb8\xad\xe6\x96\x87";
    Payload.push_back('\0');
    Payload += "tail";
    const ir::Value *Arguments[] = {&Test.integer(Test.Int32, Pipe.writer()), &Test.string(Payload), &Test.integer(Test.CountType, Payload.size())};
    unsigned BodyCalls = 0;
    const auto Result = Test.Engine.call(*Function, *Module, Arguments, [&](ExecutionFrame &) -> ExecutionResult
    {
      ++BodyCalls;
      return {};
    });
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value, &Test.integer(Test.ReturnType, Payload.size()));
    EXPECT_EQ(BodyCalls, 0U);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, Payload);
  }

  // A void-pointer parameter receives a valid temporary buffer even for a zero-byte native write.
  TEST(ExecutionExternalCallTest, SupportsVoidPointerAndZeroByteWrites)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    ExternalContext Test;
    auto Function = Test.writeFunction(true);
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *Arguments[] = {&Test.integer(Test.Int32, Pipe.writer()), &Test.string(""), &Test.integer(Test.CountType, 0)};
    const auto Result = Test.Engine.call(*Function, *Module, Arguments, {});
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value, &Test.integer(Test.ReturnType, 0));
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_TRUE(Output.empty());
  }

  // Exported zero-argument and void functions are resolved by name and share observable native state across calls.
  TEST(ExecutionExternalCallTest, CallsZeroArgumentAndVoidFunctionsByTheirActualNames)
  {
    ExternalContext Test;
    const ir::Type *Parameters[] = {&Test.Int32};
    auto Zero = Test.function("inkTestExternalZero", Test.Int32);
    auto Set = Test.function("inkTestExternalSet", Test.Context.typePool().getType<ir::TypeKind::Void>(), Parameters);
    auto Get = Test.function("inkTestExternalGet", Test.Int32);
    ASSERT_NE(Zero, nullptr);
    ASSERT_NE(Set, nullptr);
    ASSERT_NE(Get, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *Arguments[] = {&Test.integer(Test.Int32, 12345)};
    EXPECT_EQ(Test.Engine.call(*Zero, *Module, {}, {}).Value, &Test.integer(Test.Int32, 37));
    const auto Stored = Test.Engine.call(*Set, *Module, Arguments, {});
    ASSERT_TRUE(Stored);
    EXPECT_EQ(Stored.Value, nullptr);
    EXPECT_EQ(Test.Engine.call(*Get, *Module, {}, {}).Value, &Test.integer(Test.Int32, 12345));
  }

  // Mixed native integer widths preserve signed inputs, unsigned upper bits and a negative 64-bit return.
  TEST(ExecutionExternalCallTest, MarshalsAllSupportedIntegerWidths)
  {
    ExternalContext Test;
    const auto &I8 = Test.integerType(8, true);
    const auto &U16 = Test.integerType(16, false);
    const auto &I64 = Test.integerType(64, true);
    const auto &U64 = Test.integerType(64, false);
    const ir::Type *MixParameters[] = {&I8, &U16, &Test.Int32, &U64};
    auto Mix = Test.function("inkTestExternalMix", I64, MixParameters);
    const auto &U8 = Test.integerType(8, false);
    const auto &I16 = Test.integerType(16, true);
    const auto &U32 = Test.integerType(32, false);
    const ir::Type *NarrowParameters[] = {&U8, &I16, &U32, &I64};
    auto Narrow = Test.function("inkTestExternalNarrow", U64, NarrowParameters);
    ASSERT_NE(Mix, nullptr);
    ASSERT_NE(Narrow, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *MixArguments[] = {&Test.integer(I8, static_cast<std::uint8_t>(-7)), &Test.integer(U16, 60000), &Test.integer(Test.Int32, static_cast<std::uint32_t>(-100000)), &Test.integer(U64, 0xf000000000000000ULL)};
    const ir::Value *NarrowArguments[] = {&Test.integer(U8, 255), &Test.integer(I16, static_cast<std::uint16_t>(-30000)), &Test.integer(U32, 4000000000ULL), &Test.integer(I64, static_cast<std::uint64_t>(-9999999983LL))};
    const auto Mixed = Test.Engine.call(*Mix, *Module, MixArguments, {});
    const auto Narrowed = Test.Engine.call(*Narrow, *Module, NarrowArguments, {});
    ASSERT_TRUE(Mixed);
    ASSERT_TRUE(Narrowed);
    EXPECT_EQ(Mixed.Value, &Test.integer(I64, static_cast<std::uint64_t>(-39992)));
    EXPECT_EQ(Narrowed.Value, &Test.integer(U64, 4000000272ULL));
  }

  // Narrow native returns are extracted at their declared width instead of retaining libffi return-register extension bits.
  TEST(ExecutionExternalCallTest, PreservesSignedAndUnsignedNarrowReturnBits)
  {
    ExternalContext Test;
    const auto &I8 = Test.integerType(8, true);
    const auto &U16 = Test.integerType(16, false);
    const ir::Type *SignedParameters[] = {&I8};
    const ir::Type *UnsignedParameters[] = {&U16};
    auto Negate = Test.function("inkTestExternalNegate", I8, SignedParameters);
    auto Increment = Test.function("inkTestExternalIncrement", U16, UnsignedParameters);
    ASSERT_NE(Negate, nullptr);
    ASSERT_NE(Increment, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *SignedArguments[] = {&Test.integer(I8, 7)};
    const ir::Value *UnsignedArguments[] = {&Test.integer(U16, 60000)};
    EXPECT_EQ(Test.Engine.call(*Negate, *Module, SignedArguments, {}).Value, &Test.integer(I8, static_cast<std::uint8_t>(-7)));
    EXPECT_EQ(Test.Engine.call(*Increment, *Module, UnsignedArguments, {}).Value, &Test.integer(U16, 60001));
  }

  // libffi marshals arguments beyond platform register capacity in the declaration's original order.
  TEST(ExecutionExternalCallTest, CallsFunctionsWithMoreArgumentsThanRegisters)
  {
    ExternalContext Test;
    const auto &I64 = Test.integerType(64, true);
    std::vector<const ir::Type *> ParameterTypes(10, &Test.Int32);
    auto Function = Test.function("inkTestExternalMany", I64, ParameterTypes);
    ASSERT_NE(Function, nullptr);
    std::vector<const ir::Value *> Arguments;
    for (std::uint64_t Value = 1; Value <= 10; ++Value)
    {
      Arguments.push_back(&Test.integer(Test.Int32, Value));
    }
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const auto Result = Test.Engine.call(*Function, *Module, Arguments, {});
    ASSERT_TRUE(Result);
    EXPECT_EQ(Result.Value, &Test.integer(I64, 385));
  }

  // Mixed single- and double-precision arguments and both floating return formats use their native ABI registers.
  TEST(ExecutionExternalCallTest, MarshalsFloatingArgumentsAndReturns)
  {
    ExternalContext Test;
    const auto *F32 = Test.Context.typePool().getType<ir::TypeKind::Float>(32);
    const auto *F64 = Test.Context.typePool().getType<ir::TypeKind::Float>(64);
    ASSERT_NE(F32, nullptr);
    ASSERT_NE(F64, nullptr);
    const ir::Type *MixedParameters[] = {F32, F64};
    const ir::Type *SingleParameters[] = {F32};
    auto Mixed = Test.function("inkTestExternalFloat", *F64, MixedParameters);
    auto Single = Test.function("inkTestExternalFloat32", *F32, SingleParameters);
    ASSERT_NE(Mixed, nullptr);
    ASSERT_NE(Single, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const auto &OneAndHalf = Test.floating(32, std::bit_cast<std::uint32_t>(1.5F));
    const auto &TwoAndQuarter = Test.floating(64, std::bit_cast<std::uint64_t>(2.25));
    const ir::Value *MixedArguments[] = {&OneAndHalf, &TwoAndQuarter};
    const ir::Value *SingleArguments[] = {&OneAndHalf};
    const auto MixedResult = Test.Engine.call(*Mixed, *Module, MixedArguments, {});
    const auto SingleResult = Test.Engine.call(*Single, *Module, SingleArguments, {});
    ASSERT_TRUE(MixedResult);
    ASSERT_TRUE(SingleResult);
    EXPECT_EQ(MixedResult.Value, &Test.floating(64, std::bit_cast<std::uint64_t>(5.25)));
    EXPECT_EQ(SingleResult.Value, &Test.floating(32, std::bit_cast<std::uint32_t>(2.0F)));
  }

  // Boolean native arguments and returns remain canonical true/false values through the generic bridge.
  TEST(ExecutionExternalCallTest, MarshalsBooleanArgumentsAndReturns)
  {
    ExternalContext Test;
    const auto &Bool = Test.Context.typePool().getType<ir::TypeKind::Bool>();
    const ir::Type *Parameters[] = {&Bool};
    auto Function = Test.function("inkTestExternalNot", Bool, Parameters);
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const auto &True = Test.Context.constantPool().getBoolConstant(true);
    const auto &False = Test.Context.constantPool().getBoolConstant(false);
    const ir::Value *TrueArguments[] = {&True};
    const ir::Value *FalseArguments[] = {&False};
    EXPECT_EQ(Test.Engine.call(*Function, *Module, TrueArguments, {}).Value, &False);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, FalseArguments, {}).Value, &True);
  }

  // Pointer arguments receive writable NUL-terminated copies, preserving embedded NUL bytes and immutable pool contents.
  TEST(ExecutionExternalCallTest, CopiesStringBuffersBeforeNativeMutation)
  {
    ExternalContext Test;
    const auto &U32 = Test.integerType(32, false);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.integerType(8, false), ir::AccessKind::ReadWrite);
    const ir::Type *ReadParameters[] = {Pointer, &U32};
    const ir::Type *MutateParameters[] = {Pointer};
    auto Byte = Test.function("inkTestExternalByte", U32, ReadParameters);
    auto Mutate = Test.function("inkTestExternalMutate", U32, MutateParameters);
    ASSERT_NE(Byte, nullptr);
    ASSERT_NE(Mutate, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const auto &Buffer = Test.string(std::string_view("A\0B", 3));
    const ir::Value *NullArguments[] = {&Buffer, &Test.integer(U32, 1)};
    const ir::Value *TailArguments[] = {&Buffer, &Test.integer(U32, 2)};
    const ir::Value *TerminatorArguments[] = {&Buffer, &Test.integer(U32, 3)};
    const ir::Value *MutateArguments[] = {&Buffer};
    EXPECT_EQ(Test.Engine.call(*Byte, *Module, NullArguments, {}).Value, &Test.integer(U32, 0));
    EXPECT_EQ(Test.Engine.call(*Byte, *Module, TailArguments, {}).Value, &Test.integer(U32, 'B'));
    EXPECT_EQ(Test.Engine.call(*Byte, *Module, TerminatorArguments, {}).Value, &Test.integer(U32, 0));
    EXPECT_EQ(Test.Engine.call(*Mutate, *Module, MutateArguments, {}).Value, &Test.integer(U32, 'Z'));
    EXPECT_EQ(Buffer.value(), std::string_view("A\0B", 3));
  }

  // Unknown symbols and unsupported native types are rejected before any mismatched native signature can be invoked.
  TEST(ExecutionExternalCallTest, RejectsMissingSymbolsAndUnsupportedTypes)
  {
    ExternalContext Test;
    auto Missing = Test.function("InkMissingExternalSymbol94c8f17e", Test.Int32);
    auto Wide = Test.function("inkTestExternalZero", Test.integerType(128, true));
    const auto *Half = Test.Context.typePool().getType<ir::TypeKind::Float>(16);
    ASSERT_NE(Half, nullptr);
    auto HalfReturn = Test.function("inkTestExternalZero", *Half);
    ASSERT_NE(Missing, nullptr);
    ASSERT_NE(Wide, nullptr);
    ASSERT_NE(HalfReturn, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    EXPECT_EQ(Test.Engine.call(*Missing, *Module, {}, {}).Status, ExecutionStatus::SymbolNotFound);
    EXPECT_EQ(Test.Engine.call(*Wide, *Module, {}, {}).Status, ExecutionStatus::UnsupportedExternalSignature);
    EXPECT_EQ(Test.Engine.call(*HalfReturn, *Module, {}, {}).Status, ExecutionStatus::UnsupportedExternalSignature);
  }

  // Native scalar parameters require exact local constant types and counts before the host function can execute.
  TEST(ExecutionExternalCallTest, RejectsInvalidNativeArgumentsBeforeCalling)
  {
    ExternalContext Test;
    ExternalContext Other;
    const ir::Type *Parameters[] = {&Test.Int32};
    auto Function = Test.function("inkTestExternalSet", Test.Context.typePool().getType<ir::TypeKind::Void>(), Parameters);
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *Null[] = {nullptr};
    const ir::Value *Wrong[] = {&Test.Context.constantPool().getBoolConstant(true)};
    const ir::Value *Foreign[] = {&Other.integer(Other.Int32, 1)};
    EXPECT_EQ(Test.Engine.call(*Function, *Module, {}, {}).Status, ExecutionStatus::InvalidArguments);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Null, {}).Status, ExecutionStatus::InvalidArguments);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Wrong, {}).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Foreign, {}).Status, ExecutionStatus::ForeignContext);
  }

  // A same-typed IR parameter cannot reach native code, while a constant passed through the same Value array can.
  TEST(ExecutionExternalCallTest, RejectsUnevaluatedValuesWithoutChangingNativeState)
  {
    ExternalContext Test;
    const ir::Type *Parameters[] = {&Test.Int32};
    auto Set = Test.function("inkTestExternalSet", Test.Context.typePool().getType<ir::TypeKind::Void>(), Parameters);
    auto Get = Test.function("inkTestExternalGet", Test.Int32);
    ASSERT_NE(Set, nullptr);
    ASSERT_NE(Get, nullptr);
    ASSERT_EQ(Set->parameters().size(), 1U);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *Arguments[] = {&Test.integer(Test.Int32, 6789)};
    ASSERT_TRUE(Test.Engine.call(*Set, *Module, Arguments));
    Arguments[0] = Set->parameters().front().get();
    EXPECT_EQ(Test.Engine.call(*Set, *Module, Arguments).Status, ExecutionStatus::RuntimeValue);
    const auto Observed = Test.Engine.call(*Get, *Module, {});
    ASSERT_TRUE(Observed);
    EXPECT_EQ(Observed.Value, &Test.integer(Test.Int32, 6789));
    Arguments[0] = &Test.integer(Test.Int32, 1357);
    ASSERT_TRUE(Test.Engine.call(*Set, *Module, Arguments));
    const auto Updated = Test.Engine.call(*Get, *Module, {});
    ASSERT_TRUE(Updated);
    EXPECT_EQ(Updated.Value, &Test.integer(Test.Int32, 1357));
  }

  // An explicitly described target cannot invoke host code even when its pointer width matches the host.
  TEST(ExecutionExternalCallTest, RejectsNonNativeTargetBeforeCallingHostCode)
  {
    const auto Native = core::TargetContext::native();
    ExternalContext Test(core::TargetContext(Native.pointerWidth(), Native.byteOrder()));
    auto Function = Test.function("inkTestExternalZero", Test.Int32);
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    EXPECT_EQ(Test.Engine.call(*Function, *Module, {}, {}).Status, ExecutionStatus::HostAbiMismatch);
  }

  // Repeated queries for one event do not replay native output, while a distinct event makes a new generic call.
  TEST(ExecutionExternalCallTest, CommitsExternalSideEffectsOncePerSemanticEvent)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    ExternalContext Test;
    auto Function = Test.writeFunction();
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *Arguments[] = {&Test.integer(Test.Int32, Pipe.writer()), &Test.string("X"), &Test.integer(Test.CountType, 1)};
    int FirstEvent = 0;
    int SecondEvent = 0;
    unsigned Executions = 0;
    auto Write = [&]() -> ExecutionResult
    {
      ++Executions;
      return Test.Engine.call(*Function, *Module, Arguments, {});
    };
    const auto First = Test.Engine.executeOnce(*Module, &FirstEvent, Write);
    const auto Repeated = Test.Engine.executeOnce(*Module, &FirstEvent, Write);
    const auto Second = Test.Engine.executeOnce(*Module, &SecondEvent, Write);
    ASSERT_TRUE(First);
    ASSERT_TRUE(Repeated);
    ASSERT_TRUE(Second);
    EXPECT_EQ(Repeated.Value, First.Value);
    EXPECT_EQ(Executions, 2U);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_EQ(Output, "XX");
  }

  // Cancellation is checked before entering a generic external function and leaves the output pipe untouched.
  TEST(ExecutionExternalCallTest, CancellationPreventsNativeSideEffects)
  {
    NativePipe Pipe;
    ASSERT_TRUE(Pipe.valid());
    ExternalContext Test;
    auto Function = Test.writeFunction();
    ASSERT_NE(Function, nullptr);
    ExecutionFrame *Module = Test.Engine.createFrame(ExecutionFrameKind::Module);
    ASSERT_NE(Module, nullptr);
    const ir::Value *Arguments[] = {&Test.integer(Test.Int32, Pipe.writer()), &Test.string("A"), &Test.integer(Test.CountType, 1)};
    Test.Engine.cancel();
    EXPECT_EQ(Test.Engine.call(*Function, *Module, Arguments, {}).Status, ExecutionStatus::Cancelled);
    std::string Output;
    ASSERT_TRUE(Pipe.readAll(Output));
    EXPECT_TRUE(Output.empty());
  }
} // namespace ink::execution::test
#endif

