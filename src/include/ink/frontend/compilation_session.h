#ifndef INK_FRONTEND_COMPILATION_SESSION_H
#define INK_FRONTEND_COMPILATION_SESSION_H

#include "ink/ast/ast_context.h"
#include "ink/ast/cst_lowering.h"
#include "ink/core/diagnostic.h"
#include "ink/core/source_file_id.h"
#include "ink/core/source_manager.h"
#include "ink/core/string_interner.h"
#include "ink/parser/parser.h"
#include "ink/sema/analyzer.h"
#include "ink/target/target_context.h"
#include "ink/tokenizer/tokenizer.h"
#include "ink/type/type_context.h"

#include <optional>
#include <string>
#include <vector>

namespace ink::frontend
{
  class CompilationSession
  {
  public:
    explicit CompilationSession(target::TargetContext Target = target::TargetContext::host());
    CompilationSession(const CompilationSession &) = delete;
    CompilationSession &operator=(const CompilationSession &) = delete;
    CompilationSession(CompilationSession &&) = delete;
    CompilationSession &operator=(CompilationSession &&) = delete;

    core::SourceFileId addSource(std::string Path, std::string Contents);
    tokenizer::TokenizedBuffer tokenize(core::SourceFileId File, tokenizer::TokenizerOptions Options = {});
    std::optional<parser::ParsedFile> parse(core::SourceFileId File, tokenizer::TokenizerOptions TokenizerOptions = {}, parser::ParserOptions ParserOptions = {});
    std::optional<ast::AstFile> parseAndLower(core::SourceFileId File, tokenizer::TokenizerOptions TokenizerOptions = {}, parser::ParserOptions ParserOptions = {});
    sema::SemanticAnalysisResult analyze(const ast::AstFile &File);

    core::SourceManager &sourceManager() noexcept;
    const core::SourceManager &sourceManager() const noexcept;
    core::StringInterner &stringInterner() noexcept;
    const core::StringInterner &stringInterner() const noexcept;
    ast::AstContext &astContext() noexcept;
    const ast::AstContext &astContext() const noexcept;
    type::TypeContext &typeContext() noexcept;
    const type::TypeContext &typeContext() const noexcept;
    const target::TargetContext &targetContext() const noexcept;
    const std::vector<core::Diagnostic> &diagnostics() const noexcept;
    void clearDiagnostics() noexcept;

  private:
    void appendDiagnostics(const std::vector<core::Diagnostic> &NewDiagnostics);
    ast::AstFile lowerToAst(const parser::ParsedFile &ParsedFile);

    core::SourceManager Sources;
    core::StringInterner Strings;
    ast::AstContext Ast;
    type::TypeContext Types;
    target::TargetContext Target;
    std::vector<core::Diagnostic> Diagnostics;
  };
} // namespace ink::frontend

#endif
