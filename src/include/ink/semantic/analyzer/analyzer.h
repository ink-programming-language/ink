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

  // Analysis uses Context-owned state and IR; concrete node lowering is not implemented yet.
  class Analyzer
  {
    public:
      // Input must belong to Context's compilation and remain alive for the call.
      // Unusable input returns false; unimplemented nodes report the existing SemanticUnsupported ICE.
      bool analyze(SemanticContext &Context, const frontend::parser::ParseResult &Input);

    private:
      bool analyzeStmt(SemanticContext &Context, const frontend::parser::Stmt &Stmt);
      bool analyzeDecl(SemanticContext &Context, const frontend::parser::Decl &Declaration);
      bool reportUnsupported(SemanticContext &Context, const frontend::parser::ASTNodeBase &Node);

      // Keep handlers explicit so new AST statements and declarations require a deliberate implementation.
      bool analyzeMissingStmt(SemanticContext &Context, const frontend::parser::MissingStmt &Node);
      bool analyzeErrorStmt(SemanticContext &Context, const frontend::parser::ErrorStmt &Node);
      bool analyzeMissingDecl(SemanticContext &Context, const frontend::parser::MissingDecl &Node);
      bool analyzeErrorDecl(SemanticContext &Context, const frontend::parser::ErrorDecl &Node);
      bool analyzeSimpleStmt(SemanticContext &Context, const frontend::parser::SimpleStmt &Node);
      bool analyzeBlockStmt(SemanticContext &Context, const frontend::parser::BlockStmt &Node);
      bool analyzeDeclStmt(SemanticContext &Context, const frontend::parser::DeclStmt &Node);
      bool analyzeIfStmt(SemanticContext &Context, const frontend::parser::IfStmt &Node);
      bool analyzeWhileStmt(SemanticContext &Context, const frontend::parser::WhileStmt &Node);
      bool analyzeClassicForStmt(SemanticContext &Context, const frontend::parser::ClassicForStmt &Node);
      bool analyzeForInStmt(SemanticContext &Context, const frontend::parser::ForInStmt &Node);
      bool analyzeSwitchStmt(SemanticContext &Context, const frontend::parser::SwitchStmt &Node);
      bool analyzeReturnStmt(SemanticContext &Context, const frontend::parser::ReturnStmt &Node);
      bool analyzeBreakStmt(SemanticContext &Context, const frontend::parser::BreakStmt &Node);
      bool analyzeContinueStmt(SemanticContext &Context, const frontend::parser::ContinueStmt &Node);
      bool analyzeYieldStmt(SemanticContext &Context, const frontend::parser::YieldStmt &Node);
      bool analyzeDeferStmt(SemanticContext &Context, const frontend::parser::DeferStmt &Node);
      bool analyzeDirectImportStmt(SemanticContext &Context, const frontend::parser::DirectImportStmt &Node);
      bool analyzeFromImportStmt(SemanticContext &Context, const frontend::parser::FromImportStmt &Node);
      bool analyzeFieldDecl(SemanticContext &Context, const frontend::parser::FieldDecl &Node);
      bool analyzeVarDecl(SemanticContext &Context, const frontend::parser::VarDecl &Node);
      bool analyzeFunctionDecl(SemanticContext &Context, const frontend::parser::FunctionDecl &Node);
      bool analyzeClassDecl(SemanticContext &Context, const frontend::parser::ClassDecl &Node);
      bool analyzeEnumDecl(SemanticContext &Context, const frontend::parser::EnumDecl &Node);
      bool analyzeInterfaceDecl(SemanticContext &Context, const frontend::parser::InterfaceDecl &Node);
  };
} // namespace ink::semantic

#endif
