#pragma once

#include "ink/backend/vm/instruction/address_operand.h"
#include "ink/backend/vm/instruction/immediate_operand.h"
#include "ink/backend/vm/instruction/register_operand.h"

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>

namespace ink::vm
{
  enum class InstructionForm : std::uint8_t
  {
#define OPCODE_FORM(Kind, ...) Kind,
#define OPCODE(...)
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
#undef OPCODE_FORM
  };

  namespace detail
  {
    template <typename... ValueTypes>
    struct InstructionOperandLayout
    {
        using OperandTuple = std::tuple<ValueTypes...>;

        static constexpr std::size_t OperandCount = sizeof...(ValueTypes);
        static constexpr std::size_t RegisterCount = (std::size_t{0} + ... + static_cast<std::size_t>(std::is_same_v<ValueTypes, RegisterOperand>));
        static constexpr std::size_t AddressCount = (std::size_t{0} + ... + static_cast<std::size_t>(std::is_same_v<ValueTypes, AddressOperand>));
        static constexpr std::size_t ImmediateCount = (std::size_t{0} + ... + static_cast<std::size_t>(std::is_same_v<ValueTypes, ImmediateOperand>));
    };
  } // namespace detail

  // Forms describe ordered operand kinds independently of operation and width; kind counts alone do not determine this layout.
  template <InstructionForm FormValue>
  struct FormTraits;

#define OPCODE_FORM(Kind, ...)                                                             \
  template <>                                                                              \
  struct FormTraits<InstructionForm::Kind> : detail::InstructionOperandLayout<__VA_ARGS__> \
  {                                                                                        \
  };
#define OPCODE(...)
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
#undef OPCODE_FORM
} // namespace ink::vm
