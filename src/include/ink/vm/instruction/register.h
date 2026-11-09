#pragma once

#include <cstdint>
#include <string_view>

namespace ink::vm
{
  enum class RegisterId : std::uint8_t
  {
#define REGISTER(Kind, Encoding, Spelling) Kind = Encoding,
#include "ink/vm/instruction/register.def"
#undef REGISTER
  };

  constexpr std::string_view getRegisterName(RegisterId Register) noexcept
  {
    switch (Register)
    {
#define REGISTER(Kind, Encoding, Spelling) \
  case RegisterId::Kind:                   \
    return Spelling;
#include "ink/vm/instruction/register.def"
#undef REGISTER
    }
    return "InvalidRegister";
  }
} // namespace ink::vm
