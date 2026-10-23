#ifndef INK_IR_MODULE_MODULE_MANAGER_H
#define INK_IR_MODULE_MODULE_MANAGER_H

#include "ink/ir/module/module_error.h"
#include "ink/ir/module/module_id.h"
#include "ink/ir/module/symbol_export.h"
#include "ink/ir/module/symbol_ref.h"

#include <cstddef>
#include <expected>
#include <memory>
#include <unordered_map>
#include <utility>

namespace ink::ir
{
  class IRContext;
  class Module;

  // Owns named modules in one borrowed IRContext, which must outlive this manager.
  // Module addresses and IDs remain stable until manager destruction; unloading is not supported.
  // All users of the modules and their declarations must finish before manager destruction.
  // Operations require external synchronization and never load files, compile sources or execute IR.
  class ModuleManager final
  {
    public:
      explicit ModuleManager(IRContext &Context) noexcept;
      ~ModuleManager() noexcept;

      ModuleManager(const ModuleManager &) = delete;
      ModuleManager &operator=(const ModuleManager &) = delete;
      ModuleManager(ModuleManager &&) = delete;
      ModuleManager &operator=(ModuleManager &&) = delete;

      auto &irContext(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Context);
      }

      // IDs must contain at least one valid name. Canonical spelling is the producer's responsibility.
      // Duplicate IDs are rejected without changing existing modules.
      std::expected<Module *, ModuleError> createModule(ModuleID ID);
      // Transfers ownership only on success; rejection preserves ModuleValue and its construction-time ID.
      // A matching target configuration does not permit adopting a different IRContext's module.
      std::expected<Module *, ModuleError> adoptModule(std::unique_ptr<Module> &ModuleValue);

      // Missing or invalid IDs return nullptr, without registering or loading anything.
      Module *findModule(const ModuleID &ID) noexcept;
      const Module *findModule(const ModuleID &ID) const noexcept;

      std::size_t moduleCount() const noexcept;

      // Finds the entire export group for a name in the loaded module, including all overload candidates.
      // Registering a new export name may invalidate the returned record pointer; query again after mutation.
      // Overload selection, import binding and native execution addresses are not involved.
      std::expected<const SymbolExport *, ModuleError> resolveSymbol(const SymbolRef &Reference) const noexcept;

    private:
      struct ModuleIDHash
      {
          std::size_t operator()(const ModuleID &ID) const noexcept;
      };

      IRContext &Context;
      // Owns every mapped module and releases it during manager destruction.
      std::unordered_map<ModuleID, Module *, ModuleIDHash> Modules;
  };
} // namespace ink::ir

#endif
