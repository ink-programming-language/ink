#include "link_bindings.h"

#include "function_references.h"
#include "link_diagnostic.h"
#include "ink/core/name/name_pool.h"

#include <spdlog/fmt/fmt.h>

#include <cassert>
#include <expected>
#include <string>
#include <unordered_set>
#include <utility>

namespace ink::vm::detail
{
  std::optional<MachineSymbolLinkage> SymbolRecord::linkage() const
  {
    if (const auto *Function = std::get_if<const MachineFunction *>(&Target))
    {
      return (*Function)->linkage();
    }
    if (const auto *Global = std::get_if<const MachineGlobalVariable *>(&Target))
    {
      return (*Global)->linkage();
    }
    return std::nullopt;
  }

  MachineSymbolKind SymbolRecord::kind() const
  {
    if (const auto *Declaration = std::get_if<MachineSymbolDeclaration>(&Target))
    {
      return Declaration->Kind;
    }
    return std::holds_alternative<const MachineFunction *>(Target) ? MachineSymbolKind::Function : MachineSymbolKind::GlobalVariable;
  }

  bool SymbolRecord::isConstant() const
  {
    if (const auto *Declaration = std::get_if<MachineSymbolDeclaration>(&Target))
    {
      return Declaration->IsConstant;
    }
    const auto *Global = std::get_if<const MachineGlobalVariable *>(&Target);
    return Global && (*Global)->isConstant();
  }

  core::Name LinkBindings::outputName(std::size_t ModuleIndex, core::Name SourceName) const
  {
    if (SourceName == core::NAME_None)
    {
      return SourceName;
    }
    const auto Position = Scopes[ModuleIndex].find(SourceName);
    assert(Position != Scopes[ModuleIndex].end() && "link bindings must resolve every name before cloning");
    return Symbols[Position->second.Index].OutputName;
  }

  namespace
  {
    bool validName(core::Name Name, bool AllowAnonymous = false)
    {
      return (AllowAnonymous && Name == core::NAME_None) || core::NamePool::instance().contains(Name);
    }

    std::string_view spelling(core::Name Name)
    {
      return core::NamePool::instance().text(Name);
    }

    std::optional<SymbolId> lookup(const SymbolTable &Table, core::Name Name)
    {
      const auto Position = Table.find(Name);
      return Position == Table.end() ? std::nullopt : std::optional{Position->second};
    }
  } // namespace

  class SymbolResolver final
  {
    public:
      SymbolResolver(std::span<const MachineModule *const> Inputs, const core::DiagnosticEngine &Diagnostics)
          : Inputs(Inputs),
            Diagnostics(Diagnostics)
      {
      }

      std::optional<LinkBindings> resolve()
      {
        if (!checkInputs() || !collectDefinitions() || !bindDeclarations() || !bindReferences() || !assignOutputNames())
        {
          return std::nullopt;
        }
        return std::move(Bindings);
      }

    private:
      bool fail(std::size_t Index, std::string_view Description) const
      {
        return reportLinkFailure(Diagnostics, Index, Inputs[Index], Description);
      }

      SymbolId addSymbol(std::size_t Index, core::Name Name, SymbolTarget Target)
      {
        const SymbolId Id{Bindings.Symbols.size()};
        Bindings.Symbols.push_back({std::move(Target), Name, Index});
        return Id;
      }

      bool checkInputs()
      {
        std::unordered_set<const MachineModule *> Seen;
        for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
        {
          const MachineModule *Input = Inputs[Index];
          if (!Input || !Seen.insert(Input).second)
          {
            return reportLinkFailure(Diagnostics, Index, nullptr, Input ? "module was supplied more than once" : "module pointer is null");
          }
          if (!validName(Input->name(), true))
          {
            return fail(Index, "module name is not present in the name pool");
          }
        }
        Bindings.Scopes.resize(Inputs.size());
        return true;
      }

      template <typename DefinitionType>
      bool addDefinition(std::size_t Index, const DefinitionType &Definition)
      {
        if (Definition.parent() != Inputs[Index])
        {
          return fail(Index, "definition has an invalid parent module");
        }
        const auto Linkage = Definition.linkage();
        if (Linkage != MachineSymbolLinkage::Local && Linkage != MachineSymbolLinkage::External)
        {
          return fail(Index, "definition has invalid linkage");
        }
        const auto Name = Definition.name();
        if (!validName(Name, Linkage == MachineSymbolLinkage::Local))
        {
          return fail(Index, "a named definition is required for External linkage, and names must be registered");
        }
        if (Name == core::NAME_None)
        {
          return true;
        }
        const SymbolId Id = addSymbol(Index, Name, &Definition);
        if (!Bindings.Scopes[Index].emplace(Name, Id).second)
        {
          return fail(Index, fmt::format("duplicate definition of '{}' in the same module", spelling(Name)));
        }
        ReservedNames.insert(Name);
        if (Linkage == MachineSymbolLinkage::External)
        {
          const auto [Position, Inserted] = ExternalDefinitions.emplace(Name, Id);
          if (!Inserted)
          {
            return fail(Index, fmt::format("duplicate External definition of '{}'; first defined in input #{}", spelling(Name), Bindings.Symbols[Position->second.Index].ModuleIndex));
          }
        }
        return true;
      }

