#pragma once

#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"

#include <cstddef>
#include <optional>
#include <span>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ink::vm::detail
{
  struct SymbolId
  {
      std::size_t Index;

      bool operator==(const SymbolId &) const = default;
  };

  using SymbolTarget = std::variant<const MachineFunction *, const MachineGlobalVariable *, MachineSymbolDeclaration>;
  using SymbolTable = std::unordered_map<core::Name, SymbolId>;

  // A canonical definition or host import. Index tables contain IDs, never copies of these facts.
  struct SymbolRecord
  {
      SymbolTarget Target;
      core::Name OutputName;
      std::size_t ModuleIndex;

      // Native imports have no VM definition linkage.
      std::optional<MachineSymbolLinkage> linkage() const;
      MachineSymbolKind kind() const;
      bool isConstant() const;
  };

  class SymbolResolver;

  // Resolved names for one ordered set of inputs. Definition targets borrow those inputs until cloning finishes.
  // Only the resolver can construct or change bindings; consumers see canonical symbols and output names.
  class LinkBindings final
  {
    public:
      core::Name outputName(std::size_t ModuleIndex, core::Name SourceName) const;

      std::span<const SymbolRecord> symbols() const noexcept
      {
        return Symbols;
      }

    private:
      friend class SymbolResolver;

      LinkBindings() = default;

      std::vector<SymbolRecord> Symbols;
      std::vector<SymbolTable> Scopes;
  };

  // Binds all original names before assigning Local output names, preserving unresolved-reference diagnostics.
  std::optional<LinkBindings> resolveLinkSymbols(std::span<const MachineModule *const> Inputs, const core::DiagnosticEngine &Diagnostics);
} // namespace ink::vm::detail
