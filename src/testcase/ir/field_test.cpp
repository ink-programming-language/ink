#include "ink/core/name/name_pool.h"
#include "ink/ir/type/nominal_type/field.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    constexpr std::uint64_t MaximumSize = std::numeric_limits<std::uint64_t>::max();

    std::array<NominalType *, 3> createNominalTypes(TypeContext &Types, std::uint64_t Size)
    {
      return {Types.createClassType(Size, 8), Types.createInterfaceType(Size, 8), Types.createStructType(Size, 8)};
    }

    void expectField(const Field *Member, const Type *FieldType, core::Name Name, std::uint64_t Offset)
    {
      ASSERT_NE(Member, nullptr);
      EXPECT_EQ(Member->type(), FieldType);
      EXPECT_EQ(Member->name(), Name);
      EXPECT_EQ(Member->offset(), Offset);
      EXPECT_TRUE(Member->valid());
    }
  } // namespace

  static_assert(std::is_final_v<Field>);
  static_assert(!std::is_base_of_v<Value, Field>);
  static_assert(std::is_copy_constructible_v<Field>);
  static_assert(std::is_copy_assignable_v<Field>);
  static_assert(std::is_nothrow_constructible_v<Field, const Type *, core::Name, std::uint64_t>);
  static_assert(!std::is_convertible_v<const Type *, Field>);
  static_assert(std::is_same_v<decltype(std::declval<const Field &>().type()), const Type *>);
  static_assert(std::is_same_v<decltype(std::declval<NominalType &>().field(0)), const Field *>);

  // Field values preserve borrowed types, names and full-width offsets across independent copies.
  TEST(IRFieldTest, PreservesMetadataWhenCopied)
  {
    TypeContext Types(Target);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const Field Original(Types.getInt32Type(), Name, MaximumSize);
    EXPECT_TRUE(Original.hasName());
    expectField(&Original, Types.getInt32Type(), Name, MaximumSize);
    const Field Copied = Original;
    expectField(&Copied, Types.getInt32Type(), Name, MaximumSize);
    Field Assigned(Types.getBoolType());
    Assigned = Copied;
    expectField(&Assigned, Types.getInt32Type(), Name, MaximumSize);
    const Field DefaultOffset(Types.getBoolType(), Name);
    expectField(&DefaultOffset, Types.getBoolType(), Name, 0);
  }

  // Missing types, absent names and types without layouts are invalid; zero-sized laid-out types remain valid.
  TEST(IRFieldTest, RejectsMissingMetadataAndUnavailableLayouts)
  {
    TypeContext Types(Target);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const Field MissingType(nullptr, Name);
    EXPECT_EQ(MissingType.type(), nullptr);
    EXPECT_TRUE(MissingType.hasName());
    EXPECT_FALSE(MissingType.valid());
    const Field Unnamed(Types.getInt32Type());
    EXPECT_EQ(Unnamed.name(), core::NAME_None);
    EXPECT_EQ(Unnamed.offset(), 0U);
    EXPECT_FALSE(Unnamed.hasName());
    EXPECT_FALSE(Unnamed.valid());
    ASSERT_FALSE(Types.getVariadicType()->hasLayout());
    EXPECT_FALSE(Field(Types.getVariadicType(), Name).valid());
    ASSERT_TRUE(Types.getVoidType()->hasLayout());
    EXPECT_TRUE(Field(Types.getVoidType(), Name).valid());
  }

  // Every nominal kind preserves registration order and finds all same-name fields without imposing identity rules.
  TEST(IRFieldTest, NominalFieldsPreserveOrderAndAllowDuplicateNames)
  {
    TypeContext Types(Target);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const core::Name OtherName = core::NamePool::instance().intern("IRFieldTest.other");
    const core::Name MissingName = core::NamePool::instance().intern("IRFieldTest.missing");
    for (NominalType *Owner : createNominalTypes(Types, 16))
    {
      ASSERT_NE(Owner, nullptr);
      EXPECT_TRUE(Owner->fields().empty());
      EXPECT_EQ(Owner->fieldCount(), 0U);
      EXPECT_EQ(Owner->field(0), nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 0)));
      ASSERT_TRUE(Owner->addField(Field(Types.getBoolType(), OtherName, 4)));
      ASSERT_TRUE(Owner->addField(Field(Types.getInt64Type(), Name, 8)));
      ASSERT_EQ(Owner->fieldCount(), 3U);
      ASSERT_EQ(Owner->fields().size(), 3U);
      expectField(Owner->field(0), Types.getInt32Type(), Name, 0);
      expectField(Owner->field(1), Types.getBoolType(), OtherName, 4);
      expectField(Owner->field(2), Types.getInt64Type(), Name, 8);
      EXPECT_EQ(Owner->field(1), &Owner->fields()[1]);
      EXPECT_EQ(Owner->field(3), nullptr);
      EXPECT_EQ(Owner->field(std::numeric_limits<std::size_t>::max()), nullptr);
      const auto Matches = Owner->findFields(Name);
      ASSERT_EQ(Matches.size(), 2U);
      EXPECT_EQ(Matches[0], Owner->field(0));
      EXPECT_EQ(Matches[1], Owner->field(2));
      const auto OtherMatches = Owner->findFields(OtherName);
      ASSERT_EQ(OtherMatches.size(), 1U);
      EXPECT_EQ(OtherMatches[0], Owner->field(1));
      EXPECT_TRUE(Owner->findFields(MissingName).empty());
      EXPECT_TRUE(Owner->findFields(core::NAME_None).empty());
    }
  }

  // Replacement updates field metadata and name lookup in place, while invalid indices preserve every entry.
  TEST(IRFieldTest, ReplacesFieldsAndRejectsInvalidIndices)
  {
    TypeContext Types(Target);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const core::Name OtherName = core::NamePool::instance().intern("IRFieldTest.other");
    for (NominalType *Owner : createNominalTypes(Types, 8))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 0)));
      ASSERT_TRUE(Owner->addField(Field(Types.getBoolType(), OtherName, 4)));
      ASSERT_TRUE(Owner->replaceField(1, Field(Types.getInt32Type(), Name, 4)));
      EXPECT_EQ(Owner->fieldCount(), 2U);
      expectField(Owner->field(0), Types.getInt32Type(), Name, 0);
      expectField(Owner->field(1), Types.getInt32Type(), Name, 4);
      EXPECT_TRUE(Owner->findFields(OtherName).empty());
      const auto Matches = Owner->findFields(Name);
      ASSERT_EQ(Matches.size(), 2U);
      EXPECT_EQ(Matches[0], Owner->field(0));
      EXPECT_EQ(Matches[1], Owner->field(1));
      const Field Replacement(Types.getBoolType(), OtherName, 7);
      EXPECT_FALSE(Owner->replaceField(2, Replacement));
      EXPECT_FALSE(Owner->replaceField(std::numeric_limits<std::size_t>::max(), Replacement));
      EXPECT_EQ(Owner->fieldCount(), 2U);
      expectField(Owner->field(0), Types.getInt32Type(), Name, 0);
      expectField(Owner->field(1), Types.getInt32Type(), Name, 4);
      expectField(&Replacement, Types.getBoolType(), OtherName, 7);
    }
  }

  // Invalid metadata, foreign types, bounds failures and overflowing field ends reject both mutations without losing existing entries.
  TEST(IRFieldTest, RejectsInvalidFieldsWithoutChangingNominalStorage)
  {
    TypeContext Types(Target);
    TypeContext OtherTypes(Target);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const Field Rejected[] = {
        Field(nullptr, Name),
        Field(Types.getInt32Type()),
        Field(Types.getVariadicType(), Name),
        Field(OtherTypes.getInt32Type(), Name),
        Field(Types.getInt32Type(), Name, 9),
        Field(Types.getInt32Type(), Name, 5),
        Field(Types.getInt32Type(), Name, MaximumSize - 1),
    };
    for (NominalType *Owner : createNominalTypes(Types, 8))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 4)));
      for (const Field &Entry : Rejected)
      {
        EXPECT_FALSE(Owner->addField(Entry));
        EXPECT_FALSE(Owner->replaceField(0, Entry));
        ASSERT_EQ(Owner->fieldCount(), 1U);
        expectField(Owner->field(0), Types.getInt32Type(), Name, 4);
      }
    }
    for (NominalType *Owner : createNominalTypes(Types, MaximumSize))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, MaximumSize - 4)));
      const Field Overflow(Types.getInt32Type(), Name, MaximumSize - 1);
      EXPECT_FALSE(Owner->addField(Overflow));
      EXPECT_FALSE(Owner->replaceField(0, Overflow));
      ASSERT_EQ(Owner->fieldCount(), 1U);
      expectField(Owner->field(0), Types.getInt32Type(), Name, MaximumSize - 4);
    }
  }

  // Exact-end and zero-sized fields fit the owner, and metadata registration permits unaligned or overlapping storage.
  TEST(IRFieldTest, AcceptsBoundaryLayoutsWithoutImposingAlignmentOrOverlapRules)
  {
    TypeContext Types(Target);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const auto Empty = Types.createStructType(0, 1);
    ASSERT_NE(Empty, nullptr);
    for (NominalType *Owner : createNominalTypes(Types, 8))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 4)));
      ASSERT_TRUE(Owner->addField(Field(Empty, Name, 8)));
      ASSERT_TRUE(Owner->addField(Field(Types.getVoidType(), Name, 8)));
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 1)));
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 2)));
      EXPECT_FALSE(Owner->addField(Field(Empty, Name, 9)));
      EXPECT_EQ(Owner->fieldCount(), 5U);
      expectField(Owner->field(0), Types.getInt32Type(), Name, 4);
      expectField(Owner->field(1), Empty, Name, 8);
      expectField(Owner->field(2), Types.getVoidType(), Name, 8);
      expectField(Owner->field(3), Types.getInt32Type(), Name, 1);
      expectField(Owner->field(4), Types.getInt32Type(), Name, 2);
    }
  }

  // Freezing any nominal kind prevents field addition and replacement while retaining readable metadata and lookup.
  TEST(IRFieldTest, FrozenNominalTypesRejectFieldMutations)
  {
    TypeContext Types(Target);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const core::Name OtherName = core::NamePool::instance().intern("IRFieldTest.other");
    for (NominalType *Owner : createNominalTypes(Types, 8))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.getInt32Type(), Name, 0)));
      Owner->freeze();
      ASSERT_TRUE(Owner->isFrozen());
      const Field Replacement(Types.getBoolType(), OtherName, 4);
      EXPECT_FALSE(Owner->addField(Replacement));
      EXPECT_FALSE(Owner->replaceField(0, Replacement));
      ASSERT_EQ(Owner->fieldCount(), 1U);
      expectField(Owner->field(0), Types.getInt32Type(), Name, 0);
      const auto Matches = Owner->findFields(Name);
      ASSERT_EQ(Matches.size(), 1U);
      EXPECT_EQ(Matches[0], Owner->field(0));
      EXPECT_TRUE(Owner->findFields(OtherName).empty());
      expectField(&Replacement, Types.getBoolType(), OtherName, 4);
    }
  }
} // namespace ink::ir::test
