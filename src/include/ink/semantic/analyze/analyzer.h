#ifndef INK_SEMANTIC_ANALYZE_ANALYZER_H
#define INK_SEMANTIC_ANALYZE_ANALYZER_H

#include <string_view>

namespace ink::parser
{
  struct ParseResult;
  class ASTNodeBase;
  class Stmt;
  class Decl;
#define AST_NODE(Name, Base, Category, Id) class Name;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
} // namespace ink::parser

namespace ink::semantic
{
  class SemanticContext;
  class Module;

  class Analyzer
  {
    public:
      // Builds a module from successfully parsed input. Unsupported syntax reports
      // a diagnostic and returns null; only empty modules/blocks are currently supported.
      // The returned module is owned by Context. Analysis state is local to each call.
      Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName = "main");

    private:
      struct AnalysisState;

      bool analyzeStmt(AnalysisState &State, const parser::Stmt &Stmt);
      bool analyzeDecl(AnalysisState &State, const parser::Decl &Declaration);
      bool reportUnsupported(AnalysisState &State, const parser::ASTNodeBase &Node);

      // Keep handlers explicit: a new AST statement/declaration must choose its behavior.
      bool analyzeMissingStmt(AnalysisState &State, const parser::MissingStmt &Node);
      bool analyzeErrorStmt(AnalysisState &State, const parser::ErrorStmt &Node);
      bool analyzeSimpleStmt(AnalysisState &State, const parser::SimpleStmt &Node);
      bool analyzeBlockStmt(AnalysisState &State, const parser::BlockStmt &Node);
      bool analyzeDeclStmt(AnalysisState &State, const parser::DeclStmt &Node);
      bool analyzeIfStmt(AnalysisState &State, const parser::IfStmt &Node);
      bool analyzeWhileStmt(AnalysisState &State, const parser::WhileStmt &Node);
      bool analyzeClassicForStmt(AnalysisState &State, const parser::ClassicForStmt &Node);
      bool analyzeForInStmt(AnalysisState &State, const parser::ForInStmt &Node);
      bool analyzeSwitchStmt(AnalysisState &State, const parser::SwitchStmt &Node);
      bool analyzeReturnStmt(AnalysisState &State, const parser::ReturnStmt &Node);
      bool analyzeBreakStmt(AnalysisState &State, const parser::BreakStmt &Node);
      bool analyzeContinueStmt(AnalysisState &State, const parser::ContinueStmt &Node);
      bool analyzeYieldStmt(AnalysisState &State, const parser::YieldStmt &Node);
      bool analyzeDeferStmt(AnalysisState &State, const parser::DeferStmt &Node);
      bool analyzeComptimeStmt(AnalysisState &State, const parser::ComptimeStmt &Node);
      bool analyzeDirectImportStmt(AnalysisState &State, const parser::DirectImportStmt &Node);
      bool analyzeFromImportStmt(AnalysisState &State, const parser::FromImportStmt &Node);
      bool analyzeMissingDecl(AnalysisState &State, const parser::MissingDecl &Node);
      bool analyzeErrorDecl(AnalysisState &State, const parser::ErrorDecl &Node);
      bool analyzeVarDecl(AnalysisState &State, const parser::VarDecl &Node);
      bool analyzeFieldDecl(AnalysisState &State, const parser::FieldDecl &Node);
      bool analyzeFunctionDecl(AnalysisState &State, const parser::FunctionDecl &Node);
      bool analyzeClassDecl(AnalysisState &State, const parser::ClassDecl &Node);
      bool analyzeEnumDecl(AnalysisState &State, const parser::EnumDecl &Node);
      bool analyzeInterfaceDecl(AnalysisState &State, const parser::InterfaceDecl &Node);
  };
} // namespace ink::semantic

#endif
