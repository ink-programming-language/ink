#include "ink/ir/ids.h"
#include "ink/ir/opcode.h"
#include "ink/ir/plan.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>

namespace ink::ir
{
  namespace
  {
    static_assert(!std::is_convertible<IrTypeId, IrTypeId::ValueType>::value, "IR IDs must not implicitly convert to their storage type");
    static_assert(!std::is_convertible<IrTypeId::ValueType, IrTypeId>::value, "IR IDs must not implicitly construct from their storage type");
    static_assert(!std::is_same<IrTypeId, IrValueId>::value, "IR table IDs must remain strongly typed");

    // Verifies strongly typed IR IDs and half-open table ranges preserve invalid, equality, ordering, and boundary semantics.
    TEST(IrIdentityContractTest, PreservesTypedIdsAndTableRanges)
    {
      constexpr IrTypeId Invalid;
      constexpr IrTypeId First = IrTypeId::fromValue(4);
      constexpr IrTypeId Equal = IrTypeId::fromValue(4);
      constexpr IrTypeId Later = IrTypeId::fromValue(5);
      constexpr IrTableRange Empty{4, 0};
      constexpr IrTableRange Range{4, 3};

      EXPECT_FALSE(Invalid.isValid());
      EXPECT_FALSE(static_cast<bool>(Invalid));
      EXPECT_TRUE(First.isValid());
      EXPECT_TRUE(static_cast<bool>(First));
      EXPECT_EQ(First, Equal);
      EXPECT_NE(First, Later);
      EXPECT_LT(First, Later);
      EXPECT_TRUE(Empty.empty());
      EXPECT_EQ(Empty.end(), 4U);
      EXPECT_FALSE(Empty.contains(4));
      EXPECT_FALSE(Range.empty());
      EXPECT_EQ(Range.end(), 7U);
      EXPECT_TRUE(Range.contains(4));
      EXPECT_TRUE(Range.contains(6));
      EXPECT_FALSE(Range.contains(3));
      EXPECT_FALSE(Range.contains(7));
      EXPECT_EQ(Range, (IrTableRange{4, 3}));
      EXPECT_NE(Range, Empty);
    }

    // Verifies stage and effect masks detect every requested flag without treating the empty mask as a present capability.
    TEST(IrFlagContractTest, CombinesAndQueriesStageAndEffectMasks)
    {
      constexpr IrStage Stages = IrStage::Staged | IrStage::Closed;
      constexpr IrEffect Effects = IrEffect::ReadMemory | IrEffect::MayTrap | IrEffect::Runtime;

      EXPECT_TRUE(hasStage(Stages, IrStage::Staged));
      EXPECT_TRUE(hasStage(Stages, IrStage::Closed));
      EXPECT_FALSE(hasStage(Stages, IrStage::None));
      EXPECT_FALSE(hasStage(IrStage::None, IrStage::Staged));
      EXPECT_TRUE(hasEffect(Effects, IrEffect::ReadMemory));
      EXPECT_TRUE(hasEffect(Effects, IrEffect::MayTrap));
      EXPECT_TRUE(hasEffect(Effects, IrEffect::Runtime));
      EXPECT_FALSE(hasEffect(Effects, IrEffect::WriteMemory));
      EXPECT_FALSE(hasEffect(Effects, IrEffect::None));
    }

