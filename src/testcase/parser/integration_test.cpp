#include "parser_test_support.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace ink::parser
{
  namespace
  {
    struct IntegrationTestCase
    {
        std::string DirectoryName;
        std::filesystem::path SolutionPath;
        std::string SetupError;
    };

    std::filesystem::path integrationDirectory()
    {
      return std::filesystem::path(__FILE__).parent_path().parent_path() / "integration";
    }

    std::vector<IntegrationTestCase> integrationFailure(std::string Message)
    {
      return {{"SetupFailure", {}, std::move(Message)}};
    }

    std::vector<IntegrationTestCase> integrationTestCases()
    {
      const std::filesystem::path IntegrationDirectory = integrationDirectory();
      std::vector<IntegrationTestCase> TestCases;
      std::error_code Error;
      std::filesystem::directory_iterator Iterator(IntegrationDirectory, Error);
      if (Error)
      {
        return integrationFailure("cannot enumerate parser integration directory: " + Error.message());
      }

      const std::filesystem::directory_iterator End;
      while (Iterator != End)
      {
        const std::filesystem::directory_entry &Entry = *Iterator;
        std::error_code StatusError;
        const bool IsDirectory = Entry.is_directory(StatusError);
        if (StatusError)
        {
          return integrationFailure("cannot inspect a parser integration directory entry: " + StatusError.message());
        }
        if (IsDirectory)
        {
          TestCases.push_back({Entry.path().filename().string(), Entry.path() / "solution.ink", {}});
        }

        Iterator.increment(Error);
        if (Error)
        {
          return integrationFailure("cannot continue enumerating parser integration directory: " + Error.message());
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
        Result << '\n'
               << Diagnostic.code() << " [" << Diagnostic.Span.Start << ", " << Diagnostic.Span.End << ')';
      }
      return Result.str();
    }

    std::string integrationTestName(const testing::TestParamInfo<IntegrationTestCase> &Information)
    {
      if (!Information.param.SetupError.empty())
      {
        return "SetupFailure";
      }
      return "Problem" + Information.param.DirectoryName.substr(0, 4);
    }

    class ParserIntegrationTest : public testing::TestWithParam<IntegrationTestCase>
    {
    };

    // Verifies that each discovered CodeContests Ink solution tokenizes and parses completely, while discovery failures are reported explicitly.
    TEST_P(ParserIntegrationTest, CompilesWithoutDiagnostics)
    {
      const IntegrationTestCase &TestCase = GetParam();
      SCOPED_TRACE(TestCase.DirectoryName);
      if (!TestCase.SetupError.empty())
      {
        ADD_FAILURE() << TestCase.SetupError;
        return;
      }

      std::ifstream SourceStream(TestCase.SolutionPath, std::ios::binary);
      ASSERT_TRUE(SourceStream.is_open()) << "unable to open " << TestCase.SolutionPath.string();
      std::string Source{std::istreambuf_iterator<char>(SourceStream), std::istreambuf_iterator<char>()};

      tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(std::move(Source));
      ASSERT_TRUE(LexedFile.succeeded()) << "tokenizer rejected " << TestCase.SolutionPath.string();

      const ParsedFile File = parse(std::move(LexedFile));
      EXPECT_TRUE(File.succeeded()) << diagnosticSummary(test::testDiagnostics(File));
      EXPECT_TRUE(test::testDiagnostics(File).empty()) << diagnosticSummary(test::testDiagnostics(File));
      EXPECT_EQ(File.completeness(), ParseCompleteness::Complete);
    }

    INSTANTIATE_TEST_SUITE_P(CodeContests, ParserIntegrationTest, testing::ValuesIn(integrationTestCases()), integrationTestName);
  } // namespace
} // namespace ink::parser
