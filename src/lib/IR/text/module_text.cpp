#include "ink/ir/text/module_text.h"

#include "text_format.h"

#include <utility>

namespace ink::ir
{
  std::expected<std::string, core::Diagnostic> writeModuleText(const Module &ModuleValue)
  {
    auto Syntax = text::extractModule(ModuleValue);
    if (!Syntax)
    {
      return std::unexpected(std::move(Syntax.error()));
    }
    text::TextWriter Writer;
    if (!text::formatModule(Writer, *Syntax))
    {
      return std::unexpected(*Writer.error());
    }
    return Writer.take();
  }

  std::expected<std::unique_ptr<ParsedModule>, core::Diagnostic> readModuleText(std::string_view Text)
  {
    text::TextReader Reader(Text);
    text::ModuleSyntax Syntax;
    if (!text::formatModule(Reader, Syntax) || !Reader.done())
    {
      return std::unexpected(*Reader.error());
    }
    return text::buildModule(Syntax);
  }
} // namespace ink::ir
