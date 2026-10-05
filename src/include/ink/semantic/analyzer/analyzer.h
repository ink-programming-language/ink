#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_H

namespace ink::parser
{
  class Stmt;
  class Decl;
#define AST_NODE(Name, Base, Category, Id) class Name;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
} // namespace ink::parser

namespace ink::semantic
{
  // Preserve AST dispatch while analysis state and semantic behavior are rebuilt.
  class Analyzer
  {
    private:
      struct AnalysisState;

      bool analyzeStmt(AnalysisState &State, const parser::Stmt &Stmt);
      bool analyzeDecl(AnalysisState &State, const parser::Decl &Declaration);

      // Keep handlers explicit so new AST statements and declarations require a deliberate implementation.
      bool analyzeMissingStmt(AnalysisState &State, const parser::MissingStmt &Node);
      bool analyzeErrorStmt(AnalysisState &State, const parser::ErrorStmt &Node);
      bool analyzeMissingDecl(AnalysisState &State, const parser::MissingDecl &Node);
      bool analyzeErrorDecl(AnalysisState &State, const parser::ErrorDecl &Node);
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
      bool analyzeDirectImportStmt(AnalysisState &State, const parser::DirectImportStmt &Node);
      bool analyzeFromImportStmt(AnalysisState &State, const parser::FromImportStmt &Node);
      bool analyzeFieldDecl(AnalysisState &State, const parser::FieldDecl &Node);
      bool analyzeVarDecl(AnalysisState &State, const parser::VarDecl &Node);
      bool analyzeFunctionDecl(AnalysisState &State, const parser::FunctionDecl &Node);
      bool analyzeClassDecl(AnalysisState &State, const parser::ClassDecl &Node);
      bool analyzeEnumDecl(AnalysisState &State, const parser::EnumDecl &Node);
      bool analyzeInterfaceDecl(AnalysisState &State, const parser::InterfaceDecl &Node);
  };
} // namespace ink::semantic

#endif
