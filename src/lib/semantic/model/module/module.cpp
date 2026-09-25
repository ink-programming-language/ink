#include "ink/semantic/model/module/module.h"

#include "ink/semantic/context.h"

namespace ink::semantic
{
  Module::Module(const SemanticContext &Context, Name ModuleName, BasicBlock &EntryBlock) noexcept
      : Value(Context, ValueKind::Module, Context.getModuleType()),
        ModuleName(ModuleName),
        EntryBlock(EntryBlock)
  {
  }
} // namespace ink::semantic
