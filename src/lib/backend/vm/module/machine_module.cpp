#include "ink/backend/vm/module/machine_module.h"

#include <cstdint>
#include <functional>
#include <limits>
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

  MachineFunction &MachineModule::createFunction(core::Name FunctionName)
  {
    MachineFunction &Created = Functions.emplace_back(FunctionName);
    Created.Parent = this;
    return Created;
  }

  std::expected<std::reference_wrapper<MachineConstant>, MachineModuleDataError> MachineModule::createConstant(std::span<const std::uint8_t> Bytes, std::uint64_t Alignment, core::Name ConstantName)
  {
    if (!isValidAlignment(Alignment))
    {
      return std::unexpected(MachineModuleDataError::InvalidAlignment);
    }
    std::vector<std::uint8_t> OwnedBytes;
    if (Bytes.size() > OwnedBytes.max_size() || std::cmp_greater(Bytes.size(), std::numeric_limits<std::uint64_t>::max()))
    {
      return std::unexpected(MachineModuleDataError::SizeOverflow);
    }
    if (!Bytes.empty())
    {
      OwnedBytes.assign(Bytes.begin(), Bytes.end());
    }
    MachineConstant &Created = Constants.emplace_back();
    Created.Name = ConstantName;
    Created.Parent = this;
    Created.Alignment = Alignment;
    Created.Bytes = std::move(OwnedBytes);
    return std::ref(Created);
  }

  std::expected<std::reference_wrapper<MachineGlobalVariable>, MachineModuleDataError> MachineModule::createGlobalVariable(std::uint64_t ByteSize, std::uint64_t Alignment, std::span<const std::uint8_t> InitialBytes, core::Name GlobalName)
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
    MachineGlobalVariable &Created = Globals.emplace_back();
    Created.Name = GlobalName;
    Created.Parent = this;
    Created.ByteSize = ByteSize;
    Created.Alignment = Alignment;
    Created.InitialBytes = std::move(OwnedBytes);
    return std::ref(Created);
  }
} // namespace ink::vm
