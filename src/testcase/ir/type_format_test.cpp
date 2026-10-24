#include "ink/core/name/name_pool.h"
#include "ink/ir/module/module.h"

#include <gtest/gtest.h>

#include <limits>
#include <string>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);
  } // namespace

  // Numeric spellings preserve the represented width and signedness rather than using broad kind names.
  TEST(IRTypeFormatTest, FormatsConcreteScalarTypesAndMarkers)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    EXPECT_EQ(Types.get<ink::ir::MetaType>()->toString(), "type");
    EXPECT_EQ(Types.get<ink::ir::BoolType>()->toString(), "bool");
    EXPECT_EQ(Types.get<ink::ir::VoidType>()->toString(), "void");
    EXPECT_EQ(Types.get<ink::ir::IntegerType>(8, true)->toString(), "i8");
    EXPECT_EQ(Types.get<ink::ir::IntegerType>(8, false)->toString(), "u8");
    EXPECT_EQ(Types.get<ink::ir::IntegerType>(16, true)->toString(), "i16");
    EXPECT_EQ(Types.get<ink::ir::IntegerType>(16, false)->toString(), "u16");
    EXPECT_EQ(Types.get<ink::ir::IntegerType>(32, true)->toString(), "i32");
    EXPECT_EQ(Types.get<ink::ir::IntegerType>(32, false)->toString(), "u32");
    EXPECT_EQ(Types.get<ink::ir::IntegerType>(64, true)->toString(), "i64");
    EXPECT_EQ(Types.get<ink::ir::IntegerType>(64, false)->toString(), "u64");
    EXPECT_EQ(Types.get<ink::ir::FloatType>(32)->toString(), "f32");
    EXPECT_EQ(Types.get<ink::ir::FloatType>(64)->toString(), "f64");
    EXPECT_EQ(Types.get<ink::ir::NullPointerType>()->toString(), "null_pointer");
    EXPECT_EQ(Types.get<ink::ir::VariadicType>()->toString(), "variadic");
  }

  // Structural spellings retain nested arrays, pointers and references in expression order.
  TEST(IRTypeFormatTest, FormatsNestedStructuralTypes)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const auto *Array = Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(8, false), 3);
    const auto *Reference = Types.get<ink::ir::ReferenceType>(Array);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Reference);
    ASSERT_NE(Pointer, nullptr);
    EXPECT_EQ(Pointer->toString(), "*&[u8; 3]");
    EXPECT_EQ(Types.get<ink::ir::ArrayType>(Pointer, 2)->toString(), "[*&[u8; 3]; 2]");
    EXPECT_EQ(Types.get<ink::ir::PointerType>(Types.get<ink::ir::VoidType>())->toString(), "*void");
  }

  // Array counts use the complete unsigned 64-bit range even when their storage size is zero.
  TEST(IRTypeFormatTest, FormatsMaximumArrayCountWithoutTruncation)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const auto *Empty = Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(8, false), 0);
    const auto *Array = Types.get<ink::ir::ArrayType>(Empty, std::numeric_limits<std::uint64_t>::max());
    ASSERT_NE(Array, nullptr);
    EXPECT_EQ(Array->toString(), "[[u8; 0]; 18446744073709551615]");
  }

  // Function spellings preserve ordered arguments, variadic flags and nested callable return types.
  TEST(IRTypeFormatTest, FormatsFunctionSignaturesAndVariadicArguments)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const auto *Callback = Types.get<ink::ir::FunctionType>(Types.get<ink::ir::BoolType>(), {Types.get<ink::ir::IntegerType>(8, false)});
    const auto *Function = Types.get<ink::ir::FunctionType>(Callback, {{Types.get<ink::ir::IntegerType>(32, true), true}, Types.get<ink::ir::PointerType>(Types.get<ink::ir::FloatType>(64)), {Types.get<ink::ir::IntegerType>(64, false), true}});
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->toString(), "func(i32..., *f64, u64...): func(u8): bool");
    EXPECT_EQ(Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>())->toString(), "func(): void");
  }

  // Nominal declarations use current names without expanding self-referential fields and fall back to their kinds when unnamed.
  TEST(IRTypeFormatTest, FormatsNominalDeclarationNames)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    auto &Names = core::NamePool::instance();
    auto *Struct = Owner.createStructType(8, 8, Names.intern("FormatNode"));
    ASSERT_NE(Struct, nullptr);
    ASSERT_TRUE(Struct->addField(Field(Context.typePool().get<ink::ir::PointerType>(Struct), Names.intern("Next"), 0)));
    EXPECT_EQ(Struct->toString(), "FormatNode");
    ASSERT_TRUE(Struct->setName(Names.intern("RenamedFormatNode")));
    EXPECT_EQ(Struct->toString(), "RenamedFormatNode");
    EXPECT_EQ(Owner.createStructType(0, 1)->toString(), "struct");
    EXPECT_EQ(Owner.createClassType(0, 1, Names.intern("FormatClass"))->toString(), "FormatClass");
    EXPECT_EQ(Owner.createInterfaceType(0, 1, Names.intern("FormatInterface"))->toString(), "FormatInterface");
  }

  // Composite types use named leaf spellings for nested elements, parameters and return types.
  TEST(IRTypeFormatTest, FormatsNamedLeavesInsideCompositeTypes)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    auto &Types = Context.typePool();
    auto *Leaf = Owner.createStructType(8, 8, core::NamePool::instance().intern("FormatLeaf"));
    ASSERT_NE(Leaf, nullptr);
    const auto *Reference = Types.get<ink::ir::ReferenceType>(Leaf);
    const auto *Array = Types.get<ink::ir::ArrayType>(Reference, 3);
    const auto *Pointer = Types.get<ink::ir::PointerType>(Array);
    const Value *Signature = Types.get<ink::ir::FunctionType>(Leaf, {{Pointer, true}});
    ASSERT_NE(Signature, nullptr);
    EXPECT_EQ(Signature->toString(), "func(*[&FormatLeaf; 3]...): FormatLeaf");
  }

  // Formatting a deeply nested pointer chain preserves every pointer marker without exhausting the call stack.
  TEST(IRTypeFormatTest, FormatsDeepPointerChainsWithoutStackOverflow)
  {
    IRContext Context(Target64);
    const Type *Current = Context.typePool().get<ink::ir::IntegerType>(32, true);
    constexpr std::size_t Depth = 16384;
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      Current = Context.typePool().get<ink::ir::PointerType>(Current);
      ASSERT_NE(Current, nullptr);
    }
    const Value &ValueObject = *Current;
    EXPECT_EQ(ValueObject.toString(), std::string(Depth, '*') + "i32");
  }

  // Deeply nested callable, array, reference and pointer types preserve their complete spelling without exhausting the call stack.
  TEST(IRTypeFormatTest, FormatsDeepMixedCompositeTypesWithoutStackOverflow)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const Type *Current = Types.get<ink::ir::IntegerType>(32, true);
    constexpr std::size_t Depth = 4096;
    std::string Expected;
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      Current = Types.get<ink::ir::PointerType>(Current);
      Current = Types.get<ink::ir::ReferenceType>(Current);
      Current = Types.get<ink::ir::ArrayType>(Current, 1);
      Current = Types.get<ink::ir::FunctionType>(Current);
      ASSERT_NE(Current, nullptr);
      Expected += "func(): [&*";
    }
    Expected += "i32";
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      Expected += "; 1]";
    }
    const Value &ValueObject = *Current;
    EXPECT_EQ(ValueObject.toString(), Expected);
  }
} // namespace ink::ir::test
