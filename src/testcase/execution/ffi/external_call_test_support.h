#ifndef INK_TESTCASE_EXECUTION_EXTERNAL_CALL_TEST_SUPPORT_H
#define INK_TESTCASE_EXECUTION_EXTERNAL_CALL_TEST_SUPPORT_H

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(__linux__)
namespace ink::execution::test
{
#ifdef _WIN32
  constexpr std::string_view WriteSymbol = "_write";
  constexpr std::uint32_t WriteCountWidth = 32;
  constexpr std::uint32_t WriteReturnWidth = 32;
#else
  constexpr std::string_view WriteSymbol = "write";
  constexpr std::uint32_t WriteCountWidth = sizeof(std::size_t) * 8;
  constexpr std::uint32_t WriteReturnWidth = sizeof(ssize_t) * 8;
#endif

  // Tests write only to this private pipe; closing the writer makes reads finite.
  class NativePipe final
  {
    public:
      NativePipe() noexcept
      {
#ifdef _WIN32
        const int Result = _pipe(Descriptors, 4096, _O_BINARY);
#else
        const int Result = pipe(Descriptors);
#endif
        if (Result != 0)
        {
          Descriptors[0] = -1;
          Descriptors[1] = -1;
        }
      }

      ~NativePipe() noexcept
      {
        closeDescriptor(Descriptors[0]);
        closeDescriptor(Descriptors[1]);
      }

      NativePipe(const NativePipe &) = delete;
      NativePipe &operator=(const NativePipe &) = delete;
      NativePipe(NativePipe &&) = delete;
      NativePipe &operator=(NativePipe &&) = delete;

      bool valid() const noexcept
      {
        return Descriptors[0] >= 0 && Descriptors[1] >= 0;
      }

      int writer() const noexcept
      {
        return Descriptors[1];
      }

      bool readAll(std::string &Output) noexcept
      {
        closeDescriptor(Descriptors[1]);
        Output.clear();
        char Buffer[256];
        while (true)
        {
#ifdef _WIN32
          const auto Received = _read(Descriptors[0], Buffer, sizeof(Buffer));
#else
          const auto Received = read(Descriptors[0], Buffer, sizeof(Buffer));
#endif
          if (Received < 0)
          {
            if (errno == EINTR)
            {
              continue;
            }
            return false;
          }
          if (Received == 0)
          {
            return true;
          }
          Output.append(Buffer, static_cast<std::size_t>(Received));
        }
      }

    private:
      static void closeDescriptor(int &Descriptor) noexcept
      {
        if (Descriptor < 0)
        {
          return;
        }
#ifdef _WIN32
        _close(Descriptor);
#else
        close(Descriptor);
#endif
        Descriptor = -1;
      }

      int Descriptors[2] = {-1, -1};
  };

  inline std::string writeDeclaration(bool VoidBuffer = false)
  {
    return "import \"C\" func " + std::string(WriteSymbol) + "(Fd: i32, Buffer: " + std::string(VoidBuffer ? "*void" : "*u8") + ", Count: u" + std::to_string(WriteCountWidth) + "): i" + std::to_string(WriteReturnWidth) + ";\n";
  }

  inline std::string writeReturnType()
  {
    return "i" + std::to_string(WriteReturnWidth);
  }
} // namespace ink::execution::test
#endif

#endif
