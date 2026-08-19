#include "ink/core/context.h"
#include "ink/ir/analysis/verifier.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/function.h"
#include "ink/ir/model/module.h"
#include "ink/ir/serialization.h"

#include "../diagnostic_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    bool hasDiagnostic(const std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind)
    {
      return std::any_of(Diagnostics.begin(), Diagnostics.end(), [Kind](const core::Diagnostic &Diagnostic)
      {
        return Diagnostic.Kind == Kind;
      });
    }

    // Verifies that a function owns ordered attributes, preserves canonical argument constants, and exposes reusable lookup helpers.
    TEST(FunctionAttributeTest, StoresAndFindsAttributes)
    {
      core::CompilationContext Compilation;
      IRContext Context(Compilation);
      const Type &VoidType = Context.getType(TypeKind::Void);
      const Type &I32Type = Context.getType(TypeKind::I32);
      const IntegerConstant &Level = Context.constantPool().getIntegerConstant(I32Type, 3);
      std::vector<AttributeArgument> Arguments;
      Arguments.emplace_back("Level", Level);
      Function FunctionValue(VoidType);
      FunctionValue.Attributes.emplace_back(AttributeKind::Reflect, std::move(Arguments));
      FunctionValue.Attributes.emplace_back(AttributeKind::SideEffect);

      ASSERT_EQ(FunctionValue.Attributes.size(), 2U);
      const Attribute *Reflect = FunctionValue.attribute(AttributeKind::Reflect);
      ASSERT_NE(Reflect, nullptr);
      ASSERT_EQ(Reflect->arguments().size(), 1U);
      EXPECT_EQ(Reflect->arguments()[0].key(), "Level");
      EXPECT_EQ(&Reflect->arguments()[0].value(), &Level);
      EXPECT_TRUE(FunctionValue.hasAttribute(AttributeKind::SideEffect));
      EXPECT_FALSE(FunctionValue.hasAttribute(AttributeKind::Serialize));
      EXPECT_EQ(findAttribute(FunctionValue.Attributes, AttributeKind::SideEffect), &FunctionValue.Attributes[1]);
      EXPECT_TRUE(hasAttribute(FunctionValue.Attributes, AttributeKind::Reflect));
      EXPECT_STREQ(attributeKindName(AttributeKind::SideEffect), "SideEffect");
      EXPECT_STREQ(attributeKindSpelling(AttributeKind::SideEffect), "sideeffect");
      EXPECT_EQ(attributeKindFromSpelling("sideeffect"), AttributeKind::SideEffect);
    }

    // Verifies that external, imported, and defined functions preserve ordered attributes and typed constant arguments across canonical text round trips.
    TEST(FunctionAttributeSerializationTest, RoundTripsEveryFunctionKind)
    {
      core::CompilationContext FirstCompilation;
      IRContext FirstContext(FirstCompilation);
      const std::string Text =
          "inkir 1\n"
          "\n"
          "declare extern \"C\" i32 @external(i32) [sideeffect, reflect(Level = i32 7)]\n"
          "\n"
          "declare import void @dependency.hook() from module dependency.api, symbol @hook [serialize(Version = i32 1)]\n"
          "\n"
          "define void @main() [reflect(Label = const byte[] c\"entry\"), stored] {\n"
          "entry:\n"
          "  ret void\n"
          "}\n";

      DeserializeResult FirstResult = deserialize(FirstContext, Text);

      ASSERT_TRUE(FirstResult.succeeded());
      ASSERT_TRUE(FirstResult.module().has_value());
      ASSERT_EQ(FirstResult.module()->Functions.size(), 3U);
      const Function &External = FirstResult.module()->Functions[0];
      const Function &Imported = FirstResult.module()->Functions[1];
      const Function &Defined = FirstResult.module()->Functions[2];
      ASSERT_EQ(External.Attributes.size(), 2U);
      EXPECT_TRUE(External.hasAttribute(AttributeKind::SideEffect));
      ASSERT_NE(External.attribute(AttributeKind::Reflect), nullptr);
      ASSERT_EQ(External.attribute(AttributeKind::Reflect)->arguments().size(), 1U);
      EXPECT_EQ(External.attribute(AttributeKind::Reflect)->arguments()[0].value().kind(), ValueKind::IntegerConstant);
      ASSERT_EQ(Imported.Attributes.size(), 1U);
      EXPECT_EQ(Imported.Attributes[0].kind(), AttributeKind::Serialize);
      ASSERT_EQ(Imported.Attributes[0].arguments().size(), 1U);
      ASSERT_EQ(Defined.Attributes.size(), 2U);
      EXPECT_EQ(Defined.Attributes[0].kind(), AttributeKind::Reflect);
      EXPECT_EQ(Defined.Attributes[0].arguments()[0].value().kind(), ValueKind::StringConstant);
      EXPECT_EQ(Defined.Attributes[1].kind(), AttributeKind::Stored);
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
      EXPECT_TRUE(SecondResult.module()->Functions[0].hasAttribute(AttributeKind::SideEffect));
      ASSERT_NE(SecondResult.module()->Functions[2].attribute(AttributeKind::Reflect), nullptr);
      EXPECT_EQ(SecondResult.module()->Functions[2].attribute(AttributeKind::Reflect)->arguments()[0].value().kind(), ValueKind::StringConstant);
    }

    // Verifies that function attribute verification reports unknown kinds, invalid and duplicate argument names, and constants owned by another IR context.
    TEST(FunctionAttributeVerifierTest, RejectsMalformedAttributeMetadata)
    {
      core::CompilationContext Compilation;
      ink::test::DiagnosticCapture Diagnostics(Compilation);
      IRContext Context(Compilation);
      core::CompilationContext ForeignCompilation;
      IRContext ForeignContext(ForeignCompilation);
      const Type &VoidType = Context.getType(TypeKind::Void);
      const Type &I32Type = Context.getType(TypeKind::I32);
      const IntegerConstant &ForeignValue = ForeignContext.constantPool().getIntegerConstant(I32Type, 1);
      std::vector<AttributeArgument> InvalidArguments;
      InvalidArguments.emplace_back("", ForeignValue);
      InvalidArguments.emplace_back("Duplicate", ForeignValue);
      InvalidArguments.emplace_back("Duplicate", ForeignValue);
      Function External(VoidType);
      External.Name = "external";
      External.Kind = FunctionKind::External;
      External.Convention = CallingConvention::C;
      External.Attributes.emplace_back(AttributeKind::Reflect, std::move(InvalidArguments));
      External.Attributes.emplace_back(static_cast<AttributeKind>(static_cast<std::uint8_t>(AttributeKind::Count) + 1U));
      Module ModuleValue(Context);
      ModuleValue.Functions.push_back(std::move(External));

      const VerificationResult Result = verify(Context, ModuleValue);

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrUnknownFunctionAttribute));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrInvalidFunctionAttributeArgumentName));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrDuplicateFunctionAttributeArgumentName));
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrConstantPoolMismatch));
    }

    // Verifies that the sideeffect attribute retains the old declaration-only rule when represented through the common attribute model.
    TEST(FunctionAttributeVerifierTest, RejectsSideEffectOnDefinedFunction)
    {
      core::CompilationContext Compilation;
      ink::test::DiagnosticCapture Diagnostics(Compilation);
      IRContext Context(Compilation);
      const Type &VoidType = Context.getType(TypeKind::Void);
      Function Defined(VoidType);
      Defined.Name = "defined";
      Defined.Attributes.emplace_back(AttributeKind::SideEffect);
      BasicBlock Entry;
      Entry.Name = "entry";
      Entry.Instructions.push_back(std::make_unique<ReturnInstruction>());
      Defined.Blocks.push_back(std::move(Entry));
      Module ModuleValue(Context);
      ModuleValue.Functions.push_back(std::move(Defined));

      const VerificationResult Result = verify(Context, ModuleValue);

      EXPECT_FALSE(Result.succeeded());
      EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrDefinedFunctionHasExternalSideEffects));

      core::CompilationContext TextCompilation;
      ink::test::DiagnosticCapture TextDiagnostics(TextCompilation);
      IRContext TextContext(TextCompilation);
      const DeserializeResult Parsed = deserialize(TextContext, "inkir 1\ndefine void @defined() [sideeffect] {\nentry:\n  ret void\n}\n");

      EXPECT_FALSE(Parsed.succeeded());
      EXPECT_TRUE(hasDiagnostic(TextDiagnostics.diagnostics(), core::DiagnosticKind::IrDefinedFunctionHasExternalSideEffects));
    }

    // Verifies that every function syntax rejects names outside the fixed built-in attribute registry.
    TEST(FunctionAttributeSerializationTest, RejectsUnknownAttributeKinds)
    {
      const std::vector<std::string> InvalidTexts = {
          "inkir 1\ndeclare extern \"C\" void @external() [unknown]\n",
          "inkir 1\ndeclare import void @imported() from module dependency, symbol @target [unknown]\n",
          "inkir 1\ndefine void @defined() [unknown] {\nentry:\n  ret void\n}\n",
      };
      for (const std::string &Text : InvalidTexts)
      {
        core::CompilationContext Compilation;
        ink::test::DiagnosticCapture Diagnostics(Compilation);
        IRContext Context(Compilation);

        const DeserializeResult Result = deserialize(Context, Text);

        EXPECT_FALSE(Result.succeeded());
        EXPECT_TRUE(hasDiagnostic(Diagnostics.diagnostics(), core::DiagnosticKind::IrExpected));
      }
    }
  } // namespace
} // namespace ink::ir
