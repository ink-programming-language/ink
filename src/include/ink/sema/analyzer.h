#ifndef INK_SEMA_ANALYZER_H
#define INK_SEMA_ANALYZER_H

#include "ink/core/diagnostic.h"
#include "ink/sema/verifier.h"

#include <optional>
#include <vector>

namespace ink::sema
{
  struct SemanticAnalysisResult
  {
    std::vector<core::Diagnostic> Diagnostics;
    SemanticVerificationResult Verification;
    std::optional<VerifiedSemanticModule> Module;

    bool succeeded() const noexcept;
  };

  class SemanticAnalyzer
  {
  public:
    SemanticAnalyzer(const ast::AstContext &AstContext, const ast::AstFile &AstFile, const core::StringInterner &Strings, type::TypeContext &Types) noexcept;
    SemanticAnalysisResult analyze() const;

  private:
    const ast::AstContext &AstContext;
    const ast::AstFile &AstFile;
    const core::StringInterner &Strings;
    type::TypeContext &Types;
  };

  SemanticAnalysisResult analyze(const ast::AstContext &AstContext, const ast::AstFile &AstFile, const core::StringInterner &Strings, type::TypeContext &Types);
} // namespace ink::sema

#endif
