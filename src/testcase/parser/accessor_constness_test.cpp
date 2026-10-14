#include "ink/frontend/context.h"
#include "ink/frontend/parser/ast/array_expr.h"
#include "ink/frontend/parser/ast/binary_expr.h"
#include "ink/frontend/parser/ast/missing_expr.h"
#include "ink/frontend/parser/ast/return_stmt.h"
#include "ink/frontend/parser/parser.h"
#include <gtest/gtest.h>
#include <type_traits>
#include <utility>

namespace ink::frontend::parser::test
{
  // Reference members follow the context's constness and remain lvalues even on rvalue contexts.
  static_assert(std::is_same_v<decltype(std::declval<frontend::FrontendContext &>().sourceManager()), core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<const frontend::FrontendContext &>().sourceManager()), const core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<frontend::FrontendContext &&>().sourceManager()), core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<const frontend::FrontendContext &&>().sourceManager()), const core::SourceManager &>);
  static_assert(std::is_same_v<decltype(std::declval<frontend::FrontendContext &>().diagnosticEngine()), core::DiagnosticEngine &>);
  static_assert(std::is_same_v<decltype(std::declval<const frontend::FrontendContext &>().diagnosticEngine()), const core::DiagnosticEngine &>);

  // Raw child pointers propagate owner constness without changing their pointer return category.
  static_assert(std::is_same_v<decltype(std::declval<BinaryExpr &>().left()), Expr *>);
  static_assert(std::is_same_v<decltype(std::declval<const BinaryExpr &>().left()), const Expr *>);
  static_assert(std::is_same_v<decltype(std::declval<BinaryExpr &&>().left()), Expr *>);
  static_assert(std::is_same_v<decltype(std::declval<const BinaryExpr &&>().left()), const Expr *>);
  static_assert(std::is_same_v<decltype(std::declval<ParsedUnit &>().root()), ModuleAST *>);
  static_assert(std::is_same_v<decltype(std::declval<const ParsedUnit &>().root()), const ModuleAST *>);
  static_assert(std::is_same_v<decltype(std::declval<ParsedUnit &&>().root()), ModuleAST *>);
  static_assert(std::is_same_v<decltype(std::declval<const ParsedUnit &&>().root()), const ModuleAST *>);

  // Array access retains the existing span and transitive-const view types for every owner category.
  static_assert(std::is_same_v<decltype(std::declval<ArrayExpr &>().elements()), ASTArray<Expr *>>);
  static_assert(std::is_same_v<decltype(std::declval<const ArrayExpr &>().elements()), ConstNodeArray<Expr>>);
  static_assert(std::is_same_v<decltype(std::declval<ArrayExpr &&>().elements()), ASTArray<Expr *>>);
  static_assert(std::is_same_v<decltype(std::declval<const ArrayExpr &&>().elements()), ConstNodeArray<Expr>>);
  static_assert(std::is_same_v<decltype(std::declval<ArrayExpr &>().elements()[0]), Expr *const &>);
  static_assert(std::is_same_v<decltype(std::declval<const ArrayExpr &>().elements()[0]), const Expr *>);
  static_assert(noexcept(std::declval<const frontend::FrontendContext &>().sourceManager()));
  static_assert(noexcept(std::declval<const BinaryExpr &>().left()));
  static_assert(noexcept(std::declval<const ParsedUnit &>().root()));
  static_assert(noexcept(std::declval<const ArrayExpr &>().elements()));

  // Optional children stay null and non-null children preserve identity through const and rvalue access.
  TEST(ASTAccessorTest, NullableChildPointersPreserveIdentity)
  {
    ReturnStmt Empty({}, nullptr);
    EXPECT_EQ(Empty.value(), nullptr);
    EXPECT_EQ(std::as_const(Empty).value(), nullptr);
    EXPECT_EQ(std::move(Empty).value(), nullptr);
    EXPECT_EQ(std::move(std::as_const(Empty)).value(), nullptr);

    MissingExpr Value(SourceRange{});
    ReturnStmt Return({}, &Value);
    EXPECT_EQ(Return.value(), &Value);
    EXPECT_EQ(std::as_const(Return).value(), &Value);
    EXPECT_EQ(std::move(Return).value(), &Value);
    EXPECT_EQ(std::move(std::as_const(Return)).value(), &Value);
  }

  // Mutable and const array views share the original storage without copying nodes or replacing null entries.
  TEST(ASTAccessorTest, ArrayViewsPreserveBorrowedStorage)
  {
    MissingExpr First(SourceRange{});
    MissingExpr Second(SourceRange{});
    Expr *Elements[] = {&First, nullptr};
    ArrayExpr Array({}, Elements);
    const auto Mutable = Array.elements();
    const auto ReadOnly = std::as_const(Array).elements();
    ASSERT_EQ(Mutable.size(), 2U);
    ASSERT_EQ(ReadOnly.size(), 2U);
    EXPECT_EQ(Mutable.data(), Elements);
    EXPECT_EQ(ReadOnly[0], &First);
    EXPECT_EQ(ReadOnly[1], nullptr);
    Elements[0] = &Second;
    EXPECT_EQ(Mutable[0], &Second);
    EXPECT_EQ(ReadOnly[0], &Second);
    EXPECT_EQ(std::move(Array).elements()[0], &Second);
    EXPECT_EQ(std::move(std::as_const(Array)).elements()[0], &Second);

    ArrayExpr Empty({}, {});
    EXPECT_TRUE(Empty.elements().empty());
    EXPECT_TRUE(std::as_const(Empty).elements().empty());
  }
} // namespace ink::frontend::parser::test
