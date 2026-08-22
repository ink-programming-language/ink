#ifndef INK_IR_SERIALIZATION_H
#define INK_IR_SERIALIZATION_H

#include "ink/core/diagnostic.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/module.h"

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace ink::ir
{
  class SerializeResult
  {
    public:
      bool succeeded() const noexcept
      {
        return Text.has_value();
      }

      const std::optional<std::string> &text() const noexcept
      {
        return Text;
      }

      std::optional<std::string> &text() noexcept
      {
        return Text;
      }

    private:
      std::optional<std::string> Text;

      friend SerializeResult printText(IRContext &Context, const Module &ModuleValue);
      friend SerializeResult serialize(IRContext &Context, const Module &ModuleValue);
  };

  class DeserializeResult
  {
    public:
      bool succeeded() const noexcept
      {
        return Value.has_value();
      }

      const std::optional<Module> &module() const noexcept
      {
        return Value;
      }

      std::optional<Module> &module() noexcept
      {
        return Value;
      }

    private:
      std::optional<Module> Value;

      friend DeserializeResult parseText(IRContext &Context, std::string_view Text);
      friend DeserializeResult parseSource(IRContext &Context, core::SourceId Source);
      friend DeserializeResult deserialize(IRContext &Context, std::string_view Text);
      friend DeserializeResult deserializeSource(IRContext &Context, core::SourceId Source);
  };

  SerializeResult printText(IRContext &Context, const Module &ModuleValue);
  SerializeResult printText(const Module &ModuleValue);
  DeserializeResult parseText(IRContext &Context, std::string_view Text);
  DeserializeResult parseSource(IRContext &Context, core::SourceId Source);
  SerializeResult serialize(IRContext &Context, const Module &ModuleValue);
  SerializeResult serialize(const Module &ModuleValue);
  DeserializeResult deserialize(IRContext &Context, std::string_view Text);
  DeserializeResult deserializeSource(IRContext &Context, core::SourceId Source);
} // namespace ink::ir

#endif
