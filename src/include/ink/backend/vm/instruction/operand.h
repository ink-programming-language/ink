#pragma once

#include <cstdint>

namespace ink::vm
{
  // Operand kinds identify the C++ model; instruction opcodes determine operand kinds on the wire.
  enum class OperandKind : std::uint8_t
  {
    Register,
    Address,
    Immediate,
  };

  class Operand
  {
    public:
      constexpr OperandKind getKind() const noexcept
      {
        return Kind;
      }

    protected:
      constexpr explicit Operand(OperandKind Kind) noexcept
          : Kind(Kind)
      {
      }

      // Concrete operands are copyable values; the base cannot be sliced or assigned through a base reference.
      constexpr Operand(const Operand &) noexcept = default;
      constexpr Operand &operator=(const Operand &) noexcept = default;

      // Operands are destroyed through their concrete class, never through Operand.
      ~Operand() = default;

    private:
      OperandKind Kind;
  };
} // namespace ink::vm
