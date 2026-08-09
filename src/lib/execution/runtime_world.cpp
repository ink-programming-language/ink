#include "ink/execution/runtime_world.h"

#include <string>
#include <utility>

namespace ink::execution
{
  RuntimeWorld::RuntimeWorld(target::TargetKey TargetKeyValue) noexcept : TargetKey(std::move(TargetKeyValue))
  {
  }

  const target::TargetKey &RuntimeWorld::targetKey() const noexcept
  {
    return TargetKey;
  }

  bool RuntimeWorld::bindExternalFunction(std::string Name, ExternalFunctionHandler Handler)
  {
    if (Name.empty() || !Handler)
    {
      return false;
    }
    return ExternalFunctions.emplace(std::move(Name), std::move(Handler)).second;
  }

  const ExternalFunctionHandler *RuntimeWorld::externalFunction(std::string_view Name) const noexcept
  {
    const auto Function = ExternalFunctions.find(Name);
    return Function == ExternalFunctions.end() ? nullptr : &Function->second;
  }
} // namespace ink::execution
