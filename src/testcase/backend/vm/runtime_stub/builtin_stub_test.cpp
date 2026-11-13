#include "ink/backend/vm/runtime_stub/builtin_stub.h"
#include "ink/backend/vm/vm_runtime/frame.h"
#include "ink/backend/vm/vm_runtime/runtime.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace ink::vm::test
{
  namespace
  {
    constexpr std::size_t ReturnSlotOffset = 8;
    constexpr std::size_t ArgumentSlotOffset = ReturnSlotOffset + sizeof(std::int64_t);
    constexpr std::size_t CallEndOffset = ArgumentSlotOffset + sizeof(std::int64_t);
    using StackBytes = std::array<std::uint8_t, CallEndOffset + 8>;
  }

  // Identity reads the i64 argument below SP and changes only its return slot for signed boundaries and distinct byte patterns.
  TEST(VMBuiltinStubTest, IdentityPreservesStackOutsideReturnSlotForSignedIntegerValues)
  {
    constexpr std::int64_t Values[] = {
        0,
        1,
        -1,
        42,
        -42,
        std::numeric_limits<std::int64_t>::min(),
        std::numeric_limits<std::int64_t>::max(),
        0x0123456789ABCDEF,
    };

    for (const auto Value : Values)
    {
      SCOPED_TRACE(Value);
      alignas(8) StackBytes Stack;
      Stack.fill(0xA5);
      std::memcpy(Stack.data() + ArgumentSlotOffset, &Value, sizeof(Value));
      auto Expected = Stack;
      std::memcpy(Expected.data() + ReturnSlotOffset, &Value, sizeof(Value));
      Frame CallFrame{Stack.data() + CallEndOffset};
      auto *const InitialSP = CallFrame.SP;

      identityStub(&CallFrame);

      EXPECT_EQ(CallFrame.SP, InitialSP);
      EXPECT_EQ(Stack, Expected);
    }
  }

  // The native identity name resolves to the Frame adapter and dispatches it without changing SP, arguments, or surrounding bytes.
  TEST(VMBuiltinStubTest, RegistersAndInvokesIdentityAdapterByNativeSymbolName)
  {
    const auto IdentityName = core::NamePool::instance().intern("identity");
    const NativeSymbolAddress *IdentitySymbol = nullptr;
    for (const auto &Symbol : NativeSymbolAddresses)
    {
      if (Symbol.Name == IdentityName)
      {
        IdentitySymbol = &Symbol;
        break;
      }
    }
    ASSERT_NE(IdentitySymbol, nullptr);
    ASSERT_EQ(IdentitySymbol->Address, reinterpret_cast<void *>(&identityStub));
    const auto Stub = reinterpret_cast<decltype(&identityStub)>(IdentitySymbol->Address);

    constexpr std::int64_t Value = -0x0123456789ABCDEF;
    alignas(8) StackBytes Stack;
    Stack.fill(0x5A);
    std::memcpy(Stack.data() + ArgumentSlotOffset, &Value, sizeof(Value));
    auto Expected = Stack;
    std::memcpy(Expected.data() + ReturnSlotOffset, &Value, sizeof(Value));
    Frame CallFrame{Stack.data() + CallEndOffset};
    auto *const InitialSP = CallFrame.SP;

    Stub(&CallFrame);

    EXPECT_EQ(CallFrame.SP, InitialSP);
    EXPECT_EQ(Stack, Expected);
  }
} // namespace ink::vm::test
