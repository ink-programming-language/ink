#include "ink/execution/execution_engine.h"
#include "ink/ir/analysis/verifier.h"
#include "ink/ir/builder.h"

#include <gtest/gtest.h>

#include <utility>
#include <vector>

namespace ink::execution
{
  namespace
  {
    // Verifies that the typed IRBuilder API can construct, verify, and execute a complete module containing memory, aggregate, control-flow, phi, and call instructions.
    TEST(IRBuilderExecutionTest, BuildsAndExecutesCompleteModule)
    {
      core::CompilationContext Compilation;
      ir::IRContext IR(Compilation);
      ExecutionContext Execution(Compilation);
      ir::IRBuilder Builder(IR);
      const ir::Type &BoolType = IR.getType(ir::TypeKind::Bool);
      const ir::Type &I32Type = IR.getType(ir::TypeKind::I32);
      const ir::Type &PointerSizeType = IR.getType(ir::TypeKind::PointerSize);
      const ir::Type &BytePointerType = IR.getType(ir::TypeKind::BytePointer);
      const ir::Type &ByteSliceType = IR.getType(ir::TypeKind::ByteSlice);
      const ir::StructType &BoxType = Builder.createStructType("Box", {ir::StructField("value", &I32Type)});

      const ir::FunctionId Adjust = Builder.createFunction("adjust", I32Type, {ir::Parameter(&I32Type)});
      const std::optional<ir::BlockId> AdjustEntry = Builder.createBlock(Adjust, "entry");
      const std::optional<ir::BlockId> Positive = Builder.createBlock(Adjust, "positive");
      const std::optional<ir::BlockId> Fallback = Builder.createBlock(Adjust, "fallback");
      const std::optional<ir::BlockId> Merge = Builder.createBlock(Adjust, "merge");
      ASSERT_TRUE(AdjustEntry.has_value());
      ASSERT_TRUE(Positive.has_value());
      ASSERT_TRUE(Fallback.has_value());
      ASSERT_TRUE(Merge.has_value());

      ASSERT_TRUE(Builder.setInsertionPoint(Adjust, *AdjustEntry));
      ir::CompareInstruction *IsPositive = Builder.createCompare(ir::ComparePredicate::GreaterThan, Builder.createValueOperand(I32Type, ir::ValueId{0}), Builder.getIntegerConstant(I32Type, 0));
      ASSERT_NE(IsPositive, nullptr);
      ASSERT_NE(Builder.createConditionalBranch(Builder.createValueOperand(BoolType, IsPositive->Result), *Positive, *Fallback), nullptr);

      ASSERT_TRUE(Builder.setInsertionPoint(Adjust, *Positive));
      ir::AddInstruction *PositiveResult = Builder.createAdd(Builder.createValueOperand(I32Type, ir::ValueId{0}), Builder.getIntegerConstant(I32Type, 2));
      ASSERT_NE(PositiveResult, nullptr);
      ASSERT_NE(Builder.createBranch(*Merge), nullptr);

      ASSERT_TRUE(Builder.setInsertionPoint(Adjust, *Fallback));
      ir::AddInstruction *FallbackResult = Builder.createAdd(Builder.createValueOperand(I32Type, ir::ValueId{0}), Builder.getIntegerConstant(I32Type, 1));
      ASSERT_NE(FallbackResult, nullptr);
      ASSERT_NE(Builder.createBranch(*Merge), nullptr);

      ASSERT_TRUE(Builder.setInsertionPoint(Adjust, *Merge));
      std::vector<ir::PhiIncoming> IncomingValues;
      IncomingValues.push_back({Builder.createValueOperand(I32Type, PositiveResult->Result), *Positive});
      IncomingValues.push_back({Builder.createValueOperand(I32Type, FallbackResult->Result), *Fallback});
      ir::PhiInstruction *MergedResult = Builder.createPhi(I32Type, std::move(IncomingValues));
      ASSERT_NE(MergedResult, nullptr);
      ASSERT_NE(Builder.createReturn(Builder.createValueOperand(I32Type, MergedResult->Result)), nullptr);

      const ir::FunctionId Main = Builder.createFunction("main", I32Type);
      const std::optional<ir::BlockId> MainEntry = Builder.createBlock(Main, "entry");
      ASSERT_TRUE(MainEntry.has_value());
      ASSERT_TRUE(Builder.setInsertionPoint(Main, *MainEntry));
      ir::AllocaInstruction *Storage = Builder.createAlloca(ByteSliceType, Builder.getIntegerConstant(PointerSizeType, 4));
      ASSERT_NE(Storage, nullptr);
      ir::SliceDataInstruction *StorageData = Builder.createSliceData(Builder.createValueOperand(ByteSliceType, Storage->Result));
      ASSERT_NE(StorageData, nullptr);
      ir::GetElementPointerInstruction *StoragePointer = Builder.createGetElementPointer(I32Type, Builder.createValueOperand(BytePointerType, StorageData->Result), Builder.getIntegerConstant(PointerSizeType, 0));
      ASSERT_NE(StoragePointer, nullptr);
      ir::InsertValueInstruction *Box = Builder.createInsertValue(Builder.getZeroInitializer(BoxType), Builder.getIntegerConstant(I32Type, 40), 0);
      ASSERT_NE(Box, nullptr);
      ir::ExtractValueInstruction *BoxValue = Builder.createExtractValue(Builder.createValueOperand(BoxType, Box->Result), 0);
      ASSERT_NE(BoxValue, nullptr);
      ASSERT_NE(Builder.createStore(Builder.createValueOperand(I32Type, BoxValue->Result), Builder.createValueOperand(BytePointerType, StoragePointer->Result)), nullptr);
      ir::LoadInstruction *Loaded = Builder.createLoad(I32Type, Builder.createValueOperand(BytePointerType, StoragePointer->Result));
      ASSERT_NE(Loaded, nullptr);
      ir::SliceLengthInstruction *StorageLength = Builder.createSliceLength(Builder.createValueOperand(ByteSliceType, Storage->Result));
      ASSERT_NE(StorageLength, nullptr);
      ASSERT_NE(Builder.createCompare(ir::ComparePredicate::GreaterThan, Builder.createValueOperand(PointerSizeType, StorageLength->Result), Builder.getIntegerConstant(PointerSizeType, 0)), nullptr);
      ASSERT_NE(Builder.createLifetimeEnd(Builder.createValueOperand(ByteSliceType, Storage->Result)), nullptr);
      std::vector<ir::ValueHandle> Arguments;
      Arguments.push_back(Builder.createValueOperand(I32Type, Loaded->Result));
      ir::CallInstruction *Call = Builder.createCall(Adjust, std::move(Arguments));
      ASSERT_NE(Call, nullptr);
      ASSERT_TRUE(Call->Result.has_value());
      ASSERT_NE(Builder.createReturn(Builder.createValueOperand(I32Type, *Call->Result)), nullptr);

      ir::Module ModuleValue = Builder.takeModule();
      const ir::VerificationResult Verification = ir::verify(IR, ModuleValue);
      ASSERT_TRUE(Verification.succeeded());
      ExecutionEngine Engine(Execution, ModuleValue);
      const ExecutionResult Result = Engine.execute("main");

      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      EXPECT_EQ(&Result.returnValue()->type(), &I32Type);
      EXPECT_EQ(Result.returnValue()->integer(), 42U);
    }
  } // namespace
} // namespace ink::execution
