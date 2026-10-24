#include "ink/core/support/core_define.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/dereference_instruction.h"
#include "ink/ir/module/global_variable.h"

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

  static_assert(std::is_final_v<DereferenceInstruction>);
  static_assert(std::is_base_of_v<Instruction, DereferenceInstruction>);
  static_assert(std::is_nothrow_constructible_v<DereferenceInstruction, const Type &, const Value &>);

  // A dereference borrows one evaluated pointer and identifies mutable object storage without recording declaration initialization.
  TEST(IRDereferenceInstructionTest, RecordsObjectTypePointerAndStorageIdentity)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer}));
    DereferenceInstruction Storage(*Integer, *Owner.parameter(0));
    EXPECT_EQ(Storage.type(), Integer);
    EXPECT_EQ(Storage.pointer(), Owner.parameter(0));
    EXPECT_EQ(Storage.parentBlock(), nullptr);
    EXPECT_TRUE(Storage.isStorage());
    EXPECT_TRUE(Storage.isMutable());
    EXPECT_FALSE(Storage.isInitialized());
    EXPECT_FALSE(Storage.isTerminator());
    EXPECT_EQ(Storage.kind(), ValueKind::DereferenceInstruction);
    EXPECT_EQ(valueKindName(Storage.kind()), "dereference");
    EXPECT_EQ(Storage.toString(), "dereference");
    EXPECT_TRUE(Instruction::classof(&Storage));
    EXPECT_TRUE(DereferenceInstruction::classof(&Storage));
    EXPECT_FALSE(AllocaInstruction::classof(&Storage));
    EXPECT_FALSE(LoadInstruction::classof(&Storage));
    EXPECT_FALSE(AddressOfInstruction::classof(&Storage));
    EXPECT_FALSE(Constant::classof(&Storage));
    EXPECT_FALSE(Type::classof(&Storage));
    EXPECT_FALSE(DereferenceInstruction::classof(nullptr));
    EXPECT_FALSE(DereferenceInstruction::classof(Owner.parameter(0)));
    EXPECT_FALSE(DereferenceInstruction::classof(Integer));
  }

  // Selecting and addressing a pointee emit no target load; explicit loads and stores keep the dereference as their storage operand.
  TEST(IRDereferenceInstructionTest, DelaysTargetReadUntilExplicitLoad)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Storage = Builder.createDereference(*Owner.parameter(0));
    ASSERT_NE(Storage, nullptr);
    EXPECT_EQ(Storage->type(), Integer);
    EXPECT_EQ(Storage->pointer(), Owner.parameter(0));
    EXPECT_EQ(Storage->parentBlock(), Entry);
    EXPECT_EQ(Entry->instructionCount(), 1U);
    auto *Address = Builder.createAddressOf(*Storage);
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->storage(), Storage);
    EXPECT_EQ(Address->type(), Pointer);
    ASSERT_EQ(Entry->instructionCount(), 2U);
    EXPECT_EQ(Entry->instruction(0), Storage);
    EXPECT_EQ(Entry->instruction(1), Address);
    EXPECT_FALSE(LoadInstruction::classof(Entry->instruction(0)));
    EXPECT_FALSE(LoadInstruction::classof(Entry->instruction(1)));
    auto *Load = Builder.createLoad(*Integer, *Storage);
    auto *Store = Builder.createStore(*Seven, *Storage);
    ASSERT_NE(Load, nullptr);
    ASSERT_NE(Store, nullptr);
    EXPECT_EQ(Load->pointer(), Storage);
    EXPECT_EQ(Load->type(), Integer);
    EXPECT_FALSE(Load->isStorage());
    EXPECT_EQ(Store->pointer(), Storage);
    EXPECT_EQ(Store->storedValue(), Seven);
    EXPECT_FALSE(Storage->isInitialized());
    EXPECT_EQ(Entry->instructionCount(), 4U);
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createLoad(*Types.get<ink::ir::BoolType>(), *Storage), nullptr);
    EXPECT_EQ(Builder.createStore(*Context.constantPool().get<ink::ir::BoolConstant>(true), *Storage), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // Addressing, storing through and loading from a call-produced pointer reuse that single call instruction.
  TEST(IRDereferenceInstructionTest, ReusesPointerProducingCallForEveryStorageAccess)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    Function Producer(*Types.get<ink::ir::FunctionType>(Pointer));
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>()));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Call = Builder.createCall(Producer);
    ASSERT_NE(Call, nullptr);
    auto *Storage = Builder.createDereference(*Call);
    ASSERT_NE(Storage, nullptr);
    auto *Address = Builder.createAddressOf(*Storage);
    auto *Store = Builder.createStore(*Seven, *Storage);
    auto *Load = Builder.createLoad(*Integer, *Storage);
    ASSERT_NE(Address, nullptr);
    ASSERT_NE(Store, nullptr);
    ASSERT_NE(Load, nullptr);
    EXPECT_EQ(Storage->pointer(), Call);
    EXPECT_EQ(Address->storage(), Storage);
    EXPECT_EQ(Store->pointer(), Storage);
    EXPECT_EQ(Load->pointer(), Storage);
    const Instruction *Expected[] = {
        Call,
        Storage,
        Address,
        Store,
        Load,
    };
    ASSERT_EQ(Entry->instructionCount(), 5U);
    std::size_t CallCount = 0;
    for (std::size_t Index = 0; Index < 5; ++Index)
    {
      EXPECT_EQ(Entry->instruction(Index), Expected[Index]);
      EXPECT_EQ(Expected[Index]->parentBlock(), Entry);
      CallCount += CallInstruction::classof(Entry->instruction(Index)) ? 1U : 0U;
    }
    EXPECT_EQ(CallCount, 1U);
  }

  // A pointer-valued pointee requires an explicit intermediate load, and arrays retain their complete type even when their layout is empty.
  TEST(IRDereferenceInstructionTest, SupportsNestedPointersAndZeroSizedArrays)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *Array = Types.get<ink::ir::ArrayType>(Integer, 3);
    const auto *EmptyArray = Types.get<ink::ir::ArrayType>(Integer, 0);
    const auto *UnitArray = Types.get<ink::ir::ArrayType>(Types.get<ink::ir::VoidType>(), 2);
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Types.get<ink::ir::PointerType>(Pointer), Types.get<ink::ir::PointerType>(Array), Types.get<ink::ir::PointerType>(EmptyArray), Types.get<ink::ir::PointerType>(UnitArray)}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *PointerStorage = Builder.createDereference(*Owner.parameter(0));
    ASSERT_NE(PointerStorage, nullptr);
    EXPECT_EQ(PointerStorage->type(), Pointer);
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createDereference(*PointerStorage), nullptr);
    EXPECT_EQ(Builder.createLoad(*Integer, *PointerStorage), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    auto *LoadedPointer = Builder.createLoad(*Pointer, *PointerStorage);
    ASSERT_NE(LoadedPointer, nullptr);
    auto *IntegerStorage = Builder.createDereference(*LoadedPointer);
    ASSERT_NE(IntegerStorage, nullptr);
    EXPECT_EQ(IntegerStorage->type(), Integer);
    EXPECT_EQ(IntegerStorage->pointer(), LoadedPointer);
    EXPECT_EQ(LoadedPointer->pointer(), PointerStorage);
    EXPECT_NE(Builder.createLoad(*Integer, *IntegerStorage), nullptr);
    EXPECT_NE(Builder.createStore(*Context.constantPool().get<ink::ir::NullPointerConstant>(), *PointerStorage), nullptr);
    const Type *Arrays[] = {
        Array,
        EmptyArray,
        UnitArray,
    };
    for (std::size_t Index = 0; Index < 3; ++Index)
    {
      auto *Storage = Builder.createDereference(*Owner.parameter(Index + 1));
      ASSERT_NE(Storage, nullptr);
      EXPECT_EQ(Storage->type(), Arrays[Index]);
      auto *Address = Builder.createAddressOf(*Storage);
      auto *Load = Builder.createLoad(*Arrays[Index], *Storage);
      ASSERT_NE(Address, nullptr);
      ASSERT_NE(Load, nullptr);
      EXPECT_EQ(Address->type(), Types.get<ink::ir::PointerType>(Arrays[Index]));
      EXPECT_EQ(Address->storage(), Storage);
      EXPECT_EQ(Load->type(), Arrays[Index]);
      EXPECT_EQ(Load->pointer(), Storage);
    }
    EXPECT_EQ(EmptyArray->size(), 0U);
    EXPECT_EQ(UnitArray->size(), 0U);
  }

  // Dereferenced storage cannot feed arithmetic, comparisons, calls, branches, returns or GEP indices before an explicit load.
  TEST(IRDereferenceInstructionTest, RequiresExplicitLoadsForOrdinaryValueOperands)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Signature = Types.get<ink::ir::FunctionType>(Integer, {Integer});
    const auto *CalleePointer = Types.get<ink::ir::PointerType>(Signature);
    const auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    Function Callee(*Signature);
    Function Owner(*Types.get<ink::ir::FunctionType>(Integer, {Types.get<ink::ir::PointerType>(Integer), Types.get<ink::ir::PointerType>(Types.get<ink::ir::BoolType>()), CalleePointer}));
    auto *Entry = Owner.createBlock();
    auto *TargetBlock = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *IntegerStorage = Builder.createDereference(*Owner.parameter(0));
    auto *BoolStorage = Builder.createDereference(*Owner.parameter(1));
    auto *CalleeStorage = Builder.createDereference(*Owner.parameter(2));
    ASSERT_NE(IntegerStorage, nullptr);
    ASSERT_NE(BoolStorage, nullptr);
    ASSERT_NE(CalleeStorage, nullptr);
    const std::size_t PreviousCount = Entry->instructionCount();
    EXPECT_EQ(Builder.createNeg(*IntegerStorage), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *IntegerStorage, *Zero), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Zero, *IntegerStorage), nullptr);
    EXPECT_EQ(Builder.createCall(Callee, {Argument(*IntegerStorage)}), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, *CalleeStorage, {Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createStore(*IntegerStorage, *IntegerStorage), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {IntegerStorage}), nullptr);
    EXPECT_EQ(Builder.createConditionalBranch(*BoolStorage, *TargetBlock, *TargetBlock), nullptr);
    EXPECT_EQ(Builder.createReturn(*IntegerStorage), nullptr);
    EXPECT_EQ(Entry->instructionCount(), PreviousCount);
    auto *LoadedInteger = Builder.createLoad(*Integer, *IntegerStorage);
    auto *LoadedCallee = Builder.createLoad(*Signature, *CalleeStorage);
    ASSERT_NE(LoadedInteger, nullptr);
    ASSERT_NE(LoadedCallee, nullptr);
    auto *Negation = Builder.createNeg(*LoadedInteger);
    ASSERT_NE(Negation, nullptr);
    auto *Call = Builder.createCall(*Signature, *LoadedCallee, {Argument(*Negation)});
    ASSERT_NE(Call, nullptr);
    EXPECT_NE(Builder.createReturn(*Call), nullptr);
  }

  // Scalar, null, opaque, storage, detached and foreign-function pointers are rejected without appending instructions.
  TEST(IRDereferenceInstructionTest, RejectsInvalidPointersWithoutChangingBlock)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer, Types.get<ink::ir::PointerType>(Types.get<ink::ir::VoidType>())}));
    Function Other(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer}));
    Function ForeignOwner(*Foreign.typePool().get<ink::ir::FunctionType>(Foreign.typePool().get<ink::ir::VoidType>(), {Foreign.typePool().get<ink::ir::PointerType>(Foreign.typePool().get<ink::ir::IntegerType>(32, true))}));
    auto *Entry = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    AllocaInstruction DetachedObject(*Integer);
    AddressOfInstruction DetachedAddress(*Pointer, DetachedObject);
    GlobalVariable GlobalPointer(*Pointer);
    auto *OtherAddress = OtherEntry->appendInstruction(std::make_unique<AddressOfInstruction>(*Pointer, DetachedObject));
    ASSERT_NE(OtherAddress, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *PointerSlot = Builder.createAlloca(*Pointer);
    ASSERT_NE(PointerSlot, nullptr);
    const Value *InvalidPointers[] = {
        Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7)),
        Context.constantPool().get<ink::ir::NullPointerConstant>(),
        Owner.parameter(1),
        PointerSlot,
        &GlobalPointer,
        &DetachedAddress,
        Other.parameter(0),
        OtherAddress,
        ForeignOwner.parameter(0),
        Pointer,
    };
    const std::size_t PreviousCount = Entry->instructionCount();
    for (const auto *PointerValue : InvalidPointers)
    {
      ASSERT_NE(PointerValue, nullptr);
      EXPECT_EQ(Builder.createDereference(*PointerValue), nullptr);
      EXPECT_EQ(Entry->instructionCount(), PreviousCount);
      EXPECT_EQ(Builder.insertBlock(), Entry);
    }
    auto *LoadedPointer = Builder.createLoad(*Pointer, *PointerSlot);
    ASSERT_NE(LoadedPointer, nullptr);
    EXPECT_NE(Builder.createDereference(*LoadedPointer), nullptr);
  }

  // Low-level construction cannot bypass storage ownership, pointer validity, non-opaque pointees or matching object types.
  TEST(IRDereferenceInstructionTest, RejectsMalformedAndUnownedStorageDescriptors)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer, Types.get<ink::ir::PointerType>(Types.get<ink::ir::BoolType>()), Types.get<ink::ir::PointerType>(Types.get<ink::ir::VoidType>())}));
    Function Other(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer}));
    Function ForeignOwner(*Foreign.typePool().get<ink::ir::FunctionType>(Foreign.typePool().get<ink::ir::VoidType>(), {Foreign.typePool().get<ink::ir::PointerType>(Foreign.typePool().get<ink::ir::IntegerType>(32, true))}));
    auto *Entry = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    BasicBlock DetachedBlock;
    AllocaInstruction PointerSlot(*Pointer);
    LoadInstruction DetachedPointer(*Pointer, PointerSlot);
    DereferenceInstruction DetachedStorage(*Integer, *Owner.parameter(0));
    auto *IntegerSlot = Entry->appendInstruction(std::make_unique<AllocaInstruction>(*Integer));
    ASSERT_NE(IntegerSlot, nullptr);
    const Value *InvalidStorage[] = {
        &DetachedStorage,
        DetachedBlock.appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *Owner.parameter(0))),
        OtherEntry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *Owner.parameter(0))),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *Other.parameter(0))),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *Owner.parameter(1))),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *Owner.parameter(2))),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *IntegerSlot)),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, PointerSlot)),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, DetachedPointer)),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *Seven)),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *Context.constantPool().get<ink::ir::NullPointerConstant>())),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Integer, *ForeignOwner.parameter(0))),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Foreign.typePool().get<ink::ir::IntegerType>(32, true), *Owner.parameter(0))),
        Entry->appendInstruction(std::make_unique<DereferenceInstruction>(*Types.get<ink::ir::VoidType>(), *Owner.parameter(2))),
    };
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    const std::size_t PreviousCount = Entry->instructionCount();
    for (const auto *Storage : InvalidStorage)
    {
      ASSERT_NE(Storage, nullptr);
      EXPECT_EQ(Builder.createAddressOf(*Storage), nullptr);
      EXPECT_EQ(Builder.createLoad(*Storage->type(), *Storage), nullptr);
      EXPECT_EQ(Builder.createStore(*Seven, *Storage), nullptr);
      EXPECT_EQ(Entry->instructionCount(), PreviousCount);
      EXPECT_EQ(Builder.insertBlock(), Entry);
    }
  }

  // Dereference creation requires a selected function block without a terminator, matching the other builder operations.
  TEST(IRDereferenceInstructionTest, RequiresValidUnterminatedInsertionPoint)
  {
#if !INK_ENABLE_ASSERTIONS
    GTEST_SKIP() << "Insertion-state checks require enabled assertions.";
#else
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Pointer = Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, true));
    Function Owner(*Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer}));
    IRBuilder Builder(Context);
    EXPECT_DEATH(static_cast<void>(Builder.createDereference(*Owner.parameter(0))), "check failed:");
    auto *Entry = Owner.createBlock();
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    ASSERT_NE(Builder.createReturn(), nullptr);
    EXPECT_DEATH(static_cast<void>(Builder.createDereference(*Owner.parameter(0))), "check failed:");
    EXPECT_EQ(Entry->instructionCount(), 1U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
#endif
  }
} // namespace ink::ir::test
