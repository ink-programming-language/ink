#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/module/module.h"
#include "ink/ir/type/type_pool.h"
#include "ink/ir/verification/verifier.h"

#include <gtest/gtest.h>

#include <array>

namespace ink::ir::test
{
  // C ellipsis declarations retain only their typed fixed parameters and create no argument for the ellipsis itself.
  TEST(IRVariadicVerifierTest, AcceptsCVariadicDeclarationsWithOnlyFixedArguments)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const std::array Parameters{Parameter(Int32)};
    const FunctionType &Signature = Types.getFunctionType(Int32, Parameters, CallABI{ABIKind::C, CallingConvention::CDecl}, true);
    Module ModuleValue;
    Function &Declaration = ModuleValue.createFunction(Signature);
    verifyModule(ModuleValue, Engine);
    verifyFunction(Declaration, Engine);
    EXPECT_TRUE(Signature.isVariadic());
    EXPECT_EQ(Signature.parameters().size(), 1U);
    ASSERT_EQ(Declaration.argumentCount(), 1U);
    EXPECT_EQ(&Declaration.argument(0).getType(), &Int32);
    EXPECT_EQ(Declaration.argument(0).kind(), Parameter::Kind::Positional);
  }

  // A variadic definition can return a fixed argument without modeling any of its additional call-site values as local arguments.
  TEST(IRVariadicVerifierTest, AcceptsCVariadicDefinitionsReturningAFixedArgument)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const std::array Parameters{Parameter(Int32, Parameter::Kind::Named)};
    const FunctionType &Signature = Types.getFunctionType(Int32, Parameters, CallABI{ABIKind::C, CallingConvention::CDecl}, true);
    Module ModuleValue;
    Function &Definition = ModuleValue.createFunction(Signature);
    InstructionBuilder Builder(Types);
    Builder.setInsertPoint(Definition.createBasicBlock());
    ReturnInstruction &Return = Builder.createReturn(Definition.argument(0));
    verifyModule(ModuleValue, Engine);
    verifyFunction(Definition, Engine);
    EXPECT_TRUE(Signature.isVariadic());
    EXPECT_EQ(Definition.argumentCount(), Signature.parameters().size());
    EXPECT_EQ(Return.getReturnValue(), &Definition.argument(0));
  }

  // An ellipsis-only signature has no fixed arguments, and validation leaves ABI-specific support decisions to lowering.
  TEST(IRVariadicVerifierTest, AcceptsEllipsisOnlySignaturesForCAndInkABI)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const CallABI Contracts[] = {
        {ABIKind::C, CallingConvention::CDecl},
        {ABIKind::Ink, CallingConvention::Default},
    };
    for (CallABI ABI : Contracts)
    {
      const FunctionType &Signature = Types.getFunctionType(Types.getVoidType(), {}, ABI, true);
      Module ModuleValue;
      Function &Declaration = ModuleValue.createFunction(Signature);
      Function &Definition = ModuleValue.createFunction(Signature);
      InstructionBuilder Builder(Types);
      Builder.setInsertPoint(Definition.createBasicBlock());
      static_cast<void>(Builder.createReturn());
      verifyModule(ModuleValue, Engine);
      EXPECT_TRUE(Signature.isVariadic());
      EXPECT_TRUE(Signature.parameters().empty());
      EXPECT_EQ(Declaration.argumentCount(), 0U);
      EXPECT_EQ(Definition.argumentCount(), 0U);
    }
  }

  // Enabling ellipsis never exempts a declared fixed parameter from the non-void parameter-type invariant.
  TEST(IRVariadicVerifierDeathTest, RejectsVoidFixedParametersInVariadicSignatures)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const std::array Parameters{Parameter(Types.getVoidType())};
    const FunctionType Signature(Types.getVoidType(), Parameters, CallABI{ABIKind::C, CallingConvention::CDecl}, true);
    Module ModuleValue;
    static_cast<void>(ModuleValue.createFunction(Signature));
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // A fixed parameter still requires a type value when its signature also permits additional untyped arguments.
  TEST(IRVariadicVerifierDeathTest, RejectsNonTypeFixedParametersInVariadicSignatures)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const IntegerConstant OrdinaryValue(Int32, 1);
    const std::array Parameters{Parameter(OrdinaryValue)};
    const FunctionType Signature(Types.getVoidType(), Parameters, CallABI{ABIKind::C, CallingConvention::CDecl}, true);
    Module ModuleValue;
    static_cast<void>(ModuleValue.createFunction(Signature));
    EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
  }

  // The former Variadic parameter enum value is invalid even when the replacement function-level ellipsis flag is enabled.
  TEST(IRVariadicVerifierDeathTest, RejectsLegacyVariadicParameterKindInEverySignatureMode)
  {
    const core::DiagnosticEngine Engine;
    TypePool Types;
    const IntegerType &Int32 = Types.getIntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed);
    const std::array Parameters{Parameter(Int32, static_cast<Parameter::Kind>(2))};
    for (bool IsVariadic : {false, true})
    {
      const FunctionType Signature(Types.getVoidType(), Parameters, CallABI{}, IsVariadic);
      Module ModuleValue;
      static_cast<void>(ModuleValue.createFunction(Signature));
      EXPECT_DEATH(verifyModule(ModuleValue, Engine), "INK-I0001");
    }
  }
} // namespace ink::ir::test
