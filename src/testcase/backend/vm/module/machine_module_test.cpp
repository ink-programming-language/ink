#include "ink/core/name/name_pool.h"
#include "ink/backend/vm/module/machine_global_variable.h"
#include "ink/backend/vm/module/machine_module.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
  static_assert(!std::is_copy_constructible_v<MachineModule> && !std::is_move_constructible_v<MachineModule>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<MachineModule &>().functions())>, MachineFunction &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const MachineModule &>().functions())>, const MachineFunction &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<MachineModule &>().globalVariables())>, MachineGlobalVariable &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const MachineModule &>().globalVariables())>, const MachineGlobalVariable &>);
  static_assert(std::is_same_v<decltype(std::declval<const MachineFunction &>().parent()), const MachineModule *>);
  static_assert(std::is_same_v<decltype(std::declval<const MachineGlobalVariable &>().parent()), const MachineModule *>);

  // A fresh module owns no declarations or data, and its name does not require an IR context.
  TEST(VMMachineModuleTest, StartsWithEmptyIndependentCollections)
  {
    const auto Name = core::NamePool::instance().intern("vm-module");
    MachineModule Value(Name);
    EXPECT_EQ(Value.name(), Name);
    EXPECT_EQ(Value.functionCount(), 0U);
    EXPECT_EQ(Value.globalVariableCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(Value.functions()));
    EXPECT_TRUE(std::ranges::empty(Value.globalVariables()));
    MachineModule Anonymous;
    EXPECT_EQ(Anonymous.name(), core::NAME_None);
  }

  // Appending module functions preserves declaration order, parent links, references, and existing iterators.
  TEST(VMMachineModuleTest, OwnsStableOrderedFunctions)
  {
    MachineModule Value;
    const auto Name = core::NamePool::instance().intern("entry");
    MachineFunction &First = Value.createFunction(Name);
    const auto FirstPosition = Value.functions().begin();
    MachineFunction &Second = Value.createFunction();
    for (std::size_t Index = 0; Index < 128; ++Index)
    {
      Value.createFunction();
    }
    EXPECT_EQ(Value.functionCount(), 130U);
    EXPECT_EQ(&*FirstPosition, &First);
    EXPECT_EQ(&*std::next(FirstPosition), &Second);
    EXPECT_EQ(First.name(), Name);
    EXPECT_EQ(First.parent(), &Value);
    EXPECT_EQ(Second.parent(), &Value);
    const MachineModule &ReadOnly = Value;
    EXPECT_EQ(ReadOnly.functions().begin()->parent(), &ReadOnly);
  }

  // Read-only and writable globals own copies of their source bytes and preserve independent sizes, alignments and linkage.
  TEST(VMMachineModuleTest, CopiesDataPayloadsAndRecordsOwnership)
  {
    MachineModule Value;
    std::array<std::uint8_t, 3> Bytes{1, 2, 3};
    const auto ConstantName = core::NamePool::instance().intern("message");
    const auto GlobalName = core::NamePool::instance().intern("buffer");
    const auto Constant = Value.createGlobalVariable(32, 4, Bytes, ConstantName, MachineSymbolLinkage::External, true);
    const auto Global = Value.createGlobalVariable(64, 16, Bytes, GlobalName);
    ASSERT_TRUE(Constant.has_value());
    ASSERT_TRUE(Global.has_value());
    Bytes[0] = 99;
    EXPECT_EQ(Constant->get().name(), ConstantName);
    EXPECT_EQ(Constant->get().parent(), &Value);
    EXPECT_EQ(Constant->get().alignment(), 4U);
    EXPECT_EQ(Constant->get().byteSize(), 32U);
    ASSERT_EQ(Constant->get().initialBytes().size(), 3U);
    EXPECT_EQ(Constant->get().initialBytes()[0], 1U);
    EXPECT_EQ(Constant->get().linkage(), MachineSymbolLinkage::External);
    EXPECT_TRUE(Constant->get().isConstant());
    EXPECT_EQ(Global->get().name(), GlobalName);
    EXPECT_EQ(Global->get().parent(), &Value);
    EXPECT_EQ(Global->get().alignment(), 16U);
    EXPECT_EQ(Global->get().byteSize(), 64U);
    EXPECT_EQ(Global->get().linkage(), MachineSymbolLinkage::Local);
    EXPECT_FALSE(Global->get().isConstant());
    ASSERT_EQ(Global->get().initialBytes().size(), 3U);
    EXPECT_EQ(Global->get().initialBytes()[0], 1U);
    EXPECT_EQ(Value.globalVariableCount(), 2U);
  }

  // Invalid alignments and oversized global initializers fail explicitly without appending partial data declarations.
  TEST(VMMachineModuleTest, RejectsInvalidDataWithoutMutatingCollections)
  {
    MachineModule Value;
    const std::array<std::uint8_t, 3> Bytes{1, 2, 3};
    const std::uint64_t InvalidAlignments[] = {0, 3, std::numeric_limits<std::uint64_t>::max()};
    for (const bool IsConstant : {false, true})
    {
      SCOPED_TRACE(IsConstant);
      for (const auto Alignment : InvalidAlignments)
      {
        SCOPED_TRACE(Alignment);
        const auto Global = Value.createGlobalVariable(3, Alignment, Bytes, core::NAME_None, MachineSymbolLinkage::Local, IsConstant);
        ASSERT_FALSE(Global.has_value());
        EXPECT_EQ(Global.error(), MachineModuleDataError::InvalidAlignment);
      }
      const auto TooLarge = Value.createGlobalVariable(2, 1, Bytes, core::NAME_None, MachineSymbolLinkage::Local, IsConstant);
      ASSERT_FALSE(TooLarge.has_value());
      EXPECT_EQ(TooLarge.error(), MachineModuleDataError::InitializerTooLarge);
    }
    EXPECT_EQ(Value.globalVariableCount(), 0U);
  }

  // Empty data remains valid, and a huge global declaration stores only its short initializer instead of allocating runtime storage.
  TEST(VMMachineModuleTest, AcceptsEmptyPayloadsAndUnmaterializedGlobalStorage)
  {
    MachineModule Value;
    const auto EmptyConstant = Value.createGlobalVariable(0, 8, {}, core::NAME_None, MachineSymbolLinkage::Local, true);
    const auto EmptyGlobal = Value.createGlobalVariable(0);
    const std::array<std::uint8_t, 2> Bytes{7, 8};
    const auto HugeGlobal = Value.createGlobalVariable(std::numeric_limits<std::uint64_t>::max(), 1, Bytes);
    const auto HugeConstant = Value.createGlobalVariable(std::numeric_limits<std::uint64_t>::max(), 16, Bytes, core::NAME_None, MachineSymbolLinkage::Local, true);
    ASSERT_TRUE(EmptyConstant.has_value());
    ASSERT_TRUE(EmptyGlobal.has_value());
    ASSERT_TRUE(HugeGlobal.has_value());
    ASSERT_TRUE(HugeConstant.has_value());
    EXPECT_EQ(EmptyConstant->get().byteSize(), 0U);
    EXPECT_EQ(EmptyConstant->get().alignment(), 8U);
    EXPECT_TRUE(EmptyConstant->get().initialBytes().empty());
    EXPECT_TRUE(EmptyConstant->get().isConstant());
    EXPECT_EQ(EmptyGlobal->get().byteSize(), 0U);
    EXPECT_TRUE(EmptyGlobal->get().initialBytes().empty());
    EXPECT_FALSE(EmptyGlobal->get().isConstant());
    EXPECT_EQ(HugeGlobal->get().byteSize(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_TRUE(std::ranges::equal(HugeGlobal->get().initialBytes(), Bytes));
    EXPECT_EQ(HugeConstant->get().byteSize(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(HugeConstant->get().alignment(), 16U);
    EXPECT_TRUE(HugeConstant->get().isConstant());
    EXPECT_TRUE(std::ranges::equal(HugeConstant->get().initialBytes(), Bytes));
  }

  // Appending either read-only or writable globals preserves earlier addresses, iterators and shared collection order.
  TEST(VMMachineModuleTest, PreservesDataReferencesAcrossAppends)
  {
    MachineModule Value;
    const std::array<std::uint8_t, 1> Bytes{42};
    const auto FirstConstant = Value.createGlobalVariable(1, 1, Bytes, core::NAME_None, MachineSymbolLinkage::Local, true);
    const auto FirstGlobal = Value.createGlobalVariable(1, 1, Bytes);
    ASSERT_TRUE(FirstConstant.has_value());
    ASSERT_TRUE(FirstGlobal.has_value());
    const auto ConstantPosition = Value.globalVariables().begin();
    const auto GlobalPosition = std::next(ConstantPosition);
    for (std::size_t Index = 0; Index < 128; ++Index)
    {
      ASSERT_TRUE(Value.createGlobalVariable(0, 1, {}, core::NAME_None, MachineSymbolLinkage::Local, true).has_value());
      ASSERT_TRUE(Value.createGlobalVariable(0).has_value());
    }
    EXPECT_EQ(&*ConstantPosition, &FirstConstant->get());
    EXPECT_EQ(&*GlobalPosition, &FirstGlobal->get());
    EXPECT_EQ(FirstConstant->get().initialBytes()[0], 42U);
    EXPECT_TRUE(FirstConstant->get().isConstant());
    EXPECT_EQ(FirstGlobal->get().initialBytes()[0], 42U);
    EXPECT_FALSE(FirstGlobal->get().isConstant());
    EXPECT_EQ(Value.globalVariableCount(), 258U);
  }

  // Ordinary declarations retain the data mutability contract independently of their function or global symbol kind.
  TEST(VMMachineModuleTest, RecordsGlobalMutabilityInSymbolDeclarations)
  {
    MachineModule Value;
    const auto FunctionName = core::NamePool::instance().intern("declared-function");
    const auto ConstantName = core::NamePool::instance().intern("declared-read-only-data");
    const auto GlobalName = core::NamePool::instance().intern("declared-writable-data");
    Value.declareSymbol(FunctionName, MachineSymbolKind::Function);
    Value.declareSymbol(ConstantName, MachineSymbolKind::GlobalVariable, true);
    Value.declareSymbol(GlobalName, MachineSymbolKind::GlobalVariable);
    const auto Declarations = Value.symbolDeclarations();
    ASSERT_EQ(Declarations.size(), 3U);
    EXPECT_EQ(Declarations[0].Name, FunctionName);
    EXPECT_EQ(Declarations[0].Kind, MachineSymbolKind::Function);
    EXPECT_FALSE(Declarations[0].IsConstant);
    EXPECT_EQ(Declarations[1].Name, ConstantName);
    EXPECT_EQ(Declarations[1].Kind, MachineSymbolKind::GlobalVariable);
    EXPECT_TRUE(Declarations[1].IsConstant);
    EXPECT_EQ(Declarations[2].Name, GlobalName);
    EXPECT_EQ(Declarations[2].Kind, MachineSymbolKind::GlobalVariable);
    EXPECT_FALSE(Declarations[2].IsConstant);
  }
} // namespace ink::vm::test
