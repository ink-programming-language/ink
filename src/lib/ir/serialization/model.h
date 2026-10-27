#ifndef INK_IR_SERIALIZATION_MODEL_H
#define INK_IR_SERIALIZATION_MODEL_H

#include "ink/ir/serialization/serialization.h"
#include "schema.h"

#include <limits>
#include <array>

namespace ink::ir::detail
{
  inline constexpr std::uint32_t NullId = std::numeric_limits<std::uint32_t>::max();

  // IDs index Records. References contains IDs (NullId is an optional absent reference).
  // Numbers stores flags, widths, sizes and exact constant words. Strings stores byte strings.
  struct Record
  {
      RecordKind Kind = RecordKind::Module;
      std::vector<std::uint64_t> Numbers;
      std::vector<std::uint32_t> References;
      std::vector<std::string> Strings;
  };

  inline Record makeRecord(RecordKind Kind)
  {
    const auto Shape = recordShape(Kind);
    return {Kind, std::vector<std::uint64_t>(Shape.NumberCount), std::vector<std::uint32_t>(Shape.ReferenceCount, NullId), std::vector<std::string>(Shape.StringCount)};
  }

  struct Model
  {
      std::uint32_t PointerBits = 0;
      std::uint32_t ByteOrder = 0;
      bool NativeAbi = false;
      std::array<std::uint64_t, 5> Alignments{};
      std::vector<Record> Records;
  };

  IRSerializationResult<Model> collectModel(const Module &ModuleValue);
  IRSerializationResult<std::unique_ptr<Module>> decodeModel(IRContext &Context, const Model &Data, const IRReadOptions &Options);
} // namespace ink::ir::detail

#endif
