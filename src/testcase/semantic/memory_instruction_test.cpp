#include "ink/semantic/model/instruction/alloca_instruction.h"
#include "ink/semantic/model/instruction/load_instruction.h"
#include "ink/semantic/model/instruction/store_instruction.h"
#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <type_traits>

namespace ink::semantic::test
{
  static_assert(std::is_base_of_v<Value, AllocaInstruction>);
  static_assert(std::is_base_of_v<Value, LoadInstruction>);
  static_assert(std::is_base_of_v<Value, StoreInstruction>);
  static_assert(!std::is_copy_constructible_v<AllocaInstruction>);
  static_assert(!std::is_move_constructible_v<LoadInstruction>);
  static_assert(!std::is_copy_constructible_v<StoreInstruction>);

  // Explicit stores and distinct reads form an ordered function body without changing operand parent links.
  TEST(SemanticMemoryInstructionTest, InitializationAssignmentAndReadsPreserveBlockOrder)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    Function *Main = Context.createFunction(Context.namePool().intern("Main"), *Signature);
    ASSERT_NE(Main, nullptr);
    BasicBlock *Entry = Context.createFunctionBody(*Main);
    AllocaInstruction *Slot = Context.createAllocaInstruction(*Int32);
    AllocaInstruction *SecondSlot = Context.createAllocaInstruction(*Int32);
    ASSERT_NE(Entry, nullptr);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(SecondSlot, nullptr);
    EXPECT_NE(Slot, SecondSlot);
    EXPECT_EQ(&Slot->type(), &SecondSlot->type());
    EXPECT_EQ(&Slot->allocatedType(), Int32);
    EXPECT_EQ(static_cast<const PointerType &>(Slot->type()).access(), AccessKind::ReadWrite);
    EXPECT_EQ(Slot->outer(), nullptr);

    const IntegerConstant *Two = Context.getIntegerConstant(*Int32, IntegerBits(32, 2));
    const IntegerConstant *Four = Context.getIntegerConstant(*Int32, IntegerBits(32, 4));
    ASSERT_NE(Two, nullptr);
    ASSERT_NE(Four, nullptr);
    StoreInstruction *Initialize = Context.createStoreInstruction(*Slot, *Two);
    LoadInstruction *FirstRead = Context.createLoadInstruction(*Slot);
    StoreInstruction *Assign = Context.createStoreInstruction(*Slot, *Four);
    LoadInstruction *SecondRead = Context.createLoadInstruction(*Slot);
    ASSERT_NE(Initialize, nullptr);
    ASSERT_NE(FirstRead, nullptr);
    ASSERT_NE(Assign, nullptr);
    ASSERT_NE(SecondRead, nullptr);
    EXPECT_NE(FirstRead, SecondRead);
    EXPECT_EQ(&Initialize->storedValue(), Two);
    EXPECT_EQ(&Assign->storedValue(), Four);
    EXPECT_EQ(&FirstRead->address(), Slot);
    EXPECT_EQ(&SecondRead->address(), Slot);
    EXPECT_EQ(&FirstRead->type(), Int32);
    EXPECT_EQ(&Assign->type(), &Context.getVoidType());

