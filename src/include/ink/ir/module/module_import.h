#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/module/module_path.h"

#include <utility>

namespace ink::ir
{
    class Module;

    // Models one external module dependency owned by a referencing Module with a stable address.
    class ModuleImport final
    {
        public:
            explicit ModuleImport(ModulePath Path) noexcept
                : Path(std::move(Path))
            {
            }

            ModuleImport(const ModuleImport&) = delete;
            ModuleImport& operator=(const ModuleImport&) = delete;
            ModuleImport(ModuleImport&&) = delete;
            ModuleImport& operator=(ModuleImport&&) = delete;

            const ModulePath& path() const noexcept
            {
                return Path;
            }

            core::Name qualifiedName() const noexcept
            {
                return Path.qualifiedName();
            }

            [[nodiscard]] core::Name qualifiedSymbolName(core::Name SymbolName) const
            {
                return Path.qualifiedSymbolName(SymbolName);
            }

            auto* parent(this auto&& Self) noexcept
            {
                return Self.Parent ? &std::forward_like<decltype(Self)&>(*Self.Parent) : nullptr;
            }

        private:
            friend class Module;

            ModulePath Path = {};
            Module* Parent = nullptr;
    };
} // namespace ink::ir
