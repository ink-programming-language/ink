#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/string_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/verification/verifier.h"

#include <gtest/gtest.h>

#include <array>
#include <bit>
#include <cstdint>
#include <string_view>

namespace ink::ir::test
{
  namespace
  {
    class UnknownDataValue final : public Value
    {
      public:
        explicit UnknownDataValue(ValueRef ValueType) noexcept
            : Value(ValueType)
        {
        }
    };
  } // namespace

  // Float storage, loads, stores, and arithmetic retain exact operand types through a complete verified expression.
  TEST(IROperandVerifierTest, AcceptsFloatMemoryAndArithmeticOperands)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const FloatType &Float = Types.getFloatType(FloatType::Width::Bits32);
    const FloatConstant One(Float, std::bit_cast<std::uint32_t>(1.0f));
    const FloatConstant Two(Float, std::bit_cast<std::uint32_t>(2.0f));
    Function Owner(Types.getFunctionType(Float));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Owner.createBasicBlock());
    AllocaInstruction &Slot = Builder.createAlloca(Float);
    static_cast<void>(Builder.createStore(Slot, One));
    LoadInstruction &Loaded = Builder.createLoad(Slot);
    AddInstruction &Sum = Builder.createAdd(Loaded, Two);
    static_cast<void>(Builder.createReturn(Sum));
    verifyFunction(Owner, Engine);
    EXPECT_EQ(&Loaded.getType(), &Float);
  }

  // Strings, function values, and global addresses are accepted return operands when signatures borrow their exact result types.
  TEST(IROperandVerifierTest, AcceptsStringFunctionAndGlobalReturnOperands)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    InstructionBuilder Builder(Types);
    const IntegerType &Byte = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const ArrayType &StringType = Types.getArrayType(Byte, 3);
    const StringConstant String(StringType, std::string_view("a\0b", 3));
    Function StringOwner(Types.getFunctionType(StringType));
    Builder.setInsertPoint(StringOwner.createBasicBlock());
    static_cast<void>(Builder.createReturn(String));
    verifyFunction(StringOwner, Engine);
    Function Target(Types.getFunctionType(Types.getVoidType()));
    Function FunctionOwner(Types.getFunctionType(Target.getType()));
    Builder.setInsertPoint(FunctionOwner.createBasicBlock());
    static_cast<void>(Builder.createReturn(Target));
    verifyFunction(FunctionOwner, Engine);
    Module GlobalOwner;
    GlobalVariable &Global = GlobalOwner.createGlobalVariable(Types.getPointerType(Types.getBoolType()));
    Function AddressOwner(Types.getFunctionType(Global.getType()));
    Builder.setInsertPoint(AddressOwner.createBasicBlock());
    static_cast<void>(Builder.createReturn(Global));
    verifyFunction(AddressOwner, Engine);
    EXPECT_EQ(String.value().size(), 3U);
  }

  // String constants must match both their declared byte count and unsigned eight-bit array element type.
  TEST(IROperandVerifierDeathTest, ReportsMalformedStringLengthsAndElementTypes)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    InstructionBuilder Builder(Types);
    const IntegerType &Byte = Types.getIntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned);
    const ArrayType &WrongLengthType = Types.getArrayType(Byte, 4);
    const StringConstant WrongLength(WrongLengthType, "abc");
    Function LengthOwner(Types.getFunctionType(WrongLengthType));
    Builder.setInsertPoint(LengthOwner.createBasicBlock());
    static_cast<void>(Builder.createReturn(WrongLength));
    EXPECT_DEATH(verifyFunction(LengthOwner, Engine), "INK-I0001");
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const ArrayType &WrongElementType = Types.getArrayType(Int32, 3);
    const StringConstant WrongElement(WrongElementType, "abc");
    Function ElementOwner(Types.getFunctionType(WrongElementType));
    Builder.setInsertPoint(ElementOwner.createBasicBlock());
    static_cast<void>(Builder.createReturn(WrongElement));
    EXPECT_DEATH(verifyFunction(ElementOwner, Engine), "INK-I0001");
  }

  // An unknown value subclass with an integer type cannot silently pass as a constant or an owned SSA operand.
  TEST(IROperandVerifierDeathTest, ReportsUnknownDataOperandKinds)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const UnknownDataValue Unknown(Int32);
    const IntegerConstant One(Int32, 1);
    Function Owner(Types.getFunctionType(Int32));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Owner.createBasicBlock());
    AddInstruction &Sum = Builder.createAdd(Unknown, One);
    static_cast<void>(Builder.createReturn(Sum));
    EXPECT_DEATH(verifyFunction(Owner, Engine), "INK-I0001");
  }

  // Void pointers are valid parameter values, but loading void or allocating void is rejected by semantic verification.
  TEST(IROperandVerifierDeathTest, ReportsVoidLoadsAndAllocations)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const PointerType &VoidPointer = Types.getPointerType(Types.getVoidType());
    Function LoadOwner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(VoidPointer)}));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(LoadOwner.createBasicBlock());
    static_cast<void>(Builder.createLoad(LoadOwner.argument(0)));
    static_cast<void>(Builder.createReturn());
    EXPECT_DEATH(verifyFunction(LoadOwner, Engine), "INK-I0001");
    Function AllocationOwner(Types.getFunctionType(Types.getVoidType()));
    Builder.setInsertPoint(AllocationOwner.createBasicBlock());
    static_cast<void>(Builder.createAlloca(Types.getVoidType()));
    static_cast<void>(Builder.createReturn());
    EXPECT_DEATH(verifyFunction(AllocationOwner, Engine), "INK-I0001");
  }

  // Function declarations cannot depend on detached instruction results directly or through record method signatures.
  TEST(IROperandVerifierDeathTest, ReportsInstructionDependenciesInFunctionSignatures)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const PhiInstruction Detached(Types.getMetaType());
    const PointerType &Pointer = Types.getPointerType(Detached);
    const FunctionType Signature(Types.getVoidType(), std::array{Parameter(Pointer)});
    const Function Owner(Signature);
    EXPECT_DEATH(verifyFunction(Owner, Engine), "INK-I0001");
    const FunctionType MethodSignature(Detached);
    StructType Record;
    static_cast<void>(Record.addMethod(MethodSignature));
    const Function RecordOwner(Types.getFunctionType(Record));
    EXPECT_DEATH(verifyFunction(RecordOwner, Engine), "INK-I0001");
  }

  // Returning a concrete type value must inspect dependencies inside that value instead of stopping at its shared MetaType.
  TEST(IROperandVerifierDeathTest, ReportsInstructionDependenciesCarriedByReturnedTypeValues)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const PhiInstruction Detached(Types.getMetaType());
    const PointerType Wrapped(Detached);
    Function DetachedOwner(Types.getFunctionType(Types.getMetaType()));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(DetachedOwner.createBasicBlock());
    static_cast<void>(Builder.createReturn(Wrapped));
    EXPECT_DEATH(verifyFunction(DetachedOwner, Engine), "INK-I0001");
    const PointerType &TypePointer = Types.getPointerType(Types.getMetaType());
    Function Other(Types.getFunctionType(Types.getMetaType(), std::array{Parameter(TypePointer)}));
    Builder.setInsertPoint(Other.createBasicBlock());
    LoadInstruction &ForeignType = Builder.createLoad(Other.argument(0));
    static_cast<void>(Builder.createReturn(ForeignType));
    verifyFunction(Other, Engine);
    const PointerType ForeignWrapped(ForeignType);
    Function ForeignOwner(Types.getFunctionType(Types.getMetaType()));
    Builder.setInsertPoint(ForeignOwner.createBasicBlock());
    static_cast<void>(Builder.createReturn(ForeignWrapped));
    EXPECT_DEATH(verifyFunction(ForeignOwner, Engine), "INK-I0001");
  }

  // A type-producing load must precede an allocation that borrows its result as type metadata in the same block.
  TEST(IROperandVerifierDeathTest, ReportsForwardInstructionDependenciesInTypeMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const PointerType &TypePointer = Types.getPointerType(Types.getMetaType());
    Function Owner(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(TypePointer)}));
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Owner.createBasicBlock());
    LoadInstruction &TypeValue = Builder.createLoad(Owner.argument(0));
    static_cast<void>(Builder.createReturn());
    ASSERT_TRUE(Builder.setInsertPoint(TypeValue));
    static_cast<void>(Builder.createAlloca(TypeValue));
    EXPECT_DEATH(verifyFunction(Owner, Engine), "INK-I0001");
  }

  // A type-producing instruction on one diamond arm cannot define allocation metadata on the other reachable arm.
  TEST(IROperandVerifierDeathTest, ReportsNonDominatingInstructionDependenciesInTypeMetadata)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const PointerType &TypePointer = Types.getPointerType(Types.getMetaType());
    const std::array Parameters{Parameter(Types.getBoolType()), Parameter(TypePointer)};
    Function Owner(Types.getFunctionType(Types.getVoidType(), Parameters));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Left = Owner.createBasicBlock();
    BasicBlock &Right = Owner.createBasicBlock();
    BasicBlock &Merge = Owner.createBasicBlock();
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Entry);
    static_cast<void>(Builder.createCondBranch(Owner.argument(0), Left, Right));
    Builder.setInsertPoint(Left);
    LoadInstruction &TypeValue = Builder.createLoad(Owner.argument(1));
    static_cast<void>(Builder.createBranch(Merge));
    Builder.setInsertPoint(Right);
    static_cast<void>(Builder.createAlloca(TypeValue));
    static_cast<void>(Builder.createBranch(Merge));
    Builder.setInsertPoint(Merge);
    static_cast<void>(Builder.createReturn());
    EXPECT_DEATH(verifyFunction(Owner, Engine), "INK-I0001");
  }
} // namespace ink::ir::test
