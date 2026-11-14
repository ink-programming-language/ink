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

namespace ink::vm
{
  // Native stubs receive the caller's validated call area without a VM BP/PC save area.
  struct Frame
  {
      Frame() noexcept = default;

      explicit Frame(std::uint8_t *StackPointer) noexcept
      {
        setSP(StackPointer);
      }

      // Index directly with RegisterId; the read-only view prevents writes to the ZERO slot.
      const auto &getRegisters() const noexcept
      {
        return Register;
      }

      std::uint8_t *getBP() const noexcept
      {
        return reinterpret_cast<std::uint8_t *>(static_cast<std::uintptr_t>(Register[static_cast<std::size_t>(RegisterId::BP)]));
      }

      void setBP(std::uint8_t *Value) noexcept
      {
        Register[static_cast<std::size_t>(RegisterId::BP)] = reinterpret_cast<std::uintptr_t>(Value);
      }

      // The first free byte after the return and argument slots; native stubs leave it unchanged.
      std::uint8_t *getSP() const noexcept
      {
        return reinterpret_cast<std::uint8_t *>(static_cast<std::uintptr_t>(Register[static_cast<std::size_t>(RegisterId::SP)]));
      }

      void setSP(std::uint8_t *Value) noexcept
      {
        Register[static_cast<std::size_t>(RegisterId::SP)] = reinterpret_cast<std::uintptr_t>(Value);
      }

      std::uint8_t *getPC() const noexcept
      {
        return reinterpret_cast<std::uint8_t *>(static_cast<std::uintptr_t>(Register[static_cast<std::size_t>(RegisterId::PC)]));
      }

      void setPC(std::uint8_t *Value) noexcept
      {
        Register[static_cast<std::size_t>(RegisterId::PC)] = reinterpret_cast<std::uintptr_t>(Value);
      }

      vm::EFlags getEFlags() const noexcept
      {
        return static_cast<vm::EFlags>(Register[static_cast<std::size_t>(RegisterId::EFLAGS)]);
      }

      void setEFlags(vm::EFlags Value) noexcept
      {
        Register[static_cast<std::size_t>(RegisterId::EFLAGS)] = Value;
      }

      // ArgumentTypes must list every parameter in declaration order, using types whose host and VM representations match.
      template <std::size_t Index, typename... ArgumentTypes>
      [[nodiscard]] auto getArgument() const noexcept
      {
        static_assert(Index < sizeof...(ArgumentTypes), "Frame argument index must be within the complete parameter list");
        using ValueType = std::tuple_element_t<Index, std::tuple<ArgumentTypes...>>;
        constexpr std::size_t ByteOffset = getArgumentByteOffset<Index, ArgumentTypes...>();
        std::array<std::byte, sizeof(ValueType)> Bytes;
        std::memcpy(Bytes.data(), getSP() - ByteOffset, sizeof(ValueType));
        return std::bit_cast<ValueType>(Bytes);
      }

      // Copy only the value bytes, preserving slot padding; void-returning stubs do not call this method.
      template <typename ReturnType, typename... ArgumentTypes>
      void setReturnValue(const ReturnType &Value) const noexcept
      {
        constexpr std::size_t ByteOffset = getSlotByteSize<ReturnType>() + (getSlotByteSize<ArgumentTypes>() + ... + std::size_t{0});
        std::memcpy(getSP() - ByteOffset, &Value, sizeof(ReturnType));
      }

    private:
      // Sole storage in register-number order: BP, SP, PC, EFLAGS, ZERO. ZERO has no write accessor.
      std::array<std::uint64_t, static_cast<std::size_t>(RegisterId::ZERO) + 1> Register{};

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
