#include "ink/core/name_pool.h"
#include "ink/ir/constant/constant.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/nominal_type.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    std::array<NominalType *, 3> createMetadataTypes(TypeContext &Types, core::Name Name = core::NAME_None)
    {
      return {Types.createClassType(8, 8, Name), Types.createInterfaceType(8, 8, Name), Types.createStructType(8, 8, Name)};
    }

    class MethodBodyInstruction final : public Instruction
    {
      public:
        explicit MethodBodyInstruction(bool &Destroyed) noexcept
            : Destroyed(Destroyed)
        {
        }

        ~MethodBodyInstruction() noexcept override
        {
          Destroyed = true;
        }

      private:
        bool &Destroyed;
    };
  } // namespace

  static_assert(std::is_abstract_v<NominalType>);
  static_assert(std::is_base_of_v<Type, NominalType>);
  static_assert(std::is_base_of_v<NominalType, ClassType>);
  static_assert(std::is_base_of_v<NominalType, InterfaceType>);
  static_assert(std::is_base_of_v<NominalType, StructType>);
  static_assert(!std::is_copy_constructible_v<ClassType>);
  static_assert(!std::is_copy_assignable_v<ClassType>);
  static_assert(!std::is_move_constructible_v<ClassType>);
  static_assert(!std::is_move_assignable_v<ClassType>);
  static_assert(!std::is_copy_constructible_v<InterfaceType>);
  static_assert(!std::is_move_constructible_v<InterfaceType>);
  static_assert(!std::is_copy_constructible_v<StructType>);
  static_assert(!std::is_move_constructible_v<StructType>);

  // Class, interface and struct metadata lives on the corresponding context-owned type identity.
  TEST(IRNominalMetadataTest, ClassifiesNominalTypesAndStartsWithEmptyMetadata)
  {
    TypeContext Types(Target);
    const auto Values = createMetadataTypes(Types);
    for (std::size_t Index = 0; Index < Values.size(); ++Index)
    {
      const NominalType *Value = Values[Index];
      ASSERT_NE(Value, nullptr);
      EXPECT_TRUE(Types.owns(Value));
      EXPECT_TRUE(NominalType::classof(Value));
      EXPECT_TRUE(Type::classof(Value));
      EXPECT_FALSE(Constant::classof(Value));
      EXPECT_EQ(ClassType::classof(Value), Index == 0);
      EXPECT_EQ(InterfaceType::classof(Value), Index == 1);
      EXPECT_EQ(StructType::classof(Value), Index == 2);
      EXPECT_FALSE(Value->hasName());
      EXPECT_EQ(Value->name(), core::NAME_None);
      EXPECT_TRUE(Value->methods().empty());
      EXPECT_EQ(Value->methodCount(), 0U);
      EXPECT_EQ(Value->method(0), nullptr);
      EXPECT_EQ(Value->method(std::numeric_limits<std::size_t>::max()), nullptr);
      EXPECT_TRUE(Value->findMethods(core::NAME_None).empty());
      EXPECT_FALSE(Value->isFrozen());
    }
    EXPECT_FALSE(NominalType::classof(nullptr));
    EXPECT_FALSE(NominalType::classof(Types.getInt32Type()));
    EXPECT_FALSE(NominalType::classof(Types.getFunctionType(Types.getVoidType())));
  }

  // Names can be completed without changing any nominal layout or type identity.
  TEST(IRNominalMetadataTest, UpdatesNamesBeforeFreezing)
  {
    TypeContext Types(Target);
    const core::Name OriginalName = core::NamePool::instance().intern("IRNominalMetadataTest.Original");
    const core::Name NewName = core::NamePool::instance().intern("IRNominalMetadataTest.Renamed");
    for (NominalType *Value : createMetadataTypes(Types, OriginalName))
    {
      ASSERT_NE(Value, nullptr);
      const auto Pointer = Types.getPointerType(Value);
      const auto Array = Types.getArrayType(Value, 2);
      EXPECT_TRUE(Value->hasName());
      EXPECT_EQ(Value->name(), OriginalName);
      ASSERT_TRUE(Value->setName(NewName));
      EXPECT_EQ(Value->name(), NewName);
      ASSERT_TRUE(Value->setName(core::NAME_None));
      EXPECT_FALSE(Value->hasName());
      EXPECT_EQ(Value->size(), 8U);
      EXPECT_EQ(Value->alignment(), 8U);
      EXPECT_EQ(Types.getPointerType(Value), Pointer);
      EXPECT_EQ(Types.getArrayType(Value, 2), Array);
      ASSERT_NE(Array, nullptr);
      EXPECT_EQ(Array->size(), 16U);
    }
  }

  // Equal names, layouts and member metadata do not merge independently created nominal types.
  TEST(IRNominalMetadataTest, EqualMetadataKeepsDistinctNominalIdentities)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRNominalMetadataTest.Equal");
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const auto First = createMetadataTypes(Types, Name);
    const auto Second = createMetadataTypes(Types, Name);
    for (std::size_t Index = 0; Index < First.size(); ++Index)
    {
      ASSERT_NE(First[Index], nullptr);
      ASSERT_NE(Second[Index], nullptr);
      ASSERT_TRUE(First[Index]->addMethod(Method(MemberName, *Signature)));
      ASSERT_TRUE(Second[Index]->addMethod(Method(MemberName, *Signature)));
      EXPECT_NE(First[Index], Second[Index]);
      EXPECT_NE(Types.getPointerType(First[Index]), Types.getPointerType(Second[Index]));
      EXPECT_EQ(First[Index]->method(0)->functionType(), Second[Index]->method(0)->functionType());
    }
  }

  // Source-name lookup preserves registration order, including repeated signatures with independent dispatch metadata.
  TEST(IRNominalMetadataTest, StoresMethodOrderAndVirtualMetadataWithoutIdentityRules)
  {
    TypeContext Types(Target);
    const auto FirstSignature = Types.getFunctionType(Types.getVoidType());
    const auto SecondSignature = Types.getFunctionType(Types.getVoidType(), {Types.getInt32Type()});
    ASSERT_NE(FirstSignature, nullptr);
    ASSERT_NE(SecondSignature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const core::Name OtherName = core::NamePool::instance().intern("other");
    for (NominalType *Value : createMetadataTypes(Types))
    {
      ASSERT_NE(Value, nullptr);
      ASSERT_TRUE(Value->addMethod(Method(MemberName, *SecondSignature, {MethodDispatch::Virtual, true, true, 0})));
      ASSERT_TRUE(Value->addMethod(Method(MemberName, *FirstSignature)));
      ASSERT_TRUE(Value->addMethod(Method(MemberName, *SecondSignature)));
      ASSERT_TRUE(Value->addMethod(Method(OtherName, *SecondSignature)));
      EXPECT_EQ(Value->methodCount(), 4U);
      ASSERT_EQ(Value->methods().size(), 4U);
      ASSERT_NE(Value->method(0), nullptr);
      EXPECT_EQ(Value->method(0)->name(), MemberName);
      EXPECT_EQ(Value->method(0)->functionType(), SecondSignature);
      EXPECT_TRUE(Value->method(0)->isVirtual());
      EXPECT_FALSE(Value->method(0)->isAbstract());
      EXPECT_TRUE(Value->method(0)->isOverride());
      EXPECT_TRUE(Value->method(0)->isFinal());
      EXPECT_EQ(Value->method(0)->virtualSlot(), 0U);
      EXPECT_EQ(Value->method(4), nullptr);
      EXPECT_EQ(Value->method(std::numeric_limits<std::size_t>::max()), nullptr);
      const auto Overloads = Value->findMethods(MemberName);
      ASSERT_EQ(Overloads.size(), 3U);
      EXPECT_EQ(Overloads[0], Value->method(0));
      EXPECT_EQ(Overloads[1], Value->method(1));
      EXPECT_EQ(Overloads[2], Value->method(2));
      EXPECT_EQ(Overloads[2]->functionType(), Overloads[0]->functionType());
      EXPECT_FALSE(Overloads[2]->isVirtual());
      const auto OtherMethods = Value->findMethods(OtherName);
      ASSERT_EQ(OtherMethods.size(), 1U);
      EXPECT_EQ(OtherMethods[0], Value->method(3));
      EXPECT_TRUE(Value->findMethods(core::NAME_None).empty());
    }
  }

  // Invalid names, dispatch properties and foreign signatures never enter the method list.
  TEST(IRNominalMetadataTest, RejectsInvalidAndForeignMethods)
  {
    TypeContext Types(Target);
    TypeContext ForeignTypes(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    const auto ForeignSignature = ForeignTypes.getFunctionType(ForeignTypes.getVoidType());
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(ForeignSignature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    for (NominalType *Value : createMetadataTypes(Types))
    {
      ASSERT_NE(Value, nullptr);
      EXPECT_FALSE(Value->addMethod(Method(core::NAME_None, *Signature)));
      EXPECT_FALSE(Value->addMethod(Method(MemberName, *Signature, {MethodDispatch::Direct, true, false, std::nullopt})));
      EXPECT_FALSE(Value->addMethod(Method(MemberName, *ForeignSignature)));
      EXPECT_TRUE(Value->methods().empty());
      ASSERT_TRUE(Value->addMethod(Method(MemberName, *Signature)));
      EXPECT_EQ(Value->methodCount(), 1U);
      EXPECT_EQ(Value->method(0)->functionType(), Signature);
    }
  }

  // Freezing all nominal kinds rejects every metadata mutation while preserving readable members and fixed layouts.
  TEST(IRNominalMetadataTest, FrozenMetadataRejectsMutation)
  {
    TypeContext Types(Target);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRNominalMetadataTest.Frozen");
    const core::Name MemberName = core::NamePool::instance().intern("method");
    for (NominalType *Value : createMetadataTypes(Types, Name))
    {
      ASSERT_NE(Value, nullptr);
      ASSERT_TRUE(Value->addMethod(Method(MemberName, *Signature)));
      Value->freeze();
      Value->freeze();
      EXPECT_TRUE(Value->isFrozen());
      EXPECT_FALSE(Value->setName(core::NAME_None));
      EXPECT_FALSE(Value->addMethod(Method(Name, *Signature)));
      EXPECT_FALSE(Value->replaceMethod(0, Method(Name, *Signature)));
      EXPECT_EQ(Value->name(), Name);
      EXPECT_EQ(Value->methodCount(), 1U);
      ASSERT_EQ(Value->findMethods(MemberName).size(), 1U);
      EXPECT_EQ(Value->findMethods(MemberName)[0]->functionType(), Signature);
      EXPECT_EQ(Value->size(), 8U);
      EXPECT_EQ(Value->alignment(), 8U);
    }
  }

  // Replacing a method can complete virtual slots while invalid replacements preserve the original entry.
  TEST(IRNominalMetadataTest, ReplacesMethodBeforeFreezingAndPreservesEntryOnFailure)
  {
    TypeContext Types(Target);
    TypeContext ForeignTypes(Target);
    auto *Value = Types.createClassType(8, 8);
    const auto Signature = Types.getFunctionType(Types.getVoidType());
    const auto OtherSignature = Types.getFunctionType(Types.getVoidType(), {Types.getInt32Type()});
    const auto ForeignSignature = ForeignTypes.getFunctionType(ForeignTypes.getVoidType());
    ASSERT_NE(Value, nullptr);
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(OtherSignature, nullptr);
    ASSERT_NE(ForeignSignature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const core::Name OtherName = core::NamePool::instance().intern("other");
    ASSERT_TRUE(Value->addMethod(Method(MemberName, *Signature, {MethodDispatch::Virtual, false, false, std::nullopt})));
    ASSERT_TRUE(Value->addMethod(Method(OtherName, *OtherSignature)));
    EXPECT_FALSE(Value->method(0)->virtualSlot().has_value());
    ASSERT_TRUE(Value->replaceMethod(0, Method(MemberName, *Signature, {MethodDispatch::Virtual, false, false, 3})));
    EXPECT_EQ(Value->method(0)->virtualSlot(), 3U);
    EXPECT_FALSE(Value->replaceMethod(2, Method(MemberName, *Signature)));
    EXPECT_FALSE(Value->replaceMethod(std::numeric_limits<std::size_t>::max(), Method(MemberName, *Signature)));
    EXPECT_FALSE(Value->replaceMethod(0, Method(core::NAME_None, *Signature)));
    EXPECT_FALSE(Value->replaceMethod(0, Method(MemberName, *ForeignSignature)));
    EXPECT_FALSE(Value->replaceMethod(0, Method(MemberName, *Signature, {MethodDispatch::Abstract, false, true, 3})));
    EXPECT_EQ(Value->methodCount(), 2U);
    EXPECT_EQ(Value->method(0)->name(), MemberName);
    EXPECT_EQ(Value->method(0)->functionType(), Signature);
    EXPECT_EQ(Value->method(0)->virtualSlot(), 3U);
    ASSERT_TRUE(Value->replaceMethod(1, Method(MemberName, *Signature, {MethodDispatch::Virtual, true, false, 4})));
    EXPECT_EQ(Value->method(1)->name(), MemberName);
    EXPECT_EQ(Value->method(1)->functionType(), Signature);
    EXPECT_EQ(Value->method(1)->virtualSlot(), 4U);
    ASSERT_EQ(Value->findMethods(MemberName).size(), 2U);
    Value->freeze();
    EXPECT_FALSE(Value->replaceMethod(0, Method(MemberName, *Signature, {MethodDispatch::Virtual, false, false, 4})));
    EXPECT_EQ(Value->method(0)->virtualSlot(), 3U);
  }

  // Method metadata remains readable after the function used to supply its signature and the owning module are destroyed.
  TEST(IRNominalMetadataTest, MemberMetadataSurvivesFunctionRenamingAndModuleDestruction)
  {
    TypeContext Types(Target);
    auto *Point = Types.createClassType(8, 8);
    const auto Signature = Types.getFunctionType(Types.getVoidType(), {Types.getInt32Type()});
    ASSERT_NE(Point, nullptr);
    ASSERT_NE(Signature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const core::Name LoweredName = core::NamePool::instance().intern("ink_math_Point_method_i32");
    const core::Name ModuleName = core::NamePool::instance().intern("ink.math");
    bool BodyDestroyed = false;
    {
      Module DefiningModule(ModuleName);
      auto *Callable = DefiningModule.createFunction(*Signature, MemberName);
      ASSERT_NE(Callable, nullptr);
      auto *Block = Callable->createBlock();
      ASSERT_NE(Block, nullptr);
      ASSERT_NE(Block->appendInstruction(std::make_unique<MethodBodyInstruction>(BodyDestroyed)), nullptr);
      ASSERT_TRUE(Point->addMethod(Method(MemberName, *Callable->functionType())));
      Callable->setName(LoweredName);
      const auto Members = Point->findMethods(MemberName);
      ASSERT_EQ(Members.size(), 1U);
      EXPECT_EQ(Members[0]->name(), MemberName);
      EXPECT_EQ(Members[0]->functionType(), Callable->functionType());
      EXPECT_TRUE(Point->findMethods(LoweredName).empty());
      EXPECT_FALSE(BodyDestroyed);
    }
    EXPECT_TRUE(BodyDestroyed);
    const auto Members = Point->findMethods(MemberName);
    ASSERT_EQ(Members.size(), 1U);
    EXPECT_TRUE(Members[0]->valid());
    EXPECT_EQ(Members[0]->functionType(), Signature);
    EXPECT_TRUE(Types.owns(Members[0]->functionType()));
    Point->freeze();
    EXPECT_TRUE(Point->isFrozen());
  }
} // namespace ink::ir::test
