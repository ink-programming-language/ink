#include "ink/cli/io.h"

#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <memory>
#include <ostream>
#include <string>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <fcntl.h>
#include <io.h>
#endif

namespace ink::cli
{
  namespace
  {
    bool isUtf8Continuation(unsigned char Byte) noexcept
    {
      return Byte >= 0x80 && Byte <= 0xBF;
    }
  } // namespace

  bool isValidUtf8(std::string_view Text) noexcept
  {
    std::size_t Offset = 0;
    while (Offset < Text.size())
    {
      const unsigned char First = static_cast<unsigned char>(Text[Offset]);
      if (First <= 0x7F)
      {
        ++Offset;
        continue;
      }

      std::size_t Length = 0;
      if (First >= 0xC2 && First <= 0xDF)
      {
        Length = 2;
      }
      else if (First >= 0xE0 && First <= 0xEF)
      {
        Length = 3;
      }
      else if (First >= 0xF0 && First <= 0xF4)
      {
        Length = 4;
      }
      else
      {
        return false;
      }

      if (Offset + Length > Text.size())
      {
        return false;
      }
      for (std::size_t Index = 1; Index < Length; ++Index)
      {
        if (!isUtf8Continuation(static_cast<unsigned char>(Text[Offset + Index])))
        {
          return false;
        }
      }

      const unsigned char Second = static_cast<unsigned char>(Text[Offset + 1]);
      if ((First == 0xE0 && Second < 0xA0) || (First == 0xED && Second > 0x9F) || (First == 0xF0 && Second < 0x90) || (First == 0xF4 && Second > 0x8F))
      {
        return false;
      }
      Offset += Length;
    }
    return true;
  }

  bool pathFromUtf8(std::string_view Path, std::filesystem::path &Result) noexcept
  {
    if (!isValidUtf8(Path))
    {
      return false;
    }
    Result = std::filesystem::u8path(Path.begin(), Path.end());
    return true;
  }

  EnvironmentVariableStatus readEnvironmentVariable(std::string_view Name, std::string &Value) noexcept
  {
    Value.clear();
    if (Name.empty() || !isValidUtf8(Name))
    {
      return EnvironmentVariableStatus::Invalid;
    }
#ifdef _WIN32
    if (Name.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
      return EnvironmentVariableStatus::Invalid;
    }
    const int WideNameLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, Name.data(), static_cast<int>(Name.size()), nullptr, 0);
    if (WideNameLength <= 0)
    {
      return EnvironmentVariableStatus::Invalid;
    }
    std::wstring WideName(static_cast<std::size_t>(WideNameLength), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, Name.data(), static_cast<int>(Name.size()), WideName.data(), WideNameLength) != WideNameLength)
    {
      return EnvironmentVariableStatus::Invalid;
    }
    SetLastError(ERROR_SUCCESS);
    const DWORD RequiredLength = GetEnvironmentVariableW(WideName.c_str(), nullptr, 0);
    if (RequiredLength == 0)
    {
      return GetLastError() == ERROR_ENVVAR_NOT_FOUND ? EnvironmentVariableStatus::NotFound : EnvironmentVariableStatus::Invalid;
    }
    std::wstring WideValue(static_cast<std::size_t>(RequiredLength), L'\0');
    const DWORD WrittenLength = GetEnvironmentVariableW(WideName.c_str(), WideValue.data(), RequiredLength);
    if (WrittenLength >= RequiredLength)
    {
      return EnvironmentVariableStatus::Invalid;
    }
    WideValue.resize(static_cast<std::size_t>(WrittenLength));
    if (WideValue.empty())
    {
      return EnvironmentVariableStatus::Found;
    }
    if (WideValue.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
    {
      return EnvironmentVariableStatus::Invalid;
    }
    const int Utf8Length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, WideValue.data(), static_cast<int>(WideValue.size()), nullptr, 0, nullptr, nullptr);
    if (Utf8Length <= 0)
    {
      return EnvironmentVariableStatus::Invalid;
    }
    Value.resize(static_cast<std::size_t>(Utf8Length));
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, WideValue.data(), static_cast<int>(WideValue.size()), Value.data(), Utf8Length, nullptr, nullptr) != Utf8Length)
    {
      Value.clear();
      return EnvironmentVariableStatus::Invalid;
    }
    return EnvironmentVariableStatus::Found;
#else
    const char *EnvironmentValue = std::getenv(std::string(Name).c_str());
    if (EnvironmentValue == nullptr)
    {
      return EnvironmentVariableStatus::NotFound;
    }
    if (!isValidUtf8(EnvironmentValue))
    {
      return EnvironmentVariableStatus::Invalid;
    }
    Value = EnvironmentValue;
    return EnvironmentVariableStatus::Found;
#endif
  }

  bool useBinaryStandardInput() noexcept
  {
#ifdef _WIN32
    return _setmode(_fileno(stdin), _O_BINARY) != -1;
#else
    return true;
#endif
  }

  bool writeOutput(std::ostream &Output, std::string_view Message)
  {
    const auto Sink = std::make_shared<spdlog::sinks::ostream_sink_st>(Output, true);
    spdlog::logger Logger("ink-output", Sink);
    bool Failed = false;
    Logger.set_error_handler([&Failed](const std::string &)
                             {
                               Failed = true;
                             });
    Logger.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
    Logger.log(spdlog::level::info, spdlog::string_view_t(Message.data(), Message.size()));
    return !Failed && static_cast<bool>(Output);
  }
} // namespace ink::cli
