#pragma once

#include "ink/core/name/name.h"
#include "ink/backend/vm/module/machine_symbol.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <utility>
#include <vector>

namespace ink::vm
{
    class MachineModule;

    enum class MachineModuleDataError : std::uint8_t
    {
        InvalidAlignment,
        InitializerTooLarge,
        SizeOverflow,
        InvalidRelocationSymbol,
        InvalidRelocationAlignment,
        RelocationOutOfBounds,
        RelocationOverlap,
        RelocationNonZeroInitialBytes,
    };

    // Describes an 8-byte absolute symbol address field embedded within a MachineGlobalVariable.
    struct MachineGlobalRelocation
    {
            static constexpr std::uint64_t ByteSize = 8;

            std::uint64_t ByteOffset = 0;
            core::Name Symbol = core::NAME_None;
            std::int64_t Addend = 0;

            constexpr bool operator==(const MachineGlobalRelocation& Other) const noexcept = default;
    };

    // MachineModule-owned global storage; writable and read-only data both zero-fill an uninitialized suffix when loaded.
    class MachineGlobalVariable final
    {
        public:
            explicit MachineGlobalVariable(MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local, bool IsConstant = false) noexcept
                : Linkage(Linkage),
                  IsConstant(IsConstant)
            {
            }

            MachineGlobalVariable(const MachineGlobalVariable&) = delete;
            MachineGlobalVariable& operator=(const MachineGlobalVariable&) = delete;
            MachineGlobalVariable(MachineGlobalVariable&&) = delete;
            MachineGlobalVariable& operator=(MachineGlobalVariable&&) = delete;

            core::Name name() const noexcept
            {
                return Name;
            }

            MachineSymbolLinkage linkage() const noexcept
            {
                return Linkage;
            }

            void setLinkage(MachineSymbolLinkage Value) noexcept
            {
                Linkage = Value;
            }

            bool isConstant() const noexcept
            {
                return IsConstant;
            }

            auto* parent(this auto&& Self) noexcept
            {
                return Self.Parent ? &std::forward_like<decltype(Self)&>(*Self.Parent) : nullptr;
            }

            std::uint64_t alignment() const noexcept
            {
                return Alignment;
            }

            std::uint64_t byteSize() const noexcept
            {
                return ByteSize;
            }

            std::span<const std::uint8_t> initialBytes() const noexcept
            {
                return InitialBytes;
            }

            std::span<const MachineGlobalRelocation> relocations() const noexcept
            {
                return Relocations;
            }

            std::size_t relocationCount() const noexcept
            {
                return Relocations.size();
            }

            [[nodiscard]] std::expected<void, MachineModuleDataError> addRelocation(MachineGlobalRelocation Relocation);
            [[nodiscard]] std::expected<void, MachineModuleDataError> addRelocation(std::uint64_t ByteOffset, core::Name Symbol, std::int64_t Addend = 0);

        private:
            friend class MachineModule;

            core::Name Name = core::NAME_None;
            MachineSymbolLinkage Linkage;
            bool IsConstant;
            MachineModule* Parent = nullptr;
            std::uint64_t ByteSize = 0;
            std::uint64_t Alignment = 1;
            std::vector<std::uint8_t> InitialBytes;
            std::vector<MachineGlobalRelocation> Relocations;
    };
} // namespace ink::vm
