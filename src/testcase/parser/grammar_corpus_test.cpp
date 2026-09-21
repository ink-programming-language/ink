#include "parser_test_support.h"

namespace ink::parser::test
{
  namespace
  {
    struct GrammarCase
    {
        const char *Name;
        const char *Rule;
        const char *Derivation;
        const char *Source;
    };

#include "corpus/grammar_cases.inc"

    std::string grammarCaseName(const testing::TestParamInfo<GrammarCase> &Info)
    {
      return Info.param.Name;
    }
  } // namespace

  class ParserGrammarCorpusTest : public ParserTest, public testing::WithParamInterface<GrammarCase>
  {
  };

  // Every BNF-derived input must parse without diagnostics or recovery and satisfy the shared AST and source-range contracts.
  TEST_P(ParserGrammarCorpusTest, AcceptsValidGrammar)
  {
    const GrammarCase &Case = GetParam();
    SCOPED_TRACE(testing::Message() << Case.Name << " rule=" << Case.Rule << ' ' << Case.Derivation << '\n' << Case.Source);
    const auto Result = read(Case.Source);
    EXPECT_TRUE(Result.succeeded());
    EXPECT_EQ(Result.Status, ParseStatus::Completed);
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    EXPECT_TRUE(Result.Unit->recoveryInfo().Entries.empty());
  }

  INSTANTIATE_TEST_SUITE_P(GrammarBranches, ParserGrammarCorpusTest, testing::ValuesIn(GrammarBranchCases), grammarCaseName);
  INSTANTIATE_TEST_SUITE_P(GrammarCombinations, ParserGrammarCorpusTest, testing::ValuesIn(GrammarCombinationCases), grammarCaseName);
} // namespace ink::parser::test
