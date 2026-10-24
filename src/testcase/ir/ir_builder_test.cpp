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

#include <cstdint>
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
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    Function Other(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    Function Foreign(*ForeignTypes.typePool().get<ink::ir::FunctionType>(ForeignTypes.typePool().get<ink::ir::VoidType>()));
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
    auto *Array = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(32, true), 8);
    auto *ArrayPointer = Types.typePool().get<ink::ir::PointerType>(Array);
    auto *ElementPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    Function Callee(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {ElementPointer}));
    Function Caller(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {ArrayPointer, Types.typePool().get<ink::ir::IntegerType>(32, true)}));
    auto *Entry = Caller.createBlock();
    auto *Accepted = Caller.createBlock();
    auto *Rejected = Caller.createBlock();
    auto *Exit = Caller.createBlock();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
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
    EXPECT_EQ(Compare->type(), Types.typePool().get<ink::ir::BoolType>());
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
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    auto *True = Types.constantPool().get<ink::ir::BoolConstant>(true);
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
        ASSERT_NE(Block->appendInstruction(std::make_unique<ConditionalBranchInstruction>(*Types.typePool().get<ink::ir::VoidType>(), *True, *Block, *Block)), nullptr);
      }
      ASSERT_NE(Block->appendInstruction(std::make_unique<ICmpInstruction>(*Types.typePool().get<ink::ir::BoolType>(), ICmpPredicate::Equal, *Zero, *Zero)), nullptr);
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
    auto *Integer = Types.get<ink::ir::IntegerType>(32, true);
    auto *Signature = Types.get<ink::ir::FunctionType>(Integer, {{Integer, true}});
    ASSERT_NE(Signature, nullptr);
    Function Callee(*Signature);
    Function Caller(*Types.get<ink::ir::FunctionType>(Integer, {Signature, {Integer, true}}));
    Function FixedCallee(*Types.get<ink::ir::FunctionType>(Integer, {Integer}));
    auto *Entry = Caller.createBlock();
    auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
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
    Function Caller(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    Function Callee(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {Types.typePool().get<ink::ir::IntegerType>(32, true)}));
    Function Foreign(*ForeignTypes.typePool().get<ink::ir::FunctionType>(ForeignTypes.typePool().get<ink::ir::IntegerType>(32, true), {ForeignTypes.typePool().get<ink::ir::IntegerType>(32, true)}));
    auto *Entry = Caller.createBlock();
    auto *Integer = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(7));
    auto *Unsigned = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(7));
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
    EXPECT_EQ(Call->type(), Types.typePool().get<ink::ir::IntegerType>(32, true));
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
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    Function Caller(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    Function PointerCallee(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer}));
    Function IntegerCallee(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Types.typePool().get<ink::ir::IntegerType>(32, true)}));
    auto *Entry = Caller.createBlock();
    auto *Null = Types.constantPool().get<ink::ir::NullPointerConstant>();
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(PointerCallee, {Argument(*ForeignTypes.constantPool().get<ink::ir::NullPointerConstant>())}), nullptr);
    EXPECT_EQ(Builder.createCall(IntegerCallee, {Argument(*Null)}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *Call = Builder.createCall(PointerCallee, {Argument(*Null)});
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->argument(0)->value(), Null);
  }

  // Indirect calls require function values with the exact signature; function-storage pointers need an explicit load.
  TEST(IRBuilderTest, RequiresFunctionValuesForIndirectCalls)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    auto *OtherSignature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, false), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    auto *CallablePointer = Types.typePool().get<ink::ir::PointerType>(Signature);
    Function Caller(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Signature, CallablePointer, Types.typePool().get<ink::ir::IntegerType>(32, true)}));
    Function Direct(*Signature);
    auto *Entry = Caller.createBlock();
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(*OtherSignature, *Caller.parameter(0), {Argument(*Caller.parameter(2))}), nullptr);
    EXPECT_EQ(Builder.createCall(*OtherSignature, *Caller.parameter(1), {Argument(*Caller.parameter(2))}), nullptr);
    EXPECT_EQ(Builder.createCall(*OtherSignature, Direct, {Argument(*Caller.parameter(2))}), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, *Caller.parameter(1), {Argument(*Caller.parameter(2))}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *FunctionCall = Builder.createCall(*Signature, *Caller.parameter(0), {Argument(*Caller.parameter(2))});
    auto *CalleeStorage = Caller.parameter(1);
    ASSERT_NE(CalleeStorage, nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, *CalleeStorage, {Argument(*Caller.parameter(2))}), nullptr);
    auto *LoadedCallee = Builder.createLoad(*Signature, *CalleeStorage);
    ASSERT_NE(LoadedCallee, nullptr);
    auto *LoadedCall = Builder.createCall(*Signature, *LoadedCallee, {Argument(*Caller.parameter(2))});
    auto *DirectCall = Builder.createCall(*Signature, Direct, {Argument(*Caller.parameter(2))});
    ASSERT_NE(FunctionCall, nullptr);
    ASSERT_NE(LoadedCall, nullptr);
    ASSERT_NE(DirectCall, nullptr);
    EXPECT_EQ(FunctionCall->directCallee(), nullptr);
    EXPECT_EQ(LoadedCall->callee(), LoadedCallee);
    EXPECT_EQ(LoadedCall->directCallee(), nullptr);
    EXPECT_EQ(DirectCall->directCallee(), &Direct);
    EXPECT_EQ(Entry->instructionCount(), 4U);
  }

  // A function-valued local directly produces its address, and its contents must be loaded before an indirect call.
  TEST(IRBuilderTest, LoadsFunctionAllocationPointerBeforeCalling)
  {
    IRContext Context(Target);
    const auto *Signature = Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::IntegerType>(32, true));
    Function Callee(*Signature);
    Function Caller(*Context.typePool().get<ink::ir::FunctionType>(Context.typePool().get<ink::ir::VoidType>()));
    auto *Entry = Caller.createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *Storage = Builder.createAlloca(*Signature);
    ASSERT_NE(Storage, nullptr);
    ASSERT_NE(Builder.createInitialize(Callee, *Storage), nullptr);
    EXPECT_EQ(Storage->type(), Context.typePool().get<ink::ir::PointerType>(Signature));
    EXPECT_EQ(Builder.createCall(*Signature, *Storage), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 2U);
    auto *Loaded = Builder.createLoad(*Signature, *Storage);
    ASSERT_NE(Loaded, nullptr);
    auto *Call = Builder.createCall(*Signature, *Loaded);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->callee(), Loaded);
    EXPECT_EQ(Entry->instructionCount(), 4U);
  }

  // Void and value returns match the enclosing signature, and rejected overloads leave the block open for a valid return.
  TEST(IRBuilderTest, MatchesReturnValueToEnclosingSignature)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    Function VoidFunction(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    Function ValueFunction(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true)));
    auto *VoidEntry = VoidFunction.createBlock();
    auto *ValueEntry = ValueFunction.createBlock();
    auto *Integer = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(42));
    auto *Unsigned = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(42));
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*VoidEntry));
    EXPECT_EQ(Builder.createReturn(*Integer), nullptr);
    EXPECT_EQ(VoidEntry->instructionCount(), 0U);
    auto *VoidReturn = Builder.createReturn();
    ASSERT_NE(VoidReturn, nullptr);
    EXPECT_FALSE(VoidReturn->hasReturnedValue());
    EXPECT_EQ(VoidReturn->type(), Types.typePool().get<ink::ir::VoidType>());
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
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true))));
    auto *Entry = Owner.createBlock();
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createReturn(*ForeignTypes.constantPool().get<ink::ir::NullPointerConstant>()), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    auto *Return = Builder.createReturn(*Types.constantPool().get<ink::ir::NullPointerConstant>());
    ASSERT_NE(Return, nullptr);
    EXPECT_EQ(Return->returnedValue(), Types.constantPool().get<ink::ir::NullPointerConstant>());
  }

  // All declared integer comparison predicates retain their identity and accept matching integer or pointer operands.
  TEST(IRBuilderTest, AcceptsEveryComparisonPredicateForMatchingIntegerAndPointerTypes)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer, Pointer}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
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
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    auto *OtherPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, false));
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer, OtherPointer}));
    auto *Entry = Owner.createBlock();
    auto *Integer = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    auto *Unsigned = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, false), static_cast<std::uint64_t>(0));
    auto *Boolean = Types.constantPool().get<ink::ir::BoolConstant>(false);
    auto *Float = Types.constantPool().get<ink::ir::FloatConstant>(0.0);
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
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer}));
    auto *Entry = Owner.createBlock();
    auto *Null = Types.constantPool().get<ink::ir::NullPointerConstant>();
    auto *Value = Owner.parameter(0);
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Null, *Null), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::SignedLessThan, *Value, *Null), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::UnsignedGreaterThan, *Null, *Value), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Value, *ForeignTypes.constantPool().get<ink::ir::NullPointerConstant>()), nullptr);
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
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    Function Other(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    auto *Entry = Owner.createBlock();
    auto *Sibling = Owner.createBlock();
    auto *OtherEntry = Other.createBlock();
    BasicBlock Detached;
    auto *True = Types.constantPool().get<ink::ir::BoolConstant>(true);
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
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
    auto *Array = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(32, true), 4);
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Array);
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer}));
    auto *Entry = Owner.createBlock();
    auto *One = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(1));
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
    auto *Inner = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(32, true), 2);
    auto *Outer = Types.typePool().get<ink::ir::ArrayType>(Inner, 3);
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Outer);
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer, Types.typePool().get<ink::ir::IntegerType>(32, true)}));
    auto *Entry = Owner.createBlock();
    auto *Negative = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(-1));
    auto *Large = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(64, false), static_cast<std::uint64_t>(1000));
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Address = Builder.createGetElementPtr(*Outer, *Owner.parameter(0), {Negative, Owner.parameter(1), Large}, true);
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->type(), Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true)));
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
    auto *Array = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::IntegerType>(32, true), 4);
    auto *Record = Types.createStructType(24, 8);
    ASSERT_TRUE(Record->addField(Field(Types.typePool().get<ink::ir::IntegerType>(64, true), core::NamePool::instance().intern("builder_header"), 0)));
    ASSERT_TRUE(Record->addField(Field(Array, core::NamePool::instance().intern("builder_values"), 8)));
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Record);
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer, Types.typePool().get<ink::ir::IntegerType>(32, true)}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    auto *One = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(64, false), static_cast<std::uint64_t>(1));
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Address = Builder.createGetElementPtr(*Record, *Owner.parameter(0), {Zero, One, Owner.parameter(1)});
    ASSERT_NE(Address, nullptr);
    EXPECT_EQ(Address->type(), Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true)));
    EXPECT_EQ(Address->sourceElementType(), Record);
    EXPECT_EQ(Address->index(1), One);
  }

  // Nominal selection rejects dynamic, negative and missing field indices without changing the insertion block.
  TEST(IRBuilderTest, RejectsInvalidNominalFieldIndices)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Record = Types.createStructType(4, 4);
    ASSERT_TRUE(Record->addField(Field(Types.typePool().get<ink::ir::IntegerType>(32, true), core::NamePool::instance().intern("builder_field"))));
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Record);
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer, Types.typePool().get<ink::ir::IntegerType>(32, true)}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    auto *Negative = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(-1));
    auto *PastEnd = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(64, false), static_cast<std::uint64_t>(1));
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
    auto *Integer = Types.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Integer);
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer}));
    auto *Entry = Owner.createBlock();
    auto *Null = Types.constantPool().get<ink::ir::NullPointerConstant>();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    auto *ForeignZero = ForeignTypes.constantPool().get<ink::ir::IntegerConstant>(ForeignTypes.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {nullptr}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {Types.constantPool().get<ink::ir::BoolConstant>(false)}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {Types.constantPool().get<ink::ir::FloatConstant>(0.0)}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {ForeignZero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Zero, {Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().get<ink::ir::IntegerType>(32, false), *Owner.parameter(0), {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*ForeignTypes.typePool().get<ink::ir::IntegerType>(32, true), *Null, {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *ForeignTypes.constantPool().get<ink::ir::NullPointerConstant>(), {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().get<ink::ir::VoidType>(), *Null, {}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().get<ink::ir::VariadicType>(), *Null, {}), nullptr);
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
    auto *Integer = Types.typePool().get<ink::ir::IntegerType>(32, true);
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Integer);
    auto *PointerPointer = Types.typePool().get<ink::ir::PointerType>(Pointer);
    auto *UnitPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::VoidType>());
    auto *UnitPointerPointer = Types.typePool().get<ink::ir::PointerType>(UnitPointer);
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Pointer, PointerPointer, UnitPointer, UnitPointerPointer}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Owner.parameter(0), {Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Pointer, *Owner.parameter(1), {Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().get<ink::ir::VoidType>(), *Owner.parameter(2), {Zero}), nullptr);
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
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Types.typePool().get<ink::ir::VoidType>()}));
    Function UnitProducer(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>()));
    Function UnitConsumer(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {Types.typePool().get<ink::ir::VoidType>()}));
    auto *Entry = Owner.createBlock();
    auto *Sibling = Owner.createBlock();
    auto *Terminator = Sibling->appendInstruction(std::make_unique<ReturnInstruction>(*Types.typePool().get<ink::ir::VoidType>()));
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

  // Global address values cannot substitute for integer contents or a callable value, and GEP checks their pointee type.
  TEST(IRBuilderTest, RejectsGlobalPointersInIncompatibleValuePositions)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    auto *Pointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::IntegerType>(32, true));
    auto *Signature = Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true), {Types.typePool().get<ink::ir::IntegerType>(32, true)});
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::IntegerType>(32, true)));
    Function Callee(*Signature);
    GlobalVariable IntegerGlobal(*Pointer);
    GlobalVariable PointerGlobal(*Types.typePool().get<ink::ir::PointerType>(Pointer));
    GlobalVariable CallableGlobal(*Types.typePool().get<ink::ir::PointerType>(Signature));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(Callee, {Argument(IntegerGlobal)}), nullptr);
    EXPECT_EQ(Builder.createCall(*Signature, CallableGlobal, {Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, IntegerGlobal, *Zero), nullptr);
    EXPECT_EQ(Builder.createReturn(IntegerGlobal), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().get<ink::ir::IntegerType>(32, true), PointerGlobal, {Zero}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
    EXPECT_EQ(Builder.insertBlock(), Entry);
  }

  // A module's Value identity cannot be used as a callee, argument, comparison, branch condition, return or GEP operand.
  TEST(IRBuilderTest, RejectsModulesAsOperandsWithoutChangingInsertionState)
  {
    IRContext Context(Target);
    Module Types(Context);
    const Value &ModuleValue = Types;
    const auto *Integer = Types.typePool().get<ink::ir::IntegerType>(32, true);
    const auto *Signature = Types.typePool().get<ink::ir::FunctionType>(Integer, {Integer});
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Integer));
    Function Callee(*Signature);
    auto *Entry = Owner.createBlock();
    const auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Integer, static_cast<std::int64_t>(0));
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    EXPECT_EQ(Builder.createCall(*Signature, ModuleValue, {Argument(*Zero)}), nullptr);
    EXPECT_EQ(Builder.createCall(Callee, {Argument(ModuleValue)}), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, ModuleValue, *Zero), nullptr);
    EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Zero, ModuleValue), nullptr);
    EXPECT_EQ(Builder.createConditionalBranch(ModuleValue, *Entry, *Entry), nullptr);
    EXPECT_EQ(Builder.createReturn(ModuleValue), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, ModuleValue, {Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Integer, *Types.constantPool().get<ink::ir::NullPointerConstant>(), {&ModuleValue}), nullptr);
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
    auto *UnitArray = Types.typePool().get<ink::ir::ArrayType>(Types.typePool().get<ink::ir::VoidType>(), 2);
    ASSERT_NE(UnitArray, nullptr);
    auto *Record = Types.createStructType(0, 1);
    ASSERT_NE(Record, nullptr);
    ASSERT_TRUE(Record->addField(Field(Types.typePool().get<ink::ir::VoidType>(), core::NamePool::instance().intern("builder_unit"))));
    auto *ArrayPointer = Types.typePool().get<ink::ir::PointerType>(UnitArray);
    auto *RecordPointer = Types.typePool().get<ink::ir::PointerType>(Record);
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::VoidType>(), {ArrayPointer, RecordPointer}));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
    IRBuilder Builder(Types.irContext());
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Element = Builder.createGetElementPtr(*UnitArray, *Owner.parameter(0), {Zero, Zero});
    auto *Field = Builder.createGetElementPtr(*Record, *Owner.parameter(1), {Zero, Zero});
    ASSERT_NE(Element, nullptr);
    ASSERT_NE(Field, nullptr);
    const auto *UnitPointer = Types.typePool().get<ink::ir::PointerType>(Types.typePool().get<ink::ir::VoidType>());
    EXPECT_EQ(Element->type(), UnitPointer);
    EXPECT_EQ(Field->type(), UnitPointer);
    EXPECT_EQ(Builder.createGetElementPtr(*UnitArray, *Owner.parameter(0), {Zero, Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Record, *Owner.parameter(1), {Zero, Zero, Zero}), nullptr);
    EXPECT_EQ(Builder.createGetElementPtr(*Types.typePool().get<ink::ir::VoidType>(), *Element, {Zero}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 2U);
  }

  // Builder destruction releases no IR, and a replacement builder can continue appending to the same function-owned block.
  TEST(IRBuilderTest, LeavesInstructionOwnershipWithTheBasicBlock)
  {
    IRContext TypesContext(Target);
    Module Types(TypesContext);
    Function Owner(*Types.typePool().get<ink::ir::FunctionType>(Types.typePool().get<ink::ir::BoolType>()));
    auto *Entry = Owner.createBlock();
    auto *Zero = Types.constantPool().get<ink::ir::IntegerConstant>(Types.typePool().get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(0));
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
