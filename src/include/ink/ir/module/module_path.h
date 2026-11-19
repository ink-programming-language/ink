#pragma once

#include "ink/core/name/name.h"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <span>
#include <string_view>
#include <vector>

namespace ink::ir
{
    // Normalized absolute module path: pkg.sub.mod -> {pkg, sub, mod}.
    // Relative imports and local aliases are resolved by the frontend before constructing IR paths.
    class ModulePath final
    {
        public:
            ModulePath() noexcept = default;
            explicit ModulePath(std::span<const core::Name> Components);
            ModulePath(std::initializer_list<core::Name> Components);

            // Splits a dot-separated path into interned components; malformed segments return an invalid path.
            [[nodiscard]] static ModulePath parse(std::string_view QualifiedText);

            std::span<const core::Name> components() const noexcept
            {
                return Components;
            }

            std::size_t componentCount() const noexcept
            {
                return Components.size();
            }

            bool empty() const noexcept
            {
                return Components.empty();
            }

            bool valid() const noexcept
            {
                return QualifiedName.valid();
            }

            core::Name qualifiedName() const noexcept
            {
                return QualifiedName;
            }

            // Formats "<qualifiedName>::<SymbolName>" into the NamePool, or returns NAME_None if either side is invalid.
            [[nodiscard]] core::Name qualifiedSymbolName(core::Name SymbolName) const;

            std::size_t hash() const noexcept;

            friend bool operator==(const ModulePath& Left, const ModulePath& Right) noexcept
            {
                return Left.QualifiedName == Right.QualifiedName && Left.Components == Right.Components;
            }

        private:
            std::vector<core::Name> Components = {};
            core::Name QualifiedName = core::NAME_None;
    };
} // namespace ink::ir

namespace std
{
    template <>
    struct hash<ink::ir::ModulePath>
    {
            std::size_t operator()(const ink::ir::ModulePath& Value) const noexcept
            {
                return Value.hash();
            }
    };
} // namespace std
