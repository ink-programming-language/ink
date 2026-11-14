#pragma once

#include "ink/backend/vm/instruction/eflags.h"
#include "ink/backend/vm/instruction/register.h"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <type_traits>

// Id must be a validated RegisterId; each expression argument is evaluated exactly once.
#define GET_REGISTER(Id, FrameValue) (static_cast<const void *>((FrameValue).Register[static_cast<::std::size_t>(Id)]))
#define SET_REGISTER(Id, FrameValue) (const_cast<void *>(static_cast<::ink::vm::Frame &>(FrameValue).Register[static_cast<::std::size_t>(Id)]))

namespace ink::vm
{
  // Native stubs receive the caller's validated call area without a VM BP/PC save area.
  struct Frame
  {
      // Native stubs leave SP unchanged; BP and PC are borrowed host addresses.
      std::uint8_t *SP = nullptr;
      std::uint8_t *BP = nullptr;
      std::uint8_t *PC = nullptr;
      vm::EFlags EFlags = 0;
      inline static constexpr std::uint64_t ZERO = 0;

      // Address values in register-number order: BP, SP, PC, &EFlags, &ZERO.
      std::array<const void *, static_cast<std::size_t>(RegisterId::ZERO) + 1> Register{};

      Frame() noexcept
      {
        initializeRegisters();
      }

      explicit Frame(std::uint8_t *StackPointer) noexcept
          : SP(StackPointer)
      {
        initializeRegisters();
      }

      Frame(const Frame &Other) noexcept
          : SP(Other.SP),
            BP(Other.BP),
            PC(Other.PC),
            EFlags(Other.EFlags)
      {
        initializeRegisters();
      }

      Frame &operator=(const Frame &Other) noexcept
      {
        SP = Other.SP;
        BP = Other.BP;
        PC = Other.PC;
        EFlags = Other.EFlags;
        initializeRegisters();
        return *this;
      }

      // Refresh after changing BP, SP, or PC directly; scalar slots always address this frame's members.
      void initializeRegisters() noexcept
      {
        Register = {
            BP,
            SP,
            PC,
            &EFlags,
            &ZERO,
        };
      }

      // ArgumentTypes must list every parameter in declaration order, using types whose host and VM representations match.
      template <std::size_t Index, typename... ArgumentTypes>
      [[nodiscard]] auto getArgument() const noexcept
      {
        static_assert(Index < sizeof...(ArgumentTypes), "Frame argument index must be within the complete parameter list");
        using ValueType = std::tuple_element_t<Index, std::tuple<ArgumentTypes...>>;
        constexpr std::size_t ByteOffset = getArgumentByteOffset<Index, ArgumentTypes...>();
        std::array<std::byte, sizeof(ValueType)> Bytes;
        std::memcpy(Bytes.data(), SP - ByteOffset, sizeof(ValueType));
        return std::bit_cast<ValueType>(Bytes);
      }

      // Copy only the value bytes, preserving slot padding; void-returning stubs do not call this method.
      template <typename ReturnType, typename... ArgumentTypes>
      void setReturnValue(const ReturnType &Value) const noexcept
      {
        constexpr std::size_t ByteOffset = getSlotByteSize<ReturnType>() + (getSlotByteSize<ArgumentTypes>() + ... + std::size_t{0});
        std::memcpy(SP - ByteOffset, &Value, sizeof(ReturnType));
      }

    private:
      template <typename ValueType>
      static constexpr std::size_t getSlotByteSize() noexcept
      {
        static_assert(std::is_trivially_copyable_v<ValueType> && !std::is_array_v<ValueType> && !std::is_volatile_v<ValueType>, "Frame values must be trivially copyable, non-volatile values rather than raw arrays");
        static_assert(alignof(ValueType) <= 8, "Frame values with alignment greater than eight bytes need a separate VM ABI mapping");
        return (sizeof(ValueType) + 7) / 8 * 8;
      }

      template <std::size_t Index, typename... ArgumentTypes>
      static constexpr std::size_t getArgumentByteOffset() noexcept
      {
        constexpr std::array<std::size_t, sizeof...(ArgumentTypes)> SlotBytes{getSlotByteSize<ArgumentTypes>()...};
        std::size_t ByteOffset = 0;
        for (std::size_t ArgumentIndex = Index; ArgumentIndex < SlotBytes.size(); ++ArgumentIndex)
        {
          ByteOffset += SlotBytes[ArgumentIndex];
        }
        return ByteOffset;
      }
  };
} // namespace ink::vm
