#include "ink/execution/ffi/native_call_cache.h"
#include "ink/execution/ffi/native_symbol_cache.h"

#include <gtest/gtest.h>

#include <bit>
#include <cstdint>
#include <optional>
#include <string_view>

namespace ink::execution::test
{
  namespace
  {
    class RuntimeCallContext final
    {
      public:
        RuntimeCallContext()
            : Int32(scalar(RuntimeKind::Integer, 32, true)),
              Float32(scalar(RuntimeKind::Float, 32)),
              Byte(scalar(RuntimeKind::Integer, 8)),
              Boolean(scalar(RuntimeKind::Boolean, 8)),
              String(scalar(RuntimeKind::String, 0)),
              BytePointer(pointer(Byte)),
              Int32Pointer(pointer(Int32))
        {
        }

        RuntimeTypeId scalar(RuntimeKind Kind, std::uint32_t Width, bool Signed = false)
        {
          TypeDesc Layout;
          Layout.setKind(Kind);
          if (Kind == RuntimeKind::Integer || Kind == RuntimeKind::Float)
          {
            Layout.setBitWidth(Width);
          }
          Layout.Size = Width / 8;
          Layout.Alignment = Layout.Size == 0 ? 1 : Layout.Size;
          if (Kind == RuntimeKind::Integer)
          {
            Layout.editInteger().Signed = Signed;
          }
          Layout.Native = Kind == RuntimeKind::Integer || Kind == RuntimeKind::Float || Kind == RuntimeKind::Boolean;
          return Types.append(std::move(Layout));
        }

        RuntimeTypeId pointer(RuntimeTypeId Pointee)
        {
          TypeDesc Layout;
          Layout.setKind(RuntimeKind::Pointer);
          Layout.Size = sizeof(void *);
          Layout.Alignment = alignof(void *);
          Layout.editPointer().Pointee = Pointee;
          Layout.editPointer().Writable = true;
          return Types.append(std::move(Layout));
        }

        RuntimeFunctionDescriptor function(std::string_view Symbol, RuntimeTypeId Return, std::initializer_list<RuntimeTypeId> Parameters)
        {
          TypeDesc Layout;
          Layout.setKind(RuntimeKind::Function);
          Layout.editFunction().ReturnType = Return;
          Layout.editFunction().Parameters = Parameters;
          RuntimeFunctionDescriptor Function;
          Function.Id = NextFunction++;
          Function.Signature = Types.append(std::move(Layout));
          Function.Symbol = Symbol;
          Function.External = true;
          Function.NativeAbi = true;
          Function.Supported = true;
          return Function;
        }

        RuntimeTypeTable Types;
        ExecutionMemoryManager Memory;
        NativeCallCache Calls;
        RuntimeTypeId Int32;
        RuntimeTypeId Float32;
        RuntimeTypeId Byte;
        RuntimeTypeId Boolean;
        RuntimeTypeId String;
        RuntimeTypeId BytePointer;
        RuntimeTypeId Int32Pointer;
        FunctionId NextFunction = 0;
    };

    extern "C" std::int32_t incrementRuntimeInteger(std::int32_t Value) noexcept
    {
      return Value + 1;
    }

    extern "C" std::int32_t incrementRuntimeIntegerTwice(std::int32_t Value) noexcept
    {
      return Value + 2;
    }

    extern "C" std::uint8_t incrementRuntimeByte(std::uint8_t Value) noexcept
    {
      return static_cast<std::uint8_t>(Value + 1);
    }

    extern "C" char *mutateRuntimeString(char *Text) noexcept
    {
      Text[1] = 'Z';
      return Text + 1;
    }

    extern "C" std::int32_t readRuntimeInteger(std::int32_t *Value) noexcept
    {
      return *Value;
    }
  } // namespace

