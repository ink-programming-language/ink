#include "ink/ir/module/module.h"

#include "ink/ir/context.h"
#include "ink/ir/decl/module_decl.h"

namespace ink::ir
{
  Module::~Module() = default;

  Module::Module(const IRContext &Context, Name ModuleName, std::unique_ptr<BasicBlock> EntryBlock) noexcept
      : Value(Context, ValueKind::Module, Context.typePool().getType<TypeKind::Module>()),
        ModuleName(ModuleName),
        EntryBlock(std::move(EntryBlock))
  {
  }
} // namespace ink::ir
