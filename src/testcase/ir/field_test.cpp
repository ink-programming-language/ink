#include "ink/core/name/name_pool.h"
#include "ink/ir/type/nominal_type/field.h"
#include "ink/ir/module/module.h"

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

    std::array<NominalType *, 3> createNominalTypes(Module &Types, std::uint64_t Size)
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
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const Field Original(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, MaximumSize);
    EXPECT_TRUE(Original.hasName());
    expectField(&Original, Types.typePool().get<ink::ir::IntegerType>(32, true), Name, MaximumSize);
    const Field Copied = Original;
    expectField(&Copied, Types.typePool().get<ink::ir::IntegerType>(32, true), Name, MaximumSize);
    Field Assigned(Types.typePool().get<ink::ir::BoolType>());
    Assigned = Copied;
    expectField(&Assigned, Types.typePool().get<ink::ir::IntegerType>(32, true), Name, MaximumSize);
    const Field DefaultOffset(Types.typePool().get<ink::ir::BoolType>(), Name);
    expectField(&DefaultOffset, Types.typePool().get<ink::ir::BoolType>(), Name, 0);
  }

  // Missing types, absent names and types without layouts are invalid; zero-sized laid-out types remain valid.
  TEST(IRFieldTest, RejectsMissingMetadataAndUnavailableLayouts)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const Field MissingType(nullptr, Name);
    EXPECT_EQ(MissingType.type(), nullptr);
    EXPECT_TRUE(MissingType.hasName());
    EXPECT_FALSE(MissingType.valid());
    const Field Unnamed(Types.typePool().get<ink::ir::IntegerType>(32, true));
    EXPECT_EQ(Unnamed.name(), core::NAME_None);
    EXPECT_EQ(Unnamed.offset(), 0U);
    EXPECT_FALSE(Unnamed.hasName());
    EXPECT_FALSE(Unnamed.valid());
    ASSERT_FALSE(Types.typePool().get<ink::ir::VariadicType>()->hasLayout());
    EXPECT_FALSE(Field(Types.typePool().get<ink::ir::VariadicType>(), Name).valid());
    ASSERT_TRUE(Types.typePool().get<ink::ir::VoidType>()->hasLayout());
    EXPECT_TRUE(Field(Types.typePool().get<ink::ir::VoidType>(), Name).valid());
  }

  // Every nominal kind preserves registration order and finds all same-name fields without imposing identity rules.
  TEST(IRFieldTest, NominalFieldsPreserveOrderAndAllowDuplicateNames)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const core::Name OtherName = core::NamePool::instance().intern("IRFieldTest.other");
    const core::Name MissingName = core::NamePool::instance().intern("IRFieldTest.missing");
    for (NominalType *Owner : createNominalTypes(Types, 16))
    {
      ASSERT_NE(Owner, nullptr);
      EXPECT_TRUE(Owner->fields().empty());
      EXPECT_EQ(Owner->fieldCount(), 0U);
      EXPECT_EQ(Owner->field(0), nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0)));
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::BoolType>(), OtherName, 4)));
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(64, true), Name, 8)));
      ASSERT_EQ(Owner->fieldCount(), 3U);
      ASSERT_EQ(Owner->fields().size(), 3U);
      expectField(Owner->field(0), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0);
      expectField(Owner->field(1), Types.typePool().get<ink::ir::BoolType>(), OtherName, 4);
      expectField(Owner->field(2), Types.typePool().get<ink::ir::IntegerType>(64, true), Name, 8);
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
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const core::Name OtherName = core::NamePool::instance().intern("IRFieldTest.other");
    for (NominalType *Owner : createNominalTypes(Types, 8))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0)));
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::BoolType>(), OtherName, 4)));
      ASSERT_TRUE(Owner->replaceField(1, Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4)));
      EXPECT_EQ(Owner->fieldCount(), 2U);
      expectField(Owner->field(0), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0);
      expectField(Owner->field(1), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4);
      EXPECT_TRUE(Owner->findFields(OtherName).empty());
      const auto Matches = Owner->findFields(Name);
      ASSERT_EQ(Matches.size(), 2U);
      EXPECT_EQ(Matches[0], Owner->field(0));
      EXPECT_EQ(Matches[1], Owner->field(1));
      const Field Replacement(Types.typePool().get<ink::ir::BoolType>(), OtherName, 7);
      EXPECT_FALSE(Owner->replaceField(2, Replacement));
      EXPECT_FALSE(Owner->replaceField(std::numeric_limits<std::size_t>::max(), Replacement));
      EXPECT_EQ(Owner->fieldCount(), 2U);
      expectField(Owner->field(0), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0);
      expectField(Owner->field(1), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4);
      expectField(&Replacement, Types.typePool().get<ink::ir::BoolType>(), OtherName, 7);
    }
  }

  // Invalid metadata, foreign types, bounds failures and overflowing field ends reject both mutations without losing existing entries.
  TEST(IRFieldTest, RejectsInvalidFieldsWithoutChangingNominalStorage)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext OtherTypesContext(Target);
    Module OtherTypes(OtherTypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const Field Rejected[] = {
        Field(nullptr, Name),
        Field(Types.typePool().get<ink::ir::IntegerType>(32, true)),
        Field(Types.typePool().get<ink::ir::VariadicType>(), Name),
        Field(OtherTypes.typePool().get<ink::ir::IntegerType>(32, true), Name),
        Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 9),
        Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 5),
        Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, MaximumSize - 1),
    };
    for (NominalType *Owner : createNominalTypes(Types, 8))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4)));
      for (const Field &Entry : Rejected)
      {
        EXPECT_FALSE(Owner->addField(Entry));
        EXPECT_FALSE(Owner->replaceField(0, Entry));
        ASSERT_EQ(Owner->fieldCount(), 1U);
        expectField(Owner->field(0), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4);
      }
    }
    for (NominalType *Owner : createNominalTypes(Types, MaximumSize))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, MaximumSize - 4)));
      const Field Overflow(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, MaximumSize - 1);
      EXPECT_FALSE(Owner->addField(Overflow));
      EXPECT_FALSE(Owner->replaceField(0, Overflow));
      ASSERT_EQ(Owner->fieldCount(), 1U);
      expectField(Owner->field(0), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, MaximumSize - 4);
    }
  }

  // Exact-end and zero-sized fields fit the owner, and metadata registration permits unaligned or overlapping storage.
  TEST(IRFieldTest, AcceptsBoundaryLayoutsWithoutImposingAlignmentOrOverlapRules)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const auto Empty = Types.createStructType(0, 1);
    ASSERT_NE(Empty, nullptr);
    for (NominalType *Owner : createNominalTypes(Types, 8))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4)));
      ASSERT_TRUE(Owner->addField(Field(Empty, Name, 8)));
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::VoidType>(), Name, 8)));
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 1)));
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 2)));
      EXPECT_FALSE(Owner->addField(Field(Empty, Name, 9)));
      EXPECT_EQ(Owner->fieldCount(), 5U);
      expectField(Owner->field(0), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 4);
      expectField(Owner->field(1), Empty, Name, 8);
      expectField(Owner->field(2), Types.typePool().get<ink::ir::VoidType>(), Name, 8);
      expectField(Owner->field(3), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 1);
      expectField(Owner->field(4), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 2);
    }
  }

  // Freezing any nominal kind prevents field addition and replacement while retaining readable metadata and lookup.
  TEST(IRFieldTest, FrozenNominalTypesRejectFieldMutations)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name Name = core::NamePool::instance().intern("IRFieldTest.member");
    const core::Name OtherName = core::NamePool::instance().intern("IRFieldTest.other");
    for (NominalType *Owner : createNominalTypes(Types, 8))
    {
      ASSERT_NE(Owner, nullptr);
      ASSERT_TRUE(Owner->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0)));
      Owner->freeze();
      ASSERT_TRUE(Owner->isFrozen());
      const Field Replacement(Types.typePool().get<ink::ir::BoolType>(), OtherName, 4);
      EXPECT_FALSE(Owner->addField(Replacement));
      EXPECT_FALSE(Owner->replaceField(0, Replacement));
      ASSERT_EQ(Owner->fieldCount(), 1U);
      expectField(Owner->field(0), Types.typePool().get<ink::ir::IntegerType>(32, true), Name, 0);
      const auto Matches = Owner->findFields(Name);
      ASSERT_EQ(Matches.size(), 1U);
      EXPECT_EQ(Matches[0], Owner->field(0));
      EXPECT_TRUE(Owner->findFields(OtherName).empty());
      expectField(&Replacement, Types.typePool().get<ink::ir::BoolType>(), OtherName, 4);
    }
  }
} // namespace ink::ir::test
