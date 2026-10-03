#include "ink/ir/ir_builder.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // Arrays are interned by exact type and ordered canonical elements, including nested and empty arrays.
  TEST(IRArrayConstantTest, CanonicalizesOrderedNestedAndEmptyValues)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const auto *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    const auto *Two = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 2));
    const auto *Pair = Context.typePool().getType<TypeKind::Array>(*Integer, 2);
    const Constant *Elements[] = {One, Two};
    const auto *Array = Context.constantPool().getArrayConstant(*Pair, Elements);
    ASSERT_NE(Array, nullptr);
    EXPECT_TRUE(ArrayConstant::classof(Array));
    EXPECT_TRUE(Constant::classof(Array));
    EXPECT_EQ(&Array->arrayType(), Pair);
    EXPECT_EQ(Array->elements()[0], One);
    EXPECT_EQ(Array->elements()[1], Two);
    EXPECT_EQ(Context.constantPool().getArrayConstant(*Pair, Elements), Array);
    const Constant *Reversed[] = {Two, One};
    EXPECT_NE(Context.constantPool().getArrayConstant(*Pair, Reversed), Array);
    const auto *NestedType = Context.typePool().getType<TypeKind::Array>(*Pair, 2);
    const Constant *NestedElements[] = {Array, Array};
    const auto *Nested = Context.constantPool().getArrayConstant(*NestedType, NestedElements);
    ASSERT_NE(Nested, nullptr);
    EXPECT_EQ(Nested->elements()[0], Array);
    EXPECT_EQ(Context.constantPool().getArrayConstant(*NestedType, NestedElements), Nested);
    const auto *EmptyType = Context.typePool().getType<TypeKind::Array>(*Integer, 0);
    const auto *Empty = Context.constantPool().getArrayConstant(*EmptyType, {});
    ASSERT_NE(Empty, nullptr);
    EXPECT_TRUE(Empty->elements().empty());
    EXPECT_EQ(Context.constantPool().getArrayConstant(*EmptyType, {}), Empty);
    EXPECT_TRUE(Context.constantPool().owns(*Nested));
    EXPECT_TRUE(Context.constantPool().owns(*Empty));
    EXPECT_EQ(Context.constantPool().size(), 8U);
  }

  // Wrong counts, null elements, foreign contexts and incorrect element types never add array constants.
  TEST(IRArrayConstantTest, RejectsMalformedOrForeignElements)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRContext Foreign(Compilation);
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(8, false);
    const auto *ArrayTypeValue = Context.typePool().getType<TypeKind::Array>(*Integer, 1);
    const auto *ForeignInteger = Foreign.typePool().getType<TypeKind::Integer>(8, false);
    const auto *ForeignArray = Foreign.typePool().getType<TypeKind::Array>(*ForeignInteger, 1);
    const auto *Byte = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(8, 1));
    const auto *ForeignByte = Foreign.constantPool().getIntegerConstant(*ForeignInteger, IntegerBits(8, 1));
    const Constant *Null[] = {nullptr};
    const Constant *WrongType[] = {&Context.constantPool().getBoolConstant(true)};
    const Constant *ForeignElements[] = {ForeignByte};
    const Constant *Elements[] = {Byte};
    const auto Size = Context.constantPool().size();
    EXPECT_EQ(Context.constantPool().getArrayConstant(*ArrayTypeValue, {}), nullptr);
    EXPECT_EQ(Context.constantPool().getArrayConstant(*ArrayTypeValue, Null), nullptr);
    EXPECT_EQ(Context.constantPool().getArrayConstant(*ArrayTypeValue, WrongType), nullptr);
    EXPECT_EQ(Context.constantPool().getArrayConstant(*ArrayTypeValue, ForeignElements), nullptr);
    EXPECT_EQ(Context.constantPool().getArrayConstant(*ForeignArray, Elements), nullptr);
    EXPECT_EQ(Context.constantPool().size(), Size);
  }

  // Constructed arrays preserve operand order and identity; repetition has one operand even when the array is empty.
  TEST(IRArrayInstructionTest, ConstructsAndExtractsValuesAtInsertionPoint)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const auto *One = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 1));
    const auto *Pair = Context.typePool().getType<TypeKind::Array>(*Integer, 2);
    const Value *Elements[] = {One, One};
    const Value *Repeated[] = {One};
    auto Block = Builder.createBasicBlock();
    EXPECT_EQ(Builder.createArrayInstruction(*Pair, Elements), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    const auto *Array = Builder.createArrayInstruction(*Pair, Elements);
    ASSERT_NE(Array, nullptr);
    EXPECT_FALSE(Array->repeated());
    EXPECT_EQ(Array->elements().size(), 2U);
    EXPECT_EQ(&Array->arrayType(), Pair);
    const auto *Repeat = Builder.createArrayInstruction(*Pair, Repeated, true);
    ASSERT_NE(Repeat, nullptr);
    EXPECT_TRUE(Repeat->repeated());
    EXPECT_EQ(Repeat->elements().size(), 1U);
    const auto *Extract = Builder.createArrayExtractInstruction(*Array, *One);
    ASSERT_NE(Extract, nullptr);
    EXPECT_EQ(&Extract->array(), Array);
    EXPECT_EQ(&Extract->index(), One);
    EXPECT_EQ(&Extract->type(), Integer);
    const auto *EmptyType = Context.typePool().getType<TypeKind::Array>(*Integer, 0);
    EXPECT_NE(Builder.createArrayInstruction(*EmptyType, {}), nullptr);
    EXPECT_NE(Builder.createArrayInstruction(*EmptyType, Repeated, true), nullptr);
    EXPECT_EQ(Block->values().size(), 5U);
    auto Detached = Builder.createDetachedArrayInstruction(*Pair, Elements);
    ASSERT_NE(Detached, nullptr);
    EXPECT_EQ(Detached->outer(), nullptr);
    EXPECT_EQ(Block->values().size(), 5U);
  }

  // Element addresses inherit read-only or read-write access, so writes through read-only arrays remain invalid.
  TEST(IRArrayInstructionTest, ElementPointersPreserveAccessAndNestedType)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Builder(Context);
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const auto *Index = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 0));
    const auto *Array = Context.typePool().getType<TypeKind::Array>(*Integer, 2);
    const auto *Nested = Context.typePool().getType<TypeKind::Array>(*Array, 3);
    auto Storage = Builder.createDetachedAllocaInstruction(*Nested);
    auto Pointer = Builder.createDetachedArrayElementPointerInstruction(*Storage, *Index);
    ASSERT_NE(Pointer, nullptr);
    EXPECT_EQ(&Pointer->address(), Storage.get());
    EXPECT_EQ(&Pointer->index(), Index);
    EXPECT_EQ(&static_cast<const PointerType &>(Pointer->type()).pointeeType(), Array);
    EXPECT_EQ(static_cast<const PointerType &>(Pointer->type()).access(), AccessKind::ReadWrite);
    auto Leaf = Builder.createDetachedArrayElementPointerInstruction(*Pointer, *Index);
    ASSERT_NE(Leaf, nullptr);
    EXPECT_NE(Builder.createDetachedStoreInstruction(*Leaf, *Index), nullptr);
    const auto *ReadOnly = Context.typePool().getType<TypeKind::Pointer>(*Array, AccessKind::ReadOnly);
    const Type *Parameters[] = {ReadOnly};
    const auto *Signature = Context.typePool().getType<TypeKind::Function>(*Integer, Parameters);
    auto FunctionValue = Builder.createFunction(Context.namePool().intern("read"), *Signature);
    auto ReadOnlyElement = Builder.createDetachedArrayElementPointerInstruction(*FunctionValue->parameters()[0], *Index);
    ASSERT_NE(ReadOnlyElement, nullptr);
    EXPECT_EQ(static_cast<const PointerType &>(ReadOnlyElement->type()).access(), AccessKind::ReadOnly);
    EXPECT_NE(Builder.createDetachedLoadInstruction(*ReadOnlyElement), nullptr);
    EXPECT_EQ(Builder.createDetachedStoreInstruction(*ReadOnlyElement, *Index), nullptr);
  }

  // Factory validation rejects malformed arrays, non-array bases, non-integer indices and foreign operands without insertion.
  TEST(IRArrayInstructionTest, RejectsInvalidOperandsWithoutMutatingBlock)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRContext Foreign(Compilation);
    IRBuilder Builder(Context);
    const auto *Integer = Context.typePool().getType<TypeKind::Integer>(32, true);
    const auto *ArrayTypeValue = Context.typePool().getType<TypeKind::Array>(*Integer, 2);
    const auto *Index = Context.constantPool().getIntegerConstant(*Integer, IntegerBits(32, 0));
    const auto &Boolean = Context.constantPool().getBoolConstant(false);
    const auto *ForeignInteger = Foreign.typePool().getType<TypeKind::Integer>(32, true);
    const auto *ForeignIndex = Foreign.constantPool().getIntegerConstant(*ForeignInteger, IntegerBits(32, 0));
    const Value *Valid[] = {Index, Index};
    const Value *Null[] = {Index, nullptr};
    const Value *Wrong[] = {Index, &Boolean};
    const Value *ForeignElements[] = {Index, ForeignIndex};
    auto Array = Builder.createDetachedArrayInstruction(*ArrayTypeValue, Valid);
    auto Address = Builder.createDetachedAllocaInstruction(*ArrayTypeValue);
    auto ScalarAddress = Builder.createDetachedAllocaInstruction(*Integer);
    auto Block = Builder.createBasicBlock();
    ASSERT_TRUE(Builder.setInsertPoint(*Block));
    EXPECT_EQ(Builder.createArrayInstruction(*ArrayTypeValue, {}), nullptr);
    EXPECT_EQ(Builder.createArrayInstruction(*ArrayTypeValue, Valid, true), nullptr);
    EXPECT_EQ(Builder.createArrayInstruction(*ArrayTypeValue, Null), nullptr);
    EXPECT_EQ(Builder.createArrayInstruction(*ArrayTypeValue, Wrong), nullptr);
    EXPECT_EQ(Builder.createArrayInstruction(*ArrayTypeValue, ForeignElements), nullptr);
    EXPECT_EQ(Builder.createArrayExtractInstruction(*Index, *Index), nullptr);
    EXPECT_EQ(Builder.createArrayExtractInstruction(*Array, Boolean), nullptr);
    EXPECT_EQ(Builder.createArrayExtractInstruction(*Array, *ForeignIndex), nullptr);
    EXPECT_EQ(Builder.createArrayElementPointerInstruction(*ScalarAddress, *Index), nullptr);
    EXPECT_EQ(Builder.createArrayElementPointerInstruction(*Array, *Index), nullptr);
    EXPECT_EQ(Builder.createArrayElementPointerInstruction(*Address, Boolean), nullptr);
    EXPECT_EQ(Builder.createArrayElementPointerInstruction(*Address, *ForeignIndex), nullptr);
    EXPECT_TRUE(Block->values().empty());
  }
} // namespace ink::ir::test
