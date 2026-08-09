#include "ink/frontend/compiler.h"

#include "ink/ast/verifier.h"

#include <optional>
#include <utility>

namespace ink::frontend
{
  namespace
  {
    CompilationIssue makeIssue(CompilationPhase Phase, core::SourceFileId File, std::string Message, core::SourceRange Range = {})
    {
      return {Phase, File, Range, core::DiagnosticSeverity::Error, std::nullopt, {}, std::move(Message)};
    }

    CompilationIssue makeDiagnosticIssue(CompilationPhase Phase, const core::Diagnostic &Diagnostic)
    {
      const core::DiagnosticFormatter Formatter;
      return {Phase, Diagnostic.File, Diagnostic.Span, core::diagnosticDefaultSeverity(Diagnostic.Kind), Diagnostic.Kind, Diagnostic.Related, Formatter.format(Diagnostic).Message};
    }

    ir::IrOrigin originForError(const ir::IrModule &Module, const ir::IrVerificationError &Error)
    {
      ir::IrOriginId Origin;
      if (Module.contains(Error.Operation))
      {
        Origin = Module.operation(Error.Operation).Origin;
      }
      else if (Module.contains(Error.Block))
      {
        Origin = Module.block(Error.Block).Origin;
      }
      else if (Module.contains(Error.Function))
      {
        Origin = Module.function(Error.Function).Origin;
      }
      return Module.contains(Origin) ? Module.origin(Origin) : ir::IrOrigin{};
    }
  } // namespace

  bool StagedCompilationResult::succeeded() const noexcept
  {
    return Module.has_value() && Issues.empty();
  }

  bool ClosedCompilationResult::succeeded() const noexcept
  {
    return Module.has_value() && Issues.empty();
  }

  Compiler::Compiler(CompilationSession &Session) noexcept : Session(Session)
  {
  }

  StagedCompilationResult Compiler::compileToStaged(core::SourceFileId File) const
  {
    StagedCompilationResult Result;
    const std::size_t DiagnosticBegin = Session.diagnostics().size();
    std::optional<ast::AstFile> AstFile = Session.parseAndLower(File);
    for (std::size_t Index = DiagnosticBegin; Index < Session.diagnostics().size(); ++Index)
    {
      const core::Diagnostic &Diagnostic = Session.diagnostics()[Index];
      if (core::diagnosticDefaultSeverity(Diagnostic.Kind) == core::DiagnosticSeverity::Error)
      {
        Result.Issues.push_back(makeDiagnosticIssue(CompilationPhase::Parsing, Diagnostic));
      }
    }
    if (!Result.Issues.empty())
    {
      return Result;
    }
    if (!AstFile)
    {
      Result.Issues.push_back(makeIssue(CompilationPhase::Parsing, File, "source could not be parsed and lowered to AST"));
      return Result;
    }

    const ast::AstVerificationResult AstVerification = ast::verifyAst(Session.astContext(), *AstFile, Session.stringInterner());
    if (!AstVerification.succeeded())
    {
      for (const ast::AstVerificationError &Error : AstVerification.Errors)
      {
        Result.Issues.push_back(makeIssue(CompilationPhase::AstVerification, File, Error.Message));
      }
      return Result;
    }

    sema::SemanticAnalysisResult Semantic = Session.analyze(*AstFile);
    if (!Semantic.succeeded() || !Semantic.Module)
    {
      if (Semantic.Diagnostics.empty())
      {
        for (const sema::SemanticVerificationError &Error : Semantic.Verification.Errors)
        {
          Result.Issues.push_back(makeIssue(CompilationPhase::SemanticAnalysis, File, Error.Message));
        }
      }
      else
      {
        for (const core::Diagnostic &Diagnostic : Semantic.Diagnostics)
        {
          Result.Issues.push_back(makeDiagnosticIssue(CompilationPhase::SemanticAnalysis, Diagnostic));
        }
      }
      return Result;
    }

    IrGenerationResult Generated = generateIr(*Semantic.Module);
    if (!Generated.succeeded() || !Generated.Module)
    {
      for (IrGenerationError &Error : Generated.Errors)
      {
        Result.Issues.push_back(makeIssue(CompilationPhase::IrGeneration, File, std::move(Error.Message), Error.Range));
      }
      return Result;
    }

    ir::IrStagedVerificationResult Verified = ir::verifyStaged(*Generated.Module);
    if (!Verified.succeeded())
    {
      for (const ir::IrVerificationError &Error : Verified.errors())
      {
        const ir::IrOrigin Origin = originForError(Generated.Module->module(), Error);
        Result.Issues.push_back(makeIssue(CompilationPhase::IrVerification, Origin.File.isValid() ? Origin.File : File, Error.Message, Origin.Range));
      }
      return Result;
    }

    Result.PendingForceValues = std::move(Generated.PendingForceValues);
    Result.Module.emplace(Verified.takeVerified());
    return Result;
  }

  ClosedCompilationResult Compiler::close(const ir::VerifiedStagedModule &Module, const std::vector<ir::IrForceValueResolution> &Resolutions) const
  {
    ClosedCompilationResult Result;
    ir::IrClosedVerificationResult Closed = ir::closeAndVerify(Module, Session.targetContext().key(), Resolutions);
    if (!Closed.succeeded())
    {
      for (const ir::IrVerificationError &Error : Closed.errors())
      {
        const ir::IrOrigin Origin = originForError(Module.module(), Error);
        Result.Issues.push_back(makeIssue(CompilationPhase::IrClosure, Origin.File, Error.Message, Origin.Range));
      }
      return Result;
    }
    Result.Module.emplace(Closed.takeVerified());
    return Result;
  }

  ClosedCompilationResult Compiler::compile(core::SourceFileId File) const
  {
    StagedCompilationResult Staged = compileToStaged(File);
    if (!Staged.succeeded() || !Staged.Module)
    {
      ClosedCompilationResult Result;
      Result.Issues = std::move(Staged.Issues);
      return Result;
    }
    if (!Staged.PendingForceValues.empty())
    {
      ClosedCompilationResult Result;
      Result.Issues.push_back(makeIssue(CompilationPhase::IrClosure, File, "staged module contains unresolved force-value plans"));
      return Result;
    }
    return close(*Staged.Module);
  }
} // namespace ink::frontend