      bool collectDefinitions()
      {
        for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
        {
          for (const auto &Global : Inputs[Index]->globalVariables())
          {
            if (!addDefinition(Index, Global))
            {
              return false;
            }
          }
          for (const auto &Function : Inputs[Index]->functions())
          {
            if (!addDefinition(Index, Function))
            {
              return false;
            }
          }
        }
        return true;
      }

      bool bindDeclaration(std::size_t Index, const MachineSymbolDeclaration &Declaration)
      {
        if (!validName(Declaration.Name) || (Declaration.Kind != MachineSymbolKind::Function && Declaration.Kind != MachineSymbolKind::GlobalVariable) || (Declaration.Kind == MachineSymbolKind::Function && Declaration.IsConstant))
        {
          return fail(Index, "symbol declaration has an invalid name, symbol kind, or read-only attribute");
        }
        ReservedNames.insert(Declaration.Name);
        auto &Scope = Bindings.Scopes[Index];
        auto Target = lookup(Scope, Declaration.Name);
        if (!Target)
        {
          Target = lookup(ExternalDefinitions, Declaration.Name);
        }
        if (!Target)
        {
          Target = lookup(NativeSymbols, Declaration.Name);
          if (!Target)
          {
            Target = addSymbol(Index, Declaration.Name, Declaration);
            NativeSymbols.emplace(Declaration.Name, *Target);
          }
        }
        const auto &Symbol = Bindings.Symbols[Target->Index];
        if (Symbol.kind() != Declaration.Kind)
        {
          return fail(Index, fmt::format("declaration '{}' does not match the symbol kind in input #{}", spelling(Declaration.Name), Symbol.ModuleIndex));
        }
        if (Symbol.isConstant() != Declaration.IsConstant)
        {
          return fail(Index, fmt::format("declaration '{}' does not match the read-only attribute in input #{}", spelling(Declaration.Name), Symbol.ModuleIndex));
        }
        Scope.emplace(Declaration.Name, *Target);
        return true;
      }

      bool bindDeclarations()
      {
        for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
        {
          for (const auto &Declaration : Inputs[Index]->symbolDeclarations())
          {
            if (!bindDeclaration(Index, Declaration))
            {
              return false;
            }
          }
        }
        return true;
      }

      std::expected<void, std::string> bindReference(std::size_t Index, core::Name Name, SymbolReferenceTarget ExpectedTarget)
      {
        if (!validName(Name))
        {
          return std::unexpected("symbol operand requires a nonempty name registered in the name pool");
        }
        auto &Scope = Bindings.Scopes[Index];
        auto Resolved = lookup(Scope, Name);
        if (!Resolved)
        {
          Resolved = lookup(ExternalDefinitions, Name);
          if (!Resolved)
          {
            return std::unexpected(fmt::format("undefined symbol '{}'; native symbols require an explicit declaration in the referencing module", spelling(Name)));
          }
          Scope.emplace(Name, *Resolved);
        }
        const auto &Target = Bindings.Symbols[Resolved->Index].Target;
        if (ExpectedTarget == SymbolReferenceTarget::VMFunction && !std::holds_alternative<const MachineFunction *>(Target))
        {
          return std::unexpected(fmt::format("calli target '{}' must resolve to a VM function definition", spelling(Name)));
        }
        if (ExpectedTarget == SymbolReferenceTarget::NativeFunction && (!std::holds_alternative<MachineSymbolDeclaration>(Target) || Bindings.Symbols[Resolved->Index].kind() != MachineSymbolKind::Function))
        {
          return std::unexpected(fmt::format("callNativei target '{}' must resolve to an explicitly declared native function", spelling(Name)));
        }
        return {};
      }

      bool bindReferences()
      {
        for (std::size_t Index = 0; Index < Inputs.size(); ++Index)
        {
          const auto BindReference = [&](core::Name Name, SymbolReferenceTarget ExpectedTarget)
          {
            return bindReference(Index, Name, ExpectedTarget);
          };
          for (const auto &Function : Inputs[Index]->functions())
          {
            if (const auto Result = visitFunctionReferences(Function, BindReference); !Result)
            {
              return fail(Index, Result.error());
            }
          }
        }
        return true;
      }

      bool assignOutputNames()
      {
        std::size_t NextLocalIndex = 0;
        for (auto &Symbol : Bindings.Symbols)
        {
          if (Symbol.linkage() != MachineSymbolLinkage::Local)
          {
            continue;
          }
          core::Name Candidate;
          do
          {
            Candidate = core::NamePool::instance().intern(fmt::format(".__ink_link_local_{}", NextLocalIndex++));
            if (!Candidate.valid())
            {
              return fail(Symbol.ModuleIndex, "name pool cannot represent a generated Local symbol name");
            }
          } while (!ReservedNames.insert(Candidate).second);
          Symbol.OutputName = Candidate;
        }
        return true;
      }

      std::span<const MachineModule *const> Inputs;
      const core::DiagnosticEngine &Diagnostics;
      LinkBindings Bindings;
      SymbolTable ExternalDefinitions;
      SymbolTable NativeSymbols;
      std::unordered_set<core::Name> ReservedNames;
  };

  std::optional<LinkBindings> resolveLinkSymbols(std::span<const MachineModule *const> Inputs, const core::DiagnosticEngine &Diagnostics)
  {
    return SymbolResolver(Inputs, Diagnostics).resolve();
  }
} // namespace ink::vm::detail