    // Verifies every generated core opcode resolves to complete metadata at the matching ordinal and that invalid ordinals are rejected.
    TEST(IrSchemaContractTest, ExposesEveryCoreOpcodeMetadataEntry)
    {
      constexpr IrOpcodeMetadata Expectations[] = {
          {IrOpcode::Unknown, "unknown", IrPayloadKind::None, 0, 0, 0, 0, 0, false, IrEffect::None, IrStage::None},
#define INK_IR_OPCODE(Name, Mnemonic, Payload, MinimumOperands, MaximumOperands, MinimumResults, MaximumResults, Successors, Terminator, Effects, Stages) {IrOpcode::Name, Mnemonic, IrPayloadKind::Payload, MinimumOperands, MaximumOperands, MinimumResults, MaximumResults, Successors, Terminator, Effects, Stages},
#include "ink/ir/generated/opcode.def"
#undef INK_IR_OPCODE
      };

      for (const IrOpcodeMetadata &Expectation : Expectations)
      {
        const IrOpcodeMetadata *Metadata = irOpcodeMetadata(Expectation.Opcode);
        ASSERT_NE(Metadata, nullptr);
        EXPECT_EQ(Metadata->Opcode, Expectation.Opcode);
        EXPECT_STREQ(Metadata->Mnemonic, Expectation.Mnemonic);
        EXPECT_EQ(Metadata->Payload, Expectation.Payload);
        EXPECT_EQ(Metadata->MinimumOperands, Expectation.MinimumOperands);
        EXPECT_EQ(Metadata->MaximumOperands, Expectation.MaximumOperands);
        EXPECT_EQ(Metadata->MinimumResults, Expectation.MinimumResults);
        EXPECT_EQ(Metadata->MaximumResults, Expectation.MaximumResults);
        EXPECT_EQ(Metadata->Successors, Expectation.Successors);
        EXPECT_EQ(Metadata->Terminator, Expectation.Terminator);
        EXPECT_EQ(Metadata->Effects, Expectation.Effects);
        EXPECT_EQ(Metadata->Stages, Expectation.Stages);
        EXPECT_STREQ(irOpcodeName(Expectation.Opcode), Expectation.Mnemonic);
        EXPECT_EQ(isIrOpcode(Expectation.Opcode), Expectation.Opcode != IrOpcode::Unknown);
      }

      const auto OutOfDomain = static_cast<IrOpcode>(255);
      EXPECT_EQ(irOpcodeMetadata(OutOfDomain), nullptr);
      EXPECT_STREQ(irOpcodeName(OutOfDomain), "unknown");
      EXPECT_FALSE(isIrOpcode(OutOfDomain));
    }

    // Verifies every generated elaboration-plan opcode resolves to complete metadata and that unknown ordinals cannot masquerade as valid plan operations.
    TEST(IrSchemaContractTest, ExposesEveryPlanOpcodeMetadataEntry)
    {
      constexpr IrPlanOpcodeMetadata Expectations[] = {
          {IrPlanOpcode::Unknown, "unknown", 0, 0, 0, IrStage::None},
#define INK_IR_PLAN_OPCODE(Name, Mnemonic, MinimumInputs, MaximumInputs, ResultCount, Stages) {IrPlanOpcode::Name, Mnemonic, MinimumInputs, MaximumInputs, ResultCount, Stages},
#include "ink/ir/generated/plan_opcode.def"
#undef INK_IR_PLAN_OPCODE
      };

      for (const IrPlanOpcodeMetadata &Expectation : Expectations)
      {
        const IrPlanOpcodeMetadata *Metadata = irPlanOpcodeMetadata(Expectation.Opcode);
        ASSERT_NE(Metadata, nullptr);
        EXPECT_EQ(Metadata->Opcode, Expectation.Opcode);
        EXPECT_STREQ(Metadata->Mnemonic, Expectation.Mnemonic);
        EXPECT_EQ(Metadata->MinimumInputs, Expectation.MinimumInputs);
        EXPECT_EQ(Metadata->MaximumInputs, Expectation.MaximumInputs);
        EXPECT_EQ(Metadata->ResultCount, Expectation.ResultCount);
        EXPECT_EQ(Metadata->Stages, Expectation.Stages);
        EXPECT_STREQ(irPlanOpcodeName(Expectation.Opcode), Expectation.Mnemonic);
        EXPECT_EQ(isIrPlanOpcode(Expectation.Opcode), Expectation.Opcode != IrPlanOpcode::Unknown);
      }

      const auto OutOfDomain = static_cast<IrPlanOpcode>(255);
      EXPECT_EQ(irPlanOpcodeMetadata(OutOfDomain), nullptr);
      EXPECT_STREQ(irPlanOpcodeName(OutOfDomain), "unknown");
      EXPECT_FALSE(isIrPlanOpcode(OutOfDomain));
    }
  } // namespace
} // namespace ink::ir
