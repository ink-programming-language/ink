#include "ink/ir/ir.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    target::TargetKey testTargetKey()
    {
      return {"x86_64-pc-windows-msvc", "generic", "+sse2", 64, target::TargetEndianness::Little};
    }

    bool hasError(const std::vector<IrVerificationError> &Errors, IrVerificationErrorCode Code)
    {
      return std::any_of(Errors.begin(), Errors.end(), [Code](const IrVerificationError &Error) { return Error.Code == Code; });
    }

    // Verifies the generated registries expose the required C0 Core and elaboration-plan opcodes with their stage metadata.
    TEST(IrSchemaTest, ExposesGeneratedCoreAndPlanMetadata)
    {
      EXPECT_STREQ(irOpcodeName(IrOpcode::IntNeg), "arith.neg");
      EXPECT_STREQ(irOpcodeName(IrOpcode::CastInt), "cast.int");
      EXPECT_STREQ(irOpcodeName(IrOpcode::Unreachable), "cf.unreachable");
      const auto *CastMetadata = irOpcodeMetadata(IrOpcode::CastInt);
      ASSERT_NE(CastMetadata, nullptr);
      EXPECT_EQ(CastMetadata->Payload, IrPayloadKind::Type);
      EXPECT_TRUE(hasStage(CastMetadata->Stages, IrStage::Closed));
      const auto *ForceMetadata = irPlanOpcodeMetadata(IrPlanOpcode::ForceValue);
      ASSERT_NE(ForceMetadata, nullptr);
      EXPECT_STREQ(ForceMetadata->Mnemonic, "stage.force_value");
      EXPECT_TRUE(hasStage(ForceMetadata->Stages, IrStage::Staged));
      EXPECT_FALSE(hasStage(ForceMetadata->Stages, IrStage::Closed));
    }

    // Verifies a first-slice module crosses the unverified, staged, and target-bound closed capability boundary only after every force-value plan node is resolved.
    TEST(IrCapabilityTest, ResolvesForceValueAndBindsTargetBeforeClosing)
    {
      IrBuilder Builder;
      const auto I32 = Builder.integerType(32, IrSignedness::Signed);
      const auto CalleeSignature = Builder.functionType({I32}, I32);
      const auto Callee = Builder.addFunction("identity", CalleeSignature, IrFunctionKind::External);
      const auto MainSignature = Builder.functionType({}, I32);
      const auto Main = Builder.addFunction("main", MainSignature, IrFunctionKind::Definition);
      const auto Entry = Builder.addBlock(Main);
      const auto TrueBlock = Builder.addBlock(Main, {I32});
      const auto FalseBlock = Builder.addBlock(Main, {I32});
      const auto MergeBlock = Builder.addBlock(Main, {I32});
      const auto OneConstant = Builder.integerConstant(I32, 1);
      const auto ZeroConstant = Builder.integerConstant(I32, 0);
      const auto TrueConstant = Builder.boolConstant(true);
      const auto One = Builder.createIntegerConstant(Entry.Block, OneConstant);
      const auto ResolvedValue = Builder.createIntegerConstant(Entry.Block, ZeroConstant);
      const auto Place = Builder.createAlloca(Entry.Block, I32, IrPlaceAccess::ReadWrite);
      Builder.createStore(Entry.Block, Place, One);
      const auto Loaded = Builder.createLoad(Entry.Block, Place);
      const auto Negated = Builder.createIntegerNegate(Entry.Block, Loaded);
      const auto Wide = Builder.integerType(64, IrSignedness::Signed);
      Builder.createIntegerCast(Entry.Block, Negated, Wide);
      const auto Condition = Builder.createBoolConstant(Entry.Block, TrueConstant);
      Builder.createConditionalBranch(Entry.Block, Condition, TrueBlock.Block, {ResolvedValue}, FalseBlock.Block, {Loaded});
      const auto Call = Builder.createDirectCall(TrueBlock.Block, Callee, {TrueBlock.Arguments[0]});
      ASSERT_EQ(Call.Results.size(), 1U);
      Builder.createBranch(TrueBlock.Block, MergeBlock.Block, {Call.Results[0]});
      Builder.createBranch(FalseBlock.Block, MergeBlock.Block, {FalseBlock.Arguments[0]});
      Builder.createReturn(MergeBlock.Block, MergeBlock.Arguments[0]);
      const auto Force = Builder.addForceValuePlan(One, ResolvedValue);

      auto Unverified = Builder.finish();
      EXPECT_TRUE(verifyFunction(Unverified.module(), Main, IrStage::Staged).empty());
      auto StagedResult = verifyStaged(Unverified);
      ASSERT_TRUE(StagedResult.succeeded());
      auto Staged = StagedResult.takeVerified();
      auto MissingResolution = closeAndVerify(Staged, testTargetKey());
      EXPECT_FALSE(MissingResolution.succeeded());
      EXPECT_TRUE(hasError(MissingResolution.errors(), IrVerificationErrorCode::InvalidStage));

      auto InvalidTarget = testTargetKey();
      InvalidTarget.Endianness = static_cast<target::TargetEndianness>(255);
      auto InvalidTargetResult = closeAndVerify(Staged, InvalidTarget, {{Force, I32, IrConstantKind::Integer, 1}});
      EXPECT_FALSE(InvalidTargetResult.succeeded());
      EXPECT_TRUE(hasError(InvalidTargetResult.errors(), IrVerificationErrorCode::InvalidStage));

      auto ClosedResult = closeAndVerify(Staged, testTargetKey(), {{Force, I32, IrConstantKind::Integer, 1}});
      ASSERT_TRUE(ClosedResult.succeeded());
      const auto Closed = ClosedResult.takeVerified();
      EXPECT_EQ(Closed.targetKey(), testTargetKey());
      EXPECT_EQ(Closed.module().planNodeCount(), 0U);
      EXPECT_TRUE(verifyModule(Closed.module(), IrStage::Closed).empty());
    }

    // Verifies the canonical printer uses stable table order and exact IDs for a small function.
    TEST(IrPrinterTest, PrintsDeterministicGoldenText)
    {
      IrBuilder Builder;
      const auto I32 = Builder.integerType(32, IrSignedness::Signed);
      const auto Signature = Builder.functionType({}, I32);
      const auto Function = Builder.addFunction("answer", Signature, IrFunctionKind::Definition);
      const auto Entry = Builder.addBlock(Function);
      const auto Constant = Builder.integerConstant(I32, 42);
      const auto Value = Builder.createIntegerConstant(Entry.Block, Constant);
      Builder.createReturn(Entry.Block, Value);
      const auto Module = Builder.finish();
      const std::string Expected = "ink.module stage=unverified-staged {\n  types {\n    !t0 = i32\n    !t1 = fn() -> !t0\n  }\n  constants {\n    #c0 = 0x000000000000002a : !t0\n  }\n  origins {\n  }\n  func @f0 \"answer\" : !t1 {\n    ^bb0():\n      %v0 = const.int {constant=#c0} : !t0\n      cf.return %v0\n  }\n}\n";
      EXPECT_EQ(printIr(Module), Expected);
      EXPECT_EQ(printIr(Module), printIr(Module));
    }

    // Verifies a defined block without a terminator is rejected instead of acquiring a verified capability.
    TEST(IrVerifierTest, RejectsMissingTerminator)
    {
      IrBuilder Builder;
      const auto Signature = Builder.functionType({}, std::nullopt);
      const auto Function = Builder.addFunction("unterminated", Signature, IrFunctionKind::Definition);
      Builder.addBlock(Function);
      const auto Result = verifyStaged(Builder.finish());
      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::MissingTerminator));
    }

    // Verifies every enum accepted through the public IR builder is rejected at verification when its underlying value is outside the declared domain.
    TEST(IrVerifierTest, RejectsOutOfDomainPublicBuilderEnums)
    {
      {
        SCOPED_TRACE("IrSignedness");
        IrBuilder Builder;
        const auto InvalidInteger = Builder.integerType(32, static_cast<IrSignedness>(255));
        const auto Signature = Builder.functionType({}, InvalidInteger);
        const auto Function = Builder.addFunction("invalid_signedness", Signature, IrFunctionKind::Definition);
        const auto Entry = Builder.addBlock(Function);
        const auto Value = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(InvalidInteger, 0));
        Builder.createReturn(Entry.Block, Value);
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidType));
      }
      {
        SCOPED_TRACE("IrPlaceAccess");
        IrBuilder Builder;
        const auto I32 = Builder.integerType(32, IrSignedness::Signed);
        const auto Signature = Builder.functionType({}, std::nullopt);
        const auto Function = Builder.addFunction("invalid_place_access", Signature, IrFunctionKind::Definition);
        const auto Entry = Builder.addBlock(Function);
        Builder.createAlloca(Entry.Block, I32, static_cast<IrPlaceAccess>(255));
        Builder.createReturn(Entry.Block);
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidType));
      }
      {
        SCOPED_TRACE("IrFunctionKind");
        IrBuilder Builder;
        const auto Signature = Builder.functionType({}, std::nullopt);
        const auto Function = Builder.addFunction("invalid_function_kind", Signature, static_cast<IrFunctionKind>(255));
        const auto Entry = Builder.addBlock(Function);
        Builder.createReturn(Entry.Block);
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidFunction));
      }
      {
        SCOPED_TRACE("IrComparePredicate");
        IrBuilder Builder;
        const auto I32 = Builder.integerType(32, IrSignedness::Signed);
        const auto Bool = Builder.boolType();
        const auto Signature = Builder.functionType({}, Bool);
        const auto Function = Builder.addFunction("invalid_compare_predicate", Signature, IrFunctionKind::Definition);
        const auto Entry = Builder.addBlock(Function);
        const auto One = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 1));
        const auto Compared = Builder.createIntegerCompare(Entry.Block, static_cast<IrComparePredicate>(255), One, One);
        Builder.createReturn(Entry.Block, Compared);
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidPayload));
      }
      {
        SCOPED_TRACE("IrTrapKind");
        IrBuilder Builder;
        const auto Signature = Builder.functionType({}, std::nullopt);
        const auto Function = Builder.addFunction("invalid_trap_kind", Signature, IrFunctionKind::Definition);
        const auto Entry = Builder.addBlock(Function);
        Builder.createTrap(Entry.Block, static_cast<IrTrapKind>(255));
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidPayload));
      }
      {
        SCOPED_TRACE("IrOpcode");
        IrBuilder Builder;
        const auto Signature = Builder.functionType({}, std::nullopt);
        const auto Function = Builder.addFunction("invalid_opcode", Signature, IrFunctionKind::Definition);
        const auto Entry = Builder.addBlock(Function);
        IrOperationSpec InvalidOperation;
        InvalidOperation.Opcode = static_cast<IrOpcode>(255);
        Builder.appendOperation(Entry.Block, std::move(InvalidOperation));
        Builder.createReturn(Entry.Block);
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidOperation));
      }
    }

    // Verifies place values cannot cross function or block argument boundaries and that alloca cannot manufacture a read-only place.
    TEST(IrVerifierTest, RejectsPlaceValuesAtCapabilityBoundaries)
    {
      {
        SCOPED_TRACE("function place parameter");
        IrBuilder Builder;
        const auto I32 = Builder.integerType(32, IrSignedness::Signed);
        const auto Place = Builder.placeType(I32, IrPlaceAccess::ReadWrite);
        const auto Signature = Builder.functionType({Place}, std::nullopt);
        const auto Function = Builder.addFunction("place_parameter", Signature, IrFunctionKind::Definition);
        const auto Entry = Builder.addBlock(Function, {Place});
        Builder.createReturn(Entry.Block);
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidType));
      }
      {
        SCOPED_TRACE("non-entry place block argument");
        IrBuilder Builder;
        const auto I32 = Builder.integerType(32, IrSignedness::Signed);
        const auto Place = Builder.placeType(I32, IrPlaceAccess::ReadWrite);
        const auto Signature = Builder.functionType({}, std::nullopt);
        const auto Function = Builder.addFunction("place_block_argument", Signature, IrFunctionKind::Definition);
        const auto Entry = Builder.addBlock(Function);
        const auto Target = Builder.addBlock(Function, {Place});
        const auto Allocation = Builder.createAlloca(Entry.Block, I32, IrPlaceAccess::ReadWrite);
        Builder.createBranch(Entry.Block, Target.Block, {Allocation});
        Builder.createReturn(Target.Block);
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidType));
      }
      {
        SCOPED_TRACE("read-only alloca");
        IrBuilder Builder;
        const auto I32 = Builder.integerType(32, IrSignedness::Signed);
        const auto Signature = Builder.functionType({}, std::nullopt);
        const auto Function = Builder.addFunction("read_only_alloca", Signature, IrFunctionKind::Definition);
        const auto Entry = Builder.addBlock(Function);
        Builder.createAlloca(Entry.Block, I32, IrPlaceAccess::ReadOnly);
        Builder.createReturn(Entry.Block);
        const auto Result = verifyStaged(Builder.finish());
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidType));
      }
    }

    // Verifies a store that occurs only after a load does not initialize the alloca for that earlier read.
    TEST(IrVerifierTest, RejectsLoadBeforeStoreToSameAlloca)
    {
      IrBuilder Builder;
      const auto I32 = Builder.integerType(32, IrSignedness::Signed);
      const auto Signature = Builder.functionType({}, I32);
      const auto Function = Builder.addFunction("load_before_store", Signature, IrFunctionKind::Definition);
      const auto Entry = Builder.addBlock(Function);
      const auto Allocation = Builder.createAlloca(Entry.Block, I32, IrPlaceAccess::ReadWrite);
      const auto Loaded = Builder.createLoad(Entry.Block, Allocation);
      const auto Value = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 7));
      Builder.createStore(Entry.Block, Allocation, Value);
      Builder.createReturn(Entry.Block, Loaded);
      const auto Result = verifyStaged(Builder.finish());
      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidOperation));
    }

    // Verifies a store in a dominating block initializes the same alloca for a load in its successor block.
    TEST(IrVerifierTest, AcceptsLoadAfterDominatingStoreToSameAlloca)
    {
      IrBuilder Builder;
      const auto I32 = Builder.integerType(32, IrSignedness::Signed);
      const auto Signature = Builder.functionType({}, I32);
      const auto Function = Builder.addFunction("dominating_store", Signature, IrFunctionKind::Definition);
      const auto Entry = Builder.addBlock(Function);
      const auto Target = Builder.addBlock(Function);
      const auto Allocation = Builder.createAlloca(Entry.Block, I32, IrPlaceAccess::ReadWrite);
      const auto Value = Builder.createIntegerConstant(Entry.Block, Builder.integerConstant(I32, 7));
      Builder.createStore(Entry.Block, Allocation, Value);
      Builder.createBranch(Entry.Block, Target.Block);
      const auto Loaded = Builder.createLoad(Target.Block, Allocation);
      Builder.createReturn(Target.Block, Loaded);
      const auto Result = verifyStaged(Builder.finish());
      EXPECT_TRUE(Result.succeeded());
    }

    // Verifies return operands must exactly match the declared function result type.
    TEST(IrVerifierTest, RejectsReturnTypeMismatch)
    {
      IrBuilder Builder;
      const auto I32 = Builder.integerType(32, IrSignedness::Signed);
      const auto Signature = Builder.functionType({}, I32);
      const auto Function = Builder.addFunction("bad_return", Signature, IrFunctionKind::Definition);
      const auto Entry = Builder.addBlock(Function);
      const auto BooleanConstant = Builder.boolConstant(false);
      const auto Boolean = Builder.createBoolConstant(Entry.Block, BooleanConstant);
      Builder.createReturn(Entry.Block, Boolean);
      const auto Result = verifyStaged(Builder.finish());
      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidType));
    }

    // Verifies each CFG edge supplies the exact argument count required by its destination block.
    TEST(IrVerifierTest, RejectsBranchArgumentMismatch)
    {
      IrBuilder Builder;
      const auto I32 = Builder.integerType(32, IrSignedness::Signed);
      const auto Signature = Builder.functionType({}, std::nullopt);
      const auto Function = Builder.addFunction("bad_branch", Signature, IrFunctionKind::Definition);
      const auto Entry = Builder.addBlock(Function);
      const auto Target = Builder.addBlock(Function, {I32});
      Builder.createBranch(Entry.Block, Target.Block);
      Builder.createReturn(Target.Block);
      const auto Result = verifyStaged(Builder.finish());
      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::InvalidControlFlow));
    }

    // Verifies a value defined in an unrelated block cannot be consumed where its definition does not dominate the use.
    TEST(IrVerifierTest, RejectsNonDominatingValue)
    {
      IrBuilder Builder;
      const auto I32 = Builder.integerType(32, IrSignedness::Signed);
      const auto Signature = Builder.functionType({}, I32);
      const auto Function = Builder.addFunction("bad_dominance", Signature, IrFunctionKind::Definition);
      const auto Entry = Builder.addBlock(Function);
      const auto Producer = Builder.addBlock(Function);
      const auto Constant = Builder.integerConstant(I32, 7);
      const auto ForeignValue = Builder.createIntegerConstant(Producer.Block, Constant);
      Builder.createReturn(Producer.Block, ForeignValue);
      const auto Sum = Builder.createIntegerBinary(Entry.Block, IrOpcode::IntAdd, ForeignValue, ForeignValue);
      Builder.createReturn(Entry.Block, Sum);
      const auto Result = verifyStaged(Builder.finish());
      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasError(Result.errors(), IrVerificationErrorCode::NonDominatingValue));
    }

    // Verifies a reachable unreachable terminator is accepted only when the preceding direct call has a never result.
    TEST(IrVerifierTest, DistinguishesNeverCallFromArbitraryUnreachable)
    {
      IrBuilder InvalidBuilder;
      const auto VoidSignature = InvalidBuilder.functionType({}, std::nullopt);
      const auto InvalidFunction = InvalidBuilder.addFunction("invalid_unreachable", VoidSignature, IrFunctionKind::Definition);
      const auto InvalidEntry = InvalidBuilder.addBlock(InvalidFunction);
      InvalidBuilder.createUnreachable(InvalidEntry.Block);
      const auto InvalidResult = verifyStaged(InvalidBuilder.finish());
      EXPECT_FALSE(InvalidResult.succeeded());
      EXPECT_TRUE(hasError(InvalidResult.errors(), IrVerificationErrorCode::InvalidControlFlow));

      IrBuilder ValidBuilder;
      const auto Never = ValidBuilder.neverType();
      const auto NeverSignature = ValidBuilder.functionType({}, Never);
      const auto Abort = ValidBuilder.addFunction("abort", NeverSignature, IrFunctionKind::External);
      const auto Wrapper = ValidBuilder.addFunction("wrapper", NeverSignature, IrFunctionKind::Definition);
      const auto Entry = ValidBuilder.addBlock(Wrapper);
      const auto Call = ValidBuilder.createDirectCall(Entry.Block, Abort, {});
      EXPECT_TRUE(Call.Results.empty());
      ValidBuilder.createUnreachable(Entry.Block);
      const auto ValidResult = verifyStaged(ValidBuilder.finish());
      EXPECT_TRUE(ValidResult.succeeded());
    }
  } // namespace
} // namespace ink::ir
