#include "ink/core/config.h"

#if INK_PLATFORM_WINDOWS

#include "ink/backend/vm/runtime_stub/windows/kernel32_stub.h"
#include "ink/backend/vm/vm_runtime/runtime.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace ink::vm::test
{
  namespace
  {
    class ScopedHandle
    {
      public:
        explicit ScopedHandle(HANDLE Value) noexcept
            : Value(Value)
        {
        }

        ~ScopedHandle()
        {
          if (Value != nullptr && Value != INVALID_HANDLE_VALUE)
          {
            EXPECT_NE(::CloseHandle(Value), FALSE);
          }
        }

        ScopedHandle(const ScopedHandle &) = delete;
        ScopedHandle &operator=(const ScopedHandle &) = delete;

        HANDLE get() const noexcept
        {
          return Value;
        }

      private:
        HANDLE Value;
    };

    class ScopedTemporaryFilePath
    {
      public:
        ~ScopedTemporaryFilePath()
        {
          if (Created)
          {
            EXPECT_NE(::DeleteFileW(Path.data()), FALSE);
          }
        }

        bool create(const wchar_t *Directory) noexcept
        {
          Created = ::GetTempFileNameW(Directory, L"ink", 0, Path.data()) != 0;
          return Created;
        }

        const wchar_t *get() const noexcept
        {
          return Path.data();
        }

      private:
        std::array<wchar_t, MAX_PATH> Path{};
        bool Created = false;
    };

    struct WriteResult
    {
        BOOL Value;
        DWORD Error;
    };

    WriteResult invokeWriteFile(HANDLE File, LPCVOID Buffer, DWORD BytesToWrite, LPDWORD BytesWritten, LPOVERLAPPED Overlapped)
    {
      alignas(8) std::array<std::uint8_t, 64> Stack;
      Stack.fill(0xA5);
      // The call area is return [8,16), followed by five eight-byte argument slots ending at 56.
      std::memcpy(Stack.data() + 16, &File, sizeof(File));
      std::memcpy(Stack.data() + 24, &Buffer, sizeof(Buffer));
      std::memcpy(Stack.data() + 32, &BytesToWrite, sizeof(BytesToWrite));
      std::memcpy(Stack.data() + 40, &BytesWritten, sizeof(BytesWritten));
      std::memcpy(Stack.data() + 48, &Overlapped, sizeof(Overlapped));
      auto Expected = Stack;
      Frame CallFrame{Stack.data() + 56};
      auto *const InitialSP = CallFrame.getSP();

      ::SetLastError(ERROR_SUCCESS);
      writeFileStub(&CallFrame);
      const DWORD Error = ::GetLastError();

      BOOL Value = FALSE;
      std::memcpy(&Value, Stack.data() + 8, sizeof(Value));
      std::memcpy(Expected.data() + 8, &Value, sizeof(Value));
      EXPECT_EQ(CallFrame.getSP(), InitialSP);
      EXPECT_EQ(Stack, Expected);
      return {Value, Error};
    }
  }

  // All standard selectors preserve native handle values, while an invalid selector preserves the native error and every non-return stack byte.
  TEST(VMWindowsStubTest, GetStdHandleMatchesNativeSelectorsAndPreservesCallArea)
  {
    constexpr DWORD Selectors[] = {
        STD_INPUT_HANDLE,
        STD_OUTPUT_HANDLE,
        STD_ERROR_HANDLE,
        0,
    };

    for (const DWORD Selector : Selectors)
    {
      SCOPED_TRACE(Selector);
      ::SetLastError(ERROR_SUCCESS);
      const HANDLE ExpectedHandle = ::GetStdHandle(Selector);
      const DWORD ExpectedError = ::GetLastError();
      alignas(8) std::array<std::uint8_t, 32> Stack;
      Stack.fill(0x5A);
      std::memcpy(Stack.data() + 16, &Selector, sizeof(Selector));
      auto ExpectedStack = Stack;
      std::memcpy(ExpectedStack.data() + 8, &ExpectedHandle, sizeof(ExpectedHandle));
      Frame CallFrame{Stack.data() + 24};
      auto *const InitialSP = CallFrame.getSP();

      ::SetLastError(ERROR_SUCCESS);
      getStdHandleStub(&CallFrame);
      const DWORD ActualError = ::GetLastError();

      EXPECT_EQ(CallFrame.getSP(), InitialSP);
      EXPECT_EQ(Stack, ExpectedStack);
      if (Selector == 0)
      {
        EXPECT_EQ(ExpectedHandle, INVALID_HANDLE_VALUE);
        EXPECT_EQ(ActualError, ExpectedError);
      }
    }
  }

  // A synchronous pipe write preserves binary bytes including NUL and 0xFF, reports the byte count, and changes only the BOOL return value in its call area.
  TEST(VMWindowsStubTest, WriteFileForwardsBinaryPipePayloadAndByteCount)
  {
    HANDLE ReadHandle = nullptr;
    HANDLE WriteHandle = nullptr;
    const BOOL Created = ::CreatePipe(&ReadHandle, &WriteHandle, nullptr, 0);
    const ScopedHandle Reader(ReadHandle);
    const ScopedHandle Writer(WriteHandle);
    ASSERT_NE(Created, FALSE);
    constexpr std::array<std::uint8_t, 5> Payload = {
        0x41,
        0x00,
        0xFF,
        0x0D,
        0x0A,
    };
    DWORD BytesWritten = 0;

    const auto Result = invokeWriteFile(Writer.get(), Payload.data(), static_cast<DWORD>(Payload.size()), &BytesWritten, nullptr);

    ASSERT_NE(Result.Value, FALSE);
    ASSERT_EQ(BytesWritten, static_cast<DWORD>(Payload.size()));
    std::array<std::uint8_t, Payload.size()> Actual{};
    DWORD BytesRead = 0;
    ASSERT_NE(::ReadFile(Reader.get(), Actual.data(), static_cast<DWORD>(Actual.size()), &BytesRead, nullptr), FALSE);
    EXPECT_EQ(BytesRead, static_cast<DWORD>(Payload.size()));
    EXPECT_EQ(Actual, Payload);
  }

  // An invalid file handle returns FALSE, preserves ERROR_INVALID_HANDLE, and resets the output count without corrupting its call area.
  TEST(VMWindowsStubTest, WriteFilePreservesInvalidHandleFailureAndResetsByteCount)
  {
    const std::uint8_t Byte = 0x41;
    DWORD BytesWritten = 0x12345678;

    const auto Result = invokeWriteFile(INVALID_HANDLE_VALUE, &Byte, 1, &BytesWritten, nullptr);

    EXPECT_EQ(Result.Value, FALSE);
    EXPECT_EQ(Result.Error, static_cast<DWORD>(ERROR_INVALID_HANDLE));
    EXPECT_EQ(BytesWritten, 0U);
  }

  // A non-null OVERLAPPED parameter forwards its offset even for a synchronous file handle and writes the payload at that exact position.
  TEST(VMWindowsStubTest, WriteFileForwardsOverlappedOffsetForSynchronousFile)
  {
    std::array<wchar_t, MAX_PATH> Directory{};
    const DWORD DirectoryLength = ::GetTempPathW(static_cast<DWORD>(Directory.size()), Directory.data());
    ASSERT_GT(DirectoryLength, 0U);
    ASSERT_LT(DirectoryLength, static_cast<DWORD>(Directory.size()));
    ScopedTemporaryFilePath Path;
    ASSERT_TRUE(Path.create(Directory.data()));
    const ScopedHandle File(::CreateFileW(Path.get(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_TEMPORARY, nullptr));
    ASSERT_NE(File.get(), INVALID_HANDLE_VALUE);
    constexpr std::array<std::uint8_t, 3> Payload = {
        0x42,
        0x00,
        0xFE,
    };
    OVERLAPPED Overlapped{};
    Overlapped.Offset = 7;
    DWORD BytesWritten = 0;

    const auto Result = invokeWriteFile(File.get(), Payload.data(), static_cast<DWORD>(Payload.size()), &BytesWritten, &Overlapped);

    ASSERT_NE(Result.Value, FALSE);
    EXPECT_EQ(BytesWritten, static_cast<DWORD>(Payload.size()));
    LARGE_INTEGER FileSize{};
    ASSERT_NE(::GetFileSizeEx(File.get(), &FileSize), FALSE);
    EXPECT_EQ(FileSize.QuadPart, 7 + static_cast<LONGLONG>(Payload.size()));
    LARGE_INTEGER Position{};
    Position.QuadPart = 7;
    ASSERT_NE(::SetFilePointerEx(File.get(), Position, nullptr, FILE_BEGIN), FALSE);
    std::array<std::uint8_t, Payload.size()> Actual{};
    DWORD BytesRead = 0;
    ASSERT_NE(::ReadFile(File.get(), Actual.data(), static_cast<DWORD>(Actual.size()), &BytesRead, nullptr), FALSE);
    EXPECT_EQ(BytesRead, static_cast<DWORD>(Payload.size()));
    EXPECT_EQ(Actual, Payload);
  }

  // Exact Windows import names appear once in the native registry and resolve to the matching Frame adapter addresses.
  TEST(VMWindowsStubTest, RegistersWindowsImportsAsFrameAdapters)
  {
    const NativeSymbolAddress ExpectedSymbols[] = {
        {core::NamePool::instance().intern("GetStdHandle"), reinterpret_cast<void *>(&getStdHandleStub)},
        {core::NamePool::instance().intern("WriteFile"), reinterpret_cast<void *>(&writeFileStub)},
    };
    for (const auto &Expected : ExpectedSymbols)
    {
      const NativeSymbolAddress *Actual = nullptr;
      for (const auto &Symbol : NativeSymbolAddresses)
      {
        if (Symbol.Name == Expected.Name)
        {
          ASSERT_EQ(Actual, nullptr);
          Actual = &Symbol;
        }
      }
      ASSERT_NE(Actual, nullptr);
      EXPECT_EQ(Actual->Address, Expected.Address);
    }
  }
} // namespace ink::vm::test

#endif
