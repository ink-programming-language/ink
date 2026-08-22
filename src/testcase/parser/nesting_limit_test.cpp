#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <string>
#include <vector>

namespace ink::parser
{
  namespace
  {
    using test::expectFullFidelity;
    using test::hasDiagnostic;
    using test::nodeText;
    using test::parseSource;

    std::string nestedArraySource(std::size_t Depth)
    {
      std::string Source = "const Deep = ";
      Source.append(Depth, '[');
      Source += '0';
      Source.append(Depth, ']');
      Source += ';';
      return Source;
    }

    std::string nestedTupleTypeSource(std::size_t Depth)
    {
      std::string Type = "i32";
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Type = "(" + Type + ",)";
      }
      return "func deep(Value: " + Type + ");";
    }

    std::string nestedStatementBlockSource(std::size_t Depth)
    {
      std::string Source = "func deep() {";
      Source.append(Depth, '{');
      Source.append(Depth, '}');
      Source += '}';
      return Source;
    }

    std::string nestedTuplePatternSource(std::size_t Depth)
    {
      std::string Source = "const ";
      Source.append(Depth, '(');
      Source += "Value";
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Source += ",)";
      }
      Source += " = Input;";
      return Source;
    }

    std::string nestedTypeDeclarationSource(std::size_t Depth)
    {
      std::string Source;
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Source += "class Nested {";
      }
      Source.append(Depth, '}');
      return Source;
    }

    std::string nestedComptimeRegionSource(std::size_t Depth)
    {
      std::string Source;
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Source += "comptime {";
      }
      Source.append(Depth, '}');
      return Source;
    }

    std::string elseIfStatementSource(std::size_t Depth)
    {
      std::string Source = "func deep() { if (Ready) {}";
      for (std::size_t Index = 1; Index < Depth; ++Index)
      {
        Source += " else if (Ready) {}";
      }
      Source += " }";
      return Source;
    }

    std::string elseIfRegionSource(std::size_t Depth)
    {
      std::string Source = "comptime if (Ready) {}";
      for (std::size_t Index = 1; Index < Depth; ++Index)
      {
        Source += " else if (Ready) {}";
      }
      return Source;
    }

    std::string nestedGenericArgumentSource(std::size_t Depth)
    {
      std::string Source = "const Deep = ";
      for (std::size_t Index = 0; Index < Depth; ++Index)
      {
        Source += "Generic::<";
      }
      Source += "Value";
      Source.append(Depth, '>');
      Source += ';';
      return Source;
    }

    struct DeepSyntaxCase
    {
        const char *Name;
        std::string Source;
    };

    // Verifies syntax within the configured recursion budget remains valid while the first expression beyond it reports the dedicated limit diagnostic.
    TEST(ParserNestingLimitTest, HonorsConfiguredExpressionBoundary)
    {
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 32;
      const ParsedFile WithinLimit = parseSource(nestedArraySource(31), Options);
      const ParsedFile BeyondLimit = parseSource(nestedArraySource(32), Options);

      EXPECT_TRUE(WithinLimit.succeeded());
      EXPECT_FALSE(BeyondLimit.succeeded());
      EXPECT_TRUE(hasDiagnostic(BeyondLimit, core::DiagnosticKind::SyntaxNestingLimit));
      expectFullFidelity(WithinLimit);
      expectFullFidelity(BeyondLimit);
    }

    // Verifies deeply nested expressions and types recover deterministically at the configured limit without recursive stack exhaustion or token loss.
    TEST(ParserNestingLimitTest, RecoversDeepExpressionsAndTypesWithFullFidelity)
    {
      constexpr std::size_t Depth = 512;
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 32;
      const std::vector<DeepSyntaxCase> Cases = {
          {"ArrayExpression", nestedArraySource(Depth)},
          {"TupleType", nestedTupleTypeSource(Depth)},
      };

      for (const DeepSyntaxCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile First = parseSource(TestCase.Source, Options);
        const ParsedFile Second = parseSource(TestCase.Source, Options);

        EXPECT_FALSE(First.succeeded());
        EXPECT_EQ(First.completeness(), ParseCompleteness::Complete);
        EXPECT_TRUE(hasDiagnostic(First, core::DiagnosticKind::SyntaxNestingLimit));
        EXPECT_EQ(First.cst().nodes(), Second.cst().nodes());
        EXPECT_EQ(First.cst().children(), Second.cst().children());
        EXPECT_TRUE(test::diagnosticsEqual(First, Second));
        EXPECT_EQ(nodeText(First, First.cst().root()), TestCase.Source);
        expectFullFidelity(First);
      }
    }

    // Verifies every structurally recursive grammar family shares the nesting budget and recovers deterministically instead of exhausting the process stack.
    TEST(ParserNestingLimitTest, RecoversDeepStructuralSyntaxWithFullFidelity)
    {
      constexpr std::size_t Depth = 768;
      ParserOptions Options;
      Options.MaxSyntaxNestingDepth = 32;
      const std::vector<DeepSyntaxCase> Cases = {
          {"StatementBlock", nestedStatementBlockSource(Depth)},
          {"TuplePattern", nestedTuplePatternSource(Depth)},
          {"TypeDeclaration", nestedTypeDeclarationSource(Depth)},
          {"ComptimeRegion", nestedComptimeRegionSource(Depth)},
          {"ElseIfStatement", elseIfStatementSource(Depth)},
          {"ElseIfRegion", elseIfRegionSource(Depth)},
          {"GenericArgument", nestedGenericArgumentSource(Depth)},
      };

      for (const DeepSyntaxCase &TestCase : Cases)
      {
        SCOPED_TRACE(TestCase.Name);
        const ParsedFile First = parseSource(TestCase.Source, Options);
        const ParsedFile Second = parseSource(TestCase.Source, Options);

        EXPECT_FALSE(First.succeeded());
        EXPECT_EQ(First.completeness(), ParseCompleteness::Complete);
        EXPECT_TRUE(hasDiagnostic(First, core::DiagnosticKind::SyntaxNestingLimit));
        EXPECT_EQ(First.cst().nodes(), Second.cst().nodes());
        EXPECT_EQ(First.cst().children(), Second.cst().children());
        EXPECT_TRUE(test::diagnosticsEqual(First, Second));
        EXPECT_EQ(nodeText(First, First.cst().root()), TestCase.Source);
        expectFullFidelity(First);
      }
    }

    // Verifies missing expressions, types, and generic arguments at synchronization tokens do not consume those tokens or masquerade as nesting-limit failures.
    TEST(ParserNestingLimitTest, PrioritizesMissingSyntaxAtNestingBoundaryStops)
    {
      ParserOptions StatementOptions;
      StatementOptions.MaxSyntaxNestingDepth = 1;
      ParserOptions DeclarationOptions;
      DeclarationOptions.MaxSyntaxNestingDepth = 0;
      const ParsedFile MissingCondition = parseSource("func missing() { if () {} }", StatementOptions);
      const ParsedFile MissingTypeAndArgument = parseSource("func missing<T: = >();", DeclarationOptions);

      for (const ParsedFile *File : {&MissingCondition, &MissingTypeAndArgument})
      {
        EXPECT_FALSE(File->succeeded());
        EXPECT_FALSE(hasDiagnostic(*File, core::DiagnosticKind::SyntaxNestingLimit));
        expectFullFidelity(*File);
      }
    }
  } // namespace
} // namespace ink::parser
