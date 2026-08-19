#include "ink/core/context.h"
#include "ink/ir/analysis/verifier.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/function.h"
#include "ink/ir/model/module.h"
#include "ink/ir/model/parameter.h"
#include "ink/ir/serialization.h"

#include "../diagnostic_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    bool hasParameterDiagnostic(const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind)
    {
      return std::any_of(Diagnostics.begin(), Diagnostics.end(), [Kind](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == Kind;
      });
    }

    // Verifies that Parameter encapsulates its name, type, and optional default constant.
    TEST(FunctionParameterTest, StoresParameterMetadata)
    {
      core::CompilationContext Compilation;
      IRContext Context(Compilation);
      const Type &VoidType = Context.getType(TypeKind::Void);
      const Type &I32Type = Context.getType(TypeKind::I32);
      const IntegerConstant &DefaultValue = Context.constantPool().getIntegerConstant(I32Type, 7);
      Parameter Value("Value", &I32Type, &DefaultValue);
      Function FunctionValue(VoidType);
      FunctionValue.Parameters.push_back(Value);
      FunctionValue.Parameters.emplace_back(&I32Type);

      EXPECT_EQ(Value.name(), "Value");
      EXPECT_EQ(Value.type(), &I32Type);
      EXPECT_EQ(Value.defaultValue(), &DefaultValue);
      ASSERT_EQ(FunctionValue.parameterCount(), 2U);
      EXPECT_EQ(&FunctionValue.parameter(0), &FunctionValue.Parameters[0]);
      EXPECT_EQ(FunctionValue.parameterType(0), &I32Type);
      EXPECT_TRUE(FunctionValue.parameter(1).name().empty());
      EXPECT_EQ(FunctionValue.parameter(1).defaultValue(), nullptr);
    }

    // Verifies that names and defaults on external, imported, and defined parameters survive canonical InkIR text round trips.
    TEST(FunctionParameterSerializationTest, RoundTripsEveryFunctionKind)
    {
      core::CompilationContext FirstCompilation;
      IRContext FirstContext(FirstCompilation);
      const std::string Text =
          "inkir 1\n"
          "\n"
          "declare extern \"C\" i32 @external(const byte*, Count: i32 = i32 7) [sideeffect]\n"
          "\n"
          "declare import void @dependency.hook(Value: i32 = i32 1) from module dependency.api, symbol @hook\n"
          "\n"
          "define i32 @defined(Value: i32 %0, Step: i32 %1 = i32 3) [stored] {\n"
          "entry:\n"
          "  ret i32 %0\n"
          "}\n";

      DeserializeResult FirstResult = deserialize(FirstContext, Text);

      ASSERT_TRUE(FirstResult.succeeded());
      ASSERT_TRUE(FirstResult.module().has_value());
      ASSERT_EQ(FirstResult.module()->Functions.size(), 3U);
      const Function &External = FirstResult.module()->Functions[0];
      const Function &Imported = FirstResult.module()->Functions[1];
      const Function &Defined = FirstResult.module()->Functions[2];
      ASSERT_EQ(External.parameterCount(), 2U);
      EXPECT_TRUE(External.parameter(0).name().empty());
      EXPECT_EQ(External.parameter(1).name(), "Count");
      ASSERT_NE(External.parameter(1).defaultValue(), nullptr);
      EXPECT_EQ(External.parameter(1).defaultValue()->kind(), ValueKind::IntegerConstant);
      ASSERT_EQ(Imported.parameterCount(), 1U);
      EXPECT_EQ(Imported.parameter(0).name(), "Value");
      ASSERT_EQ(Defined.parameterCount(), 2U);
      EXPECT_EQ(Defined.parameter(0).name(), "Value");
      EXPECT_EQ(Defined.parameter(1).name(), "Step");
      EXPECT_EQ(Defined.parameterType(1)->kind(), TypeKind::I32);
      ASSERT_NE(Defined.parameter(1).defaultValue(), nullptr);
      SerializeResult Serialized = serialize(FirstContext, *FirstResult.module());
      ASSERT_TRUE(Serialized.succeeded());
      ASSERT_TRUE(Serialized.text().has_value());
      EXPECT_EQ(*Serialized.text(), Text);

      core::CompilationContext SecondCompilation;
      IRContext SecondContext(SecondCompilation);
      DeserializeResult SecondResult = deserialize(SecondContext, *Serialized.text());

      ASSERT_TRUE(SecondResult.succeeded());
      ASSERT_TRUE(SecondResult.module().has_value());
      ASSERT_EQ(SecondResult.module()->Functions.size(), 3U);
      EXPECT_EQ(SecondResult.module()->Functions[0].parameter(1).name(), "Count");
      ASSERT_NE(SecondResult.module()->Functions[2].parameter(1).defaultValue(), nullptr);
      EXPECT_EQ(SecondResult.module()->Functions[2].parameter(1).defaultValue()->kind(), ValueKind::IntegerConstant);
      EXPECT_NE(Defined.parameter(1).defaultValue(), SecondResult.module()->Functions[2].parameter(1).defaultValue());
    }

    // Verifies that parameter verification diagnoses invalid and duplicate names together with mismatched and foreign defaults.
    TEST(FunctionParameterVerifierTest, RejectsMalformedParameterMetadata)
    {
      core::CompilationContext Compilation;
      ink::test::DiagnosticCapture Diagnostics(Compilation);
      IRContext Context(Compilation);
      core::CompilationContext ForeignCompilation;
      IRContext ForeignContext(ForeignCompilation);
      const Type &VoidType = Context.getType(TypeKind::Void);
      const Type &I32Type = Context.getType(TypeKind::I32);
      const Type &ByteType = Context.getType(TypeKind::Byte);
      const IntegerConstant &WrongTypeDefault = Context.constantPool().getIntegerConstant(ByteType, 1);
      const IntegerConstant &ForeignDefault = ForeignContext.constantPool().getIntegerConstant(I32Type, 2);
      Function External(VoidType);
      External.Name = "external";
      External.Kind = FunctionKind::External;
      External.Convention = CallingConvention::C;
      External.Parameters.emplace_back("1Invalid", &I32Type, &WrongTypeDefault);
      External.Parameters.emplace_back("Repeated", &I32Type, &ForeignDefault);
      External.Parameters.emplace_back("Repeated", &I32Type);
      Module ModuleValue(Context);
      ModuleValue.Functions.push_back(std::move(External));

      const VerificationResult Result = verify(Context, ModuleValue);

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasParameterDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrInvalidFunctionParameterName));
      EXPECT_TRUE(hasParameterDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrDuplicateFunctionParameterName));
      EXPECT_TRUE(hasParameterDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrFunctionParameterDefaultTypeMismatch));
      EXPECT_TRUE(hasParameterDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrConstantPoolMismatch));
    }

    // Verifies that parsed defaults on every function form must match their parameter type.
    TEST(FunctionParameterSerializationTest, RejectsInvalidTextMetadata)
    {
      const std::vector<std::pair<std::string, core::DiagnosticKind>> Cases = {
          {"inkir 1\ndeclare extern \"C\" void @external(Value: i32 = byte 1)\n", core::DiagnosticKind::IrFunctionParameterDefaultTypeMismatch},
          {"inkir 1\ndeclare import void @imported(Value: i32 = byte 1) from module dependency, symbol @target\n", core::DiagnosticKind::IrFunctionParameterDefaultTypeMismatch},
          {"inkir 1\ndefine void @defined(Value: i32 %0 = byte 1) {\nentry:\n  ret void\n}\n", core::DiagnosticKind::IrFunctionParameterDefaultTypeMismatch},
      };
      for (const auto &Case : Cases)
      {
        core::CompilationContext Compilation;
        ink::test::DiagnosticCapture Diagnostics(Compilation);
        IRContext Context(Compilation);

        const DeserializeResult Result = deserialize(Context, Case.first);

        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasParameterDiagnostic(Diagnostics.diagnostics(), Case.second));
      }
    }

    // Verifies that external, imported, and defined parameters all reject attribute lists because parameter attributes have no InkIR semantics.
    TEST(FunctionParameterSerializationTest, RejectsParameterAttributes)
    {
      const std::vector<std::string> InvalidTexts = {
          "inkir 1\ndeclare extern \"C\" void @external(Value: i32 [reflect])\n",
          "inkir 1\ndeclare import void @imported(Value: i32 [reflect]) from module dependency, symbol @target\n",
          "inkir 1\ndefine void @defined(Value: i32 %0 [reflect]) {\nentry:\n  ret void\n}\n",
      };
      for (const std::string &Text : InvalidTexts)
      {
        core::CompilationContext Compilation;
        ink::test::DiagnosticCapture Diagnostics(Compilation);
        IRContext Context(Compilation);

        const DeserializeResult Result = deserialize(Context, Text);

        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasParameterDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrExpected));
      }
    }
  } // namespace
} // namespace ink::ir
