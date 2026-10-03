#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_H

#include "ink/ir/coredefines.h"
#include "ink/execution/support/execution_result.h"
#include "ink/tokenizer/token.h"

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <vector>

namespace ink::parser
{
  struct ParseResult;
  class ASTNodeBase;
  class Stmt;
  class Decl;
  class Expr;
  class SimpleItem;
#define AST_NODE(Name, Base, Category, Id) class Name;
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
} // namespace ink::parser

namespace ink::ir
{
  class Module;
  class Type;
  class FunctionType;
  class Value;
  class Function;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;

  class Analyzer
  {
    public:
      struct ModuleInput
      {
          std::string_view Name;
          const parser::ParseResult *Input = nullptr;
      };

      // Builds a module from successfully parsed input. Unsupported syntax reports
      // an ICE. User errors return null. Source-ordered comptime evaluation uses
      // context-owned execution frames; runtime bodies support locals, calls, addition, logic, comparisons and branches.
      // The returned module is owned by Context. Analysis state is local to each call.
      // ir::Module declaration roots borrow Input's AST; its ParsedUnit must outlive any allocated module, including on failure.
      ir::Module *analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName = "main");
      // Predeclares all top-level function signatures before resolving imports and checking bodies.
      // Every input AST must outlive Context, including when analysis fails.
      ir::Module *analyzeModules(SemanticContext &Context, std::span<const ModuleInput> Inputs, std::string_view EntryModuleName);

    private:
      struct AnalysisState;
      struct ExpressionResult;
      struct ModuleAnalysis;
      struct ModuleGraph;

      bool analyzeStmt(AnalysisState &State, const parser::Stmt &Stmt);
      bool analyzeDecl(AnalysisState &State, const parser::Decl &Declaration);
      bool reportUnsupported(AnalysisState &State, const parser::ASTNodeBase &Node);
      const ir::Type *analyzeType(AnalysisState &State, const parser::Expr &Node, std::size_t Depth = 0);
      ExpressionResult analyzeExpr(AnalysisState &State, const parser::Expr &Node, std::size_t Depth = 0);
      ExpressionResult analyzeParenExpr(AnalysisState &State, const parser::ParenExpr &Node, std::size_t Depth);
      ExpressionResult analyzeLiteralExpr(AnalysisState &State, const parser::LiteralExpr &Node);
      ExpressionResult analyzeNameExpr(AnalysisState &State, const parser::NameExpr &Node);
      ExpressionResult analyzeMemberExpr(AnalysisState &State, const parser::MemberExpr &Node, std::size_t Depth);
      bool resolveMemberFunctions(AnalysisState &State, const parser::MemberExpr &Node, std::vector<const ir::Value *> &Functions, std::size_t Depth);
      ExpressionResult analyzeUnaryExpr(AnalysisState &State, const parser::UnaryExpr &Node, std::size_t Depth);
      ExpressionResult analyzeArrayExpr(AnalysisState &State, const parser::Expr &Node, std::size_t Depth);
      ExpressionResult analyzeIndexExpr(AnalysisState &State, const parser::IndexExpr &Node, std::size_t Depth);
      const ir::Value *analyzeArrayIndex(AnalysisState &State, const parser::Expr &Node, std::uint64_t Count, std::size_t Depth);
      std::optional<std::uint64_t> analyzeArrayLength(AnalysisState &State, const parser::Expr &Node, std::size_t Depth);
      bool checkArrayMaterialization(AnalysisState &State, const ir::Type &Element, std::uint64_t Count, const parser::ASTNodeBase &Node);
      ExpressionResult assignComptimeArray(AnalysisState &State, const parser::AssignmentItem &Node, std::size_t Depth);
      bool prepareIntegerLiteral(AnalysisState &State, const parser::Expr &Node, std::size_t Depth);
      ExpressionResult materializeIntegerLiteral(AnalysisState &State, const parser::Expr &Node, const ir::Type &Target);
      ExpressionResult analyzeCallExpr(AnalysisState &State, const parser::CallExpr &Node, std::size_t Depth);
      ExpressionResult analyzeBinaryExpr(AnalysisState &State, const parser::BinaryExpr &Node, std::size_t Depth);
      ExpressionResult analyzeLogicalBinaryExpr(AnalysisState &State, const parser::BinaryExpr &Node, std::size_t Depth);
      ExpressionResult analyzeComparisonExpr(AnalysisState &State, const parser::BinaryExpr &Node, std::size_t Depth);
      ExpressionResult analyzeUpdateExpr(AnalysisState &State, const parser::Expr &Operand, tokenizer::TokenKind Operator, bool Postfix, const parser::Expr &Node, std::size_t Depth);
      ExpressionResult analyzeSimpleItem(AnalysisState &State, const parser::SimpleItem &Node, std::size_t Depth = 0);
      ExpressionResult evaluateComptime(AnalysisState &State, const parser::Expr &Node);
      ExpressionResult callComptime(AnalysisState &State, const ir::Function &Function, std::span<const ir::Value *const> Arguments, const parser::Expr &Node);
      const ir::Value *resolveVariable(AnalysisState &State, const parser::Expr &Node);
      const ir::Value *resolveAddress(AnalysisState &State, const parser::Expr &Node, std::size_t Depth, bool RequireInitialized = true);
      bool reportExecution(AnalysisState &State, execution::ExecutionStatus Status, const parser::ASTNodeBase &Node);
      const ir::Value *convertExpression(AnalysisState &State, const ExpressionResult &Expression, const ir::Type &Target, const parser::Expr &Node, bool CArgument = false);

      // Keep handlers explicit: a new AST statement/declaration must choose its behavior.
      // Recovery nodes remain unsupported and are handled in analyzer.cpp.
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

      bool analyzeVarDecl(AnalysisState &State, const parser::VarDecl &Node);
      bool analyzeFieldDecl(AnalysisState &State, const parser::FieldDecl &Node);

      bool analyzeFunctionDecl(AnalysisState &State, const parser::FunctionDecl &Node);
      std::unique_ptr<ir::Function> declareFunction(AnalysisState &State, const parser::FunctionDecl &Node);
      bool analyzeFunctionBody(AnalysisState &State, const parser::FunctionDecl &Node, ir::Function &FunctionValue);
      bool ensureModuleFunctionBody(AnalysisState &State, const ir::Function &FunctionValue, const parser::ASTNodeBase &Use);
      bool prepareComptimeFunctions(AnalysisState &State, const ir::Function &FunctionValue, const parser::ASTNodeBase &Use);
      bool analyzeModuleStatement(ModuleAnalysis &Module, const parser::Stmt &Statement);
      std::optional<ir::LanguageLinkage> analyzeFunctionLinkage(AnalysisState &State, const parser::FunctionDecl &Node);
      bool checkFunctionConflicts(AnalysisState &State, const parser::FunctionDecl &Node, const ir::FunctionType &Signature, ir::FunctionBinding Binding);
      bool analyzeClassDecl(AnalysisState &State, const parser::ClassDecl &Node);
      bool analyzeEnumDecl(AnalysisState &State, const parser::EnumDecl &Node);
      bool analyzeInterfaceDecl(AnalysisState &State, const parser::InterfaceDecl &Node);
  };
} // namespace ink::semantic

#endif
