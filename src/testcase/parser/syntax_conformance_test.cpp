#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    struct ValidSyntaxCase
    {
        const char *Name;
        const char *Source;
        std::vector<AstKind> ExpectedKinds;
    };

    void expectValidSyntax(const ValidSyntaxCase &TestCase)
    {
      SCOPED_TRACE(TestCase.Name);
      const ParsedFile File = test::parseSource(TestCase.Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(test::testDiagnostics(File).empty());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      for (AstKind Kind : TestCase.ExpectedKinds)
      {
        EXPECT_TRUE(test::hasKind(File, Kind)) << "missing " << astKindName(Kind);
      }
      test::expectAstIntegrity(File);
    }

    // Verifies single-segment, qualified, aliased, and single-member imports from the current grammar.
    TEST(ParserDeclarationSyntaxTest, ParsesEveryImportForm)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"SingleModule", "import core;", {AstKind::ImportDeclaration}},
          {"QualifiedAlias", "import platform.window as window;", {AstKind::ImportDeclaration}},
          {"Member", "from application.model import User as CurrentUser;", {AstKind::ImportDeclaration}},
      };
      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies mandatory type and initializer clauses, access modifiers, wildcards, and nested tuple binding patterns.
    TEST(ParserDeclarationSyntaxTest, ParsesBindingDeclarationForms)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"Named", "public let Answer: int32 = 42; private var Cache: Data = Default; const Ready: bool = true;", {AstKind::BindingDeclaration}},
          {"Tuple", "let (First, (Second, _)): Pair = Input;", {AstKind::BindingDeclaration, AstKind::TuplePattern, AstKind::WildcardPattern}},
          {"LocalAndComputed", "func bindings() -> void { var Mutable: makeType() = value; const _: int32 = 1; }", {AstKind::FunctionDeclaration, AstKind::BindingDeclaration, AstKind::CallExpression}},
      };
      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies named function variants, linkage, square generic declarations, default ordering, and final parameter packs.
    TEST(ParserDeclarationSyntaxTest, ParsesFunctionDeclarations)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"ExternConstGeneric", "public extern \"C\" const func load[T: type, N: int32 = 4, Rest: type...](path: const ref T, count: int32 = 1, values: T...) -> Result;", {AstKind::FunctionDeclaration, AstKind::ReferenceTypeExpression}},
          {"Method", "private class_method create[T: type](value: T) -> Self { return Self(value); }", {AstKind::FunctionDeclaration, AstKind::CallExpression}},
          {"BareVariadic", "extern \"C\" func trace(format: Text, ...) -> void;", {AstKind::FunctionDeclaration}},
          {"DestructuredParameter", "func sum((left, right): (int32, int32), _: bool) -> int32 { return left + right; }", {AstKind::TuplePattern, AstKind::WildcardPattern, AstKind::TupleExpression}},
      };
      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies class, interface, and enum headers and their uniform statement bodies without premature semantic region checks.
    TEST(ParserDeclarationSyntaxTest, ParsesNominalDeclarationsAndFieldStatements)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"Class", "public class Box[T: type] extends makeBase(T) implements Printable, Sized { private const class_field Value: T = Default; class_method get() -> T { return self.Value; } class Nested {} }", {AstKind::ClassDeclaration, AstKind::ClassFieldDeclaration, AstKind::FunctionDeclaration, AstKind::ReceiverExpression}},
          {"Interface", "interface Container[T: type] extends First, Second { func read() -> T; interface Nested {} }", {AstKind::InterfaceDeclaration, AstKind::FunctionDeclaration}},
          {"Enum", "enum State[T: type] : int32 implements Printable { enum_field Ready; enum_field Busy = 2; }", {AstKind::EnumDeclaration, AstKind::EnumFieldDeclaration}},
          {"SemanticRegionChecksDeferred", "class C { return; import core; } interface I { let Value: int32 = 1; } enum E { func f() -> void; } class_field Top: int32; enum_field First;", {AstKind::ClassDeclaration, AstKind::InterfaceDeclaration, AstKind::EnumDeclaration, AstKind::ClassFieldDeclaration, AstKind::EnumFieldDeclaration}},
      };
      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies control flow, both for forms, comma-separated updates, assignments, and every defer form.
    TEST(ParserStatementSyntaxTest, ParsesRuntimeStatementFamilies)
    {
      const ValidSyntaxCase TestCase = {"Statements", "func flow() -> void { if (Ready) { break; } else if (Other) { continue; } else {} while (Ready) { tick(); } for (let (Key, Value): Entry in Items) {} for (var I: int32 = 0; I < 10; I += 1, visit(I)) {} for (First = 0, Second = 1; Ready; First += 1, Second -= 1) {} for (;;) {} defer close(); defer Value = Next; defer {} return; }", {AstKind::IfStatement, AstKind::WhileStatement, AstKind::ForStatement, AstKind::ForInStatement, AstKind::AssignmentStatement, AstKind::DeferStatement, AstKind::BreakStatement, AstKind::ContinueStatement, AstKind::ReturnStatement}};
      expectValidSyntax(TestCase);
    }

    // Verifies statements remain syntactically accepted at the top level and inside explicit blocks for later semantic filtering.
    TEST(ParserStatementSyntaxTest, DefersRegionAndControlFlowLegalityToSemantics)
    {
      const ValidSyntaxCase TestCase = {"UniformStatements", "return; break; continue; Value; if (Ready) {} { import core; class Nested {} }", {AstKind::ReturnStatement, AstKind::BreakStatement, AstKind::ContinueStatement, AstKind::ExpressionStatement, AstKind::IfStatement, AstKind::BlockStatement}};
      expectValidSyntax(TestCase);
    }

    // Verifies literals, tuples, arrays, fixed operator precedence, conditional expressions, and every shared postfix.
    TEST(ParserExpressionSyntaxTest, ParsesExpressionFamiliesAndPostfixChains)
    {
      const ValidSyntaxCase TestCase = {"Expressions", "let Ordered: int32 = A + B * C << D & E ^ F | G && H || I; let Choice: T = Ready ? First : Second; let Postfix: T = Object.method::[T](Value, Arguments...)[Index].field->next; let Builtins: type = (int32, float); let Tuple: Data = (true, false, null, 1, 1.0, \"text\", self, super); let Array: Data = [1, 2, 3]; let Empty: Data = []; let Unary: int32 = comptime - + ! ~ * & Value;", {AstKind::BinaryExpression, AstKind::ConditionalExpression, AstKind::CallExpression, AstKind::IndexExpression, AstKind::MemberExpression, AstKind::GenericInstantiationExpression, AstKind::ArrayExpression, AstKind::TupleExpression, AstKind::UnaryExpression, AstKind::ComptimeExpression, AstKind::LiteralExpression, AstKind::BuiltinTypeExpression, AstKind::ReceiverExpression}};
      expectValidSyntax(TestCase);
    }

    // Verifies all type positions parse ordinary expressions and prefix type constructors consume unary operands.
    TEST(ParserTypeSyntaxTest, ParsesTypeSyntaxFamilies)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"PrefixTypes", "func types(A: ref T, B: const ptr T, C: ptr ref T, D: (T, U), E: T[N], F: makeType()) -> Ready ? T : U;", {AstKind::ReferenceTypeExpression, AstKind::PointerTypeExpression, AstKind::TupleExpression, AstKind::IndexExpression, AstKind::CallExpression, AstKind::ConditionalExpression}},
          {"FunctionTypes", "let A: type = func() -> void; let B: type = extern \"C\" func(int32, ptr T, ...) -> bool; let C: type = func(T...) -> (Ready ? T : U);", {AstKind::FunctionTypeExpression, AstKind::PointerTypeExpression, AstKind::ConditionalExpression}},
          {"ComplexHeaders", "func f() -> makeType() { return value; } class C extends Ready ? Base : Other implements Contract::[T] {}", {AstKind::FunctionDeclaration, AstKind::ClassDeclaration, AstKind::CallExpression, AstKind::ConditionalExpression, AstKind::GenericInstantiationExpression}},
      };
      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies comptime wraps any complete statement while expression-position comptime stays a unary expression.
    TEST(ParserComptimeSyntaxTest, ParsesUnifiedComptimeStatements)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"ExpressionAndAssignment", "comptime A + B; comptime Value = Next; let X: int32 = A + comptime B;", {AstKind::ComptimeStatement, AstKind::AssignmentStatement, AstKind::ComptimeExpression}},
          {"Controls", "comptime if (Enabled) {} else if (Other) {} else {} comptime for (let Item: T in Items) {} comptime while (Enabled) {}", {AstKind::ComptimeStatement, AstKind::IfStatement, AstKind::ForInStatement, AstKind::WhileStatement}},
          {"DeclarationsAndReturn", "comptime func f() -> void; comptime class C {} comptime return; comptime { let Value: int32 = 1; }", {AstKind::ComptimeStatement, AstKind::FunctionDeclaration, AstKind::ClassDeclaration, AstKind::ReturnStatement}},
          {"NestedMembers", "class C { class D { comptime if (Enabled) { class_field Value: int32; } } }", {AstKind::ClassDeclaration, AstKind::ComptimeStatement, AstKind::ClassFieldDeclaration}},
      };
      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }
  } // namespace
} // namespace ink::parser
