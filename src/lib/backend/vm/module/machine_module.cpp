#include "ink/backend/vm/module/machine_module.h"

#include "ink/core/name/name_pool.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <span>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::vm
{
    namespace
    {
        constexpr bool isValidAlignment(std::uint64_t Alignment) noexcept
        {
            return Alignment != 0 && (Alignment & (Alignment - 1)) == 0;
        }

        constexpr bool relocationsOverlap(std::uint64_t FirstOffset, std::uint64_t SecondOffset) noexcept
        {
            return FirstOffset < SecondOffset + MachineGlobalRelocation::ByteSize && SecondOffset < FirstOffset + MachineGlobalRelocation::ByteSize;
        }

        std::expected<void, MachineModuleDataError> validateRelocation(const MachineGlobalRelocation& Relocation, std::uint64_t ByteSize, std::uint64_t Alignment, std::span<const std::uint8_t> InitialBytes)
        {
            if (!Relocation.Symbol.valid() || !core::NamePool::instance().contains(Relocation.Symbol))
            {
                return std::unexpected(MachineModuleDataError::InvalidRelocationSymbol);
            }
            if (Alignment < MachineGlobalRelocation::ByteSize || Relocation.ByteOffset % MachineGlobalRelocation::ByteSize != 0)
            {
                return std::unexpected(MachineModuleDataError::InvalidRelocationAlignment);
            }
            if (Relocation.ByteOffset > ByteSize || MachineGlobalRelocation::ByteSize > ByteSize - Relocation.ByteOffset)
            {
                return std::unexpected(MachineModuleDataError::RelocationOutOfBounds);
            }
            if (std::cmp_less(Relocation.ByteOffset, InitialBytes.size()))
            {
                const auto Start = static_cast<std::size_t>(Relocation.ByteOffset);
                const std::size_t OverlapCount = std::min<std::size_t>(InitialBytes.size() - Start, static_cast<std::size_t>(MachineGlobalRelocation::ByteSize));
                for (std::size_t Offset = 0; Offset < OverlapCount; ++Offset)
                {
                    if (InitialBytes[Start + Offset] != 0)
                    {
                        return std::unexpected(MachineModuleDataError::RelocationNonZeroInitialBytes);
                    }
                }
            }
            return {};
        }
    } // namespace

    std::expected<void, MachineModuleDataError> MachineGlobalVariable::addRelocation(MachineGlobalRelocation Relocation)
    {
        if (const auto Valid = validateRelocation(Relocation, ByteSize, Alignment, InitialBytes); !Valid)
        {
            return Valid;
        }
        for (const MachineGlobalRelocation& Existing : Relocations)
        {
            if (relocationsOverlap(Existing.ByteOffset, Relocation.ByteOffset))
            {
                return std::unexpected(MachineModuleDataError::RelocationOverlap);
            }
        }
        if (Relocations.size() == Relocations.max_size())
        {
            return std::unexpected(MachineModuleDataError::SizeOverflow);
        }
        Relocations.push_back(Relocation);
        return {};
    }

    std::expected<void, MachineModuleDataError> MachineGlobalVariable::addRelocation(std::uint64_t ByteOffset, core::Name Symbol, std::int64_t Addend)
    {
        return addRelocation(MachineGlobalRelocation{ByteOffset, Symbol, Addend});
    }

    MachineModule::MachineModule(core::Name Name) noexcept
        : Name(Name)
    {
    }

    MachineFunction& MachineModule::createFunction(core::Name FunctionName, MachineSymbolLinkage Linkage)
    {
        MachineFunction& Created = Functions.emplace_back(FunctionName, Linkage);
        Created.Parent = this;
        return Created;
    }

    std::expected<std::reference_wrapper<MachineGlobalVariable>, MachineModuleDataError> MachineModule::createGlobalVariable(std::uint64_t ByteSize, std::uint64_t Alignment, std::span<const std::uint8_t> InitialBytes, core::Name GlobalName, MachineSymbolLinkage Linkage, bool IsConstant, std::span<const MachineGlobalRelocation> Relocations)
    {
        if (!isValidAlignment(Alignment))
        {
            return std::unexpected(MachineModuleDataError::InvalidAlignment);
        }
        if (std::cmp_greater(InitialBytes.size(), ByteSize))
        {
            return std::unexpected(MachineModuleDataError::InitializerTooLarge);
        }
        std::vector<std::uint8_t> OwnedBytes;
        std::vector<MachineGlobalRelocation> OwnedRelocations;
        if (InitialBytes.size() > OwnedBytes.max_size() || Relocations.size() > OwnedRelocations.max_size())
        {
            return std::unexpected(MachineModuleDataError::SizeOverflow);
        }
        std::unordered_set<std::uint64_t> SeenRelocationOffsets;
        SeenRelocationOffsets.reserve(Relocations.size());
        for (const MachineGlobalRelocation& Relocation : Relocations)
        {
            if (const auto Valid = validateRelocation(Relocation, ByteSize, Alignment, InitialBytes); !Valid)
            {
                return std::unexpected(Valid.error());
            }
            if (!SeenRelocationOffsets.insert(Relocation.ByteOffset).second)
            {
                return std::unexpected(MachineModuleDataError::RelocationOverlap);
            }
        }
        if (!InitialBytes.empty())
        {
            OwnedBytes.assign(InitialBytes.begin(), InitialBytes.end());
        }
        if (!Relocations.empty())
        {
            OwnedRelocations.assign(Relocations.begin(), Relocations.end());
        }
        MachineGlobalVariable& Created = Globals.emplace_back(Linkage, IsConstant);
        Created.Name = GlobalName;
        Created.Parent = this;
        Created.ByteSize = ByteSize;
        Created.Alignment = Alignment;
        Created.InitialBytes = std::move(OwnedBytes);
        Created.Relocations = std::move(OwnedRelocations);
        return std::ref(Created);
    }

    void MachineModule::declareSymbol(core::Name SymbolName, MachineSymbolKind Kind, bool IsConstant)
    {
        SymbolDeclarations.push_back({SymbolName, Kind, IsConstant});
    }
} // namespace ink::vm
