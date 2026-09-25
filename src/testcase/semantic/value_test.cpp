#include "ink/semantic/model/function/basic_block.h"
#include "ink/semantic/model/context.h"
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
            : Value(ValueType.context(), static_cast<ValueKind>(255)),
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
    void expectValueKind(const SemanticContext &Context, const Concrete *Object, ValueKind ExpectedKind)
    {
      ASSERT_NE(Object, nullptr);
      SCOPED_TRACE(static_cast<unsigned>(ExpectedKind));
      const Value *ValueObject = Object;
      EXPECT_EQ(&ValueObject->context(), &Context);
      EXPECT_EQ(ValueObject->kind(), ExpectedKind);
      EXPECT_EQ(Type::classof(ValueObject), (std::is_base_of_v<Type, Concrete>));
      EXPECT_EQ(UserDefinedType::classof(ValueObject), (std::is_base_of_v<UserDefinedType, Concrete>));
      EXPECT_EQ(Constant::classof(ValueObject), (std::is_base_of_v<Constant, Concrete>));
#define INK_SEMANTIC_VALUE(Name) EXPECT_EQ(Name::classof(ValueObject), (std::is_base_of_v<Name, Concrete>));
#include "ink/semantic/model/Values.def"
    }
  } // namespace

  // Each factory preserves its context, and value tags and classof checks agree with the C++ inheritance hierarchy.
  TEST(SemanticValueTest, KindsAndClassificationMatchConcreteClasses)
  {
    core::CompilationContext Compilation;
    parser::NameExpr Expression({}, {});
    SemanticContext Context(Compilation);
    expectValueKind(Context, &Context.getMetaType(), ValueKind::BuiltinType);
    expectValueKind(Context, &Context.getVoidType(), ValueKind::BuiltinType);
    expectValueKind(Context, &Context.getBoolType(), ValueKind::BuiltinType);
    expectValueKind(Context, &Context.getLabelType(), ValueKind::BuiltinType);
    expectValueKind(Context, &Context.getModuleType(), ValueKind::BuiltinType);
    expectValueKind(Context, Context.createBasicBlock(), ValueKind::BasicBlock);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    ASSERT_NE(Int32, nullptr);
    expectValueKind(Context, Int32, ValueKind::IntegerType);
    expectValueKind(Context, Context.getFloatType(32), ValueKind::FloatType);
    expectValueKind(Context, Context.getArrayType(*Int32, 4), ValueKind::ArrayType);
    expectValueKind(Context, Context.getSliceType(*Int32, AccessKind::ReadOnly), ValueKind::SliceType);
    expectValueKind(Context, Context.getPointerType(*Int32, AccessKind::ReadWrite), ValueKind::PointerType);
    expectValueKind(Context, Context.getReferenceType(*Int32, AccessKind::ReadOnly), ValueKind::ReferenceType);

    const Name NameValue = Context.namePool().intern("Named");
    expectValueKind(Context, Context.createModule(NameValue), ValueKind::Module);
    const ClassType *Class = Context.createClassType(NameValue);
    const EnumType *Enum = Context.createEnumType(NameValue);
    const InterfaceType *Interface = Context.createInterfaceType(NameValue);
    ASSERT_NE(Class, nullptr);
    ASSERT_NE(Enum, nullptr);
    ASSERT_NE(Interface, nullptr);
    expectValueKind(Context, Class, ValueKind::ClassType);
    expectValueKind(Context, Enum, ValueKind::EnumType);
    expectValueKind(Context, Interface, ValueKind::InterfaceType);

    expectValueKind(Context, Context.getIntegerConstant(*Int32, IntegerBits(32, 42)), ValueKind::IntegerConstant);
    expectValueKind(Context, &Context.getBoolConstant(false), ValueKind::BoolConstant);
    expectValueKind(Context, &Context.getBoolConstant(true), ValueKind::BoolConstant);
    const IntegerType *Byte = Context.getIntegerType(8, false);
    ASSERT_NE(Byte, nullptr);
    const SliceType *String = Context.getSliceType(*Byte, AccessKind::ReadOnly);
    const FloatType *Float32 = Context.getFloatType(32);
    ASSERT_NE(String, nullptr);
    ASSERT_NE(Float32, nullptr);
    expectValueKind(Context, Context.getStringConstant(*String, "hello,world"), ValueKind::StringConstant);
    expectValueKind(Context, Context.getFloatConstant(*Float32, FloatBits(32, 0x3f800000)), ValueKind::FloatConstant);
    expectValueKind(Context, Context.createExprValue(*Int32, Expression), ValueKind::ExprValue);
    const FunctionType *Signature = Context.getFunctionType(*Int32);
    ASSERT_NE(Signature, nullptr);
    expectValueKind(Context, Signature, ValueKind::FunctionType);
    const Function *Target = Context.createFunction(NameValue, *Signature);
    ASSERT_NE(Target, nullptr);
    expectValueKind(Context, Target, ValueKind::Function);
    expectValueKind(Context, Context.createCallInstruction(*Target), ValueKind::CallInstruction);
    AllocaInstruction *Slot = Context.createAllocaInstruction(*Int32);
    ASSERT_NE(Slot, nullptr);
    expectValueKind(Context, Slot, ValueKind::AllocaInstruction);
    const LoadInstruction *Loaded = Context.createLoadInstruction(*Slot);
    expectValueKind(Context, Loaded, ValueKind::LoadInstruction);
    ASSERT_NE(Loaded, nullptr);
    expectValueKind(Context, Context.createStoreInstruction(*Slot, *Loaded), ValueKind::StoreInstruction);
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
    EXPECT_FALSE(Type::classof(ValueObject));
    EXPECT_FALSE(UserDefinedType::classof(ValueObject));
    EXPECT_FALSE(Constant::classof(ValueObject));
#define INK_SEMANTIC_VALUE(Name)        \
  EXPECT_FALSE(Name::classof(nullptr)); \
  EXPECT_FALSE(Name::classof(ValueObject));
#include "ink/semantic/model/Values.def"
  }
} // namespace ink::semantic::test
