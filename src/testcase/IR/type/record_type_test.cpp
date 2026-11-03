#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/class_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/struct_type.h"

#include <gtest/gtest.h>

namespace ink::ir::test
{
  // A struct retains its distinct kind and spelling while exposing value semantics through Type.
  TEST(IRTypeTest, IdentifiesStructWithValueSemanticsThroughBasePointer)
  {
    const StructType Struct;
    const Type *Base = &Struct;
    EXPECT_EQ(Base->getKind(), TypeKind::Struct);
    EXPECT_EQ(Base->getName(), "struct");
    EXPECT_EQ(typeKindName(TypeKind::Struct), "struct");
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Value);
  }

  // A class retains its distinct kind and spelling while overriding value semantics through Type.
  TEST(IRTypeTest, IdentifiesClassWithReferenceSemanticsThroughBasePointer)
  {
    const ClassType Class;
    const Type *Base = &Class;
    EXPECT_EQ(Base->getKind(), TypeKind::Class);
    EXPECT_EQ(Base->getName(), "class");
    EXPECT_EQ(typeKindName(TypeKind::Class), "class");
    EXPECT_EQ(Base->getSemantics(), TypeSemantics::Reference);
  }

  // Record kind checks distinguish struct from class and reject null, scalar, and array types.
  TEST(IRTypeTest, DistinguishesStructAndClassFromOtherKinds)
  {
    const StructType Struct;
    const ClassType Class;
    const IntegerType Int32(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const BoolType Bool;
    const ArrayType Array(Int32, 2);
    EXPECT_TRUE(StructType::classof(&Struct));
    EXPECT_TRUE(ClassType::classof(&Class));
    EXPECT_FALSE(StructType::classof(&Class));
    EXPECT_FALSE(ClassType::classof(&Struct));
    const Type *OtherTypes[] = {
        nullptr,
        &Int32,
        &Bool,
        &Array,
    };
    for (const Type *Other : OtherTypes)
    {
      EXPECT_FALSE(StructType::classof(Other));
      EXPECT_FALSE(ClassType::classof(Other));
    }
    EXPECT_FALSE(IntegerType::classof(&Struct));
    EXPECT_FALSE(IntegerType::classof(&Class));
    EXPECT_FALSE(ArrayType::classof(&Struct));
    EXPECT_FALSE(ArrayType::classof(&Class));
  }
} // namespace ink::ir::test
