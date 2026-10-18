#ifndef INK_SEMANTIC_ANALYZER_ANALYZER_H
#define INK_SEMANTIC_ANALYZER_ANALYZER_H

#include "ink/semantic/analyzer/expr_result.h"

#include <string_view>

namespace ink::core
{
  class SourceRange;
} // namespace ink::core

namespace ink::frontend::parser
{
  struct ParseResult;
  class ASTNodeBase;
  class Stmt;
  class Decl;
  class Expr;
#define AST_NODE(Name, Base, Category, Id) class Name;
#include "ink/frontend/parser/ASTNodes.def"
#undef AST_NODE
} // namespace ink::frontend::parser

namespace ink::ir
{
  class IntegerType;
  class Module;
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::semantic
{
  class SemanticContext;
  class AnalyzerState;
  class Binding;

  // Supplies call-local contextual typing; each child expression explicitly inherits or replaces these hints.
  // Consuming positions validate runtime layout requirements and report their own semantic diagnostics.
  struct ExprHints
  {
      const ir::Type *ExpectedType = nullptr;
      const ir::IntegerType *DefaultIntegerLiteralType = nullptr;
      // Only transparent parentheses propagate callee context to a final name or member binding.
      bool IsCallCallee = false;
  };

  // Analysis uses call-local traversal state and caller-owned output; unsupported nodes report SemanticUnsupported.
  class Analyzer
  {
    public:
      // Input must belong to Context's compilation and remain alive for the call.
      // Output must borrow Context.irContext(); failure does not roll back existing output.
      // Scope bindings borrow Output's declarations; remove those targets from Context.scopeStore() before destroying them.
      // Unusable input returns false; unimplemented nodes report the existing SemanticUnsupported ICE.
      bool analyze(SemanticContext &Context, const frontend::parser::ParseResult &Input, ir::Module &Output);

      // Resolves a type annotation belonging to State's input in its current name scope.
      // The caller validates input/context compatibility and selects the diagnostic source; a missing expression reports SemanticInvalidAST ICE.
      // Returns a borrowed context/module-owned type, or nullptr after a diagnostic.
      // Unimplemented expressions and nesting limits use the existing ICE policy.
      const ir::Type *analyzeType(AnalyzerState &State, const frontend::parser::TypeSyntax &Node);

      // Analyzes an expression in State's current scope, with the same input/source preconditions as analyzeType().
      // Non-null hint types must belong to the current IR context; violations report SemanticConstructionFailed ICE.
      // Hints.ExpectedType is optional; integer literals default to i32,
      // floating literals to f64 and character literals to u32. Strings retain their decoded [u8; byteCount] type.
      // A non-null target materializes literals in that type and checks other values without implicit numeric conversions.
      // Call arguments, return values and variable initializers supply their declared target types.
      // Returns an explicit rvalue, lvalue, type or module result without loading lvalue storage.
      // Lvalue Type describes the stored object; consumers use toRValue() to read its contents.
      // Hints.DefaultIntegerLiteralType changes only uncontextualized integer literals and must belong to the current IR context.
      // Parentheses and numeric negation preserve both type hints; other nested operands select their own hints.
      // Hints.IsCallCallee marks the final callee binding and propagates only through parentheses.
      // Consumers validate the returned value's suitability for their position; names report SemanticUnknownName uniformly.
      // Runtime layout is required by individual consumers, not by the presence of an expected-type hint.
      // Failure returns an empty result after a diagnostic; unimplemented expressions, nesting and comptime use the existing ICE policy.
      ExprResult analyzeExpr(AnalyzerState &State, const frontend::parser::Expr &Node, ExprHints Hints = {});

    private:
      bool analyzeStmt(AnalyzerState &State, const frontend::parser::Stmt &Stmt);
      bool analyzeDecl(AnalyzerState &State, const frontend::parser::Decl &Declaration);
      bool reportUnsupported(AnalyzerState &State, const frontend::parser::ASTNodeBase &Node);
      bool reportUnsupported(AnalyzerState &State, core::SourceRange Range, std::string_view Description);

      // Converts an lvalue to an rvalue, checking initialization and emitting one load; rvalues pass through.
      ExprResult toRValue(AnalyzerState &State, core::SourceRange Range, ExprResult Result);
      ExprResult analyzeBinding(AnalyzerState &State, const frontend::parser::ASTNodeBase &Node, const Binding &Found, bool IsCallCallee = false);
      ExprResult analyzeNameExpr(AnalyzerState &State, const frontend::parser::NameExpr &Node, bool IsCallCallee = false);
      ExprResult analyzeLiteralExpr(AnalyzerState &State, const frontend::parser::LiteralExpr &Node, ExprHints Hints);
      ExprResult analyzeParenExpr(AnalyzerState &State, const frontend::parser::ParenExpr &Node, ExprHints Hints);
      ExprResult analyzeMemberExpr(AnalyzerState &State, const frontend::parser::MemberExpr &Node, bool IsCallCallee = false);
      ExprResult analyzeCallExpr(AnalyzerState &State, const frontend::parser::CallExpr &Node);
      ExprResult analyzeUnaryExpr(AnalyzerState &State, const frontend::parser::UnaryExpr &Node, ExprHints Hints);
      ExprResult analyzeArrayRepeatExpr(AnalyzerState &State, const frontend::parser::ArrayRepeatExpr &Node);
      ExprResult analyzeFunctionTypeExpr(AnalyzerState &State, const frontend::parser::FunctionTypeExpr &Node);

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
