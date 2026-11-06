#pragma once

#include <cstdint>

namespace ink::ir
{
  enum class ABIKind : std::uint8_t
  {
    Ink,
    C,
  };

  // These values describe the requested contract; target lowering must validate support before use.
  enum class CallingConvention : std::uint8_t
  {
    Default,
    CDecl,
    StdCall,
    FastCall,
    VectorCall,
  };

  // Default selects the target default for ABI; explicit conventions retain their own type identity.
  struct CallABI
  {
      ABIKind ABI = ABIKind::Ink;
      CallingConvention Convention = CallingConvention::Default;

      constexpr bool operator==(const CallABI &Other) const noexcept = default;
  };
} // namespace ink::ir
