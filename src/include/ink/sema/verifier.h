#ifndef INK_SEMA_VERIFIER_H
#define INK_SEMA_VERIFIER_H

#include "ink/sema/semantic_model.h"

#include <optional>
#include <string>
#include <vector>

namespace ink::sema
{
  struct SemanticVerificationError
  {
    std::string Message;
  };

  struct SemanticVerificationResult
  {
    std::vector<SemanticVerificationError> Errors;

    bool succeeded() const noexcept;
  };

  class VerifiedSemanticModule
  {
  public:
    VerifiedSemanticModule(const VerifiedSemanticModule &) = delete;
    VerifiedSemanticModule &operator=(const VerifiedSemanticModule &) = delete;
    VerifiedSemanticModule(VerifiedSemanticModule &&) noexcept = default;
    VerifiedSemanticModule &operator=(VerifiedSemanticModule &&) noexcept = default;

    const SemanticModel &model() const noexcept;
    core::SourceFileId sourceFile() const noexcept;

  private:
    explicit VerifiedSemanticModule(SemanticModel Model) noexcept;

    SemanticModel Model;

    friend class SemaVerifier;
  };

  struct SemanticVerificationOutcome
  {
    SemanticVerificationResult Verification;
    std::optional<VerifiedSemanticModule> Module;
  };

  class SemaVerifier
  {
  public:
    SemanticVerificationResult verify(const SemanticModel &Model) const;
    SemanticVerificationOutcome verifyAndSeal(SemanticModel Model) const;
  };
} // namespace ink::sema

#endif
