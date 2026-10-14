#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction.h"
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

  // Class, interface and struct metadata lives on the corresponding module-owned type identity.
  TEST(IRNominalMetadataTest, ClassifiesNominalTypesAndStartsWithEmptyMetadata)
  {
    Module Types(Target);
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
    EXPECT_FALSE(NominalType::classof(Types.typePool().getInt32Type()));
    EXPECT_FALSE(NominalType::classof(Types.typePool().getFunctionType(Types.typePool().getVoidType())));
  }

  // Names can be completed without changing any nominal layout or type identity.
  TEST(IRNominalMetadataTest, UpdatesNamesBeforeFreezing)
  {
    Module Types(Target);
    const core::Name OriginalName = core::NamePool::instance().intern("IRNominalMetadataTest.Original");
    const core::Name NewName = core::NamePool::instance().intern("IRNominalMetadataTest.Renamed");
    for (NominalType *Value : createMetadataTypes(Types, OriginalName))
    {
      ASSERT_NE(Value, nullptr);
      const auto Pointer = Types.typePool().getPointerType(Value);
      const auto Array = Types.typePool().getArrayType(Value, 2);
      EXPECT_TRUE(Value->hasName());
      EXPECT_EQ(Value->name(), OriginalName);
      ASSERT_TRUE(Value->setName(NewName));
      EXPECT_EQ(Value->name(), NewName);
      ASSERT_TRUE(Value->setName(core::NAME_None));
      EXPECT_FALSE(Value->hasName());
      EXPECT_EQ(Value->size(), 8U);
      EXPECT_EQ(Value->alignment(), 8U);
      EXPECT_EQ(Types.typePool().getPointerType(Value), Pointer);
      EXPECT_EQ(Types.typePool().getArrayType(Value, 2), Array);
      ASSERT_NE(Array, nullptr);
      EXPECT_EQ(Array->size(), 16U);
    }
  }

  // Equal names, layouts and member metadata do not merge independently created nominal types.
  TEST(IRNominalMetadataTest, EqualMetadataKeepsDistinctNominalIdentities)
  {
    Module Types(Target);
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType());
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
      EXPECT_NE(Types.typePool().getPointerType(First[Index]), Types.typePool().getPointerType(Second[Index]));
      EXPECT_EQ(First[Index]->method(0)->functionType(), Second[Index]->method(0)->functionType());
      EXPECT_NE(First[Index]->method(0)->function(), Second[Index]->method(0)->function());
    }
  }

  // Function-name lookup preserves registration order for independently owned functions with equal signatures.
  TEST(IRNominalMetadataTest, StoresMethodOrderAndVirtualMetadataWithoutIdentityRules)
  {
    Module Types(Target);
    const auto FirstSignature = Types.typePool().getFunctionType(Types.typePool().getVoidType());
    const auto SecondSignature = Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getInt32Type()});
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
    Module Types(Target);
    Module ForeignTypes(Target);
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType());
    const auto ForeignSignature = ForeignTypes.typePool().getFunctionType(ForeignTypes.typePool().getVoidType());
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
    Module Types(Target);
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType());
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

  // Successful replacement destroys the previous body, while rejected replacements preserve both owners.
  TEST(IRNominalMetadataTest, ReplacesMethodBeforeFreezingAndPreservesEntryOnFailure)
  {
    bool OriginalBodyDestroyed = false;
    bool ReplacementBodyDestroyed = false;
    Module Types(Target);
    Module ForeignTypes(Target);
    auto *Value = Types.createClassType(8, 8);
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType());
    const auto OtherSignature = Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getInt32Type()});
    const auto ForeignSignature = ForeignTypes.typePool().getFunctionType(ForeignTypes.typePool().getVoidType());
    ASSERT_NE(Value, nullptr);
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(OtherSignature, nullptr);
    ASSERT_NE(ForeignSignature, nullptr);
    const core::Name MemberName = core::NamePool::instance().intern("method");
    const core::Name OtherName = core::NamePool::instance().intern("other");
    auto OriginalCallable = std::make_unique<Function>(*Signature, MemberName);
    auto *OriginalBlock = OriginalCallable->createBlock();
    ASSERT_NE(OriginalBlock, nullptr);
    ASSERT_NE(OriginalBlock->appendInstruction(std::make_unique<MethodBodyInstruction>(OriginalBodyDestroyed)), nullptr);
    ASSERT_TRUE(Value->addMethod(Method(std::move(OriginalCallable), {MethodDispatch::Virtual, false, false, std::nullopt})));
    ASSERT_TRUE(Value->addMethod(createMethod(*OtherSignature, OtherName)));
    EXPECT_FALSE(Value->method(0)->virtualSlot().has_value());
    Method Replacement = createMethod(*Signature, MemberName, {MethodDispatch::Virtual, false, false, 3});
    auto *ReplacementCallable = Replacement.function();
    auto *ReplacementBlock = ReplacementCallable->createBlock();
    ASSERT_NE(ReplacementBlock, nullptr);
    ASSERT_NE(ReplacementBlock->appendInstruction(std::make_unique<MethodBodyInstruction>(ReplacementBodyDestroyed)), nullptr);
    EXPECT_FALSE(OriginalBodyDestroyed);
    ASSERT_TRUE(Value->replaceMethod(0, std::move(Replacement)));
    EXPECT_EQ(Replacement.function(), nullptr);
    EXPECT_TRUE(OriginalBodyDestroyed);
    EXPECT_FALSE(ReplacementBodyDestroyed);
    EXPECT_EQ(Value->method(0)->virtualSlot(), 3U);
    Method OutOfBounds = createMethod(*Signature, MemberName);
    const auto *OutOfBoundsCallable = OutOfBounds.function();
    EXPECT_FALSE(Value->replaceMethod(2, std::move(OutOfBounds)));
    EXPECT_EQ(OutOfBounds.function(), OutOfBoundsCallable);
    EXPECT_FALSE(Value->replaceMethod(std::numeric_limits<std::size_t>::max(), std::move(OutOfBounds)));
    EXPECT_EQ(OutOfBounds.function(), OutOfBoundsCallable);
    std::array<Method, 4> Rejected = {
        Method(nullptr),
        createMethod(*Signature, core::NAME_None),
        createMethod(*ForeignSignature, MemberName),
        createMethod(*Signature, MemberName, {MethodDispatch::Abstract, false, true, 3}),
    };
    for (Method &Entry : Rejected)
    {
      const auto *Callable = Entry.function();
      EXPECT_FALSE(Value->replaceMethod(0, std::move(Entry)));
      EXPECT_EQ(Entry.function(), Callable);
      EXPECT_EQ(Value->method(0)->function(), ReplacementCallable);
      EXPECT_FALSE(ReplacementBodyDestroyed);
    }
    EXPECT_EQ(Value->methodCount(), 2U);
    EXPECT_EQ(Value->method(0)->name(), MemberName);
    EXPECT_EQ(Value->method(0)->function(), ReplacementCallable);
    EXPECT_EQ(Value->method(0)->functionType(), Signature);
    EXPECT_EQ(Value->method(0)->virtualSlot(), 3U);
    ASSERT_TRUE(Value->replaceMethod(1, createMethod(*Signature, MemberName, {MethodDispatch::Virtual, true, false, 4})));
    EXPECT_EQ(Value->method(1)->name(), MemberName);
    EXPECT_NE(Value->method(1)->function(), ReplacementCallable);
    EXPECT_EQ(Value->method(1)->functionType(), Signature);
    EXPECT_EQ(Value->method(1)->virtualSlot(), 4U);
    ASSERT_EQ(Value->findMethods(MemberName).size(), 2U);
    Value->freeze();
    EXPECT_FALSE(Value->replaceMethod(0, std::move(OutOfBounds)));
    EXPECT_EQ(OutOfBounds.function(), OutOfBoundsCallable);
    EXPECT_EQ(Value->method(0)->function(), ReplacementCallable);
    EXPECT_FALSE(ReplacementBodyDestroyed);
    EXPECT_EQ(Value->method(0)->virtualSlot(), 3U);
  }

  // All nominal kinds resolve names from their bound functions even after the method list is frozen.
  TEST(IRNominalMetadataTest, MethodLookupTracksFunctionRenamingBeforeAndAfterFreezing)
  {
    Module Types(Target);
    const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType());
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

  // Method storage growth preserves function, parameter and body addresses until the owning module is destroyed.
  TEST(IRNominalMetadataTest, OwnsMethodBodiesAtStableAddressesUntilModuleDestruction)
  {
    std::array<bool, 3> BodiesDestroyed = {};
    {
      Module Types(Target);
      const auto Values = createMetadataTypes(Types);
      const auto Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getInt32Type()});
      ASSERT_NE(Signature, nullptr);
      const core::Name MemberName = core::NamePool::instance().intern("method");
      for (std::size_t Index = 0; Index < Values.size(); ++Index)
      {
        NominalType *Value = Values[Index];
        ASSERT_NE(Value, nullptr);
        Method Entry = createMethod(*Signature, MemberName);
        auto *Callable = Entry.function();
        auto *ParameterValue = Callable->parameter(0);
        auto *Block = Callable->createBlock();
        ASSERT_NE(ParameterValue, nullptr);
        ASSERT_NE(Block, nullptr);
        auto *BodyInstruction = Block->appendInstruction(std::make_unique<MethodBodyInstruction>(BodiesDestroyed[Index]));
        ASSERT_NE(BodyInstruction, nullptr);
        ASSERT_TRUE(Value->addMethod(std::move(Entry)));
        EXPECT_EQ(Entry.function(), nullptr);
        for (std::size_t MemberIndex = 0; MemberIndex < 64; ++MemberIndex)
        {
          ASSERT_TRUE(Value->addMethod(createMethod(*Signature, MemberName)));
        }
        EXPECT_EQ(Value->methodCount(), 65U);
        ASSERT_NE(Value->method(0), nullptr);
        EXPECT_EQ(Value->method(0)->function(), Callable);
        EXPECT_EQ(Value->method(0)->function()->parameter(0), ParameterValue);
        EXPECT_EQ(Value->method(0)->function()->entryBlock(), Block);
        EXPECT_EQ(Value->method(0)->function()->entryBlock()->instruction(0), BodyInstruction);
        EXPECT_EQ(&ParameterValue->function(), Callable);
        EXPECT_EQ(Block->function(), Callable);
        EXPECT_FALSE(BodiesDestroyed[Index]);
        Value->freeze();
      }
    }
    for (bool Destroyed : BodiesDestroyed)
    {
      EXPECT_TRUE(Destroyed);
    }
  }
} // namespace ink::ir::test
