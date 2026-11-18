#pragma once

#include "ink/core/name/name.h"

#include <cstdint>

namespace ink::vm
{
    enum class ExecutableSymbolKind : std::uint8_t
    {
        Function,
        Data,
    };

    enum class CallingConvention : std::uint8_t
    {
        Default,
        CDecl,
        StdCall,
        FastCall,
        VectorCall,
    };

    struct DataSymbolInfo
    {
            bool IsConstant = false;

            constexpr bool operator==(const DataSymbolInfo& Other) const noexcept = default;
    };

    struct FunctionSymbolInfo
    {
            CallingConvention Convention = CallingConvention::Default;
            bool IsVariadic = false;

            constexpr bool operator==(const FunctionSymbolInfo& Other) const noexcept = default;
    };

    enum class RelocationKind : std::uint8_t
    {
        AbsoluteAddress,
        DirectCall,
    };

    // Identifies an unresolved imported symbol, its function or data classification, and kind-specific contract metadata.
    // Name is its unique lookup identity; bound runtime addresses are outside this record.
    struct DynamicImportSymbol
    {
            core::Name Name = core::NAME_None;
            ExecutableSymbolKind Kind = ExecutableSymbolKind::Function;
            DataSymbolInfo Data{};
            FunctionSymbolInfo Function{};

            constexpr bool operator==(const DynamicImportSymbol& Other) const noexcept = default;
    };

    // ImageOffset identifies the first byte of an eight-byte imported symbol address field in the image.
    // SymbolIndex indexes ExecutableFile's DynamicImportSymbols; Addend is a signed byte displacement.
    // DirectCall relocations also rewrite the preceding eight-byte call instruction header to Callvmi when bound to a VM function.
    struct DynamicImportSymbolRelocation
    {
            static constexpr std::uint8_t ByteSize = 8;

            std::uint64_t ImageOffset = 0;
            std::uint32_t SymbolIndex = 0;
            std::int64_t Addend = 0;
            RelocationKind Kind = RelocationKind::AbsoluteAddress;

            constexpr bool operator==(const DynamicImportSymbolRelocation& Other) const noexcept = default;
    };

    // ImageOffset identifies the first byte of an eight-byte address field in the image.
    // TargetOffset is relative to the beginning of Image and may identify its one-past end.
    // DirectCall relocations also rewrite the preceding eight-byte call instruction header to Callvmi during loading.
    struct ImageBaseAddressRelocation
    {
            static constexpr std::uint8_t ByteSize = 8;

            std::uint64_t ImageOffset = 0;
            std::uint64_t TargetOffset = 0;
            std::int64_t Addend = 0;
            RelocationKind Kind = RelocationKind::AbsoluteAddress;

            constexpr bool operator==(const ImageBaseAddressRelocation& Other) const noexcept = default;
    };

    // Names an exported image symbol and its function or data metadata. Offset is relative to the beginning of Image, never a host pointer.
    struct ExportSymbol
    {
            core::Name Name = core::NAME_None;
            std::uint64_t Offset = 0;
            ExecutableSymbolKind Kind = ExecutableSymbolKind::Function;
            DataSymbolInfo Data{};
            FunctionSymbolInfo Function{};

            constexpr bool operator==(const ExportSymbol& Other) const noexcept = default;
    };
} // namespace ink::vm
