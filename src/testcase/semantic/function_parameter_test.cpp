#include "ink/semantic/model/function/function_parameter.h"

#include "ink/semantic/context.h"

#include <gtest/gtest.h>

#include <string>

namespace ink::semantic::test
{
  // Parameters keep copied names and kinds after caller changes, with outer as the only function identity.
  TEST(SemanticFunctionParameterTest, PreservesKindsAndUsesOuterAsFunctionIdentity)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type *Types[] = {
        Context.getIntegerType(32, true),
        &Context.getBoolType(),
        Context.getSliceType(*Context.getIntegerType(32, true), AccessKind::ReadOnly),
    };
    ParameterKind Kinds[] = {
        ParameterKind::Positional,
        ParameterKind::Named,
        ParameterKind::Variadic,
    };
    Name Names[] = {
        Context.namePool().intern("value"),
        Context.namePool().intern("flag"),
        Context.namePool().intern("rest"),
    };
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType(), Types);
    ASSERT_NE(Signature, nullptr);
    Function *Target = Context.createFunction(Context.namePool().intern("f"), *Signature, Kinds, Names);
    ASSERT_NE(Target, nullptr);
    ASSERT_EQ(Target->parameters().size(), 3U);
    for (std::size_t Index = 0; Index < Target->parameters().size(); ++Index)
    {
      const FunctionParameter *Parameter = Target->parameters()[Index];
      EXPECT_EQ(Parameter->outer(), Target);
      EXPECT_EQ(&Parameter->function(), Parameter->outer());
      EXPECT_EQ(Parameter->index(), Index);
      EXPECT_EQ(&Parameter->type(), Types[Index]);
      EXPECT_EQ(Parameter->parameterKind(), Kinds[Index]);
      EXPECT_EQ(Parameter->name(), Names[Index]);
      EXPECT_EQ(Parameter->kind(), ValueKind::FunctionParameter);
      EXPECT_TRUE(FunctionParameter::classof(Parameter));
    }
    Kinds[1] = ParameterKind::Positional;
    Kinds[2] = ParameterKind::Positional;
    Names[0] = Context.namePool().intern("changed");
    Names[1] = Name{};
    for (std::size_t Index = 0; Index < 256; ++Index)
    {
      Context.namePool().intern("extra" + std::to_string(Index));
    }
    EXPECT_EQ(Context.namePool().text(Target->parameters()[0]->name()), "value");
    EXPECT_EQ(Context.namePool().text(Target->parameters()[1]->name()), "flag");
    EXPECT_EQ(Context.namePool().text(Target->parameters()[2]->name()), "rest");
    EXPECT_EQ(Target->parameters()[1]->parameterKind(), ParameterKind::Named);
    EXPECT_EQ(Target->parameters()[2]->parameterKind(), ParameterKind::Variadic);
    Function *Positional = Context.createFunction(Context.namePool().intern("g"), *Signature);
    ASSERT_NE(Positional, nullptr);
    EXPECT_EQ(&Positional->type(), &Target->type());
    for (const FunctionParameter *Parameter : Positional->parameters())
    {
      EXPECT_EQ(Parameter->parameterKind(), ParameterKind::Positional);
      EXPECT_FALSE(Parameter->name().valid());
      EXPECT_EQ(&Parameter->function(), Positional);
    }
  }

  // Name lists must match the signature and use local handles, while explicit unnamed slots remain supported.
  TEST(SemanticFunctionParameterTest, ChecksParameterNamesAndAllowsUnnamedSlots)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type *Types[] = {&Context.getBoolType(), &Context.getBoolType()};
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType(), Types);
    ASSERT_NE(Signature, nullptr);
    const Name FunctionName = Context.namePool().intern("f");
    const Name Local = Context.namePool().intern("flag");
    const Name One[] = {Local};
    const Name Three[] = {Local, Local, Local};
    EXPECT_EQ(Context.createFunction(FunctionName, *Signature, {}, One), nullptr);
    EXPECT_EQ(Context.createFunction(FunctionName, *Signature, {}, Three), nullptr);
    EXPECT_EQ(Context.createFunction(FunctionName, *Context.getFunctionType(Context.getVoidType()), {}, One), nullptr);
    NamePool Other;
    Name OutOfRange;
    for (std::size_t Index = 0; Index < 16; ++Index)
    {
      OutOfRange = Other.intern("foreign" + std::to_string(Index));
    }
    ASSERT_FALSE(Context.namePool().contains(OutOfRange));
    Name Names[] = {Local, OutOfRange};
    EXPECT_EQ(Context.createFunction(FunctionName, *Signature, {}, Names), nullptr);
    Names[1] = Name{};
    Function *Target = Context.createFunction(FunctionName, *Signature, {}, Names);
    ASSERT_NE(Target, nullptr);
    EXPECT_EQ(Target->parameters()[0]->name(), Local);
    EXPECT_FALSE(Target->parameters()[1]->name().valid());
  }

  // A supplied kind list must match the signature exactly and contain only defined enumerators.
  TEST(SemanticFunctionParameterTest, RejectsWrongKindCountAndInvalidKinds)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type *Types[] = {&Context.getBoolType(), &Context.getBoolType()};
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType(), Types);
    ASSERT_NE(Signature, nullptr);
    const Name FunctionName = Context.namePool().intern("f");
    const ParameterKind One[] = {ParameterKind::Named};
    const ParameterKind Three[] = {ParameterKind::Positional, ParameterKind::Named, ParameterKind::Variadic};
    EXPECT_EQ(Context.createFunction(FunctionName, *Signature, One), nullptr);
    EXPECT_EQ(Context.createFunction(FunctionName, *Signature, Three), nullptr);
    EXPECT_EQ(Context.createFunction(FunctionName, *Context.getFunctionType(Context.getVoidType()), One), nullptr);
    ParameterKind Kinds[] = {ParameterKind::Positional, ParameterKind::Named};
    for (std::size_t Index = 0; Index < 2; ++Index)
    {
      const ParameterKind Saved = Kinds[Index];
      Kinds[Index] = static_cast<ParameterKind>(255);
      EXPECT_EQ(Context.createFunction(FunctionName, *Signature, Kinds), nullptr);
      Kinds[Index] = Saved;
    }
    ASSERT_NE(Context.createFunction(FunctionName, *Signature, Kinds), nullptr);
  }

  // Binding categories do not change the already normalized operand slots of a runtime call.
  TEST(SemanticFunctionParameterTest, CallsStillRequireNormalizedSignatureSlots)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type *Types[] = {&Context.getBoolType(), &Context.getBoolType()};
    const ParameterKind Kinds[] = {ParameterKind::Named, ParameterKind::Variadic};
    const FunctionType *Signature = Context.getFunctionType(Context.getVoidType(), Types);
    ASSERT_NE(Signature, nullptr);
    Function *Target = Context.createFunction(Context.namePool().intern("f"), *Signature, Kinds);
    ASSERT_NE(Target, nullptr);
    const Value *Arguments[] = {&Context.getBoolConstant(true), &Context.getBoolConstant(false)};
    const CallInstruction *Call = Context.createCallInstruction(*Target, Arguments);
    ASSERT_NE(Call, nullptr);
    EXPECT_EQ(Call->arguments()[0], Arguments[0]);
    EXPECT_EQ(Call->arguments()[1], Arguments[1]);
    EXPECT_EQ(Context.createCallInstruction(*Target, std::span(Arguments).first(1)), nullptr);
  }
} // namespace ink::semantic::test
