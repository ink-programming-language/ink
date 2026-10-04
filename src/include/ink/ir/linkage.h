#ifndef INK_IR_LINKAGE_H
#define INK_IR_LINKAGE_H

#include "ink/abi/name_mangling.h"

namespace ink::ir
{
  class Type;
  class Function;
  class Module;

  // The sole semantic-to-wire adapter used by all code generators.
  abi::Record declarationRecord(const abi::ModuleIdentity &Module, std::span<const abi::Record> Owners, char Kind, std::string_view Name, const abi::Record &Pattern = {'H', {}});
  std::optional<abi::Record> typeRecord(const Type &Value, std::size_t Depth = 0);
  std::optional<abi::Record> functionRecord(const Function &Value, const abi::ModuleIdentity *DetachedModule = nullptr, std::string_view DetachedName = {}, std::size_t Depth = 0);
  abi::MangleResult functionSymbol(const Function &Value, const abi::ModuleIdentity *DetachedModule = nullptr, std::string_view DetachedName = {});
  abi::MangleResult reflectionSymbol(const Module &Value);
  abi::MangleResult reflectionThunkSymbol(const Function &Value);
} // namespace ink::ir

#endif
