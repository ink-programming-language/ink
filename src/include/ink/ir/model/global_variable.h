#ifndef INK_IR_GLOBAL_VARIABLE_H
#define INK_IR_GLOBAL_VARIABLE_H

#include "ink/ir/model/import_info.h"
#include "ink/ir/model/name.h"

#include <cstdint>
#include <optional>

namespace ink::ir
{
  class Type;

  enum class GlobalVariableKind : std::uint8_t
  {
    Definition,
    Imported,
  };

  struct GlobalVariable
  {
      ink::ir::Name Name;
      const Type *ValueType = nullptr;
      bool Mutable = true;
      GlobalVariableKind Kind = GlobalVariableKind::Definition;
      std::optional<ImportInfo> Import;
  };
} // namespace ink::ir

#endif
