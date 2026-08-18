#ifndef INK_IR_FUNCTION_H
#define INK_IR_FUNCTION_H

#include "ink/ir/model/attribute.h"
#include "ink/ir/model/basic_block.h"
#include "ink/ir/model/import_info.h"
#include "ink/ir/model/name.h"
#include "ink/ir/model/parameter.h"
#include "ink/ir/model/type.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace ink::ir
{
  enum class FunctionKind : std::uint8_t
  {
    Definition,
    Imported,
    External,
  };

  enum class CallingConvention : std::uint8_t
  {
    Ink,
    C,
  };

  struct Function
  {
      explicit Function(const Type &ResultType) noexcept;

      const Attribute *attribute(AttributeKind AttributeKindValue) const noexcept;
      bool hasAttribute(AttributeKind AttributeKindValue) const noexcept;
      std::size_t parameterCount() const noexcept;
      const Parameter &parameter(std::size_t ParameterIndex) const noexcept;
      const Type *parameterType(std::size_t ParameterIndex) const noexcept;

      ink::ir::Name Name;
      FunctionKind Kind = FunctionKind::Definition;
      CallingConvention Convention = CallingConvention::Ink;
      std::optional<ImportInfo> Import;
      const Type *ResultType;
      std::vector<Parameter> Parameters;
      std::vector<Attribute> Attributes;
      std::vector<BasicBlock> Blocks;
  };
} // namespace ink::ir

#endif
