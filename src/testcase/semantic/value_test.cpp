#include "ink/semantic/model/context.h"
#include "ink/semantic/model/instruction/instruction.h"
#include "ink/semantic/model/type/class_type.h"
#include "ink/semantic/model/type/enum_type.h"
#include "ink/semantic/model/type/interface_type.h"
#include "ink/parser/ast.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::semantic::test
{
  namespace
  {
    class UnknownValue final : public Value
    {
      public:
        explicit UnknownValue(const Type &ValueType) noexcept
            : Value(static_cast<ValueKind>(255)),
              ValueType(ValueType)
        {
        }

        const Type &type() const noexcept override
        {
          return ValueType;
        }

      private:
        const Type &ValueType;
    };

    template <typename Concrete>
    void expectValueKind(const Concrete *Object, ValueKind ExpectedKind)
    {
      ASSERT_NE(Object, nullptr);
      SCOPED_TRACE(static_cast<unsigned>(ExpectedKind));
      const Value *ValueObject = Object;
      EXPECT_EQ(ValueObject->kind(), ExpectedKind);
      EXPECT_EQ(Type::classof(ValueObject), (std::is_base_of_v<Type, Concrete>));
      EXPECT_EQ(UserDefinedType::classof(ValueObject), (std::is_base_of_v<UserDefinedType, Concrete>));
      EXPECT_EQ(Constant::classof(ValueObject), (std::is_base_of_v<Constant, Concrete>));
      EXPECT_EQ(Instruction::classof(ValueObject), (std::is_base_of_v<Instruction, Concrete>));
#define INK_SEMANTIC_VALUE(Name) EXPECT_EQ(Name::classof(ValueObject), (std::is_base_of_v<Name, Concrete>));
#include "ink/semantic/model/Values.def"
    }
  } // namespace

  // Each factory's value tag and every classof check agree with the actual C++ inheritance hierarchy.
  TEST(SemanticValueTest, KindsAndClassificationMatchConcreteClasses)
  {
    core::CompilationContext Compilation;
    parser::NameExpr Expression({}, {});
    SemanticContext Context(Compilation);
    expectValueKind(&Context.getMetaType(), ValueKind::BuiltinType);
    expectValueKind(&Context.getVoidType(), ValueKind::BuiltinType);
    expectValueKind(&Context.getBoolType(), ValueKind::BuiltinType);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    ASSERT_NE(Int32, nullptr);
    expectValueKind(Int32, ValueKind::IntegerType);
    expectValueKind(Context.getFloatType(32), ValueKind::FloatType);
    expectValueKind(Context.getArrayType(*Int32, 4), ValueKind::ArrayType);
    expectValueKind(Context.getSliceType(*Int32, AccessKind::ReadOnly), ValueKind::SliceType);
    expectValueKind(Context.getPointerType(*Int32, AccessKind::ReadWrite), ValueKind::PointerType);
    expectValueKind(Context.getReferenceType(*Int32, AccessKind::ReadOnly), ValueKind::ReferenceType);

    const Name NameValue = Context.namePool().intern("Named");
    const ClassType *Class = Context.createClassType(NameValue);
    const EnumType *Enum = Context.createEnumType(NameValue);
    const InterfaceType *Interface = Context.createInterfaceType(NameValue);
    ASSERT_NE(Class, nullptr);
    ASSERT_NE(Enum, nullptr);
    ASSERT_NE(Interface, nullptr);
    expectValueKind(Class, ValueKind::ClassType);
    expectValueKind(Enum, ValueKind::EnumType);
    expectValueKind(Interface, ValueKind::InterfaceType);

    expectValueKind(Context.getIntegerConstant(*Int32, IntegerBits(32, 42)), ValueKind::IntegerConstant);
    expectValueKind(&Context.getBoolConstant(false), ValueKind::BoolConstant);
    expectValueKind(&Context.getBoolConstant(true), ValueKind::BoolConstant);
    const IntegerType *Byte = Context.getIntegerType(8, false);
    ASSERT_NE(Byte, nullptr);
    const SliceType *String = Context.getSliceType(*Byte, AccessKind::ReadOnly);
    const FloatType *Float32 = Context.getFloatType(32);
    ASSERT_NE(String, nullptr);
    ASSERT_NE(Float32, nullptr);
    expectValueKind(Context.getStringConst(*String, "hello,world"), ValueKind::StringConst);
    expectValueKind(Context.getFloatConst(*Float32, FloatBits(32, 0x3f800000)), ValueKind::FloatConst);
    expectValueKind(Context.createExprValue(*Int32, Expression), ValueKind::ExprValue);
    const FunctionType *Signature = Context.getFunctionType(*Int32);
    ASSERT_NE(Signature, nullptr);
    expectValueKind(Signature, ValueKind::FunctionType);
    const Function *Target = Context.createFunction(NameValue, *Signature);
    ASSERT_NE(Target, nullptr);
    expectValueKind(Target, ValueKind::Function);
    expectValueKind(Context.createCallInstruction(*Target), ValueKind::CallInstruction);
  }

  // Null pointers and unregistered value tags never match a model class.
  TEST(SemanticValueTest, NullAndUnknownKindsAreRejected)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const UnknownValue Unknown(Context.getBoolType());
    const Value *ValueObject = &Unknown;
    EXPECT_FALSE(Type::classof(nullptr));
    EXPECT_FALSE(UserDefinedType::classof(nullptr));
    EXPECT_FALSE(Constant::classof(nullptr));
    EXPECT_FALSE(Instruction::classof(nullptr));
    EXPECT_FALSE(Type::classof(ValueObject));
    EXPECT_FALSE(UserDefinedType::classof(ValueObject));
    EXPECT_FALSE(Constant::classof(ValueObject));
    EXPECT_FALSE(Instruction::classof(ValueObject));
#define INK_SEMANTIC_VALUE(Name)        \
  EXPECT_FALSE(Name::classof(nullptr)); \
  EXPECT_FALSE(Name::classof(ValueObject));
#include "ink/semantic/model/Values.def"
  }
} // namespace ink::semantic::test
