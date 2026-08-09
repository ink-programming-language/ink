#include "ink/execution/interpreter.h"
#include "ink/frontend/compiler.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <optional>
#include <string>

namespace ink::frontend
{
  namespace
  {
    std::optional<ir::IrFunctionId> findFunction(const ir::IrModule &Module, const std::string &Name)
    {
      for (std::uint32_t Index = 0; Index < Module.functionCount(); ++Index)
      {
        const ir::IrFunctionId Function = ir::IrFunctionId::fromValue(Index);
        if (Module.function(Function).Name == Name)
        {
          return Function;
        }
      }
      return std::nullopt;
    }

    // Verifies one Compiler entry point carries a first-slice source program through AST, Sema, staged IR, closure, and RuntimeWorld execution.
    TEST(CompilerTest, CompilesSourceToExecutableClosedIr)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("pipeline.ink", "func add(Left: i32, Right: i32) -> i32 { return Left + Right; } func main() -> i32 { var Value: i32 = 0; while (Value < 3) { Value += 1; } if (Value == 3) { return add(Value, 39); } else { return 1; } }");
      Compiler Frontend(Session);

      ClosedCompilationResult Compiled = Frontend.compile(File);

      ASSERT_TRUE(Compiled.succeeded()) << (Compiled.Issues.empty() ? "" : Compiled.Issues.front().Message);
      ASSERT_TRUE(Compiled.Module.has_value());
      const std::optional<ir::IrFunctionId> Main = findFunction(Compiled.Module->module(), "main");
      ASSERT_TRUE(Main.has_value());
      execution::RuntimeWorld World(Compiled.Module->targetKey());
      const execution::ExecutionResult Executed = execution::interpret(*Compiled.Module, *Main, World, {});
      ASSERT_EQ(Executed.Status, execution::ExecutionStatus::Returned);
      ASSERT_TRUE(Executed.Value.has_value());
      EXPECT_EQ(Executed.Value->bits(), 42U);
    }

    // Verifies the reserved zero-argument never-returning trap declaration lowers directly to rt.trap and propagates through Compiler into RuntimeWorld without an external IR symbol.
    TEST(CompilerTest, LowersReservedTrapIntrinsicIntoRuntimeWorld)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("trap.ink", "func trap() -> never; func main() -> never { trap(); }");
      Compiler Frontend(Session);

      ClosedCompilationResult Compiled = Frontend.compile(File);

