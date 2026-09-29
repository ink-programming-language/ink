#include "ink/ir/ir_builder.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/context.h"

#include <gtest/gtest.h>

#include <initializer_list>
#include <type_traits>

namespace ink::ir::test
{
  static_assert(std::is_base_of_v<Value, AllocaInstruction>);
  static_assert(std::is_base_of_v<Value, LoadInstruction>);
  static_assert(std::is_base_of_v<Value, StoreInstruction>);
  static_assert(!std::is_copy_constructible_v<AllocaInstruction>);
  static_assert(!std::is_move_constructible_v<LoadInstruction>);
  static_assert(!std::is_copy_constructible_v<StoreInstruction>);

  // Explicit stores and distinct reads form an ordered function body without changing operand parent links.
  TEST(IRMemoryInstructionTest, InitializationAssignmentAndReadsPreserveBlockOrder)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>());
    auto MainOwner = Factory.createFunction(Context.namePool().intern("Main"), *Signature);
    Function *Main = MainOwner.get();
    ASSERT_NE(Main, nullptr);
    BasicBlock *Entry = Factory.createFunctionBody(*Main);
    auto SlotOwner = Factory.createDetachedAllocaInstruction(*Int32);
    AllocaInstruction *Slot = SlotOwner.get();
    auto SecondSlotOwner = Factory.createDetachedAllocaInstruction(*Int32);
    AllocaInstruction *SecondSlot = SecondSlotOwner.get();
    ASSERT_NE(Entry, nullptr);
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(SecondSlot, nullptr);
    EXPECT_NE(Slot, SecondSlot);
    EXPECT_EQ(&Slot->type(), &SecondSlot->type());
    EXPECT_EQ(&Slot->allocatedType(), Int32);
    EXPECT_EQ(static_cast<const PointerType &>(Slot->type()).access(), AccessKind::ReadWrite);
    EXPECT_EQ(Slot->outer(), nullptr);

    const IntegerConstant *Two = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 2));
    const IntegerConstant *Four = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 4));
    ASSERT_NE(Two, nullptr);
    ASSERT_NE(Four, nullptr);
    auto InitializeOwner = Factory.createDetachedStoreInstruction(*Slot, *Two);
    StoreInstruction *Initialize = InitializeOwner.get();
    auto FirstReadOwner = Factory.createDetachedLoadInstruction(*Slot);
    LoadInstruction *FirstRead = FirstReadOwner.get();
    auto AssignOwner = Factory.createDetachedStoreInstruction(*Slot, *Four);
    StoreInstruction *Assign = AssignOwner.get();
    auto SecondReadOwner = Factory.createDetachedLoadInstruction(*Slot);
    LoadInstruction *SecondRead = SecondReadOwner.get();
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
    EXPECT_EQ(&Assign->type(), &Context.typePool().getType<TypeKind::Void>());

    const Type *Parameters[] = {Int32};
    const FunctionType *PrintType = Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>(), Parameters);
    auto PrintOwner = Factory.createFunction(Context.namePool().intern("Print"), *PrintType);
    const Function *Print = PrintOwner.get();
    ASSERT_NE(Print, nullptr);
    const Value *Arguments[] = {SecondRead};
    auto CallOwner = Factory.createDetachedCallInstruction(*Print, Arguments);
    CallInstruction *Call = CallOwner.get();
    ASSERT_NE(Call, nullptr);
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(SlotOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(InitializeOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(FirstReadOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(AssignOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(SecondReadOwner)));
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(CallOwner)));
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
      EXPECT_EQ(Operation->outer(), Entry);
    }
    ASSERT_EQ(Entry->values().size(), 6U);
    for (std::size_t Index = 0; Index < Entry->values().size(); ++Index)
    {
      EXPECT_EQ(Entry->values()[Index].get(), Sequence[Index]);
    }
    EXPECT_EQ(Entry->outer(), Main);
    EXPECT_EQ(Two->outer(), nullptr);
    EXPECT_EQ(Four->outer(), nullptr);
    EXPECT_EQ(Print->outer(), nullptr);
    EXPECT_FALSE(Factory.appendValue(*Entry, std::move(InitializeOwner)));
    auto RemovedCallOwner = Factory.removeValue(*Entry, *Call);
    ASSERT_EQ(RemovedCallOwner.get(), Call);
    EXPECT_EQ(Call->outer(), nullptr);
    EXPECT_EQ(Call->arguments()[0], SecondRead);
    ASSERT_TRUE(Factory.appendValue(*Entry, std::move(RemovedCallOwner)));
    EXPECT_EQ(Entry->values().back().get(), Call);
  }

  // Scalars, indirect values, slices and nested fixed arrays use canonical writable address types.
  TEST(IRMemoryInstructionTest, AllocationsSupportConcreteStorageTypes)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const ArrayType *Row = Context.typePool().getType<TypeKind::Array>(*Int32, 3);
    const Type *Types[] = {
        &Context.typePool().getType<TypeKind::Bool>(),
        Int32,
        Context.typePool().getType<TypeKind::Integer>(129, false),
        Context.typePool().getType<TypeKind::Float>(16),
        Context.typePool().getType<TypeKind::Float>(32),
        Context.typePool().getType<TypeKind::Float>(64),
        Context.typePool().getType<TypeKind::Pointer>(*Int32, AccessKind::ReadOnly),
        Context.typePool().getType<TypeKind::Reference>(*Int32, AccessKind::ReadWrite),
        Context.typePool().getType<TypeKind::Slice>(*Int32, AccessKind::ReadOnly),
        Row,
        Context.typePool().getType<TypeKind::Array>(*Row, 2),
        Context.typePool().getType<TypeKind::Array>(*Int32, 0),
    };
    for (const Type *TypeValue : Types)
    {
      ASSERT_NE(TypeValue, nullptr);
      auto SlotOwner = Factory.createDetachedAllocaInstruction(*TypeValue);
      AllocaInstruction *Slot = SlotOwner.get();
      ASSERT_NE(Slot, nullptr);
      EXPECT_EQ(&Slot->allocatedType(), TypeValue);
      EXPECT_EQ(&Slot->type(), Context.typePool().getType<TypeKind::Pointer>(*TypeValue, AccessKind::ReadWrite));
      EXPECT_EQ(&Slot->context(), &Context);
      // Construction validates types; definite initialization is a separate verifier obligation.
      auto ReadOwner = Factory.createDetachedLoadInstruction(*Slot);
      LoadInstruction *Read = ReadOwner.get();
      ASSERT_NE(Read, nullptr);
      EXPECT_EQ(&Read->type(), TypeValue);
      auto WriteOwner = Factory.createDetachedStoreInstruction(*Slot, *Read);
      const StoreInstruction *Write = WriteOwner.get();
      ASSERT_NE(Write, nullptr);
      EXPECT_EQ(&Write->storedValue(), Read);
      EXPECT_EQ(&Write->type(), &Context.typePool().getType<TypeKind::Void>());
    }
  }

  // Meta/control types, raw signatures and nominal types without layouts cannot be allocated or dereferenced.
  TEST(IRMemoryInstructionTest, UnsupportedStorageTypesAreRejectedAtEveryFactory)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const Name NameValue = Context.namePool().intern("Opaque");
    const ArrayType *VoidArray = Context.typePool().getType<TypeKind::Array>(Context.typePool().getType<TypeKind::Void>(), 1);
    const Type *Unsupported[] = {
        &Context.typePool().getType<TypeKind::Meta>(),
        &Context.typePool().getType<TypeKind::Void>(),
        &Context.typePool().getType<TypeKind::Label>(),
        &Context.typePool().getType<TypeKind::Module>(),
        Context.typePool().getType<TypeKind::Function>(Context.typePool().getType<TypeKind::Void>()),
        Factory.createClassType(NameValue),
        Factory.createEnumType(NameValue),
        Factory.createInterfaceType(NameValue),
        VoidArray,
        Context.typePool().getType<TypeKind::Array>(*VoidArray, 2),
    };
    for (const Type *TypeValue : Unsupported)
    {
      ASSERT_NE(TypeValue, nullptr);
      EXPECT_EQ(Factory.createDetachedAllocaInstruction(*TypeValue), nullptr);
      const PointerType *Pointer = Context.typePool().getType<TypeKind::Pointer>(*TypeValue, AccessKind::ReadWrite);
      // Storing an address is allowed even when dereferencing its pointee is not yet supported.
      auto PointerSlotOwner = Factory.createDetachedAllocaInstruction(*Pointer);
      AllocaInstruction *PointerSlot = PointerSlotOwner.get();
      ASSERT_NE(PointerSlot, nullptr);
      auto AddressOwner = Factory.createDetachedLoadInstruction(*PointerSlot);
      LoadInstruction *Address = AddressOwner.get();
      ASSERT_NE(Address, nullptr);
      EXPECT_EQ(Factory.createDetachedLoadInstruction(*Address), nullptr);
      EXPECT_EQ(Factory.createDetachedStoreInstruction(*Address, Context.constantPool().getBoolConstant(false)), nullptr);
    }
  }

  // Read-only pointees can be loaded but not stored, independently of the pointer slot's own write access.
  TEST(IRMemoryInstructionTest, LoadedPointersPreserveReadOnlyAccess)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const PointerType *ReadOnly = Context.typePool().getType<TypeKind::Pointer>(*Int32, AccessKind::ReadOnly);
    const PointerType *ReadWrite = Context.typePool().getType<TypeKind::Pointer>(*Int32, AccessKind::ReadWrite);
    for (const PointerType *Pointer : {ReadOnly, ReadWrite})
    {
      auto PointerSlotOwner = Factory.createDetachedAllocaInstruction(*Pointer);
      AllocaInstruction *PointerSlot = PointerSlotOwner.get();
      ASSERT_NE(PointerSlot, nullptr);
      auto AddressOwner = Factory.createDetachedLoadInstruction(*PointerSlot);
      LoadInstruction *Address = AddressOwner.get();
      ASSERT_NE(Address, nullptr);
      auto ReadOwner = Factory.createDetachedLoadInstruction(*Address);
      LoadInstruction *Read = ReadOwner.get();
      ASSERT_NE(Read, nullptr);
      EXPECT_EQ(&Read->type(), Int32);
      EXPECT_EQ(&Read->address(), Address);
      auto WriteOwner = Factory.createDetachedStoreInstruction(*Address, *Read);
      const StoreInstruction *Write = WriteOwner.get();
      EXPECT_EQ(Write != nullptr, Pointer->access() == AccessKind::ReadWrite);
      EXPECT_NE(Factory.createDetachedStoreInstruction(*PointerSlot, *Address), nullptr);
    }
    const ReferenceType *Reference = Context.typePool().getType<TypeKind::Reference>(*Int32, AccessKind::ReadWrite);
    auto ReferenceSlotOwner = Factory.createDetachedAllocaInstruction(*Reference);
    AllocaInstruction *ReferenceSlot = ReferenceSlotOwner.get();
    ASSERT_NE(ReferenceSlot, nullptr);
    auto ReferenceValueOwner = Factory.createDetachedLoadInstruction(*ReferenceSlot);
    LoadInstruction *ReferenceValue = ReferenceValueOwner.get();
    ASSERT_NE(ReferenceValue, nullptr);
    EXPECT_EQ(Factory.createDetachedLoadInstruction(*ReferenceValue), nullptr);
    EXPECT_EQ(Factory.createDetachedStoreInstruction(*ReferenceValue, Context.constantPool().getBoolConstant(false)), nullptr);
  }

  // Memory operands must be local, pointer-typed and exactly match width, signedness and pointee identity.
  TEST(IRMemoryInstructionTest, ForeignOperandsAndTypeMismatchesAreRejected)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    IRContext Other(Compilation);
    IRBuilder OtherFactory(Other);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    const IntegerType *ForeignInt32 = Other.typePool().getType<TypeKind::Integer>(32, true);
    auto SlotOwner = Factory.createDetachedAllocaInstruction(*Int32);
    AllocaInstruction *Slot = SlotOwner.get();
    auto ForeignSlotOwner = OtherFactory.createDetachedAllocaInstruction(*ForeignInt32);
    AllocaInstruction *ForeignSlot = ForeignSlotOwner.get();
    ASSERT_NE(Slot, nullptr);
    ASSERT_NE(ForeignSlot, nullptr);
    EXPECT_EQ(Factory.createDetachedAllocaInstruction(*ForeignInt32), nullptr);
    EXPECT_EQ(Factory.createDetachedLoadInstruction(*ForeignSlot), nullptr);
    EXPECT_EQ(Factory.createDetachedStoreInstruction(*ForeignSlot, Context.constantPool().getBoolConstant(false)), nullptr);
    const Value *InvalidValues[] = {
        Context.constantPool().getIntegerConstant(*Context.typePool().getType<TypeKind::Integer>(32, false), IntegerBits(32, 2)),
        Context.constantPool().getIntegerConstant(*Context.typePool().getType<TypeKind::Integer>(64, true), IntegerBits(64, 2)),
        Other.constantPool().getIntegerConstant(*ForeignInt32, IntegerBits(32, 2)),
        Context.constantPool().getFloatConstant(*Context.typePool().getType<TypeKind::Float>(32), FloatBits(32, 0)),
        &Context.constantPool().getBoolConstant(false),
        Int32,
        ForeignSlot,
    };
    for (const Value *Invalid : InvalidValues)
    {
      ASSERT_NE(Invalid, nullptr);
      EXPECT_EQ(Factory.createDetachedStoreInstruction(*Slot, *Invalid), nullptr);
    }
    const IntegerConstant *Two = Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 2));
    ASSERT_NE(Two, nullptr);
    EXPECT_EQ(Factory.createDetachedLoadInstruction(*Two), nullptr);
    EXPECT_EQ(Factory.createDetachedLoadInstruction(Slot->type()), nullptr);
    EXPECT_EQ(Factory.createDetachedStoreInstruction(*Two, *Two), nullptr);
    EXPECT_EQ(Factory.createDetachedStoreInstruction(Slot->type(), *Two), nullptr);
    auto StoreOwner = Factory.createDetachedStoreInstruction(*Slot, *Two);
    StoreInstruction *Store = StoreOwner.get();
    ASSERT_NE(Store, nullptr);
    EXPECT_EQ(Factory.createDetachedStoreInstruction(*Slot, *Store), nullptr);
    EXPECT_EQ(Factory.createDetachedLoadInstruction(*Store), nullptr);
    EXPECT_EQ(Slot->outer(), nullptr);
    EXPECT_EQ(&Store->storedValue(), Two);
    EXPECT_NE(Factory.createDetachedLoadInstruction(*Slot), nullptr);
  }

  // Growing a block's owning list preserves all borrowed address, result and constant references.
  TEST(IRMemoryInstructionTest, InstructionOperandsSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    auto SlotOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
    AllocaInstruction *Slot = SlotOwner.get();
    ASSERT_NE(Slot, nullptr);
    auto FirstStoreOwner = Factory.createDetachedStoreInstruction(*Slot, Context.constantPool().getBoolConstant(true));
    const StoreInstruction *FirstStore = FirstStoreOwner.get();
    auto FirstLoadOwner = Factory.createDetachedLoadInstruction(*Slot);
    const LoadInstruction *FirstLoad = FirstLoadOwner.get();
    ASSERT_NE(FirstStore, nullptr);
    ASSERT_NE(FirstLoad, nullptr);
    auto SecondStoreOwner = Factory.createDetachedStoreInstruction(*Slot, *FirstLoad);
    const StoreInstruction *SecondStore = SecondStoreOwner.get();
    ASSERT_NE(SecondStore, nullptr);
    auto Block = Factory.createBasicBlock();
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(SlotOwner)));
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(FirstStoreOwner)));
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(FirstLoadOwner)));
    ASSERT_TRUE(Factory.appendValue(*Block, std::move(SecondStoreOwner)));
    for (unsigned Index = 0; Index < 1024; ++Index)
    {
      auto NextOwner = Factory.createDetachedAllocaInstruction(Context.typePool().getType<TypeKind::Bool>());
      AllocaInstruction *Next = NextOwner.get();
      ASSERT_NE(Next, nullptr);
      ASSERT_TRUE(Factory.appendValue(*Block, std::move(NextOwner)));
      ASSERT_TRUE(Factory.appendValue(*Block, Factory.createDetachedStoreInstruction(*Next, *FirstLoad)));
      ASSERT_TRUE(Factory.appendValue(*Block, Factory.createDetachedLoadInstruction(*Next)));
    }
    EXPECT_EQ(&FirstStore->address(), Slot);
    EXPECT_EQ(&FirstStore->storedValue(), &Context.constantPool().getBoolConstant(true));
    EXPECT_EQ(&FirstLoad->address(), Slot);
    EXPECT_EQ(&SecondStore->storedValue(), FirstLoad);
    EXPECT_EQ(&FirstLoad->type(), &Context.typePool().getType<TypeKind::Bool>());
    EXPECT_EQ(&SecondStore->type(), &Context.typePool().getType<TypeKind::Void>());
  }
} // namespace ink::ir::test
