#include "ink/semantic/model/context.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::semantic::test
{
  static_assert(std::is_base_of_v<Value, Function>);
  static_assert(!std::is_base_of_v<Decl, Function>);
  static_assert(!std::is_copy_constructible_v<Function>);
  static_assert(!std::is_move_constructible_v<Function>);

  // Same-named functions share a structural signature while keeping independent callable identities and bindings.
  TEST(SemanticFunctionTest, ClosedFunctionsHaveIndependentValueIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getBoolType());
    ASSERT_NE(Signature, nullptr);
    const Name NameValue = Context.namePool().intern("Ready");
    const Function *First = Context.createFunction(NameValue, *Signature);
    const Function *Second = Context.createFunction(NameValue, *Signature);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_EQ(First->name(), Second->name());
    EXPECT_EQ(&First->type(), Signature);
    EXPECT_TRUE(Function::classof(First));
    EXPECT_FALSE(Function::classof(Signature));
    EXPECT_FALSE(Function::classof(nullptr));
    EXPECT_FALSE(Instruction::classof(First));
    EXPECT_FALSE(Constant::classof(First));
    Variable *Binding = Context.createVariable(Context.namePool().intern("Callback"), BindingMutability::Immutable, First);
    ASSERT_NE(Binding, nullptr);
    EXPECT_TRUE(Binding->setType(*Signature));
    const CallInstruction *Call = Context.createCallInstruction(*Binding->initializer());
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->directCallee(), First);
    EXPECT_EQ(&Call->type(), &Context.getBoolType());
  }

  // Closed functions require a valid local name and signature and cannot retain foreign type dependencies.
  TEST(SemanticFunctionTest, InvalidNamesAndForeignSignaturesAreRejected)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType());
    const FunctionType *Foreign = Other.getFunctionType(Other.getVoidType());
    ASSERT_NE(Signature, nullptr);
    ASSERT_NE(Foreign, nullptr);
    EXPECT_EQ(Context.createFunction(Name{}, *Signature), nullptr);
    EXPECT_EQ(Context.createFunction(Other.namePool().intern("Foreign"), *Signature), nullptr);
    const Name NameValue = Context.namePool().intern("Local");
    EXPECT_EQ(Context.createFunction(NameValue, *Foreign), nullptr);
    EXPECT_NE(Context.createFunction(NameValue, *Signature), nullptr);
  }
} // namespace ink::semantic::test
