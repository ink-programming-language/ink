#include "ink/ir/module/module_serialization.h"
#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>
#include <llvm/Support/Endian.h>

#include <array>
#include <iterator>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr std::string_view LogicalModuleText = R"(ink-ir 7
module @Logic {
  define bool @compare(bool %left, bool %right, i32 %number, i32 %other) {
  entry:
    %negated = not bool %left
    %both = and bool %negated, %right
    %either = or bool %both, %left
    %equal = cmp eq i32 %number, %other
    %not_equal = cmp ne i32 %number, %other
    %less = cmp lt i32 %number, %other
    %less_equal = cmp le i32 %number, %other
    %greater = cmp gt i32 %number, %other
    %greater_equal = cmp ge i32 %number, %other
    %same_bool = cmp eq bool %left, %right
    %different_bool = cmp ne bool %left, %right
    %unsigned_greater = cmp gt u8 255, 128
    ret bool %either
  }
}
)";

    constexpr core::ComparisonPredicate Predicates[] = {
        core::ComparisonPredicate::Equal,
        core::ComparisonPredicate::NotEqual,
        core::ComparisonPredicate::Less,
        core::ComparisonPredicate::LessEqual,
        core::ComparisonPredicate::Greater,
        core::ComparisonPredicate::GreaterEqual,
    };

    struct BinaryRecord
    {
        std::size_t Offset;
        std::uint32_t Id;
        std::uint32_t Kind;
    };

    std::vector<BinaryRecord> binaryRecords(std::string_view Bytes)
    {
      std::vector<BinaryRecord> Records;
      for (std::size_t Offset = 16; Offset < Bytes.size();)
      {
        const auto *Header = Bytes.data() + Offset;
        Records.push_back({Offset, static_cast<std::uint32_t>(Records.size() + 1), llvm::support::endian::read32le(Header)});
        Offset += 32 + static_cast<std::size_t>(llvm::support::endian::read32le(Header + 12)) * 8 + static_cast<std::size_t>(llvm::support::endian::read64le(Header + 16)) + static_cast<std::size_t>(llvm::support::endian::read64le(Header + 24));
      }
      return Records;
    }
  } // namespace

  // Logical operands and all integer and bool comparison predicates preserve identity in text and binary archives.
  TEST(IRModuleSerializationTest, LogicalAndComparisonInstructionsRoundTrip)
  {
    core::CompilationContext Compilation;
    IRContext Source(Compilation);
    const auto Parsed = deserializeModuleText(Source, LogicalModuleText);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    for (const bool Binary : {false, true})
    {
      const auto Encoded = Binary ? serializeModuleBinary(*Parsed.ModuleValue) : serializeModuleText(*Parsed.ModuleValue);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      IRContext Destination(Compilation);
      const auto Decoded = Binary ? deserializeModuleBinary(Destination, Encoded.Bytes) : deserializeModuleText(Destination, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      const auto &FunctionValue = static_cast<const Function &>(*Decoded.ModuleValue->entryBlock().values()[0]);
      const auto &Values = FunctionValue.entryBlock()->values();
      ASSERT_EQ(Values.size(), 13U);
      ASSERT_TRUE(LogicalNotInstruction::classof(Values[0].get()));
      EXPECT_EQ(&static_cast<const LogicalNotInstruction &>(*Values[0]).operand(), FunctionValue.parameters()[0].get());
      ASSERT_TRUE(LogicalAndInstruction::classof(Values[1].get()));
      const auto &And = static_cast<const LogicalAndInstruction &>(*Values[1]);
      EXPECT_EQ(&And.left(), Values[0].get());
      EXPECT_EQ(&And.right(), FunctionValue.parameters()[1].get());
      ASSERT_TRUE(LogicalOrInstruction::classof(Values[2].get()));
      const auto &Or = static_cast<const LogicalOrInstruction &>(*Values[2]);
      EXPECT_EQ(&Or.left(), Values[1].get());
      EXPECT_EQ(&Or.right(), FunctionValue.parameters()[0].get());
      for (std::size_t Index = 0; Index < std::size(Predicates); ++Index)
      {
        ASSERT_TRUE(CompareInstruction::classof(Values[Index + 3].get()));
        const auto &Compare = static_cast<const CompareInstruction &>(*Values[Index + 3]);
        EXPECT_EQ(Compare.predicate(), Predicates[Index]);
        EXPECT_EQ(&Compare.left(), FunctionValue.parameters()[2].get());
        EXPECT_EQ(&Compare.right(), FunctionValue.parameters()[3].get());
      }
      EXPECT_EQ(static_cast<const CompareInstruction &>(*Values[9]).predicate(), core::ComparisonPredicate::Equal);
      EXPECT_EQ(static_cast<const CompareInstruction &>(*Values[10]).predicate(), core::ComparisonPredicate::NotEqual);
      const auto &Unsigned = static_cast<const CompareInstruction &>(*Values[11]);
      EXPECT_EQ(Unsigned.predicate(), core::ComparisonPredicate::Greater);
      const auto &UnsignedType = static_cast<const IntegerType &>(Unsigned.left().type());
      EXPECT_FALSE(UnsignedType.isSigned());
      EXPECT_EQ(UnsignedType.bitWidth(), 8U);
      for (std::size_t Index = 0; Index < Values.size() - 1; ++Index)
      {
        EXPECT_EQ(Values[Index]->type().typeKind(), TypeKind::Bool);
      }
      const auto Reencoded = Binary ? serializeModuleBinary(*Decoded.ModuleValue) : serializeModuleText(*Decoded.ModuleValue);
      ASSERT_TRUE(Reencoded.succeeded()) << Reencoded.Message;
      EXPECT_EQ(Reencoded.Bytes, Encoded.Bytes);
    }
  }

  // New wire tags remain appended at 52-55 and comparison predicates use their documented stable values.
  TEST(IRModuleSerializationTest, LogicalInstructionsUseStableWireIdsAndPredicateValues)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto Parsed = deserializeModuleText(Context, LogicalModuleText);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    const auto Binary = serializeModuleBinary(*Parsed.ModuleValue);
    ASSERT_TRUE(Binary.succeeded()) << Binary.Message;
    std::array<std::size_t, 4> Counts{};
    const std::uint64_t ExpectedPredicates[] = {
        0,
        1,
        2,
        3,
        4,
        5,
        0,
        1,
        4,
    };
    for (const auto &Record : binaryRecords(Binary.Bytes))
    {
      if (Record.Kind >= 52 && Record.Kind <= 55)
      {
        const auto Index = Record.Kind - 52;
        if (Record.Kind == 55)
        {
          ASSERT_LT(Counts[Index], std::size(ExpectedPredicates));
          EXPECT_EQ(llvm::support::endian::read64le(Binary.Bytes.data() + Record.Offset + 32), ExpectedPredicates[Counts[Index]]);
        }
        ++Counts[Index];
      }
    }
    EXPECT_EQ(Counts, (std::array<std::size_t, 4>{1, 1, 1, 9}));
    const auto Text = serializeModuleText(*Parsed.ModuleValue);
    ASSERT_TRUE(Text.succeeded()) << Text.Message;
    EXPECT_NE(Text.Bytes.find("not bool %left"), std::string::npos);
    EXPECT_NE(Text.Bytes.find("and bool %v0, %right"), std::string::npos);
    EXPECT_NE(Text.Bytes.find("or bool %v1, %left"), std::string::npos);
    EXPECT_NE(Text.Bytes.find("cmp ge i32 %number, %other"), std::string::npos);
    EXPECT_NE(Text.Bytes.find("cmp gt u8 255, 128"), std::string::npos);
  }

  // Forward dependencies resolve through logical and comparison nodes without confusing a predicate with an object ID.
  TEST(IRModuleSerializationTest, LogicalInstructionsResolveForwardOperands)
  {
    constexpr std::string_view Text = R"(ink-ir 7
module @Forward {
  define bool @f(i32 %left, i32 %right) {
  entry:
    %result = and bool %compared, %negated
    %compared = cmp eq i32 %left, %right
    %negated = not bool %later
    %later = or bool true, false
    ret bool %result
  }
}
)";
    core::CompilationContext Compilation;
    IRContext Source(Compilation);
    const auto Parsed = deserializeModuleText(Source, Text);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    for (const bool Binary : {false, true})
    {
      const auto Encoded = Binary ? serializeModuleBinary(*Parsed.ModuleValue) : serializeModuleText(*Parsed.ModuleValue);
      ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
      IRContext Destination(Compilation);
      const auto Decoded = Binary ? deserializeModuleBinary(Destination, Encoded.Bytes) : deserializeModuleText(Destination, Encoded.Bytes);
      ASSERT_TRUE(Decoded.succeeded()) << Decoded.Message;
      const auto &FunctionValue = static_cast<const Function &>(*Decoded.ModuleValue->entryBlock().values()[0]);
      const auto &Values = FunctionValue.entryBlock()->values();
      const auto &And = static_cast<const LogicalAndInstruction &>(*Values[0]);
      EXPECT_EQ(&And.left(), Values[1].get());
      EXPECT_EQ(&And.right(), Values[2].get());
      EXPECT_EQ(&static_cast<const LogicalNotInstruction &>(*Values[2]).operand(), Values[3].get());
    }
  }

  // Invalid predicates, mismatched operand types, unsupported comparisons and cyclic references fail without publishing IR.
  TEST(IRModuleSerializationTest, RejectsMalformedLogicalAndComparisonText)
  {
    const std::string_view Instructions[] = {
        "%x = not i32 1 ret bool %x",
        "%x = and i32 1, 0 ret bool %x",
        "%x = or bool %number, false ret bool %x",
        "%x = cmp unknown i32 1, 2 ret bool %x",
        "%x = cmp 6 i32 1, 2 ret bool %x",
        "%x = cmp lt bool true, false ret bool %x",
        "%x = cmp le bool true, false ret bool %x",
        "%x = cmp gt bool true, false ret bool %x",
        "%x = cmp ge bool true, false ret bool %x",
        "%x = cmp eq i32 %flag, 1 ret bool %x",
        "%x = cmp eq u32 %number, 1 ret bool %x",
        "%x = cmp eq f32 bits(0x0), bits(0x0) ret bool %x",
        "%x = not bool %missing ret bool %x",
        "%x = and bool %x, true ret bool %x",
        "%x = not bool %x ret bool %x",
        "%x = or bool %y, false %y = not bool %x ret bool %x",
        "%x = cmp eq bool %x, true ret bool %x",
        "not bool true ret bool false",
        "cmp eq i32 1, 2 ret bool false",
    };
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    for (const auto InstructionsText : Instructions)
    {
      const auto Result = deserializeModuleText(Context, "ink-ir 7 module @Invalid { define bool @f(bool %flag, i32 %number) { entry: " + std::string(InstructionsText) + " } }");
      EXPECT_EQ(Result.Status, core::ArchiveStatus::InvalidArchive) << InstructionsText << ": " << Result.Message;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_TRUE(Context.modules().empty());
    }
  }

  // Binary decoding validates every logical operand, comparison predicate, result type and operand type before publication.
  TEST(IRModuleSerializationTest, RejectsMalformedLogicalAndComparisonBinary)
  {
    core::CompilationContext Compilation;
    IRContext Source(Compilation);
    const auto Parsed = deserializeModuleText(Source, LogicalModuleText);
    ASSERT_TRUE(Parsed.succeeded()) << Parsed.Message;
    const auto Encoded = serializeModuleBinary(*Parsed.ModuleValue);
    ASSERT_TRUE(Encoded.succeeded()) << Encoded.Message;
    const auto Records = binaryRecords(Encoded.Bytes);
    std::uint32_t IntegerTypeId = 0, IntegerConstantId = 0, BoolParameterId = 0;
    for (const auto &Record : Records)
    {
      if (Record.Kind == 21)
      {
        IntegerTypeId = Record.Id;
      }
      else if (Record.Kind == 31)
      {
        IntegerConstantId = Record.Id;
      }
      else if (Record.Kind == 38 && !BoolParameterId)
      {
        BoolParameterId = Record.Id;
      }
    }
    ASSERT_NE(IntegerTypeId, 0U);
    ASSERT_NE(IntegerConstantId, 0U);
    ASSERT_NE(BoolParameterId, 0U);
    const auto Reject = [&](const std::string &Bytes)
    {
      IRContext Destination(Compilation);
      const auto Result = deserializeModuleBinary(Destination, Bytes);
      EXPECT_EQ(Result.Status, core::ArchiveStatus::InvalidArchive) << Result.Message;
      EXPECT_EQ(Result.ModuleValue, nullptr);
      EXPECT_TRUE(Destination.modules().empty());
    };
    for (const auto &Record : Records)
    {
      if (Record.Kind < 52 || Record.Kind > 55)
      {
        continue;
      }
      auto WrongResult = Encoded.Bytes;
      llvm::support::endian::write32le(WrongResult.data() + Record.Offset + 4, IntegerTypeId);
      Reject(WrongResult);
      const auto Fields = llvm::support::endian::read32le(Encoded.Bytes.data() + Record.Offset + 12);
      for (std::uint32_t Index = Record.Kind == 55 ? 1 : 0; Index < Fields; ++Index)
      {
        for (const std::uint64_t InvalidId : {std::uint64_t{0}, std::uint64_t{UINT32_MAX}, std::uint64_t{1}, std::uint64_t{Record.Id}})
        {
          auto BadReference = Encoded.Bytes;
          llvm::support::endian::write64le(BadReference.data() + Record.Offset + 32 + Index * 8, InvalidId);
          Reject(BadReference);
        }
      }
      auto WrongOperand = Encoded.Bytes;
      const auto OperandOffset = Record.Offset + (Record.Kind == 55 ? 40 : 32);
      const auto OriginalOperandId = llvm::support::endian::read64le(Encoded.Bytes.data() + OperandOffset);
      const auto OriginalOperandTypeId = llvm::support::endian::read32le(Encoded.Bytes.data() + Records[static_cast<std::size_t>(OriginalOperandId - 1)].Offset + 4);
      const auto OriginalOperandTypeKind = Records[OriginalOperandTypeId - 1].Kind;
      llvm::support::endian::write64le(WrongOperand.data() + OperandOffset, OriginalOperandTypeKind == 18 ? IntegerConstantId : BoolParameterId);
      Reject(WrongOperand);
      if (Record.Kind == 55)
      {
        for (const auto InvalidPredicate : {std::uint64_t{6}, std::uint64_t{UINT64_MAX}})
        {
          auto WrongPredicate = Encoded.Bytes;
          llvm::support::endian::write64le(WrongPredicate.data() + Record.Offset + 32, InvalidPredicate);
          Reject(WrongPredicate);
        }
        if (OriginalOperandTypeKind == 18)
        {
          auto OrderedBool = Encoded.Bytes;
          llvm::support::endian::write64le(OrderedBool.data() + Record.Offset + 32, 2);
          Reject(OrderedBool);
        }
      }
    }
  }
} // namespace ink::ir::test
