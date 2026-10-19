#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class DetachedMemoryInstruction final : public Instruction
    {
      public:
        explicit DetachedMemoryInstruction(const Type &ResultType) noexcept
            : Instruction(ValueKind::Instruction, &ResultType)
        {
        }
    };

    class UnpooledMemoryConstant final : public Constant
    {
      public:
        explicit UnpooledMemoryConstant(const Type &ConstantType) noexcept
            : Constant(ValueKind::IntegerConstant, &ConstantType)
        {
        }
    };
  } // namespace

  static_assert(std::is_final_v<AllocaInstruction> && std::is_base_of_v<Instruction, AllocaInstruction>);
  static_assert(std::is_final_v<AddressOfInstruction> && std::is_base_of_v<Instruction, AddressOfInstruction>);
  static_assert(std::is_final_v<LoadInstruction> && std::is_base_of_v<Instruction, LoadInstruction>);
  static_assert(std::is_final_v<StoreInstruction> && std::is_base_of_v<Instruction, StoreInstruction>);

  // Each memory instruction borrows its explicit types and operands and identifies as a nonterminating IR operation.
  TEST(IRMemoryInstructionTest, RecordsTypesOperandsAndInstructionIdentity)
  {
    IRContext Context(Target);
    const auto *Integer = Context.typePool().getInt32Type();
    const auto *Seven = Context.constantPool().getSignedIntegerConstant(Integer, 7);
    AllocaInstruction Allocation(*Integer);
    LoadInstruction Load(*Integer, Allocation);
    StoreInstruction Store(*Context.typePool().getVoidType(), *Seven, Allocation);
    EXPECT_EQ(Allocation.allocatedType(), Integer);
    EXPECT_EQ(Allocation.type(), Integer);
    EXPECT_TRUE(Allocation.isStorage());
    EXPECT_TRUE(Allocation.isMutable());
    EXPECT_FALSE(Allocation.isInitialized());
    EXPECT_FALSE(Load.isStorage());
    EXPECT_FALSE(Load.isMutable());
    EXPECT_TRUE(Load.isInitialized());
    EXPECT_EQ(Load.pointer(), &Allocation);
    EXPECT_EQ(Load.type(), Integer);
    EXPECT_EQ(Store.storedValue(), Seven);
    EXPECT_EQ(Store.pointer(), &Allocation);
    EXPECT_EQ(Store.type(), Context.typePool().getVoidType());
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
    const auto *Integer = Types.getInt32Type();
    Function Owner(*Types.getFunctionType(Integer, {Integer}));
    Function Callee(*Types.getFunctionType(Integer, {Integer}));
    Function UnitConsumer(*Types.getFunctionType(Types.getVoidType(), {Types.getVoidType()}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Slot = Builder.createAlloca(*Integer);
    ASSERT_NE(Slot, nullptr);
    EXPECT_EQ(Slot->type(), Integer);
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
    const auto *Pointer = Types.getPointerType(Types.getInt32Type());
    const auto *Opaque = Types.getPointerType(Types.getVoidType());
    const auto *Null = Context.constantPool().getNullPointerConstant();
    Function Owner(*Types.getFunctionType(Types.getVoidType(), {Pointer, Opaque}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (const auto *StoredType : {Pointer, Opaque})
    {
      auto *Slot = Builder.createAlloca(*StoredType);
      ASSERT_NE(Slot, nullptr);
      EXPECT_EQ(Slot->type(), StoredType);
      auto *Store = Builder.createStore(*Null, *Slot);
      ASSERT_NE(Store, nullptr);
      EXPECT_EQ(Store->storedValue(), Null);
      auto *Load = Builder.createLoad(*StoredType, *Slot);
      ASSERT_NE(Load, nullptr);
      EXPECT_EQ(Load->type(), StoredType);
      const std::size_t PreviousCount = Entry->instructionCount();
      EXPECT_EQ(Builder.createStore(*Foreign.constantPool().getNullPointerConstant(), *Slot), nullptr);
      EXPECT_EQ(Builder.createLoad(*Types.getInt32Type(), *Slot), nullptr);
      EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    }
    auto *IntegerSlot = Builder.createAlloca(*Types.getInt32Type());
    ASSERT_NE(IntegerSlot, nullptr);
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createStore(*Null, *IntegerSlot), nullptr);
    EXPECT_EQ(Builder.createStore(*Owner.parameter(1), *IntegerSlot), nullptr);
    EXPECT_EQ(Builder.createLoad(*Types.getInt32Type(), *Null), nullptr);
    EXPECT_EQ(Builder.createStore(*Owner.parameter(0), *Null), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
  }

  // Non-void zero-sized arrays remain addressable while unit, marker and foreign types cannot allocate a local object.
  TEST(IRMemoryInstructionTest, SupportsZeroSizedAggregatesAndRejectsUnsupportedAllocatedTypes)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *EmptyArray = Types.getArrayType(Types.getInt32Type(), 0);
    const auto *UnitArray = Types.getArrayType(Types.getVoidType(), 2);
    const auto *Empty = Context.constantPool().getArrayConstant(EmptyArray);
    ASSERT_NE(EmptyArray, nullptr);
    ASSERT_NE(UnitArray, nullptr);
    ASSERT_NE(Empty, nullptr);
    EXPECT_EQ(EmptyArray->size(), 0U);
    EXPECT_EQ(UnitArray->size(), 0U);
    Function Owner(*Types.getFunctionType(Types.getVoidType()));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *EmptySlot = Builder.createAlloca(*EmptyArray);
    ASSERT_NE(EmptySlot, nullptr);
    EXPECT_NE(Builder.createStore(*Empty, *EmptySlot), nullptr);
    EXPECT_NE(Builder.createLoad(*EmptyArray, *EmptySlot), nullptr);
    auto *UnitSlot = Builder.createAlloca(*UnitArray);
    ASSERT_NE(UnitSlot, nullptr);
    const auto *Zero = Context.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 0);
    auto *ArrayAddress = Builder.createAddressOf(*UnitSlot);
    ASSERT_NE(ArrayAddress, nullptr);
    auto *UnitAddress = Builder.createGetElementPtr(*UnitArray, *ArrayAddress, {Zero, Zero});
    ASSERT_NE(UnitAddress, nullptr);
    EXPECT_EQ(UnitAddress->type(), Types.getPointerType(Types.getVoidType()));
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createLoad(*Types.getVoidType(), *UnitAddress), nullptr);
    EXPECT_EQ(Builder.createStore(*Zero, *UnitAddress), nullptr);
    EXPECT_EQ(Builder.createAlloca(*Types.getVoidType()), nullptr);
    EXPECT_EQ(Builder.createAlloca(*Types.getMetaType()), nullptr);
    EXPECT_EQ(Builder.createAlloca(*Types.getVariadicType()), nullptr);
    EXPECT_EQ(Builder.createAlloca(*Foreign.typePool().getInt32Type()), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // Invalid addresses and stored values cannot mutate the insertion block or bypass function and context ownership checks.
  TEST(IRMemoryInstructionTest, RejectsForeignDetachedStorageAndMismatchedOperands)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *Pointer = Types.getPointerType(Integer);
    const auto *Opaque = Types.getPointerType(Types.getVoidType());
    Function Owner(*Types.getFunctionType(Types.getVoidType(), {Integer, Pointer, Opaque, {Pointer, true}}));
    Function Other(*Types.getFunctionType(Types.getVoidType(), {Integer, Pointer}));
    Function ForeignOwner(*Foreign.typePool().getFunctionType(Foreign.typePool().getVoidType(), {Foreign.typePool().getPointerType(Foreign.typePool().getInt32Type())}));
    auto *Entry = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    auto *OtherSlot = OtherEntry->appendInstruction(std::make_unique<AllocaInstruction>(*Integer));
    DetachedMemoryInstruction DetachedAddress(*Pointer);
    DetachedMemoryInstruction DetachedValue(*Integer);
    UnpooledMemoryConstant Unpooled(*Integer);
    GlobalVariable GlobalAddress(*Pointer);
    GlobalVariable GlobalValue(*Integer);
    const auto *Seven = Context.constantPool().getSignedIntegerConstant(Integer, 7);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Slot = Builder.createAlloca(*Integer);
    ASSERT_NE(Slot, nullptr);
    auto *Store = Builder.createStore(*Seven, *Slot);
    ASSERT_NE(Store, nullptr);
    const std::size_t PreviousCount = Entry->instructionCount();
    const Value *Addresses[] = {
        Owner.parameter(0),
        Owner.parameter(2),
        Owner.parameter(3),
        Other.parameter(1),
        ForeignOwner.parameter(0),
        OtherSlot,
        &DetachedAddress,
        &GlobalAddress,
        Context.constantPool().getNullPointerConstant(),
        Seven,
        &Owner,
        Entry,
        Store,
    };
    for (const auto *Address : Addresses)
    {
      ASSERT_NE(Address, nullptr);
      EXPECT_EQ(Builder.createLoad(*Integer, *Address), nullptr);
      EXPECT_EQ(Builder.createStore(*Seven, *Address), nullptr);
    }
    const Value *Values[] = {
        Other.parameter(0),
        Context.constantPool().getBoolConstant(true),
        Foreign.constantPool().getSignedIntegerConstant(Foreign.typePool().getInt32Type(), 7),
        &DetachedValue,
        &Unpooled,
        &GlobalValue,
        Store,
    };
    for (const auto *ValueObject : Values)
    {
      ASSERT_NE(ValueObject, nullptr);
      EXPECT_EQ(Builder.createStore(*ValueObject, *Slot), nullptr);
    }
    EXPECT_EQ(Builder.createLoad(*Foreign.typePool().getInt32Type(), *Slot), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    EXPECT_EQ(OtherEntry->instructionCount(), 1U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
    EXPECT_NE(Builder.createLoad(*Integer, *Owner.parameter(1)), nullptr);
    EXPECT_NE(Builder.createStore(*Owner.parameter(0), *Owner.parameter(1)), nullptr);
  }

  // Local storage, its address and its loaded contents retain distinct types and operand categories, including pointer-valued locals.
  TEST(IRMemoryInstructionTest, SeparatesStorageFromAddressAndLoadedValue)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *Pointer = Types.getPointerType(Integer);
    Function Owner(*Types.getFunctionType(Types.getVoidType(), {Pointer}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Slot = Builder.createAlloca(*Integer);
    auto *PointerSlot = Builder.createAlloca(*Pointer);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(PointerSlot, nullptr);
    const auto Name = core::NamePool::instance().intern("IRMemoryInstructionTest.Local");
    EXPECT_FALSE(Slot->hasName());
    Slot->setName(Name);
    EXPECT_TRUE(Slot->hasName());
    EXPECT_EQ(Slot->name(), Name);
    auto *Address = Builder.createAddressOf(*Slot);
    auto *PointerAddress = Builder.createAddressOf(*PointerSlot);
    ASSERT_NE(Address, nullptr);
    ASSERT_NE(PointerAddress, nullptr);
    EXPECT_EQ(Address->storage(), Slot);
    EXPECT_EQ(Address->type(), Pointer);
    EXPECT_EQ(PointerAddress->type(), Types.getPointerType(Pointer));
    EXPECT_FALSE(Address->isStorage());
    EXPECT_FALSE(Address->isMutable());
    EXPECT_TRUE(Address->isInitialized());
    EXPECT_TRUE(Instruction::classof(Address));
    EXPECT_TRUE(AddressOfInstruction::classof(Address));
    EXPECT_FALSE(AddressOfInstruction::classof(Slot));
    EXPECT_FALSE(AddressOfInstruction::classof(nullptr));
    EXPECT_FALSE(Address->isTerminator());
    EXPECT_EQ(valueKindName(Address->kind()), "address_of");
    EXPECT_EQ(Address->parentBlock(), Entry);
    auto *Load = Builder.createLoad(*Integer, *Slot);
    auto *PointerLoad = Builder.createLoad(*Pointer, *PointerSlot);
    ASSERT_NE(Load, nullptr);
    ASSERT_NE(PointerLoad, nullptr);
    EXPECT_EQ(Load->type(), Integer);
    EXPECT_FALSE(Load->isStorage());
    EXPECT_EQ(PointerLoad->type(), Pointer);
    EXPECT_FALSE(PointerLoad->isStorage());
    EXPECT_NE(Builder.createLoad(*Integer, *PointerLoad), nullptr);
    EXPECT_NE(Builder.createLoad(*Pointer, *PointerAddress), nullptr);
    EXPECT_NE(Builder.createStore(*Owner.parameter(0), *PointerSlot), nullptr);
    EXPECT_EQ(Builder.createLoad(*Integer, *PointerSlot), nullptr);
    EXPECT_EQ(Builder.createAddressOf(*Address), nullptr);
    EXPECT_EQ(Builder.createAddressOf(*Load), nullptr);
    EXPECT_EQ(Builder.createAddressOf(*Owner.parameter(0)), nullptr);
    const auto *Zero = Context.constantPool().getSignedIntegerConstant(Integer, 0);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Slot, {Zero}), nullptr);
    EXPECT_NE(Builder.createGetElementPtr(*Integer, *Address, {Zero}), nullptr);
  }

  // Declaration initialization permits one write to immutable local storage while ordinary stores preserve the declaration marker.
  TEST(IRMemoryInstructionTest, InitializesImmutableLocalsOnceAndRejectsAssignment)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *Seven = Context.constantPool().getSignedIntegerConstant(Integer, 7);
    Function Owner(*Types.getFunctionType(Types.getVoidType()));
    Function Other(*Types.getFunctionType(Types.getVoidType()));
    auto *Entry = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    auto *OtherSlot = static_cast<AllocaInstruction *>(OtherEntry->appendInstruction(std::make_unique<AllocaInstruction>(*Integer, false)));
    AllocaInstruction Detached(*Integer, false);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Immutable = Builder.createAlloca(*Integer, false);
    auto *Mutable = Builder.createAlloca(*Integer);
    auto *Recorded = Builder.createAlloca(*Integer, false, true);
    ASSERT_NE(Immutable, nullptr);
    ASSERT_NE(Mutable, nullptr);
    ASSERT_NE(Recorded, nullptr);
    const Value &StorageValue = *Immutable;
    EXPECT_TRUE(StorageValue.isStorage());
    EXPECT_FALSE(StorageValue.isMutable());
    EXPECT_FALSE(StorageValue.isInitialized());
    EXPECT_TRUE(Recorded->isInitialized());
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createInitialize(*Context.constantPool().getBoolConstant(true), *Immutable), nullptr);
    EXPECT_EQ(Builder.createInitialize(*Seven, *OtherSlot), nullptr);
    EXPECT_EQ(Builder.createInitialize(*Seven, Detached), nullptr);
    EXPECT_EQ(Builder.createInitialize(*Seven, *Recorded), nullptr);
    EXPECT_EQ(Builder.createStore(*Seven, *Immutable), nullptr);
    EXPECT_EQ(Builder.createAddressOf(*OtherSlot), nullptr);
    EXPECT_EQ(Builder.createAddressOf(Detached), nullptr);
    EXPECT_FALSE(Immutable->isInitialized());
    EXPECT_FALSE(OtherSlot->isInitialized());
    EXPECT_FALSE(Detached.isInitialized());
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    auto *Initialization = Builder.createInitialize(*Seven, *Immutable);
    ASSERT_NE(Initialization, nullptr);
    EXPECT_EQ(Initialization->storedValue(), Seven);
    EXPECT_EQ(Initialization->pointer(), Immutable);
    EXPECT_TRUE(StorageValue.isInitialized());
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

  // Storage cannot masquerade as a numeric operand, condition, callee, argument, returned value or GEP index merely because its type matches.
  TEST(IRMemoryInstructionTest, RequiresExplicitLoadBeforeValueOperations)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *Signature = Types.getFunctionType(Integer, {Integer});
    Function Owner(*Types.getFunctionType(Integer));
    Function Callee(*Signature);
    auto *Entry = Owner.createBlock();
    auto *TargetBlock = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *IntegerSlot = Builder.createAlloca(*Integer);
    auto *BoolSlot = Builder.createAlloca(*Types.getBoolType());
    auto *CalleeSlot = Builder.createAlloca(*Types.getPointerType(Signature));
    ASSERT_NE(IntegerSlot, nullptr);
    ASSERT_NE(BoolSlot, nullptr);
    ASSERT_NE(CalleeSlot, nullptr);
    auto *Address = Builder.createAddressOf(*IntegerSlot);
    ASSERT_NE(Address, nullptr);
    const auto *Zero = Context.constantPool().getSignedIntegerConstant(Integer, 0);
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

  // Global variables expose storage properties and explicit load/address operations without being usable as their contents.
  TEST(IRMemoryInstructionTest, AccessesGlobalStorageExplicitly)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *Pointer = Types.getPointerType(Integer);
    const auto *Seven = Context.constantPool().getSignedIntegerConstant(Integer, 7);
    GlobalVariable Mutable(*Integer);
    GlobalVariable Immutable(*Integer, core::NAME_None, false);
    GlobalVariable PointerSlot(*Pointer);
    GlobalVariable ForeignStorage(*Foreign.typePool().getInt32Type());
    Function Owner(*Types.getFunctionType(Types.getVoidType()));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    const Value &ImmutableValue = Immutable;
    EXPECT_TRUE(ImmutableValue.isStorage());
    EXPECT_FALSE(ImmutableValue.isMutable());
    EXPECT_FALSE(ImmutableValue.isInitialized());
    ASSERT_TRUE(Immutable.setInitializer(Seven));
    EXPECT_TRUE(ImmutableValue.isInitialized());
    EXPECT_TRUE(Mutable.isMutable());
    EXPECT_NE(Builder.createLoad(*Integer, Immutable), nullptr);
    EXPECT_NE(Builder.createStore(*Seven, Mutable), nullptr);
    EXPECT_FALSE(Mutable.isInitialized());
    auto *Address = Builder.createAddressOf(Immutable);
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->storage(), &Immutable);
    EXPECT_EQ(Address->type(), Pointer);
    EXPECT_FALSE(Address->isStorage());
    auto *PointerAddress = Builder.createAddressOf(PointerSlot);
    ASSERT_NE(PointerAddress, nullptr);
    EXPECT_EQ(PointerAddress->type(), Types.getPointerType(Pointer));
    EXPECT_NE(Builder.createLoad(*Pointer, PointerSlot), nullptr);
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createStore(*Seven, Immutable), nullptr);
    EXPECT_EQ(Builder.createNeg(Immutable), nullptr);
    EXPECT_EQ(Builder.createLoad(*Integer, PointerSlot), nullptr);
    EXPECT_EQ(Builder.createLoad(*Integer, ForeignStorage), nullptr);
    EXPECT_EQ(Builder.createAddressOf(ForeignStorage), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
  }

  // Memory creation enforces the same insertion-point and unterminated-block preconditions as existing builder operations.
  TEST(IRMemoryInstructionTest, RequiresValidUnterminatedInsertionPoint)
  {
#if !INK_ENABLE_ASSERTIONS
    GTEST_SKIP() << "Insertion-state checks require enabled assertions.";
#else
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.getInt32Type();
    const auto *Pointer = Types.getPointerType(Integer);
    Function Owner(*Types.getFunctionType(Types.getVoidType(), {Integer, Pointer}));
    AllocaInstruction Detached(*Integer);
    IRBuilder Builder(Context);
    EXPECT_DEATH(static_cast<void>(Builder.createAlloca(*Integer)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createAddressOf(Detached)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createInitialize(*Owner.parameter(0), Detached)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(*Integer, *Owner.parameter(1))), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Owner.parameter(0), *Owner.parameter(1))), "check failed:");
    auto *Entry = Owner.createBlock();
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    ASSERT_NE(Builder.createReturn(), nullptr);
    EXPECT_DEATH(static_cast<void>(Builder.createAlloca(*Integer)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createAddressOf(Detached)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createInitialize(*Owner.parameter(0), Detached)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(*Integer, *Owner.parameter(1))), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Owner.parameter(0), *Owner.parameter(1))), "check failed:");
    EXPECT_EQ(Entry->instructionCount(), 1U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
#endif
  }
} // namespace ink::ir::test
