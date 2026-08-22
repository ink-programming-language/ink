#ifndef INK_TESTCASE_TOKENIZER_TOKENIZER_TEST_SUPPORT_H
#define INK_TESTCASE_TOKENIZER_TOKENIZER_TEST_SUPPORT_H

#include "ink/tokenizer/tokenizer.h"

#include "../diagnostic_test_support.h"

#include <string>
#include <utility>
#include <vector>

namespace ink::tokenizer
{
  inline TokenizedBuffer tokenize(std::string Source, TokenizerOptions Options = {})
  {
    test::SharedDiagnosticTestContext &TestContext = test::sharedDiagnosticTestContext();
    const std::size_t Checkpoint = TestContext.checkpoint();
    core::FrontendContext Context(TestContext.compilationContext());
    TokenizedBuffer Result = tokenize(Context, std::move(Source), Options);
    TestContext.record(Result.sourceId(), Checkpoint);
    return Result;
  }

  inline TokenizedBuffer tokenizeNamedSource(std::string Name, std::string Source, TokenizerOptions Options = {})
  {
    test::SharedDiagnosticTestContext &TestContext = test::sharedDiagnosticTestContext();
    const std::size_t Checkpoint = TestContext.checkpoint();
    core::FrontendContext Context(TestContext.compilationContext());
    const core::SourceId SourceId = Context.sourceManager().addSource(std::move(Name), std::move(Source));
    TokenizedBuffer Result = tokenizeSource(Context, SourceId, Options);
    TestContext.record(Result.sourceId(), Checkpoint);
    return Result;
  }

  inline const std::vector<core::Diagnostic> &testDiagnostics(const TokenizedBuffer &Buffer) noexcept
  {
    return test::sharedDiagnosticTestContext().diagnostics(Buffer.sourceId());
  }
} // namespace ink::tokenizer

#endif
