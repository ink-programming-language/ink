#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
#include "ink/ir/instruction/icmp_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <memory>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class TypedInstruction final : public Instruction
    {
      public:
        explicit TypedInstruction(const Type *ResultType) noexcept
            : Instruction(ValueKind::Instruction, ResultType)
        {
        }
    };

    class UnpooledConstant final : public Constant
    {
      public:
        explicit UnpooledConstant(const Type *ConstantType) noexcept
            : Constant(ValueKind::IntegerConstant, ConstantType)
        {
        }
    };
  } // namespace

  // Insertion starts unset, switches between owned blocks, and survives rejected detached or foreign-module blocks.
  TEST(IRBuilderTest, ManagesInsertionPointAndPreservesItOnRejection)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    Function Other(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    Function Foreign(*ForeignTypes.typePool().getFunctionType(ForeignTypes.typePool().getVoidType()));
    auto *Entry = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    auto *ForeignEntry = Foreign.createBlock();
    BasicBlock Detached;
    IRBuilder Builder(Types.irContext());
    EXPECT_EQ(&Builder.irContext(), &Types.irContext());
    EXPECT_EQ(&Builder.constantPool(), &Types.constantPool());
    const IRBuilder &ReadOnly = Builder;
    EXPECT_EQ(&ReadOnly.irContext(), &Types.irContext());
    EXPECT_EQ(&ReadOnly.constantPool(), &Types.constantPool());
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    EXPECT_EQ(Builder.insertFunction(), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.insertFunction(), &Owner);
    EXPECT_FALSE(Builder.setInsertPoint(Detached));
    EXPECT_FALSE(Builder.setInsertPoint(*ForeignEntry));
    EXPECT_EQ(Builder.insertBlock(), Entry);
    EXPECT_EQ(Builder.insertFunction(), &Owner);
    ASSERT_TRUE(Builder.setInsertPoint(*OtherEntry));
    EXPECT_EQ(Builder.insertFunction(), &Other);
    Builder.clearInsertPoint();
    EXPECT_EQ(Builder.insertBlock(), nullptr);
    EXPECT_EQ(Builder.insertFunction(), nullptr);
  }

  // Every creation entry point rejects a missing insertion block without altering available functions or blocks.
  TEST(IRBuilderTest, RejectsEveryCreationWithoutInsertionPoint)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    auto *Entry = Owner.createBlock();
    auto *Integer = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    auto *Pointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    auto *Null = Types.constantPool().getNullPointerConstant();
    IRBuilder Builder(Types.irContext());
    EXPECT_EQ(Builder.createCall(Owner), nullptr);
    EXPECT_EQ(Builder.createCall(*Owner.functionType(), Owner), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Integer, *Integer), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Types.typePool().getInt32Type(), *Null, {}), nullptr);
    EXPECT_EQ(Builder.createBranch(*Entry), nullptr);
    EXPECT_EQ(Builder.createConditionalBranch(*Types.constantPool().getBoolConstant(true), *Entry, *Entry), nullptr);
    EXPECT_EQ(Builder.createReturn(), nullptr);
    EXPECT_EQ(Builder.createReturn(*Integer), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), nullptr);
  }

  // Appended GEP, call, comparison and branches form a function whose owned results are reused across blocks.
  TEST(IRBuilderTest, BuildsConnectedFunctionAndAppendsInOrder)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Array = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 8);
    auto *ArrayPointer = Types.typePool().getPointerType(Array);
    auto *ElementPointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    Function Callee(*Types.typePool().getFunctionType(Types.typePool().getInt32Type(), {ElementPointer}));
    Function Caller(*Types.typePool().getFunctionType(Types.typePool().getInt32Type(), {ArrayPointer, Types.typePool().getInt32Type()}));
    auto *Entry = Caller.createBlock();
    auto *Accepted = Caller.createBlock();
    auto *Rejected = Caller.createBlock();
    auto *Exit = Caller.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Address = Builder.createGetElementPtr(*ElementPointer, *Array, *Caller.parameter(0), {Zero, Caller.parameter(1)}, true);
    ASSERT_NE(Address, nullptr);
    auto *Call = Builder.createCall(Callee, {Argument(*Address)});
    ASSERT_NE(Call, nullptr);
    auto *Compare = Builder.createICmp(ICmpPredicate::SignedGreaterThan, *Call, *Zero);
    ASSERT_NE(Compare, nullptr);
    auto *Branch = Builder.createConditionalBranch(*Compare, *Accepted, *Rejected);
    ASSERT_NE(Branch, nullptr);
    ASSERT_EQ(Entry->instructionCount(), 4U);
    EXPECT_EQ(Entry->instruction(0), Address);
    EXPECT_EQ(Entry->instruction(1), Call);
    EXPECT_EQ(Entry->instruction(2), Compare);
    EXPECT_EQ(Entry->instruction(3), Branch);
    EXPECT_EQ(Address->type(), ElementPointer);
    EXPECT_EQ(Address->index(1), Caller.parameter(1));
    EXPECT_TRUE(Address->isInBounds());
    EXPECT_EQ(Call->callee(), &Callee);
    EXPECT_EQ(Call->argument(0)->value(), Address);
    EXPECT_EQ(Compare->type(), Types.typePool().getBoolType());
    EXPECT_EQ(Compare->left(), Call);
    EXPECT_EQ(Branch->condition(), Compare);
    EXPECT_EQ(Branch->trueTarget(), Accepted);
    EXPECT_EQ(Branch->falseTarget(), Rejected);
    EXPECT_EQ(Builder.insertBlock(), Entry);
    ASSERT_TRUE(Builder.setInsertPoint(*Accepted));
    auto *Return = Builder.createReturn(*Call);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Call);
    ASSERT_TRUE(Builder.setInsertPoint(*Rejected));
    auto *Jump = Builder.createBranch(*Exit);
    ASSERT_NE(Jump, nullptr);
    EXPECT_EQ(Jump->target(), Exit);
    ASSERT_TRUE(Builder.setInsertPoint(*Exit));
    EXPECT_NE(Builder.createReturn(*Zero), nullptr);
  }

  // A second builder observes an externally appended terminator even when a later low-level append leaves it inside the block.
  TEST(IRBuilderTest, DetectsAllTerminatorKindsIncludingNonfinalInstructions)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    auto *True = Types.constantPool().getBoolConstant(true);
    IRBuilder First(Types.irContext());
    IRBuilder Second(Types.irContext());
    for (unsigned Kind = 0; Kind < 3; ++Kind)
    {
      auto *Block = Owner.createBlock();
      ASSERT_TRUE(First.setInsertPoint(*Block));
      ASSERT_TRUE(Second.setInsertPoint(*Block));
      if (Kind == 0)
      {
        ASSERT_NE(First.createReturn(), nullptr);
      }
      else if (Kind == 1)
      {
        ASSERT_NE(First.createBranch(*Block), nullptr);
      }
      else
      {
        ASSERT_NE(Block->appendInstruction(std::make_unique<ConditionalBranchInstruction>(*Types.typePool().getVoidType(), *True, *Block, *Block)), nullptr);
      }
      ASSERT_NE(Block->appendInstruction(std::make_unique<ICmpInstruction>(*Types.typePool().getBoolType(), ICmpPredicate::Equal, *Zero, *Zero)), nullptr);
      EXPECT_TRUE(Second.setInsertPoint(*Block));
      EXPECT_EQ(Second.createICmp(ICmpPredicate::Equal, *Zero, *Zero), nullptr);
      EXPECT_EQ(Second.createCall(Owner), nullptr);
      EXPECT_EQ(Second.createCall(*Owner.functionType(), Owner), nullptr);
      EXPECT_EQ(Second.createGetElementPtr(*Types.typePool().getPointerType(Types.typePool().getInt32Type()), *Types.typePool().getInt32Type(), *Types.constantPool().getNullPointerConstant(), {}), nullptr);
      EXPECT_EQ(Second.createReturn(), nullptr);
      EXPECT_EQ(Second.createReturn(*Zero), nullptr);
      EXPECT_EQ(Second.createBranch(*Block), nullptr);
      EXPECT_EQ(Second.createConditionalBranch(*True, *Block, *Block), nullptr);
      EXPECT_EQ(Block->instructionCount(), 2U);
      EXPECT_EQ(Second.insertBlock(), Block);
    }
  }

  // Direct calls require exact arity and type identity while retaining named argument records on success.
  TEST(IRBuilderTest, ValidatesDirectCallArgumentsAndSignatureContext)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    Function Caller(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    Function Callee(*Types.typePool().getFunctionType(Types.typePool().getInt32Type(), {Types.typePool().getInt32Type()}));
    Function Foreign(*ForeignTypes.typePool().getFunctionType(ForeignTypes.typePool().getInt32Type(), {ForeignTypes.typePool().getInt32Type()}));
    auto *Entry = Caller.createBlock();
    auto *Integer = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 7);
    auto *Unsigned = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt32Type(), 7);
    auto Name = core::NamePool::instance().intern("builder_argument");
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(Callee), nullptr);
    EXPECT_EQ(Builder.createCall(Callee, {Argument(*Integer), Argument(*Integer)}), nullptr);
    EXPECT_EQ(Builder.createCall(Callee, {Argument(*Unsigned)}), nullptr);
    EXPECT_EQ(Builder.createCall(Foreign, {Argument(*Integer)}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *Call = Builder.createCall(Callee, {Argument(*Integer, Name)});
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->type(), Types.typePool().getInt32Type());
    EXPECT_EQ(Call->argument(0)->value(), Integer);
    EXPECT_EQ(Call->argument(0)->name(), Name);
    EXPECT_EQ(Entry->instructionCount(), 1U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // A local canonical null initializes a pointer parameter but foreign nulls and null-to-integer arguments are rejected.
  TEST(IRBuilderTest, AllowsOnlyLocalNullForPointerCallArguments)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    auto *Pointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    Function Caller(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    Function PointerCallee(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer}));
    Function IntegerCallee(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getInt32Type()}));
    auto *Entry = Caller.createBlock();
    auto *Null = Types.constantPool().getNullPointerConstant();
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(PointerCallee, {Argument(*ForeignTypes.constantPool().getNullPointerConstant())}), nullptr);
    EXPECT_EQ(Builder.createCall(IntegerCallee, {Argument(*Null)}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *Call = Builder.createCall(PointerCallee, {Argument(*Null)});
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->argument(0)->value(), Null);
  }

  // Indirect calls accept owned function-valued and function-pointer parameters only with their exact signature.
  TEST(IRBuilderTest, AcceptsFunctionAndFunctionPointerIndirectCalls)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Signature = Types.typePool().getFunctionType(Types.typePool().getInt32Type(), {Types.typePool().getInt32Type()});
    auto *OtherSignature = Types.typePool().getFunctionType(Types.typePool().getUInt32Type(), {Types.typePool().getInt32Type()});
    auto *CallablePointer = Types.typePool().getPointerType(Signature);
    Function Caller(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Signature, CallablePointer, Types.typePool().getInt32Type()}));
    Function Direct(*Signature);
    auto *Entry = Caller.createBlock();
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(*OtherSignature, *Caller.parameter(0), {Argument(*Caller.parameter(2))}), nullptr);
    EXPECT_EQ(Builder.createCall(*OtherSignature, *Caller.parameter(1), {Argument(*Caller.parameter(2))}), nullptr);
    EXPECT_EQ(Builder.createCall(*OtherSignature, Direct, {Argument(*Caller.parameter(2))}), nullptr);
    auto *FunctionCall = Builder.createCall(*Signature, *Caller.parameter(0), {Argument(*Caller.parameter(2))});
    auto *PointerCall = Builder.createCall(*Signature, *Caller.parameter(1), {Argument(*Caller.parameter(2))});
    auto *DirectCall = Builder.createCall(*Signature, Direct, {Argument(*Caller.parameter(2))});
    ASSERT_NE(FunctionCall, nullptr);
    ASSERT_NE(PointerCall, nullptr);
    ASSERT_NE(DirectCall, nullptr);
    EXPECT_EQ(FunctionCall->directCallee(), nullptr);
    EXPECT_EQ(PointerCall->directCallee(), nullptr);
    EXPECT_EQ(DirectCall->directCallee(), &Direct);
    EXPECT_EQ(Entry->instructionCount(), 3U);
  }

  // An explicit signature cannot make null, a type object, an integer or an unrelated function's parameter callable.
  TEST(IRBuilderTest, RejectsInvalidIndirectCallees)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Signature = Types.typePool().getFunctionType(Types.typePool().getVoidType());
    auto *CallablePointer = Types.typePool().getPointerType(Signature);
    Function Caller(*Signature);
    Function Other(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {CallablePointer}));
    auto *Entry = Caller.createBlock();
    TypedInstruction Detached(CallablePointer);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(*Signature, *Types.constantPool().getNullPointerConstant()), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, *Signature), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, *Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0)), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, *Other.parameter(0)), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, Detached), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
  }

  // Void and value returns match the enclosing signature, and rejected overloads leave the block open for a valid return.
  TEST(IRBuilderTest, MatchesReturnValueToEnclosingSignature)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    Function VoidFunction(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    Function ValueFunction(*Types.typePool().getFunctionType(Types.typePool().getInt32Type()));
    auto *VoidEntry = VoidFunction.createBlock();
    auto *ValueEntry = ValueFunction.createBlock();
    auto *Integer = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 42);
    auto *Unsigned = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt32Type(), 42);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*VoidEntry));
    EXPECT_EQ(Builder.createReturn(*Integer), nullptr);
    EXPECT_EQ(VoidEntry->instructionCount(), 0U);
    auto *VoidReturn = Builder.createReturn();
    ASSERT_NE(VoidReturn, nullptr);
    EXPECT_FALSE(VoidReturn->hasReturnedValue());
    EXPECT_EQ(VoidReturn->type(), Types.typePool().getVoidType());
    ASSERT_TRUE(Builder.setInsertPoint(*ValueEntry));
    EXPECT_EQ(Builder.createReturn(), nullptr);
    EXPECT_EQ(Builder.createReturn(*Unsigned), nullptr);
    EXPECT_EQ(ValueEntry->instructionCount(), 0U);
    auto *Return = Builder.createReturn(*Integer);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Integer);
  }

  // Pointer returns accept the local null literal but reject a foreign null without terminating the function.
  TEST(IRBuilderTest, ReturnsCanonicalLocalNullForPointerResult)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getPointerType(Types.typePool().getInt32Type())));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createReturn(*ForeignTypes.constantPool().getNullPointerConstant()), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *Return = Builder.createReturn(*Types.constantPool().getNullPointerConstant());
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Types.constantPool().getNullPointerConstant());
  }

  // Returns reject unpooled constants, type objects, blocks, void results, untyped instructions and foreign constants.
  TEST(IRBuilderTest, RejectsUnownedConstantsAndNonvalueOperands)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getInt32Type()));
    Function VoidCallee(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    UnpooledConstant Unowned(Types.typePool().getInt32Type());
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *VoidCall = Builder.createCall(VoidCallee);
    ASSERT_NE(VoidCall, nullptr);
    auto *Untyped = Entry->appendInstruction(std::make_unique<TypedInstruction>(nullptr));
    ASSERT_NE(Untyped, nullptr);
    EXPECT_EQ(Builder.createReturn(Unowned), nullptr);
    EXPECT_EQ(Builder.createReturn(*Types.typePool().getInt32Type()), nullptr);
    EXPECT_EQ(Builder.createReturn(*Entry), nullptr);
    EXPECT_EQ(Builder.createReturn(*VoidCall), nullptr);
    EXPECT_EQ(Builder.createReturn(*Untyped), nullptr);
    EXPECT_EQ(Builder.createReturn(*ForeignTypes.constantPool().getSignedIntegerConstant(ForeignTypes.typePool().getInt32Type(), 0)), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, Unowned, *Zero), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 2U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // Parameters and instructions require ownership by the current function while a different block in that function is allowed.
  TEST(IRBuilderTest, EnforcesOperandFunctionOwnershipWithoutCheckingDominance)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    auto *Signature = Types.typePool().getFunctionType(Types.typePool().getInt32Type(), {Types.typePool().getInt32Type()});
    Function Owner(*Signature);
    Function Other(*Signature);
    auto *Entry = Owner.createBlock();
    auto *Sibling = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    auto *Owned = Sibling->appendInstruction(std::make_unique<TypedInstruction>(Types.typePool().getInt32Type()));
    auto *Unrelated = OtherEntry->appendInstruction(std::make_unique<TypedInstruction>(Types.typePool().getInt32Type()));
    auto *ForeignTyped = Sibling->appendInstruction(std::make_unique<TypedInstruction>(ForeignTypes.typePool().getInt32Type()));
    TypedInstruction Detached(Types.typePool().getInt32Type());
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Owner.parameter(0), *Other.parameter(0)), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Owner.parameter(0), Detached), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Owner.parameter(0), *Unrelated), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *ForeignTyped, *ForeignTyped), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_NE(Builder.createICmp(ICmpPredicate::Equal, *Owner.parameter(0), *Owned), nullptr);
    EXPECT_NE(Builder.createReturn(*Owned), nullptr);
  }

  // All declared integer comparison predicates retain their identity and accept matching integer or pointer operands.
  TEST(IRBuilderTest, AcceptsEveryComparisonPredicateForMatchingIntegerAndPointerTypes)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Pointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, Pointer}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    constexpr ICmpPredicate Predicates[] = {
        ICmpPredicate::Equal,
        ICmpPredicate::NotEqual,
        ICmpPredicate::UnsignedGreaterThan,
        ICmpPredicate::UnsignedGreaterEqual,
        ICmpPredicate::UnsignedLessThan,
        ICmpPredicate::UnsignedLessEqual,
        ICmpPredicate::SignedGreaterThan,
        ICmpPredicate::SignedGreaterEqual,
        ICmpPredicate::SignedLessThan,
        ICmpPredicate::SignedLessEqual,
    };
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    for (auto Predicate : Predicates)
    {
      auto *IntegerCompare = Builder.createICmp(Predicate, *Zero, *Zero);
      auto *PointerCompare = Builder.createICmp(Predicate, *Owner.parameter(0), *Owner.parameter(1));
      ASSERT_NE(IntegerCompare, nullptr);
      ASSERT_NE(PointerCompare, nullptr);
      EXPECT_EQ(IntegerCompare->predicate(), Predicate);
      EXPECT_EQ(PointerCompare->predicate(), Predicate);
    }
    EXPECT_EQ(Entry->instructionCount(), 20U);
  }

  // Integer comparisons reject booleans, floats, different integer or pointer types, and invalid predicate enumerators.
  TEST(IRBuilderTest, RejectsInvalidComparisonTypesAndPredicates)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Pointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    auto *OtherPointer = Types.typePool().getPointerType(Types.typePool().getUInt32Type());
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, OtherPointer}));
    auto *Entry = Owner.createBlock();
    auto *Integer = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    auto *Unsigned = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt32Type(), 0);
    auto *Boolean = Types.constantPool().getBoolConstant(false);
    auto *Float = Types.constantPool().getFloatConstant(0.0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Boolean, *Boolean), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Float, *Float), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Integer, *Unsigned), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Owner.parameter(0), *Owner.parameter(1)), nullptr);
    EXPECT_EQ(Builder.createICmp(static_cast<ICmpPredicate>(255), *Integer, *Integer), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
  }

  // Null comparisons require one concrete pointer and equality or inequality, in either operand order.
  TEST(IRBuilderTest, RestrictsNullComparisonsToPointerEquality)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    auto *Pointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer}));
    auto *Entry = Owner.createBlock();
    auto *Null = Types.constantPool().getNullPointerConstant();
    auto *Value = Owner.parameter(0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Null, *Null), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::SignedLessThan, *Value, *Null), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::UnsignedGreaterThan, *Null, *Value), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Value, *ForeignTypes.constantPool().getNullPointerConstant()), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_NE(Builder.createICmp(ICmpPredicate::Equal, *Value, *Null), nullptr);
    EXPECT_NE(Builder.createICmp(ICmpPredicate::Equal, *Null, *Value), nullptr);
    EXPECT_NE(Builder.createICmp(ICmpPredicate::NotEqual, *Value, *Null), nullptr);
    EXPECT_NE(Builder.createICmp(ICmpPredicate::NotEqual, *Null, *Value), nullptr);
  }

  // Branch destinations stay in the current function, conditional branches require bool, and self-edges are permitted.
  TEST(IRBuilderTest, ChecksBranchTargetsAndBooleanConditions)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    Function Other(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    auto *Entry = Owner.createBlock();
    auto *Sibling = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    BasicBlock Detached;
    auto *True = Types.constantPool().getBoolConstant(true);
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createBranch(Detached), nullptr);
    EXPECT_EQ(Builder.createBranch(*OtherEntry), nullptr);
    EXPECT_EQ(Builder.createConditionalBranch(*Zero, *Entry, *Sibling), nullptr);
    EXPECT_EQ(Builder.createConditionalBranch(*True, *OtherEntry, *Sibling), nullptr);
    EXPECT_EQ(Builder.createConditionalBranch(*True, *Sibling, Detached), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
    EXPECT_NE(Builder.createConditionalBranch(*True, *Entry, *Sibling), nullptr);
    ASSERT_TRUE(Builder.setInsertPoint(*Sibling));
    EXPECT_NE(Builder.createBranch(*Sibling), nullptr);
  }

  // Empty GEP index lists preserve the source type and the first index performs pointer stepping without selecting an element.
  TEST(IRBuilderTest, PreservesSourceForEmptyAndSingleIndexGEP)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Array = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 4);
    auto *Pointer = Types.typePool().getPointerType(Array);
    auto *ElementPointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer}));
    auto *Entry = Owner.createBlock();
    auto *One = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 1);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Empty = Builder.createGetElementPtr(*Pointer, *Array, *Owner.parameter(0), {});
    auto *Step = Builder.createGetElementPtr(*Pointer, *Array, *Owner.parameter(0), {One});
    ASSERT_NE(Empty, nullptr);
    ASSERT_NE(Step, nullptr);
    EXPECT_EQ(Empty->indexCount(), 0U);
    EXPECT_FALSE(Empty->isInBounds());
    EXPECT_EQ(Step->index(0), One);
    EXPECT_EQ(Step->sourceElementType(), Array);
    EXPECT_EQ(Builder.createGetElementPtr(*ElementPointer, *Array, *Owner.parameter(0), {One}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 2U);
  }

  // Array indexing accepts dynamic, negative and beyond-bound integer offsets without proving the inbounds promise.
  TEST(IRBuilderTest, IndexesNestedArraysWithoutRuntimeBoundsChecks)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Inner = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 2);
    auto *Outer = Types.typePool().getArrayType(Inner, 3);
    auto *Pointer = Types.typePool().getPointerType(Outer);
    auto *ElementPointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, Types.typePool().getInt32Type()}));
    auto *Entry = Owner.createBlock();
    auto *Negative = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), -1);
    auto *Large = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt64Type(), 1000);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Address = Builder.createGetElementPtr(*ElementPointer, *Outer, *Owner.parameter(0), {Negative, Owner.parameter(1), Large}, true);
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->indexCount(), 3U);
    EXPECT_EQ(Address->index(0), Negative);
    EXPECT_EQ(Address->index(1), Owner.parameter(1));
    EXPECT_EQ(Address->index(2), Large);
    EXPECT_TRUE(Address->isInBounds());
  }

  // A constant field index selects nominal metadata before a later dynamic array index selects its element.
  TEST(IRBuilderTest, IndexesNominalFieldsAndNestedArrays)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Array = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 4);
    auto *Record = Types.createStructType(24, 8);
    ASSERT_TRUE(Record->addField(Field(Types.typePool().getInt64Type(), core::NamePool::instance().intern("builder_header"), 0)));
    ASSERT_TRUE(Record->addField(Field(Array, core::NamePool::instance().intern("builder_values"), 8)));
    auto *Pointer = Types.typePool().getPointerType(Record);
    auto *ElementPointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, Types.typePool().getInt32Type()}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    auto *One = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt64Type(), 1);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Address = Builder.createGetElementPtr(*ElementPointer, *Record, *Owner.parameter(0), {Zero, One, Owner.parameter(1)});
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->type(), ElementPointer);
    EXPECT_EQ(Address->sourceElementType(), Record);
    EXPECT_EQ(Address->index(1), One);
  }

  // Nominal selection rejects dynamic, negative and missing field indices without changing the insertion block.
  TEST(IRBuilderTest, RejectsInvalidNominalFieldIndices)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Record = Types.createStructType(4, 4);
    ASSERT_TRUE(Record->addField(Field(Types.typePool().getInt32Type(), core::NamePool::instance().intern("builder_field"))));
    auto *Pointer = Types.typePool().getPointerType(Record);
    auto *ElementPointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, Types.typePool().getInt32Type()}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    auto *Negative = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), -1);
    auto *PastEnd = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt64Type(), 1);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createGetElementPtr(*ElementPointer, *Record, *Owner.parameter(0), {Zero, Owner.parameter(1)}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*ElementPointer, *Record, *Owner.parameter(0), {Zero, Negative}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*ElementPointer, *Record, *Owner.parameter(0), {Zero, PastEnd}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
    EXPECT_NE(Builder.createGetElementPtr(*ElementPointer, *Record, *Owner.parameter(0), {Zero, Zero}), nullptr);
  }

  // GEP rejects absent, noninteger and foreign indices together with incompatible pointer, source and result types.
  TEST(IRBuilderTest, RejectsInvalidGEPOperandsAndForeignTypes)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    auto *Integer = Types.typePool().getInt32Type();
    auto *Pointer = Types.typePool().getPointerType(Integer);
    auto *OtherPointer = Types.typePool().getPointerType(Types.typePool().getUInt32Type());
    auto *ForeignPointer = ForeignTypes.typePool().getPointerType(ForeignTypes.typePool().getInt32Type());
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer}));
    auto *Entry = Owner.createBlock();
    auto *Null = Types.constantPool().getNullPointerConstant();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Integer, 0);
    auto *ForeignZero = ForeignTypes.constantPool().getSignedIntegerConstant(ForeignTypes.typePool().getInt32Type(), 0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Integer, *Owner.parameter(0), {nullptr}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Integer, *Owner.parameter(0), {Types.constantPool().getBoolConstant(false)}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Integer, *Owner.parameter(0), {Types.constantPool().getFloatConstant(0.0)}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Integer, *Owner.parameter(0), {ForeignZero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Integer, *Zero, {Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*OtherPointer, *Types.typePool().getUInt32Type(), *Owner.parameter(0), {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*OtherPointer, *Integer, *Owner.parameter(0), {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*ForeignPointer, *Integer, *Owner.parameter(0), {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *ForeignTypes.typePool().getInt32Type(), *Null, {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Integer, *ForeignTypes.constantPool().getNullPointerConstant(), {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().getPointerType(Types.typePool().getVoidType()), *Types.typePool().getVoidType(), *Null, {}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *Address = Builder.createGetElementPtr(*Pointer, *Integer, *Null, {Zero});
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->pointer(), Null);
  }

  // Extra indices cannot descend through a scalar or implicitly load through an embedded pointer.
  TEST(IRBuilderTest, RejectsGEPDescentThroughScalarsAndPointers)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Integer = Types.typePool().getInt32Type();
    auto *Pointer = Types.typePool().getPointerType(Integer);
    auto *PointerPointer = Types.typePool().getPointerType(Pointer);
    auto *UnitPointer = Types.typePool().getPointerType(Types.typePool().getVoidType());
    auto *UnitPointerPointer = Types.typePool().getPointerType(UnitPointer);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, PointerPointer, UnitPointer, UnitPointerPointer}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Integer, 0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Integer, *Owner.parameter(0), {Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Pointer, *Owner.parameter(1), {Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*UnitPointer, *Types.typePool().getVoidType(), *Owner.parameter(2), {Zero}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_NE(Builder.createGetElementPtr(*PointerPointer, *Pointer, *Owner.parameter(1), {Zero}), nullptr);
    EXPECT_NE(Builder.createGetElementPtr(*UnitPointerPointer, *UnitPointer, *Owner.parameter(3), {Zero}), nullptr);
  }

  // Unit parameters and calls supply unit arguments, but a terminator in another block cannot supply a unit value.
  TEST(IRBuilderTest, AcceptsUnitArgumentsButRejectsTerminatorOperands)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getVoidType()}));
    Function UnitProducer(*Types.typePool().getFunctionType(Types.typePool().getVoidType()));
    Function UnitConsumer(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Types.typePool().getVoidType()}));
    auto *Entry = Owner.createBlock();
    auto *Sibling = Owner.createBlock();
    auto *Terminator = Sibling->appendInstruction(std::make_unique<ReturnInstruction>(*Types.typePool().getVoidType()));
    ASSERT_NE(Terminator, nullptr);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(UnitConsumer, {Argument(*Terminator)}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *ProducerCall = Builder.createCall(UnitProducer);
    ASSERT_NE(ProducerCall, nullptr);
    auto *ConsumeCall = Builder.createCall(UnitConsumer, {Argument(*ProducerCall)});
    auto *ConsumeParameter = Builder.createCall(UnitConsumer, {Argument(*Owner.parameter(0))});
    ASSERT_NE(ConsumeCall, nullptr);
    ASSERT_NE(ConsumeParameter, nullptr);
    EXPECT_EQ(ConsumeCall->argument(0)->value(), ProducerCall);
    EXPECT_EQ(ConsumeParameter->argument(0)->value(), Owner.parameter(0));
    EXPECT_EQ(Builder.createReturn(*ProducerCall), nullptr);
    EXPECT_NE(Builder.createReturn(), nullptr);
  }

  // Global storage declarations cannot be implicitly loaded or converted to addresses when used as operands.
  TEST(IRBuilderTest, RejectsGlobalsAsDirectValueOperands)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Pointer = Types.typePool().getPointerType(Types.typePool().getInt32Type());
    auto *Signature = Types.typePool().getFunctionType(Types.typePool().getInt32Type(), {Types.typePool().getInt32Type()});
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getInt32Type()));
    Function Callee(*Signature);
    GlobalVariable IntegerGlobal(*Types.typePool().getInt32Type());
    GlobalVariable PointerGlobal(*Pointer);
    GlobalVariable CallableGlobal(*Types.typePool().getPointerType(Signature));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(Callee, {Argument(IntegerGlobal)}), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, CallableGlobal, {Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, IntegerGlobal, *Zero), nullptr);
    EXPECT_EQ(Builder.createReturn(IntegerGlobal), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Types.typePool().getInt32Type(), PointerGlobal, {Zero}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // GEP may select a unit array element or nominal field as its endpoint, but cannot descend further through unit.
  TEST(IRBuilderTest, AllowsUnitEndpointsInAggregateGEP)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *UnitArray = Types.typePool().getArrayType(Types.typePool().getVoidType(), 2);
    ASSERT_NE(UnitArray, nullptr);
    auto *Record = Types.createStructType(0, 1);
    ASSERT_NE(Record, nullptr);
    ASSERT_TRUE(Record->addField(Field(Types.typePool().getVoidType(), core::NamePool::instance().intern("builder_unit"))));
    auto *UnitPointer = Types.typePool().getPointerType(Types.typePool().getVoidType());
    auto *ArrayPointer = Types.typePool().getPointerType(UnitArray);
    auto *RecordPointer = Types.typePool().getPointerType(Record);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {ArrayPointer, RecordPointer}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Element = Builder.createGetElementPtr(*UnitPointer, *UnitArray, *Owner.parameter(0), {Zero, Zero});
    auto *Field = Builder.createGetElementPtr(*UnitPointer, *Record, *Owner.parameter(1), {Zero, Zero});
    ASSERT_NE(Element, nullptr);
    ASSERT_NE(Field, nullptr);
    EXPECT_EQ(Element->type(), UnitPointer);
    EXPECT_EQ(Field->type(), UnitPointer);
    EXPECT_EQ(Builder.createGetElementPtr(*UnitPointer, *UnitArray, *Owner.parameter(0), {Zero, Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*UnitPointer, *Record, *Owner.parameter(1), {Zero, Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*UnitPointer, *Types.typePool().getVoidType(), *Element, {Zero}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 2U);
  }

  // Builder destruction releases no IR, and a replacement builder can continue appending to the same function-owned block.
  TEST(IRBuilderTest, LeavesInstructionOwnershipWithTheBasicBlock)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getBoolType()));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    ICmpInstruction *Compare = nullptr;
    {
      IRBuilder Builder(Types.irContext());
      ASSERT_TRUE(Builder.setInsertPoint(*Entry));
      Compare = Builder.createICmp(ICmpPredicate::Equal, *Zero, *Zero);
      ASSERT_NE(Compare, nullptr);
    }
    ASSERT_EQ(Entry->instructionCount(), 1U);
    EXPECT_EQ(Entry->instruction(0), Compare);
    EXPECT_EQ(Compare->left(), Zero);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Return = Builder.createReturn(*Compare);
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Compare);
    EXPECT_EQ(Entry->instructionCount(), 2U);
  }
} // namespace ink::ir::test
