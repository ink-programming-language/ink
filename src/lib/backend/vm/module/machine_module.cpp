#include "ink/backend/vm/module/machine_module.h"

#include <cstdint>
#include <functional>
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
  } // namespace

  MachineModule::MachineModule(core::Name Name) noexcept
      : Name(Name)
  {
  }

  MachineFunction &MachineModule::createFunction(core::Name FunctionName, MachineSymbolLinkage Linkage)
  {
    MachineFunction &Created = Functions.emplace_back(FunctionName, Linkage);
    Created.Parent = this;
    return Created;
  }

  std::expected<std::reference_wrapper<MachineGlobalVariable>, MachineModuleDataError> MachineModule::createGlobalVariable(std::uint64_t ByteSize, std::uint64_t Alignment, std::span<const std::uint8_t> InitialBytes, core::Name GlobalName, MachineSymbolLinkage Linkage, bool IsConstant)
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
    if (InitialBytes.size() > OwnedBytes.max_size())
    {
      return std::unexpected(MachineModuleDataError::SizeOverflow);
    }
    if (!InitialBytes.empty())
    {
      OwnedBytes.assign(InitialBytes.begin(), InitialBytes.end());
    }
    MachineGlobalVariable &Created = Globals.emplace_back(Linkage, IsConstant);
    Created.Name = GlobalName;
    Created.Parent = this;
    Created.ByteSize = ByteSize;
    Created.Alignment = Alignment;
    Created.InitialBytes = std::move(OwnedBytes);
    return std::ref(Created);
  }

  void MachineModule::declareSymbol(core::Name SymbolName, MachineSymbolKind Kind, bool IsConstant)
  {
    SymbolDeclarations.push_back({SymbolName, Kind, IsConstant});
  }
} // namespace ink::vm
