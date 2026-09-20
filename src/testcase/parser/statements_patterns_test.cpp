#include "parser_test_support.h"
namespace ink::parser::test
{
  // Dangling else binds to the closest if, including through while and for bodies.
  TEST_F(ParserTest, DanglingElse)
  {
    const auto Result = read("if (a) while (b) if (c) x(); else y(); if (a) for (;;) if (b) {} else {} else {}");
    ASSERT_TRUE(Result.succeeded());
    const auto *Outer = cast<IfStmt>(Result.Unit->root()->statements()[0]);
    EXPECT_EQ(Outer->elseBranch(), nullptr);
    const auto *Inner = cast<IfStmt>(cast<WhileStmt>(Outer->thenBranch())->body());
    EXPECT_NE(Inner->elseBranch(), nullptr);
    EXPECT_NE(cast<IfStmt>(Result.Unit->root()->statements()[1])->elseBranch(), nullptr);
  }

  // Strict for-in trials roll back completely for classic headers with array and tuple expressions.
  TEST_F(ParserTest, ForHeaderDisambiguation)
  {
    const auto Result = read("for (var i = 0; i < n; i++, j += 2) {} for ([x, rest...] in xs) {} for ((a,b); test; a = b = c) {} for ([1,2];; ) {} for (;;) {}");
    ASSERT_TRUE(Result.succeeded());
    const auto *Classic = cast<ClassicForStmt>(Result.Unit->root()->statements()[0]);
    EXPECT_TRUE(isa<DeclStmt>(Classic->initializer()));
    EXPECT_EQ(Classic->step().size(), 2U);
    const auto *In = cast<ForInStmt>(Result.Unit->root()->statements()[1]);
    EXPECT_TRUE(isa<ArrayBindingPattern>(In->binding()));
    EXPECT_TRUE(isa<ClassicForStmt>(Result.Unit->root()->statements()[2]));
    EXPECT_TRUE(Result.Unit->recoveryInfo().Entries.empty());
    const auto Error = read("for ([x] in ) {} var kept = 1;");
    EXPECT_FALSE(Error.succeeded());
    EXPECT_TRUE(isa<ForInStmt>(Error.Unit->root()->statements()[0]));
    EXPECT_EQ(Error.Unit->root()->statements().size(), 2U);
  }

  // Switch clauses preserve adjacent labels and ordered statements; all basic statements remain distinct.
  TEST_F(ParserTest, SwitchAndBasicStatements)
  {
    const auto Result = read("switch (x) { case 1: case 2: defer f(); break; default: defer { f(); } continue; return; yield x; yield return x; }");
    ASSERT_TRUE(Result.succeeded());
    const auto *Switch = cast<SwitchStmt>(Result.Unit->root()->statements()[0]);
    ASSERT_EQ(Switch->clauses().size(), 3U);
    EXPECT_TRUE(Switch->clauses()[0].statements().empty());
    EXPECT_TRUE(Switch->clauses()[2].isDefault());
    const auto Statements = Switch->clauses()[2].statements();
    ASSERT_EQ(Statements.size(), 5U);
    EXPECT_TRUE(isa<BlockStmt>(cast<DeferStmt>(Statements[0])->body()));
    EXPECT_EQ(cast<ReturnStmt>(Statements[2])->value(), nullptr);
    EXPECT_FALSE(cast<YieldStmt>(Statements[3])->isReturn());
    EXPECT_TRUE(cast<YieldStmt>(Statements[4])->isReturn());
  }

  // Comptime modifiers are restricted to grammar-approved statements and standalone semicolons stay invalid.
  TEST_F(ParserTest, StatementRestrictions)
  {
    for (const auto *Source : {"comptime {}", "comptime var x = 1;", "comptime import foo;", "comptime while (x) {}", "comptime for (;;) {}", "comptime switch(x) {}", "comptime [tag] var x;"})
    {
      EXPECT_TRUE(read(Source).succeeded()) << Source;
    }
    for (const auto *Source : {";", "{};", "comptime return 1;", "comptime yield x;", "comptime break;", "comptime continue;", "defer return x;", "yield;", "yield return;"})
    {
      EXPECT_FALSE(read(Source).succeeded()) << Source;
    }
  }

