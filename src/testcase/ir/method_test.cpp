#include "ink/core/name_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/function/method.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <optional>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(!std::is_base_of_v<Type, Method>);
  static_assert(!std::is_base_of_v<Value, Method>);
  static_assert(std::is_copy_constructible_v<Method>);
  static_assert(std::is_copy_assignable_v<Method>);

  // Default dispatch is a valid direct method without virtual, override, final or slot metadata.
  TEST(IRMethodTest, DirectMethodHasNoVirtualProperties)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.direct");
    const Method Member(Name, *Signature);
    EXPECT_TRUE(Member.valid());
    EXPECT_EQ(Member.name(), Name);
    EXPECT_EQ(Member.functionType(), Signature);
    EXPECT_EQ(Member.virtualInfo().Dispatch, MethodDispatch::Direct);
    EXPECT_FALSE(Member.isVirtual());
    EXPECT_FALSE(Member.isAbstract());
    EXPECT_FALSE(Member.isOverride());
    EXPECT_FALSE(Member.isFinal());
    EXPECT_EQ(Member.virtualSlot(), std::nullopt);
  }

  // Direct dispatch rejects every nonempty combination of virtual-only flags and assigned slots.
  TEST(IRMethodTest, DirectMethodRejectsVirtualProperties)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.direct");
    for (std::uint32_t Flags = 1; Flags < 8; ++Flags)
    {
      const MethodVirtualInfo Info{MethodDispatch::Direct, (Flags & 1U) != 0, (Flags & 2U) != 0, (Flags & 4U) != 0 ? std::optional<std::uint32_t>(0) : std::nullopt};
      EXPECT_FALSE(Method(Name, *Signature, Info).valid()) << Flags;
    }
  }

  // Virtual methods support independent override and final flags with an optional slot, including slot zero.
  TEST(IRMethodTest, VirtualMethodsAcceptOverrideFinalAndAssignedSlots)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.virtual");
    for (std::uint32_t Flags = 0; Flags < 8; ++Flags)
    {
      const MethodVirtualInfo Info{MethodDispatch::Virtual, (Flags & 1U) != 0, (Flags & 2U) != 0, (Flags & 4U) != 0 ? std::optional<std::uint32_t>(0) : std::nullopt};
      const Method Member(Name, *Signature, Info);
      EXPECT_TRUE(Member.valid()) << Flags;
      EXPECT_TRUE(Member.isVirtual());
      EXPECT_FALSE(Member.isAbstract());
      EXPECT_EQ(Member.isOverride(), Info.Override);
      EXPECT_EQ(Member.isFinal(), Info.Final);
      EXPECT_EQ(Member.virtualSlot(), Info.Slot);
    }
    const Method LastSlot(Name, *Signature, {MethodDispatch::Virtual, false, false, std::numeric_limits<std::uint32_t>::max()});
    EXPECT_TRUE(LastSlot.valid());
    EXPECT_EQ(LastSlot.virtualSlot(), std::numeric_limits<std::uint32_t>::max());
  }

  // Abstract methods remain virtual and can override a base slot, but cannot forbid their implementation with final.
  TEST(IRMethodTest, AbstractMethodsAllowOverridesAndRejectFinal)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.abstract");
    for (std::uint32_t Flags = 0; Flags < 8; ++Flags)
    {
      const MethodVirtualInfo Info{MethodDispatch::Abstract, (Flags & 1U) != 0, (Flags & 2U) != 0, (Flags & 4U) != 0 ? std::optional<std::uint32_t>(0) : std::nullopt};
      const Method Member(Name, *Signature, Info);
      EXPECT_EQ(Member.valid(), !Info.Final) << Flags;
      EXPECT_TRUE(Member.isVirtual());
      EXPECT_TRUE(Member.isAbstract());
      EXPECT_EQ(Member.isOverride(), Info.Override);
      EXPECT_EQ(Member.virtualSlot(), Info.Slot);
    }
  }

  // Missing source names and unrecognized dispatch values are invalid.
  TEST(IRMethodTest, RejectsMissingNameAndUnknownDispatch)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.name");
    EXPECT_FALSE(Method(core::NAME_None, *Signature).valid());
    const Method Unknown(Name, *Signature, {static_cast<MethodDispatch>(255), false, false, std::nullopt});
    EXPECT_FALSE(Unknown.valid());
    EXPECT_FALSE(Unknown.isVirtual());
    EXPECT_FALSE(Unknown.isAbstract());
  }

  // Member metadata borrowed from a function signature survives function destruction while the type context lives.
  TEST(IRMethodTest, MetadataOutlivesFunctionAndRetainsSourceName)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getInt32Type(), {Types.getInt32Type()});
    ASSERT_NE(Signature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const core::Name LoweredName = core::NamePool::instance().intern("ink_math_Point_method_i32");
    std::optional<Method> Member;
    {
      Function Callable(*Signature, MemberName);
      Member.emplace(MemberName, *Callable.functionType(), MethodVirtualInfo{MethodDispatch::Virtual, true, true, 0});
      Callable.setName(LoweredName);
      EXPECT_EQ(Member->name(), MemberName);
      EXPECT_NE(Member->name(), Callable.name());
    }
    ASSERT_TRUE(Member.has_value());
    EXPECT_TRUE(Member->valid());
    EXPECT_EQ(Member->name(), MemberName);
    EXPECT_EQ(Member->functionType(), Signature);
    EXPECT_EQ(Member->functionType()->parameterCount(), 1U);
    EXPECT_TRUE(Member->isOverride());
    EXPECT_TRUE(Member->isFinal());
    EXPECT_EQ(Member->virtualSlot(), 0U);
  }

  // Methods can share an interned signature, while copying metadata preserves the source name and dispatch properties.
  TEST(IRMethodTest, SharedSignaturePreservesIndependentMethodMetadata)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name FirstName = core::NamePool::instance().intern("IRMethodTest.first");
    const core::Name SecondName = core::NamePool::instance().intern("IRMethodTest.second");
    const Method First(FirstName, *Signature, {MethodDispatch::Abstract, true, false, 3});
    Method Second(SecondName, *Signature);
    EXPECT_EQ(First.functionType(), Second.functionType());
    EXPECT_NE(First.name(), Second.name());
    const Method Copy = First;
    Second = Copy;
    EXPECT_TRUE(Second.valid());
    EXPECT_EQ(Second.name(), FirstName);
    EXPECT_TRUE(Second.isAbstract());
    EXPECT_TRUE(Second.isOverride());
    EXPECT_FALSE(Second.isFinal());
    EXPECT_EQ(Second.virtualSlot(), 3U);
  }
} // namespace ink::ir::test
