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
  static_assert(std::is_same_v<decltype(std::declval<const TypePool &>().get<ink::ir::MetaType>()), const MetaType *>);
  static_assert(noexcept(std::declval<const TypePool &>().get<ink::ir::MetaType>()));
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
      const MetaType *Meta = Context.typePool().get<ink::ir::MetaType>();
      ASSERT_NE(Meta, nullptr);
      EXPECT_EQ(Meta, Context.typePool().get<ink::ir::MetaType>());
      EXPECT_EQ(Meta, First.typePool().get<ink::ir::MetaType>());
      EXPECT_EQ(Meta, Second.typePool().get<ink::ir::MetaType>());
      EXPECT_NE(Meta, Other.typePool().get<ink::ir::MetaType>());
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
    const MetaType *Meta = Context.typePool().get<ink::ir::MetaType>();
    const Type *TypeValue = Meta;
    const Value *ValueObject = Meta;
    EXPECT_EQ(ValueObject->kind(), ValueKind::MetaType);
    EXPECT_EQ(valueKindName(ValueObject->kind()), "type");
    EXPECT_EQ(ValueObject->toString(), "type");
    EXPECT_EQ(ValueObject->type(), Meta);
    EXPECT_TRUE(Type::classof(ValueObject));
    EXPECT_TRUE(MetaType::classof(ValueObject));
    EXPECT_FALSE(MetaType::classof(Context.typePool().get<ink::ir::IntegerType>(32, true)));
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
        Types.get<ink::ir::MetaType>(),
        Types.get<ink::ir::VoidType>(),
        Types.get<ink::ir::VariadicType>(),
        Types.get<ink::ir::BoolType>(),
        Types.get<ink::ir::NullPointerType>(),
        Types.get<ink::ir::IntegerType>(8, true),
        Types.get<ink::ir::IntegerType>(8, false),
        Types.get<ink::ir::IntegerType>(16, true),
        Types.get<ink::ir::IntegerType>(16, false),
        Types.get<ink::ir::IntegerType>(32, true),
        Types.get<ink::ir::IntegerType>(32, false),
        Types.get<ink::ir::IntegerType>(64, true),
        Types.get<ink::ir::IntegerType>(64, false),
        Types.get<ink::ir::FloatType>(32),
        Types.get<ink::ir::FloatType>(64),
        Types.get<ink::ir::PointerType>(Types.get<ink::ir::IntegerType>(32, true)),
        Types.get<ink::ir::ReferenceType>(Types.get<ink::ir::IntegerType>(32, true)),
        Types.get<ink::ir::ArrayType>(Types.get<ink::ir::IntegerType>(32, true), 2),
        Types.get<ink::ir::FunctionType>(Types.get<ink::ir::IntegerType>(32, true), {Types.get<ink::ir::BoolType>()}),
        Owner.createClassType(8, 8),
        Owner.createInterfaceType(8, 8),
        Owner.createStructType(8, 8),
    };
    for (const Type *Descriptor : Descriptors)
    {
      ASSERT_NE(Descriptor, nullptr);
      SCOPED_TRACE(Descriptor->toString());
      const Value &TypeValue = *Descriptor;
      EXPECT_EQ(Descriptor->type(), Types.get<ink::ir::MetaType>());
      EXPECT_EQ(TypeValue.type(), Types.get<ink::ir::MetaType>());
    }
    const Value *Integer = Context.constantPool().get<ink::ir::IntegerConstant>(Types.get<ink::ir::IntegerType>(32, true), static_cast<std::int64_t>(42));
    ASSERT_NE(Integer, nullptr);
    EXPECT_EQ(Integer->type(), Types.get<ink::ir::IntegerType>(32, true));
    EXPECT_EQ(Integer->type()->type(), Types.get<ink::ir::MetaType>());
    EXPECT_EQ(Integer->type()->type()->type(), Types.get<ink::ir::MetaType>());
  }

  // Runtime storage factories reject the metatype as pointee, referent, element, signature component, field and global type.
  TEST(IRMetaTypeTest, RuntimeStorageRejectsMetatypeWithoutMutatingDeclarations)
  {
    IRContext Context(Target64);
    Module Owner(Context);
    auto &Types = Context.typePool();
    const MetaType *Meta = Types.get<ink::ir::MetaType>();
    EXPECT_EQ(Types.get<ink::ir::PointerType>(Meta), nullptr);
    EXPECT_EQ(Types.get<ink::ir::ReferenceType>(Meta), nullptr);
    EXPECT_EQ(Types.get<ink::ir::ArrayType>(Meta, 0), nullptr);
    EXPECT_EQ(Types.get<ink::ir::ArrayType>(Meta, 1), nullptr);
    EXPECT_EQ(Types.get<ink::ir::FunctionType>(Meta), nullptr);
    EXPECT_EQ(Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {Meta}), nullptr);
    EXPECT_EQ(Types.get<ink::ir::FunctionType>(Types.get<ink::ir::VoidType>(), {{Meta, true}}), nullptr);
    const core::Name Name = core::NamePool::instance().intern("IRMetaTypeTest.member");
    const Field TypeField(Meta, Name);
    EXPECT_FALSE(TypeField.valid());
    auto *Struct = Owner.createStructType(8, 8);
    ASSERT_NE(Struct, nullptr);
    ASSERT_TRUE(Struct->addField(Field(Types.get<ink::ir::IntegerType>(32, true), Name)));
    EXPECT_FALSE(Struct->addField(TypeField));
    EXPECT_FALSE(Struct->replaceField(0, TypeField));
    ASSERT_EQ(Struct->fieldCount(), 1U);
    EXPECT_EQ(Struct->field(0)->type(), Types.get<ink::ir::IntegerType>(32, true));
    auto *Global = Owner.createGlobalVariable(*Types.get<ink::ir::IntegerType>(32, true), Name);
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
    const auto *IntegerType = Types.get<ink::ir::IntegerType>(32, true);
    const auto *Signature = Types.get<ink::ir::FunctionType>(IntegerType, {IntegerType});
    ASSERT_NE(Signature, nullptr);
    Function Caller(*Signature);
    Function Callee(*Signature);
    auto *Entry = Caller.createBlock();
    auto *Accepted = Caller.createBlock();
    auto *Rejected = Caller.createBlock();
    const auto *Zero = Context.constantPool().get<ink::ir::IntegerConstant>(IntegerType, static_cast<std::int64_t>(0));
    const auto *Null = Context.constantPool().get<ink::ir::NullPointerConstant>();
    ASSERT_NE(Zero, nullptr);
    IRBuilder Builder(Context);
    ASSERT_TRUE(Builder.setInsertPoint(*Entry));
    const Type *Descriptors[] = {
        Types.get<ink::ir::MetaType>(),
        IntegerType,
        Types.get<ink::ir::BoolType>(),
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
    EXPECT_EQ(Builder.createGetElementPtr(*Types.get<ink::ir::MetaType>(), *Null, {}), nullptr);
    EXPECT_EQ(Entry->instructionCount(), 0U);
  }
} // namespace ink::ir::test
