#include "module_serialization_internal.h"
#include "../../core/archive_text.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallString.h>
#include <charconv>
#include <unordered_map>
#include <unordered_set>

#include "module_serialization_text_writer.inc"
#include "module_serialization_text_reader.inc"

namespace ink::ir::archive
{
  bool readText(std::string_view Text, State &Data)
  {
    return TextReader(Text, Data).run();
  }

  std::string writeText(State &Data)
  {
    return TextWriter(Data).run();
  }
} // namespace ink::ir::archive
