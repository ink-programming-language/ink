#include "ink/core/name/name_pool.h"
#include "ink/ir/builder/ir_builder.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/instruction/icmp_instruction.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/builtin_type/meta_type.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  namespace
  {
    constexpr target::TargetContext Target32(target::PointerWidth::Bits32, target::ByteOrder::LittleEndian);
    constexpr target::TargetContext Target64(target::PointerWidth::Bits64, target::ByteOrder::BigEndian);

    constexpr std::optional<std::uint64_t> fixedValueSize(ValueKind Kind) noexcept
    {
      switch (Kind)
      {
#define INK_VALUE(Kind, Name, FixedSize) \
  case ValueKind::Kind:                  \
    return FixedSize;
#include "ink/ir/type/Value.def"
#undef INK_VALUE
      }
      return std::nullopt;
    }
  } // namespace

  static_assert(std::is_base_of_v<Type, MetaType>);
  static_assert(std::is_final_v<MetaType>);
  static_assert(!std::is_default_constructible_v<MetaType>);
  static_assert(!std::is_copy_constructible_v<MetaType> && !std::is_move_constructible_v<MetaType>);
  static_assert(std::is_same_v<decltype(std::declval<const TypePool &>().getMetaType()), const MetaType *>);
  static_assert(noexcept(std::declval<const TypePool &>().getMetaType()));
  static_assert(!fixedValueSize(ValueKind::MetaType).has_value());

  // Every context owns one self-typed metatype shared by its modules, with distinct identities across contexts and targets.
  TEST(IRMetaTypeTest, CanonicalIdentityBelongsToContextAndClosesTypeChain)
  {
    for (const target::TargetContext &Target : {Target32, Target64})
    {
      IRContext Context(Target);
      IRContext OtherContext(Target);
      Module First(Context);
      Module Second(Context);
      Module Other(OtherContext);
      const MetaType *Meta = Context.typePool().getMetaType();
      ASSERT_NE(Meta, nullptr);
      EXPECT_EQ(Meta, Context.typePool().getMetaType());
      EXPECT_EQ(Meta, First.typePool().getMetaType());
      EXPECT_EQ(Meta, Second.typePool().getMetaType());
      EXPECT_NE(Meta, Other.typePool().getMetaType());
      EXPECT_EQ(Meta->type(), Meta);
      EXPECT_EQ(Meta->type()->type(), Meta);
      EXPECT_TRUE(Context.typePool().owns(Meta));
      EXPECT_TRUE(Context.containsType(Meta));
      EXPECT_TRUE(First.owns(Meta));
      EXPECT_TRUE(Second.contains(Meta));
      EXPECT_FALSE(OtherContext.containsType(Meta));
      EXPECT_FALSE(Other.ownsType(Meta));
    }
  }

  // The metatype is a recognizable type value whose size and alignment placeholders never grant a runtime layout.
  TEST(IRMetaTypeTest, ClassifiesAndFormatsWithoutRuntimeLayout)
  {
    IRContext Context(Target64);
    const MetaType *Meta = Context.typePool().getMetaType();
    const Type *TypeValue = Meta;
    const Value *ValueObject = Meta;
    EXPECT_EQ(ValueObject->kind(), ValueKind::MetaType);
    EXPECT_EQ(valueKindName(ValueObject->kind()), "type");
    EXPECT_EQ(ValueObject->toString(), "type");
    EXPECT_EQ(ValueObject->type(), Meta);
    EXPECT_TRUE(Type::classof(ValueObject));
    EXPECT_TRUE(MetaType::classof(ValueObject));
    EXPECT_FALSE(MetaType::classof(Context.typePool().getInt32Type()));
    EXPECT_FALSE(MetaType::classof(nullptr));
    EXPECT_FALSE(TypeValue->hasLayout());
    EXPECT_EQ(TypeValue->size(), 0U);
    EXPECT_EQ(TypeValue->alignment(), 1U);
  }

  // Builtin, structural and nominal descriptors all expose their context's metatype while integer constants retain their integer type.
  TEST(IRMetaTypeTest, ConcreteTypeValuesShareMetatypeAcrossEveryFamily)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    auto &Types = Context.typePool();
    const Type *Descriptors[] = {
        Types.getMetaType(),
        Types.getVoidType(),
        Types.getVariadicType(),
        Types.getBoolType(),
        Types.getNullPointerType(),
        Types.getInt8Type(),
        Types.getUInt8Type(),
        Types.getInt16Type(),
        Types.getUInt16Type(),
        Types.getInt32Type(),
        Types.getUInt32Type(),
        Types.getInt64Type(),
        Types.getUInt64Type(),
        Types.getFloat32Type(),
        Types.getFloat64Type(),
        Types.getPointerType(Types.getInt32Type()),
        Types.getReferenceType(Types.getInt32Type()),
        Types.getArrayType(Types.getInt32Type(), 2),
        Types.getFunctionType(Types.getInt32Type(), {Types.getBoolType()}),
        Owner.createClassType(8, 8),
        Owner.createInterfaceType(8, 8),
        Owner.createStructType(8, 8),
    };
    for (const Type *Descriptor : Descriptors)
    {
      ASSERT_NE(Descriptor, nullptr);
      SCOPED_TRACE(Descriptor->toString());
      const Value &TypeValue = *Descriptor;
      EXPECT_EQ(Descriptor->type(), Types.getMetaType());
      EXPECT_EQ(TypeValue.type(), Types.getMetaType());
    }
    const Value *Integer = Context.constantPool().getSignedIntegerConstant(Types.getInt32Type(), 42);
    ASSERT_NE(Integer, nullptr);
    EXPECT_EQ(Integer->type(), Types.getInt32Type());
    EXPECT_EQ(Integer->type()->type(), Types.getMetaType());
    EXPECT_EQ(Integer->type()->type()->type(), Types.getMetaType());
  }

  // Runtime storage factories reject the metatype as pointee, referent, element, signature component, field and global type.
  TEST(IRMetaTypeTest, RuntimeStorageRejectsMetatypeWithoutMutatingDeclarations)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    auto &Types = Context.typePool();
    const MetaType *Meta = Types.getMetaType();
    EXPECT_EQ(Types.getPointerType(Meta), nullptr);
    EXPECT_EQ(Types.getReferenceType(Meta), nullptr);
    EXPECT_EQ(Types.getArrayType(Meta, 0), nullptr);
    EXPECT_EQ(Types.getArrayType(Meta, 1), nullptr);
    EXPECT_EQ(Types.getFunctionType(Meta), nullptr);
    EXPECT_EQ(Types.getFunctionType(Types.getVoidType(), {Meta}), nullptr);
    EXPECT_EQ(Types.getFunctionType(Types.getVoidType(), {{Meta, true}}), nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMetaTypeTest.member");
    const Field TypeField(Meta, Name);
    EXPECT_FALSE(TypeField.valid());
    auto *Struct = Owner.createStructType(8, 8);
    ASSERT_NE(Struct, nullptr);
    ASSERT_TRUE(Struct->addField(Field(Types.getInt32Type(), Name)));
    EXPECT_FALSE(Struct->addField(TypeField));
    EXPECT_FALSE(Struct->replaceField(0, TypeField));
    ASSERT_EQ(Struct->fieldCount(), 1U);
    EXPECT_EQ(Struct->field(0)->type(), Types.getInt32Type());
    auto *Global = Owner.createGlobalVariable(*Types.getInt32Type(), Name);
    ASSERT_NE(Global, nullptr);
    EXPECT_EQ(Owner.createGlobalVariable(*Meta, Name), nullptr);
    EXPECT_EQ(Owner.appendGlobalVariable(std::make_unique<GlobalVariable>(*Meta, Name)), nullptr);
    ASSERT_EQ(Owner.globalVariableCount(), 1U);
    EXPECT_EQ(Owner.globalVariable(0), Global);
  }

  // Obtaining a metatype does not make descriptors usable as runtime callees, arguments, comparisons, addresses, indices, conditions or return values.
  TEST(IRMetaTypeTest, RuntimeBuilderRejectsTypeValuesWithoutAppendingInstructions)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    auto &Types = Context.typePool();
    const auto *IntegerType = Types.getInt32Type();
    const auto *Signature = Types.getFunctionType(IntegerType, {IntegerType});
    ASSERT_NE(Signature, nullptr);
    Function Caller(*Signature);
    Function Callee(*Signature);
    auto *Entry = Caller.createBlock();
    auto *Accepted = Caller.createBlock();
    auto *Rejected = Caller.createBlock();
    const auto *Zero = Context.constantPool().getSignedIntegerConstant(IntegerType, 0);
    const auto *Null = Context.constantPool().getNullPointerConstant();
    ASSERT_NE(Zero, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    const Type *Descriptors[] = {
        Types.getMetaType(),
        IntegerType,
        Types.getBoolType(),
        Signature,
        Owner.createStructType(8, 8),
    };
    for (const Type *Descriptor : Descriptors)
    {
      ASSERT_NE(Descriptor, nullptr);
      SCOPED_TRACE(Descriptor->toString());
      EXPECT_EQ(Builder.createCall(*Signature, *Descriptor, {Argument(*Zero)}), nullptr);
      EXPECT_EQ(Builder.createCall(Callee, {Argument(*Descriptor)}), nullptr);
      EXPECT_EQ(Builder.createICmp(ICmpPredicate::Equal, *Descriptor, *Descriptor), nullptr);
      EXPECT_EQ(Builder.createNeg(*Descriptor), nullptr);
      EXPECT_EQ(Builder.createGetElementPtr(*IntegerType, *Descriptor, {}), nullptr);
      EXPECT_EQ(Builder.createGetElementPtr(*IntegerType, *Null, {Descriptor}), nullptr);
      EXPECT_EQ(Builder.createConditionalBranch(*Descriptor, *Accepted, *Rejected), nullptr);
      EXPECT_EQ(Builder.createReturn(*Descriptor), nullptr);
      EXPECT_EQ(Entry->instructionCount(), 0U);
      EXPECT_EQ(Builder.insertBlock(), Entry);
    }
    EXPECT_EQ(Builder.createGetElementPtr(*Types.getMetaType(), *Null, {}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
  }
} // namespace ink::ir::test
