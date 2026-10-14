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

namespace ink::ir
{
  class Module;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;
  class AnalyzerState;

  // Analysis uses call-local traversal state and caller-owned output; concrete node lowering is not implemented yet.
  class Analyzer
  {
    public:
      // Input must belong to Context's compilation and remain alive for the call.
      // Output must borrow Context.irContext(); failure does not roll back existing output.
      // Unusable input returns false; unimplemented nodes report the existing SemanticUnsupported ICE.
      bool analyze(SemanticContext &Context, const frontend::parser::ParseResult &Input, ir::Module &Output);

    private:
      bool analyzeStmt(AnalyzerState &State, const frontend::parser::Stmt &Stmt);
      bool analyzeDecl(AnalyzerState &State, const frontend::parser::Decl &Declaration);
      bool reportUnsupported(AnalyzerState &State, const frontend::parser::ASTNodeBase &Node);

      // Keep handlers explicit so new AST statements and declarations require a deliberate implementation.
      bool analyzeMissingStmt(AnalyzerState &State, const frontend::parser::MissingStmt &Node);
      bool analyzeErrorStmt(AnalyzerState &State, const frontend::parser::ErrorStmt &Node);
      bool analyzeMissingDecl(AnalyzerState &State, const frontend::parser::MissingDecl &Node);
      bool analyzeErrorDecl(AnalyzerState &State, const frontend::parser::ErrorDecl &Node);
      bool analyzeSimpleStmt(AnalyzerState &State, const frontend::parser::SimpleStmt &Node);
      bool analyzeBlockStmt(AnalyzerState &State, const frontend::parser::BlockStmt &Node);
      bool analyzeDeclStmt(AnalyzerState &State, const frontend::parser::DeclStmt &Node);
      bool analyzeIfStmt(AnalyzerState &State, const frontend::parser::IfStmt &Node);
      bool analyzeWhileStmt(AnalyzerState &State, const frontend::parser::WhileStmt &Node);
      bool analyzeClassicForStmt(AnalyzerState &State, const frontend::parser::ClassicForStmt &Node);
      bool analyzeForInStmt(AnalyzerState &State, const frontend::parser::ForInStmt &Node);
      bool analyzeSwitchStmt(AnalyzerState &State, const frontend::parser::SwitchStmt &Node);
      bool analyzeReturnStmt(AnalyzerState &State, const frontend::parser::ReturnStmt &Node);
      bool analyzeBreakStmt(AnalyzerState &State, const frontend::parser::BreakStmt &Node);
      bool analyzeContinueStmt(AnalyzerState &State, const frontend::parser::ContinueStmt &Node);
      bool analyzeYieldStmt(AnalyzerState &State, const frontend::parser::YieldStmt &Node);
      bool analyzeDeferStmt(AnalyzerState &State, const frontend::parser::DeferStmt &Node);
      bool analyzeDirectImportStmt(AnalyzerState &State, const frontend::parser::DirectImportStmt &Node);
      bool analyzeFromImportStmt(AnalyzerState &State, const frontend::parser::FromImportStmt &Node);
      bool analyzeFieldDecl(AnalyzerState &State, const frontend::parser::FieldDecl &Node);
      bool analyzeVarDecl(AnalyzerState &State, const frontend::parser::VarDecl &Node);
      bool analyzeFunctionDecl(AnalyzerState &State, const frontend::parser::FunctionDecl &Node);
      bool analyzeClassDecl(AnalyzerState &State, const frontend::parser::ClassDecl &Node);
      bool analyzeEnumDecl(AnalyzerState &State, const frontend::parser::EnumDecl &Node);
      bool analyzeInterfaceDecl(AnalyzerState &State, const frontend::parser::InterfaceDecl &Node);
  };
} // namespace ink::semantic

#endif
