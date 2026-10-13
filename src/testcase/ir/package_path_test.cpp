#include "ink/core/name/name_pool.h"
#include "ink/ir/package/package_path.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::ir::test
{
  static_assert(std::is_aggregate_v<PackagePath>);
  static_assert(std::is_copy_constructible_v<PackagePath>);
  static_assert(std::is_copy_assignable_v<PackagePath>);
  static_assert(std::is_move_constructible_v<PackagePath>);
  static_assert(std::is_move_assignable_v<PackagePath>);

  // Empty paths represent the root package, and separately constructed paths with the same ordered segments compare equal.
  TEST(IRPackagePathTest, RootAndIndependentlyConstructedPathsCompareByValue)
  {
    const PackagePath Root;
    const PackagePath OtherRoot;
    EXPECT_TRUE(Root.Components.empty());
    EXPECT_EQ(Root, OtherRoot);
    EXPECT_FALSE(Root != OtherRoot);

    auto &Names = core::NamePool::instance();
    const PackagePath First{{Names.intern("std"), Names.intern("net")}};
    const PackagePath Second{{Names.intern("std"), Names.intern("net")}};
    EXPECT_EQ(First, Second);
    EXPECT_EQ(Second, First);
    EXPECT_FALSE(First != Second);
    EXPECT_NE(First, Root);
  }

  // A different segment, a different number of segments or a different order identifies a different package path.
  TEST(IRPackagePathTest, SegmentValuesLengthAndOrderParticipateInEquality)
  {
    auto &Names = core::NamePool::instance();
    const core::Name Standard = Names.intern("std");
    const core::Name Network = Names.intern("net");
    const core::Name InputOutput = Names.intern("io");
    const PackagePath Original{{Standard, Network}};
    const PackagePath ChangedSegment{{Standard, InputOutput}};
    const PackagePath Shorter{{Standard}};
    const PackagePath Longer{{Standard, Network, InputOutput}};
    const PackagePath Reordered{{Network, Standard}};
    EXPECT_NE(Original, ChangedSegment);
    EXPECT_NE(Original, Shorter);
    EXPECT_NE(Original, Longer);
    EXPECT_NE(Original, Reordered);
  }

  // Appending or removing the last segment of a copy creates child or parent paths without changing the original value.
  TEST(IRPackagePathTest, CopiedPathsCanDeriveParentsAndChildrenIndependently)
  {
    auto &Names = core::NamePool::instance();
    const core::Name Standard = Names.intern("std");
    const core::Name Network = Names.intern("net");
    const core::Name Http = Names.intern("http");
    const PackagePath Original{{Standard, Network}};
    const PackagePath ExpectedOriginal{{Standard, Network}};
    const PackagePath ExpectedChild{{Standard, Network, Http}};
    const PackagePath ExpectedParent{{Standard}};

    PackagePath Child = Original;
    Child.Components.push_back(Http);
    EXPECT_EQ(Child, ExpectedChild);
    EXPECT_EQ(Original, ExpectedOriginal);
    Child.Components.pop_back();
    EXPECT_EQ(Child, Original);

    PackagePath Parent;
    Parent = Original;
    Parent.Components.pop_back();
    EXPECT_EQ(Parent, ExpectedParent);
    EXPECT_EQ(Original, ExpectedOriginal);
    Parent.Components.pop_back();
    EXPECT_EQ(Parent, PackagePath{});
    EXPECT_EQ(Original, ExpectedOriginal);
  }
} // namespace ink::ir::test
