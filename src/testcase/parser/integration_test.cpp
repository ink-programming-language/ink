#include "ink/parser/parser.h"
#include "ink/tokenizer/tokenizer.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ink::parser
{
  namespace
  {
    inline constexpr core::SourceFileId TestSourceFileId = core::SourceFileId::fromValue(0);

    struct IntegrationTestCase
    {
      std::string DirectoryName;
      std::filesystem::path SolutionPath;
    };

    std::filesystem::path integrationDirectory()
    {
      return std::filesystem::path(__FILE__).parent_path().parent_path() / "integration";
    }

    std::vector<IntegrationTestCase> integrationTestCases()
    {
      const std::filesystem::path IntegrationDirectory = integrationDirectory();
      std::vector<IntegrationTestCase> TestCases;
      for (const std::filesystem::directory_entry &Entry : std::filesystem::directory_iterator(IntegrationDirectory))
      {
        if (Entry.is_directory())
        {
          TestCases.push_back({Entry.path().filename().string(), Entry.path() / "solution.ink"});
        }
      }
      std::sort(TestCases.begin(), TestCases.end(), [](const IntegrationTestCase &Left, const IntegrationTestCase &Right)
                {
                  return Left.DirectoryName < Right.DirectoryName;
                });
      return TestCases;
    }

    std::string diagnosticSummary(const std::vector<core::Diagnostic> &Diagnostics)
    {
      std::ostringstream Result;
      for (const core::Diagnostic &Diagnostic : Diagnostics)
      {
        Result << '\n' << Diagnostic.code() << " [" << Diagnostic.Span.Start << ", " << Diagnostic.Span.End << ')';
      }
      return Result.str();
    }

    std::string integrationTestName(const testing::TestParamInfo<IntegrationTestCase> &Information)
    {
      return "Problem" + Information.param.DirectoryName.substr(0, 4);
    }

    class ParserIntegrationTest : public testing::TestWithParam<IntegrationTestCase>
    {
    };

    // Verifies that one generated CodeContests Ink solution tokenizes and parses completely without diagnostics.
    TEST_P(ParserIntegrationTest, CompilesWithoutDiagnostics)
    {
      const IntegrationTestCase &TestCase = GetParam();
      SCOPED_TRACE(TestCase.DirectoryName);

      std::ifstream SourceStream(TestCase.SolutionPath, std::ios::binary);
      ASSERT_TRUE(SourceStream.is_open()) << "unable to open " << TestCase.SolutionPath.string();
      std::string Source{std::istreambuf_iterator<char>(SourceStream), std::istreambuf_iterator<char>()};

      tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(TestSourceFileId, std::move(Source));
      ASSERT_TRUE(LexedFile.succeeded()) << "tokenizer rejected " << TestCase.SolutionPath.string();

      const ParsedFile File = parse(std::move(LexedFile));
      EXPECT_TRUE(File.succeeded()) << diagnosticSummary(File.diagnostics());
      EXPECT_TRUE(File.diagnostics().empty()) << diagnosticSummary(File.diagnostics());
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
    }

    INSTANTIATE_TEST_SUITE_P(CodeContests, ParserIntegrationTest, testing::ValuesIn(integrationTestCases()), integrationTestName);
  } // namespace
} // namespace ink::parser
