#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/verification/verifier.h"

#include <gtest/gtest.h>

#include <array>

namespace ink::ir::test
{
  namespace
  {
    class FunctionVerificationContext final
    {
      public:
        FunctionVerificationContext()
            : Int32(Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed)),
              One(Int32, 1),
              Two(Int32, 2),
              Builder(Types)
        {
        }

        TypePool Types;
        const IntegerType &Int32;
        const IntegerConstant One;
        const IntegerConstant Two;
        const core::DiagnosticEngine Engine;
        InstructionBuilder Builder;
    };

    class VerificationDiamond final
    {
      public:
        explicit VerificationDiamond(FunctionVerificationContext &Context)
            : Owner(Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Context.Types.getBoolType())})),
              Entry(Owner.createBasicBlock()),
              Left(Owner.createBasicBlock()),
              Right(Owner.createBasicBlock()),
              Merge(Owner.createBasicBlock())
        {
          Context.Builder.setInsertPoint(Entry);
          static_cast<void>(Context.Builder.createCondBranch(Owner.argument(0), Left, Right));
          Context.Builder.setInsertPoint(Left);
          static_cast<void>(Context.Builder.createBranch(Merge));
          Context.Builder.setInsertPoint(Right);
          static_cast<void>(Context.Builder.createBranch(Merge));
        }

        Function Owner;
        BasicBlock &Entry;
        BasicBlock &Left;
        BasicBlock &Right;
        BasicBlock &Merge;
    };
  } // namespace

  // An empty declaration and a completed straight-line integer expression both verify without a module owner.
  TEST(IRFunctionVerifierTest, AcceptsDeclarationsAndOwnedStraightLineDefinitions)
  {
    FunctionVerificationContext Context;
    Function Declaration(Context.Types.getFunctionType(Context.Int32));
    verifyFunction(Declaration, Context.Engine);
    Function Definition(Context.Types.getFunctionType(Context.Int32));
    BasicBlock &Entry = Definition.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    AddInstruction &Sum = Context.Builder.createAdd(Context.One, Context.Two);
    static_cast<void>(Context.Builder.createReturn(Sum));
    verifyFunction(Definition, Context.Engine);
    EXPECT_EQ(Entry.instructionCount(), 2U);
  }

  // Phi inputs in a diamond are checked at the ends of their respective predecessor edges.
  TEST(IRFunctionVerifierTest, AcceptsDiamondPhiValuesDefinedOnTheirOwnEdges)
  {
    FunctionVerificationContext Context;
    VerificationDiamond Graph(Context);
    ASSERT_TRUE(Context.Builder.setInsertPoint(*Graph.Left.terminator()));
    AddInstruction &LeftValue = Context.Builder.createAdd(Context.One, Context.One);
    ASSERT_TRUE(Context.Builder.setInsertPoint(*Graph.Right.terminator()));
    AddInstruction &RightValue = Context.Builder.createAdd(Context.Two, Context.Two);
    Context.Builder.setInsertPoint(Graph.Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(LeftValue, Graph.Left);
    Phi.addIncoming(RightValue, Graph.Right);
    static_cast<void>(Context.Builder.createReturn(Phi));
    verifyFunction(Graph.Owner, Context.Engine);
    EXPECT_EQ(Phi.incomingCount(), 2U);
  }

  // A loop phi can be completed after its backedge value is created in a block dominated by the loop header.
  TEST(IRFunctionVerifierTest, AcceptsLoopPhiWithStagedBackedgeValues)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Context.Types.getBoolType())}));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Header = Owner.createBasicBlock();
    BasicBlock &Body = Owner.createBasicBlock();
    BasicBlock &Exit = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createBranch(Header));
    Context.Builder.setInsertPoint(Header);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    static_cast<void>(Context.Builder.createCondBranch(Owner.argument(0), Body, Exit));
    Context.Builder.setInsertPoint(Body);
    AddInstruction &Next = Context.Builder.createAdd(Phi, Context.One);
    static_cast<void>(Context.Builder.createBranch(Header));
    Context.Builder.setInsertPoint(Exit);
    static_cast<void>(Context.Builder.createReturn(Phi));
    Phi.addIncoming(Context.One, Entry);
    Phi.addIncoming(Next, Body);
    verifyFunction(Owner, Context.Engine);
    EXPECT_EQ(Phi.incomingCount(), 2U);
  }

  // Repeated conditional edges require repeated phi inputs with the same value identity.
  TEST(IRFunctionVerifierTest, AcceptsRepeatedConditionalEdgesWithMatchingPhiValues)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Context.Types.getBoolType())}));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Merge = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createCondBranch(Owner.argument(0), Merge, Merge));
    Context.Builder.setInsertPoint(Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(Context.One, Entry);
    Phi.addIncoming(Context.One, Entry);
    static_cast<void>(Context.Builder.createReturn(Phi));
    verifyFunction(Owner, Context.Engine);
    EXPECT_EQ(Merge.successorCount(), 0U);
  }

  // Complete unreachable blocks permit cross-block uses without dominance while preserving ownership and local instruction order.
  TEST(IRFunctionVerifierTest, AcceptsUnreachableBlocksWithOwnedDataOperands)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &DeadUse = Owner.createBasicBlock();
    BasicBlock &DeadDefinition = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createReturn(Context.One));
    Context.Builder.setInsertPoint(DeadDefinition);
    AddInstruction &Definition = Context.Builder.createAdd(Context.One, Context.Two);
    static_cast<void>(Context.Builder.createReturn(Definition));
    Context.Builder.setInsertPoint(DeadUse);
    AddInstruction &Use = Context.Builder.createAdd(Definition, Context.One);
    static_cast<void>(Context.Builder.createReturn(Use));
    verifyFunction(Owner, Context.Engine);
    EXPECT_EQ(Owner.basicBlockCount(), 3U);
  }

  // Unreachable phi predecessor edges waive cross-block dominance without waiving the incoming value's type or ownership.
  TEST(IRFunctionVerifierTest, AcceptsUnreachablePhiEdgesWithoutCrossBlockDominance)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Merge = Owner.createBasicBlock();
    BasicBlock &Dead = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createBranch(Merge));
    Context.Builder.setInsertPoint(Dead);
    static_cast<void>(Context.Builder.createBranch(Merge));
    Context.Builder.setInsertPoint(Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(Context.One, Entry);
    Phi.addIncoming(Phi, Dead);
    static_cast<void>(Context.Builder.createReturn(Phi));
    verifyFunction(Owner, Context.Engine);
    EXPECT_EQ(Phi.incomingCount(), 2U);
  }

  // Incomplete blocks report a registered fatal diagnostic even when assertion checks are disabled.
  TEST(IRFunctionVerifierDeathTest, ReportsMissingTerminatorsThroughTheDiagnosticEngine)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    static_cast<void>(Owner.createBasicBlock());
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*basic block is missing a terminator");
  }

  // A phi whose incoming value has another concrete integer width fails verification after valid graph construction.
  TEST(IRFunctionVerifierDeathTest, ReportsPhiIncomingTypeMismatch)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Merge = Owner.createBasicBlock();
    const IntegerConstant WrongType(Context.Types.getIntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Signed), 1);
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createBranch(Merge));
    Context.Builder.setInsertPoint(Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(WrongType, Entry);
    static_cast<void>(Context.Builder.createReturn(Phi));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*phi incoming type does not match its result type");
  }

  // A phi must describe every predecessor edge, rather than only one arm of a diamond.
  TEST(IRFunctionVerifierDeathTest, ReportsMissingPhiPredecessorEdges)
  {
    FunctionVerificationContext Context;
    VerificationDiamond Graph(Context);
    Context.Builder.setInsertPoint(Graph.Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(Context.One, Graph.Left);
    static_cast<void>(Context.Builder.createReturn(Phi));
    EXPECT_DEATH(verifyFunction(Graph.Owner, Context.Engine), "INK-I0001.*phi incoming edges do not match CFG predecessors");
  }

  // Extra phi inputs cannot create predecessor edges that are absent from the control-flow graph.
  TEST(IRFunctionVerifierDeathTest, ReportsExtraPhiPredecessorEdges)
  {
    FunctionVerificationContext Context;
    VerificationDiamond Graph(Context);
    Context.Builder.setInsertPoint(Graph.Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(Context.One, Graph.Left);
    Phi.addIncoming(Context.Two, Graph.Right);
    Phi.addIncoming(Context.One, Graph.Left);
    static_cast<void>(Context.Builder.createReturn(Phi));
    EXPECT_DEATH(verifyFunction(Graph.Owner, Context.Engine), "INK-I0001.*phi incoming edges do not match CFG predecessors");
  }

  // A same-function block that does not branch to a phi block cannot be substituted for an actual predecessor.
  TEST(IRFunctionVerifierDeathTest, ReportsPhiInputsFromNonPredecessors)
  {
    FunctionVerificationContext Context;
    VerificationDiamond Graph(Context);
    Context.Builder.setInsertPoint(Graph.Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(Context.One, Graph.Entry);
    Phi.addIncoming(Context.Two, Graph.Right);
    static_cast<void>(Context.Builder.createReturn(Phi));
    EXPECT_DEATH(verifyFunction(Graph.Owner, Context.Engine), "INK-I0001.*phi incoming edges do not match CFG predecessors");
  }

  // Two CFG edges from one predecessor cannot provide distinct value identities to the same phi.
  TEST(IRFunctionVerifierDeathTest, ReportsConflictingValuesOnRepeatedPhiPredecessorEdges)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Context.Types.getBoolType())}));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Merge = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createCondBranch(Owner.argument(0), Merge, Merge));
    Context.Builder.setInsertPoint(Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(Context.One, Entry);
    Phi.addIncoming(Context.Two, Entry);
    static_cast<void>(Context.Builder.createReturn(Phi));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*phi has different values for repeated predecessor edges");
  }

  // Data operands cannot borrow an argument owned by another function even when their integer types match.
  TEST(IRFunctionVerifierDeathTest, ReportsForeignArgumentOperands)
  {
    FunctionVerificationContext Context;
    Function Other(Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Context.Int32)}));
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    AddInstruction &Sum = Context.Builder.createAdd(Other.argument(0), Context.One);
    static_cast<void>(Context.Builder.createReturn(Sum));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*operand belongs to another function");
  }

  // Unreachable blocks still reject foreign arguments instead of treating reachability as an ownership exemption.
  TEST(IRFunctionVerifierDeathTest, ReportsForeignArgumentsInsideUnreachableBlocks)
  {
    FunctionVerificationContext Context;
    Function Other(Context.Types.getFunctionType(Context.Int32, std::array{Parameter(Context.Int32)}));
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    static_cast<void>(Context.Builder.createReturn(Context.One));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    AddInstruction &Use = Context.Builder.createAdd(Other.argument(0), Context.One);
    static_cast<void>(Context.Builder.createReturn(Use));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*operand belongs to another function");
  }

  // Instructions owned by another function cannot become local data operands through their shared result type.
  TEST(IRFunctionVerifierDeathTest, ReportsForeignInstructionOperands)
  {
    FunctionVerificationContext Context;
    Function Other(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Other.createBasicBlock());
    AddInstruction &Foreign = Context.Builder.createAdd(Context.One, Context.Two);
    static_cast<void>(Context.Builder.createReturn(Foreign));
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    AddInstruction &Use = Context.Builder.createAdd(Foreign, Context.One);
    static_cast<void>(Context.Builder.createReturn(Use));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*operand belongs to another function");
  }

  // A correctly typed but detached instruction must be attached before its result is used in a function body.
  TEST(IRFunctionVerifierDeathTest, ReportsDetachedInstructionOperands)
  {
    FunctionVerificationContext Context;
    const AddInstruction Detached(Context.One, Context.Two);
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    AddInstruction &Use = Context.Builder.createAdd(Detached, Context.One);
    static_cast<void>(Context.Builder.createReturn(Use));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*operand instruction is detached");
  }

  // Inserting a use before its existing definition violates local order without requiring an invalid builder operation.
  TEST(IRFunctionVerifierDeathTest, ReportsSameBlockForwardReferences)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    AddInstruction &Definition = Context.Builder.createAdd(Context.One, Context.Two);
    static_cast<void>(Context.Builder.createReturn(Definition));
    ASSERT_TRUE(Context.Builder.setInsertPoint(Definition));
    static_cast<void>(Context.Builder.createAdd(Definition, Context.One));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001.*definition does not precede its use");
    Function DeadOwner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(DeadOwner.createBasicBlock());
    static_cast<void>(Context.Builder.createReturn(Context.One));
    Context.Builder.setInsertPoint(DeadOwner.createBasicBlock());
    AddInstruction &DeadDefinition = Context.Builder.createAdd(Context.One, Context.Two);
    static_cast<void>(Context.Builder.createReturn(DeadDefinition));
    ASSERT_TRUE(Context.Builder.setInsertPoint(DeadDefinition));
    static_cast<void>(Context.Builder.createAdd(DeadDefinition, Context.One));
    EXPECT_DEATH(verifyFunction(DeadOwner, Context.Engine), "INK-I0001.*definition does not precede its use");
  }

  // A value defined on one diamond arm does not dominate an ordinary instruction on the other reachable arm.
  TEST(IRFunctionVerifierDeathTest, ReportsOrdinaryCrossBlockUsesWithoutDominance)
  {
    FunctionVerificationContext Context;
    VerificationDiamond Graph(Context);
    ASSERT_TRUE(Context.Builder.setInsertPoint(*Graph.Left.terminator()));
    AddInstruction &LeftValue = Context.Builder.createAdd(Context.One, Context.Two);
    ASSERT_TRUE(Context.Builder.setInsertPoint(*Graph.Right.terminator()));
    static_cast<void>(Context.Builder.createAdd(LeftValue, Context.One));
    Context.Builder.setInsertPoint(Graph.Merge);
    static_cast<void>(Context.Builder.createReturn(Context.One));
    EXPECT_DEATH(verifyFunction(Graph.Owner, Context.Engine), "INK-I0001.*definition does not dominate its use");
  }

  // Phi inputs are uses on their individual edges, so a left-arm definition cannot supply the right-arm incoming edge.
  TEST(IRFunctionVerifierDeathTest, ReportsPhiEdgeUsesWithoutDominance)
  {
    FunctionVerificationContext Context;
    VerificationDiamond Graph(Context);
    ASSERT_TRUE(Context.Builder.setInsertPoint(*Graph.Left.terminator()));
    AddInstruction &LeftValue = Context.Builder.createAdd(Context.One, Context.Two);
    Context.Builder.setInsertPoint(Graph.Merge);
    PhiInstruction &Phi = Context.Builder.createPhi(Context.Int32);
    Phi.addIncoming(LeftValue, Graph.Left);
    Phi.addIncoming(LeftValue, Graph.Right);
    static_cast<void>(Context.Builder.createReturn(Phi));
    EXPECT_DEATH(verifyFunction(Graph.Owner, Context.Engine), "INK-I0001.*definition does not dominate its use");
  }

  // A backedge to the definition entry is rejected by verification even though the CFG query API can represent it.
  TEST(IRFunctionVerifierDeathTest, ReportsEntryBlocksWithIncomingEdges)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Types.getVoidType()));
    BasicBlock &Entry = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createBranch(Entry));
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001");
  }

  // Empty phis cannot produce values in either the entry block or an unreachable block with no predecessor edges.
  TEST(IRFunctionVerifierDeathTest, ReportsEmptyPhisWithoutPredecessors)
  {
    FunctionVerificationContext Context;
    Function EntryOwner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(EntryOwner.createBasicBlock());
    PhiInstruction &EntryPhi = Context.Builder.createPhi(Context.Int32);
    static_cast<void>(Context.Builder.createReturn(EntryPhi));
    EXPECT_DEATH(verifyFunction(EntryOwner, Context.Engine), "INK-I0001.*phi requires at least one predecessor");
    Function DeadOwner(Context.Types.getFunctionType(Context.Int32));
    Context.Builder.setInsertPoint(DeadOwner.createBasicBlock());
    static_cast<void>(Context.Builder.createReturn(Context.One));
    Context.Builder.setInsertPoint(DeadOwner.createBasicBlock());
    PhiInstruction &DeadPhi = Context.Builder.createPhi(Context.Int32);
    static_cast<void>(Context.Builder.createReturn(DeadPhi));
    EXPECT_DEATH(verifyFunction(DeadOwner, Context.Engine), "INK-I0001.*phi requires at least one predecessor");
  }

  // A phi producing MetaType can be used as allocation type metadata after its definition in the same function.
  TEST(IRFunctionVerifierTest, AcceptsDefinedTypeProducingPhiInAllocationMetadata)
  {
    FunctionVerificationContext Context;
    Function Owner(Context.Types.getFunctionType(Context.Types.getVoidType(), std::array{Parameter(Context.Types.getBoolType())}));
    BasicBlock &Entry = Owner.createBasicBlock();
    BasicBlock &Left = Owner.createBasicBlock();
    BasicBlock &Right = Owner.createBasicBlock();
    BasicBlock &Merge = Owner.createBasicBlock();
    Context.Builder.setInsertPoint(Entry);
    static_cast<void>(Context.Builder.createCondBranch(Owner.argument(0), Left, Right));
    Context.Builder.setInsertPoint(Left);
    static_cast<void>(Context.Builder.createBranch(Merge));
    Context.Builder.setInsertPoint(Right);
    static_cast<void>(Context.Builder.createBranch(Merge));
    Context.Builder.setInsertPoint(Merge);
    PhiInstruction &TypePhi = Context.Builder.createPhi(Context.Types.getMetaType());
    TypePhi.addIncoming(Context.Int32, Left);
    TypePhi.addIncoming(Context.Int32, Right);
    AllocaInstruction &Slot = Context.Builder.createAlloca(TypePhi);
    static_cast<void>(Context.Builder.createReturn());
    verifyFunction(Owner, Context.Engine);
    EXPECT_EQ(&Slot.getAllocatedType(), &TypePhi);
  }

  // Pointer wrappers cannot hide a detached type-producing phi from the instruction metadata dependency checks.
  TEST(IRFunctionVerifierDeathTest, ReportsDetachedTypeProducingInstructionsInsideMetadata)
  {
    FunctionVerificationContext Context;
    const PhiInstruction Detached(Context.Types.getMetaType());
    const PointerType &Wrapped = Context.Types.getPointerType(Detached);
    Function Owner(Context.Types.getFunctionType(Context.Types.getVoidType()));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    static_cast<void>(Context.Builder.createAlloca(Wrapped));
    static_cast<void>(Context.Builder.createReturn());
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001");
  }

  // Type metadata may borrow generic arguments, but it cannot borrow an instruction result from another function.
  TEST(IRFunctionVerifierDeathTest, ReportsForeignTypeProducingInstructionsInsideMetadata)
  {
    FunctionVerificationContext Context;
    Function Other(Context.Types.getFunctionType(Context.Types.getMetaType()));
    BasicBlock &OtherEntry = Other.createBasicBlock();
    BasicBlock &OtherMerge = Other.createBasicBlock();
    Context.Builder.setInsertPoint(OtherEntry);
    static_cast<void>(Context.Builder.createBranch(OtherMerge));
    Context.Builder.setInsertPoint(OtherMerge);
    PhiInstruction &ForeignType = Context.Builder.createPhi(Context.Types.getMetaType());
    ForeignType.addIncoming(Context.Int32, OtherEntry);
    static_cast<void>(Context.Builder.createReturn(ForeignType));
    verifyFunction(Other, Context.Engine);
    const PointerType &Wrapped = Context.Types.getPointerType(ForeignType);
    Function Owner(Context.Types.getFunctionType(Context.Types.getVoidType()));
    Context.Builder.setInsertPoint(Owner.createBasicBlock());
    static_cast<void>(Context.Builder.createAlloca(Wrapped));
    static_cast<void>(Context.Builder.createReturn());
    EXPECT_DEATH(verifyFunction(Owner, Context.Engine), "INK-I0001");
  }
} // namespace ink::ir::test
