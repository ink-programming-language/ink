#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/phi_instruction.h"
#include "ink/ir/module/global_variable.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/value/value.h"
#include "ink/ir/value/value_kind.h"

#include <gtest/gtest.h>

#include <array>
#include <type_traits>
#include <utility>

namespace ink::ir::test
{
  static_assert(!std::is_polymorphic_v<Value>);
  static_assert(std::is_polymorphic_v<Type>);

  namespace
  {
    class LegacyValue final : public Value
    {
      public:
        constexpr explicit LegacyValue(ValueRef ValueType) noexcept
            : Value(ValueType)
        {
        }
    };

    constexpr IntegerType CompileTimeIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    constexpr FloatType CompileTimeFloatType(FloatType::Width::Bits64);
    constexpr IntegerConstant CompileTimeInteger(CompileTimeIntegerType, 42);
    constexpr FloatConstant CompileTimeFloat(CompileTimeFloatType, 0);
    constexpr AddInstruction CompileTimeAdd(CompileTimeInteger, CompileTimeInteger);
    constexpr LegacyValue CompileTimeLegacy(CompileTimeIntegerType);
    constexpr LegacyValue CompileTimeTypeValue(MetaType::instance());
    constexpr const Value *CompileTimeErasedType = &CompileTimeIntegerType;
    static_assert(MetaType::instance().valueKind() == ValueKind::Type);
    static_assert(CompileTimeIntegerType.valueKind() == ValueKind::Type);
    static_assert(CompileTimeFloatType.valueKind() == ValueKind::Type);
    static_assert(CompileTimeInteger.valueKind() == ValueKind::IntegerConstant);
    static_assert(CompileTimeFloat.valueKind() == ValueKind::FloatConstant);
    static_assert(CompileTimeAdd.valueKind() == ValueKind::Instruction);
    static_assert(CompileTimeLegacy.valueKind() == ValueKind::Unknown);
    static_assert(CompileTimeTypeValue.valueKind() == ValueKind::Unknown);
    static_assert(Type::classof(CompileTimeErasedType));
    static_assert(static_cast<const Type *>(CompileTimeErasedType) == &CompileTimeIntegerType);
    static_assert(IntegerType::classof(CompileTimeErasedType));
    static_assert(MetaType::classof(&MetaType::instance()));
    static_assert(!Type::classof(nullptr) && !MetaType::classof(nullptr) && !IntegerType::classof(nullptr));
    static_assert(!Type::classof(&CompileTimeInteger) && !Type::classof(&CompileTimeFloat) && !Type::classof(&CompileTimeAdd));
    static_assert(!Type::classof(&CompileTimeLegacy) && !Type::classof(&CompileTimeTypeValue));
    static_assert(!MetaType::classof(&CompileTimeTypeValue));
  } // namespace

  // Base references distinguish all built-in value categories, including a type-valued argument from an actual Type object.
  TEST(IRValueKindTest, ClassifiesBuiltinValuesWithoutChangingTheirTypeEdges)
  {
    TypePool Types;
    ConstantPool Constants(Types);
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant Integer(Int32, 42);
    const FloatConstant &Float = Constants.getFloat(2.0f);
    const StringConstant &String = Constants.getString("value");
    const FunctionType &Signature = Types.getFunctionType(Int32, std::array{Parameter(Int32), Parameter(Types.getMetaType())});
    Function FunctionValue(Signature);
    const Argument &IntegerArgument = FunctionValue.argument(0);
    const Argument &TypeArgument = FunctionValue.argument(1);
    const AddInstruction Add(IntegerArgument, Integer);
    const PhiInstruction Phi(Int32);
    const PhiInstruction TypePhi(Types.getMetaType());
    const GlobalVariable Global(Types.getPointerType(Int32));
    const std::pair<const Value *, ValueKind> Expected[] = {
        {&Int32, ValueKind::Type},
        {&IntegerArgument, ValueKind::Argument},
        {&TypeArgument, ValueKind::Argument},
        {&Add, ValueKind::Instruction},
        {&Phi, ValueKind::Instruction},
        {&TypePhi, ValueKind::Instruction},
        {&Integer, ValueKind::IntegerConstant},
        {&Float, ValueKind::FloatConstant},
        {&String, ValueKind::StringConstant},
        {&FunctionValue, ValueKind::Function},
        {&Global, ValueKind::GlobalVariable},
    };
    for (const auto &[Node, Kind] : Expected)
    {
      EXPECT_EQ(Node->valueKind(), Kind);
      EXPECT_EQ(Type::classof(Node), Kind == ValueKind::Type);
    }
    EXPECT_EQ(&IntegerArgument.getType(), &Int32);
    EXPECT_EQ(&TypeArgument.getType(), &Types.getMetaType());
    EXPECT_TRUE(TypeArgument.isTypeValue());
    EXPECT_EQ(TypeArgument.valueKind(), ValueKind::Argument);
    EXPECT_FALSE(Type::classof(&TypeArgument));
    EXPECT_FALSE(MetaType::classof(&TypeArgument));
    EXPECT_TRUE(TypePhi.isTypeValue());
    EXPECT_FALSE(Type::classof(&TypePhi));
    const Value *ErasedInteger = &Int32;
    ASSERT_TRUE(Type::classof(ErasedInteger));
    EXPECT_EQ(static_cast<const Type *>(ErasedInteger), &Int32);
    EXPECT_EQ(&FunctionValue.getType(), &Signature);
    EXPECT_EQ(&Global.getType(), &Types.getPointerType(Int32));
  }

  // All concrete types, including the self-typed metatype and mutable records, share ValueKind::Type while retaining their own TypeKind.
  TEST(IRValueKindTest, ClassifiesEveryConcreteTypeAsATypeValue)
  {
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const Type *Nodes[] = {
        &Types.getMetaType(),
        &Int32,
        &Types.getFloatType(FloatType::Width::Bits32),
        &Types.getBoolType(),
        &Types.getVoidType(),
        &Types.getArrayType(Int32, 4),
        &Types.createStructType(),
        &Types.createObjectType(),
        &Types.getPointerType(Int32),
        &Types.getFunctionType(Int32),
    };
    for (const Type *Node : Nodes)
    {
      const Value *Erased = Node;
      EXPECT_EQ(Erased->valueKind(), ValueKind::Type);
      ASSERT_TRUE(Type::classof(Erased));
      EXPECT_EQ(static_cast<const Type *>(Erased), Node);
      EXPECT_EQ(&Node->getType(), &Types.getMetaType());
    }
  }

  // Existing subclasses that use the protected one-argument Value constructor keep an explicit Unknown category and borrowed type edge.
  TEST(IRValueKindTest, PreservesUnknownClassificationForLegacyValueSubclasses)
  {
    TypePool Types;
    const LegacyValue Ordinary(Types.getBoolType());
    const LegacyValue TypeValue(Types.getMetaType());
    EXPECT_EQ(Ordinary.valueKind(), ValueKind::Unknown);
    EXPECT_EQ(TypeValue.valueKind(), ValueKind::Unknown);
    EXPECT_EQ(&Ordinary.getType(), &Types.getBoolType());
    EXPECT_EQ(&TypeValue.getType(), &Types.getMetaType());
    EXPECT_FALSE(Ordinary.isTypeValue());
    EXPECT_TRUE(TypeValue.isTypeValue());
    EXPECT_FALSE(Type::classof(&Ordinary));
    EXPECT_FALSE(Type::classof(&TypeValue));
    EXPECT_FALSE(MetaType::classof(&TypeValue));
  }
} // namespace ink::ir::test
