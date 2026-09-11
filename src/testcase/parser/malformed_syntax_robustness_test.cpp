#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    void expectMalformedSyntax(std::string Source)
    {
      Source += " let After: int32 = 1;";
      SCOPED_TRACE(Source);
      const ParsedFile First = test::parseSource(Source);
      const ParsedFile Second = test::parseSource(Source);
      EXPECT_FALSE(First.succeeded());
      EXPECT_FALSE(test::testDiagnostics(First).empty());
      EXPECT_EQ(First.completeness(), ParseCompleteness::Complete);
      const std::vector<std::string> Bindings = test::nodeTextsOfKind(First, AstKind::BindingDeclaration);
      EXPECT_TRUE(std::any_of(Bindings.begin(), Bindings.end(), [](const std::string &Text)
      {
        return Text == "let After: int32 = 1;";
      }));
      EXPECT_EQ(test::astSnapshot(First), test::astSnapshot(Second));
      EXPECT_TRUE(test::diagnosticsEqual(First, Second));
      test::expectAstIntegrity(First);
      test::expectAstIntegrity(Second);
    }

    // Verifies malformed module paths, multiple selective imports, and empty statements recover at the next declaration.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversMalformedImportsAndSemicolonBoundaries)
    {
      for (const std::string Source : {"import .core;", "import core.;", "from core import A, B;", "import core as ;", ";", "func f() -> void {}; "})
      {
        expectMalformedSyntax(Source);
      }
    }

    // Verifies bindings require both a type and initializer and calls admit positional arguments only.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversBindingsAndCallArguments)
    {
      const std::vector<std::string> Sources = {
          "var;",
          "var Value;",
          "const Value: int32;",
          "let Value = 1;",
          "let (A, B): Pair;",
          "let Value: int32 = ;",
          "call(name = Value);",
          "call(...);",
          "call(...Values);",
          "call(Values..., More);",
      };
      for (const std::string &Source : Sources)
      {
        expectMalformedSyntax(Source);
      }
    }

    // Verifies current generic syntax rejects old angle forms, empty arguments, malformed indices, and omitted function-type results.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversGenericIndexAndFunctionTypeBoundaries)
    {
      const std::vector<std::string> Sources = {
          "F::<T>;",
          "F::[];",
          "F::[T,,U];",
          "Value[];",
          "Value[Low:High];",
          "Value[::];",
          "Value.0;",
          "let F: type = func();",
          "let F: type = func(int32) -> ;",
      };
      for (const std::string &Source : Sources)
      {
        expectMalformedSyntax(Source);
      }
    }

    // Verifies generic and function parameter defaults must be trailing and parameter packs must be last.
    TEST(ParserMalformedSyntaxRobustnessTest, RejectsInvalidParameterOrdering)
    {
      const std::vector<std::string> Sources = {
          "func f[]() -> void;",
          "func f[T: type = Default, U: type]() -> void;",
          "func f[T: type..., U: type]() -> void;",
          "func f(A: T = Value, B: T) -> void;",
          "func f(A: T..., B: T) -> void;",
          "func f(..., B: T) -> void;",
          "func f() ;",
          "class_method f();",
      };
      for (const std::string &Source : Sources)
      {
        expectMalformedSyntax(Source);
      }
    }

    // Verifies missing control-flow operands, types, and required blocks preserve following declarations.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversControlFlowSyntax)
    {
      const std::vector<std::string> Sources = {
          "for (let Item in Items) {}",
          "for (const Item: T in Items) {}",
          "for (in Items) {}",
          "for (let Item: T in ) {}",
          "for (var I: int32 = 0; ; I += ) {}",
          "if () {}",
          "if (Ready) return;",
          "while (Ready) return;",
      };
      for (const std::string &Source : Sources)
      {
        expectMalformedSyntax(Source);
      }
    }

    // Verifies object braces, empty and singleton tuples, named arguments, and other removed forms cannot silently retain old meanings.
    TEST(ParserMalformedSyntaxRobustnessTest, RejectsRemovedSyntaxForms)
    {
      const std::vector<std::string> Sources = {
          "Point{1, 2};",
          "Point{};",
          "let Value: Point = Point{1, 2};",
          "();",
          "(Value,);",
          "let (Value,): T = Input;",
          "let X: type = class {};",
          "func f<T: type>() -> void;",
          "class C : Base {}",
          "interface I implements Base {}",
          "enum E { Ready, Busy }",
      };
      for (const std::string &Source : Sources)
      {
        expectMalformedSyntax(Source);
      }
    }

    // Verifies assignments are allowed only in designated statement clauses and require a unary target.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversInvalidAssignmentsAndComptimeOperands)
    {
      const std::vector<std::string> Sources = {
          "let Value: T = (Left = Right);",
          "Left = Middle = Right;",
          "Left + Offset = Right;",
          "call(Left = Right);",
          "let Value: T = comptime ;",
          "let Value: T = comptime if (Ready) {}",
          "Value++;",
          "Value--;",
      };
      for (const std::string &Source : Sources)
      {
        expectMalformedSyntax(Source);
      }
    }

    // Verifies configured depth exhaustion preserves the rejected region and resumes at a later valid declaration.
    TEST(ParserMalformedSyntaxRobustnessTest, RecoversAfterSyntaxNestingLimit)
    {
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 2;
      const std::string Source = "let Broken: Data = [[0]]; let After: int32 = 1;";
      const ParsedFile First = test::parseSource(Source, Options);
      const ParsedFile Second = test::parseSource(Source, Options);
      EXPECT_FALSE(First.succeeded());
      EXPECT_TRUE(test::hasDiagnostic(First, core::DiagnosticKind::SyntaxNestingLimit));
      EXPECT_TRUE(test::hasKind(First, AstKind::Error));
      const auto Bindings = test::nodeTextsOfKind(First, AstKind::BindingDeclaration);
      EXPECT_TRUE(std::find(Bindings.begin(), Bindings.end(), "let After: int32 = 1;") != Bindings.end());
      EXPECT_EQ(test::astSnapshot(First), test::astSnapshot(Second));
      EXPECT_TRUE(test::diagnosticsEqual(First, Second));
      test::expectAstIntegrity(First);
    }
  } // namespace
} // namespace ink::parser
