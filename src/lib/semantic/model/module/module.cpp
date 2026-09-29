#include "ink/semantic/model/module/module.h"

#include "ink/semantic/context.h"
#include "ink/semantic/model/decl/module_decl.h"

namespace ink::semantic
{
  Module::~Module() = default;

  Module::Module(const SemanticContext &Context, Name ModuleName, std::unique_ptr<BasicBlock> EntryBlock) noexcept
      : Value(Context, ValueKind::Module, Context.typePool().getType<TypeKind::Module>()),
        ModuleName(ModuleName),
        EntryBlock(std::move(EntryBlock))
  {
  }
} // namespace ink::semantic