      ASSERT_TRUE(Compiled.succeeded()) << (Compiled.Issues.empty() ? "" : Compiled.Issues.front().Message);
      ASSERT_TRUE(Compiled.Module.has_value());
      const ir::IrModule &Module = Compiled.Module->module();
      EXPECT_FALSE(findFunction(Module, "trap").has_value());
      const std::optional<ir::IrFunctionId> Main = findFunction(Module, "main");
      ASSERT_TRUE(Main.has_value());
      std::size_t TrapCount = 0;
      std::size_t DirectCallCount = 0;
      for (std::uint32_t Index = 0; Index < Module.operationCount(); ++Index)
      {
        const ir::IrOpcode Opcode = Module.operation(ir::IrOperationId::fromValue(Index)).Opcode;
        TrapCount += Opcode == ir::IrOpcode::Trap ? 1U : 0U;
        DirectCallCount += Opcode == ir::IrOpcode::DirectCall ? 1U : 0U;
      }
      EXPECT_EQ(TrapCount, 1U);
      EXPECT_EQ(DirectCallCount, 0U);
      execution::RuntimeWorld World(Compiled.Module->targetKey());
      const execution::ExecutionResult Executed = execution::interpret(*Compiled.Module, *Main, World, {});
      ASSERT_EQ(Executed.Status, execution::ExecutionStatus::LanguageTrap);
      ASSERT_TRUE(Executed.Trap.has_value());
      EXPECT_EQ(*Executed.Trap, ir::IrTrapKind::User);
    }

    // Verifies semantic diagnostics stop the pipeline before an invalid source can acquire a closed IR capability.
    TEST(CompilerTest, RejectsSemanticallyInvalidSourceBeforeIr)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("invalid.ink", "func main() -> i32 { return Missing; }");
      Compiler Frontend(Session);

      ClosedCompilationResult Compiled = Frontend.compile(File);

      EXPECT_FALSE(Compiled.succeeded());
      EXPECT_FALSE(Compiled.Module.has_value());
      ASSERT_FALSE(Compiled.Issues.empty());
      EXPECT_EQ(Compiled.Issues.front().Phase, CompilationPhase::SemanticAnalysis);
      ASSERT_FALSE(Session.diagnostics().empty());
      EXPECT_EQ(Session.diagnostics().front().Kind, core::DiagnosticKind::UnresolvedName);
    }

    // Verifies parser recovery can still build an AST for tooling but Compiler never grants an IR capability when this compilation added a syntax diagnostic.
    TEST(CompilerTest, RejectsRecoveredParserErrorsBeforeSemanticAnalysis)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("recovered.ink", "func main() -> i32 { return 42 }");
      Compiler Frontend(Session);

      ClosedCompilationResult Compiled = Frontend.compile(File);

      EXPECT_FALSE(Compiled.succeeded());
      EXPECT_FALSE(Compiled.Module.has_value());
      ASSERT_FALSE(Compiled.Issues.empty());
      EXPECT_EQ(Compiled.Issues.front().Phase, CompilationPhase::Parsing);
      EXPECT_EQ(Compiled.Issues.front().File, File);
      EXPECT_EQ(Compiled.Issues.front().Severity, core::DiagnosticSeverity::Error);
      EXPECT_TRUE(Compiled.Issues.front().DiagnosticKind.has_value());
    }

    // Verifies parser diagnostics retained for an earlier file do not poison a later clean compilation in the same long-lived session.
    TEST(CompilerTest, ScopesParsingFailureToTheCurrentCompileCall)
    {
      CompilationSession Session;
      const core::SourceFileId Broken = Session.addSource("earlier-broken.ink", "func broken() -> i32 { return 1 }");
      ASSERT_TRUE(Session.parseAndLower(Broken).has_value());
      ASSERT_FALSE(Session.diagnostics().empty());
      const core::SourceFileId Clean = Session.addSource("later-clean.ink", "func main() -> i32 { return 42; }");
      Compiler Frontend(Session);

      ClosedCompilationResult Compiled = Frontend.compile(Clean);

      ASSERT_TRUE(Compiled.succeeded()) << (Compiled.Issues.empty() ? "" : Compiled.Issues.front().Message);
      ASSERT_TRUE(Compiled.Module.has_value());
      EXPECT_TRUE(Compiled.Issues.empty());
    }

    // Verifies comptime syntax becomes an explicit staged force-value plan and cannot silently cross the Closed IR boundary unresolved.
    TEST(CompilerTest, PreservesUnresolvedComptimePlanAtStagedBoundary)
    {
      CompilationSession Session;
      const core::SourceFileId File = Session.addSource("comptime.ink", "func main() -> i32 { return comptime 42; }");
      Compiler Frontend(Session);

      StagedCompilationResult Staged = Frontend.compileToStaged(File);

      ASSERT_TRUE(Staged.succeeded()) << (Staged.Issues.empty() ? "" : Staged.Issues.front().Message);
      ASSERT_TRUE(Staged.Module.has_value());
      ASSERT_EQ(Staged.PendingForceValues.size(), 1U);
      EXPECT_EQ(Staged.Module->module().planNodeCount(), 1U);
      ClosedCompilationResult Closed = Frontend.close(*Staged.Module);
      EXPECT_FALSE(Closed.succeeded());
      EXPECT_FALSE(Closed.Module.has_value());
      ASSERT_FALSE(Closed.Issues.empty());
      EXPECT_EQ(Closed.Issues.front().Phase, CompilationPhase::IrClosure);
    }
  } // namespace
} // namespace ink::frontend
