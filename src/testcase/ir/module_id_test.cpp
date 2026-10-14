#include "ink/core/name/name_pool.h"
#include "ink/ir/module/module_id.h"

#include <gtest/gtest.h>

#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(std::is_aggregate_v<ModuleID>);
  static_assert(std::is_copy_constructible_v<ModuleID>);
  static_assert(std::is_copy_assignable_v<ModuleID>);
  static_assert(std::is_nothrow_move_constructible_v<ModuleID>);
  static_assert(std::is_nothrow_move_assignable_v<ModuleID>);

  // Empty identifiers represent unnamed modules, and independently constructed identifiers compare by their complete ordered segments.
  TEST(IRModuleIDTest, EmptyAndIndependentlyConstructedIDsCompareByValue)
  {
    const ModuleID Unnamed;
    const ModuleID OtherUnnamed;
    EXPECT_TRUE(Unnamed.Components.empty());
    EXPECT_EQ(Unnamed, OtherUnnamed);
    EXPECT_FALSE(Unnamed != OtherUnnamed);

    auto &Names = core::NamePool::instance();
    const ModuleID First{{Names.intern("std"), Names.intern("net"), Names.intern("http")}};
    const ModuleID Second{{Names.intern("std"), Names.intern("net"), Names.intern("http")}};
    EXPECT_EQ(First, Second);
    EXPECT_EQ(Second, First);
    EXPECT_FALSE(First != Second);
    EXPECT_NE(First, Unnamed);
  }

  // Package segments, the module name, the number of segments and their order all participate in module identity.
  TEST(IRModuleIDTest, PackageAndModuleSegmentsLengthAndOrderParticipateInEquality)
  {
    auto &Names = core::NamePool::instance();
    const core::Name Standard = Names.intern("std");
    const core::Name Network = Names.intern("net");
    const core::Name InputOutput = Names.intern("io");
    const core::Name Http = Names.intern("http");
    const core::Name Client = Names.intern("client");
    const ModuleID Original{{Standard, Network, Http}};
    const ModuleID ChangedPackage{{Standard, InputOutput, Http}};
    const ModuleID ChangedModule{{Standard, Network, Client}};
    const ModuleID Shorter{{Http}};
    const ModuleID Longer{{Standard, Network, Http, Client}};
    const ModuleID Reordered{{Network, Standard, Http}};
    EXPECT_NE(Original, ChangedPackage);
    EXPECT_NE(Original, ChangedModule);
    EXPECT_NE(Original, Shorter);
    EXPECT_NE(Original, Longer);
    EXPECT_NE(Original, Reordered);
  }

  // Copies own independent segments, and moving a copied or assigned identifier preserves its complete value.
  TEST(IRModuleIDTest, CopiesRemainIndependentThroughMutationAndMoves)
  {
    auto &Names = core::NamePool::instance();
    const core::Name Standard = Names.intern("std");
    const core::Name Network = Names.intern("net");
    const core::Name Http = Names.intern("http");
    const core::Name Client = Names.intern("client");
    const ModuleID Original{{Standard, Network, Http}};
    const ModuleID ExpectedOriginal{{Standard, Network, Http}};
    const ModuleID ExpectedCopy{{Standard, Network, Client}};

    ModuleID Copy = Original;
    Copy.Components.back() = Client;
    EXPECT_EQ(Copy, ExpectedCopy);
    EXPECT_EQ(Original, ExpectedOriginal);

    ModuleID Assigned;
    Assigned = Original;
    Assigned.Components.clear();
    EXPECT_EQ(Assigned, ModuleID{});
    EXPECT_EQ(Original, ExpectedOriginal);

    ModuleID Moved(std::move(Copy));
    EXPECT_EQ(Moved, ExpectedCopy);
    Assigned = std::move(Moved);
    EXPECT_EQ(Assigned, ExpectedCopy);
    EXPECT_EQ(Original, ExpectedOriginal);
  }
} // namespace ink::ir::test
