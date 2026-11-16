#pragma once

#include "ink/backend/vm/instruction/register.h"
#include "ink/core/support/core_define.h"

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <type_traits>

// Id must be a validated RegisterId; each expression argument is evaluated exactly once.
#define GET_REGISTER(Id, FrameValue) (static_cast<const ::std::uint8_t*>((FrameValue).Register[static_cast<::std::size_t>(Id)]))
#define SET_REGISTER(Id, FrameValue) (const_cast<::std::uint8_t*>(static_cast<const ::std::uint8_t*>(static_cast<::ink::vm::Frame&>(FrameValue).Register[static_cast<::std::size_t>(Id)])))

// Typed value access evaluates every argument once; writing requires a mutable Frame.
#define READ_REGISTER(Id, FrameValue)                                                                               \
    ([](::ink::vm::RegisterId InkRegisterId, const ::ink::vm::Frame& InkFrame) noexcept -> ::std::uint64_t          \
     {                                                                                                              \
         check(::ink::vm::isAddressRegister(InkRegisterId));                                                        \
         return *static_cast<const ::std::uint64_t*>(InkFrame.Register[static_cast<::std::size_t>(InkRegisterId)]); \
     }((Id), (FrameValue)))

#define WRITE_REGISTER(Id, FrameValue, Value)                                                                                                                 \
    ([](::ink::vm::RegisterId InkRegisterId, ::ink::vm::Frame& InkFrame, ::std::uint64_t InkRegisterValue) noexcept                                           \
     {                                                                                                                                                        \
         check(::ink::vm::isAddressRegister(InkRegisterId) && InkRegisterId != ::ink::vm::RegisterId::ZERO);                                                  \
         *const_cast<::std::uint64_t*>(static_cast<const ::std::uint64_t*>(InkFrame.Register[static_cast<::std::size_t>(InkRegisterId)])) = InkRegisterValue; \
     }((Id), (FrameValue), (Value)))

namespace ink::vm
{
    // Native stubs receive the caller's validated call area without a VM BP/PC save area.
    struct Frame
    {
        public:
            static_assert(sizeof(std::uintptr_t) == sizeof(std::uint64_t), "VM address registers require a 64-bit host");

            // Register storage addresses in number order: &BP, &SP, &PC, &ZERO.
            // Every slot points to a uint64_t object; keep the bindings unchanged.
            std::array<const void*, static_cast<std::size_t>(RegisterId::ZERO) + 1> Register{};

            Frame() noexcept
            {
                initializeRegisters();
            }

            explicit Frame(std::uint8_t* StackPointer) noexcept
                : SP(reinterpret_cast<std::uintptr_t>(StackPointer))
            {
                initializeRegisters();
            }

            Frame(const Frame& Other) noexcept
                : SP(Other.SP),
                  BP(Other.BP),
                  PC(Other.PC)
            {
                initializeRegisters();
            }

            Frame& operator=(const Frame& Other) noexcept
            {
                SP = Other.SP;
                BP = Other.BP;
                PC = Other.PC;
                initializeRegisters();
                return *this;
            }

            // Access the same live storage exposed by Register[PC], without caching its value.
            [[nodiscard]] FORCE_INLINE std::uint64_t getPC() const noexcept
            {
                return PC;
            }

            FORCE_INLINE void setPC(std::uint64_t Value) noexcept
            {
                PC = Value;
            }

            // ArgumentTypes must list every parameter in declaration order, using types whose host and VM representations match.
            template <std::size_t Index, typename... ArgumentTypes>
            [[nodiscard]] auto getArgument() const noexcept
            {
                static_assert(Index < sizeof...(ArgumentTypes), "Frame argument index must be within the complete parameter list");
                using ValueType = std::tuple_element_t<Index, std::tuple<ArgumentTypes...>>;
                constexpr std::size_t ByteOffset = getArgumentByteOffset<Index, ArgumentTypes...>();
                const auto* StackPointer = reinterpret_cast<const std::uint8_t*>(static_cast<std::uintptr_t>(SP));
                std::array<std::byte, sizeof(ValueType)> Bytes;
                std::memcpy(Bytes.data(), StackPointer - ByteOffset, sizeof(ValueType));
                return std::bit_cast<ValueType>(Bytes);
            }

            // Copy only the value bytes, preserving slot padding; void-returning stubs do not call this method.
            template <typename ReturnType, typename... ArgumentTypes>
            void setReturnValue(const ReturnType& Value) const noexcept
            {
                constexpr std::size_t ByteOffset = getSlotByteSize<ReturnType>() + (getSlotByteSize<ArgumentTypes>() + ... + std::size_t{0});
                auto* StackPointer = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(SP));
                std::memcpy(StackPointer - ByteOffset, &Value, sizeof(ReturnType));
            }

        private:
            // Register binds to these values, so external writes also update native-call state.
            std::uint64_t SP = 0;
            std::uint64_t BP = 0;
            std::uint64_t PC = 0;
            inline static constexpr std::uint64_t ZERO = 0;

            void initializeRegisters() noexcept
            {
                Register = {
                    &BP,
                    &SP,
                    &PC,
                    &ZERO,
                };
            }

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
