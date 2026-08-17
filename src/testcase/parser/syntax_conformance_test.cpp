#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    using test::expectFullFidelity;
    using test::hasKind;
    using test::parseSource;

    struct ValidSyntaxCase
    {
        const char *Name;
        const char *Source;
        std::vector<CstKind> ExpectedKinds;
    };

    void expectValidSyntax(const ValidSyntaxCase &TestCase)
    {
      SCOPED_TRACE(TestCase.Name);
      const ParsedFile File = parseSource(TestCase.Source);
      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(File.diagnostics().empty());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      for (CstKind Kind : TestCase.ExpectedKinds)
      {
        EXPECT_TRUE(hasKind(File, Kind)) << "missing " << cstKindName(Kind);
      }
      expectFullFidelity(File);
    }

    // Verifies absolute, relative, aliased, and selective import declarations with their distinct CST forms.
    TEST(ParserDeclarationSyntaxTest, ParsesEveryImportForm)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"AbsoluteModule", "import core.io;", {CstKind::ModuleImportDeclaration, CstKind::ModulePath}},
          {"RelativeAlias", "import ..platform.window as window;", {CstKind::ModuleImportDeclaration, CstKind::ModulePath, CstKind::ImportAlias}},
          {"SelectiveMembers", "from application.model import User, Session as CurrentSession;", {CstKind::MemberImportDeclaration, CstKind::ImportedMember, CstKind::ImportAlias}},
      };

      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies top-level and local named bindings plus tuple destructuring with type annotations and initializers.
    TEST(ParserDeclarationSyntaxTest, ParsesBindingDeclarationForms)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"TopLevelAccessAndType", "public const Answer: i32 = 42; private var Cache: Data; protected var Ready = true;", {CstKind::TopLevelBindingDeclaration, CstKind::AccessModifier, CstKind::NamedBindingDeclaration, CstKind::TypeSyntax}},
          {"TopLevelTuplePattern", "const (First, Second) = Pair;", {CstKind::TopLevelBindingDeclaration, CstKind::TupleDestructuringDeclaration, CstKind::TuplePattern, CstKind::BindingPattern}},
          {"LocalBindings", "func bindings() { var Mutable: i32 = 0; const Fixed = 1; const (Left, _) = Pair; }", {CstKind::LocalBindingDeclaration, CstKind::NamedBindingDeclaration, CstKind::TupleDestructuringDeclaration, CstKind::WildcardPattern}},
      };

      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies function attributes, decorators, modifiers, generics, defaults, packs, receiver qualifiers, and declaration bodies.
    TEST(ParserDeclarationSyntaxTest, ParsesFunctionAndDecoratorDeclarations)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"AnnotatedGenericFunction", "[nothrow, reflect(level = 1)] @trace(kind = \"io\") public extern \"C\" static async func load<T: type, N: ptrsize = 4, Rest: type...>(path: const Data&, count: i32 = 1, values: Data...) const -> Result;", {CstKind::FunctionDeclaration, CstKind::AttributeList, CstKind::AttributeApplication, CstKind::DecoratorApplication, CstKind::ExternModifier, CstKind::FunctionModifier, CstKind::IdentifierFunctionName, CstKind::GenericParameterClause, CstKind::DefaultArgument, CstKind::ParameterPackSuffix, CstKind::FunctionParameterClause, CstKind::ReceiverQualifier, CstKind::ReturnClause}},
          {"DecoratorDeclaration", "[meta] decorator trace(level: i32 = 1) { return; }", {CstKind::DecoratorDeclaration, CstKind::AttributeList, CstKind::FunctionDefinition, CstKind::ReturnStatement}},
          {"ConstructorInitializers", "func initialize(value: i32) : Base(value), storage.slot(value) { return; }", {CstKind::FunctionDeclaration, CstKind::ConstructorInitializerClause, CstKind::ConstructorInitializer, CstKind::ConstructorInitializerTarget}},
          {"DestructorName", "func ~Resource();", {CstKind::FunctionDeclaration, CstKind::DestructorFormFunctionName}},
      };

      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies class, interface, enum, nesting, inheritance, fields, enum payloads, and class type expressions.
    TEST(ParserDeclarationSyntaxTest, ParsesNominalAndAnonymousTypeDeclarations)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"ClassMembers", "[entity] public final class Box<T: type> : Base, Printable { public var Value: T; func get() -> T { return Value; } class Nested {} interface Contract {} enum State { Ready, Busy(i32) = 2 } }", {CstKind::ClassDeclaration, CstKind::TypeDeclarationPrefix, CstKind::ClassDefinitionTail, CstKind::TypeModifier, CstKind::InheritanceClause, CstKind::ClassMemberBlock, CstKind::FieldDeclaration, CstKind::FieldAnnotationSequence, CstKind::FieldModifierSequence, CstKind::FunctionDeclaration, CstKind::InterfaceDeclaration, CstKind::EnumDeclaration}},
          {"InterfaceMembers", "interface Printable : Base { func print() -> void; protected var Version: i32; }", {CstKind::InterfaceDeclaration, CstKind::TypeDeclarationPrefix, CstKind::InterfaceMemberBlock, CstKind::InheritanceClause, CstKind::FunctionDeclaration, CstKind::FieldDeclaration}},
          {"EnumPayloads", "enum Result<T: type> { [success] Success(T), Failure(i32, String) = 2, Empty }", {CstKind::EnumDeclaration, CstKind::TypeDeclarationPrefix, CstKind::EnumMemberBlock, CstKind::EnumBranch, CstKind::EnumPayloadClause, CstKind::EnumDiscriminantClause}},
          {"AnonymousClassValue", "const Anonymous = class : Base { var Value: i32; func read() -> i32 { return Value; } };", {CstKind::ClassTypeExpression, CstKind::TypeDeclarationPrefix, CstKind::ClassDefinitionTail, CstKind::InheritanceClause, CstKind::FieldDeclaration, CstKind::FunctionDeclaration}},
      };

      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies declarations and class-valued expressions share the required prefix and definition-tail shape while fields retain annotations, modifiers, and initializers as separate CST regions.
    TEST(ParserDeclarationSyntaxTest, PreservesSharedClassAndFieldStructure)
    {
      const ParsedFile File = parseSource("[entity] public final class Box<T: type> : Base { [stored] private var Value: T = Default; } const Anonymous = [entity] final class Named<T: type> : Base { [stored] public const Value: T = Default; };");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(test::countKind(File, CstKind::TypeDeclarationPrefix), 2u);
      EXPECT_EQ(test::countKind(File, CstKind::ClassDefinitionTail), 2u);
      EXPECT_EQ(test::countKind(File, CstKind::FieldAnnotationSequence), 2u);
      EXPECT_EQ(test::countKind(File, CstKind::FieldModifierSequence), 2u);
      EXPECT_EQ(test::countKind(File, CstKind::FieldInitializer), 2u);
      expectFullFidelity(File);
    }

    // Verifies all runtime statement families, conditional patterns, loops, jumps, defers, throws, and catch clauses.
    TEST(ParserStatementSyntaxTest, ParsesRuntimeStatementFamilies)
    {
      const ValidSyntaxCase TestCase = {"RuntimeStatements", "func statements() { var Local: i32 = 0; const Fixed = 1; Local = Fixed; Local + Fixed; if (Ready) { Local += 1; } else if (Fallback) {} else {} if (match .some(Value) = Candidate) {} match (Candidate) { .some(Value) => { Value; } _ => return; } while (Ready) { break; continue; } while (match .some(Value) = Candidate) {} for (var Item in Values) { Item; } for (const Index in Begin .. End) {} for (const _ in Values) {} return Local; defer cleanup(); defer { cleanup(); } throw; throw Failure; throw Failure from Cause; try {} catch Error as Failure {} catch as Remaining {} }", {CstKind::AssignmentStatement, CstKind::ExpressionStatement, CstKind::IfStatement, CstKind::MatchCondition, CstKind::MatchStatement, CstKind::MatchStatementArm, CstKind::WhileStatement, CstKind::WhileMatchCondition, CstKind::ForStatement, CstKind::ForBindingMode, CstKind::ForBindingPattern, CstKind::ForWildcardPattern, CstKind::ForRangeSource, CstKind::BreakStatement, CstKind::ContinueStatement, CstKind::ReturnStatement, CstKind::DeferStatement, CstKind::ThrowStatement, CstKind::ThrowCauseClause, CstKind::TryStatement, CstKind::TypedCatchClause, CstKind::CatchAllClause, CstKind::CatchBinding}};

      expectValidSyntax(TestCase);
    }

    // Verifies an if followed by a value expression at statement entry remains an expression statement rather than committing to block-form control flow.
    TEST(ParserStatementSyntaxTest, DistinguishesIfExpressionStatementsFromIfStatements)
    {
      const ParsedFile File = parseSource("func choose() { if (Ready) First else Second; }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(hasKind(File, CstKind::ExpressionStatement));
      EXPECT_TRUE(hasKind(File, CstKind::IfExpression));
      EXPECT_FALSE(hasKind(File, CstKind::IfStatement));
      expectFullFidelity(File);
    }

    // Verifies primary, unary, binary, conditional, match, postfix, aggregate, collection, and type-valued expressions.
    TEST(ParserExpressionSyntaxTest, ParsesExpressionFamiliesAndPostfixChains)
    {
      const ValidSyntaxCase TestCase = {"Expressions", "func expressions() { const Precedence = A + B * C << D & E ^ F | G && H || I; const Conditional = if (Ready) First else Second; const Matched = match (Value) { .some(Item) => Item, _ => Fallback, }; const Postfix = Object.method::<T>(Value, ...Arguments, name = Named)[Index][Low:High].field->next; const Aggregate = Record { First, Second: Value }; const Tuple = (1, 2, ...Items); const Array = [1, 2, 3]; const Unary = comptime await - + ! ~ * & Value; const Literals = (true, false, null, 1, 1.0, 'a', \"text\"); const Builtins = (i32, this); const FunctionType = func(i32, ...Types) -> bool; const ConstantType = const Data*; const Anonymous = class { var Value: i32; }; }", {CstKind::BinaryExpression, CstKind::IfExpression, CstKind::MatchExpression, CstKind::MatchExpressionArm, CstKind::CallExpression, CstKind::PositionalArgument, CstKind::NamedArgument, CstKind::ListExpansion, CstKind::BracketPostfixSuffix, CstKind::SliceExpression, CstKind::MemberExpression, CstKind::PointerMemberExpression, CstKind::GenericArgumentClause, CstKind::AggregateInitializationExpression, CstKind::AggregateFieldInitializer, CstKind::ArrayExpression, CstKind::ParenthesizedCommaList, CstKind::UnaryExpression, CstKind::ComptimeExpression, CstKind::LiteralExpression, CstKind::BuiltinTypeExpression, CstKind::ThisExpression, CstKind::FunctionTypeExpression, CstKind::ConstTypeValueExpression, CstKind::ClassTypeExpression}};

      expectValidSyntax(TestCase);
    }

    // Verifies explicit type syntax for qualifiers, tuples, functions, generics, arrays, slices, pointers, and references.
    TEST(ParserTypeSyntaxTest, ParsesTypeSyntaxFamilies)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"PostfixTypeForms", "func types(Value: const Map::<String, Data*>[Count][]&&) -> Result**;", {CstKind::TypeSyntax, CstKind::ConstTypeQualifier, CstKind::TypeName, CstKind::GenericArgumentClause, CstKind::BracketPostfixSuffix, CstKind::EmptyBracketTypeSuffix, CstKind::PointerTypeSuffix, CstKind::ReferenceTypeSuffix}},
          {"TupleAndFunctionTypes", "func consume(Handler: async func((i32, String), ...Types) -> const Result&, Pair: (i32, String));", {CstKind::FunctionType, CstKind::FunctionTypeParameter, CstKind::FunctionTypeResult, CstKind::ParenthesizedCommaList, CstKind::ListExpansion, CstKind::ConstTypeQualifier}},
          {"ParenthesizedFunctionPointer", "const Callback: type = (func(i32) -> bool)*;", {CstKind::ParenthesizedExpression, CstKind::FunctionTypeExpression, CstKind::TypeConstructorExpression, CstKind::PointerTypeSuffix}},
          {"ConstAsyncFunctionTypeValue", "const Callback = const async func() -> bool;", {CstKind::ConstTypeQualifier, CstKind::FunctionTypeExpression, CstKind::FunctionType}},
          {"PrefixedAnonymousClassValues", "const PublicClass = public class {}; const FinalClass = final class {};", {CstKind::ClassTypeExpression, CstKind::AccessModifier, CstKind::TypeModifier}},
          {"ParenthesizedGenericType", "func consume(Value: (Map::<String, Vector::<i32>>));", {CstKind::ParenthesizedTypeExpression, CstKind::GenericArgumentClause}},
      };

      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }

    // Verifies commas nested in generic argument clauses do not reclassify an enclosing parenthesized type as a tuple type.
    TEST(ParserTypeSyntaxTest, KeepsGenericArgumentCommasInsideParenthesizedTypes)
    {
      const ParsedFile File = parseSource("func consume(Value: (Map::<String, Vector::<i32>>), FunctionResult: (Wrapper::<func() -> A, B>), LiteralTarget: (1::<func() -> A, B>));");

      ASSERT_TRUE(File.succeeded());
      EXPECT_TRUE(hasKind(File, CstKind::ParenthesizedTypeExpression));
      EXPECT_FALSE(hasKind(File, CstKind::ParenthesizedCommaList));
      expectFullFidelity(File);
    }

    // Verifies each unified compile-time region control at top level and inside statement and member regions.
    TEST(ParserComptimeSyntaxTest, ParsesUnifiedComptimeRegionControls)
    {
      const std::vector<ValidSyntaxCase> Cases = {
          {"TopLevelBlock", "comptime { const Generated = 1; func helper(); }", {CstKind::ComptimeBlockControl, CstKind::TopLevelBlock, CstKind::TopLevelBindingDeclaration, CstKind::FunctionDeclaration}},
          {"TopLevelIf", "comptime if (Enabled) { const Active = 1; } else if (Fallback) { const Alternate = 2; } else { const Disabled = 0; }", {CstKind::ComptimeIfControl, CstKind::TopLevelBlock}},
          {"TopLevelMatch", "comptime match (Choice) { .some => { const Selected = 1; } _ => { const Default = 0; } }", {CstKind::ComptimeMatchControl, CstKind::RegionArm, CstKind::VariantPattern, CstKind::WildcardPattern}},
          {"TopLevelFor", "comptime for (const Item in Items) { const Generated = Item; }", {CstKind::ComptimeForControl, CstKind::ForBindingMode, CstKind::ForBindingPattern, CstKind::TopLevelBlock}},
          {"TopLevelWhile", "comptime while (Enabled) { const Generated = 1; }", {CstKind::ComptimeWhileControl, CstKind::TopLevelBlock}},
          {"StatementRegion", "func generated() { comptime { var Local = 1; } comptime if (Enabled) { Local; } else { return; } }", {CstKind::ComptimeBlockControl, CstKind::ComptimeIfControl, CstKind::StatementBlock, CstKind::LocalBindingDeclaration}},
          {"StatementStructuredControls", "func generated() { comptime match (Choice) { .some => { return; } _ => {} } comptime for (const Item in Items) { Item; } comptime while (Enabled) { break; } }", {CstKind::ComptimeMatchControl, CstKind::ComptimeForControl, CstKind::ComptimeWhileControl, CstKind::RegionArm, CstKind::StatementBlock}},
          {"ClassMemberRegion", "class Generated { comptime if (Enabled) { var Value: i32; } else { func fallback(); } }", {CstKind::ComptimeIfControl, CstKind::ClassMemberBlock, CstKind::FieldDeclaration, CstKind::FunctionDeclaration}},
          {"ClassStructuredControls", "class Generated { comptime match (Choice) { .some => { var Selected: i32; } _ => { func fallback(); } } comptime for (const Item in Items) { var Repeated: i32; } comptime while (Enabled) { func waiting(); } }", {CstKind::ComptimeMatchControl, CstKind::ComptimeForControl, CstKind::ComptimeWhileControl, CstKind::RegionArm, CstKind::ClassMemberBlock}},
          {"InterfaceRegion", "interface Generated { comptime { func Base(); } comptime if (Enabled) { func Active(); } else { func Inactive(); } comptime match (Choice) { .some => { func Selected(); } _ => { func Fallback(); } } comptime for (const Item in Items) { func Repeated(); } comptime while (Enabled) { func Waiting(); } }", {CstKind::ComptimeBlockControl, CstKind::ComptimeIfControl, CstKind::ComptimeMatchControl, CstKind::ComptimeForControl, CstKind::ComptimeWhileControl, CstKind::RegionArm, CstKind::InterfaceMemberBlock}},
          {"EnumRegion", "enum Generated { comptime { First, Second }, comptime if (Enabled) { Active } else { Inactive }, comptime match (Choice) { .some => { Selected } _ => { Fallback } }, comptime for (const Item in Items) { Repeated }, comptime while (Enabled) { Waiting } }", {CstKind::ComptimeBlockControl, CstKind::ComptimeIfControl, CstKind::ComptimeMatchControl, CstKind::ComptimeForControl, CstKind::ComptimeWhileControl, CstKind::RegionArm, CstKind::EnumMemberBlock, CstKind::EnumBranch}},
      };

      for (const ValidSyntaxCase &TestCase : Cases)
      {
        expectValidSyntax(TestCase);
      }
    }
  } // namespace
} // namespace ink::parser
