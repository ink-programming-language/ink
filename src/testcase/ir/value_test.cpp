#include "ink/ir/type/value.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
#include "ink/ir/instruction/icmp_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/type/type_context.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
    constexpr ValueKind UnknownKind = static_cast<ValueKind>(std::numeric_limits<std::uint8_t>::max());

    struct TypeQueryCase
    {
        const Value *ValueObject;
        const Type *ExpectedType;
    };

    class GenericInstruction final : public Instruction
    {
      public:
        explicit GenericInstruction(const Type *ResultType = nullptr) noexcept
            : Instruction(ValueKind::Instruction, ResultType)
        {
        }
    };

    class UnknownValue final : public Value
    {
      public:
        UnknownValue() noexcept
            : Value(UnknownKind)
        {
        }
    };

    class TypedUnknownValue final : public Value
    {
      public:
        explicit TypedUnknownValue(const Type &RecordedType) noexcept
            : Value(UnknownKind),
              RecordedType(RecordedType)
        {
        }

        const Type *type() const noexcept override
        {
          return &RecordedType;
        }

      private:
        const Type &RecordedType;
    };
  } // namespace

  static_assert(std::is_same_v<decltype(std::declval<const Value &>().type()), const Type *>);
  static_assert(noexcept(std::declval<const Value &>().type()));
  static_assert(std::is_same_v<decltype(std::declval<const Function &>().type()), const FunctionType *>);
  static_assert(noexcept(std::declval<const Function &>().type()));

  // Base Value pointers dynamically return each constant family's canonical type, including null literals and string arrays.
  TEST(IRValueTest, QueriesAllConstantFamilies)
  {
    TypeContext Types(Target);
    auto &Constants = Types.constantPool();
    const auto *Array = Types.getArrayType(Types.getBoolType(), 1);
    ASSERT_NE(Array, nullptr);
    const auto *StringType = Types.getArrayType(Types.getUInt8Type(), 5);
    ASSERT_NE(StringType, nullptr);
    const TypeQueryCase Cases[] = {
        {Constants.getBoolConstant(true), Types.getBoolType()},
        {Constants.getSignedIntegerConstant(Types.getInt32Type(), 42), Types.getInt32Type()},
        {Constants.getFloatConstant(3.5F), Types.getFloat32Type()},
        {Constants.getArrayConstant(Array, {Constants.getBoolConstant(false)}), Array},
        {Constants.getNullPointerConstant(), Types.getNullPointerType()},
        {Constants.getStringConstant("value"), StringType},
    };
    for (const auto &Case : Cases)
    {
      ASSERT_NE(Case.ValueObject, nullptr);
      EXPECT_EQ(Case.ValueObject->type(), Case.ExpectedType);
      EXPECT_TRUE(Constants.owns(static_cast<const Constant *>(Case.ValueObject)));
    }
  }

  // Base Value pointers dynamically expose concrete and generic instruction results, including void and an unspecified null type.
  TEST(IRValueTest, QueriesAllInstructionFamilies)
  {
    TypeContext Types(Target);
    const auto *Pointer = Types.getPointerType(Types.getInt32Type());
    const auto *Signature = Types.getFunctionType(Types.getInt32Type());
    ASSERT_NE(Pointer, nullptr);
    ASSERT_NE(Signature, nullptr);
    Function Callee(*Signature);
    BasicBlock Block;
    const auto *True = Types.constantPool().getBoolConstant(true);
    const GetElementPtrInstruction Address(*Pointer, *Types.getInt32Type(), *Types.constantPool().getNullPointerConstant(), {});
    const CallInstruction Call(Callee);
    const ICmpInstruction Compare(*Types.getBoolType(), ICmpPredicate::Equal, *True, *True);
    const BranchInstruction Branch(*Types.getVoidType(), Block);
    const ConditionalBranchInstruction ConditionalBranch(*Types.getVoidType(), Compare, Block, Block);
    const ReturnInstruction Return(*Types.getVoidType());
    const GenericInstruction Typed(Types.getFloat64Type());
    const GenericInstruction Untyped;
    const TypeQueryCase Cases[] = {
        {&Address, Pointer},
        {&Call, Types.getInt32Type()},
        {&Compare, Types.getBoolType()},
        {&Branch, Types.getVoidType()},
        {&ConditionalBranch, Types.getVoidType()},
        {&Return, Types.getVoidType()},
        {&Typed, Types.getFloat64Type()},
        {&Untyped, nullptr},
    };
    for (const auto &Case : Cases)
    {
      EXPECT_EQ(Case.ValueObject->type(), Case.ExpectedType);
    }
    EXPECT_EQ(Address.pointer(), Types.constantPool().getNullPointerConstant());
    EXPECT_EQ(Call.callee(), &Callee);
    EXPECT_EQ(Compare.left(), True);
    EXPECT_EQ(ConditionalBranch.condition(), &Compare);
    EXPECT_EQ(Branch.target(), &Block);
    EXPECT_FALSE(Return.hasReturnedValue());
  }

  // Base Value references return function signatures, parameter types and global storage types without changing object state.
  TEST(IRValueTest, QueriesFunctionParameterAndGlobalWithoutMutation)
  {
    TypeContext Types(Target);
    const auto *Signature = Types.getFunctionType(Types.getInt32Type(), {Types.getBoolType()});
    ASSERT_NE(Signature, nullptr);
    const auto Name = core::NamePool::instance().intern("IRValueTest.Named");
    ASSERT_TRUE(Name.valid());
    Function Callable(*Signature, Name);
    auto *ParameterValue = Callable.parameter(0);
    ASSERT_NE(ParameterValue, nullptr);
    ParameterValue->setName(Name);
    GlobalVariable Global(*Types.getBoolType(), Name);
    const auto *Initializer = Types.constantPool().getBoolConstant(true);
    ASSERT_TRUE(Global.setInitializer(Initializer));
    const Value &CallableValue = Callable;
    const Value &FormalValue = *ParameterValue;
    const Value &GlobalValue = Global;
    EXPECT_EQ(CallableValue.type(), Signature);
    EXPECT_EQ(Callable.type(), Signature);
    EXPECT_EQ(Callable.type(), Callable.functionType());
    EXPECT_EQ(FormalValue.type(), Types.getBoolType());
    EXPECT_EQ(GlobalValue.type(), Types.getBoolType());
    EXPECT_EQ(Callable.name(), Name);
    EXPECT_EQ(Callable.parameterCount(), 1U);
    EXPECT_EQ(Callable.blockCount(), 0U);
    EXPECT_EQ(&ParameterValue->function(), &Callable);
    EXPECT_EQ(ParameterValue->index(), 0U);
    EXPECT_EQ(ParameterValue->name(), Name);
    EXPECT_EQ(Global.name(), Name);
    EXPECT_EQ(Global.initializer(), Initializer);
  }

  // Type descriptors, basic blocks and unknown value kinds have no operand type even when the descriptor itself has a runtime layout.
  TEST(IRValueTest, RejectsTypesBlocksAndUnknownValues)
  {
    TypeContext Types(Target);
    const Type *Descriptors[] = {
        Types.getVoidType(),
        Types.getVariadicType(),
        Types.getBoolType(),
        Types.getNullPointerType(),
        Types.getInt32Type(),
        Types.getFloat64Type(),
        Types.getArrayType(Types.getBoolType(), 1),
        Types.getPointerType(Types.getInt32Type()),
        Types.getReferenceType(Types.getInt32Type()),
        Types.getFunctionType(Types.getVoidType()),
        Types.createClassType(8, 8),
        Types.createInterfaceType(8, 8),
        Types.createStructType(8, 8),
    };
    for (const auto *Descriptor : Descriptors)
    {
      ASSERT_NE(Descriptor, nullptr);
      const Value &DescriptorValue = *Descriptor;
      EXPECT_EQ(DescriptorValue.type(), nullptr);
    }
    BasicBlock Block;
    const UnknownValue Unknown;
    const Value &BlockValue = Block;
    const Value &UnknownValueObject = Unknown;
    EXPECT_EQ(BlockValue.type(), nullptr);
    EXPECT_EQ(UnknownValueObject.type(), nullptr);
    EXPECT_EQ(Block.function(), nullptr);
    EXPECT_EQ(Block.instructionCount(), 0U);
  }

  // A new Value subclass can supply a type through virtual dispatch even when its kind has no registered classifier.
  TEST(IRValueTest, DispatchesCustomTypeOverrideIndependentlyOfKind)
  {
    TypeContext Types(Target);
    const TypedUnknownValue Typed(*Types.getInt64Type());
    const UnknownValue Untyped;
    const Value &TypedValue = Typed;
    const Value *UntypedValue = &Untyped;
    EXPECT_EQ(TypedValue.kind(), UntypedValue->kind());
    EXPECT_EQ(valueKindName(TypedValue.kind()), "unknown");
    EXPECT_EQ(TypedValue.type(), Types.getInt64Type());
    EXPECT_EQ(UntypedValue->type(), nullptr);
  }
} // namespace ink::ir::test
