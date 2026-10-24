#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/type/nominal_type/nominal_type.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <array>
#include <cstddef>
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

    std::array<NominalType *, 3> createMetadataTypes(Module &Types, core::Name Name = core::NAME_None)
    {
      return {Types.createClassType(8, 8, Name), Types.createInterfaceType(8, 8, Name), Types.createStructType(8, 8, Name)};
    }

    Method createMethod(const FunctionType &Signature, core::Name Name, MethodVirtualInfo VirtualInfo = {})
    {
      return Method(std::make_unique<Function>(Signature, Name), VirtualInfo);
    }
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

  // Class, interface and struct metadata lives on the corresponding module-owned type identity.
  TEST(IRNominalMetadataTest, ClassifiesNominalTypesAndStartsWithEmptyMetadata)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Values = createMetadataTypes(Types);
    for (std::size_t Index = 0; Index < Values.size(); ++Index)
    {
      const NominalType *Value = Values[Index];
      ASSERT_NE(Value, nullptr);
      EXPECT_TRUE(Types.ownsType(Value));
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
    EXPECT_FALSE(NominalType::classof(Types.typePool().get<ink::ir::IntegerType>(32, true)));
    EXPECT_FALSE(NominalType::classof(Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>())));
  }

  // Names can be completed without changing any nominal layout or type identity.
  TEST(IRNominalMetadataTest, UpdatesNamesBeforeFreezing)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const core::Name OriginalName = core::NamePool::instance().intern("IRNominalMetadataTest.Original");
    const core::Name NewName = core::NamePool::instance().intern("IRNominalMetadataTest.Renamed");
    for (NominalType *Value : createMetadataTypes(Types, OriginalName))
    {
      ASSERT_NE(Value, nullptr);
      const auto Pointer = Types.typePool().get<ink::ir::PointerType>(Value);
      const auto Array = Types.typePool().get<ink::ir::ArrayType>(Value, 2);
      EXPECT_TRUE(Value->hasName());
      EXPECT_EQ(Value->name(), OriginalName);
      ASSERT_TRUE(Value->setName(NewName));
      EXPECT_EQ(Value->name(), NewName);
      ASSERT_TRUE(Value->setName(core::NAME_None));
      EXPECT_FALSE(Value->hasName());
      EXPECT_EQ(Value->size(), 8U);
      EXPECT_EQ(Value->alignment(), 8U);
      EXPECT_EQ(Types.typePool().get<ink::ir::PointerType>(Value), Pointer);
      EXPECT_EQ(Types.typePool().get<ink::ir::ArrayType>(Value, 2), Array);
      ASSERT_NE(Array, nullptr);
      EXPECT_EQ(Array->size(), 16U);
    }
  }

  // Equal names, layouts and member metadata do not merge independently created nominal types.
  TEST(IRNominalMetadataTest, EqualMetadataKeepsDistinctNominalIdentities)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRNominalMetadataTest.Equal");
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const auto First = createMetadataTypes(Types, Name);
    const auto Second = createMetadataTypes(Types, Name);
    for (std::size_t Index = 0; Index < First.size(); ++Index)
    {
      ASSERT_NE(First[Index], nullptr);
      ASSERT_NE(Second[Index], nullptr);
      ASSERT_TRUE(First[Index]->addMethod(createMethod(*Signature, MemberName)));
      ASSERT_TRUE(Second[Index]->addMethod(createMethod(*Signature, MemberName)));
      EXPECT_NE(First[Index], Second[Index]);
      EXPECT_NE(Types.typePool().get<ink::ir::PointerType>(First[Index]), Types.typePool().get<ink::ir::PointerType>(Second[Index]));
      EXPECT_EQ(First[Index]->method(0)->functionType(), Second[Index]->method(0)->functionType());
      EXPECT_NE(First[Index]->method(0)->function(), Second[Index]->method(0)->function());
    }
  }

  // Function-name lookup preserves registration order for independently owned functions with equal signatures.
  TEST(IRNominalMetadataTest, StoresMethodOrderAndVirtualMetadataWithoutIdentityRules)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto FirstSignature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    const auto SecondSignature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    ASSERT_NE(FirstSignature, nullptr);
    ASSERT_NE(SecondSignature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const core::Name OtherName = core::NamePool::instance().intern("other");
    for (NominalType *Value : createMetadataTypes(Types))
    {
      ASSERT_NE(Value, nullptr);
      ASSERT_TRUE(Value->addMethod(createMethod(*SecondSignature, MemberName, {MethodDispatch::Virtual, true, true, 0})));
      ASSERT_TRUE(Value->addMethod(createMethod(*FirstSignature, MemberName)));
      ASSERT_TRUE(Value->addMethod(createMethod(*SecondSignature, MemberName)));
      ASSERT_TRUE(Value->addMethod(createMethod(*SecondSignature, OtherName)));
      EXPECT_EQ(Value->methodCount(), 4U);
      ASSERT_EQ(Value->methods().size(), 4U);
      ASSERT_NE(Value->method(0), nullptr);
      EXPECT_EQ(Value->method(0)->name(), MemberName);
      EXPECT_NE(Value->method(0)->function(), nullptr);
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
      EXPECT_EQ(Overloads[1]->functionType(), FirstSignature);
      EXPECT_NE(Overloads[2]->function(), Overloads[0]->function());
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
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    const auto ForeignSignature = ForeignTypes.typePool().get<ink::ir::FunctionType>(ForeignTypes.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(ForeignSignature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    for (NominalType *Value : createMetadataTypes(Types))
    {
      ASSERT_NE(Value, nullptr);
      std::array<Method, 4> Rejected = {
          Method(nullptr),
          createMethod(*Signature, core::NAME_None),
          createMethod(*Signature, MemberName, {MethodDispatch::Direct, true, false, std::nullopt}),
          createMethod(*ForeignSignature, MemberName),
      };
      for (Method &Entry : Rejected)
      {
        const auto *Callable = Entry.function();
        EXPECT_FALSE(Value->addMethod(std::move(Entry)));
        EXPECT_EQ(Entry.function(), Callable);
      }
      EXPECT_TRUE(Value->methods().empty());
      ASSERT_TRUE(Value->addMethod(createMethod(*Signature, MemberName)));
      EXPECT_EQ(Value->methodCount(), 1U);
      EXPECT_EQ(Value->method(0)->functionType(), Signature);
    }
  }

  // Freezing all nominal kinds rejects every metadata mutation while preserving readable members and fixed layouts.
  TEST(IRNominalMetadataTest, FrozenMetadataRejectsMutation)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRNominalMetadataTest.Frozen");
    const core::Name MemberName = core::NamePool::instance().intern("method");
    for (NominalType *Value : createMetadataTypes(Types, Name))
    {
      ASSERT_NE(Value, nullptr);
      ASSERT_TRUE(Value->addMethod(createMethod(*Signature, MemberName)));
      Method Candidate = createMethod(*Signature, Name);
      const auto *CandidateFunction = Candidate.function();
      const auto *StoredFunction = Value->method(0)->function();
      Value->freeze();
      Value->freeze();
      EXPECT_TRUE(Value->isFrozen());
      EXPECT_FALSE(Value->setName(core::NAME_None));
      EXPECT_FALSE(Value->addMethod(std::move(Candidate)));
      EXPECT_EQ(Candidate.function(), CandidateFunction);
      EXPECT_FALSE(Value->replaceMethod(0, std::move(Candidate)));
      EXPECT_EQ(Candidate.function(), CandidateFunction);
      EXPECT_EQ(Value->method(0)->function(), StoredFunction);
      EXPECT_EQ(Value->name(), Name);
      EXPECT_EQ(Value->methodCount(), 1U);
      ASSERT_EQ(Value->findMethods(MemberName).size(), 1U);
      EXPECT_EQ(Value->findMethods(MemberName)[0]->functionType(), Signature);
      EXPECT_EQ(Value->size(), 8U);
      EXPECT_EQ(Value->alignment(), 8U);
    }
  }

  // All nominal kinds resolve names from their bound functions even after the method list is frozen.
  TEST(IRNominalMetadataTest, MethodLookupTracksFunctionRenamingBeforeAndAfterFreezing)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    const auto Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>());
    ASSERT_NE(Signature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const core::Name RenamedName = core::NamePool::instance().intern("renamed");
    for (NominalType *Value : createMetadataTypes(Types))
    {
      ASSERT_NE(Value, nullptr);
      Method Entry = createMethod(*Signature, MemberName);
      Method OtherEntry = createMethod(*Signature, RenamedName);
      auto *Callable = Entry.function();
      auto *OtherCallable = OtherEntry.function();
      ASSERT_TRUE(Value->addMethod(std::move(Entry)));
      ASSERT_TRUE(Value->addMethod(std::move(OtherEntry)));
      EXPECT_EQ(Entry.function(), nullptr);
      EXPECT_EQ(OtherEntry.function(), nullptr);
      Callable->setName(RenamedName);
      EXPECT_TRUE(Value->findMethods(MemberName).empty());
      const auto RenamedMethods = Value->findMethods(RenamedName);
      ASSERT_EQ(RenamedMethods.size(), 2U);
      EXPECT_EQ(RenamedMethods[0]->function(), Callable);
      EXPECT_EQ(RenamedMethods[1]->function(), OtherCallable);
      EXPECT_EQ(RenamedMethods[0]->name(), RenamedName);
      Value->freeze();
      Callable->setName(core::NAME_None);
      EXPECT_FALSE(Value->method(0)->valid());
      EXPECT_EQ(Value->method(0)->name(), core::NAME_None);
      EXPECT_TRUE(Value->findMethods(core::NAME_None).empty());
      ASSERT_EQ(Value->findMethods(RenamedName).size(), 1U);
      EXPECT_EQ(Value->findMethods(RenamedName)[0]->function(), OtherCallable);
      Callable->setName(MemberName);
      EXPECT_TRUE(Value->method(0)->valid());
      ASSERT_EQ(Value->findMethods(MemberName).size(), 1U);
      EXPECT_EQ(Value->findMethods(MemberName)[0]->function(), Callable);
      EXPECT_TRUE(Value->isFrozen());
    }
  }
} // namespace ink::ir::test