    const Type *Parameters[] = {Int32};
    const FunctionType *PrintType = Context.getFunctionType(Context.getVoidType(), Parameters);
    const Function *Print = Context.createFunction(Context.namePool().intern("Print"), *PrintType);
    ASSERT_NE(Print, nullptr);
    const Value *Arguments[] = {SecondRead};
    CallInstruction *Call = Context.createCallInstruction(*Print, Arguments);
    ASSERT_NE(Call, nullptr);
    Value *Sequence[] = {
        Slot,
        Initialize,
        FirstRead,
        Assign,
        SecondRead,
        Call,
    };
    for (Value *Operation : Sequence)
    {
      ASSERT_TRUE(Context.appendValue(*Entry, *Operation));
      EXPECT_EQ(Operation->outer(), Entry);
    }
    ASSERT_EQ(Entry->values().size(), 6U);
    for (std::size_t Index = 0; Index < Entry->values().size(); ++Index)
    {
      EXPECT_EQ(Entry->values()[Index], Sequence[Index]);
    }
    EXPECT_EQ(Entry->outer(), Main);
    EXPECT_EQ(Two->outer(), nullptr);
    EXPECT_EQ(Four->outer(), nullptr);
    EXPECT_EQ(Print->outer(), nullptr);
    EXPECT_FALSE(Context.appendValue(*Entry, *Initialize));
    ASSERT_TRUE(Context.removeValue(*Entry, *Call));
    EXPECT_EQ(Call->outer(), nullptr);
    EXPECT_EQ(Call->arguments()[0], SecondRead);
    ASSERT_TRUE(Context.appendValue(*Entry, *Call));
    EXPECT_EQ(Entry->values().back(), Call);
  }

  // Scalars, indirect values, slices and nested fixed arrays use canonical writable address types.
  TEST(SemanticMemoryInstructionTest, AllocationsSupportConcreteStorageTypes)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const ArrayType *Row = Context.getArrayType(*Int32, 3);
    const Type *Types[] = {
        &Context.getBoolType(),
        Int32,
        Context.getIntegerType(129, false),
        Context.getFloatType(16),
        Context.getFloatType(32),
        Context.getFloatType(64),
        Context.getPointerType(*Int32, AccessKind::ReadOnly),
        Context.getReferenceType(*Int32, AccessKind::ReadWrite),
        Context.getSliceType(*Int32, AccessKind::ReadOnly),
        Row,
        Context.getArrayType(*Row, 2),
        Context.getArrayType(*Int32, 0),
    };
    for (const Type *TypeValue : Types)
    {
      ASSERT_NE(TypeValue, nullptr);
      AllocaInstruction *Slot = Context.createAllocaInstruction(*TypeValue);
      ASSERT_NE(Slot, nullptr);
      EXPECT_EQ(&Slot->allocatedType(), TypeValue);
      EXPECT_EQ(&Slot->type(), Context.getPointerType(*TypeValue, AccessKind::ReadWrite));
      EXPECT_EQ(&Slot->context(), &Context);
      // Construction validates types; definite initialization is a separate verifier obligation.
      LoadInstruction *Read = Context.createLoadInstruction(*Slot);
      ASSERT_NE(Read, nullptr);
      EXPECT_EQ(&Read->type(), TypeValue);
      const StoreInstruction *Write = Context.createStoreInstruction(*Slot, *Read);
      ASSERT_NE(Write, nullptr);
      EXPECT_EQ(&Write->storedValue(), Read);
      EXPECT_EQ(&Write->type(), &Context.getVoidType());
    }
  }

  // Meta/control types, raw signatures and nominal types without layouts cannot be allocated or dereferenced.
  TEST(SemanticMemoryInstructionTest, UnsupportedStorageTypesAreRejectedAtEveryFactory)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Name NameValue = Context.namePool().intern("Opaque");
    const ArrayType *VoidArray = Context.getArrayType(Context.getVoidType(), 1);
    const Type *Unsupported[] = {
        &Context.getMetaType(),
        &Context.getVoidType(),
        &Context.getLabelType(),
        &Context.getModuleType(),
        Context.getFunctionType(Context.getVoidType()),
        Context.createClassType(NameValue),
        Context.createEnumType(NameValue),
        Context.createInterfaceType(NameValue),
        VoidArray,
        Context.getArrayType(*VoidArray, 2),
    };
    for (const Type *TypeValue : Unsupported)
    {
      ASSERT_NE(TypeValue, nullptr);
      EXPECT_EQ(Context.createAllocaInstruction(*TypeValue), nullptr);
      const PointerType *Pointer = Context.getPointerType(*TypeValue, AccessKind::ReadWrite);
      // Storing an address is allowed even when dereferencing its pointee is not yet supported.
      AllocaInstruction *PointerSlot = Context.createAllocaInstruction(*Pointer);
      ASSERT_NE(PointerSlot, nullptr);
      LoadInstruction *Address = Context.createLoadInstruction(*PointerSlot);
      ASSERT_NE(Address, nullptr);
      EXPECT_EQ(Context.createLoadInstruction(*Address), nullptr);
      EXPECT_EQ(Context.createStoreInstruction(*Address, Context.getBoolConstant(false)), nullptr);
    }
  }

  // Read-only pointees can be loaded but not stored, independently of the pointer slot's own write access.
  TEST(SemanticMemoryInstructionTest, LoadedPointersPreserveReadOnlyAccess)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const PointerType *ReadOnly = Context.getPointerType(*Int32, AccessKind::ReadOnly);
    const PointerType *ReadWrite = Context.getPointerType(*Int32, AccessKind::ReadWrite);
    for (const PointerType *Pointer : {ReadOnly, ReadWrite})
    {
      AllocaInstruction *PointerSlot = Context.createAllocaInstruction(*Pointer);
      ASSERT_NE(PointerSlot, nullptr);
      LoadInstruction *Address = Context.createLoadInstruction(*PointerSlot);
      ASSERT_NE(Address, nullptr);
      LoadInstruction *Read = Context.createLoadInstruction(*Address);
      ASSERT_NE(Read, nullptr);
      EXPECT_EQ(&Read->type(), Int32);
      EXPECT_EQ(&Read->address(), Address);
      const StoreInstruction *Write = Context.createStoreInstruction(*Address, *Read);
      EXPECT_EQ(Write != nullptr, Pointer->access() == AccessKind::ReadWrite);
      EXPECT_NE(Context.createStoreInstruction(*PointerSlot, *Address), nullptr);
    }
    const ReferenceType *Reference = Context.getReferenceType(*Int32, AccessKind::ReadWrite);
    AllocaInstruction *ReferenceSlot = Context.createAllocaInstruction(*Reference);
    ASSERT_NE(ReferenceSlot, nullptr);
    LoadInstruction *ReferenceValue = Context.createLoadInstruction(*ReferenceSlot);
    ASSERT_NE(ReferenceValue, nullptr);
    EXPECT_EQ(Context.createLoadInstruction(*ReferenceValue), nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*ReferenceValue, Context.getBoolConstant(false)), nullptr);
  }

  // Memory operands must be local, pointer-typed and exactly match width, signedness and pointee identity.
  TEST(SemanticMemoryInstructionTest, ForeignOperandsAndTypeMismatchesAreRejected)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const IntegerType *ForeignInt32 = Other.getIntegerType(32, true);
    AllocaInstruction *Slot = Context.createAllocaInstruction(*Int32);
    AllocaInstruction *ForeignSlot = Other.createAllocaInstruction(*ForeignInt32);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(ForeignSlot, nullptr);
    EXPECT_EQ(Context.createAllocaInstruction(*ForeignInt32), nullptr);
    EXPECT_EQ(Context.createLoadInstruction(*ForeignSlot), nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*ForeignSlot, Context.getBoolConstant(false)), nullptr);
    const Value *InvalidValues[] = {
        Context.getIntegerConstant(*Context.getIntegerType(32, false), IntegerBits(32, 2)),
        Context.getIntegerConstant(*Context.getIntegerType(64, true), IntegerBits(64, 2)),
        Other.getIntegerConstant(*ForeignInt32, IntegerBits(32, 2)),
        Context.getFloatConstant(*Context.getFloatType(32), FloatBits(32, 0)),
        &Context.getBoolConstant(false),
        Int32,
        ForeignSlot,
    };
    for (const Value *Invalid : InvalidValues)
    {
      ASSERT_NE(Invalid, nullptr);
      EXPECT_EQ(Context.createStoreInstruction(*Slot, *Invalid), nullptr);
    }
    const IntegerConstant *Two = Context.getIntegerConstant(*Int32, IntegerBits(32, 2));
    ASSERT_NE(Two, nullptr);
    EXPECT_EQ(Context.createLoadInstruction(*Two), nullptr);
    EXPECT_EQ(Context.createLoadInstruction(Slot->type()), nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*Two, *Two), nullptr);
    EXPECT_EQ(Context.createStoreInstruction(Slot->type(), *Two), nullptr);
    StoreInstruction *Store = Context.createStoreInstruction(*Slot, *Two);
    ASSERT_NE(Store, nullptr);
    EXPECT_EQ(Context.createStoreInstruction(*Slot, *Store), nullptr);
    EXPECT_EQ(Context.createLoadInstruction(*Store), nullptr);
    EXPECT_EQ(Slot->outer(), nullptr);
    EXPECT_EQ(&Store->storedValue(), Two);
    EXPECT_NE(Context.createLoadInstruction(*Slot), nullptr);
  }

  // Growing each instruction arena preserves all borrowed address, result and constant references.
  TEST(SemanticMemoryInstructionTest, InstructionOperandsSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    AllocaInstruction *Slot = Context.createAllocaInstruction(Context.getBoolType());
    ASSERT_NE(Slot, nullptr);
    const StoreInstruction *FirstStore = Context.createStoreInstruction(*Slot, Context.getBoolConstant(true));
    const LoadInstruction *FirstLoad = Context.createLoadInstruction(*Slot);
    ASSERT_NE(FirstStore, nullptr);
    ASSERT_NE(FirstLoad, nullptr);
    const StoreInstruction *SecondStore = Context.createStoreInstruction(*Slot, *FirstLoad);
    ASSERT_NE(SecondStore, nullptr);
    for (unsigned Index = 0; Index < 1024; ++Index)
    {
      AllocaInstruction *Next = Context.createAllocaInstruction(Context.getBoolType());
      ASSERT_NE(Next, nullptr);
      ASSERT_NE(Context.createStoreInstruction(*Next, *FirstLoad), nullptr);
      ASSERT_NE(Context.createLoadInstruction(*Next), nullptr);
    }
    EXPECT_EQ(&FirstStore->address(), Slot);
    EXPECT_EQ(&FirstStore->storedValue(), &Context.getBoolConstant(true));
    EXPECT_EQ(&FirstLoad->address(), Slot);
    EXPECT_EQ(&SecondStore->storedValue(), FirstLoad);
    EXPECT_EQ(&FirstLoad->type(), &Context.getBoolType());
    EXPECT_EQ(&SecondStore->type(), &Context.getVoidType());
  }
} // namespace ink::semantic::test
