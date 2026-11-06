#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/instruction_visitor.h"

#include <gtest/gtest.h>

#include <array>
#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    class IdentityVisitor final : public InstructionVisitor<IdentityVisitor, const Instruction *>
    {
      public:
#define INSTRUCTION(Kind, Number, Class, Spelling) \
        template <typename Node> const Instruction *visit##Class(Node &Value) \
        { \
          static_assert(std::is_same_v<std::remove_const_t<Node>, Class>); \
          WasConst = std::is_const_v<Node>; \
          return &Value; \
        }
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION

        bool WasConst = false;
    };

    class CountingVisitor final : public InstructionVisitor<CountingVisitor>
    {
      public:
#define INSTRUCTION(Kind, Number, Class, Spelling) \
        void visit##Class(const Class &) \
        { \
          ++Count; \
        }
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION

        unsigned Count = 0;
    };
  } // namespace

  // Each instruction preserves node identity and operand access when dispatched through mutable and const base references.
  TEST(IRInstructionVisitorTest, DispatchesEveryKindWithPreservedConstnessAndResult)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const FloatConstant &Value = Constants.getFloat(2.0f);
    AddInstruction Add(Value, Value);
    AllocaInstruction Alloca(Types.getPointerType(Value.getType()));
    LoadInstruction Load(Value.getType(), Alloca);
    StoreInstruction Store(Types.getVoidType(), Alloca, Value);
    ReturnInstruction Return(Types.getVoidType(), Add);
    Function FunctionValue(Types.getFunctionType(Types.getVoidType(), std::array{Parameter(Types.getBoolType())}));
    BasicBlock &TrueTarget = FunctionValue.createBasicBlock();
    BasicBlock &FalseTarget = FunctionValue.createBasicBlock();
    BranchInstruction Branch(Types.getVoidType(), TrueTarget);
    CondBranchInstruction CondBranch(Types.getVoidType(), FunctionValue.argument(0), TrueTarget, FalseTarget);
    PhiInstruction Phi(Value.getType());
    Instruction *Nodes[] = {
        &Add,
        &Alloca,
        &Load,
        &Store,
        &Return,
        &Branch,
        &CondBranch,
        &Phi,
    };
    IdentityVisitor Visitor;
    CountingVisitor Counter;
    for (Instruction *Node : Nodes)
    {
      EXPECT_EQ(Visitor.visit(*Node), Node);
      EXPECT_FALSE(Visitor.WasConst);
      const Instruction &Constant = *Node;
      EXPECT_EQ(Visitor.visit(Constant), Node);
      EXPECT_TRUE(Visitor.WasConst);
      Counter.visit(Constant);
    }
    EXPECT_EQ(Counter.Count, 8U);
    EXPECT_EQ(&Add.getLHS(), &Value);
    EXPECT_EQ(Return.getReturnValue(), &Add);
    EXPECT_EQ(&Branch.getTarget(), &TrueTarget);
    EXPECT_EQ(&CondBranch.getCondition(), &FunctionValue.argument(0));
    EXPECT_EQ(&CondBranch.getTrueTarget(), &TrueTarget);
    EXPECT_EQ(&CondBranch.getFalseTarget(), &FalseTarget);
  }
} // namespace ink::ir::test
