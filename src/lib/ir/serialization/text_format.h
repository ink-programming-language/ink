#ifndef INK_IR_SERIALIZATION_TEXT_FORMAT_H
#define INK_IR_SERIALIZATION_TEXT_FORMAT_H

#include "model.h"

namespace ink::ir::detail
{
  // Human-readable syntax is deliberately isolated from the binary codec and its wire model.
  IRSerializationResult<std::string> writeAssembly(const Model &Data, std::size_t MaxBytes);
  IRSerializationResult<Model> parseAssembly(std::string_view Text, const IRReadOptions &Options);
} // namespace ink::ir::detail

#endif
