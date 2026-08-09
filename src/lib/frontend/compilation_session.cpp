#include "ink/frontend/compilation_session.h"

#include <stdexcept>
#include <utility>

namespace ink::frontend
{
  CompilationSession::CompilationSession(target::TargetContext Target) : Target(std::move(Target))
  {
  }

  core::SourceFileId CompilationSession::addSource(std::string Path, std::string Contents)
  {
    return Sources.addSourceFile(std::move(Path), std::move(Contents));
  }

  tokenizer::TokenizedBuffer CompilationSession::tokenize(core::SourceFileId File, tokenizer::TokenizerOptions Options)
  {
    const core::SourceFile &Source = Sources.sourceFile(File);
    tokenizer::TokenizedBuffer Result = tokenizer::tokenize(File, Source.Contents, Options);
    appendDiagnostics(Result.diagnostics());
    return Result;
  }

  std::optional<parser::ParsedFile> CompilationSession::parse(core::SourceFileId File, tokenizer::TokenizerOptions TokenizerOptions, parser::ParserOptions ParserOptions)
  {
    const core::SourceFile &Source = Sources.sourceFile(File);
    tokenizer::TokenizedBuffer LexedFile = tokenizer::tokenize(File, Source.Contents, TokenizerOptions);
    appendDiagnostics(LexedFile.diagnostics());
    if (!LexedFile.succeeded())
    {
      return std::nullopt;
    }

    parser::ParsedFile Result = parser::parse(std::move(LexedFile), ParserOptions);
    appendDiagnostics(Result.diagnostics());
    return Result;
  }

  ast::AstFile CompilationSession::lowerToAst(const parser::ParsedFile &ParsedFile)
  {
    if (!Sources.contains(ParsedFile.sourceFileId()) || Sources.sourceFile(ParsedFile.sourceFileId()).Contents != ParsedFile.lexedFile().source())
    {
      throw std::invalid_argument("parsed file does not belong to this compilation session");
    }
    return ast::lowerCst(ParsedFile, ParsedFile.sourceFileId(), Ast, Strings);
  }

  std::optional<ast::AstFile> CompilationSession::parseAndLower(core::SourceFileId File, tokenizer::TokenizerOptions TokenizerOptions, parser::ParserOptions ParserOptions)
  {
    std::optional<parser::ParsedFile> ParsedFile = parse(File, TokenizerOptions, ParserOptions);
    if (!ParsedFile)
    {
      return std::nullopt;
    }
    return lowerToAst(*ParsedFile);
  }

  sema::SemanticAnalysisResult CompilationSession::analyze(const ast::AstFile &File)
  {
    if (!Sources.contains(File.sourceFile()))
    {
      throw std::invalid_argument("AST file does not belong to this compilation session");
    }
    sema::SemanticAnalysisResult Result = sema::analyze(Ast, File, Strings, Types);
    appendDiagnostics(Result.Diagnostics);
    return Result;
  }

  core::SourceManager &CompilationSession::sourceManager() noexcept
  {
    return Sources;
  }

  const core::SourceManager &CompilationSession::sourceManager() const noexcept
  {
    return Sources;
  }

  core::StringInterner &CompilationSession::stringInterner() noexcept
  {
    return Strings;
  }

  const core::StringInterner &CompilationSession::stringInterner() const noexcept
  {
    return Strings;
  }

  ast::AstContext &CompilationSession::astContext() noexcept
  {
    return Ast;
  }

  const ast::AstContext &CompilationSession::astContext() const noexcept
  {
    return Ast;
  }

  type::TypeContext &CompilationSession::typeContext() noexcept
  {
    return Types;
  }

  const type::TypeContext &CompilationSession::typeContext() const noexcept
  {
    return Types;
  }

  const target::TargetContext &CompilationSession::targetContext() const noexcept
  {
    return Target;
  }

  const std::vector<core::Diagnostic> &CompilationSession::diagnostics() const noexcept
  {
    return Diagnostics;
  }

  void CompilationSession::clearDiagnostics() noexcept
  {
    Diagnostics.clear();
  }

  void CompilationSession::appendDiagnostics(const std::vector<core::Diagnostic> &NewDiagnostics)
  {
    Diagnostics.insert(Diagnostics.end(), NewDiagnostics.begin(), NewDiagnostics.end());
  }
} // namespace ink::frontend
