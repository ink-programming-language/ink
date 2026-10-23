#include "ink/ir/module/module_manager.h"

#include "ink/ir/module/module.h"

#include <algorithm>
#include <functional>
#include <utility>

namespace ink::ir
{
  namespace
  {
    bool validModuleID(const ModuleID &ID) noexcept
    {
      return !ID.Components.empty() && std::all_of(ID.Components.begin(), ID.Components.end(), [](core::Name Component)
      {
        return Component.valid();
      });
    }
  } // namespace

  ModuleManager::ModuleManager(IRContext &Context) noexcept
      : Context(Context)
  {
  }

  ModuleManager::~ModuleManager() noexcept
  {
    for (const auto &Entry : Modules)
    {
      delete Entry.second;
    }
  }

  std::size_t ModuleManager::ModuleIDHash::operator()(const ModuleID &ID) const noexcept
  {
    // Process-local hashing only; serialized identities must preserve canonical spellings instead.
    std::size_t Hash = ID.Components.size();
    for (core::Name Component : ID.Components)
    {
      Hash ^= std::hash<core::Name>{}(Component) + std::size_t{0x9e3779b9U} + (Hash << 6U) + (Hash >> 2U);
    }
    return Hash;
  }

  std::expected<Module *, ModuleError> ModuleManager::createModule(ModuleID ID)
  {
    if (!validModuleID(ID))
    {
      return std::unexpected(ModuleError::InvalidModuleID);
    }
    if (Modules.contains(ID))
    {
      return std::unexpected(ModuleError::DuplicateModule);
    }
    auto ModuleValue = std::make_unique<Module>(Context, std::move(ID));
    return adoptModule(ModuleValue);
  }

  std::expected<Module *, ModuleError> ModuleManager::adoptModule(std::unique_ptr<Module> &ModuleValue)
  {
    if (!ModuleValue)
    {
      return std::unexpected(ModuleError::InvalidModule);
    }
    if (&ModuleValue->irContext() != &Context)
    {
      return std::unexpected(ModuleError::ContextMismatch);
    }
    const ModuleID &ID = ModuleValue->moduleID();
    if (!validModuleID(ID))
    {
      return std::unexpected(ModuleError::InvalidModuleID);
    }
    if (!Modules.emplace(ID, ModuleValue.get()).second)
    {
      return std::unexpected(ModuleError::DuplicateModule);
    }
    return ModuleValue.release();
  }

  Module *ModuleManager::findModule(const ModuleID &ID) noexcept
  {
    const auto Found = Modules.find(ID);
    return Found != Modules.end() ? Found->second : nullptr;
  }

  const Module *ModuleManager::findModule(const ModuleID &ID) const noexcept
  {
    const auto Found = Modules.find(ID);
    return Found != Modules.end() ? Found->second : nullptr;
  }

  std::size_t ModuleManager::moduleCount() const noexcept
  {
    return Modules.size();
  }

  std::expected<const SymbolExport *, ModuleError> ModuleManager::resolveSymbol(const SymbolRef &Reference) const noexcept
  {
    if (!validModuleID(Reference.Module))
    {
      return std::unexpected(ModuleError::InvalidModuleID);
    }
    if (!Reference.Symbol.SymbolName.valid())
    {
      return std::unexpected(ModuleError::InvalidSymbolID);
    }
    const Module *Owner = findModule(Reference.Module);
    if (!Owner)
    {
      return std::unexpected(ModuleError::ModuleNotFound);
    }
    const SymbolExport *Export = Owner->findExport(Reference.Symbol);
    if (!Export)
    {
      return std::unexpected(ModuleError::SymbolNotFound);
    }
    return Export;
  }
} // namespace ink::ir
