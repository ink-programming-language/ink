#include "ink/sema/analyzer.h"

#include "ink/sema/control_flow_checker.h"
#include "ink/sema/declaration_collector.h"
#include "ink/sema/name_resolver.h"
#include "ink/sema/signature_resolver.h"
#include "ink/sema/type_checker.h"

#include <utility>

namespace ink::sema
{
  bool SemanticAnalysisResult::succeeded() const noexcept
  {
    return Diagnostics.empty() && Verification.succeeded() && Module.has_value();
  }

  SemanticAnalyzer::SemanticAnalyzer(const ast::AstContext &AstContext, const ast::AstFile &AstFile, const core::StringInterner &Strings, type::TypeContext &Types) noexcept : AstContext(AstContext), AstFile(AstFile), Strings(Strings), Types(Types)
  {
  }

  SemanticAnalysisResult SemanticAnalyzer::analyze() const
  {
    SemanticModel Model(AstContext, AstFile, Strings, Types);
    std::vector<core::Diagnostic> Diagnostics;
    DeclarationCollector(Model, Diagnostics).run();
    SignatureResolver(Model, Diagnostics).run();
    NameResolver(Model, Diagnostics).run();
    TypeChecker(Model, Diagnostics).run();
    ControlFlowChecker(Model, Diagnostics).run();
    const SemaVerifier Verifier;
    if (!Diagnostics.empty())
    {
      return {std::move(Diagnostics), Verifier.verify(Model), std::nullopt};
    }
    SemanticVerificationOutcome Outcome = Verifier.verifyAndSeal(std::move(Model));
    return {std::move(Diagnostics), std::move(Outcome.Verification), std::move(Outcome.Module)};
  }

  SemanticAnalysisResult analyze(const ast::AstContext &AstContext, const ast::AstFile &AstFile, const core::StringInterner &Strings, type::TypeContext &Types)
  {
    return SemanticAnalyzer(AstContext, AstFile, Strings, Types).analyze();
  }
} // namespace ink::sema
