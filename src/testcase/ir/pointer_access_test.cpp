#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <memory>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // A pointer-producing call can feed loads and stores directly without evaluating the call more than once.
  TEST(IRPointerAccessTest, ReusesPointerProducingCallForMemoryAccess)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    Function Producer(*Types.get<ink::ir::FunctionType>(Pointer));
    Function Owner(*Types.get<ink::ir::FunctionType>(Integer));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Address = Builder.createCall(Producer);
    ASSERT_NE(Address, nullptr);
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    auto *Store = Builder.createStore(*Seven, *Address);
    auto *Load = Builder.createLoad(*Integer, *Address);
    ASSERT_NE(Store, nullptr);
    ASSERT_NE(Load, nullptr);
    EXPECT_EQ(Store->pointer(), Address);
    EXPECT_EQ(Load->pointer(), Address);
    EXPECT_EQ(Load->type(), Integer);
    auto *Return = Builder.createReturn(*Load);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Load);
    EXPECT_EQ(Entry->instructionCount(), 4U);
  }

  // Pointer-valued parameters support successive loads while zero-sized non-void pointees remain valid memory objects.
  TEST(IRPointerAccessTest, LoadsNestedPointersAndZeroSizedAggregates)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *Empty = Types.get<ink::ir::ArrayType>(Integer, 0);
    const auto *Units = Types.get<ink::ir::ArrayType>(Types.get<ink::ir::VoidType>(), 3);
    Function Owner(*Types.get<ink::ir::FunctionType>(Integer, {Types.get<ink::ir::PointerType>(Pointer), Types.get<ink::ir::PointerType>(Empty), Types.get<ink::ir::PointerType>(Units)}));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createLoad(*Integer, *Owner.parameter(0)), nullptr);
    auto *LoadedPointer = Builder.createLoad(*Pointer, *Owner.parameter(0));
    ASSERT_NE(LoadedPointer, nullptr);
    auto *LoadedInteger = Builder.createLoad(*Integer, *LoadedPointer);
    ASSERT_NE(LoadedInteger, nullptr);
    EXPECT_EQ(LoadedInteger->pointer(), LoadedPointer);
    const Type *ObjectTypes[] = {
        Empty,
        Units,
    };
    for (std::size_t Index = 0; Index != 2; ++Index)
    {
      auto *Loaded = Builder.createLoad(*ObjectTypes[Index], *Owner.parameter(Index + 1));
      ASSERT_NE(Loaded, nullptr);
      EXPECT_EQ(Loaded->type(), ObjectTypes[Index]);
      EXPECT_EQ(Loaded->type()->size(), 0U);
      EXPECT_NE(Builder.createStore(*Loaded, *Owner.parameter(Index + 1)), nullptr);
    }
    EXPECT_NE(Builder.createReturn(*LoadedInteger), nullptr);
  }

  // Memory accesses reject nonpointers, opaque addresses, detached values and values from a foreign function or context.
  TEST(IRPointerAccessTest, RejectsInvalidAddressesWithoutAppendingInstructions)
  {
    IRContext Context(Target);
    IRContext Foreign(Target);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    const auto *Opaque = Types.get<ink::ir::PointerType>(Types.get<ink::ir::VoidType>());
    const auto *Signature = Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Pointer, Opaque});
    Function Owner(*Signature);
    Function Other(*Signature);
    Function ForeignOwner(*Foreign.typePool().get<ink::ir::FunctionType>(Foreign.typePool().get<ink::ir::VoidType>(), {Foreign.typePool().get<ink::ir::PointerType>(Foreign.typePool().get<ink::ir::IntegerType>(32, true))}));
    Module ModuleValue(Context);
    auto *Entry = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    AllocaInstruction Detached(*Pointer);
    auto *OtherAllocation = OtherEntry->appendInstruction(std::make_unique<AllocaInstruction>(*Pointer));
    GlobalVariable ForeignGlobal(*Foreign.typePool().get<ink::ir::PointerType>(Foreign.typePool().get<ink::ir::IntegerType>(32, true)));
    const auto *Seven = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(7));
    const Value *Rejected[] = {
        Seven,
        Integer,
        Context.constantPool().get<ink::ir::NullPointerConstant>(),
        Owner.parameter(1),
        Other.parameter(0),
        ForeignOwner.parameter(0),
        &Detached,
        OtherAllocation,
        &ForeignGlobal,
        &ModuleValue,
    };
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (const Value *Address : Rejected)
    {
      EXPECT_EQ(Builder.createLoad(*Integer, *Address), nullptr);
      EXPECT_EQ(Builder.createStore(*Seven, *Address), nullptr);
    }
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_NE(Builder.createStore(*Seven, *Owner.parameter(0)), nullptr);
    EXPECT_NE(Builder.createLoad(*Integer, *Owner.parameter(0)), nullptr);
  }

  // Allocation and global addresses can be returned as ordinary pointer values without inserting an address conversion.
  TEST(IRPointerAccessTest, ReturnsAllocationAndGlobalPointersDirectly)
  {
    IRContext Context(Target);
    Module ModuleValue(Context);
    auto &Types = Context.typePool();
    const auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Integer);
    Function LocalOwner(*Types.get<ink::ir::FunctionType>(Pointer));
    Function GlobalOwner(*Types.get<ink::ir::FunctionType>(Pointer));
    auto *LocalEntry = LocalOwner.createBlock();
    auto *GlobalEntry = GlobalOwner.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*LocalEntry));
    auto *Allocation = Builder.createAlloca(*Integer);
    ASSERT_NE(Allocation, nullptr);
    auto *LocalReturn = Builder.createReturn(*Allocation);
    ASSERT_NE(LocalReturn, nullptr);
    EXPECT_EQ(LocalReturn->returnedValue(), Allocation);
    ASSERT_TRUE(Builder.setInsertPoint(*GlobalEntry));
    auto *Global = ModuleValue.createGlobalVariable(*Integer);
    ASSERT_NE(Global, nullptr);
    auto *GlobalReturn = Builder.createReturn(*Global);
    ASSERT_NE(GlobalReturn, nullptr);
    EXPECT_EQ(GlobalReturn->returnedValue(), Global);
  }
} // namespace ink::ir::test
