#ifndef INK_IR_SERIALIZATION_H
#define INK_IR_SERIALIZATION_H

#include "ink/core/diagnostic.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/module.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ink::ir
{
  class SerializeResult
  {
    public:
      bool succeeded() const noexcept
      {
        return Text.has_value() && Diagnostics.empty();
      }

      const std::optional<std::string> &text() const noexcept
      {
        return Text;
      }

      std::optional<std::string> &text() noexcept
      {
        return Text;
      }

      const std::vector<core::Diagnostic> &diagnostics() const noexcept
      {
        return Diagnostics;
      }

    private:
      std::optional<std::string> Text;
      std::vector<core::Diagnostic> Diagnostics;

      friend SerializeResult serialize(IRContext &Context, const Module &ModuleValue);
  };

  class DeserializeResult
  {
    public:
      bool succeeded() const noexcept
      {
        return Value.has_value() && Diagnostics.empty();
      }

      const std::optional<Module> &module() const noexcept
      {
        return Value;
      }

      std::optional<Module> &module() noexcept
      {
        return Value;
      }

      const std::vector<core::Diagnostic> &diagnostics() const noexcept
      {
        return Diagnostics;
      }

    private:
      std::optional<Module> Value;
      std::vector<core::Diagnostic> Diagnostics;

      friend DeserializeResult deserialize(IRContext &Context, std::string_view Text);
  };

  SerializeResult serialize(IRContext &Context, const Module &ModuleValue);
  SerializeResult serialize(const Module &ModuleValue);
  DeserializeResult deserialize(IRContext &Context, std::string_view Text);
} // namespace ink::ir

#endif
