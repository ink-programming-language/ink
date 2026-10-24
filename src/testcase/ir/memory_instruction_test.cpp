#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <cstddef>
#include <memory>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  static_assert(std::is_final_v<AllocaInstruction> && std::is_base_of_v<Instruction, AllocaInstruction>);
  static_assert(std::is_final_v<LoadInstruction> && std::is_base_of_v<Instruction, LoadInstruction>);
  static_assert(std::is_final_v<StoreInstruction> && std::is_base_of_v<Instruction, StoreInstruction>);

  // Each memory instruction borrows its explicit types and operands and identifies as a nonterminating IR operation.
  TEST(IRMemoryInstructionTest, RecordsTypesOperandsAndInstructionIdentity)
  {
    IRContext Context(Target);
    const auto *Integer = Context.typePool().get<ink::ir::IntegerType>(32, true);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    AllocaInstruction Allocation(*Context.typePool().get<ink::ir::PointerType>(Integer));
    LoadInstruction Load(*Integer, Allocation);
    StoreInstruction Store(*Context.typePool().get<ink::ir::VoidType>(), *Seven, Allocation);
    EXPECT_EQ(Allocation.allocatedType(), Integer);
    EXPECT_EQ(Allocation.type(), Context.typePool().get<ink::ir::PointerType>(Integer));
    EXPECT_TRUE(Allocation.allocatedType()->isMutable());
    EXPECT_FALSE(Allocation.isInitialized());
    EXPECT_FALSE(Load.isMutable());
    EXPECT_EQ(Load.pointer(), &Allocation);
    EXPECT_EQ(Load.type(), Integer);
    EXPECT_EQ(Store.storedValue(), Seven);
    EXPECT_EQ(Store.pointer(), &Allocation);
    EXPECT_EQ(Store.type(), Context.typePool().get<ink::ir::VoidType>());
    const Instruction *Instructions[] = {
        &Allocation,
        &Load,
        &Store,
    };
    const ValueKind Kinds[] = {
        ValueKind::AllocaInstruction,
        ValueKind::LoadInstruction,
        ValueKind::StoreInstruction,
    };
    const char *Names[] = {
        "alloca",
        "load",
        "store",
    };
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      const auto *InstructionValue = Instructions[Index];
      EXPECT_EQ(InstructionValue->kind(), Kinds[Index]);
      EXPECT_EQ(valueKindName(InstructionValue->kind()), Names[Index]);
      EXPECT_EQ(InstructionValue->toString(), Names[Index]);
      EXPECT_TRUE(Instruction::classof(InstructionValue));
      EXPECT_FALSE(InstructionValue->isTerminator());
      EXPECT_FALSE(Constant::classof(InstructionValue));
      EXPECT_FALSE(Type::classof(InstructionValue));
      EXPECT_EQ(AllocaInstruction::classof(InstructionValue), Index == 0);
      EXPECT_EQ(LoadInstruction::classof(InstructionValue), Index == 1);
      EXPECT_EQ(StoreInstruction::classof(InstructionValue), Index == 2);
    }
    EXPECT_FALSE(AllocaInstruction::classof(nullptr));
    EXPECT_FALSE(LoadInstruction::classof(nullptr));
    EXPECT_FALSE(StoreInstruction::classof(nullptr));
    EXPECT_FALSE(AllocaInstruction::classof(Seven));
    EXPECT_FALSE(LoadInstruction::classof(Seven));
    EXPECT_FALSE(StoreInstruction::classof(Seven));
  }

  // A stack slot accepts a parameter initializer and its loaded value can feed arithmetic, a call and a return.
  TEST(IRMemoryInstructionTest, ConnectsLocalStorageWithValueInstructions)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    Function Owner(*Types.get<ink::ir::FunctionType>(Integer, {Integer}));
    Function Callee(*Types.get<ink::ir::FunctionType>(Integer, {Integer}));
    Function UnitConsumer(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Types.get<ink::ir::VoidType>()}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Slot = Builder.createAlloca(*Integer);
    ASSERT_NE(Slot, nullptr);
    EXPECT_EQ(Slot->allocatedType(), Integer);
    auto *Store = Builder.createStore(*Owner.parameter(0), *Slot);
    ASSERT_NE(Store, nullptr);
    auto *Load = Builder.createLoad(*Integer, *Slot);
    ASSERT_NE(Load, nullptr);
    auto *Negation = Builder.createNeg(*Load);
    ASSERT_NE(Negation, nullptr);
    auto *Call = Builder.createCall(Callee, {Argument(*Negation)});
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Builder.createCall(UnitConsumer, {Argument(*Store)}), nullptr);
    auto *Return = Builder.createReturn(*Call);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Store->storedValue(), Owner.parameter(0));
    EXPECT_EQ(Store->pointer(), Slot);
    EXPECT_EQ(Load->pointer(), Slot);
    const Instruction *Expected[] = {
        Slot,
        Store,
        Load,
        Negation,
        Call,
        Return,
    };
    ASSERT_EQ(Entry->instructionCount(), 6U);
    for (std::size_t Index = 0; Index < 6; ++Index)
    {
      EXPECT_EQ(Entry->instruction(Index), Expected[Index]);
      EXPECT_EQ(Expected[Index]->parentBlock(), Entry);
    }
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // Typed pointer slots preserve their declared type when initialized by local null, including slots containing opaque pointer values.
  TEST(IRMemoryInstructionTest, StoresNullIntoTypedPointerSlots)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Pointer = Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, true));
    const auto *Opaque = Types.get<ink::ir::PointerType>(Types.get<ink::ir::VoidType>());
    const auto *Null = Context.constantPool().get<ink::ir::NullPointerConstant>();
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer, Opaque}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (const auto *StoredType : {Pointer, Opaque})
    {
      auto *Slot = Builder.createAlloca(*StoredType);
      ASSERT_NE(Slot, nullptr);
      EXPECT_EQ(Slot->allocatedType(), StoredType);
      EXPECT_EQ(Slot->type(), Types.get<ink::ir::PointerType>(StoredType));
      auto *Store = Builder.createStore(*Null, *Slot);
      ASSERT_NE(Store, nullptr);
      EXPECT_EQ(Store->storedValue(), Null);
      auto *Load = Builder.createLoad(*StoredType, *Slot);
      ASSERT_NE(Load, nullptr);
      EXPECT_EQ(Load->type(), StoredType);
      const std::size_t PreviousCount = Entry->instructionCount();
      EXPECT_EQ(Builder.createStore(*Foreign.constantPool().get<ink::ir::NullPointerConstant>(), *Slot), nullptr);
      EXPECT_EQ(Builder.createLoad(*Types.get<ink::ir::IntegerType>(32, true), *Slot), nullptr);
      EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    }
    auto *IntegerSlot = Builder.createAlloca(*Types.get<ink::ir::IntegerType>(32, true));
    ASSERT_NE(IntegerSlot, nullptr);
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createStore(*Null, *IntegerSlot), nullptr);
    EXPECT_EQ(Builder.createStore(*Owner.parameter(1), *IntegerSlot), nullptr);
    EXPECT_EQ(Builder.createLoad(*Types.get<ink::ir::IntegerType>(32, true), *Null), nullptr);
    EXPECT_EQ(Builder.createStore(*Owner.parameter(0), *Null), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
  }

  // Non-void zero-sized arrays remain addressable while unit, marker and foreign types cannot allocate a local object.
  TEST(IRMemoryInstructionTest, SupportsZeroSizedAggregatesAndRejectsUnsupportedAllocatedTypes)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *EmptyArray = Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(32, true), 0);
    const auto *UnitArray = Types.get<ink::ir::ArrayType>(Types.get<ink::ir::VoidType>(), 2);
    const auto *Empty = Context.constantPool().get<ink::ir::ArrayConstant>(EmptyArray);
    ASSERT_NE(EmptyArray, nullptr);
    ASSERT_NE(UnitArray, nullptr);
    ASSERT_NE(Empty, nullptr);
    EXPECT_EQ(EmptyArray->size(), 0U);
    EXPECT_EQ(UnitArray->size(), 0U);
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *EmptySlot = Builder.createAlloca(*EmptyArray);
    ASSERT_NE(EmptySlot, nullptr);
    EXPECT_NE(Builder.createStore(*Empty, *EmptySlot), nullptr);
    EXPECT_NE(Builder.createLoad(*EmptyArray, *EmptySlot), nullptr);
    auto *UnitSlot = Builder.createAlloca(*UnitArray);
    ASSERT_NE(UnitSlot, nullptr);
    const auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    auto *ArrayAddress = UnitSlot;
    ASSERT_NE(ArrayAddress, nullptr);
    auto *UnitAddress = Builder.createGetElementPtr(*UnitArray, *ArrayAddress, {Zero, Zero});
    ASSERT_NE(UnitAddress, nullptr);
    EXPECT_EQ(UnitAddress->type(), Types.get<ink::ir::PointerType>(Types.get<ink::ir::VoidType>()));
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createLoad(*Types.get<ink::ir::VoidType>(), *UnitAddress), nullptr);
    EXPECT_EQ(Builder.createStore(*Zero, *UnitAddress), nullptr);
    EXPECT_EQ(Builder.createAlloca(*Types.get<ink::ir::VoidType>()), nullptr);
    EXPECT_EQ(Builder.createAlloca(*Types.get<ink::ir::MetaType>()), nullptr);
    EXPECT_EQ(Builder.createAlloca(*Types.get<ink::ir::VariadicType>()), nullptr);
    EXPECT_EQ(Builder.createAlloca(*Foreign.typePool().get<ink::ir::IntegerType>(32, true)), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // Allocation results are directly usable addresses, while loading pointer-valued slots removes exactly one pointer level.
  TEST(IRMemoryInstructionTest, UsesAllocationPointersWithoutAddressConversions)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *PointerPointer = Types.get<ink::ir::PointerType>(Pointer);
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer}));
    Function Consumer(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Slot = Builder.createAlloca(*Integer);
    auto *PointerSlot = Builder.createAlloca(*Pointer);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(PointerSlot, nullptr);
    EXPECT_EQ(Slot->type(), Pointer);
    EXPECT_EQ(Slot->allocatedType(), Integer);
    EXPECT_EQ(PointerSlot->type(), PointerPointer);
    EXPECT_EQ(PointerSlot->allocatedType(), Pointer);
    const auto Name = core::NamePool::instance().intern("IRMemoryInstructionTest.Local");
    EXPECT_FALSE(Slot->hasName());
    Slot->setName(Name);
    EXPECT_TRUE(Slot->hasName());
    EXPECT_EQ(Slot->name(), Name);
    auto *StoreAddress = Builder.createStore(*Slot, *PointerSlot);
    ASSERT_NE(StoreAddress, nullptr);
    EXPECT_EQ(StoreAddress->storedValue(), Slot);
    auto *PointerLoad = Builder.createLoad(*Pointer, *PointerSlot);
    ASSERT_NE(PointerLoad, nullptr);
    EXPECT_EQ(PointerLoad->type(), Pointer);
    auto *Load = Builder.createLoad(*Integer, *PointerLoad);
    ASSERT_NE(Load, nullptr);
    EXPECT_EQ(Load->type(), Integer);
    EXPECT_EQ(Load->pointer(), PointerLoad);
    auto *Call = Builder.createCall(Consumer, {Argument(*Slot)});
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->argument(0)->value(), Slot);
    EXPECT_NE(Builder.createICmp(ICmpPredicate::Equal, *Slot, *Owner.parameter(0)), nullptr);
    const auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    auto *ElementAddress = Builder.createGetElementPtr(*Integer, *Slot, {Zero});
    ASSERT_NE(ElementAddress, nullptr);
    EXPECT_EQ(ElementAddress->type(), Pointer);
    EXPECT_EQ(Builder.createLoad(*Integer, *PointerSlot), nullptr);
    EXPECT_EQ(Builder.createLoad(*Pointer, *Slot), nullptr);
  }

  // Declaration initialization permits one write to immutable local storage while ordinary stores preserve the declaration marker.
  TEST(IRMemoryInstructionTest, InitializesImmutableLocalsOnceAndRejectsAssignment)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    Function Other(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    auto *Entry = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    const auto *ConstInteger = Types.get(Integer, false);
    auto *OtherSlot = static_cast<AllocaInstruction *>(OtherEntry->appendInstruction(std::make_unique<AllocaInstruction>(*Types.get<ink::ir::PointerType>(ConstInteger))));
    AllocaInstruction Detached(*Types.get<ink::ir::PointerType>(ConstInteger));
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Immutable = Builder.createAlloca(*Integer, false);
    auto *Mutable = Builder.createAlloca(*Integer);
    auto *Recorded = Builder.createAlloca(*Integer, false, true);
    ASSERT_NE(Immutable, nullptr);
    ASSERT_NE(Mutable, nullptr);
    ASSERT_NE(Recorded, nullptr);
    const Value &StorageValue = *Immutable;
    EXPECT_EQ(StorageValue.type(), Types.get<ink::ir::PointerType>(ConstInteger));
    EXPECT_EQ(Immutable->allocatedType(), ConstInteger);
    EXPECT_FALSE(StorageValue.isMutable());
    EXPECT_FALSE(Immutable->isInitialized());
    EXPECT_TRUE(Recorded->isInitialized());
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createInitialize(*Context.constantPool().get<ink::ir::BoolConstant>(true), *Immutable), nullptr);
    EXPECT_EQ(Builder.createInitialize(*Seven, *OtherSlot), nullptr);
    EXPECT_EQ(Builder.createInitialize(*Seven, Detached), nullptr);
    EXPECT_EQ(Builder.createInitialize(*Seven, *Recorded), nullptr);
    EXPECT_EQ(Builder.createStore(*Seven, *Immutable), nullptr);
    EXPECT_EQ(Builder.createLoad(*Integer, *OtherSlot), nullptr);
    EXPECT_EQ(Builder.createLoad(*Integer, Detached), nullptr);
    EXPECT_FALSE(Immutable->isInitialized());
    EXPECT_FALSE(OtherSlot->isInitialized());
    EXPECT_FALSE(Detached.isInitialized());
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    auto *Initialization = Builder.createInitialize(*Seven, *Immutable);
    ASSERT_NE(Initialization, nullptr);
    EXPECT_EQ(Initialization->storedValue(), Seven);
    EXPECT_EQ(Initialization->pointer(), Immutable);
    EXPECT_TRUE(Immutable->isInitialized());
    EXPECT_FALSE(StorageValue.isMutable());
    const std::size_t InitializedCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createInitialize(*Seven, *Immutable), nullptr);
    EXPECT_EQ(Builder.createStore(*Seven, *Immutable), nullptr);
    EXPECT_EQ(Entry->instructionCount(), InitializedCount);
    EXPECT_NE(Builder.createLoad(*Integer, *Immutable), nullptr);
    EXPECT_NE(Builder.createStore(*Seven, *Mutable), nullptr);
    EXPECT_FALSE(Mutable->isInitialized());
    EXPECT_NE(Builder.createInitialize(*Seven, *Mutable), nullptr);
    EXPECT_TRUE(Mutable->isInitialized());
    EXPECT_NE(Builder.createStore(*Seven, *Mutable), nullptr);
  }

  // Address values cannot masquerade as numeric contents, booleans, function values or GEP indices without a correctly typed load.
  TEST(IRMemoryInstructionTest, RequiresExplicitLoadBeforeValueOperations)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Signature = Types.get<ink::ir::FunctionType>(Integer, {Integer});
    Function Owner(*Types.get<ink::ir::FunctionType>(Integer));
    Function Callee(*Signature);
    auto *Entry = Owner.createBlock();
    auto *TargetBlock = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *IntegerSlot = Builder.createAlloca(*Integer);
    auto *BoolSlot = Builder.createAlloca(*Types.get<ink::ir::BoolType>());
    auto *CalleeSlot = Builder.createAlloca(*Types.get<ink::ir::PointerType>(Signature));
    ASSERT_NE(IntegerSlot, nullptr);
    ASSERT_NE(BoolSlot, nullptr);
    ASSERT_NE(CalleeSlot, nullptr);
    auto *Address = IntegerSlot;
    ASSERT_NE(Address, nullptr);
    const auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createNeg(*IntegerSlot), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *IntegerSlot, *Zero), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Zero, *IntegerSlot), nullptr);
    EXPECT_EQ(Builder.createCall(Callee, {Argument(*IntegerSlot)}), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, *CalleeSlot, {Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createStore(*IntegerSlot, *IntegerSlot), nullptr);
    EXPECT_EQ(Builder.createInitialize(*IntegerSlot, *IntegerSlot), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Address, {IntegerSlot}), nullptr);
    EXPECT_EQ(Builder.createConditionalBranch(*BoolSlot, *TargetBlock, *TargetBlock), nullptr);
    EXPECT_EQ(Builder.createReturn(*IntegerSlot), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    EXPECT_FALSE(IntegerSlot->isInitialized());
    auto *Loaded = Builder.createLoad(*Integer, *IntegerSlot);
    ASSERT_NE(Loaded, nullptr);
    EXPECT_NE(Builder.createReturn(*Loaded), nullptr);
  }

  // Globals are ordinary address operands while const pointees and initializer state remain independent of the pointer value.
  TEST(IRMemoryInstructionTest, UsesGlobalPointersForLoadsStoresCallsAndGEP)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *ConstInteger = Types.get(Integer, false);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    const auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    GlobalVariable Mutable(*Pointer);
    GlobalVariable Immutable(*Types.get<ink::ir::PointerType>(ConstInteger));
    GlobalVariable PointerSlot(*Types.get<ink::ir::PointerType>(Pointer));
    GlobalVariable ForeignStorage(*Foreign.typePool().get<ink::ir::PointerType>(Foreign.typePool().get<ink::ir::IntegerType>(32, true)));
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    Function Consumer(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Immutable.type()->pointeeType(), ConstInteger);
    EXPECT_FALSE(Immutable.variableType()->isMutable());
    EXPECT_FALSE(Immutable.isInitialized());
    ASSERT_TRUE(Immutable.setInitializer(Seven));
    EXPECT_TRUE(Immutable.isInitialized());
    EXPECT_TRUE(Mutable.variableType()->isMutable());
    EXPECT_NE(Builder.createLoad(*Integer, Immutable), nullptr);
    EXPECT_NE(Builder.createStore(*Seven, Mutable), nullptr);
    EXPECT_FALSE(Mutable.isInitialized());
    EXPECT_NE(Builder.createStore(Mutable, PointerSlot), nullptr);
    EXPECT_NE(Builder.createLoad(*Pointer, PointerSlot), nullptr);
    EXPECT_NE(Builder.createCall(Consumer, {Argument(Mutable)}), nullptr);
    EXPECT_NE(Builder.createICmp(ICmpPredicate::Equal, Mutable, Mutable), nullptr);
    EXPECT_NE(Builder.createGetElementPtr(*Integer, Mutable, {Zero}), nullptr);
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createStore(*Seven, Immutable), nullptr);
    EXPECT_EQ(Builder.createNeg(Immutable), nullptr);
    EXPECT_EQ(Builder.createLoad(*Integer, PointerSlot), nullptr);
    EXPECT_EQ(Builder.createLoad(*Integer, ForeignStorage), nullptr);
    EXPECT_EQ(Builder.createCall(Consumer, {Argument(ForeignStorage)}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
  }

  // Direct const types and immutable allocation requests both preserve write restrictions through derived pointers.
  TEST(IRMemoryInstructionTest, PreservesConstPointeesThroughDerivedPointers)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *ConstInteger = Types.get(Integer, false);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    const auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (auto *Slot : {Builder.createAlloca(*Integer, false), Builder.createAlloca(*ConstInteger)})
    {
      ASSERT_NE(Slot, nullptr);
      EXPECT_EQ(Slot->allocatedType(), ConstInteger);
      EXPECT_EQ(Slot->type(), Types.get<ink::ir::PointerType>(ConstInteger));
      EXPECT_FALSE(Slot->allocatedType()->isMutable());
      ASSERT_NE(Builder.createInitialize(*Seven, *Slot), nullptr);
      auto *Selected = Builder.createGetElementPtr(*ConstInteger, *Slot, {Zero});
      ASSERT_NE(Selected, nullptr);
      EXPECT_EQ(Selected->type(), Slot->type());
      auto *Load = Builder.createLoad(*ConstInteger, *Selected);
      ASSERT_NE(Load, nullptr);
      EXPECT_EQ(Load->type(), Integer);
      const auto Count = Entry->instructionCount();
      EXPECT_EQ(Builder.createStore(*Seven, *Slot), nullptr);
      EXPECT_EQ(Builder.createStore(*Seven, *Selected), nullptr);
      EXPECT_EQ(Builder.createInitialize(*Seven, *Slot), nullptr);
      EXPECT_EQ(Entry->instructionCount(), Count);
    }
  }

  // Loading a const pointer slot removes that slot qualifier while preserving separate pointee permissions at deeper levels.
  TEST(IRMemoryInstructionTest, KeepsPointerAndPointeePermissionsIndependentAcrossLoads)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *ConstInteger = Types.get(Integer, false);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *ConstPointer = Types.get(Pointer, false);
    const auto *ReadonlyPointer = Types.get<ink::ir::PointerType>(ConstInteger);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer, Types.get<ink::ir::PointerType>(ConstPointer), Types.get<ink::ir::PointerType>(ReadonlyPointer)}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *FixedPointer = Builder.createAlloca(*Pointer, false);
    ASSERT_NE(FixedPointer, nullptr);
    ASSERT_NE(Builder.createInitialize(*Owner.parameter(0), *FixedPointer), nullptr);
    EXPECT_EQ(FixedPointer->allocatedType(), ConstPointer);
    EXPECT_EQ(FixedPointer->type(), Types.get<ink::ir::PointerType>(ConstPointer));
    auto *LoadedFixedPointer = Builder.createLoad(*ConstPointer, *FixedPointer);
    ASSERT_NE(LoadedFixedPointer, nullptr);
    EXPECT_EQ(LoadedFixedPointer->type(), Pointer);
    EXPECT_NE(Builder.createStore(*Seven, *LoadedFixedPointer), nullptr);
    EXPECT_EQ(Builder.createStore(*Owner.parameter(0), *FixedPointer), nullptr);

    EXPECT_EQ(Builder.createStore(*Owner.parameter(0), *Owner.parameter(1)), nullptr);
    auto *LoadedPointer = Builder.createLoad(*ConstPointer, *Owner.parameter(1));
    ASSERT_NE(LoadedPointer, nullptr);
    EXPECT_EQ(LoadedPointer->type(), Pointer);
    EXPECT_NE(Builder.createStore(*Seven, *LoadedPointer), nullptr);

    auto *LoadedReadonlyPointer = Builder.createLoad(*ReadonlyPointer, *Owner.parameter(2));
    ASSERT_NE(LoadedReadonlyPointer, nullptr);
    EXPECT_EQ(LoadedReadonlyPointer->type(), ReadonlyPointer);
    EXPECT_NE(Builder.createStore(*LoadedReadonlyPointer, *Owner.parameter(2)), nullptr);
    EXPECT_EQ(Builder.createStore(*Seven, *LoadedReadonlyPointer), nullptr);
    EXPECT_EQ(Builder.createStore(*Owner.parameter(0), *Owner.parameter(2)), nullptr);
  }

  // Selecting members through a const aggregate yields const member storage without making the members' pointer targets const.
  TEST(IRMemoryInstructionTest, PropagatesAggregateConstToSelectedStorageOnly)
  {
    IRContext Context(Target);
    Module ModuleValue(Context);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *ConstPointer = Types.get(Pointer, false);
    const auto *ConstArray = Types.get(Types.get<ink::ir::ArrayType>(Pointer, 2), false);
    auto *Record = ModuleValue.createStructType(8, 8);
    ASSERT_NE(Record, nullptr);
    ASSERT_TRUE(Record->addField(Field(Pointer, core::NamePool::instance().intern("Pointer"), 0)));
    const auto *ConstRecord = Types.get(Record, false);
    ASSERT_NE(ConstRecord, nullptr);
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Types.get<ink::ir::PointerType>(ConstArray), Types.get<ink::ir::PointerType>(ConstRecord)}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    const auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    const Type *Aggregates[] = {
        ConstArray,
        ConstRecord,
    };
    for (std::size_t Index = 0; Index != 2; ++Index)
    {
      auto *Address = Builder.createGetElementPtr(*Aggregates[Index], *Owner.parameter(Index), {Zero, Zero});
      ASSERT_NE(Address, nullptr);
      ASSERT_EQ(Address->type(), Types.get<ink::ir::PointerType>(ConstPointer));
      EXPECT_FALSE(static_cast<const PointerType *>(Address->type())->pointeeType()->isMutable());
      auto *Loaded = Builder.createLoad(*ConstPointer, *Address);
      ASSERT_NE(Loaded, nullptr);
      EXPECT_EQ(Loaded->type(), Pointer);
      EXPECT_NE(Builder.createStore(*Seven, *Loaded), nullptr);
      EXPECT_EQ(Builder.createStore(*Loaded, *Address), nullptr);
    }
  }

  // Memory creation enforces the same insertion-point and unterminated-block preconditions as existing builder operations.
  TEST(IRMemoryInstructionTest, RequiresValidUnterminatedInsertionPoint)
  {
#if !INK_ENABLE_ASSERTIONS
    GTEST_SKIP() << "Insertion-state checks require enabled assertions.";
#else
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Integer, Pointer}));
    AllocaInstruction Detached(*Pointer);
    IRBuilder Builder(Context);
    EXPECT_DEATH(static_cast<void>(Builder.createAlloca(*Integer)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createInitialize(*Owner.parameter(0), Detached)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(*Integer, *Owner.parameter(1))), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Owner.parameter(0), *Owner.parameter(1))), "check failed:");
    auto *Entry = Owner.createBlock();
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    ASSERT_NE(Builder.createReturn(), nullptr);
    EXPECT_DEATH(static_cast<void>(Builder.createAlloca(*Integer)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createInitialize(*Owner.parameter(0), Detached)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(*Integer, *Owner.parameter(1))), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Owner.parameter(0), *Owner.parameter(1))), "check failed:");
    EXPECT_EQ(Entry->instructionCount(), 1U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
#endif
  }
} // namespace ink::ir::test
