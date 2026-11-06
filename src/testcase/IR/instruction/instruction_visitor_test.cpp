#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/instruction/instruction_visitor.h"

#include <gtest/gtest.h>

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
    Instruction *Nodes[] = {
        &Add,
        &Alloca,
        &Load,
        &Store,
        &Return,
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
    EXPECT_EQ(Counter.Count, 5U);
    EXPECT_EQ(&Add.getLHS(), &Value);
    EXPECT_EQ(Return.getReturnValue(), &Add);
  }
} // namespace ink::ir::test