  // Imports retain path segments, aliases and relative levels counted in dots, including ellipsis tokens.
  TEST_F(ParserTest, ImportForms)
  {
    const auto Result = read("import a.b as c, d; from ....pkg import x as y, z; from ... import q;");
    ASSERT_TRUE(Result.succeeded());
    const auto *Direct = cast<DirectImportStmt>(Result.Unit->root()->statements()[0]);
    ASSERT_EQ(Direct->imports().size(), 2U);
    EXPECT_EQ(Direct->imports()[0].path().size(), 2U);
    EXPECT_EQ(Direct->imports()[0].alias()->Text, "c");
    const auto *From = cast<FromImportStmt>(Result.Unit->root()->statements()[1]);
    EXPECT_EQ(From->relativeLevel(), 4U);
    EXPECT_EQ(From->relativeTokens().size(), 2U);
    EXPECT_EQ(From->path()[0].Text, "pkg");
    EXPECT_EQ(cast<FromImportStmt>(Result.Unit->root()->statements()[2])->relativeLevel(), 3U);
    for (const auto *Source : {"import;", "from import x;", "import a,;", "from a import b.c;"})
    {
      EXPECT_FALSE(read(Source).succeeded()) << Source;
    }
  }

  // Match paths retain per-segment generic arguments and distinguish absent constructor arguments from empty ones.
  TEST_F(ParserTest, MatchPathAndConstructorPresence)
  {
    const auto Result = read("match(x) { ns::[T].Some::[U](a) if (ok) => a, None() => 0, None => 1 };");
    ASSERT_TRUE(Result.succeeded());
    const auto *Match = cast<MatchExpr>(expression(Result));
    ASSERT_EQ(Match->arms().size(), 3U);
    const auto *Path = cast<NameMatchPattern>(Match->arms()[0].pattern());
    ASSERT_EQ(Path->path().size(), 2U);
    EXPECT_TRUE(Path->path()[0].hasGenericArguments());
    EXPECT_TRUE(Path->path()[1].hasGenericArguments());
    EXPECT_EQ(Path->arguments().size(), 1U);
    EXPECT_NE(Match->arms()[0].guard(), nullptr);
    EXPECT_TRUE(cast<NameMatchPattern>(Match->arms()[1].pattern())->hasArguments());
    EXPECT_FALSE(cast<NameMatchPattern>(Match->arms()[2].pattern())->hasArguments());
  }

  // Match patterns include literals, grouping, tuples, arrays and or patterns without extending binding grammar.
  TEST_F(ParserTest, MatchPatternForms)
  {
    const auto Result = read("match(x) { -1 | 2.5 | 'c' | \"s\" => 0, (x) => x, () => 0, (x,) => x, (x,y) => y, [] => 0, [a, _...] => a, _ => 0 };");
    ASSERT_TRUE(Result.succeeded());
    const auto Arms = cast<MatchExpr>(expression(Result))->arms();
    EXPECT_EQ(cast<OrMatchPattern>(Arms[0].pattern())->alternatives().size(), 4U);
    EXPECT_TRUE(isa<GroupedMatchPattern>(Arms[1].pattern()));
    EXPECT_TRUE(cast<TupleMatchPattern>(Arms[2].pattern())->elements().empty());
    EXPECT_TRUE(cast<ArrayMatchPattern>(Arms[6].pattern())->rest()->Wildcard);
    for (const auto *Source : {"match(x) { (a,b,) => 0 };", "match(x) { [a,] => 0 };", "match(x) { [xs..., a] => 0 };", "match(x) { _ => 0, };", "match(x) { -\"s\" => 0 };", "match(x) { A(x,) => 0 };"})
    {
      EXPECT_FALSE(read(Source).succeeded()) << Source;
    }
    EXPECT_TRUE(read("match(x) {};").succeeded());
  }
} // namespace ink::parser::test