  // Native signatures, values and cached calls work without constructing any semantic IR context.
  TEST(RuntimeFfiTest, ExecutesPreparedScalarsWithoutSemanticObjects)
  {
    RuntimeCallContext Test;
    const auto Function = Test.function("increment", Test.Int32, {Test.Int32});
    unsigned Resolutions = 0;
    NativeSymbolCache Symbols([&](std::string_view) -> NativeSymbol
    {
      ++Resolutions;
      return reinterpret_cast<NativeSymbol>(&incrementRuntimeInteger);
    });
    for (const std::uint64_t Bits : {9, 19, 29})
    {
      const RuntimeValue Arguments[] = {RuntimeValue::fromBits(Bits, Test.Int32)};
      const auto Result = Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments);
      ASSERT_TRUE(Result);
      EXPECT_EQ(Result.Value.Type, Test.Int32);
      EXPECT_EQ(Result.Value.Bits, Bits + 1);
    }
    EXPECT_EQ(Resolutions, 1U);
    EXPECT_EQ(Test.Calls.size(), 1U);
  }

  // Same-width booleans and bytes retain distinct identities at the native argument boundary.
  TEST(RuntimeFfiTest, RejectsSameSizeTypeConfusionAfterPreparingPlan)
  {
    RuntimeCallContext Test;
    const auto Function = Test.function("incrementByte", Test.Byte, {Test.Byte});
    NativeSymbolCache Symbols([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&incrementRuntimeByte);
    });
    RuntimeValue Arguments[] = {RuntimeValue::fromBits(8, Test.Byte)};
    ASSERT_TRUE(Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments));
    Arguments[0] = RuntimeValue::fromBits(1, Test.Boolean);
    EXPECT_EQ(Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments).Status, ExecutionStatus::TypeMismatch);
  }

  // A writable string copy returned by interior pointer preserves storage identity and expires on explicit release.
  TEST(RuntimeFfiTest, PromotesReturnedStringAliasWithoutMutatingItsSource)
  {
    RuntimeCallContext Test;
    const auto Function = Test.function("mutateString", Test.BytePointer, {Test.BytePointer});
    NativeSymbolCache Symbols([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&mutateRuntimeString);
    });
    const RuntimeValue Arguments[] = {RuntimeValue::fromString("ABC", Test.String)};
    const auto Result = Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments);
    ASSERT_TRUE(Result);
    ASSERT_EQ(Result.Value.kind(), RuntimeKind::Pointer);
    EXPECT_EQ(Result.Value.pointer().kind(), ExecutionPointer::Kind::Native);
    EXPECT_EQ(std::string_view(static_cast<const char *>(Result.Value.pointer().address())), "ZC");
    EXPECT_EQ(Arguments[0].string(), "ABC");
    EXPECT_EQ(Test.Memory.liveStorageCount(), 1U);
    EXPECT_EQ(Test.Memory.release(Test.Memory.storageFromAddress(Result.Value.pointer().address())), ExecutionStatus::Success);
    EXPECT_EQ(Result.Value.pointer().status(), ExecutionStatus::Success);
  }

  // Native pointers cannot reinterpret a same-size floating cell as an integer cell.
  TEST(RuntimeFfiTest, RejectsIncompatibleManagedPointeeIdentity)
  {
    RuntimeCallContext Test;
    const auto Function = Test.function("readInteger", Test.Int32, {Test.Int32Pointer});
    NativeSymbolCache Symbols([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&readRuntimeInteger);
    });
    const auto Cell = Test.Memory.allocateCell(*Test.Types.get(Test.Float32), true, RuntimeValue::fromBits(std::bit_cast<std::uint32_t>(1.0F), Test.Float32));
    ASSERT_TRUE(Cell);
    const RuntimeValue Arguments[] = {RuntimeValue::fromPointer(ExecutionPointer::fromPlace(Cell.Place), Test.Int32Pointer)};
    EXPECT_EQ(Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments).Status, ExecutionStatus::TypeMismatch);
  }

  // Equal numeric function IDs from independent type tables never reuse each other's native binding.
  TEST(RuntimeFfiTest, SeparatesFunctionIdsFromIndependentImages)
  {
    RuntimeCallContext First;
    RuntimeCallContext Second;
    const auto FirstFunction = First.function("first", First.Int32, {First.Int32});
    const auto SecondFunction = Second.function("second", Second.Int32, {Second.Int32});
    ASSERT_EQ(FirstFunction.Id, SecondFunction.Id);
    NativeSymbolCache Symbols([](std::string_view Symbol) -> NativeSymbol
    {
      return Symbol == "first" ? reinterpret_cast<NativeSymbol>(&incrementRuntimeInteger) : reinterpret_cast<NativeSymbol>(&incrementRuntimeIntegerTwice);
    });
    const RuntimeValue FirstArguments[] = {RuntimeValue::fromBits(10, First.Int32)};
    const RuntimeValue SecondArguments[] = {RuntimeValue::fromBits(10, Second.Int32)};
    const auto FirstResult = First.Calls.invoke(First.Memory, First.Types, Symbols, FirstFunction, FirstArguments);
    const auto SecondResult = First.Calls.invoke(Second.Memory, Second.Types, Symbols, SecondFunction, SecondArguments);
    ASSERT_TRUE(FirstResult);
    ASSERT_TRUE(SecondResult);
    EXPECT_EQ(FirstResult.Value.Bits, 11U);
    EXPECT_EQ(SecondResult.Value.Bits, 12U);
    EXPECT_EQ(First.Calls.size(), 2U);
  }

  // Reusing a function ID with a changed signature cannot invoke a cached incompatible native interface.
  TEST(RuntimeFfiTest, RejectsChangedSignatureForCachedFunctionId)
  {
    RuntimeCallContext Test;
    const auto Function = Test.function("increment", Test.Int32, {Test.Int32});
    NativeSymbolCache Symbols([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&incrementRuntimeInteger);
    });
    const RuntimeValue Arguments[] = {RuntimeValue::fromBits(10, Test.Int32)};
    ASSERT_TRUE(Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments));
    auto Changed = Test.function("increment", Test.Int32, {});
    Changed.Id = Function.Id;
    EXPECT_EQ(Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Changed, {}).Status, ExecutionStatus::TypeMismatch);
  }

  // Local type ID collisions cannot let a foreign image's floating cell masquerade as an integer in the same memory manager.
  TEST(RuntimeFfiTest, RejectsManagedCellsFromAnotherTypeDomain)
  {
    RuntimeCallContext Test;
    RuntimeTypeTable ForeignTypes;
    TypeDesc ForeignFloat;
    ForeignFloat.setKind(RuntimeKind::Float);
    ForeignFloat.setBitWidth(32);
    ForeignFloat.Size = sizeof(float);
    ForeignFloat.Alignment = alignof(float);
    ForeignFloat.Native = true;
    const RuntimeTypeId ForeignType = ForeignTypes.append(ForeignFloat);
    ASSERT_EQ(ForeignType, Test.Int32);
    const auto Cell = Test.Memory.allocateCell(*ForeignTypes.get(ForeignType), true, RuntimeValue::fromBits(std::bit_cast<std::uint32_t>(1.0F), ForeignType));
    ASSERT_TRUE(Cell);
    const auto Function = Test.function("readInteger", Test.Int32, {Test.Int32Pointer});
    NativeSymbolCache Symbols([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&readRuntimeInteger);
    });
    const RuntimeValue Arguments[] = {RuntimeValue::fromPointer(ExecutionPointer::fromPlace(Cell.Place), Test.Int32Pointer)};
    EXPECT_EQ(Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments).Status, ExecutionStatus::ForeignContext);
  }

  // String payloads must also carry a string type ID before native pointer adaptation, including on cache hits.
  TEST(RuntimeFfiTest, RejectsStringPayloadWithNonStringTypeIdentity)
  {
    RuntimeCallContext Test;
    const auto Function = Test.function("mutateString", Test.BytePointer, {Test.BytePointer});
    NativeSymbolCache Symbols([](std::string_view) -> NativeSymbol
    {
      return reinterpret_cast<NativeSymbol>(&mutateRuntimeString);
    });
    RuntimeValue Arguments[] = {RuntimeValue::fromString("ABC", Test.String)};
    const auto Result = Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments);
    ASSERT_TRUE(Result);
    ASSERT_EQ(Test.Memory.release(Test.Memory.storageFromAddress(Result.Value.pointer().address())), ExecutionStatus::Success);
    Arguments[0] = RuntimeValue::fromString("ABC", Test.Int32);
    EXPECT_EQ(Test.Calls.invoke(Test.Memory, Test.Types, Symbols, Function, Arguments).Status, ExecutionStatus::TypeMismatch);
    EXPECT_EQ(Test.Memory.liveStorageCount(), 0U);
  }

  // A new table reconstructed at the same address receives a fresh cache domain and cannot inherit a previous native binding.
  TEST(RuntimeFfiTest, DoesNotReusePlansWhenTypeTableAddressIsRecycled)
  {
    ExecutionMemoryManager Memory;
    NativeCallCache Calls;
    std::optional<RuntimeTypeTable> Types(std::in_place);
    auto Describe = [&](std::string_view Symbol)
    {
      TypeDesc Integer;
      Integer.setKind(RuntimeKind::Integer);
      Integer.setBitWidth(32);
      Integer.editInteger().Signed = true;
      const RuntimeTypeId Int32 = Types->append(Integer);
      TypeDesc Signature;
      Signature.setKind(RuntimeKind::Function);
      Signature.editFunction().ReturnType = Int32;
      Signature.editFunction().Parameters = {Int32};
      RuntimeFunctionDescriptor Function;
      Function.Id = 0;
      Function.Signature = Types->append(Signature);
      Function.Symbol = Symbol;
      Function.NativeAbi = true;
      Function.Supported = true;
      return Function;
    };
    NativeSymbolCache Symbols([](std::string_view Symbol) -> NativeSymbol
    {
      return Symbol == "first" ? reinterpret_cast<NativeSymbol>(&incrementRuntimeInteger) : reinterpret_cast<NativeSymbol>(&incrementRuntimeIntegerTwice);
    });
    const auto First = Describe("first");
    const RuntimeValue Arguments[] = {RuntimeValue::fromBits(10, 0)};
    const auto FirstResult = Calls.invoke(Memory, *Types, Symbols, First, Arguments);
    ASSERT_TRUE(FirstResult);
    EXPECT_EQ(FirstResult.Value.Bits, 11U);
    const std::weak_ptr<const RuntimeTypeDomain> OldDomain = Types->domain();
    const auto *OriginalAddress = &*Types;
    Types.reset();
    EXPECT_FALSE(OldDomain.expired());
    Types.emplace();
    ASSERT_EQ(&*Types, OriginalAddress);
    const auto Second = Describe("second");
    const auto SecondResult = Calls.invoke(Memory, *Types, Symbols, Second, Arguments);
    ASSERT_TRUE(SecondResult);
    EXPECT_EQ(SecondResult.Value.Bits, 12U);
    EXPECT_EQ(Calls.size(), 2U);
    Calls.clear();
    EXPECT_TRUE(OldDomain.expired());
  }
} // namespace ink::execution::test
