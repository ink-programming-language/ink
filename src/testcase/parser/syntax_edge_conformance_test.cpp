#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    using test::countKind;
    using test::expectFullFidelity;
    using test::nodesOfKind;
    using test::nodeText;
    using test::parseSource;

    std::vector<CstNodeId> directChildrenOfKind(const ParsedFile &File, CstNodeId Parent, CstKind Kind)
    {
      std::vector<CstNodeId> Result;
      const CstNode &ParentNode = File.cst().node(Parent);
      for (std::size_t Offset = 0; Offset < ParentNode.ChildCount; ++Offset)
      {
        const CstElement &Element = File.cst().children()[ParentNode.FirstChild + Offset];
        if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
        {
          if (File.cst().node(Child->Id).Kind == Kind)
          {
            Result.push_back(Child->Id);
          }
        }
      }
      return Result;
    }

    std::vector<CstNodeId> nodesOfKindWithText(const ParsedFile &File, CstKind Kind, const std::string &ExpectedText)
    {
      std::vector<CstNodeId> Result;
      for (CstNodeId Id : nodesOfKind(File, Kind))
      {
        if (nodeText(File, Id) == ExpectedText)
        {
          Result.push_back(Id);
        }
      }
      return Result;
    }

    bool isDescendant(const ParsedFile &File, CstNodeId Ancestor, CstNodeId Candidate)
    {
      std::vector<CstNodeId> Work = {Ancestor};
      while (!Work.empty())
      {
        const CstNodeId Parent = Work.back();
        Work.pop_back();
        const CstNode &ParentNode = File.cst().node(Parent);
        for (std::size_t Offset = 0; Offset < ParentNode.ChildCount; ++Offset)
        {
          const CstElement &Element = File.cst().children()[ParentNode.FirstChild + Offset];
          if (const CstNodeRef *Child = std::get_if<CstNodeRef>(&Element))
          {
            if (Child->Id == Candidate)
            {
              return true;
            }
            Work.push_back(Child->Id);
          }
        }
      }
      return false;
    }

    std::size_t countDescendantsOfKind(const ParsedFile &File, CstNodeId Ancestor, CstKind Kind)
    {
      std::size_t Result = 0;
      for (CstNodeId Candidate : nodesOfKind(File, Kind))
      {
        if (isDescendant(File, Ancestor, Candidate))
        {
          ++Result;
        }
      }
      return Result;
    }

    void expectSuccessfulCompleteParse(const ParsedFile &File)
    {
      EXPECT_TRUE(File.succeeded());
      EXPECT_TRUE(test::testDiagnostics(File).empty());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      expectFullFidelity(File);
    }

    // Verifies var tuple destructuring and the empty, singleton, standalone-expansion, and mixed-expansion tuple forms in value and type positions.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesTuplePatternValueAndTypeBoundaries)
    {
      const ParsedFile File = parseSource("func tuples() { var () = Empty; var (Only,) = Single; const EmptyValue = (); const SingleValue = (Only,); const ExpandedValue = (...Values); const MixedValue = (Head, ...Tail); } func tupleTypes(Empty: (), Single: (i32,), Expanded: (...Types), Mixed: (i32, ...Types));");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::TupleDestructuringDeclaration), 2u);
      EXPECT_EQ(countKind(File, CstKind::TuplePattern), 2u);
      EXPECT_EQ(countKind(File, CstKind::ParenthesizedCommaList), 8u);
      EXPECT_EQ(countKind(File, CstKind::ListExpansion), 4u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::TuplePattern, "()").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::TuplePattern, "(Only,)").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ParenthesizedCommaList, "()").size(), 2u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ParenthesizedCommaList, "(Only,)").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ParenthesizedCommaList, "(...Values)").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ParenthesizedCommaList, "(Head, ...Tail)").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ParenthesizedCommaList, "(i32,)").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ParenthesizedCommaList, "(...Types)").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ParenthesizedCommaList, "(i32, ...Types)").size(), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies all three slice forms with an omitted endpoint and both numeric tuple-position member selectors retain their exact postfix CST text.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesOmittedSliceEndpointsAndTuplePositionSelectors)
    {
      const ParsedFile File = parseSource("func postfixes() { const All = Values[:]; const Prefix = Values[:High]; const Suffix = Values[Low:]; const TupleMember = Pair.0; const TuplePointerMember = Pair->0; }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::SliceExpression), 3u);
      EXPECT_EQ(countKind(File, CstKind::MemberExpression), 1u);
      EXPECT_EQ(countKind(File, CstKind::PointerMemberExpression), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::SliceExpression, "Values[:]").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::SliceExpression, "Values[:High]").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::SliceExpression, "Values[Low:]").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::MemberExpression, "Pair.0").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::PointerMemberExpression, "Pair->0").size(), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies generic list expansion and generic-if arguments remain inside their own generic clauses before the following call suffixes.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesGenericExpansionAndGenericIfArguments)
    {
      const ParsedFile File = parseSource("func genericArguments() { const Expanded = Factory::<...Types>(); const Conditional = Factory::<if (Ready) Left else Right>(); const Mixed = Factory::<First, ...Rest>(); }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::GenericArgumentClause), 3u);
      EXPECT_EQ(countKind(File, CstKind::GenericArgument), 4u);
      EXPECT_EQ(countKind(File, CstKind::ListExpansion), 2u);
      EXPECT_EQ(countKind(File, CstKind::IfExpression), 1u);
      EXPECT_EQ(countKind(File, CstKind::CallExpression), 3u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::GenericArgumentClause, "Factory::<...Types>").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::IfExpression, "if (Ready) Left else Right").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::GenericArgumentClause, "Factory::<First, ...Rest>").size(), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies a match expression accepts a statement-block arm body while its sibling expression arm and mandatory arm commas remain distinct.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesMatchExpressionBlockArm)
    {
      const ParsedFile File = parseSource("const Result = match (Value) { .some(Item) => { return Item; }, _ => Fallback, };");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, CstKind::MatchExpression), 1u);
      ASSERT_EQ(countKind(File, CstKind::MatchExpressionArm), 2u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::MatchExpressionArm, ".some(Item) => { return Item; },").size(), 1u);
      const CstNodeId BlockArm = nodesOfKindWithText(File, CstKind::MatchExpressionArm, ".some(Item) => { return Item; },")[0];
      EXPECT_EQ(directChildrenOfKind(File, BlockArm, CstKind::StatementBlock).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, BlockArm, CstKind::NameExpression).size(), 0u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::MatchExpressionArm, "_ => Fallback,").size(), 1u);
      EXPECT_EQ(countKind(File, CstKind::ReturnStatement), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies empty array, aggregate-initializer, and enum bodies produce their dedicated nodes without fabricating elements, fields, or branches.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesEmptyArrayAggregateAndEnumForms)
    {
      const ParsedFile File = parseSource("enum Empty {} func emptyValues() { const Array = []; const Aggregate = Record {}; }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::EnumDeclaration), 1u);
      EXPECT_EQ(countKind(File, CstKind::EnumMemberBlock), 1u);
      EXPECT_EQ(countKind(File, CstKind::EnumBranch), 0u);
      EXPECT_EQ(countKind(File, CstKind::ArrayExpression), 1u);
      EXPECT_EQ(countKind(File, CstKind::AggregateInitializationExpression), 1u);
      EXPECT_EQ(countKind(File, CstKind::AggregateFieldInitializer), 0u);
      EXPECT_EQ(countKind(File, CstKind::AggregateFieldShorthand), 0u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::EnumDeclaration, "enum Empty {}").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ArrayExpression, "[]").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::AggregateInitializationExpression, "Record {}").size(), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies an empty aggregate initializer remains the base of a following ordinary member postfix instead of ending the expression early.
    TEST(ParserSyntaxEdgeConformanceTest, ContinuesPostfixParsingAfterEmptyAggregateInitialization)
    {
      const ParsedFile File = parseSource("const Member = Record {}.member;");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::AggregateInitializationExpression, "Record {}").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::MemberExpression, "Record {}.member").size(), 1u);
      const CstNodeId Aggregate = nodesOfKindWithText(File, CstKind::AggregateInitializationExpression, "Record {}")[0];
      const CstNodeId Member = nodesOfKindWithText(File, CstKind::MemberExpression, "Record {}.member")[0];
      EXPECT_EQ(directChildrenOfKind(File, Member, CstKind::AggregateInitializationExpression), (std::vector<CstNodeId>{Aggregate}));
      EXPECT_EQ(countKind(File, CstKind::AggregateFieldInitializer), 0u);
      EXPECT_EQ(countKind(File, CstKind::AggregateFieldShorthand), 0u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies typed-only, multiple-typed, catch-all-only, bound, unbound, and typed-then-catch-all sequences, including direct ownership by one try statement.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesEveryLegalCatchSequenceAndPreservesOwnership)
    {
      const ParsedFile File = parseSource("func catches() { try {} catch Error {} try {} catch Error as First {} catch Network {} try {} catch {} try {} catch as Remaining {} try {} catch First as A {} catch Second {} catch as Rest {} }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::TryStatement), 5u);
      EXPECT_EQ(countKind(File, CstKind::TypedCatchClause), 5u);
      EXPECT_EQ(countKind(File, CstKind::CatchAllClause), 3u);
      EXPECT_EQ(countKind(File, CstKind::CatchBinding), 4u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::TryStatement, "try {} catch First as A {} catch Second {} catch as Rest {}").size(), 1u);
      const CstNodeId CompleteSequence = nodesOfKindWithText(File, CstKind::TryStatement, "try {} catch First as A {} catch Second {} catch as Rest {}")[0];
      EXPECT_EQ(directChildrenOfKind(File, CompleteSequence, CstKind::StatementBlock).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, CompleteSequence, CstKind::TypedCatchClause).size(), 2u);
      EXPECT_EQ(directChildrenOfKind(File, CompleteSequence, CstKind::CatchAllClause).size(), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies a decorator declaration can use attributes, modifiers, generics, parameters, a receiver qualifier, a result, and a semicolon body while explicit empty application clauses remain real CST nodes.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesCompleteDecoratorSkeletonAndExplicitEmptyApplications)
    {
      const ParsedFile File = parseSource("[meta()] public static decorator trace<T: type>(Value: T) const -> bool; [nothrow()] @trace() func run();");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, CstKind::DecoratorDeclaration), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::DecoratorDeclaration, "[meta()] public static decorator trace<T: type>(Value: T) const -> bool;").size(), 1u);
      const CstNodeId Decorator = nodesOfKindWithText(File, CstKind::DecoratorDeclaration, "[meta()] public static decorator trace<T: type>(Value: T) const -> bool;")[0];
      EXPECT_EQ(directChildrenOfKind(File, Decorator, CstKind::AttributeList).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Decorator, CstKind::FunctionModifier).size(), 2u);
      EXPECT_EQ(directChildrenOfKind(File, Decorator, CstKind::GenericParameterClause).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Decorator, CstKind::FunctionParameterClause).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Decorator, CstKind::ReceiverQualifier).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Decorator, CstKind::ReturnClause).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Decorator, CstKind::FunctionDefinition).size(), 0u);
      EXPECT_EQ(countKind(File, CstKind::ApplicationArgumentClause), 3u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ApplicationArgumentClause, "()").size(), 3u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies every remaining access and function-modifier grammar branch is accepted and retained directly by one declaration.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesProtectedPrivateVirtualOverrideFinalAndImplicitFunctionModifiers)
    {
      const ParsedFile File = parseSource("protected private virtual override final implicit func modifiers();");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, CstKind::FunctionDeclaration), 1u);
      const CstNodeId Function = nodesOfKind(File, CstKind::FunctionDeclaration)[0];
      EXPECT_EQ(directChildrenOfKind(File, Function, CstKind::FunctionModifier).size(), 6u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::FunctionModifier, "protected").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::FunctionModifier, "private").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::FunctionModifier, "virtual").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::FunctionModifier, "override").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::FunctionModifier, "final").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::FunctionModifier, "implicit").size(), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies a generic interface owns nested generic class and interface declarations plus an empty enum through its member block.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesGenericInterfaceWithNestedTypes)
    {
      const ParsedFile File = parseSource("interface Container<T: type> { class Nested<U: type> {} interface Contract<V: type> {} enum Empty {} }");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::InterfaceDeclaration, "interface Container<T: type> { class Nested<U: type> {} interface Contract<V: type> {} enum Empty {} }").size(), 1u);
      const CstNodeId OuterInterface = nodesOfKindWithText(File, CstKind::InterfaceDeclaration, "interface Container<T: type> { class Nested<U: type> {} interface Contract<V: type> {} enum Empty {} }")[0];
      ASSERT_EQ(directChildrenOfKind(File, OuterInterface, CstKind::InterfaceMemberBlock).size(), 1u);
      const CstNodeId MemberBlock = directChildrenOfKind(File, OuterInterface, CstKind::InterfaceMemberBlock)[0];
      EXPECT_EQ(directChildrenOfKind(File, OuterInterface, CstKind::GenericParameterClause).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, MemberBlock, CstKind::ClassDeclaration).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, MemberBlock, CstKind::InterfaceDeclaration).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, MemberBlock, CstKind::EnumDeclaration).size(), 1u);
      EXPECT_EQ(countKind(File, CstKind::GenericParameterClause), 3u);
      EXPECT_EQ(countKind(File, CstKind::EnumBranch), 0u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies generic suffixes are retained inside both simple and qualified constructor-initializer targets before their call arguments.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesGenericConstructorInitializerTargets)
    {
      const ParsedFile File = parseSource("func initialize<T: type>(Value: T) : Base::<T>(Value), module.Storage::<T>(Value) {}");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, CstKind::ConstructorInitializerClause), 1u);
      ASSERT_EQ(countKind(File, CstKind::ConstructorInitializer), 2u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::ConstructorInitializerTarget, "Base::<T>").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::ConstructorInitializerTarget, "module.Storage::<T>").size(), 1u);
      const CstNodeId SimpleTarget = nodesOfKindWithText(File, CstKind::ConstructorInitializerTarget, "Base::<T>")[0];
      const CstNodeId QualifiedTarget = nodesOfKindWithText(File, CstKind::ConstructorInitializerTarget, "module.Storage::<T>")[0];
      EXPECT_EQ(directChildrenOfKind(File, SimpleTarget, CstKind::GenericArgumentList).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, QualifiedTarget, CstKind::GenericArgumentList).size(), 1u);
      EXPECT_EQ(countKind(File, CstKind::PositionalArgument), 2u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies a direct function type may omit its result and that parentheses explicitly permit a pointer suffix on the same function type value.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesResultlessDirectFunctionTypeAndParenthesizedPointer)
    {
      const ParsedFile File = parseSource("func typeValues() { const Direct = func(); const Pointer = (func())*; }");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, CstKind::FunctionTypeExpression), 2u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::TypeConstructorExpression, "(func())*").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::NamedBindingDeclaration, "const Direct = func();").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::NamedBindingDeclaration, "const Pointer = (func())*;").size(), 1u);
      const CstNodeId DirectBinding = nodesOfKindWithText(File, CstKind::NamedBindingDeclaration, "const Direct = func();")[0];
      const CstNodeId PointerBinding = nodesOfKindWithText(File, CstKind::NamedBindingDeclaration, "const Pointer = (func())*;")[0];
      EXPECT_EQ(countDescendantsOfKind(File, DirectBinding, CstKind::FunctionTypeExpression), 1u);
      EXPECT_EQ(countDescendantsOfKind(File, DirectBinding, CstKind::TypeConstructorExpression), 0u);
      EXPECT_EQ(countDescendantsOfKind(File, PointerBinding, CstKind::FunctionTypeExpression), 1u);
      EXPECT_EQ(countDescendantsOfKind(File, PointerBinding, CstKind::TypeConstructorExpression), 1u);
      EXPECT_EQ(countDescendantsOfKind(File, PointerBinding, CstKind::PointerTypeSuffix), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies explicit type syntax accepts the neutral call, member, pointer-member, and slice postfix nodes without reclassifying their spelling.
    TEST(ParserSyntaxEdgeConformanceTest, ParsesNeutralPostfixesInExplicitTypeContexts)
    {
      const ParsedFile File = parseSource("func postfixTypes(Call: Factory(), Member: Namespace.Type, PointerMember: Handle->Type, Slice: Values[:]);");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::TypeSyntax, "Factory()").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::TypeSyntax, "Namespace.Type").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::TypeSyntax, "Handle->Type").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::TypeSyntax, "Values[:]").size(), 1u);
      const CstNodeId CallType = nodesOfKindWithText(File, CstKind::TypeSyntax, "Factory()")[0];
      const CstNodeId MemberType = nodesOfKindWithText(File, CstKind::TypeSyntax, "Namespace.Type")[0];
      const CstNodeId PointerMemberType = nodesOfKindWithText(File, CstKind::TypeSyntax, "Handle->Type")[0];
      const CstNodeId SliceType = nodesOfKindWithText(File, CstKind::TypeSyntax, "Values[:]")[0];
      EXPECT_EQ(countDescendantsOfKind(File, CallType, CstKind::CallExpression), 1u);
      EXPECT_EQ(countDescendantsOfKind(File, MemberType, CstKind::MemberExpression), 1u);
      EXPECT_EQ(countDescendantsOfKind(File, PointerMemberType, CstKind::PointerMemberExpression), 1u);
      EXPECT_EQ(countDescendantsOfKind(File, SliceType, CstKind::SliceExpression), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies every mandatory terminal type-tail disambiguation from issue 30 commits pointer and reference tails while preserving the parenthesized empty-array multiplication control case.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesMandatoryTerminalTypeTailDisambiguationMatrix)
    {
      const ParsedFile File = parseSource("func tails() { return T*[]; return T*[N]; return T&[]; return T&[N]; return T * ([]); }");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::ReturnStatement), 5u);
      EXPECT_EQ(countKind(File, CstKind::TypeConstructorExpression), 4u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::TypeConstructorExpression, "T*[]").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::TypeConstructorExpression, "T*[N]").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::TypeConstructorExpression, "T&[]").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::TypeConstructorExpression, "T&[N]").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::BinaryExpression, "T * ([])").size(), 1u);
      EXPECT_EQ(nodesOfKindWithText(File, CstKind::ArrayExpression, "[]").size(), 1u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies the complete postfix-through-if precedence chain, unary right associativity, and repeated binary left associativity through exact CST parent-child relationships.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesCompleteExpressionPrecedenceAndAssociativity)
    {
      const ParsedFile File = parseSource("const Ordered = if (Condition) A || B && C == D | E ^ F & G << H + I * -J.member() else K; const BinaryLeft = A - B - C; const UnaryRight = !~-Value;");
      const std::vector<std::string> OrderedBinaryTexts = {
          "A || B && C == D | E ^ F & G << H + I * -J.member()",
          "B && C == D | E ^ F & G << H + I * -J.member()",
          "C == D | E ^ F & G << H + I * -J.member()",
          "D | E ^ F & G << H + I * -J.member()",
          "E ^ F & G << H + I * -J.member()",
          "F & G << H + I * -J.member()",
          "G << H + I * -J.member()",
          "H + I * -J.member()",
          "I * -J.member()",
      };

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, CstKind::BinaryExpression), 11u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::IfExpression, "if (Condition) A || B && C == D | E ^ F & G << H + I * -J.member() else K").size(), 1u);
      const CstNodeId IfExpression = nodesOfKindWithText(File, CstKind::IfExpression, "if (Condition) A || B && C == D | E ^ F & G << H + I * -J.member() else K")[0];
      for (std::size_t Index = 0; Index < OrderedBinaryTexts.size(); ++Index)
      {
        SCOPED_TRACE(OrderedBinaryTexts[Index]);
        ASSERT_EQ(nodesOfKindWithText(File, CstKind::BinaryExpression, OrderedBinaryTexts[Index]).size(), 1u);
        if (Index == 0)
        {
          EXPECT_EQ(directChildrenOfKind(File, IfExpression, CstKind::BinaryExpression), nodesOfKindWithText(File, CstKind::BinaryExpression, OrderedBinaryTexts[Index]));
        }
        if (Index + 1 < OrderedBinaryTexts.size())
        {
          const CstNodeId Parent = nodesOfKindWithText(File, CstKind::BinaryExpression, OrderedBinaryTexts[Index])[0];
          EXPECT_EQ(directChildrenOfKind(File, Parent, CstKind::BinaryExpression), nodesOfKindWithText(File, CstKind::BinaryExpression, OrderedBinaryTexts[Index + 1]));
        }
      }
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::UnaryExpression, "-J.member()").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::CallExpression, "J.member()").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::MemberExpression, "J.member").size(), 1u);
      const CstNodeId Multiplication = nodesOfKindWithText(File, CstKind::BinaryExpression, OrderedBinaryTexts.back())[0];
      const CstNodeId Minus = nodesOfKindWithText(File, CstKind::UnaryExpression, "-J.member()")[0];
      const CstNodeId Call = nodesOfKindWithText(File, CstKind::CallExpression, "J.member()")[0];
      const CstNodeId Member = nodesOfKindWithText(File, CstKind::MemberExpression, "J.member")[0];
      EXPECT_EQ(directChildrenOfKind(File, Multiplication, CstKind::UnaryExpression), (std::vector<CstNodeId>{Minus}));
      EXPECT_EQ(directChildrenOfKind(File, Minus, CstKind::CallExpression), (std::vector<CstNodeId>{Call}));
      EXPECT_EQ(directChildrenOfKind(File, Call, CstKind::MemberExpression), (std::vector<CstNodeId>{Member}));
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::BinaryExpression, "A - B - C").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::BinaryExpression, "A - B").size(), 1u);
      const CstNodeId FullSubtraction = nodesOfKindWithText(File, CstKind::BinaryExpression, "A - B - C")[0];
      const CstNodeId LeftSubtraction = nodesOfKindWithText(File, CstKind::BinaryExpression, "A - B")[0];
      EXPECT_EQ(directChildrenOfKind(File, FullSubtraction, CstKind::BinaryExpression), (std::vector<CstNodeId>{LeftSubtraction}));
      EXPECT_TRUE(nodesOfKindWithText(File, CstKind::BinaryExpression, "B - C").empty());
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::UnaryExpression, "!~-Value").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::UnaryExpression, "~-Value").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::UnaryExpression, "-Value").size(), 1u);
      const CstNodeId LogicalNot = nodesOfKindWithText(File, CstKind::UnaryExpression, "!~-Value")[0];
      const CstNodeId BitwiseNot = nodesOfKindWithText(File, CstKind::UnaryExpression, "~-Value")[0];
      const CstNodeId ArithmeticMinus = nodesOfKindWithText(File, CstKind::UnaryExpression, "-Value")[0];
      EXPECT_EQ(directChildrenOfKind(File, LogicalNot, CstKind::UnaryExpression), (std::vector<CstNodeId>{BitwiseNot}));
      EXPECT_EQ(directChildrenOfKind(File, BitwiseNot, CstKind::UnaryExpression), (std::vector<CstNodeId>{ArithmeticMinus}));
      expectSuccessfulCompleteParse(File);
    }

    // Verifies attributes, decorators, modifiers, generic parameters, and function parameters all remain owned by the same function declaration.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesFunctionPrefixAndClauseOwnership)
    {
      const ParsedFile File = parseSource("[reflect()] @trace() public extern \"C\" static async func load<T: type>(Value: T) const -> Result;");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, CstKind::FunctionDeclaration), 1u);
      const CstNodeId Function = nodesOfKind(File, CstKind::FunctionDeclaration)[0];
      EXPECT_EQ(directChildrenOfKind(File, Function, CstKind::AttributeList).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Function, CstKind::DecoratorApplication).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Function, CstKind::FunctionModifier).size(), 3u);
      EXPECT_EQ(directChildrenOfKind(File, Function, CstKind::ExternModifier).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Function, CstKind::GenericParameterClause).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, Function, CstKind::FunctionParameterClause).size(), 1u);
      ASSERT_EQ(countKind(File, CstKind::GenericParameter), 1u);
      ASSERT_EQ(countKind(File, CstKind::FunctionParameter), 1u);
      EXPECT_TRUE(isDescendant(File, Function, nodesOfKind(File, CstKind::GenericParameter)[0]));
      EXPECT_TRUE(isDescendant(File, Function, nodesOfKind(File, CstKind::FunctionParameter)[0]));
      EXPECT_EQ(nodeText(File, Function), "[reflect()] @trace() public extern \"C\" static async func load<T: type>(Value: T) const -> Result;");
      expectSuccessfulCompleteParse(File);
    }

    // Verifies else-if is represented as a nested if directly owned by the outer if while the terminal else block belongs only to the nested if.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesElseIfOwnership)
    {
      const ParsedFile File = parseSource("func choose() { if (First) {} else if (Second) {} else {} }");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::IfStatement, "if (First) {} else if (Second) {} else {}").size(), 1u);
      ASSERT_EQ(nodesOfKindWithText(File, CstKind::IfStatement, "if (Second) {} else {}").size(), 1u);
      const CstNodeId OuterIf = nodesOfKindWithText(File, CstKind::IfStatement, "if (First) {} else if (Second) {} else {}")[0];
      const CstNodeId NestedIf = nodesOfKindWithText(File, CstKind::IfStatement, "if (Second) {} else {}")[0];
      EXPECT_EQ(directChildrenOfKind(File, OuterIf, CstKind::IfStatement), (std::vector<CstNodeId>{NestedIf}));
      EXPECT_EQ(directChildrenOfKind(File, OuterIf, CstKind::StatementBlock).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, NestedIf, CstKind::StatementBlock).size(), 2u);
      expectSuccessfulCompleteParse(File);
    }

    // Verifies all five top-level comptime controls directly own their region blocks and match arms own exactly their corresponding blocks.
    TEST(ParserSyntaxEdgeConformanceTest, PreservesComptimeControlRegionOwnership)
    {
      const ParsedFile File = parseSource("comptime { const BlockValue = 1; } comptime if (Enabled) { const IfValue = 1; } else { const ElseValue = 0; } comptime match (Choice) { .some => { const SomeValue = 1; } _ => { const OtherValue = 0; } } comptime for (const Item in Items) { const ForValue = Item; } comptime while (Enabled) { const WhileValue = 1; }");

      ASSERT_TRUE(File.succeeded());
      ASSERT_EQ(countKind(File, CstKind::ComptimeBlockControl), 1u);
      ASSERT_EQ(countKind(File, CstKind::ComptimeIfControl), 1u);
      ASSERT_EQ(countKind(File, CstKind::ComptimeMatchControl), 1u);
      ASSERT_EQ(countKind(File, CstKind::ComptimeForControl), 1u);
      ASSERT_EQ(countKind(File, CstKind::ComptimeWhileControl), 1u);
      const CstNodeId BlockControl = nodesOfKind(File, CstKind::ComptimeBlockControl)[0];
      const CstNodeId IfControl = nodesOfKind(File, CstKind::ComptimeIfControl)[0];
      const CstNodeId MatchControl = nodesOfKind(File, CstKind::ComptimeMatchControl)[0];
      const CstNodeId ForControl = nodesOfKind(File, CstKind::ComptimeForControl)[0];
      const CstNodeId WhileControl = nodesOfKind(File, CstKind::ComptimeWhileControl)[0];
      EXPECT_EQ(directChildrenOfKind(File, BlockControl, CstKind::TopLevelBlock).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, IfControl, CstKind::TopLevelBlock).size(), 2u);
      EXPECT_EQ(directChildrenOfKind(File, MatchControl, CstKind::TopLevelBlock).size(), 0u);
      EXPECT_EQ(directChildrenOfKind(File, ForControl, CstKind::TopLevelBlock).size(), 1u);
      EXPECT_EQ(directChildrenOfKind(File, WhileControl, CstKind::TopLevelBlock).size(), 1u);
      ASSERT_EQ(directChildrenOfKind(File, MatchControl, CstKind::RegionArm).size(), 2u);
      for (CstNodeId Arm : directChildrenOfKind(File, MatchControl, CstKind::RegionArm))
      {
        EXPECT_EQ(directChildrenOfKind(File, Arm, CstKind::TopLevelBlock).size(), 1u);
      }
      expectSuccessfulCompleteParse(File);
    }
  } // namespace
} // namespace ink::parser
