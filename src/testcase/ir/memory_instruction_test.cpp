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
  static_assert(std::is_final_v<LoadInstruction> && std::is_base_of_v<Instruction, LoadInstruction>);
  static_assert(std::is_final_v<StoreInstruction> && std::is_base_of_v<Instruction, StoreInstruction>);

  // Each memory instruction borrows its explicit types and operands and identifies as a nonterminating IR operation.
  TEST(IRMemoryInstructionTest, RecordsTypesOperandsAndInstructionIdentity)
  {
    IRContext Context(Target);
    const auto *Integer = Context.typePool().getInt32Type();
    const auto *Pointer = Context.typePool().getPointerType(Integer);
    const auto *Seven = Context.constantPool().getSignedIntegerConstant(Integer, 7);
    AllocaInstruction Allocation(*Pointer, *Integer);
    LoadInstruction Load(*Integer, Allocation);
    StoreInstruction Store(*Context.typePool().getVoidType(), *Seven, Allocation);
    EXPECT_EQ(Allocation.allocatedType(), Integer);
    EXPECT_EQ(Allocation.type(), Pointer);
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
    EXPECT_EQ(Slot->type(), Types.getPointerType(Integer));
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
      EXPECT_EQ(Slot->type(), Types.getPointerType(StoredType));
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
    auto *UnitAddress = Builder.createGetElementPtr(*UnitArray, *UnitSlot, {Zero, Zero});
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
    auto *OtherSlot = OtherEntry->appendInstruction(std::make_unique<AllocaInstruction>(*Pointer, *Integer));
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
    IRBuilder Builder(Context);
    EXPECT_DEATH(static_cast<void>(Builder.createAlloca(*Integer)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(*Integer, *Owner.parameter(1))), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Owner.parameter(0), *Owner.parameter(1))), "check failed:");
    auto *Entry = Owner.createBlock();
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    ASSERT_NE(Builder.createReturn(), nullptr);
    EXPECT_DEATH(static_cast<void>(Builder.createAlloca(*Integer)), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createLoad(*Integer, *Owner.parameter(1))), "check failed:");
    EXPECT_DEATH(static_cast<void>(Builder.createStore(*Owner.parameter(0), *Owner.parameter(1))), "check failed:");
    EXPECT_EQ(Entry->instructionCount(), 1U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
#endif
  }
} // namespace ink::ir::test
