#include "../artifact/artifact_test_support.h"
#include "ink/execution/bytecode/execution_compiler.h"
#include "ink/ir/constant/class_constant.h"

#include <gtest/gtest.h>

namespace ink::execution::test
{
  namespace
  {
    const ir::ClassType *pairType(ArtifactContext &Test)
    {
      const auto *Type = Test.Builder.createClassType(Test.Context.namePool().intern("Pair"));
      const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Ready"), &Test.Bool}, {Test.Context.namePool().intern("Value"), &Test.Int32}};
      return Type && Test.Builder.defineClassType(*Type, Fields, "tests::Pair") ? Type : nullptr;
    }
  } // namespace

  // Class construction, field extraction and field addresses use typed bytecode and reject invalid field indices.
  TEST(ClassBytecodeTest, LowersAndVerifiesClassOperations)
  {
    ArtifactContext Test;
    const auto *Class = pairType(Test);
    ASSERT_NE(Class, nullptr);
    auto *Function = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *Fields[] = {&Test.Context.constantPool().getBoolConstant(true), &Test.integer(42)};
    const auto *Value = Test.Builder.createClassInstruction(*Class, Fields);
    const auto *Storage = Test.Builder.createAllocaInstruction(*Class);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, *Value), nullptr);
    ASSERT_NE(Test.Builder.createFieldPointerInstruction(*Storage, 1), nullptr);
    const auto *Extract = Test.Builder.createFieldExtractInstruction(*Value, 1);
    ASSERT_NE(Test.Builder.createReturnInstruction(Extract), nullptr);
    auto Compiled = ExecutionCompiler{}.compile(*Function, Test.Bridge);
    ASSERT_TRUE(Compiled) << static_cast<unsigned>(Compiled.Status);
    auto &Image = *Compiled.Function;
    ASSERT_EQ(Image.Code.size(), 6U);
    EXPECT_EQ(Image.Code[0].Code, BytecodeOpcode::Class);
    EXPECT_EQ(Image.Code[1].Code, BytecodeOpcode::Alloca);
    EXPECT_EQ(Image.Code[3].Code, BytecodeOpcode::FieldPointer);
    EXPECT_EQ(Image.Code[4].Code, BytecodeOpcode::FieldExtract);
    const auto Original = Image.Code;
    Image.Code[3].Operands[2] = 2;
    EXPECT_EQ(ExecutionCompiler{}.verify(Image), ExecutionStatus::TypeMismatch);
    Image.Code = Original;
    Image.Code[4].Operands[2] = 0;
    EXPECT_EQ(ExecutionCompiler{}.verify(Image), ExecutionStatus::TypeMismatch);
    Image.Code = Original;
    Image.Code[0].Operands[2] = 4;
    EXPECT_EQ(ExecutionCompiler{}.verify(Image), ExecutionStatus::TypeMismatch);
  }

  // A class snapshot remains unchanged when an address mutates the original object, while whole-object assignment retains field addresses.
  TEST(ClassBytecodeTest, CopiesAreIndependentAndFieldAddressesSurviveAssignment)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Class = pairType(Test);
    ASSERT_NE(Class, nullptr);
    auto *Function = Test.function("main", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *Fields[] = {&Test.Context.constantPool().getBoolConstant(true), &Test.integer(19)};
    const ir::Value *ReplacementFields[] = {&Test.Context.constantPool().getBoolConstant(false), &Test.integer(23)};
    const auto *Value = Test.Builder.createClassInstruction(*Class, Fields);
    const auto *Replacement = Test.Builder.createClassInstruction(*Class, ReplacementFields);
    const auto *Storage = Test.Builder.createAllocaInstruction(*Class);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, *Value), nullptr);
    const auto *Snapshot = Test.Builder.createLoadInstruction(*Storage);
    const auto *Address = Test.Builder.createFieldPointerInstruction(*Storage, 1);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Address, Test.integer(77)), nullptr);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, *Replacement), nullptr);
    const auto *Old = Test.Builder.createFieldExtractInstruction(*Snapshot, 1);
    const auto *Current = Test.Builder.createLoadInstruction(*Address);
    const auto *Result = Test.Builder.createAddInstruction(*Old, *Current);
    ASSERT_NE(Test.Builder.createReturnInstruction(Result), nullptr);
    const auto Executed = Engine.execute(*Function);
    ASSERT_TRUE(Executed) << static_cast<unsigned>(Executed.Status);
    EXPECT_EQ(Executed.Value.integer().lowWord(), 42U);
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
  }

  // Nested class/array value returns freeze recursively into immutable IR class constants.
  TEST(ClassBytecodeTest, FreezesNestedClassesContainingArrays)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Pair = pairType(Test);
    ASSERT_NE(Pair, nullptr);
    const auto *Array = Test.Context.typePool().getType<ir::TypeKind::Array>(*Pair, 2);
    const auto *Outer = Test.Builder.createClassType(Test.Context.namePool().intern("Outer"));
    const ir::ClassField OuterFields[] = {{Test.Context.namePool().intern("Pairs"), Array}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Outer, OuterFields, "tests::Outer"));
    auto *Function = Test.function("main", *Outer);
    ASSERT_TRUE(Test.begin(*Function));
    const ir::Value *Fields[] = {&Test.Context.constantPool().getBoolConstant(true), &Test.integer(42)};
    const auto *Value = Test.Builder.createClassInstruction(*Pair, Fields);
    const ir::Value *Elements[] = {Value, Value};
    const auto *Pairs = Test.Builder.createArrayInstruction(*Array, Elements);
    const ir::Value *Values[] = {Pairs};
    const auto *Result = Test.Builder.createClassInstruction(*Outer, Values);
    ASSERT_NE(Test.Builder.createReturnInstruction(Result), nullptr);
    const auto Executed = Engine.execute(*Function);
    ASSERT_TRUE(Executed);
    const auto *Frozen = Executed.Value.toConstant(Test.Context);
    ASSERT_NE(Frozen, nullptr);
    ASSERT_TRUE(ir::ClassConstant::classof(Frozen));
    EXPECT_EQ(Executed.Value.fields()[0].array()[1].fields()[1].integer().lowWord(), 42U);
  }

  // Returning a class copies its pointer fields without owning or validating their pointees.
  TEST(ClassBytecodeTest, CopiesRawPointerInsideReturnedClass)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Pointer = Test.Context.typePool().getType<ir::TypeKind::Pointer>(Test.Int32, ir::AccessKind::ReadWrite);
    const auto *Class = Test.Builder.createClassType(Test.Context.namePool().intern("Borrow"));
    const ir::ClassField Fields[] = {{Test.Context.namePool().intern("Address"), Pointer}};
    ASSERT_TRUE(Test.Builder.defineClassType(*Class, Fields, "tests::Borrow"));
    auto *Function = Test.function("main", *Class);
    ASSERT_TRUE(Test.begin(*Function));
    const auto *Storage = Test.Builder.createAllocaInstruction(Test.Int32);
    ASSERT_NE(Test.Builder.createStoreInstruction(*Storage, Test.integer(42)), nullptr);
    const ir::Value *Values[] = {Storage};
    const auto *Value = Test.Builder.createClassInstruction(*Class, Values);
    ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
    EXPECT_EQ(Engine.execute(*Function).Status, ExecutionStatus::Success);
    EXPECT_EQ(Engine.heap().liveStorageCount(), 0U);
  }

  // Field projection requires whole-object initialization and preserves readonly pointer permissions.
  TEST(ClassBytecodeTest, RejectsUninitializedProjectionAndReadonlyMutation)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Class = pairType(Test);
    ASSERT_NE(Class, nullptr);
    auto *Function = Test.function("uninitialized", Test.Int32);
    ASSERT_TRUE(Test.begin(*Function));
    const auto *Storage = Test.Builder.createAllocaInstruction(*Class);
    const auto *Address = Test.Builder.createFieldPointerInstruction(*Storage, 1);
    const auto *Value = Test.Builder.createLoadInstruction(*Address);
    ASSERT_NE(Test.Builder.createReturnInstruction(Value), nullptr);
    EXPECT_EQ(Engine.execute(*Function).Status, ExecutionStatus::Uninitialized);
    const auto *Readonly = Test.Context.typePool().getType<ir::TypeKind::Pointer>(*Class, ir::AccessKind::ReadOnly);
    const ir::Type *Parameters[] = {Readonly};
    auto *Reader = Test.function("reader", Test.Int32, Parameters);
    ASSERT_TRUE(Test.begin(*Reader));
    const auto *Field = Test.Builder.createFieldPointerInstruction(*Reader->parameters()[0], 1);
    ASSERT_NE(Field, nullptr);
    EXPECT_EQ(static_cast<const ir::PointerType &>(Field->type()).access(), ir::AccessKind::ReadOnly);
    EXPECT_EQ(Test.Builder.createStoreInstruction(*Field, Test.integer(1)), nullptr);
  }

  // Class arguments and results are rejected at the C ABI boundary before native execution even when the named symbol exists.
  TEST(ClassBytecodeTest, RejectsByValueClassesAtTheNativeBoundary)
  {
    ArtifactContext Test;
    ExecutionEngine Engine(Test.Context);
    const auto *Class = pairType(Test);
    ASSERT_NE(Class, nullptr);
    auto *ReturnsClass = Test.function("abs", *Class, {}, ir::LanguageLinkage::C, core::FunctionBinding::Import);
    ASSERT_NE(ReturnsClass, nullptr);
    EXPECT_EQ(Engine.execute(*ReturnsClass).Status, ExecutionStatus::UnsupportedExternalSignature);
    const ir::Type *Parameters[] = {Class};
    auto *TakesClass = Test.function("abs", Test.Int32, Parameters, ir::LanguageLinkage::C, core::FunctionBinding::Import);
    ASSERT_NE(TakesClass, nullptr);
    const ExecutionValueRef Arguments[] = {Engine.heap().classValue(*Class, {Engine.heap().boolean(Test.Bool, true), Engine.heap().integer(Test.Int32, ExecutionInteger(32, 42))})};
    EXPECT_EQ(Engine.execute(*TakesClass, Arguments).Status, ExecutionStatus::UnsupportedExternalSignature);
  }
} // namespace ink::execution::test
