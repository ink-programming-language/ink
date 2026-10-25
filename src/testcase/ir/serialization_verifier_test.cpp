#include "ink/ir/serialization/serialization.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"

#include <gtest/gtest.h>

#include <memory>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class UnsupportedInstruction final : public Instruction
    {
      public:
        explicit UnsupportedInstruction(const Type &ResultType)
            : Instruction(ValueKind::Instruction, &ResultType)
        {
        }
    };
  } // namespace

  // A definition may use a value from a dominating block stored later in the function's block vector.
  TEST(IRSerializationVerifierTest, AcceptsDominanceIndependentOfBlockStorageOrder)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    const auto *Void = Context.typePool().get<VoidType>();
    auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<FunctionType>(Integer, {Integer}));
    auto *Entry = Callable->createBlock();
    auto *Exit = Callable->createBlock();
    auto *Body = Callable->createBlock();
    Entry->appendInstruction(std::make_unique<BranchInstruction>(*Void, *Body));
    auto *Definition = Body->appendInstruction(std::make_unique<NegInstruction>(*Callable->parameter(0)));
    Body->appendInstruction(std::make_unique<BranchInstruction>(*Void, *Exit));
    auto *Use = Exit->appendInstruction(std::make_unique<NegInstruction>(*Definition));
    Exit->appendInstruction(std::make_unique<ReturnInstruction>(*Void, Use));
    const auto Result = IRVerifier::verify(Owner);
    ASSERT_TRUE(Result.has_value()) << Result.error().Message;
  }

  // A loop backedge to the entry block remains valid when each SSA definition precedes its same-block uses.
  TEST(IRSerializationVerifierTest, AcceptsEntryBackedgeAndDeclarations)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const auto *Void = Context.typePool().get<VoidType>();
    const auto *Signature = Context.typePool().get<FunctionType>(Void);
    ASSERT_NE(Owner.createGlobalFunction(*Signature), nullptr);
    auto *Callable = Owner.createGlobalFunction(*Signature);
    auto *Entry = Callable->createBlock();
    Entry->appendInstruction(std::make_unique<BranchInstruction>(*Void, *Entry));
    EXPECT_TRUE(IRVerifier::verify(Owner).has_value());
    Module Empty(Context);
    EXPECT_TRUE(IRVerifier::verify(Empty).has_value());
  }

  // A definition in only one branch cannot supply an operand at a join reachable through the other branch.
  TEST(IRSerializationVerifierTest, RejectsDefinitionThatDoesNotDominateJoin)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    const auto *Void = Context.typePool().get<VoidType>();
    auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<FunctionType>(Integer, {Integer, Context.typePool().get<BoolType>()}));
    auto *Entry = Callable->createBlock();
    auto *Left = Callable->createBlock();
    auto *Right = Callable->createBlock();
    auto *Join = Callable->createBlock();
    Entry->appendInstruction(std::make_unique<ConditionalBranchInstruction>(*Void, *Callable->parameter(1), *Left, *Right));
    auto *Definition = Left->appendInstruction(std::make_unique<NegInstruction>(*Callable->parameter(0)));
    Left->appendInstruction(std::make_unique<BranchInstruction>(*Void, *Join));
    Right->appendInstruction(std::make_unique<BranchInstruction>(*Void, *Join));
    Join->appendInstruction(std::make_unique<ReturnInstruction>(*Void, Definition));
    const auto Result = IRVerifier::verify(Owner);
    ASSERT_FALSE(Result.has_value());
    EXPECT_EQ(Result.error().Code, IRSerializationErrorCode::InvalidIR);
    EXPECT_NE(Result.error().Message.find("dominate"), std::string::npos);
  }

  // An instruction cannot reference another instruction placed later in the same block.
  TEST(IRSerializationVerifierTest, RejectsSameBlockUseBeforeDefinition)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<FunctionType>(Integer, {Integer}));
    auto *Block = Callable->createBlock();
    auto Definition = std::make_unique<NegInstruction>(*Callable->parameter(0));
    auto *Use = Block->appendInstruction(std::make_unique<NegInstruction>(static_cast<const Value &>(*Definition)));
    Block->appendInstruction(std::move(Definition));
    Block->appendInstruction(std::make_unique<ReturnInstruction>(*Context.typePool().get<VoidType>(), Use));
    EXPECT_FALSE(IRVerifier::verify(Owner).has_value());
  }

  // Parameters and instructions belonging to another function cannot be operands even within a shared context.
  TEST(IRSerializationVerifierTest, RejectsForeignParameterAndInstructionOperands)
  {
    for (bool UseInstruction : {false, true})
    {
      IRContext Context(Target);
      Module Owner(Context);
      const auto *Integer = Context.typePool().get<IntegerType>(32, true);
      const auto *Void = Context.typePool().get<VoidType>();
      const auto *Signature = Context.typePool().get<FunctionType>(Integer, {Integer});
      auto *First = Owner.createGlobalFunction(*Signature);
      auto *Second = Owner.createGlobalFunction(*Signature);
      auto *FirstBlock = First->createBlock();
      auto *ForeignInstruction = FirstBlock->appendInstruction(std::make_unique<NegInstruction>(*First->parameter(0)));
      FirstBlock->appendInstruction(std::make_unique<ReturnInstruction>(*Void, ForeignInstruction));
      auto *SecondBlock = Second->createBlock();
      const Value *Foreign = UseInstruction ? ForeignInstruction : static_cast<const Value *>(First->parameter(0));
      SecondBlock->appendInstruction(std::make_unique<ReturnInstruction>(*Void, Foreign));
      EXPECT_FALSE(IRVerifier::verify(Owner).has_value());
    }
  }

  // A branch cannot leave its function, and each defined block requires exactly one final terminator.
  TEST(IRSerializationVerifierTest, RejectsForeignTargetsMissingAndNonfinalTerminators)
  {
    for (unsigned Scenario = 0; Scenario < 3; ++Scenario)
    {
      IRContext Context(Target);
      Module Owner(Context);
      const auto *Void = Context.typePool().get<VoidType>();
      const auto *Signature = Context.typePool().get<FunctionType>(Void);
      auto *Callable = Owner.createGlobalFunction(*Signature);
      auto *Block = Callable->createBlock();
      if (Scenario == 0)
      {
        auto *Foreign = Owner.createGlobalFunction(*Signature)->createBlock();
        Foreign->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
        Block->appendInstruction(std::make_unique<BranchInstruction>(*Void, *Foreign));
      }
      else if (Scenario == 1)
      {
        Block->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
        Block->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
      }
      EXPECT_FALSE(IRVerifier::verify(Owner).has_value());
    }
  }

  // Direct construction cannot bypass load result, return type, numeric negation or comparison predicate validation.
  TEST(IRSerializationVerifierTest, RejectsInvalidInstructionTypesAndPredicates)
  {
    for (unsigned Scenario = 0; Scenario < 4; ++Scenario)
    {
      IRContext Context(Target);
      Module Owner(Context);
      const auto *Integer = Context.typePool().get<IntegerType>(32, true);
      const auto *Float = Context.typePool().get<FloatType>(32);
      const auto *Void = Context.typePool().get<VoidType>();
      const auto *Bool = Context.typePool().get<BoolType>();
      const auto *Number = Context.constantPool().get<IntegerConstant>(Integer, 7);
      auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<FunctionType>(Void));
      auto *Block = Callable->createBlock();
      if (Scenario == 0)
      {
        auto *Storage = Block->appendInstruction(std::make_unique<AllocaInstruction>(*Context.typePool().get<PointerType>(Integer)));
        Block->appendInstruction(std::make_unique<LoadInstruction>(*Float, *Storage));
      }
      else if (Scenario == 1)
      {
        Block->appendInstruction(std::make_unique<ReturnInstruction>(*Void, Number));
        EXPECT_FALSE(IRVerifier::verify(Owner).has_value());
        continue;
      }
      else if (Scenario == 2)
      {
        Block->appendInstruction(std::make_unique<NegInstruction>(*Context.constantPool().get<BoolConstant>(true)));
      }
      else
      {
        Block->appendInstruction(std::make_unique<ICmpInstruction>(*Bool, static_cast<ICmpPredicate>(255), *Number, *Number));
      }
      Block->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
      EXPECT_FALSE(IRVerifier::verify(Owner).has_value());
    }
  }

  // GEP must preserve const inherited from an aggregate without interning a replacement type during verification.
  TEST(IRSerializationVerifierTest, ValidatesGEPQualifierPropagation)
  {
    for (bool CorrectResult : {true, false})
    {
      IRContext Context(Target);
      Module Owner(Context);
      const auto *Integer = Context.typePool().get<IntegerType>(32, true);
      const auto *Array = Context.typePool().get<ArrayType>(Integer, 4, false);
      const auto *Zero = Context.constantPool().get<IntegerConstant>(Integer, 0);
      const auto *Void = Context.typePool().get<VoidType>();
      auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<FunctionType>(Void));
      auto *Block = Callable->createBlock();
      auto *Storage = Block->appendInstruction(std::make_unique<AllocaInstruction>(*Context.typePool().get<PointerType>(Array)));
      const Type *Selected = CorrectResult ? Context.typePool().get(Integer, false) : Integer;
      const auto *ResultType = Context.typePool().get<PointerType>(Selected);
      Block->appendInstruction(std::make_unique<GetElementPtrInstruction>(*ResultType, *Array, *Storage, std::vector<const Value *>{Zero, Zero}));
      Block->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
      EXPECT_EQ(IRVerifier::verify(Owner).has_value(), CorrectResult);
    }
  }

  // A marked const alloca accepts its single declaration initializer and rejects a second write.
  TEST(IRSerializationVerifierTest, AcceptsOneConstLocalInitializerAndRejectsRepeatedStore)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const auto *Integer = Context.typePool().get<IntegerType>(32, true);
    const auto *Void = Context.typePool().get<VoidType>();
    const auto *Number = Context.constantPool().get<IntegerConstant>(Integer, 7);
    auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<FunctionType>(Void));
    auto *Entry = Callable->createBlock();
    auto *Exit = Callable->createBlock();
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    auto *Storage = Builder.createAlloca(*Integer, false);
    ASSERT_NE(Storage, nullptr);
    ASSERT_NE(Builder.createInitialize(*Number, *Storage), nullptr);
    ASSERT_NE(Builder.createBranch(*Exit), nullptr);
    Exit->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
    EXPECT_TRUE(IRVerifier::verify(Owner).has_value());

    Module Invalid(Context);
    auto *InvalidFunction = Invalid.createGlobalFunction(*Context.typePool().get<FunctionType>(Void));
    auto *InvalidBlock = InvalidFunction->createBlock();
    ASSERT_TRUE(Builder.setInsertPoint(*InvalidBlock));
    auto *InvalidStorage = Builder.createAlloca(*Integer, false);
    ASSERT_NE(InvalidStorage, nullptr);
    ASSERT_NE(Builder.createInitialize(*Number, *InvalidStorage), nullptr);
    InvalidBlock->appendInstruction(std::make_unique<StoreInstruction>(*Void, *Number, *InvalidStorage));
    InvalidBlock->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
    EXPECT_FALSE(IRVerifier::verify(Invalid).has_value());
  }

  // Calls reject arity and argument mismatches and references to another module's undeclared function values.
  TEST(IRSerializationVerifierTest, RejectsInvalidAndForeignCalls)
  {
    for (unsigned Scenario = 0; Scenario < 3; ++Scenario)
    {
      IRContext Context(Target);
      Module Owner(Context);
      Module Foreign(Context);
      const auto *Integer = Context.typePool().get<IntegerType>(32, true);
      const auto *Void = Context.typePool().get<VoidType>();
      const auto *Signature = Context.typePool().get<FunctionType>(Void, {Integer});
      const auto *Callee = (Scenario == 2 ? Foreign : Owner).createGlobalFunction(*Signature);
      auto *Callable = Owner.createGlobalFunction(*Context.typePool().get<FunctionType>(Void));
      auto *Block = Callable->createBlock();
      std::vector<Argument> Arguments;
      if (Scenario == 1)
      {
        Arguments.emplace_back(*Context.constantPool().get<BoolConstant>(true));
      }
      else if (Scenario == 2)
      {
        Arguments.emplace_back(*Context.constantPool().get<IntegerConstant>(Integer, 1));
      }
      Block->appendInstruction(std::make_unique<CallInstruction>(*Callee, std::move(Arguments)));
      Block->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
      EXPECT_FALSE(IRVerifier::verify(Owner).has_value());
    }
  }

  // Method bodies are verified like global functions, and unsupported instruction subclasses fail explicitly.
  TEST(IRSerializationVerifierTest, VerifiesMethodBodiesAndRejectsUnsupportedInstructions)
  {
    IRContext Context(Target);
    Module Owner(Context);
    const auto *Void = Context.typePool().get<VoidType>();
    const auto *Signature = Context.typePool().get<FunctionType>(Void);
    auto *Class = Owner.createClassType(8, 8);
    ASSERT_NE(Class, nullptr);
    auto Callable = std::make_unique<Function>(*Signature, core::NamePool::instance().intern("unsupported_body"));
    auto *Block = Callable->createBlock();
    Block->appendInstruction(std::make_unique<UnsupportedInstruction>(*Void));
    Block->appendInstruction(std::make_unique<ReturnInstruction>(*Void));
    ASSERT_TRUE(Class->addMethod(Method(std::move(Callable))));
    const auto Result = IRVerifier::verify(Owner);
    ASSERT_FALSE(Result.has_value());
    EXPECT_NE(Result.error().Message.find("Unsupported instruction"), std::string::npos);
  }
} // namespace ink::ir::test
