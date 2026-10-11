#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_H

namespace ink::frontend::parser
{
  struct ParseResult;
  class ASTNodeBase;
  class Stmt;
  class Decl;
#define AST_NODE(Name, Base, Category, Id) class Name;
#include "ink/frontend/parser/ASTNodes.def"
#undef AST_NODE
} // namespace ink::frontend::parser

namespace ink::semantic
{
  class SemanticContext;

  // Analysis currently traverses module/block structure and dispatches concrete nodes without producing IR.
  class Analyzer
  {
    public:
      // Input must belong to Context's compilation and remain alive for the call.
      // Unusable input returns false; unimplemented nodes report the existing SemanticUnsupported ICE.
      bool analyze(SemanticContext &Context, const frontend::parser::ParseResult &Input);

    private:
      struct AnalysisState;

      bool analyzeStmt(AnalysisState &State, const frontend::parser::Stmt &Stmt);
      bool analyzeDecl(AnalysisState &State, const frontend::parser::Decl &Declaration);
      bool reportUnsupported(AnalysisState &State, const frontend::parser::ASTNodeBase &Node);

      // Keep handlers explicit so new AST statements and declarations require a deliberate implementation.
      bool analyzeMissingStmt(AnalysisState &State, const frontend::parser::MissingStmt &Node);
      bool analyzeErrorStmt(AnalysisState &State, const frontend::parser::ErrorStmt &Node);
      bool analyzeMissingDecl(AnalysisState &State, const frontend::parser::MissingDecl &Node);
      bool analyzeErrorDecl(AnalysisState &State, const frontend::parser::ErrorDecl &Node);
      bool analyzeSimpleStmt(AnalysisState &State, const frontend::parser::SimpleStmt &Node);
      bool analyzeBlockStmt(AnalysisState &State, const frontend::parser::BlockStmt &Node);
      bool analyzeDeclStmt(AnalysisState &State, const frontend::parser::DeclStmt &Node);
      bool analyzeIfStmt(AnalysisState &State, const frontend::parser::IfStmt &Node);
      bool analyzeWhileStmt(AnalysisState &State, const frontend::parser::WhileStmt &Node);
      bool analyzeClassicForStmt(AnalysisState &State, const frontend::parser::ClassicForStmt &Node);
      bool analyzeForInStmt(AnalysisState &State, const frontend::parser::ForInStmt &Node);
      bool analyzeSwitchStmt(AnalysisState &State, const frontend::parser::SwitchStmt &Node);
      bool analyzeReturnStmt(AnalysisState &State, const frontend::parser::ReturnStmt &Node);
      bool analyzeBreakStmt(AnalysisState &State, const frontend::parser::BreakStmt &Node);
      bool analyzeContinueStmt(AnalysisState &State, const frontend::parser::ContinueStmt &Node);
      bool analyzeYieldStmt(AnalysisState &State, const frontend::parser::YieldStmt &Node);
      bool analyzeDeferStmt(AnalysisState &State, const frontend::parser::DeferStmt &Node);
      bool analyzeDirectImportStmt(AnalysisState &State, const frontend::parser::DirectImportStmt &Node);
      bool analyzeFromImportStmt(AnalysisState &State, const frontend::parser::FromImportStmt &Node);
      bool analyzeFieldDecl(AnalysisState &State, const frontend::parser::FieldDecl &Node);
      bool analyzeVarDecl(AnalysisState &State, const frontend::parser::VarDecl &Node);
      bool analyzeFunctionDecl(AnalysisState &State, const frontend::parser::FunctionDecl &Node);
      bool analyzeClassDecl(AnalysisState &State, const frontend::parser::ClassDecl &Node);
      bool analyzeEnumDecl(AnalysisState &State, const frontend::parser::EnumDecl &Node);
      bool analyzeInterfaceDecl(AnalysisState &State, const frontend::parser::InterfaceDecl &Node);
  };
} // namespace ink::semantic

#endif
