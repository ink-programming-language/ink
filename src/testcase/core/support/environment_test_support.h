#ifndef INK_TESTCASE_CORE_ENVIRONMENT_TEST_SUPPORT_H
#define INK_TESTCASE_CORE_ENVIRONMENT_TEST_SUPPORT_H

#include <gtest/gtest.h>

#include <cstdlib>
#include <optional>
#include <string>

namespace ink::core::test
{
  class ScopedEnvironmentVariable
  {
    public:
      explicit ScopedEnvironmentVariable(const char *Name)
          : Name(Name)
      {
        if (const char *Value = std::getenv(Name))
        {
          PreviousValue = Value;
        }
      }

      ~ScopedEnvironmentVariable()
      {
        EXPECT_TRUE(set(PreviousValue ? PreviousValue->c_str() : nullptr));
      }

      ScopedEnvironmentVariable(const ScopedEnvironmentVariable &) = delete;
      ScopedEnvironmentVariable &operator=(const ScopedEnvironmentVariable &) = delete;

      bool set(const char *Value) const
      {
#ifdef _WIN32
        return _putenv_s(Name.c_str(), Value != nullptr ? Value : "") == 0;
#else
        return (Value != nullptr ? setenv(Name.c_str(), Value, 1) : unsetenv(Name.c_str())) == 0;
#endif
      }

    private:
      std::string Name;
      std::optional<std::string> PreviousValue;
  };
} // namespace ink::core::test

#endif
