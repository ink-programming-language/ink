#include "ink/ir/ir.h"
#include "ink/ir/serialization.h"
#include "ink/ir/verifier.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    struct ControlFlowIrTestContext
    {
      core::CompilationContext Compilation;
      IRContext IR{Compilation};
    };

    bool hasDiagnostic(const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind)
    {
      for (const core::Diagnostic &DiagnosticEntry : Diagnostics)
      {
        if (DiagnosticEntry.Kind == Kind)
        {
          return true;
        }
      }
      return false;
    }

    VerificationResult verifySingleControlInstruction(ControlFlowIrTestContext &Context, std::unique_ptr<Instruction> InstructionValue)
    {
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      Module ModuleValue(Context.IR);
      Function Main(VoidType);
      Main.Name = "main";
      BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::move(InstructionValue));
      Entry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Main.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Main));
      return verify(Context.IR, ModuleValue);
    }

    const std::string CounterLoopText =
        "inkir 1\n"
        "\n"
        "define i32 @count() {\n"
        "entry:\n"
        "  br loop\n"
        "loop:\n"
        "  %0 = phi i32 [0, entry], [%2, body]\n"
        "  %1 = icmp lt i32 %0, i32 5\n"
        "  condbr bool %1, body, exit\n"
        "body:\n"
        "  %2 = add i32 %0, i32 1\n"
        "  br loop\n"
        "exit:\n"
        "  ret i32 %0\n"
        "}\n";

    // Verifies that a loop phi with a backedge forward SSA reference, forward branches, comparison, and addition round-trip canonically.
    TEST(ControlFlowIrSerializationTest, RoundTripsCounterLoop)
    {
      ControlFlowIrTestContext Context;
      DeserializeResult Parsed = deserialize(Context.IR, CounterLoopText);

      ASSERT_TRUE(Parsed.succeeded());
      ASSERT_TRUE(Parsed.module().has_value());
      SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), CounterLoopText);
    }

    // Verifies that global pointer operands and predecessor labels are fixed up inside phi incoming pairs.
    TEST(ControlFlowIrSerializationTest, RoundTripsGlobalPointerPhiIncomingValues)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "\n"
          "@data = private constant [2 x byte] c\"AB\"\n"
          "\n"
          "define const byte* @choose(bool %0) {\n"
          "entry:\n"
          "  condbr bool %0, select.first, select.second\n"
          "select.first:\n"
          "  br selected\n"
          "select.second:\n"
          "  br selected\n"
          "selected:\n"
          "  %1 = phi const byte* [@data[0], select.first], [@data[1], select.second]\n"
          "  ret const byte* %1\n"
          "}\n";

      DeserializeResult Parsed = deserialize(Context.IR, Text);

      ASSERT_TRUE(Parsed.succeeded());
      SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
      ASSERT_TRUE(Serialized.succeeded());
      EXPECT_EQ(*Serialized.text(), Text);
    }

    // Verifies the stable spelling for all supported integer comparison predicates.
    TEST(ControlFlowIrSerializationTest, RoundTripsEveryComparisonPredicate)
    {
      const char *Predicates[] = {"eq", "ne", "lt", "le", "gt", "ge"};
      for (const char *Predicate : Predicates)
      {
        ControlFlowIrTestContext Context;
        const std::string Text = "inkir 1\n\ndefine bool @compare() {\nentry:\n  %0 = icmp " + std::string(Predicate) + " i32 -1, i32 0\n  ret bool %0\n}\n";
        DeserializeResult Parsed = deserialize(Context.IR, Text);
        ASSERT_TRUE(Parsed.succeeded()) << Predicate;
        SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
        ASSERT_TRUE(Serialized.succeeded()) << Predicate;
        EXPECT_EQ(*Serialized.text(), Text) << Predicate;
      }
    }

    // Verifies that equality and inequality comparisons round-trip for both mutable and const opaque byte pointer types.
    TEST(ControlFlowIrSerializationTest, RoundTripsPointerEqualityPredicates)
    {
      struct Case
      {
        const char *Predicate;
        const char *PointerType;
      };
      const Case Cases[] = {
          {"eq", "byte*"},
          {"ne", "const byte*"},
      };

      for (const Case &CaseValue : Cases)
      {
        ControlFlowIrTestContext Context;
        const std::string Text = "inkir 1\n\ndefine bool @compare(" + std::string(CaseValue.PointerType) + " %0, " + CaseValue.PointerType + " %1) {\nentry:\n  %2 = icmp " + CaseValue.Predicate + " " + CaseValue.PointerType + " %0, " + CaseValue.PointerType + " %1\n  ret bool %2\n}\n";
        DeserializeResult Parsed = deserialize(Context.IR, Text);
        ASSERT_TRUE(Parsed.succeeded()) << CaseValue.Predicate << ' ' << CaseValue.PointerType;
        SerializeResult Serialized = serialize(Context.IR, *Parsed.module());
        ASSERT_TRUE(Serialized.succeeded()) << CaseValue.Predicate << ' ' << CaseValue.PointerType;
        EXPECT_EQ(*Serialized.text(), Text) << CaseValue.Predicate << ' ' << CaseValue.PointerType;
      }
    }

    // Verifies that branches cannot re-enter the entry block and repeat entry-only allocation setup.
    TEST(ControlFlowIrVerifierTest, RejectsBranchToEntryBlock)
    {
      const char *Bodies[] = {
          "  br loop\nloop:\n  br entry\n",
          "  condbr bool 1, entry, exit\nexit:\n  ret void\n",
      };
      for (const char *Body : Bodies)
      {
        ControlFlowIrTestContext Context;
        const std::string Text = "inkir 1\ndefine void @main() {\nentry:\n" + std::string(Body) + "}\n";
        const DeserializeResult Result = deserialize(Context.IR, Text);
        ASSERT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrBranchTargetsEntryBlock));
      }
    }

    // Verifies that a phi contains one incoming value for every predecessor edge.
    TEST(ControlFlowIrVerifierTest, RejectsPhiIncomingCountMismatch)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  br target\n"
          "other:\n"
          "  br target\n"
          "target:\n"
          "  %0 = phi i32 [1, entry]\n"
          "  ret i32 %0\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrPhiIncomingCountMismatch));
    }

    // Verifies that every phi incoming value has the exact canonical type of its result.
    TEST(ControlFlowIrVerifierTest, RejectsPhiIncomingTypeMismatch)
    {
      ControlFlowIrTestContext Context;
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const std::string Text = "inkir 1\ndefine i32 @main() {\nentry:\n  br target\ntarget:\n  %0 = phi i32 [1, entry]\n  ret i32 %0\n}\n";
      DeserializeResult Parsed = deserialize(Context.IR, Text);
      ASSERT_TRUE(Parsed.succeeded());
      auto &Phi = static_cast<PhiInstruction &>(*Parsed.module()->Functions[0].Blocks[1].Instructions[0]);
      Phi.IncomingValues[0].Value = std::make_unique<IntegerConstant>(ByteType, 1);

      const VerificationResult Result = verify(Context.IR, *Parsed.module());

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrPhiIncomingTypeMismatch));
    }

    // Verifies that a definition may be used directly in a block it dominates.
    TEST(ControlFlowIrVerifierTest, AcceptsDominatedCrossBlockValue)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  %0 = add i32 1, i32 2\n"
          "  br target\n"
          "target:\n"
          "  ret i32 %0\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      EXPECT_TRUE(Result.succeeded());
    }

    // Verifies that a value defined on only one diamond arm cannot be used after the arms merge without a phi.
    TEST(ControlFlowIrVerifierTest, RejectsNonDominatingCrossBlockValue)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main(bool %0) {\n"
          "entry:\n"
          "  condbr bool %0, left, right\n"
          "left:\n"
          "  %1 = add i32 1, i32 2\n"
          "  br join\n"
          "right:\n"
          "  br join\n"
          "join:\n"
          "  ret i32 %1\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrUnavailableSsaValue));
    }

    // Verifies that function parameters remain visible in every block without an edge argument.
    TEST(ControlFlowIrVerifierTest, AcceptsFunctionParameterInEveryBlock)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "\n"
          "define i32 @main(i32 %0) {\n"
          "entry:\n"
          "  br target\n"
          "target:\n"
          "  ret i32 %0\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      EXPECT_TRUE(Result.succeeded());
    }

    // Verifies that an entry block cannot contain phi instructions because it has no incoming control-flow edge.
    TEST(ControlFlowIrVerifierTest, RejectsPhiInEntryBlock)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define void @main() {\n"
          "entry:\n"
          "  %0 = phi i32 [0, entry]\n"
          "  ret void\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrPhiInEntryBlock));
    }

    // Verifies that phi instructions are contiguous at the start of their basic block.
    TEST(ControlFlowIrVerifierTest, RejectsPhiAfterNonPhiInstruction)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  br target\n"
          "target:\n"
          "  %0 = add i32 1, i32 2\n"
          "  %1 = phi i32 [3, entry]\n"
          "  ret i32 %1\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrPhiMustBeFirstInBlock));
    }

    // Verifies that phi incoming labels must identify actual CFG predecessors.
    TEST(ControlFlowIrVerifierTest, RejectsPhiIncomingFromNonPredecessor)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  br target\n"
          "target:\n"
          "  %0 = phi i32 [7, other]\n"
          "  ret i32 %0\n"
          "other:\n"
          "  ret i32 0\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrPhiIncomingBlockNotPredecessor));
    }

    // Verifies LLVM-compatible duplicate predecessor edges accept identical phi values.
    TEST(ControlFlowIrVerifierTest, AcceptsIdenticalPhiValuesForDuplicatePredecessorEdges)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  condbr bool 1, target, target\n"
          "target:\n"
          "  %0 = phi i32 [7, entry], [7, entry]\n"
          "  ret i32 %0\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      EXPECT_TRUE(Result.succeeded());
    }

    // Verifies duplicate edges from one predecessor cannot supply different values to the same phi.
    TEST(ControlFlowIrVerifierTest, RejectsConflictingPhiValuesForDuplicatePredecessorEdges)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main() {\n"
          "entry:\n"
          "  condbr bool 1, target, target\n"
          "target:\n"
          "  %0 = phi i32 [7, entry], [8, entry]\n"
          "  ret i32 %0\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrPhiDuplicateIncomingBlock));
    }

    // Verifies a phi incoming SSA value is available on its named predecessor edge rather than merely in the destination block.
    TEST(ControlFlowIrVerifierTest, RejectsPhiValueUnavailableOnIncomingEdge)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define i32 @main(bool %0) {\n"
          "entry:\n"
          "  condbr bool %0, left, right\n"
          "left:\n"
          "  %1 = add i32 1, i32 2\n"
          "  br join\n"
          "right:\n"
          "  br join\n"
          "join:\n"
          "  %2 = phi i32 [%1, left], [%1, right]\n"
          "  ret i32 %2\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrUnavailableSsaValue));
    }

    // Verifies that ordered predicates are rejected for bool while equality remains the only ordered-free relation.
    TEST(ControlFlowIrVerifierTest, RejectsOrderedBoolComparison)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define bool @main() {\n"
          "entry:\n"
          "  %0 = icmp lt bool 0, bool 1\n"
          "  ret bool %0\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrComparePredicateUnsupportedForType));
    }

    // Verifies that opaque logical pointers only support equality because distinct backing regions have no target-stable ordering.
    TEST(ControlFlowIrVerifierTest, RejectsOrderedPointerComparison)
    {
      const char *Predicates[] = {"lt", "le", "gt", "ge"};
      const char *PointerTypes[] = {"byte*", "const byte*"};

      for (const char *PointerType : PointerTypes)
      {
        for (const char *Predicate : Predicates)
        {
          ControlFlowIrTestContext Context;
          const std::string Text = "inkir 1\ndefine bool @main(" + std::string(PointerType) + " %0, " + PointerType + " %1) {\nentry:\n  %2 = icmp " + Predicate + " " + PointerType + " %0, " + PointerType + " %1\n  ret bool %2\n}\n";
          const DeserializeResult Result = deserialize(Context.IR, Text);

          ASSERT_FALSE(Result.succeeded()) << Predicate << ' ' << PointerType;
          EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrComparePredicateUnsupportedForType)) << Predicate << ' ' << PointerType;
        }
      }
    }

    // Verifies that safe byte slices are not accepted as comparison operands.
    TEST(ControlFlowIrVerifierTest, RejectsSliceComparison)
    {
      ControlFlowIrTestContext Context;
      const std::string Text =
          "inkir 1\n"
          "define bool @main(byte[] %0, byte[] %1) {\n"
          "entry:\n"
          "  %2 = icmp eq byte[] %0, byte[] %1\n"
          "  ret bool %2\n"
          "}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrCompareUnsupportedType));
    }

    // Verifies every invalid result, predicate, null operand, and operand-type shape for add and icmp.
    TEST(ControlFlowIrVerifierTest, RejectsMalformedArithmeticAndComparisonShapes)
    {
      ControlFlowIrTestContext Context;
      const Type &BoolType = Context.IR.getType(TypeKind::Bool);
      const Type &ByteType = Context.IR.getType(TypeKind::Byte);
      const Type &I32Type = Context.IR.getType(TypeKind::I32);
      const auto I32Zero = [&]() { return std::make_unique<IntegerConstant>(I32Type, 0); };
      const auto ExpectDiagnostic = [&](std::unique_ptr<Instruction> InstructionValue, core::DiagnosticKind Expected)
      {
        const VerificationResult Result = verifySingleControlInstruction(Context, std::move(InstructionValue));
        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), Expected)) << static_cast<unsigned int>(Expected);
      };

      {
        auto InstructionValue = std::make_unique<AddInstruction>(BoolType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Left = std::make_unique<IntegerConstant>(BoolType, 0);
        InstructionValue->Right = std::make_unique<IntegerConstant>(BoolType, 1);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrAddInvalidResultType);
      }
      for (std::size_t NullOperand = 0; NullOperand < 2; ++NullOperand)
      {
        auto InstructionValue = std::make_unique<AddInstruction>(I32Type);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Left = NullOperand == 0 ? nullptr : I32Zero();
        InstructionValue->Right = NullOperand == 1 ? nullptr : I32Zero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrAddNullOperand);
      }
      {
        auto InstructionValue = std::make_unique<AddInstruction>(I32Type);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Left = std::make_unique<IntegerConstant>(ByteType, 0);
        InstructionValue->Right = I32Zero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrAddOperandTypeMismatch);
      }
      {
        auto InstructionValue = std::make_unique<CompareInstruction>(I32Type);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Left = I32Zero();
        InstructionValue->Right = I32Zero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrCompareInvalidResultType);
      }
      {
        auto InstructionValue = std::make_unique<CompareInstruction>(BoolType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Predicate = ComparePredicate::Count;
        InstructionValue->Left = I32Zero();
        InstructionValue->Right = I32Zero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrCompareInvalidPredicate);
      }
      for (std::size_t NullOperand = 0; NullOperand < 2; ++NullOperand)
      {
        auto InstructionValue = std::make_unique<CompareInstruction>(BoolType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Left = NullOperand == 0 ? nullptr : I32Zero();
        InstructionValue->Right = NullOperand == 1 ? nullptr : I32Zero();
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrCompareNullOperand);
      }
      {
        auto InstructionValue = std::make_unique<CompareInstruction>(BoolType);
        InstructionValue->Result = ValueId{0};
        InstructionValue->Left = I32Zero();
        InstructionValue->Right = std::make_unique<IntegerConstant>(ByteType, 0);
        ExpectDiagnostic(std::move(InstructionValue), core::DiagnosticKind::IrCompareOperandTypeMismatch);
      }
    }

    // Verifies invalid phi result types, null incoming values, and malformed conditional branch conditions are rejected independently.
    TEST(ControlFlowIrVerifierTest, RejectsMalformedPhiAndBranchShapes)
    {
      {
        ControlFlowIrTestContext Context;
        const Type &VoidType = Context.IR.getType(TypeKind::Void);
        Module ModuleValue(Context.IR);
        Function Main(VoidType);
        Main.Name = "main";
        BasicBlock Entry;
        Entry.Name = "entry";
        auto Branch = std::make_unique<BranchInstruction>();
        Branch->Target.Block = BlockId{1};
        Entry.Instructions.push_back(std::move(Branch));
        BasicBlock Target;
        Target.Name = "target";
        auto Phi = std::make_unique<PhiInstruction>(VoidType);
        Phi->Result = ValueId{0};
        Phi->IncomingValues.push_back({std::make_unique<ZeroInitializer>(VoidType), BlockId{0}});
        Target.Instructions.push_back(std::move(Phi));
        Target.Instructions.push_back(std::make_unique<ReturnInstruction>());
        Main.Blocks.push_back(std::move(Entry));
        Main.Blocks.push_back(std::move(Target));
        ModuleValue.Functions.push_back(std::move(Main));
        const VerificationResult Result = verify(Context.IR, ModuleValue);
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrPhiInvalidResultType));
      }
      {
        ControlFlowIrTestContext Context;
        const Type &VoidType = Context.IR.getType(TypeKind::Void);
        const Type &I32Type = Context.IR.getType(TypeKind::I32);
        Module ModuleValue(Context.IR);
        Function Main(VoidType);
        Main.Name = "main";
        BasicBlock Entry;
        Entry.Name = "entry";
        auto Branch = std::make_unique<BranchInstruction>();
        Branch->Target.Block = BlockId{1};
        Entry.Instructions.push_back(std::move(Branch));
        BasicBlock Target;
        Target.Name = "target";
        auto Phi = std::make_unique<PhiInstruction>(I32Type);
        Phi->Result = ValueId{0};
        Phi->IncomingValues.push_back({nullptr, BlockId{0}});
        Target.Instructions.push_back(std::move(Phi));
        Target.Instructions.push_back(std::make_unique<ReturnInstruction>());
        Main.Blocks.push_back(std::move(Entry));
        Main.Blocks.push_back(std::move(Target));
        ModuleValue.Functions.push_back(std::move(Main));
        const VerificationResult Result = verify(Context.IR, ModuleValue);
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrPhiNullIncomingValue));
      }
      for (bool NullCondition : {true, false})
      {
        ControlFlowIrTestContext Context;
        const Type &VoidType = Context.IR.getType(TypeKind::Void);
        const Type &I32Type = Context.IR.getType(TypeKind::I32);
        Module ModuleValue(Context.IR);
        Function Main(VoidType);
        Main.Name = "main";
        BasicBlock Entry;
        Entry.Name = "entry";
        auto Branch = std::make_unique<ConditionalBranchInstruction>();
        if (!NullCondition)
        {
          Branch->Condition = std::make_unique<IntegerConstant>(I32Type, 1);
        }
        Branch->TrueTarget.Block = BlockId{1};
        Branch->FalseTarget.Block = BlockId{2};
        Entry.Instructions.push_back(std::move(Branch));
        BasicBlock TrueBlock;
        TrueBlock.Name = "true";
        TrueBlock.Instructions.push_back(std::make_unique<ReturnInstruction>());
        BasicBlock FalseBlock;
        FalseBlock.Name = "false";
        FalseBlock.Instructions.push_back(std::make_unique<ReturnInstruction>());
        Main.Blocks.push_back(std::move(Entry));
        Main.Blocks.push_back(std::move(TrueBlock));
        Main.Blocks.push_back(std::move(FalseBlock));
        ModuleValue.Functions.push_back(std::move(Main));
        const VerificationResult Result = verify(Context.IR, ModuleValue);
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), NullCondition ? core::DiagnosticKind::IrConditionalBranchNullCondition : core::DiagnosticKind::IrConditionalBranchConditionNotBool));
      }
    }

    // Verifies that neither unconditional nor conditional branch terminators may be followed by another instruction in the same block.
    TEST(ControlFlowIrVerifierTest, RejectsEarlyBranchTerminators)
    {
      const char *Bodies[] = {
          "  br exit\n  ret void\n",
          "  condbr bool 1, exit, exit\n  ret void\n",
      };
      for (const char *Body : Bodies)
      {
        ControlFlowIrTestContext Context;
        const std::string Text = "inkir 1\ndefine void @main() {\nentry:\n" + std::string(Body) + "exit:\n  ret void\n}\n";
        const DeserializeResult Result = deserialize(Context.IR, Text);
        ASSERT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrEarlyTerminator));
      }
    }

    // Verifies that unknown comparison predicates are rejected at their source token.
    TEST(ControlFlowIrDeserializationTest, RejectsUnknownComparisonPredicate)
    {
      ControlFlowIrTestContext Context;
      const std::string Text = "inkir 1\ndefine bool @main() {\nentry:\n  %0 = icmp unordered i32 0, i32 0\n  ret bool %0\n}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_FALSE(Result.diagnostics().empty());
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::IrUnknownComparePredicate);
    }

    // Verifies that phi is rejected during parsing when it does not define an SSA result.
    TEST(ControlFlowIrDeserializationTest, RejectsPhiWithoutResult)
    {
      ControlFlowIrTestContext Context;
      const std::string Text = "inkir 1\ndefine void @main() {\nentry:\n  phi i32 [0, entry]\n  ret void\n}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_FALSE(Result.diagnostics().empty());
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::IrPhiRequiresResult);
    }

    // Verifies that unresolved forward branch names are rejected after the complete function has been parsed.
    TEST(ControlFlowIrDeserializationTest, RejectsUnknownBasicBlockTarget)
    {
      ControlFlowIrTestContext Context;
      const std::string Text = "inkir 1\ndefine void @main() {\nentry:\n  br missing\n}\n";

      const DeserializeResult Result = deserialize(Context.IR, Text);

      ASSERT_FALSE(Result.succeeded());
      ASSERT_FALSE(Result.diagnostics().empty());
      EXPECT_EQ(Result.diagnostics()[0].Kind, core::DiagnosticKind::IrUnknownBasicBlockTarget);
    }

    // Verifies that an out-of-range programmatic BlockId is rejected even when no textual fixup is involved.
    TEST(ControlFlowIrVerifierTest, RejectsInvalidProgrammaticBlockId)
    {
      ControlFlowIrTestContext Context;
      const Type &VoidType = Context.IR.getType(TypeKind::Void);
      Module ModuleValue(Context.IR);
      Function Main(VoidType);
      Main.Name = "main";
      BasicBlock Entry;
      Entry.Name = "entry";
      auto Branch = std::make_unique<BranchInstruction>();
      Branch->Target.Block = BlockId{9};
      Entry.Instructions.push_back(std::move(Branch));
      Main.Blocks.push_back(std::move(Entry));
      ModuleValue.Functions.push_back(std::move(Main));

      const VerificationResult Result = verify(Context.IR, ModuleValue);

      ASSERT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Result.diagnostics(), core::DiagnosticKind::IrBranchInvalidTarget));
    }
  } // namespace
} // namespace ink::ir
