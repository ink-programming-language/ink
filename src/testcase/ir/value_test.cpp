#include "ink/ir/ir_builder.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/context.h"
#include "ink/ir/type/class_type.h"
#include "ink/ir/type/enum_type.h"
#include "ink/ir/type/interface_type.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::ir::test
{
  namespace
  {
    class UnknownValue final : public Value
    {
      public:
        explicit UnknownValue(const Type &ValueType) noexcept
            : Value(ValueType.context(), static_cast<ValueKind>(255), ValueType)
        {
        }
    };

    template <typename Concrete>
    void expectValueKind(const IRContext &Context, const Concrete *Object, ValueKind ExpectedKind)
    {
      ASSERT_NE(Object, nullptr);
      SCOPED_TRACE(static_cast<unsigned>(ExpectedKind));
      const Value *ValueObject = Object;
      EXPECT_EQ(&ValueObject->context(), &Context);
      EXPECT_EQ(&ValueObject->type().context(), &Context);
      EXPECT_EQ(ValueObject->kind(), ExpectedKind);
      EXPECT_EQ(Type::classof(ValueObject), (std::is_base_of_v<Type, Concrete>));
      EXPECT_EQ(UserDefinedType::classof(ValueObject), (std::is_base_of_v<UserDefinedType, Concrete>));
      EXPECT_EQ(Constant::classof(ValueObject), (std::is_base_of_v<Constant, Concrete>));
#define INK_IR_VALUE(Name) EXPECT_EQ(Name::classof(ValueObject), (std::is_base_of_v<Name, Concrete>));
#include "ink/ir/Values.def"
    }

    template <typename Concrete>
    void expectValueKind(const IRContext &Context, const std::unique_ptr<Concrete> &Object, ValueKind ExpectedKind)
    {
      expectValueKind(Context, Object.get(), ExpectedKind);
    }
  } // namespace

  // Each factory preserves its context, and value tags and classof checks agree with the C++ inheritance hierarchy.
  TEST(IRValueTest, KindsAndClassificationMatchConcreteClasses)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    IRBuilder Factory(Context);
    expectValueKind(Context, &Context.typePool().getType<TypeKind::Meta>(), ValueKind::BuiltinType);
    expectValueKind(Context, &Context.typePool().getType<TypeKind::Void>(), ValueKind::BuiltinType);
    expectValueKind(Context, &Context.typePool().getType<TypeKind::Bool>(), ValueKind::BuiltinType);
    expectValueKind(Context, &Context.typePool().getType<TypeKind::Label>(), ValueKind::BuiltinType);
    expectValueKind(Context, &Context.typePool().getType<TypeKind::Module>(), ValueKind::BuiltinType);
    expectValueKind(Context, Factory.createBasicBlock(), ValueKind::BasicBlock);
    const IntegerType *Int32 = Context.typePool().getType<TypeKind::Integer>(32, true);
    ASSERT_NE(Int32, nullptr);
    expectValueKind(Context, Int32, ValueKind::IntegerType);
    expectValueKind(Context, Context.typePool().getType<TypeKind::Float>(32), ValueKind::FloatType);
    expectValueKind(Context, Context.typePool().getType<TypeKind::Array>(*Int32, 4), ValueKind::ArrayType);
    expectValueKind(Context, Context.typePool().getType<TypeKind::Slice>(*Int32, AccessKind::ReadOnly), ValueKind::SliceType);
    expectValueKind(Context, Context.typePool().getType<TypeKind::Pointer>(*Int32, AccessKind::ReadWrite), ValueKind::PointerType);
    expectValueKind(Context, Context.typePool().getType<TypeKind::Reference>(*Int32, AccessKind::ReadOnly), ValueKind::ReferenceType);

    const Name NameValue = Context.namePool().intern("Named");
    expectValueKind(Context, Factory.createModule(NameValue), ValueKind::Module);
    const ClassType *Class = Factory.createClassType(NameValue);
    const EnumType *Enum = Factory.createEnumType(NameValue);
    const InterfaceType *Interface = Factory.createInterfaceType(NameValue);
    ASSERT_NE(Class, nullptr);
    ASSERT_NE(Enum, nullptr);
    ASSERT_NE(Interface, nullptr);
    expectValueKind(Context, Class, ValueKind::ClassType);
    expectValueKind(Context, Enum, ValueKind::EnumType);
    expectValueKind(Context, Interface, ValueKind::InterfaceType);

    expectValueKind(Context, Context.constantPool().getIntegerConstant(*Int32, IntegerBits(32, 42)), ValueKind::IntegerConstant);
    expectValueKind(Context, &Context.constantPool().getBoolConstant(false), ValueKind::BoolConstant);
    expectValueKind(Context, &Context.constantPool().getBoolConstant(true), ValueKind::BoolConstant);
    const IntegerType *Byte = Context.typePool().getType<TypeKind::Integer>(8, false);
    ASSERT_NE(Byte, nullptr);
    const SliceType *String = Context.typePool().getType<TypeKind::Slice>(*Byte, AccessKind::ReadOnly);
    const FloatType *Float32 = Context.typePool().getType<TypeKind::Float>(32);
    ASSERT_NE(String, nullptr);
    ASSERT_NE(Float32, nullptr);
    expectValueKind(Context, Context.constantPool().getStringConstant(*String, "hello,world"), ValueKind::StringConstant);
    expectValueKind(Context, Context.constantPool().getFloatConstant(*Float32, FloatBits(32, 0x3f800000)), ValueKind::FloatConstant);
    const FunctionType *Signature = Context.typePool().getType<TypeKind::Function>(*Int32);
    ASSERT_NE(Signature, nullptr);
    expectValueKind(Context, Signature, ValueKind::FunctionType);
    auto TargetOwner = Factory.createFunction(NameValue, *Signature);
    const Function *Target = TargetOwner.get();
    ASSERT_NE(Target, nullptr);
    expectValueKind(Context, Target, ValueKind::Function);
    expectValueKind(Context, Factory.createDetachedCallInstruction(*Target), ValueKind::CallInstruction);
    auto SlotOwner = Factory.createDetachedAllocaInstruction(*Int32);
    AllocaInstruction *Slot = SlotOwner.get();
    ASSERT_NE(Slot, nullptr);
    expectValueKind(Context, Slot, ValueKind::AllocaInstruction);
    auto LoadedOwner = Factory.createDetachedLoadInstruction(*Slot);
    const LoadInstruction *Loaded = LoadedOwner.get();
    expectValueKind(Context, Loaded, ValueKind::LoadInstruction);
    ASSERT_NE(Loaded, nullptr);
    expectValueKind(Context, Factory.createDetachedStoreInstruction(*Slot, *Loaded), ValueKind::StoreInstruction);
    expectValueKind(Context, Factory.createDetachedAddInstruction(*Loaded, *Loaded), ValueKind::AddInstruction);
    expectValueKind(Context, Factory.createDetachedReturnInstruction(Loaded), ValueKind::ReturnInstruction);
    const Type *ParameterTypes[] = {Int32};
    const FunctionType *ParameterizedSignature = Context.typePool().getType<TypeKind::Function>(*Int32, ParameterTypes);
    ASSERT_NE(ParameterizedSignature, nullptr);
    auto ParameterizedOwner = Factory.createFunction(NameValue, *ParameterizedSignature);
    const Function *Parameterized = ParameterizedOwner.get();
    ASSERT_NE(Parameterized, nullptr);
    EXPECT_EQ(&Parameterized->functionType(), ParameterizedSignature);
    expectValueKind(Context, Parameterized->parameters()[0].get(), ValueKind::FunctionParameter);
  }

  // Null pointers and unregistered value tags never match a model class.
  TEST(IRValueTest, NullAndUnknownKindsAreRejected)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    const UnknownValue Unknown(Context.typePool().getType<TypeKind::Bool>());
    const Value *ValueObject = &Unknown;
    EXPECT_FALSE(Type::classof(nullptr));
    EXPECT_FALSE(UserDefinedType::classof(nullptr));
    EXPECT_FALSE(Constant::classof(nullptr));
    EXPECT_FALSE(Type::classof(ValueObject));
    EXPECT_FALSE(UserDefinedType::classof(ValueObject));
    EXPECT_FALSE(Constant::classof(ValueObject));
#define INK_IR_VALUE(Name)              \
  EXPECT_FALSE(Name::classof(nullptr)); \
  EXPECT_FALSE(Name::classof(ValueObject));
#include "ink/ir/Values.def"
  }
} // namespace ink::ir::test
