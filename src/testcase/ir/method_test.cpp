#include "ink/core/name/name_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/nominal_type/method.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    void expectNoFunction(Method &Member)
    {
      const Method &ReadOnly = Member;
      EXPECT_EQ(Member.function(), nullptr);
      EXPECT_EQ(ReadOnly.function(), nullptr);
      EXPECT_EQ(Member.name(), core::NAME_None);
      EXPECT_EQ(Member.functionType(), nullptr);
      EXPECT_FALSE(Member.valid());
    }
  } // namespace

  static_assert(!std::is_base_of_v<Type, Method>);
  static_assert(!std::is_base_of_v<Value, Method>);
  static_assert(!std::is_copy_constructible_v<Method>);
  static_assert(!std::is_copy_assignable_v<Method>);
  static_assert(std::is_nothrow_move_constructible_v<Method>);
  static_assert(std::is_nothrow_move_assignable_v<Method>);
  static_assert(!std::is_constructible_v<Method, Function &>);
  static_assert(!std::is_constructible_v<Method, Function &&>);
  static_assert(!std::is_convertible_v<std::unique_ptr<Function>, Method>);

  // Default dispatch is a valid direct method without virtual, override, final or slot metadata.
  TEST(IRMethodTest, DirectMethodHasNoVirtualProperties)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.direct");
    auto Callable = std::make_unique<Function>(*Signature, Name);
    const auto *FunctionAddress = Callable.get();
    const Method Member(std::move(Callable));
    EXPECT_EQ(Callable, nullptr);
    EXPECT_TRUE(Member.valid());
    EXPECT_EQ(Member.function(), FunctionAddress);
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
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.direct");
    for (std::uint32_t Flags = 1; Flags < 8; ++Flags)
    {
      const MethodVirtualInfo Info{MethodDispatch::Direct, (Flags & 1U) != 0, (Flags & 2U) != 0, (Flags & 4U) != 0 ? std::optional<std::uint32_t>(0) : std::nullopt};
      EXPECT_FALSE(Method(std::make_unique<Function>(*Signature, Name), Info).valid()) << Flags;
    }
  }

  // Virtual methods support independent override and final flags with an optional slot, including slot zero.
  TEST(IRMethodTest, VirtualMethodsAcceptOverrideFinalAndAssignedSlots)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.virtual");
    for (std::uint32_t Flags = 0; Flags < 8; ++Flags)
    {
      const MethodVirtualInfo Info{MethodDispatch::Virtual, (Flags & 1U) != 0, (Flags & 2U) != 0, (Flags & 4U) != 0 ? std::optional<std::uint32_t>(0) : std::nullopt};
      const Method Member(std::make_unique<Function>(*Signature, Name), Info);
      EXPECT_TRUE(Member.valid()) << Flags;
      EXPECT_TRUE(Member.isVirtual());
      EXPECT_FALSE(Member.isAbstract());
      EXPECT_EQ(Member.isOverride(), Info.Override);
      EXPECT_EQ(Member.isFinal(), Info.Final);
      EXPECT_EQ(Member.virtualSlot(), Info.Slot);
    }
    const Method LastSlot(std::make_unique<Function>(*Signature, Name), {MethodDispatch::Virtual, false, false, std::numeric_limits<std::uint32_t>::max()});
    EXPECT_TRUE(LastSlot.valid());
    EXPECT_EQ(LastSlot.virtualSlot(), std::numeric_limits<std::uint32_t>::max());
  }

  // Abstract methods remain virtual and can override a base slot, but cannot forbid their implementation with final.
  TEST(IRMethodTest, AbstractMethodsAllowOverridesAndRejectFinal)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.abstract");
    for (std::uint32_t Flags = 0; Flags < 8; ++Flags)
    {
      const MethodVirtualInfo Info{MethodDispatch::Abstract, (Flags & 1U) != 0, (Flags & 2U) != 0, (Flags & 4U) != 0 ? std::optional<std::uint32_t>(0) : std::nullopt};
      const Method Member(std::make_unique<Function>(*Signature, Name), Info);
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
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.name");
    EXPECT_FALSE(Method(std::make_unique<Function>(*Signature)).valid());
    const Method Unknown(std::make_unique<Function>(*Signature, Name), {static_cast<MethodDispatch>(255), false, false, std::nullopt});
    EXPECT_FALSE(Unknown.valid());
    EXPECT_FALSE(Unknown.isVirtual());
    EXPECT_FALSE(Unknown.isAbstract());
  }

  // Function renames and name removal immediately update method identity and validity without changing dispatch metadata.
  TEST(IRMethodTest, FunctionNameChangesAreReflectedImmediately)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    ASSERT_NE(Signature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const core::Name LoweredName = core::NamePool::instance().intern("ink_math_Point_method_i32");
    Method Member(std::make_unique<Function>(*Signature, MemberName), {MethodDispatch::Virtual, true, true, 0});
    auto *Callable = Member.function();
    ASSERT_NE(Callable, nullptr);
    EXPECT_TRUE(Member.valid());
    EXPECT_EQ(Member.name(), MemberName);
    Callable->setName(LoweredName);
    EXPECT_TRUE(Member.valid());
    EXPECT_EQ(Member.name(), LoweredName);
    EXPECT_EQ(Member.name(), Callable->name());
    Callable->setName(core::NAME_None);
    EXPECT_FALSE(Member.valid());
    EXPECT_EQ(Member.name(), core::NAME_None);
    Callable->setName(MemberName);
    EXPECT_TRUE(Member.valid());
    EXPECT_EQ(Member.name(), MemberName);
    EXPECT_EQ(Member.function(), Callable);
    EXPECT_EQ(Member.functionType(), Signature);
    EXPECT_EQ(Member.functionType()->parameterCount(), 1U);
    EXPECT_TRUE(Member.isOverride());
    EXPECT_TRUE(Member.isFinal());
    EXPECT_EQ(Member.virtualSlot(), 0U);
  }

  // Methods sharing a signature own distinct functions whose names and bodies remain independent.
  TEST(IRMethodTest, SharedSignaturePreservesIndependentFunctionIdentity)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name FirstName = core::NamePool::instance().intern("IRMethodTest.first");
    const core::Name SecondName = core::NamePool::instance().intern("IRMethodTest.second");
    Method First(std::make_unique<Function>(*Signature, FirstName), {MethodDispatch::Abstract, true, false, 3});
    Method Second(std::make_unique<Function>(*Signature, SecondName));
    EXPECT_EQ(First.functionType(), Second.functionType());
    EXPECT_NE(First.name(), Second.name());
    EXPECT_NE(First.function(), Second.function());
    First.function()->setName(SecondName);
    EXPECT_EQ(First.name(), SecondName);
    EXPECT_EQ(Second.name(), SecondName);
    ASSERT_NE(Second.function()->createBlock(), nullptr);
    EXPECT_TRUE(First.function()->isDeclaration());
    EXPECT_FALSE(Second.function()->isDeclaration());
    EXPECT_TRUE(First.isAbstract());
    EXPECT_FALSE(Second.isVirtual());
  }

  // Mutable and const method access reaches the same function parameters and body, including blocks added after binding.
  TEST(IRMethodTest, BoundFunctionProvidesParametersAndBody)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.body");
    Method Member(std::make_unique<Function>(*Signature, Name));
    auto *Callable = Member.function();
    ASSERT_NE(Callable, nullptr);
    const Method &ReadOnly = Member;
    ASSERT_EQ(ReadOnly.function(), Callable);
    EXPECT_EQ(Member.functionType(), Callable->functionType());
    EXPECT_EQ(Member.function()->parameter(0), Callable->parameter(0));
    EXPECT_EQ(ReadOnly.function()->parameter(0), Callable->parameter(0));
    EXPECT_TRUE(ReadOnly.function()->isDeclaration());
    auto *Entry = Member.function()->createBlock();
    ASSERT_NE(Entry, nullptr);
    EXPECT_FALSE(ReadOnly.function()->isDeclaration());
    EXPECT_EQ(Callable->entryBlock(), Entry);
    EXPECT_EQ(Member.function()->entryBlock(), Entry);
    EXPECT_EQ(ReadOnly.function()->entryBlock(), Entry);
    EXPECT_EQ(Entry->function(), Callable);
    auto *SecondBlock = Callable->createBlock();
    ASSERT_NE(SecondBlock, nullptr);
    EXPECT_EQ(Member.function()->blockCount(), 2U);
    EXPECT_EQ(ReadOnly.function()->block(1), SecondBlock);
  }

  // A null owner and an owner moved into another method both permit safe empty-function queries.
  TEST(IRMethodTest, NullAndMovedFromMethodsHaveNoFunction)
  {
    Method Empty(nullptr);
    expectNoFunction(Empty);
    Method MovedEmpty(std::move(Empty));
    expectNoFunction(Empty);
    expectNoFunction(MovedEmpty);
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMethodTest.moved");
    Method Member(std::make_unique<Function>(*Signature, Name));
    Method Moved(std::move(Member));
    expectNoFunction(Member);
    EXPECT_TRUE(Moved.valid());
    Moved = std::move(MovedEmpty);
    expectNoFunction(Moved);
    expectNoFunction(MovedEmpty);
  }
} // namespace ink::ir::test
