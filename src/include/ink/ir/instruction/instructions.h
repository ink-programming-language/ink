#pragma once

#include "ink/ir/instruction/address_operand.h"
#include "ink/ir/instruction/immediate_operand.h"
#include "ink/ir/instruction/instruction.h"

#include <cstdint>
#include <optional>

namespace ink::ir::detail
{
  // ByteCount is the allocation size in bytes; BitWidth does not describe that size.
  template <Opcode OperationCode, std::uint32_t Width>
  class AllocaInstruction final : public Instruction
  {
    public:
      static constexpr Opcode Code = OperationCode;
      static constexpr std::uint32_t BitWidth = Width;

      constexpr AllocaInstruction(AddressOperand Result, ImmediateOperand ByteCount) noexcept
          : Instruction(Code, Result),
            ByteCount(ByteCount)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getOpcode() == Code;
      }

      constexpr const ImmediateOperand &getByteCount() const noexcept
      {
        return ByteCount;
      }

    private:
      ImmediateOperand ByteCount;
  };

  template <Opcode OperationCode, std::uint32_t Width>
  class LoadInstruction final : public Instruction
  {
    public:
      static constexpr Opcode Code = OperationCode;
      static constexpr std::uint32_t BitWidth = Width;

      constexpr LoadInstruction(AddressOperand Result, AddressOperand Source) noexcept
          : Instruction(Code, Result),
            Source(Source)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getOpcode() == Code;
      }

      constexpr const AddressOperand &getSource() const noexcept
      {
        return Source;
      }

    private:
      AddressOperand Source;
  };

  // Stores have a memory destination but do not produce an instruction result.
  template <Opcode OperationCode, std::uint32_t Width>
  class StoremInstruction final : public Instruction
  {
    public:
      static constexpr Opcode Code = OperationCode;
      static constexpr std::uint32_t BitWidth = Width;

      constexpr StoremInstruction(AddressOperand Destination, AddressOperand Source) noexcept
          : Instruction(Code, std::nullopt),
            Destination(Destination),
            Source(Source)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getOpcode() == Code;
      }

      constexpr const AddressOperand &getDestination() const noexcept
      {
        return Destination;
      }

      constexpr const AddressOperand &getSource() const noexcept
      {
        return Source;
      }

    private:
      AddressOperand Destination;
      AddressOperand Source;
  };

  template <Opcode OperationCode, std::uint32_t Width>
  class StoreiInstruction final : public Instruction
  {
    public:
      static constexpr Opcode Code = OperationCode;
      static constexpr std::uint32_t BitWidth = Width;

      constexpr StoreiInstruction(AddressOperand Destination, ImmediateOperand Source) noexcept
          : Instruction(Code, std::nullopt),
            Destination(Destination),
            Source(Source)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getOpcode() == Code;
      }

      constexpr const AddressOperand &getDestination() const noexcept
      {
        return Destination;
      }

      constexpr const ImmediateOperand &getSource() const noexcept
      {
        return Source;
      }

    private:
      AddressOperand Destination;
      ImmediateOperand Source;
  };

  template <Opcode OperationCode, std::uint32_t Width>
  class AddmmInstruction final : public Instruction
  {
    public:
      static constexpr Opcode Code = OperationCode;
      static constexpr std::uint32_t BitWidth = Width;

      constexpr AddmmInstruction(AddressOperand Result, AddressOperand LHS, AddressOperand RHS) noexcept
          : Instruction(Code, Result),
            LHS(LHS),
            RHS(RHS)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getOpcode() == Code;
      }

      constexpr const AddressOperand &getLHS() const noexcept
      {
        return LHS;
      }

      constexpr const AddressOperand &getRHS() const noexcept
      {
        return RHS;
      }

    private:
      AddressOperand LHS;
      AddressOperand RHS;
  };

  template <Opcode OperationCode, std::uint32_t Width>
  class AddmiInstruction final : public Instruction
  {
    public:
      static constexpr Opcode Code = OperationCode;
      static constexpr std::uint32_t BitWidth = Width;

      constexpr AddmiInstruction(AddressOperand Result, AddressOperand LHS, ImmediateOperand RHS) noexcept
          : Instruction(Code, Result),
            LHS(LHS),
            RHS(RHS)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getOpcode() == Code;
      }

      constexpr const AddressOperand &getLHS() const noexcept
      {
        return LHS;
      }

      constexpr const ImmediateOperand &getRHS() const noexcept
      {
        return RHS;
      }

    private:
      AddressOperand LHS;
      ImmediateOperand RHS;
  };

  template <Opcode OperationCode, std::uint32_t Width>
  class AddiiInstruction final : public Instruction
  {
    public:
      static constexpr Opcode Code = OperationCode;
      static constexpr std::uint32_t BitWidth = Width;

      constexpr AddiiInstruction(AddressOperand Result, ImmediateOperand LHS, ImmediateOperand RHS) noexcept
          : Instruction(Code, Result),
            LHS(LHS),
            RHS(RHS)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getOpcode() == Code;
      }

      constexpr const ImmediateOperand &getLHS() const noexcept
      {
        return LHS;
      }

      constexpr const ImmediateOperand &getRHS() const noexcept
      {
        return RHS;
      }

    private:
      ImmediateOperand LHS;
      ImmediateOperand RHS;
  };
} // namespace ink::ir::detail

namespace ink::ir
{
#define OPCODE(Kind, Encoding, Spelling, Form, Width, AddressCount, ImmediateCount) using Kind##Instruction = detail::Form##Instruction<Opcode::Kind, Width>;
#include "ink/ir/instruction/opcode.def"
#undef OPCODE
} // namespace ink::ir
