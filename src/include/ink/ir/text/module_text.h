#pragma once

#include "ink/core/diagnostic/diagnostic.h"
#include "ink/ir/ir_context.h"
#include "ink/ir/module/module.h"

#include <expected>
#include <memory>
#include <string>
#include <string_view>

namespace ink::ir
{
  // Owns a parsed module and its IR context. Keep this owner alive while using the graph.
  // The module is destroyed before the context; record-owned methods are destroyed with the type pool.
  class ParsedModule final
  {
    public:
      explicit ParsedModule(core::Name Name = core::NAME_None)
          : ModuleValue(Name)
      {
      }

      auto &context(this auto &&Self) noexcept
      {
        return Self.Context;
      }

      auto &types(this auto &&Self) noexcept
      {
        return Self.Context.types();
      }

      auto &constants(this auto &&Self) noexcept
      {
        return Self.Context.constants();
      }

      auto &module(this auto &&Self) noexcept
      {
        return Self.ModuleValue;
      }

    private:
      IRContext Context;
      Module ModuleValue;
  };

  // Returns canonical Ink assembly with nested record fields/methods and stable, separate textual IDs.
  // Unsupported dependencies outside the module are reported explicitly instead of silently dropping nodes.
  [[nodiscard]] std::expected<std::string, core::Diagnostic> writeModuleText(const Module &ModuleValue);

  // Parses an independent owning graph. Malformed input returns a Core diagnostic and never invokes the fatal IR verifier.
  // Diagnostic ranges are byte offsets into Text; the caller may attach its own SourceId before reporting.
  [[nodiscard]] std::expected<std::unique_ptr<ParsedModule>, core::Diagnostic> readModuleText(std::string_view Text);
} // namespace ink::ir
