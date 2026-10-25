#ifndef INK_IR_SERIALIZATION_MODEL_H
#define INK_IR_SERIALIZATION_MODEL_H

#include "ink/ir/serialization/serialization.h"

#include <limits>
#include <array>

namespace ink::ir::detail
{
  inline constexpr std::uint32_t NullId = std::numeric_limits<std::uint32_t>::max();

  // Wire values are permanent and independent of ValueKind/Instruction.def ordering.
  enum class RecordKind : std::uint32_t
  {
    Module = 1,
    BoolType = 10,
    IntegerType = 11,
    FloatType = 12,
    VoidType = 13,
    NullPointerType = 14,
    MetaType = 15,
    VariadicType = 16,
    PointerType = 17,
    ReferenceType = 18,
    ArrayType = 19,
    FunctionType = 20,
    ClassType = 21,
    InterfaceType = 22,
    StructType = 23,
    QualifiedType = 24,
    Field = 30,
    Function = 31,
    Parameter = 32,
    Block = 33,
    Global = 34,
    Import = 35,
    Export = 36,
    BoolConstant = 40,
    IntegerConstant = 41,
    FloatConstant = 42,
    NullPointerConstant = 43,
    StringConstant = 44,
    ArrayConstant = 45,
    Alloca = 60,
    Load = 61,
    Store = 62,
    Neg = 63,
    ICmp = 64,
    Branch = 65,
    CondBranch = 66,
    Return = 67,
    Call = 68,
    GetElementPtr = 69,
  };

  // IDs index Records. References contains IDs (NullId is an optional absent reference).
  // Numbers stores flags, widths, sizes and exact constant words. Strings stores byte strings.
  struct Record
  {
      RecordKind Kind = RecordKind::Module;
      std::vector<std::uint64_t> Numbers;
      std::vector<std::uint32_t> References;
      std::vector<std::string> Strings;
  };

  struct Model
  {
      std::uint32_t PointerBits = 0;
      std::uint32_t ByteOrder = 0;
      bool NativeAbi = false;
      std::array<std::uint64_t, 5> Alignments{};
      std::vector<Record> Records;
  };

  std::string_view recordKindName(RecordKind Kind) noexcept;
  IRSerializationResult<Model> collectModel(const Module &ModuleValue);
  IRSerializationResult<std::unique_ptr<Module>> decodeModel(IRContext &Context, const Model &Data, const IRReadOptions &Options);
} // namespace ink::ir::detail

#endif
