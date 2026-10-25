#include "ink/ir/serialization/serialization.h"

#include "text_format.h"

namespace ink::ir
{
  IRSerializationResult<std::string> IRTextWriter::write(const Module &ModuleValue, const IRTextWriteOptions &Options)
  {
    const auto Valid = IRVerifier::verify(ModuleValue);
    if (!Valid)
    {
      return std::unexpected(Valid.error());
    }
    const auto Data = detail::collectModel(ModuleValue);
    if (!Data)
    {
      return std::unexpected(Data.error());
    }
    return detail::writeAssembly(*Data, Options.MaxBytes);
  }

  IRSerializationResult<std::unique_ptr<Module>> IRTextReader::read(IRContext &Context, std::string_view Text, const IRReadOptions &Options)
  {
    auto Data = detail::parseAssembly(Text, Options);
    if (!Data)
    {
      return std::unexpected(Data.error());
    }
    return detail::decodeModel(Context, *Data, Options);
  }

  IRSerializationResult<std::string> writeText(const Module &ModuleValue, const IRTextWriteOptions &Options)
  {
    return IRTextWriter::write(ModuleValue, Options);
  }

  IRSerializationResult<std::unique_ptr<Module>> readText(IRContext &Context, std::string_view Text, const IRReadOptions &Options)
  {
    return IRTextReader::read(Context, Text, Options);
  }
} // namespace ink::ir
