#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"
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
#include <type_traits>
#include <vector>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
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

  // Instruction creation requires an insertion block even when no operand validation is needed.
  TEST(IRBuilderTest, RequiresInsertionPointForCreation)
  {
#if !INK_ENABLE_ASSERTIONS
    GTEST_SKIP() << "Insertion-state checks require enabled assertions.";
#else
    IRContext TypesContext(Target);
    IRBuilder Builder(TypesContext);
    EXPECT_DEATH(static_cast<void>(Builder.createReturn()), "check failed:");
    EXPECT_EQ(Builder.insertBlock(), nullptr);
#endif
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
    auto *Address = Builder.createGetElementPtr(*Array, *Caller.parameter(0), {Zero, Caller.parameter(1)}, true);
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

  // All terminator kinds prevent further creation through another builder, including after a later low-level instruction.
  TEST(IRBuilderTest, RequiresUnterminatedInsertionBlock)
  {
#if !INK_ENABLE_ASSERTIONS
    GTEST_SKIP() << "Insertion-state checks require enabled assertions.";
#else
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
      EXPECT_DEATH(static_cast<void>(Second.createICmp(ICmpPredicate::Equal, *Zero, *Zero)), "check failed:");
      EXPECT_EQ(Block->instructionCount(), 2U);
      EXPECT_EQ(Second.insertBlock(), Block);
    }
#endif
  }

  // Signature descriptors cannot enter runtime operand APIs, even before builder validation.
  static_assert(!std::is_convertible_v<const FunctionTypeParameter &, const Value &>);
  static_assert(!std::is_constructible_v<Argument, const FunctionTypeParameter &>);

  // Variadic signatures and formal packs are not silently accepted as ordinary scalar calls or operands.
  TEST(IRBuilderTest, RejectsVariadicCallsAndScalarPackUses)
  {
    IRContext Context(Target);
    auto &Types = Context.typePool();
    auto *Integer = Types.getInt32Type();
    auto *Signature = Types.getFunctionType(Integer, {{Integer, true}});
    ASSERT_NE(Signature, nullptr);
    Function Callee(*Signature);
    Function Caller(*Types.getFunctionType(Integer, {Types.getPointerType(Signature), {Integer, true}}));
    Function FixedCallee(*Types.getFunctionType(Integer, {Integer}));
    auto *Entry = Caller.createBlock();
    auto *Zero = Context.constantPool().getSignedIntegerConstant(Integer, 0);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(Callee), nullptr);
    EXPECT_EQ(Builder.createCall(Callee, {Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createCall(Callee, {Argument(*Zero), Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, *Caller.parameter(0), {Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createCall(FixedCallee, {Argument(*Caller.parameter(1))}), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Caller.parameter(1), *Zero), nullptr);
    EXPECT_EQ(Builder.createReturn(*Caller.parameter(1)), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
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

  // Empty and single-index GEPs infer the source pointer type because the first index steps over objects without selecting an element.
  TEST(IRBuilderTest, PreservesSourceForEmptyAndSingleIndexGEP)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Array = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 4);
    auto *Pointer = Types.typePool().getPointerType(Array);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer}));
    auto *Entry = Owner.createBlock();
    auto *One = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 1);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Empty = Builder.createGetElementPtr(*Array, *Owner.parameter(0), {});
    auto *Step = Builder.createGetElementPtr(*Array, *Owner.parameter(0), {One});
    ASSERT_NE(Empty, nullptr);
    ASSERT_NE(Step, nullptr);
    EXPECT_EQ(Empty->indexCount(), 0U);
    EXPECT_FALSE(Empty->isInBounds());
    EXPECT_EQ(Empty->type(), Pointer);
    EXPECT_EQ(Step->type(), Pointer);
    EXPECT_EQ(Step->index(0), One);
    EXPECT_EQ(Step->sourceElementType(), Array);
    EXPECT_EQ(Entry->instructionCount(), 2U);
  }

  // Nested-array GEPs create their inferred element pointer type without prior pooling and accept offsets without proving the inbounds promise.
  TEST(IRBuilderTest, IndexesNestedArraysWithoutRuntimeBoundsChecks)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Inner = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 2);
    auto *Outer = Types.typePool().getArrayType(Inner, 3);
    auto *Pointer = Types.typePool().getPointerType(Outer);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, Types.typePool().getInt32Type()}));
    auto *Entry = Owner.createBlock();
    auto *Negative = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), -1);
    auto *Large = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt64Type(), 1000);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Address = Builder.createGetElementPtr(*Outer, *Owner.parameter(0), {Negative, Owner.parameter(1), Large}, true);
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->type(), Types.typePool().getPointerType(Types.typePool().getInt32Type()));
    EXPECT_EQ(Address->sourceElementType(), Outer);
    EXPECT_EQ(Address->indexCount(), 3U);
    EXPECT_EQ(Address->index(0), Negative);
    EXPECT_EQ(Address->index(1), Owner.parameter(1));
    EXPECT_EQ(Address->index(2), Large);
    EXPECT_TRUE(Address->isInBounds());
  }

  // Nominal-field metadata followed by a dynamic array index determines the inferred element pointer type.
  TEST(IRBuilderTest, IndexesNominalFieldsAndNestedArrays)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Array = Types.typePool().getArrayType(Types.typePool().getInt32Type(), 4);
    auto *Record = Types.createStructType(24, 8);
    ASSERT_TRUE(Record->addField(Field(Types.typePool().getInt64Type(), core::NamePool::instance().intern("builder_header"), 0)));
    ASSERT_TRUE(Record->addField(Field(Array, core::NamePool::instance().intern("builder_values"), 8)));
    auto *Pointer = Types.typePool().getPointerType(Record);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, Types.typePool().getInt32Type()}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    auto *One = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt64Type(), 1);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Address = Builder.createGetElementPtr(*Record, *Owner.parameter(0), {Zero, One, Owner.parameter(1)});
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->type(), Types.typePool().getPointerType(Types.typePool().getInt32Type()));
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
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer, Types.typePool().getInt32Type()}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    auto *Negative = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), -1);
    auto *PastEnd = Types.constantPool().getUnsignedIntegerConstant(Types.typePool().getUInt64Type(), 1);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createGetElementPtr(*Record, *Owner.parameter(0), {Zero, Owner.parameter(1)}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Record, *Owner.parameter(0), {Zero, Negative}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Record, *Owner.parameter(0), {Zero, PastEnd}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
    EXPECT_NE(Builder.createGetElementPtr(*Record, *Owner.parameter(0), {Zero, Zero}), nullptr);
  }

  // GEP rejects invalid indices, source types and pointer operands while inferring a typed result from a local null literal.
  TEST(IRBuilderTest, RejectsInvalidGEPOperandsAndForeignTypes)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    IRContext ForeignTypesContext(Target);
    Module ForeignTypes(ForeignTypesContext);
    auto *Integer = Types.typePool().getInt32Type();
    auto *Pointer = Types.typePool().getPointerType(Integer);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {Pointer}));
    auto *Entry = Owner.createBlock();
    auto *Null = Types.constantPool().getNullPointerConstant();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Integer, 0);
    auto *ForeignZero = ForeignTypes.constantPool().getSignedIntegerConstant(ForeignTypes.typePool().getInt32Type(), 0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {nullptr}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {Types.constantPool().getBoolConstant(false)}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {Types.constantPool().getFloatConstant(0.0)}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {ForeignZero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Zero, {Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().getUInt32Type(), *Owner.parameter(0), {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*ForeignTypes.typePool().getInt32Type(), *Null, {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *ForeignTypes.constantPool().getNullPointerConstant(), {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().getVoidType(), *Null, {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().getVariadicType(), *Null, {}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *Address = Builder.createGetElementPtr(*Integer, *Null, {Zero});
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->pointer(), Null);
    EXPECT_EQ(Address->type(), Pointer);
    EXPECT_EQ(Address->sourceElementType(), Integer);
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
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Owner.parameter(1), {Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().getVoidType(), *Owner.parameter(2), {Zero}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *PointerAddress = Builder.createGetElementPtr(*Pointer, *Owner.parameter(1), {Zero});
    auto *UnitPointerAddress = Builder.createGetElementPtr(*UnitPointer, *Owner.parameter(3), {Zero});
    ASSERT_NE(PointerAddress, nullptr);
    ASSERT_NE(UnitPointerAddress, nullptr);
    EXPECT_EQ(PointerAddress->type(), PointerPointer);
    EXPECT_EQ(UnitPointerAddress->type(), UnitPointerPointer);
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
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().getInt32Type(), PointerGlobal, {Zero}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // A module's Value identity cannot be used as a callee, argument, comparison, branch condition, return or GEP operand.
  TEST(IRBuilderTest, RejectsModulesAsOperandsWithoutChangingInsertionState)
  {
    IRContext Context(Target);
    Module Types(Context);
    const Value &ModuleValue = Types;
    const auto *Integer = Types.typePool().getInt32Type();
    const auto *Signature = Types.typePool().getFunctionType(Integer, {Integer});
    Function Owner(*Types.typePool().getFunctionType(Integer));
    Function Callee(*Signature);
    auto *Entry = Owner.createBlock();
    const auto *Zero = Types.constantPool().getSignedIntegerConstant(Integer, 0);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(*Signature, ModuleValue, {Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createCall(Callee, {Argument(ModuleValue)}), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, ModuleValue, *Zero), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Zero, ModuleValue), nullptr);
    EXPECT_EQ(Builder.createConditionalBranch(ModuleValue, *Entry, *Entry), nullptr);
    EXPECT_EQ(Builder.createReturn(ModuleValue), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, ModuleValue, {Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Types.constantPool().getNullPointerConstant(), {&ModuleValue}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
    EXPECT_EQ(Builder.insertFunction(), &Owner);
    EXPECT_NE(Builder.createReturn(*Zero), nullptr);
  }

  // Unit array elements and nominal fields infer the canonical opaque pointer result, but cannot be traversed further.
  TEST(IRBuilderTest, AllowsUnitEndpointsInAggregateGEP)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *UnitArray = Types.typePool().getArrayType(Types.typePool().getVoidType(), 2);
    ASSERT_NE(UnitArray, nullptr);
    auto *Record = Types.createStructType(0, 1);
    ASSERT_NE(Record, nullptr);
    ASSERT_TRUE(Record->addField(Field(Types.typePool().getVoidType(), core::NamePool::instance().intern("builder_unit"))));
    auto *ArrayPointer = Types.typePool().getPointerType(UnitArray);
    auto *RecordPointer = Types.typePool().getPointerType(Record);
    Function Owner(*Types.typePool().getFunctionType(Types.typePool().getVoidType(), {ArrayPointer, RecordPointer}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().getSignedIntegerConstant(Types.typePool().getInt32Type(), 0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Element = Builder.createGetElementPtr(*UnitArray, *Owner.parameter(0), {Zero, Zero});
    auto *Field = Builder.createGetElementPtr(*Record, *Owner.parameter(1), {Zero, Zero});
    ASSERT_NE(Element, nullptr);
    ASSERT_NE(Field, nullptr);
    const auto *UnitPointer = Types.typePool().getPointerType(Types.typePool().getVoidType());
    EXPECT_EQ(Element->type(), UnitPointer);
    EXPECT_EQ(Field->type(), UnitPointer);
    EXPECT_EQ(Builder.createGetElementPtr(*UnitArray, *Owner.parameter(0), {Zero, Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Record, *Owner.parameter(1), {Zero, Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().getVoidType(), *Element, {Zero}), nullptr);
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
