#ifndef INK_EXECUTION_RUNTIME_WORLD_H
#define INK_EXECUTION_RUNTIME_WORLD_H

#include "ink/execution/runtime_value.h"
#include "ink/target/target_key.h"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ink::execution
{
  using ExternalFunctionHandler = std::function<std::optional<RuntimeValue>(const std::vector<RuntimeValue> &)>;

  class RuntimeWorld
  {
  public:
    explicit RuntimeWorld(target::TargetKey TargetKey) noexcept;

    const target::TargetKey &targetKey() const noexcept;
    bool bindExternalFunction(std::string Name, ExternalFunctionHandler Handler);
    const ExternalFunctionHandler *externalFunction(std::string_view Name) const noexcept;

  private:
    target::TargetKey TargetKey;
    std::map<std::string, ExternalFunctionHandler, std::less<>> ExternalFunctions;
  };
} // namespace ink::execution

#endif
