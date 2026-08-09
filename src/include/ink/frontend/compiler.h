#ifndef INK_FRONTEND_COMPILER_H
#define INK_FRONTEND_COMPILER_H

#include "ink/core/source_file_id.h"
#include "ink/core/source_range.h"
#include "ink/frontend/compilation_session.h"
#include "ink/frontend/ir_generator.h"
#include "ink/ir/verifier.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ink::frontend
{
  enum class CompilationPhase : std::uint8_t
  {
    Parsing,
    AstVerification,
    SemanticAnalysis,
    IrGeneration,
    IrVerification,
    IrClosure,
  };

  struct CompilationIssue
  {
    CompilationPhase Phase = CompilationPhase::SemanticAnalysis;
    core::SourceFileId File;
    core::SourceRange Range;
    core::DiagnosticSeverity Severity = core::DiagnosticSeverity::Error;
    std::optional<core::DiagnosticKind> DiagnosticKind;
    std::vector<core::DiagnosticRelatedInformation> Related;
    std::string Message;
  };

  struct StagedCompilationResult
  {
    std::optional<ir::VerifiedStagedModule> Module;
    std::vector<PendingForceValue> PendingForceValues;
    std::vector<CompilationIssue> Issues;

    bool succeeded() const noexcept;
  };

  struct ClosedCompilationResult
  {
    std::optional<ir::VerifiedClosedModule> Module;
    std::vector<CompilationIssue> Issues;

    bool succeeded() const noexcept;
  };

  class Compiler
  {
  public:
    explicit Compiler(CompilationSession &Session) noexcept;

    StagedCompilationResult compileToStaged(core::SourceFileId File) const;
    ClosedCompilationResult close(const ir::VerifiedStagedModule &Module, const std::vector<ir::IrForceValueResolution> &Resolutions = {}) const;
    ClosedCompilationResult compile(core::SourceFileId File) const;

  private:
    CompilationSession &Session;
  };
} // namespace ink::frontend

#endif
