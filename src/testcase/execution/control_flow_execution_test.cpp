#include "ink/execution/execution_engine.h"
#include "ink/ir/instruction/arithmetic.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/serialization.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ink::execution
{
  namespace
  {
    struct ControlFlowExecutionTestContext
    {
        ControlFlowExecutionTestContext() = default;

        explicit ControlFlowExecutionTestContext(core::TargetContext Target)
            : Compilation(Target)
        {
        }

        core::CompilationContext Compilation;
        ir::IRContext IR{Compilation};
        ExecutionContext Execution{Compilation};
    };

    ExecutionResult executeText(ControlFlowExecutionTestContext &Context, const std::string &Text, const std::vector<RuntimeValueRef> &Arguments = {})
    {
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      if (!Parsed.succeeded())
      {
        ADD_FAILURE() << "expected test InkIR to deserialize";
        return {};
      }
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      return Engine.execute("main", Arguments);
    }

    void expectIntegerResult(const ExecutionResult &Result, std::uint64_t Expected)
    {
      ASSERT_TRUE(Result.succeeded());
      ASSERT_NE(Result.returnValue(), nullptr);
      const std::optional<std::uint64_t> Value = Result.returnValue()->integer();
      ASSERT_TRUE(Value.has_value());
      EXPECT_EQ(*Value, Expected);
    }

    // Verifies that an unconditional branch selects the matching constant incoming value of a target-block phi.
    TEST(ControlFlowExecutionTest, ExecutesUnconditionalBranch)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  br selected\n"
          "selected:\n"
          "  %0 = phi i32 [42, entry]\n"
          "  ret i32 %0\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 42);
    }

    // Verifies that condbr selects both its true and false target from a bool entry argument.
    TEST(ControlFlowExecutionTest, ExecutesBothConditionalBranchDirections)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main(bool %0) {\n"
          "entry:\n"
          "  condbr bool %0, yes, no\n"
          "yes:\n"
          "  ret i32 1\n"
          "no:\n"
          "  ret i32 2\n"
          "}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      RuntimeValueArena Arguments;
      RuntimeValueRef FalseValue = Arguments.integerValue(Context.IR.getType(ir::TypeKind::Bool), 0);
      RuntimeValueRef TrueValue = Arguments.integerValue(Context.IR.getType(ir::TypeKind::Bool), 1);

      const ExecutionResult FalseResult = Engine.execute("main", {FalseValue});
      const ExecutionResult TrueResult = Engine.execute("main", {TrueValue});

      expectIntegerResult(FalseResult, 2);
      expectIntegerResult(TrueResult, 1);
    }

    // Verifies that a diamond control-flow join selects the phi incoming value for either predecessor.
    TEST(ControlFlowExecutionTest, ExecutesDiamondJoin)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main(bool %0) {\n"
          "entry:\n"
          "  condbr bool %0, left, right\n"
          "left:\n"
          "  br join\n"
          "right:\n"
          "  br join\n"
          "join:\n"
          "  %1 = phi i32 [11, left], [22, right]\n"
          "  ret i32 %1\n"
          "}\n";
      RuntimeValueArena Arguments;
      RuntimeValueRef FalseCondition = Arguments.integerValue(Context.IR.getType(ir::TypeKind::Bool), 0);
      RuntimeValueRef TrueCondition = Arguments.integerValue(Context.IR.getType(ir::TypeKind::Bool), 1);

      const ExecutionResult FalseResult = executeText(Context, Text, {FalseCondition});
      const ExecutionResult TrueResult = executeText(Context, Text, {TrueCondition});

      expectIntegerResult(FalseResult, 22);
      expectIntegerResult(TrueResult, 11);
    }

    // Verifies that mutually dependent loop phis read the previous iteration before either result is rebound.
    TEST(ControlFlowExecutionTest, BindsMultipleLoopPhisSimultaneously)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  br loop\n"
          "loop:\n"
          "  %0 = phi i32 [1, entry], [%1, loop]\n"
          "  %1 = phi i32 [2, entry], [%0, loop]\n"
          "  %2 = icmp eq i32 %0, i32 2\n"
          "  condbr bool %2, exit, loop\n"
          "exit:\n"
          "  %3 = phi i32 [%0, loop]\n"
          "  ret i32 %3\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      expectIntegerResult(Result, 2);
    }

    // Verifies that a loop phi handles both an immediate exit and repeated values selected from its backedge.
    TEST(ControlFlowExecutionTest, ExecutesCounterLoop)
    {
      const std::int32_t InitialValues[] = {0, 5};
      for (const std::int32_t InitialValue : InitialValues)
      {
        ControlFlowExecutionTestContext Context;
        const std::string Text =
            "inkir 1\n"
            "define i32 @main() {\n"
            "entry:\n"
            "  br loop\n"
            "loop:\n"
            "  %0 = phi i32 [" +
            std::to_string(InitialValue) + ", entry], [%2, body]\n"
                                           "  %1 = icmp lt i32 %0, i32 5\n"
                                           "  condbr bool %1, body, exit\n"
                                           "body:\n"
                                           "  %2 = add i32 %0, i32 1\n"
                                           "  br loop\n"
                                           "exit:\n"
                                           "  ret i32 %0\n"
                                           "}\n";

        const ExecutionResult Result = executeText(Context, Text);

        expectIntegerResult(Result, 5);
      }
    }

    // Verifies wrapping addition for byte and signed i32 payloads.
    TEST(ControlFlowExecutionTest, WrapsByteAndI32Addition)
    {
      struct Case
      {
          const char *TypeName;
          const char *Left;
          const char *Right;
          std::uint64_t Expected;
      };
      const Case Cases[] = {
          {"byte", "255", "1", 0},
          {"i32", "2147483647", "1", 0xFFFFFFFF80000000ULL},
      };

      for (const Case &CaseValue : Cases)
      {
        ControlFlowExecutionTestContext Context;
        const std::string Text = "inkir 1\ndefine " + std::string(CaseValue.TypeName) + " @main() {\nentry:\n  %0 = add " + CaseValue.TypeName + " " + CaseValue.Left + ", " + CaseValue.TypeName + " " + CaseValue.Right + "\n  ret " + CaseValue.TypeName + " %0\n}\n";
        const ExecutionResult Result = executeText(Context, Text);
        expectIntegerResult(Result, CaseValue.Expected);
      }
    }

    // Verifies that ptrsize addition wraps independently at both supported target pointer widths.
    TEST(ControlFlowExecutionTest, WrapsPointerSizeAdditionAtTargetWidth)
    {
      const std::string Text =
          "inkir 1\n"
          "define ptrsize @main(ptrsize %0, ptrsize %1) {\n"
          "entry:\n"
          "  %2 = add ptrsize %0, ptrsize %1\n"
          "  ret ptrsize %2\n"
          "}\n";
      constexpr core::PointerWidth Widths[] = {core::PointerWidth::Bits32, core::PointerWidth::Bits64};

      for (const core::PointerWidth Width : Widths)
      {
        const core::TargetContext Target(Width, core::ByteOrder::LittleEndian);
        ControlFlowExecutionTestContext Context(Target);
        RuntimeValueArena Arguments(Target);
        const ir::Type &PointerSizeType = Context.IR.getType(ir::TypeKind::PointerSize);
        RuntimeValueRef Maximum = Arguments.integerValue(PointerSizeType, Target.maximumPointerSizeValue());
        RuntimeValueRef One = Arguments.integerValue(PointerSizeType, 1);
        ASSERT_NE(Maximum, nullptr);
        ASSERT_NE(One, nullptr);

        const ExecutionResult Result = executeText(Context, Text, {Maximum, One});

        expectIntegerResult(Result, 0);
      }
    }

    // Verifies all six i32 predicates use signed ordering for a negative left operand.
    TEST(ControlFlowExecutionTest, ExecutesEverySignedI32ComparisonPredicate)
    {
      struct Case
      {
          const char *Predicate;
          std::uint64_t Expected;
      };
      const Case Cases[] = {
          {"eq", 0},
          {"ne", 1},
          {"lt", 1},
          {"le", 1},
          {"gt", 0},
          {"ge", 0},
      };

      for (const Case &CaseValue : Cases)
      {
        ControlFlowExecutionTestContext Context;
        const std::string Text = "inkir 1\ndefine bool @main() {\nentry:\n  %0 = icmp " + std::string(CaseValue.Predicate) + " i32 -1, i32 0\n  ret bool %0\n}\n";
        const ExecutionResult Result = executeText(Context, Text);
        expectIntegerResult(Result, CaseValue.Expected);
      }
    }

    // Verifies unsigned byte and ptrsize ordering together with bool inequality.
    TEST(ControlFlowExecutionTest, ExecutesUnsignedAndBoolComparisons)
    {
      struct Case
      {
          const char *Expression;
      };
      const Case Cases[] = {
          {"icmp gt byte 255, byte 1"},
          {"icmp gt ptrsize 9, ptrsize 1"},
          {"icmp eq bool 1, bool 1"},
          {"icmp ne bool 0, bool 1"},
      };

      for (const Case &CaseValue : Cases)
      {
        ControlFlowExecutionTestContext Context;
        const std::string Text = "inkir 1\ndefine bool @main() {\nentry:\n  %0 = " + std::string(CaseValue.Expression) + "\n  ret bool %0\n}\n";
        const ExecutionResult Result = executeText(Context, Text);
        expectIntegerResult(Result, 1);
      }
    }

    // Verifies pointer equality and inequality compare logical address identity without relying on host ordering.
    TEST(ControlFlowExecutionTest, ComparesLogicalPointerIdentity)
    {
      struct Case
      {
          const char *Predicate;
          std::uint64_t Expected;
      };
      const Case Cases[] = {
          {"eq", 1},
          {"ne", 0},
      };
      std::uint8_t Byte = 0;
      for (const Case &CaseValue : Cases)
      {
        ControlFlowExecutionTestContext Context;
        const std::string Text = "inkir 1\ndefine bool @main(byte* %0, byte* %1) {\nentry:\n  %2 = icmp " + std::string(CaseValue.Predicate) + " byte* %0, byte* %1\n  ret bool %2\n}\n";
        RuntimeValueArena Arguments;
        const ir::Type &PointerType = Context.IR.getType(ir::TypeKind::BytePointer);
        RuntimeValueRef First = Arguments.mutablePointerValue(PointerType, &Byte);
        RuntimeValueRef Second = Arguments.mutablePointerValue(PointerType, &Byte);

        const ExecutionResult Result = executeText(Context, Text, {First, Second});

        expectIntegerResult(Result, CaseValue.Expected);
      }
    }

    // Verifies that the executor rejects a branch target corrupted after module verification with its dedicated defensive diagnostic.
    TEST(ControlFlowExecutionTest, ReportsInvalidBranchTargetAfterModuleMutation)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text = "inkir 1\ndefine void @main() {\nentry:\n  br exit\nexit:\n  ret void\n}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      ASSERT_TRUE(Engine.initialize().succeeded());
      auto &Branch = static_cast<ir::BranchInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[0]);
      Branch.Target.Block = ir::BlockId{99};

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidBranchTargetDuringExecution);
    }

    // Verifies that removing the selected phi incoming after initialization receives the dedicated runtime diagnostic.
    TEST(ControlFlowExecutionTest, ReportsPhiIncomingMismatchAfterModuleMutation)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text = "inkir 1\ndefine i32 @main() {\nentry:\n  br exit\nexit:\n  %0 = phi i32 [0, entry]\n  ret i32 %0\n}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      ASSERT_TRUE(Engine.initialize().succeeded());
      auto &Phi = static_cast<ir::PhiInstruction &>(*Parsed.module()->Functions[0].Blocks[1].Instructions[0]);
      Phi.IncomingValues.clear();

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::PhiIncomingMismatchDuringExecution);
    }

    // Verifies that an add operand corrupted after module verification receives the dedicated runtime integer-operation diagnostic.
    TEST(ControlFlowExecutionTest, ReportsInvalidIntegerOperationAfterModuleMutation)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text = "inkir 1\ndefine i32 @main() {\nentry:\n  %0 = add i32 1, i32 2\n  ret i32 %0\n}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      ASSERT_TRUE(Engine.initialize().succeeded());
      auto &Add = static_cast<ir::AddInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[0]);
      Add.Left = Context.IR.constantPool().getZeroInitializer(Context.IR.getType(ir::TypeKind::ByteSlice));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidIntegerOperationDuringExecution);
    }

    // Verifies that an icmp operand corrupted after module verification receives the dedicated runtime comparison diagnostic.
    TEST(ControlFlowExecutionTest, ReportsInvalidComparisonAfterModuleMutation)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text = "inkir 1\ndefine bool @main() {\nentry:\n  %0 = icmp eq i32 1, i32 1\n  ret bool %0\n}\n";
      ir::DeserializeResult Parsed = ir::deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      ExecutionEngine Engine(Context.Execution, *Parsed.module());
      ASSERT_TRUE(Engine.initialize().succeeded());
      auto &Compare = static_cast<ir::CompareInstruction &>(*Parsed.module()->Functions[0].Blocks[0].Instructions[0]);
      Compare.Left = Context.IR.constantPool().getZeroInitializer(Context.IR.getType(ir::TypeKind::ByteSlice));

      const ExecutionResult Result = Engine.execute("main");

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::InvalidComparisonDuringExecution);
    }

    // Verifies that a non-terminating backedge stops at the global one-million-instruction execution limit.
    TEST(ControlFlowExecutionTest, StopsInfiniteLoopAtInstructionLimit)
    {
      ControlFlowExecutionTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  br loop\n"
          "loop:\n"
          "  br loop\n"
          "}\n";

      const ExecutionResult Result = executeText(Context, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_EQ(Result.diagnostics().size(), 1u);
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::ExecutionStepLimitExceeded);
      EXPECT_EQ(core::DiagnosticFormatter().format(Result.diagnostics()[0]).Message, "function @main exceeded the execution limit of 1000000 instructions");
    }
  } // namespace
} // namespace ink::execution
