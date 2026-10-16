#include "ink/core/name/name_pool.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/generic_type/generic_type.h"

#include <gtest/gtest.h>

#include <limits>
#include <string>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::LittleEndian);

    class SyntheticType final : public Type
    {
      public:
        explicit SyntheticType(ValueKind Kind) noexcept
            : Type(Kind)
        {
        }

        std::uint64_t size() const noexcept override
        {
          return 0;
        }

        std::uint64_t alignment() const noexcept override
        {
          return 1;
        }
    };

    class SyntheticGenericType final : public GenericType
    {
      public:
        SyntheticGenericType(ValueKind Kind, core::Name Name) noexcept
            : GenericType(Kind, Name, {})
        {
        }
    };
  } // namespace

  // Numeric spellings preserve the represented width and signedness rather than using broad kind names.
  TEST(IRTypeFormatTest, FormatsConcreteScalarTypesAndMarkers)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    EXPECT_EQ(Types.getBoolType()->toString(), "bool");
    EXPECT_EQ(Types.getVoidType()->toString(), "void");
    EXPECT_EQ(Types.getInt8Type()->toString(), "i8");
    EXPECT_EQ(Types.getUInt8Type()->toString(), "u8");
    EXPECT_EQ(Types.getInt16Type()->toString(), "i16");
    EXPECT_EQ(Types.getUInt16Type()->toString(), "u16");
    EXPECT_EQ(Types.getInt32Type()->toString(), "i32");
    EXPECT_EQ(Types.getUInt32Type()->toString(), "u32");
    EXPECT_EQ(Types.getInt64Type()->toString(), "i64");
    EXPECT_EQ(Types.getUInt64Type()->toString(), "u64");
    EXPECT_EQ(Types.getFloat32Type()->toString(), "f32");
    EXPECT_EQ(Types.getFloat64Type()->toString(), "f64");
    EXPECT_EQ(Types.getNullPointerType()->toString(), "null_pointer");
    EXPECT_EQ(Types.getVariadicType()->toString(), "variadic");
  }

  // Structural spellings retain nested arrays, pointers and references in expression order.
  TEST(IRTypeFormatTest, FormatsNestedStructuralTypes)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const auto *Array = Types.getArrayType(Types.getUInt8Type(), 3);
    const auto *Reference = Types.getReferenceType(Array);
    const auto *Pointer = Types.getPointerType(Reference);
    ASSERT_NE(Pointer, nullptr);
    EXPECT_EQ(Pointer->toString(), "*&[u8; 3]");
    EXPECT_EQ(Types.getArrayType(Pointer, 2)->toString(), "[*&[u8; 3]; 2]");
    EXPECT_EQ(Types.getPointerType(Types.getVoidType())->toString(), "*void");
  }

  // Array counts use the complete unsigned 64-bit range even when their storage size is zero.
  TEST(IRTypeFormatTest, FormatsMaximumArrayCountWithoutTruncation)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const auto *Empty = Types.getArrayType(Types.getUInt8Type(), 0);
    const auto *Array = Types.getArrayType(Empty, std::numeric_limits<std::uint64_t>::max());
    ASSERT_NE(Array, nullptr);
    EXPECT_EQ(Array->toString(), "[[u8; 0]; 18446744073709551615]");
  }

  // Function spellings preserve ordered arguments, variadic flags and nested callable return types.
  TEST(IRTypeFormatTest, FormatsFunctionSignaturesAndVariadicArguments)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const auto *Callback = Types.getFunctionType(Types.getBoolType(), {Types.getUInt8Type()});
    const auto *Function = Types.getFunctionType(Callback, {{Types.getInt32Type(), true}, Types.getPointerType(Types.getFloat64Type()), {Types.getUInt64Type(), true}});
    ASSERT_NE(Function, nullptr);
    EXPECT_EQ(Function->toString(), "func(i32..., *f64, u64...): func(u8): bool");
    EXPECT_EQ(Types.getFunctionType(Types.getVoidType())->toString(), "func(): void");
  }

  // Nominal declarations use current names without expanding self-referential fields and fall back to their kinds when unnamed.
  TEST(IRTypeFormatTest, FormatsNominalDeclarationNames)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    auto &Names = core::NamePool::instance();
    auto *Struct = Owner.createStructType(8, 8, Names.intern("FormatNode"));
    ASSERT_NE(Struct, nullptr);
    ASSERT_TRUE(Struct->addField(Field(Context.typePool().getPointerType(Struct), Names.intern("Next"), 0)));
    EXPECT_EQ(Struct->toString(), "FormatNode");
    ASSERT_TRUE(Struct->setName(Names.intern("RenamedFormatNode")));
    EXPECT_EQ(Struct->toString(), "RenamedFormatNode");
    EXPECT_EQ(Owner.createStructType(0, 1)->toString(), "struct");
    EXPECT_EQ(Owner.createClassType(0, 1, Names.intern("FormatClass"))->toString(), "FormatClass");
    EXPECT_EQ(Owner.createInterfaceType(0, 1, Names.intern("FormatInterface"))->toString(), "FormatInterface");
  }

  // Generic declarations print their available names without requiring runtime layout or instantiated members.
  TEST(IRTypeFormatTest, FormatsGenericDeclarationNamesAndUnnamedFallbacks)
  {
    auto &Names = core::NamePool::instance();
    const SyntheticGenericType Class(ValueKind::GenericClass, Names.intern("FormatTemplate"));
    const SyntheticGenericType Function(ValueKind::GenericFunction, Names.intern("FormatFunction"));
    const SyntheticGenericType Variable(ValueKind::GenericVar, Names.intern("FormatVariable"));
    const SyntheticGenericType Unnamed(ValueKind::GenericClass, core::NAME_None);
    EXPECT_EQ(Class.toString(), "FormatTemplate");
    EXPECT_EQ(Function.toString(), "FormatFunction");
    EXPECT_EQ(Variable.toString(), "FormatVariable");
    EXPECT_EQ(Unnamed.toString(), "generic_class");
  }

  // Composite types use named leaf spellings for nested elements, parameters and return types.
  TEST(IRTypeFormatTest, FormatsNamedLeavesInsideCompositeTypes)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    auto &Types = Context.typePool();
    auto *Leaf = Owner.createStructType(8, 8, core::NamePool::instance().intern("FormatLeaf"));
    ASSERT_NE(Leaf, nullptr);
    const auto *Reference = Types.getReferenceType(Leaf);
    const auto *Array = Types.getArrayType(Reference, 3);
    const auto *Pointer = Types.getPointerType(Array);
    const Value *Signature = Types.getFunctionType(Leaf, {{Pointer, true}});
    ASSERT_NE(Signature, nullptr);
    EXPECT_EQ(Signature->toString(), "func(*[&FormatLeaf; 3]...): FormatLeaf");
  }

  // External Type subclasses need not implement a concrete kind's metadata to obtain a safe fallback spelling.
  TEST(IRTypeFormatTest, FormatsSyntheticAndUnknownTypesWithoutInvalidDowncasts)
  {
    const SyntheticType Struct(ValueKind::Struct);
    const SyntheticType Integer(ValueKind::Integer);
    const SyntheticType Pointer(ValueKind::Pointer);
    const SyntheticType Unknown(static_cast<ValueKind>(255));
    EXPECT_EQ(Struct.toString(), "struct");
    EXPECT_EQ(Integer.toString(), "integer");
    EXPECT_EQ(Pointer.toString(), "pointer");
    EXPECT_EQ(Unknown.toString(), "unknown");
  }

  // Formatting deeply nested pointer chains uses bounded call-stack depth instead of recursive descent.
  TEST(IRTypeFormatTest, FormatsDeepPointerChainsIteratively)
  {
    IRContext Context(Target64);
    const Type *Current = Context.typePool().getInt32Type();
    constexpr std::size_t Depth = 16384;
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      Current = Context.typePool().getPointerType(Current);
      ASSERT_NE(Current, nullptr);
    }
    const Value &ValueObject = *Current;
    EXPECT_EQ(ValueObject.toString(), std::string(Depth, '*') + "i32");
  }

  // Alternating callable, array, reference and pointer types retain bounded call-stack depth across every composite formatter.
  TEST(IRTypeFormatTest, FormatsDeepMixedCompositeTypesIteratively)
  {
    IRContext Context(Target64);
    auto &Types = Context.typePool();
    const Type *Current = Types.getInt32Type();
    constexpr std::size_t Depth = 4096;
    std::string Expected;
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      Current = Types.getPointerType(Current);
      Current = Types.getReferenceType(Current);
      Current = Types.getArrayType(Current, 1);
      Current = Types.getFunctionType(Current);
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
