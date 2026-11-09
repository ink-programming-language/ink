#pragma once

#include "ink/backend/vm/instruction/machine_instruction.h"
#include "ink/backend/vm/instruction/instruction_form.h"

#include <cstddef>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <utility>

namespace ink::vm
{
  template <Opcode OperationValue>
  struct OpcodeTraits;

#define OPCODE(Kind, Encoding, Spelling, FormValue, WidthValue, RegisterCountValue, AddressCountValue, ImmediateCountValue)             \
  template <>                                                                                                                           \
  struct OpcodeTraits<Opcode::Kind> : FormTraits<InstructionForm::FormValue>                                                            \
  {                                                                                                                                     \
      static constexpr InstructionForm Form = InstructionForm::FormValue;                                                               \
      static constexpr std::uint32_t BitWidth = WidthValue;                                                                             \
      static_assert(RegisterCount == RegisterCountValue && AddressCount == AddressCountValue && ImmediateCount == ImmediateCountValue); \
  };
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE

  // Owns typed operand values inline. The opcode fixes both the form and the constructor's exact operand types.
  template <Opcode OperationValue>
  class MachineInstructionImpl final : public MachineInstruction
  {
    public:
      using Traits = OpcodeTraits<OperationValue>;
      using OperandTuple = typename Traits::OperandTuple;

      static constexpr Opcode Operation = OperationValue;

      template <typename... ValueTypes>
      constexpr explicit MachineInstructionImpl(ValueTypes &&...Values) noexcept requires(std::is_same_v<std::tuple<std::remove_cvref_t<ValueTypes>...>, OperandTuple> && std::is_nothrow_constructible_v<OperandTuple, ValueTypes && ...>)
          : MachineInstruction(OperationValue),
            Operands(std::forward<ValueTypes>(Values)...)
      {
      }

      static constexpr bool classof(const MachineInstruction *Value) noexcept
      {
        return Value && Value->getOpcode() == OperationValue;
      }

      constexpr const auto &getDestination() const noexcept
      {
        return std::get<0>(Operands);
      }

      constexpr const auto &getSource() const noexcept requires(Traits::OperandCount == 2)
      {
        return std::get<1>(Operands);
      }

      constexpr const auto &getLHS() const noexcept requires(Traits::OperandCount == 3)
      {
        return std::get<1>(Operands);
      }

      constexpr const auto &getRHS() const noexcept requires(Traits::OperandCount == 3)
      {
        return std::get<2>(Operands);
      }

      constexpr const Operand *getOperand(std::size_t Index) const noexcept
      {
        return getOperandImpl<0>(Index);
      }

    private:
      template <std::size_t OperandIndex>
      constexpr const Operand *getOperandImpl(std::size_t Index) const noexcept
      {
        if constexpr (OperandIndex == Traits::OperandCount)
        {
          return nullptr;
        }
        else
        {
          if (Index == OperandIndex)
          {
            return &std::get<OperandIndex>(Operands);
          }
          return getOperandImpl<OperandIndex + 1>(Index);
        }
      }

      OperandTuple Operands;
  };

#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount) using Machine##Kind##Instruction = MachineInstructionImpl<Opcode::Kind>;
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
} // namespace ink::vm
