#include "ink/frontend/compilation_session.h"
#include "ink/frontend/ir_generator.h"
#include "ink/execution/interpreter.h"
#include "ink/execution/runtime_world.h"
#include "ink/ir/ir.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace ink::frontend
{
  namespace
  {
    IrGenerationResult generateSource(CompilationSession &Session, std::string Source)
    {
      const core::SourceFileId SourceFile = Session.addSource("ir-generator-test.ink", std::move(Source));
      const std::optional<ast::AstFile> AstFile = Session.parseAndLower(SourceFile);
      EXPECT_TRUE(AstFile.has_value());
      if (!AstFile)
      {
        throw std::logic_error("IR generator test source did not lower to AST");
      }
      sema::SemanticAnalysisResult Analysis = Session.analyze(*AstFile);
      EXPECT_TRUE(Analysis.succeeded());
      EXPECT_TRUE(Analysis.Module.has_value());
      if (!Analysis.Module)
      {
        throw std::logic_error("IR generator test source did not produce a verified semantic module");
      }
      return generateIr(*Analysis.Module);
    }

    std::size_t countOpcode(const ir::IrModule &Module, ir::IrOpcode Opcode)
    {
      std::size_t Count = 0;
      for (std::uint32_t Index = 0; Index < Module.operationCount(); ++Index)
      {
        if (Module.operation(ir::IrOperationId::fromValue(Index)).Opcode == Opcode)
        {
          ++Count;
        }
      }
      return Count;
    }

    // Verifies mutable locals, width-correct i32 complement, supported compound arithmetic, short-circuit CFG, loops, calls, and block arguments form verifier-valid staged and closed IR.
    TEST(IrGeneratorTest, LowersCompleteMutableControlFlowSlice)
    {
      CompilationSession Session;
      IrGenerationResult Generated = generateSource(Session, "func choose(Flag: bool, Left: i32, Right: i32) -> i32 { return if (Flag) Left else Right; } func main(Flag: bool) -> i32 { var Value: i32 = ~0; while (Value < 2) { Value += 1; } if (Flag && Value == 2) { Value -= 1; } else { Value *= 3; } return (choose)(Flag, Value, Value + 1); }");

      ASSERT_TRUE(Generated.succeeded());
      ASSERT_TRUE(Generated.Module.has_value());
      const ir::IrModule &Module = Generated.Module->module();
      EXPECT_GE(countOpcode(Module, ir::IrOpcode::Alloca), 1U);
      EXPECT_GE(countOpcode(Module, ir::IrOpcode::Load), 1U);
      EXPECT_GE(countOpcode(Module, ir::IrOpcode::Store), 1U);
      EXPECT_GE(countOpcode(Module, ir::IrOpcode::IntXor), 1U);
      EXPECT_GE(countOpcode(Module, ir::IrOpcode::IntSub), 1U);
      EXPECT_GE(countOpcode(Module, ir::IrOpcode::IntMul), 1U);
      EXPECT_GE(countOpcode(Module, ir::IrOpcode::CondBranch), 3U);
      bool FoundWidthCorrectOnes = false;
      bool FoundSignedLess = false;
      for (std::uint32_t Index = 0; Index < Module.constantCount(); ++Index)
      {
        const ir::IrConstant &Constant = Module.constant(ir::IrConstantId::fromValue(Index));
        if (Constant.Kind == ir::IrConstantKind::Integer && Module.type(Constant.Type).Kind == ir::IrTypeKind::Integer && Module.type(Constant.Type).BitWidth == 32 && Constant.Bits == 0xFFFFFFFFULL)
        {
          FoundWidthCorrectOnes = true;
        }
      }
      for (std::uint32_t Index = 0; Index < Module.operationCount(); ++Index)
      {
        const ir::IrOperation &Operation = Module.operation(ir::IrOperationId::fromValue(Index));
        if (Operation.Opcode == ir::IrOpcode::IntCompare && std::holds_alternative<ir::IrComparePayload>(Operation.Payload) && std::get<ir::IrComparePayload>(Operation.Payload).Predicate == ir::IrComparePredicate::SignedLess)
        {
          FoundSignedLess = true;
        }
      }
      EXPECT_TRUE(FoundWidthCorrectOnes);
      EXPECT_TRUE(FoundSignedLess);
      ir::IrStagedVerificationResult StagedResult = ir::verifyStaged(*Generated.Module);
      ASSERT_TRUE(StagedResult.succeeded());
      ir::IrClosedVerificationResult ClosedResult = ir::closeAndVerify(StagedResult.verified(), Session.targetContext().key());
      EXPECT_TRUE(ClosedResult.succeeded());
    }

    // Verifies an if expression over u32 values maps unsigned semantic types and transports the selected value through an exact typed block argument.
    TEST(IrGeneratorTest, PreservesUnsignedTypesAcrossIfExpressionBlockArguments)
    {
      CompilationSession Session;
      IrGenerationResult Generated = generateSource(Session, "func choose(Flag: bool, Left: u32, Right: u32) -> u32 { return if (Flag) Left else Right; }");

      ASSERT_TRUE(Generated.succeeded());
      ASSERT_TRUE(Generated.Module.has_value());
      const ir::IrModule &Module = Generated.Module->module();
      bool FoundU32 = false;
      for (std::uint32_t Index = 0; Index < Module.typeCount(); ++Index)
      {
        const ir::IrType &Type = Module.type(ir::IrTypeId::fromValue(Index));
        if (Type.Kind == ir::IrTypeKind::Integer && Type.BitWidth == 32 && Type.Signedness == ir::IrSignedness::Unsigned)
        {
          FoundU32 = true;
        }
      }
      EXPECT_TRUE(FoundU32);
      EXPECT_GE(Module.blockCount(), 4U);
      EXPECT_TRUE(ir::verifyStaged(*Generated.Module).succeeded());
    }

    // Verifies comptime computation is staged outside the runtime CFG, closure installs its typed result, and interpretation observes only that resolved value.
    TEST(IrGeneratorTest, ClosesComptimeComputationIntoRuntimeConstant)
    {
      CompilationSession Session;
      IrGenerationResult Generated = generateSource(Session, "func main() -> i32 { return comptime (20 + 22); }");

      ASSERT_TRUE(Generated.succeeded());
      ASSERT_TRUE(Generated.Module.has_value());
      ASSERT_EQ(Generated.PendingForceValues.size(), 1U);
      const PendingForceValue Pending = Generated.PendingForceValues.front();
      const ir::IrModule &Module = Generated.Module->module();
      ASSERT_TRUE(Module.contains(Pending.PlanNode));
      EXPECT_EQ(Module.planNode(Pending.PlanNode).Input, Pending.Value);
      EXPECT_EQ(Module.planNode(Pending.PlanNode).Output, Pending.Output);
      EXPECT_EQ(Module.planNode(Pending.PlanNode).ResultType, Module.value(Pending.Value).Type);
      EXPECT_EQ(Module.value(Pending.Output).Type, Module.value(Pending.Value).Type);
      EXPECT_NE(Module.value(Pending.Value).OwnerBlock, Module.function(Pending.Function).EntryBlock);
      EXPECT_EQ(Module.value(Pending.Output).OwnerBlock, Module.function(Pending.Function).EntryBlock);
      ir::IrStagedVerificationResult StagedResult = ir::verifyStaged(*Generated.Module);
      ASSERT_TRUE(StagedResult.succeeded());
      EXPECT_FALSE(ir::closeAndVerify(StagedResult.verified(), Session.targetContext().key()).succeeded());
      ir::IrClosedVerificationResult ClosedResult = ir::closeAndVerify(StagedResult.verified(), Session.targetContext().key(), {{Pending.PlanNode, Module.planNode(Pending.PlanNode).ResultType, ir::IrConstantKind::Integer, 42}});
      ASSERT_TRUE(ClosedResult.succeeded());
      const ir::VerifiedClosedModule &Closed = ClosedResult.verified();
      const ir::IrModule &ClosedModule = Closed.module();
      EXPECT_EQ(ClosedModule.planNodeCount(), 0U);
      EXPECT_EQ(ir::printIr(ClosedModule).find("elaboration_plan"), std::string::npos);
      bool FoundResolvedDefinition = false;
      for (std::uint32_t Index = 0; Index < ClosedModule.operationCount(); ++Index)
      {
        const ir::IrOperation &Operation = ClosedModule.operation(ir::IrOperationId::fromValue(Index));
        if (Operation.Results.Count == 1 && ClosedModule.operationResult(Operation.Results.First) == Pending.Output)
        {
          ASSERT_EQ(Operation.Opcode, ir::IrOpcode::ConstInt);
          ASSERT_TRUE(std::holds_alternative<ir::IrConstantPayload>(Operation.Payload));
          const ir::IrConstant &Constant = ClosedModule.constant(std::get<ir::IrConstantPayload>(Operation.Payload).Constant);
          EXPECT_EQ(Constant.Type, ClosedModule.value(Pending.Output).Type);
          EXPECT_EQ(Constant.Kind, ir::IrConstantKind::Integer);
          EXPECT_EQ(Constant.Bits, 42U);
          FoundResolvedDefinition = true;
        }
      }
      EXPECT_TRUE(FoundResolvedDefinition);
      execution::RuntimeWorld World(Closed.targetKey());
      const execution::ExecutionResult Executed = execution::interpret(Closed, Pending.Function, World, {});
      ASSERT_EQ(Executed.Status, execution::ExecutionStatus::Returned);
      ASSERT_TRUE(Executed.Value.has_value());
      EXPECT_EQ(Executed.Value->type(), ClosedModule.value(Pending.Output).Type);
      EXPECT_EQ(Executed.Value->bits(), 42U);
    }

    // Verifies an if expression whose two arms call a never-returning function terminates both arms without fabricating a value or an illegal return.
    TEST(IrGeneratorTest, PropagatesNeverThroughIfExpression)
    {
      CompilationSession Session;
      IrGenerationResult Generated = generateSource(Session, "func stop() -> never; func chooseStop(Flag: bool) -> never { return if (Flag) stop() else stop(); }");

      ASSERT_TRUE(Generated.succeeded());
      ASSERT_TRUE(Generated.Module.has_value());
      EXPECT_EQ(countOpcode(Generated.Module->module(), ir::IrOpcode::DirectCall), 2U);
      EXPECT_EQ(countOpcode(Generated.Module->module(), ir::IrOpcode::Unreachable), 2U);
      EXPECT_EQ(countOpcode(Generated.Module->module(), ir::IrOpcode::Return), 0U);
      EXPECT_TRUE(ir::verifyStaged(*Generated.Module).succeeded());
    }

    // Verifies a void-valued if expression merges control without inventing a Unit/void SSA result.
    TEST(IrGeneratorTest, MergesVoidIfExpressionWithoutBlockArgument)
    {
      CompilationSession Session;
      IrGenerationResult Generated = generateSource(Session, "func action(); func run(Flag: bool) { if (Flag) action() else action(); return; }");

      ASSERT_TRUE(Generated.succeeded());
      ASSERT_TRUE(Generated.Module.has_value());
      EXPECT_EQ(countOpcode(Generated.Module->module(), ir::IrOpcode::DirectCall), 2U);
      EXPECT_EQ(countOpcode(Generated.Module->module(), ir::IrOpcode::Return), 1U);
      EXPECT_TRUE(ir::verifyStaged(*Generated.Module).succeeded());
    }
  } // namespace
} // namespace ink::frontend
