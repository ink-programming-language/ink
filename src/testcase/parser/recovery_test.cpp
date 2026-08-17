#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace ink::parser
{
  namespace
  {
    using test::countKind;
    using test::expectFullFidelity;
    using test::hasDiagnostic;
    using test::hasKind;
    using test::missingTokens;
    using test::nodeTextsOfKind;
    using test::parseSource;

    struct MissingSyntaxCase
    {
      const char *Name;
      const char *Source;
      const char *ExpectedSpelling;
    };

    struct RegionMatchRecoveryCase
    {
      const char *Name;
      const char *Source;
      CstKind BlockKind;
    };

    struct StructuredMatchRecoveryCase
    {
      const char *Name;
      const char *Source;
      CstKind ArmKind;
    };

    struct InheritanceRecoveryCase
    {
      const char *Name;
      const char *Source;
      CstKind OwnerKind;
      CstKind BlockKind;
    };

    // Verifies missing delimiters, terminators, and required clauses become zero-width MissingToken leaves with diagnostics.
    TEST(ParserRecoveryTest, SynthesizesMissingTokensAtStableAnchors)
    {
      const std::vector<MissingSyntaxCase> Cases = {
          {"ParameterCloser", "func broken(Value: i32 { return Value; }", ")"},
          {"StatementTerminator", "func broken() { return Value }", ";"},
          {"RequiredCatch", "func broken() { try {} }", "catch"},
      };

      for (const MissingSyntaxCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile File = parseSource(TestCase.Source);
        const std::vector<MissingToken> Missing = missingTokens(File);

        ASSERT_FALSE(File.succeeded());
        EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
        EXPECT_TRUE(hasDiagnostic(File, core::DiagnosticKind::ExpectedToken));
        EXPECT_TRUE(hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasMissing));
        EXPECT_TRUE(std::any_of(Missing.begin(), Missing.end(), [TestCase](const MissingToken &Token)
                                {
                                  return Token.ExpectedSpelling == TestCase.ExpectedSpelling;
                                }));
        for (const MissingToken &Token : Missing)
        {
          EXPECT_LE(Token.AnchorByteOffset, File.lexedFile().source().size());
        }
        expectFullFidelity(File);
      }
    }

    struct ErrorRecoveryCase
    {
      const char *Name;
      const char *Source;
      core::DiagnosticKind ExpectedDiagnostic;
      CstKind RecoveredKind;
    };

    // Verifies unexpected tokens, reserved sequences, trailing commas, and declaration-placement errors recover into Error nodes and continue parsing.
    TEST(ParserRecoveryTest, PreservesUnexpectedTokensAndContinuesAtGrammarBoundaries)
    {
      const std::vector<ErrorRecoveryCase> Cases = {
          {"UnexpectedTopLevelToken", "; const After = 1;", core::DiagnosticKind::UnexpectedToken, CstKind::TopLevelBindingDeclaration},
          {"ImportTrailingComma", "from core.io import File,; const After = 1;", core::DiagnosticKind::TrailingComma, CstKind::TopLevelBindingDeclaration},
          {"ReservedUnarySequence", "func broken() { ++Value; return; }", core::DiagnosticKind::ReservedSymbolSequence, CstKind::ReturnStatement},
          {"DeclarationNeedsArmBlock", "func broken() { match (Value) { _ => var Local = 1; } return; }", core::DiagnosticKind::DeclarationRequiresBlock, CstKind::ReturnStatement},
          {"ChainedComparison", "func broken() { Left < Middle < Right; return; }", core::DiagnosticKind::UnexpectedToken, CstKind::ReturnStatement},
      };

      for (const ErrorRecoveryCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile File = parseSource(TestCase.Source);

        ASSERT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, TestCase.ExpectedDiagnostic));
        EXPECT_TRUE(hasKind(File, CstKind::Error));
        EXPECT_TRUE(hasKind(File, TestCase.RecoveredKind));
        EXPECT_TRUE(hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasError));
        expectFullFidelity(File);
      }
    }

    // Verifies trailing-comma rejection across representative nonempty comma-list grammars without losing following source.
    TEST(ParserRecoveryTest, RejectsForbiddenTrailingCommasInRepresentativeLists)
    {
      const std::vector<std::string> Sources = {
          "from core.io import File,;",
          "func generic<T: type,>();",
          "func parameters(Value: i32,);",
          "func calls() { call(Value,); }",
          "const Generic = Box::<i32,>;",
          "const Aggregate = Record { Field: Value, };",
          "enum Kind { First, }",
      };

      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = parseSource(Source);

        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, core::DiagnosticKind::TrailingComma));
        EXPECT_TRUE(hasKind(File, CstKind::Error));
        expectFullFidelity(File);
      }
    }

    // Verifies leading and repeated commas are retained in Error nodes across representative list grammars and recovery reaches the following declaration.
    TEST(ParserRecoveryTest, PreservesLeadingAndRepeatedCommasAcrossListGrammars)
    {
      const std::vector<std::string> Sources = {
          "from core.io import ,File,,Path; const After = 1;",
          "[,reflect,,serialize] func attributes() {} const After = 1;",
          "func generic<,T: type,,U: type>(); const After = 1;",
          "func parameters(,First: i32,,Second: i32) {} const After = 1;",
          "func calls() { call(,First,,Second); } const After = 1;",
          "const Array = [,First,,Second]; const After = 1;",
          "const Generic = Box::<,i32,,u32>; const After = 1;",
          "const Aggregate = Record { ,First: 1,,Second: 2 }; const After = 1;",
          "enum Kind { ,First,,Second } const After = 1;",
      };

      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = parseSource(Source);

        EXPECT_FALSE(File.succeeded());
        EXPECT_TRUE(hasDiagnostic(File, core::DiagnosticKind::UnexpectedToken));
        EXPECT_TRUE(hasKind(File, CstKind::Error));
        EXPECT_TRUE(hasKind(File, CstKind::TopLevelBindingDeclaration));
        expectFullFidelity(File);
      }
    }

    struct InteractiveCase
    {
      const char *Name;
      const char *Source;
    };

    // Verifies EOF in a construct that can be completed by more input is Incomplete interactively but remains Complete in batch mode.
    TEST(ParserInteractiveTest, DistinguishesIncompleteInputFromBatchSyntaxFailure)
    {
      const std::vector<InteractiveCase> Cases = {
          {"AttributeList", "[pending"},
          {"DecoratorArguments", "@pending("},
          {"ExternModifier", "extern"},
          {"ParameterList", "func pending("},
          {"ClassBody", "class Pending {"},
          {"ReturnExpression", "func pending() { return"},
          {"ParenthesizedExpression", "const Pending = ("},
          {"ModulePath", "import core."},
      };

      for (const InteractiveCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile Interactive = parseSource(TestCase.Source, ParserOptions{ParseMode::Interactive});
        const ParsedFile Batch = parseSource(TestCase.Source, ParserOptions{ParseMode::Batch});

        EXPECT_FALSE(Interactive.succeeded());
        EXPECT_EQ(Interactive.completeness(), ParseCompleteness::Incomplete);
        EXPECT_FALSE(Batch.succeeded());
        EXPECT_EQ(Batch.completeness(), ParseCompleteness::Complete);
        expectFullFidelity(Interactive);
        expectFullFidelity(Batch);
      }
    }

    // Verifies explicit conflicting delimiters and declaration terminators are completed syntax errors rather than inputs that more source can close.
    TEST(ParserInteractiveTest, ExplicitConflictsRemainComplete)
    {
      const std::vector<std::string> Sources = {
          "[pending)",
          "class Pending;",
          "interface Pending;",
          "enum Pending;",
          "func pending(value i32",
      };

      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = parseSource(Source, ParserOptions{ParseMode::Interactive});

        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
        EXPECT_TRUE(hasKind(File, CstKind::Error) || hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasMissing));
        expectFullFidelity(File);
      }
    }

    // Verifies misplaced annotations, duplicate receiver qualifiers, and handlers after catch-all stay inside their committed declaration or try structure.
    TEST(ParserRecoveryTest, PreservesCommittedRecoveryShapes)
    {
      const ParsedFile MisplacedAnnotation = parseSource("public [reflect] func annotated() {}");
      const ParsedFile DuplicateQualifier = parseSource("func qualified() const const {}");
      const ParsedFile CatchAfterCatchAll = parseSource("func handled() { try {} catch {} catch Failure {} }");
      const ParsedFile MisplacedTypeAnnotation = parseSource("public [reflect] class Annotated {}");
      const ParsedFile InvalidTypeDecorator = parseSource("@trace class Decorated {}");
      const ParsedFile MisplacedFieldAnnotation = parseSource("class Holder { public [reflect] var Value: i32; }");
      const ParsedFile InvalidFieldModifier = parseSource("class Holder { static var Value: i32; }");
      const ParsedFile InvalidBindingAnnotation = parseSource("[reflect] public const Value = 1;");

      EXPECT_FALSE(MisplacedAnnotation.succeeded());
      EXPECT_EQ(countKind(MisplacedAnnotation, CstKind::FunctionDeclaration), 1u);
      EXPECT_EQ(countKind(MisplacedAnnotation, CstKind::StatementBlock), 1u);
      EXPECT_TRUE(hasKind(MisplacedAnnotation, CstKind::Error));
      EXPECT_FALSE(DuplicateQualifier.succeeded());
      EXPECT_EQ(countKind(DuplicateQualifier, CstKind::FunctionDeclaration), 1u);
      EXPECT_EQ(countKind(DuplicateQualifier, CstKind::StatementBlock), 1u);
      EXPECT_TRUE(hasKind(DuplicateQualifier, CstKind::Error));
      EXPECT_FALSE(CatchAfterCatchAll.succeeded());
      EXPECT_EQ(countKind(CatchAfterCatchAll, CstKind::TypedCatchClause), 1u);
      EXPECT_EQ(countKind(CatchAfterCatchAll, CstKind::CatchAllClause), 1u);
      EXPECT_TRUE(hasKind(CatchAfterCatchAll, CstKind::Error));
      EXPECT_FALSE(MisplacedTypeAnnotation.succeeded());
      EXPECT_EQ(countKind(MisplacedTypeAnnotation, CstKind::ClassDeclaration), 1u);
      EXPECT_TRUE(hasKind(MisplacedTypeAnnotation, CstKind::Error));
      EXPECT_FALSE(InvalidTypeDecorator.succeeded());
      EXPECT_EQ(countKind(InvalidTypeDecorator, CstKind::ClassDeclaration), 1u);
      EXPECT_TRUE(hasKind(InvalidTypeDecorator, CstKind::Error));
      EXPECT_FALSE(MisplacedFieldAnnotation.succeeded());
      EXPECT_EQ(countKind(MisplacedFieldAnnotation, CstKind::FieldDeclaration), 1u);
      EXPECT_TRUE(hasKind(MisplacedFieldAnnotation, CstKind::Error));
      EXPECT_FALSE(InvalidFieldModifier.succeeded());
      EXPECT_EQ(countKind(InvalidFieldModifier, CstKind::FieldDeclaration), 1u);
      EXPECT_TRUE(hasKind(InvalidFieldModifier, CstKind::Error));
      EXPECT_FALSE(InvalidBindingAnnotation.succeeded());
      EXPECT_EQ(countKind(InvalidBindingAnnotation, CstKind::TopLevelBindingDeclaration), 1u);
      EXPECT_TRUE(hasKind(InvalidBindingAnnotation, CstKind::Error));
      expectFullFidelity(MisplacedAnnotation);
      expectFullFidelity(DuplicateQualifier);
      expectFullFidelity(CatchAfterCatchAll);
      expectFullFidelity(MisplacedTypeAnnotation);
      expectFullFidelity(InvalidTypeDecorator);
      expectFullFidelity(MisplacedFieldAnnotation);
      expectFullFidelity(InvalidFieldModifier);
      expectFullFidelity(InvalidBindingAnnotation);
    }

    // Verifies a missing inheritance comma is synthesized before the next type while the real member block remains owned by the declaration or class expression.
    TEST(ParserRecoveryTest, SynchronizesInheritanceListsAtFollowingTypeStarts)
    {
      const std::vector<InheritanceRecoveryCase> Cases = {
          {"ClassDeclaration", "class Derived : First Second {}", CstKind::ClassDeclaration, CstKind::ClassMemberBlock},
          {"InterfaceDeclaration", "interface Derived : First Second {}", CstKind::InterfaceDeclaration, CstKind::InterfaceMemberBlock},
          {"ClassTypeExpression", "const Derived = class : First Second {};", CstKind::ClassTypeExpression, CstKind::ClassMemberBlock},
      };

      for (const InheritanceRecoveryCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile File = parseSource(TestCase.Source);
        const std::vector<std::string> InheritanceClauses = nodeTextsOfKind(File, CstKind::InheritanceClause);
        const std::vector<MissingToken> Missing = missingTokens(File);
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(countKind(File, TestCase.OwnerKind), 1u);
        EXPECT_EQ(countKind(File, TestCase.BlockKind), 1u);
        ASSERT_EQ(InheritanceClauses.size(), 1u);
        EXPECT_NE(InheritanceClauses.front().find("First"), std::string::npos);
        EXPECT_NE(InheritanceClauses.front().find("Second"), std::string::npos);
        EXPECT_TRUE(std::any_of(Missing.begin(), Missing.end(), [](const MissingToken &Token)
                                {
                                  return Token.ExpectedSpelling == ",";
                                }));
        expectFullFidelity(File);
      }
    }

    // Verifies the array-versus-class-expression checkpoint commits only after complete attribute syntax and restores every speculative CST mutation on rollback.
    TEST(ParserRecoveryTest, CommitsClassTypeExpressionsOnlyAfterCompleteAttributePrefixes)
    {
      const std::vector<std::string> InvalidPrefixes = {
          "[]",
          "[attribute,]",
          "[attribute,,other]",
          "[attribute(]",
          "[attribute(...)]",
          "[attribute(name = 1, 2)]",
      };

      for (const std::string &Prefix : InvalidPrefixes)
      {
        SCOPED_TRACE(Prefix);
        const ParsedFile File = parseSource("const Value = " + Prefix + " class {};");
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(countKind(File, CstKind::ArrayExpression), 1u);
        EXPECT_EQ(countKind(File, CstKind::ClassTypeExpression), 0u);
        expectFullFidelity(File);
      }

      const ParsedFile Valid = parseSource("const Value = [attribute(Level)] public final class {};");
      ASSERT_TRUE(Valid.succeeded());
      EXPECT_EQ(countKind(Valid, CstKind::ClassTypeExpression), 1u);
      EXPECT_EQ(countKind(Valid, CstKind::ArrayExpression), 0u);
      expectFullFidelity(Valid);
    }

    // Verifies class type expressions accept only their dedicated access/final prefix set and reject function or decorator declaration prefixes.
    TEST(ParserRecoveryTest, RejectsDeclarationOnlyPrefixesBeforeClassTypeExpressions)
    {
      const std::vector<std::string> InvalidPrefixes = {
          "static ",
          "virtual ",
          "override ",
          "async ",
          "implicit ",
          "extern \"C\" ",
          "@trace ",
          "[attribute] static ",
      };
      const std::vector<std::string> ValidPrefixes = {
          "",
          "public ",
          "protected ",
          "private ",
          "final ",
          "public final ",
      };

      for (const std::string &Prefix : InvalidPrefixes)
      {
        SCOPED_TRACE(Prefix);
        const ParsedFile File = parseSource("const Value = " + Prefix + "class {};");
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(countKind(File, CstKind::ClassTypeExpression), 0u);
        expectFullFidelity(File);
      }
      for (const std::string &Prefix : ValidPrefixes)
      {
        SCOPED_TRACE(Prefix);
        const ParsedFile File = parseSource("const Value = " + Prefix + "class {};");
        EXPECT_TRUE(File.succeeded());
        EXPECT_EQ(countKind(File, CstKind::ClassTypeExpression), 1u);
        expectFullFidelity(File);
      }
    }

    // Verifies an unterminated aggregate initializer stops at its caller's semicolon so the following top-level declaration remains independently parseable.
    TEST(ParserRecoveryTest, PreservesOuterStopsAfterUnterminatedAggregateInitializers)
    {
      const ParsedFile File = parseSource("const First = Record { Field: T*; const Second = 1;");
      const std::vector<std::string> Aggregates = nodeTextsOfKind(File, CstKind::AggregateInitializationExpression);

      EXPECT_FALSE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::TopLevelBindingDeclaration), 2u);
      ASSERT_EQ(Aggregates.size(), 1u);
      EXPECT_EQ(Aggregates.front(), "Record { Field: T*");
      expectFullFidelity(File);
    }

    // Verifies aggregate field shorthand has a distinct CST kind and explicit fields retain their colon and value expression.
    TEST(ParserRecoveryTest, DistinguishesAggregateFieldShorthandFromExplicitInitializers)
    {
      const ParsedFile File = parseSource("const PointValue = Point { X, Y: 2 };");

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::AggregateFieldShorthand), 1u);
      EXPECT_EQ(countKind(File, CstKind::AggregateFieldInitializer), 1u);
      expectFullFidelity(File);
    }

    // Verifies aggregate field separators take precedence when the enclosing call, array, or match-arm expression also terminates at a comma.
    TEST(ParserRecoveryTest, PrioritizesAggregateCommasOverOverlappingOuterStops)
    {
      const std::vector<std::string> Sources = {
          "const Value = consume(Point { X: 1, Y: 2 });",
          "const Value = [Point { X: 1, Y: 2 }, Other];",
          "const Value = match (Input) { .some => Point { X: 1, Y: 2 }, };",
      };

      for (const std::string &Source : Sources)
      {
        SCOPED_TRACE(Source);
        const ParsedFile File = parseSource(Source);
        EXPECT_TRUE(File.succeeded());
        EXPECT_EQ(countKind(File, CstKind::AggregateFieldInitializer), 2u);
        expectFullFidelity(File);
      }
    }

    // Verifies invalid aggregate fields synchronize at top-level commas without treating commas inside nested delimiters as item boundaries.
    TEST(ParserRecoveryTest, RecoversInvalidAggregateFieldsAtItemBoundaries)
    {
      const std::vector<std::pair<std::string, std::size_t>> Cases = {
          {"const PointValue = Point { 0, Y: 2 };", 1u},
          {"const PointValue = Point { X: @, Y: 2 };", 2u},
          {"const PointValue = Point { 0(First, Second), Y: 2 };", 1u},
      };

      for (const auto &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.first);
        const ParsedFile File = parseSource(TestCase.first);
        const std::vector<std::string> Initializers = nodeTextsOfKind(File, CstKind::AggregateFieldInitializer);
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(Initializers.size(), TestCase.second);
        EXPECT_TRUE(std::any_of(Initializers.begin(), Initializers.end(), [](const std::string &Text)
                                {
                                  return Text.find("Y: 2") != std::string::npos;
                                }));
        EXPECT_TRUE(hasKind(File, CstKind::Error));
        expectFullFidelity(File);
      }
    }

    // Verifies syntax-only omissions carry a structural missing marker and a compound reserved sequence uses its complete source span.
    TEST(ParserRecoveryTest, RecordsStructuralErrorsAndCompoundDiagnosticSpans)
    {
      const ParsedFile EmptyMatch = parseSource("func empty() { match (Value) {} }");
      const ParsedFile Reserved = parseSource("func reserved() { ++Value; }");

      EXPECT_FALSE(EmptyMatch.succeeded());
      EXPECT_TRUE(hasFlag(EmptyMatch.cst().node(EmptyMatch.cst().root()).Flags, CstNodeFlags::HasMissing));
      const auto Diagnostic = std::find_if(Reserved.diagnostics().begin(), Reserved.diagnostics().end(), [](const core::Diagnostic &Entry)
                                           {
                                             return Entry.Kind == core::DiagnosticKind::ReservedSymbolSequence;
                                           });
      ASSERT_NE(Diagnostic, Reserved.diagnostics().end());
      const std::size_t ReservedStart = Reserved.lexedFile().source().find("++");
      ASSERT_NE(ReservedStart, std::string::npos);
      EXPECT_EQ(Diagnostic->Span, (core::SourceRange{ReservedStart, ReservedStart + 2}));
      expectFullFidelity(EmptyMatch);
      expectFullFidelity(Reserved);
    }

    // Verifies trivia before a real synchronization token belongs to the enclosing construct rather than the child missing a delimiter.
    TEST(ParserRecoveryTest, KeepsSynchronizationTriviaOutsideIncompleteChildren)
    {
      const ParsedFile File = parseSource("func trivia(Value: i32 /* gap */ {}");
      const std::vector<std::string> ParameterClauses = nodeTextsOfKind(File, CstKind::FunctionParameterClause);

      ASSERT_FALSE(File.succeeded());
      ASSERT_EQ(ParameterClauses.size(), 1u);
      EXPECT_EQ(ParameterClauses.front().find("/* gap */"), std::string::npos);
      expectFullFidelity(File);
    }

    // Verifies a complete next match-arm pattern stops a preceding body that is missing its required terminator.
    TEST(ParserRecoveryTest, SynchronizesBeforeFollowingMatchArms)
    {
      const ParsedFile Statement = parseSource("func recover() { match (Value) { .first => Result .second => return; } }");
      const ParsedFile Expression = parseSource("const Result = match (Value) { .first => Value .second => Other, };");

      EXPECT_FALSE(Statement.succeeded());
      EXPECT_EQ(countKind(Statement, CstKind::MatchStatementArm), 2u);
      EXPECT_EQ(countKind(Statement, CstKind::VariantPattern), 2u);
      EXPECT_FALSE(Expression.succeeded());
      EXPECT_EQ(countKind(Expression, CstKind::MatchExpressionArm), 2u);
      EXPECT_EQ(countKind(Expression, CstKind::VariantPattern), 2u);
      expectFullFidelity(Statement);
      expectFullFidelity(Expression);
    }

    // Verifies wholly missing arm bodies and wrong-form separators remain in the selected match form without creating synthetic extra arms.
    TEST(ParserRecoveryTest, RecoversMatchBodiesAndSeparatorsWithoutFakeArms)
    {
      const std::vector<ParsedFile> Files = {
          parseSource("func recover() { match (Value) { .first => .second => return; } }"),
          parseSource("func recover() { match (Value) { .first => return;, .second => return; } }"),
          parseSource("const Result = match (Value) { .first => .second => Other, };"),
          parseSource("const Result = match (Value) { .first => First;, .second => Second, };"),
      };

      for (std::size_t Index = 0; Index < Files.size(); ++Index)
      {
        SCOPED_TRACE(Index);
        const ParsedFile &File = Files[Index];
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(countKind(File, Index < 2 ? CstKind::MatchStatementArm : CstKind::MatchExpressionArm), 2u);
        EXPECT_EQ(countKind(File, CstKind::VariantPattern), 2u);
        EXPECT_TRUE(hasKind(File, CstKind::Error) || hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasMissing));
        expectFullFidelity(File);
      }
    }

    // Verifies match-arm synchronization propagates through missing operands, value statements, and recovery-only declarations without consuming the next pattern.
    TEST(ParserRecoveryTest, PreservesFollowingPatternsAcrossIncompleteMatchBodies)
    {
      const std::vector<std::pair<const char *, CstKind>> Cases = {
          {"const Result = match (Value) { .first => _ => Other, };", CstKind::MatchExpressionArm},
          {"const Result = match (Value) { .first => First + _ => Other, };", CstKind::MatchExpressionArm},
          {"const Result = match (Value) { .first => First; junk .second => Other, };", CstKind::MatchExpressionArm},
          {"func recover() { match (Value) { .first => First + _ => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => return First .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => return First, junk .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => defer First .second(Value) => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => throw First .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => throw First from Cause() .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => throw First from Cause.field .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => throw First from Cause->field .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => throw First from Cause::<Type> .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => var Local = First .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => var Local: Type .second => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => const Local: Type = First .second(Value) => return; } }", CstKind::MatchStatementArm},
          {"func recover() { match (Value) { .first => Record { Field: Value .inner => Other } .second => return; } }", CstKind::MatchStatementArm},
          {"const Result = match (Value) { .first => Record { Field: Value .inner => Other }, .second => Final, };", CstKind::MatchExpressionArm},
      };

      for (const auto &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.first);
        const ParsedFile File = parseSource(TestCase.first);
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(countKind(File, TestCase.second), 2u);
        EXPECT_EQ(countKind(File, CstKind::VariantPattern) + countKind(File, CstKind::WildcardPattern), 2u);
        expectFullFidelity(File);
      }
    }

    // Verifies incomplete structured statement and expression bodies stop at a complete following arm without flattening nested delimiters or losing its pattern.
    TEST(ParserRecoveryTest, PreservesArmBoundariesAcrossIncompleteStructuredBodies)
    {
      const std::vector<StructuredMatchRecoveryCase> Cases = {
          {"CatchTypeBody", "func recover() { match (Value) { .first => try {} catch Error .second(Payload) => return; } }", CstKind::MatchStatementArm},
          {"CatchBindingBody", "func recover() { match (Value) { .first => try {} catch Error as Failure .second => return; } }", CstKind::MatchStatementArm},
          {"ClassTypeBody", "func recover() { match (Value) { .first => class Local .second => return; } }", CstKind::MatchStatementArm},
          {"IfBody", "func recover() { match (Value) { .first => if (Ready) .second => return; } }", CstKind::MatchStatementArm},
          {"WhileBody", "func recover() { match (Value) { .first => while (Ready) .second => return; } }", CstKind::MatchStatementArm},
          {"ForBody", "func recover() { match (Value) { .first => for (Item in Items) .second => return; } }", CstKind::MatchStatementArm},
          {"NestedMatchBody", "func recover() { match (Value) { .first => match (Other) .second => return; } }", CstKind::MatchStatementArm},
          {"ComptimeIfBody", "func recover() { match (Value) { .first => comptime if (Ready) .second => return; } }", CstKind::MatchStatementArm},
          {"ComptimeMatchBody", "func recover() { match (Value) { .first => comptime match (Other) .second => return; } }", CstKind::MatchStatementArm},
          {"IfExpressionBody", "const Result = match (Value) { .first => if (Ready) First else .second => Other, };", CstKind::MatchExpressionArm},
          {"NestedMatchExpressionBody", "const Result = match (Value) { .first => match (Other) .second => Other, };", CstKind::MatchExpressionArm},
          {"ClassExpressionBody", "const Result = match (Value) { .first => class Local .second => Other, };", CstKind::MatchExpressionArm},
      };

      for (const StructuredMatchRecoveryCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile File = parseSource(TestCase.Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(countKind(File, TestCase.ArmKind), 2u);
        EXPECT_EQ(countKind(File, CstKind::VariantPattern) + countKind(File, CstKind::WildcardPattern), 2u);
        EXPECT_TRUE(hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasMissing) || hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasError));
        expectFullFidelity(File);
      }
    }

    // Verifies optional return and rethrow operands stay absent at an arm boundary while defer still records its required missing body.
    TEST(ParserRecoveryTest, DistinguishesOptionalAndRequiredBodiesAtMatchArmBoundaries)
    {
      const ParsedFile Return = parseSource("func recover() { match (Value) { .first => return, .second => return; } }");
      const ParsedFile Throw = parseSource("func recover() { match (Value) { .first => throw, .second => return; } }");
      const ParsedFile Defer = parseSource("func recover() { match (Value) { .first => defer, .second => return; } }");
      const auto HasMissingExpression = [](const ParsedFile &File)
      {
        const std::vector<MissingToken> Missing = missingTokens(File);
        return std::any_of(Missing.begin(), Missing.end(), [](const MissingToken &Token)
                           {
                             return Token.ExpectedSpelling.find("expression") != std::string::npos;
                           });
      };

      EXPECT_FALSE(HasMissingExpression(Return));
      EXPECT_FALSE(HasMissingExpression(Throw));
      EXPECT_TRUE(HasMissingExpression(Defer));
      EXPECT_EQ(countKind(Return, CstKind::MatchStatementArm), 2u);
      EXPECT_EQ(countKind(Throw, CstKind::MatchStatementArm), 2u);
      EXPECT_EQ(countKind(Defer, CstKind::MatchStatementArm), 2u);
      expectFullFidelity(Return);
      expectFullFidelity(Throw);
      expectFullFidelity(Defer);
    }

    // Verifies a missing comptime-match region block synchronizes before the next arm and preserves the block kind selected by every enclosing region.
    TEST(ParserRecoveryTest, SynchronizesMissingComptimeMatchBlocksInEveryRegion)
    {
      const std::vector<RegionMatchRecoveryCase> Cases = {
          {"StatementMissingBlock", "func recover() { comptime match (Value) { .first => .second => {} } }", CstKind::StatementBlock},
          {"StatementJunkBeforeArm", "func recover() { comptime match (Value) { .first => junk .second => {} } }", CstKind::StatementBlock},
          {"StatementJunkAfterBlock", "func recover() { comptime match (Value) { .first => {} junk .second => {} } }", CstKind::StatementBlock},
          {"StatementMissingCloser", "func recover() { comptime match (Value) { .first => { return; .second => {} } }", CstKind::StatementBlock},
          {"TopLevelMissingBlock", "comptime match (Value) { .first => .second => {} }", CstKind::TopLevelBlock},
          {"TopLevelJunkBeforeArm", "comptime match (Value) { .first => junk .second => {} }", CstKind::TopLevelBlock},
          {"TopLevelMissingCloser", "comptime match (Value) { .first => { const First = 1; .second => {} }", CstKind::TopLevelBlock},
          {"ClassMissingBlock", "class Recover { comptime match (Value) { .first => .second => {} } }", CstKind::ClassMemberBlock},
          {"ClassJunkBeforeArm", "class Recover { comptime match (Value) { .first => junk .second => {} } }", CstKind::ClassMemberBlock},
          {"ClassMissingCloser", "class Recover { comptime match (Value) { .first => { var First: i32; .second => {} } }", CstKind::ClassMemberBlock},
          {"InterfaceMissingBlock", "interface Recover { comptime match (Value) { .first => .second => {} } }", CstKind::InterfaceMemberBlock},
          {"InterfaceJunkBeforeArm", "interface Recover { comptime match (Value) { .first => junk .second => {} } }", CstKind::InterfaceMemberBlock},
          {"InterfaceMissingCloser", "interface Recover { comptime match (Value) { .first => { func First(); .second => {} } }", CstKind::InterfaceMemberBlock},
          {"EnumMissingBlock", "enum Recover { comptime match (Value) { .first => .second => {} } }", CstKind::EnumMemberBlock},
          {"EnumJunkBeforeArm", "enum Recover { comptime match (Value) { .first => junk .second => {} } }", CstKind::EnumMemberBlock},
          {"EnumMissingCloser", "enum Recover { comptime match (Value) { .first => { First .second => {} } }", CstKind::EnumMemberBlock},
      };

      for (const RegionMatchRecoveryCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile File = parseSource(TestCase.Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(countKind(File, CstKind::RegionArm), 2u);
        EXPECT_EQ(countKind(File, CstKind::VariantPattern), 2u);
        EXPECT_GE(countKind(File, TestCase.BlockKind), 2u);
        EXPECT_TRUE(hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasMissing) || hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasError));
        expectFullFidelity(File);
      }
    }

    // Verifies incomplete declarations and control constructs inside every comptime region stop at the next outer arm while the missing region-block closer is synthesized.
    TEST(ParserRecoveryTest, PreservesRegionArmBoundariesAcrossIncompleteItems)
    {
      const std::vector<RegionMatchRecoveryCase> Cases = {
          {"StatementTry", "func recover() { comptime match (Value) { .first => { try {} catch Error .second => {} } }", CstKind::StatementBlock},
          {"TopLevelImport", "comptime match (Value) { .first => { import core.io .second => {} }", CstKind::TopLevelBlock},
          {"TopLevelFunction", "comptime match (Value) { .first => { func broken() .second => {} }", CstKind::TopLevelBlock},
          {"TopLevelClass", "comptime match (Value) { .first => { class Broken .second => {} }", CstKind::TopLevelBlock},
          {"TopLevelDecorator", "comptime match (Value) { .first => { @trace .second => {} }", CstKind::TopLevelBlock},
          {"ClassFunction", "class Host { comptime match (Value) { .first => { func broken() .second => {} } }", CstKind::ClassMemberBlock},
          {"InterfaceFunction", "interface Host { comptime match (Value) { .first => { func broken() .second => {} } }", CstKind::InterfaceMemberBlock},
          {"EnumNestedComptime", "enum Host { comptime match (Value) { .first => { comptime if (Ready) .second => {} } }", CstKind::EnumMemberBlock},
      };

      for (const RegionMatchRecoveryCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile File = parseSource(TestCase.Source);
        EXPECT_FALSE(File.succeeded());
        EXPECT_EQ(countKind(File, CstKind::RegionArm), 2u);
        EXPECT_EQ(countKind(File, CstKind::VariantPattern), 2u);
        EXPECT_GE(countKind(File, TestCase.BlockKind), 2u);
        EXPECT_TRUE(hasFlag(File.cst().node(File.cst().root()).Flags, CstNodeFlags::HasMissing));
        expectFullFidelity(File);
      }
    }

    // Verifies a try statement without any handler retains a missing catch-clause node with placeholders for its introducer and body delimiters.
    TEST(ParserRecoveryTest, SynthesizesMissingCatchClauseStructure)
    {
      const ParsedFile File = parseSource("func recover() { try {} }");
      const std::vector<MissingToken> Missing = missingTokens(File);

      EXPECT_FALSE(File.succeeded());
      EXPECT_EQ(countKind(File, CstKind::TryStatement), 1u);
      EXPECT_EQ(countKind(File, CstKind::CatchClause), 1u);
      EXPECT_EQ(countKind(File, CstKind::TypedCatchClause), 0u);
      EXPECT_EQ(countKind(File, CstKind::CatchAllClause), 0u);
      for (const std::string &Expected : {std::string("catch"), std::string("{"), std::string("}")})
      {
        EXPECT_TRUE(std::any_of(Missing.begin(), Missing.end(), [&Expected](const MissingToken &Token)
                                {
                                  return Token.ExpectedSpelling == Expected;
                                }))
            << "missing placeholder for " << Expected;
      }
      expectFullFidelity(File);
    }

    // Verifies a direct var or const match-arm body is recovered as a binding core inside Error without manufacturing the block-only LocalBindingDeclaration wrapper.
    TEST(ParserRecoveryTest, KeepsStatementOnlyMatchBindingsOutOfLocalDeclarations)
    {
      const ParsedFile File = parseSource("func recover() { match (Value) { _ => var Item = Value; .next => return; } }");

      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(hasDiagnostic(File, core::DiagnosticKind::DeclarationRequiresBlock));
      EXPECT_EQ(countKind(File, CstKind::MatchStatementArm), 2u);
      EXPECT_EQ(countKind(File, CstKind::NamedBindingDeclaration), 1u);
      EXPECT_EQ(countKind(File, CstKind::LocalBindingDeclaration), 0u);
      EXPECT_GE(countKind(File, CstKind::Error), 1u);
      expectFullFidelity(File);
    }

    // Verifies complete input remains Complete under interactive parsing and produces the same successful syntax shape.
    TEST(ParserInteractiveTest, CompleteInputRemainsCompleteInInteractiveMode)
    {
      const ParsedFile File = parseSource("func complete() { return; }", ParserOptions{ParseMode::Interactive});

      ASSERT_TRUE(File.succeeded());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
      EXPECT_EQ(countKind(File, CstKind::ReturnStatement), 1u);
      expectFullFidelity(File);
    }

    // Verifies the Parser returns a safe unsuccessful result for a token buffer that already contains lexical errors.
    TEST(ParserApiTest, RejectsLexicallyFailedTokenBuffers)
    {
      tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize("?");

      ASSERT_FALSE(LexedFile.succeeded());
      const ParsedFile File = ink::parser::parse(std::move(LexedFile));
      EXPECT_FALSE(File.succeeded());
      EXPECT_TRUE(File.cst().nodes().empty());
      EXPECT_TRUE(File.cst().children().empty());
    }

    // Verifies one configured Parser instance can parse multiple independent token buffers without retaining prior state.
    TEST(ParserApiTest, ParserInstanceIsReusableAcrossFiles)
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      const Parser Reusable(Context, ParserOptions{ParseMode::Batch});
      const ParsedFile First = Reusable.parse(tokenizer::tokenize(Context, "const First = 1;"));
      const ParsedFile Second = Reusable.parse(tokenizer::tokenize(Context, "func Second() { return; }"));

      ASSERT_TRUE(First.succeeded());
      ASSERT_TRUE(Second.succeeded());
      EXPECT_TRUE(hasKind(First, CstKind::TopLevelBindingDeclaration));
      EXPECT_FALSE(hasKind(First, CstKind::FunctionDeclaration));
      EXPECT_TRUE(hasKind(Second, CstKind::FunctionDeclaration));
      EXPECT_FALSE(hasKind(Second, CstKind::TopLevelBindingDeclaration));
      expectFullFidelity(First);
      expectFullFidelity(Second);
    }

    // Verifies that finalized Parser diagnostics are both retained in the parsed result and published through the shared frontend context.
    TEST(ParserApiTest, PublishesDiagnosticsThroughFrontendContext)
    {
      core::CompilationContext Compilation;
      core::FrontendContext Context(Compilation);
      core::CollectingDiagnosticConsumer Diagnostics;
      Compilation.diagnosticEngine().addConsumer(Diagnostics);
      tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(Context, "const Value = ;");

      ASSERT_TRUE(LexedFile.succeeded());
      const ParsedFile File = parse(Context, std::move(LexedFile));

      ASSERT_FALSE(File.diagnostics().empty());
      EXPECT_EQ(Diagnostics.diagnostics(), File.diagnostics());
    }

    // Verifies deterministic recovery and mandatory forward progress over many lexically valid but arbitrarily ordered token streams.
    TEST(ParserRecoveryTest, AlwaysAdvancesAndPreservesArbitraryTokenStreams)
    {
      constexpr const char *Spellings[] = {
          "Name",
          "Other",
          "_",
          "1",
          "true",
          "null",
          "func",
          "class",
          "interface",
          "enum",
          "var",
          "const",
          "if",
          "else",
          "match",
          "while",
          "for",
          "in",
          "return",
          "throw",
          "try",
          "catch",
          "comptime",
          "public",
          "final",
          "async",
          "type",
          "i32",
          "(",
          ")",
          "[",
          "]",
          "{",
          "}",
          ";",
          ",",
          ".",
          ":",
          "=",
          "+",
          "-",
          "*",
          "&",
          "<",
          ">",
          "@",
      };
      constexpr std::size_t CaseCount = 128;
      constexpr std::size_t TokensPerCase = 96;
      std::uint32_t State = 0x4B1D5A77U;

      for (std::size_t CaseIndex = 0; CaseIndex < CaseCount; ++CaseIndex)
      {
        std::string Source;
        for (std::size_t TokenIndex = 0; TokenIndex < TokensPerCase; ++TokenIndex)
        {
          State = State * 1664525U + 1013904223U;
          if (!Source.empty())
          {
            Source.push_back(' ');
          }
          Source.append(Spellings[State % (sizeof(Spellings) / sizeof(Spellings[0]))]);
        }

        SCOPED_TRACE(CaseIndex);
        const ParsedFile First = parseSource(Source);
        const ParsedFile Second = parseSource(Source);

        EXPECT_EQ(First.completeness(), ParseCompleteness::Complete);
        EXPECT_EQ(First.cst().nodes(), Second.cst().nodes());
        EXPECT_EQ(First.cst().children(), Second.cst().children());
        EXPECT_EQ(First.diagnostics(), Second.diagnostics());
        expectFullFidelity(First);
      }
    }
  } // namespace
} // namespace ink::parser
