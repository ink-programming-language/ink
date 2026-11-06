#include "ink/ir/type/type_visitor.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    class IdentityVisitor final : public TypeVisitor<IdentityVisitor, const Type *>
    {
      public:
#define TYPE(Kind, Class, Spelling) \
        template <typename Node> const Type *visit##Class(Node &Value) \
        { \
          static_assert(std::is_same_v<std::remove_const_t<Node>, Class>); \
          WasConst = std::is_const_v<Node>; \
          return &Value; \
        }
#include "ink/ir/type/type.def"
#undef TYPE

        bool WasConst = false;
    };
  } // namespace

  // Every const concrete type reaches its exact callback and forwards the callback's result without copying the node.
  TEST(IRTypeVisitorTest, DispatchesEveryConstConcreteTypeAndReturnsItsIdentity)
  {
    IntegerType Integer(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    FloatType Float(FloatType::Width::Bits32);
    BoolType Bool;
    VoidType Void;
    ArrayType Array(Integer, 3);
    StructType Struct;
    ObjectType Object;
    ReferenceType Reference(Struct);
    FunctionType Function(Void);
    const Type *Nodes[] = {
        &MetaType::instance(),
        &Integer,
        &Float,
        &Bool,
        &Void,
        &Array,
        &Struct,
        &Object,
        &Reference,
        &Function,
    };
    IdentityVisitor Visitor;
    for (const Type *Node : Nodes)
    {
      EXPECT_EQ(Visitor.visit(*Node), Node);
      EXPECT_TRUE(Visitor.WasConst);
    }
  }

  // A mutable record is passed to a mutable callback while the same visitor preserves constness for its const view.
  TEST(IRTypeVisitorTest, PreservesMutableAndConstDispatchOnTheSameVisitor)
  {
    StructType Struct;
    Type &Mutable = Struct;
    const Type &Constant = Struct;
    IdentityVisitor Visitor;
    EXPECT_EQ(Visitor.visit(Mutable), &Struct);
    EXPECT_FALSE(Visitor.WasConst);
    EXPECT_EQ(Visitor.visit(Constant), &Struct);
    EXPECT_TRUE(Visitor.WasConst);
  }
} // namespace ink::ir::test
