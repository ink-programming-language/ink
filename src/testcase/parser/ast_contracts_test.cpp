#include "parser_test_support.h"
#include "ink/parser/parser_diagnostic_emitter.h"
#include <type_traits>
#include <limits>
namespace ink::parser::test
{
  namespace
  {
    struct Resource
    {
        Resource(std::vector<int> &Log, int Id)
            : Log(Log),
              Id(Id)
        {
        }
        Resource(const Resource &Other)
            : Log(Other.Log),
              Id(Other.Id)
        {
        }
        ~Resource()
        {
          Log.push_back(Id);
        }
        std::vector<int> &Log;
        int Id;
    };
    struct alignas(256) AlignedResource
    {
        std::size_t Value = 42;
    };
    struct CountingVisitor : ConstASTVisitor<CountingVisitor>
    {
        std::size_t Nodes = 0;
        std::size_t Names = 0;
        void visitASTNodeBase(const ASTNodeBase *)
        {
          ++Nodes;
        }
        void visitNameExpr(const NameExpr *)
        {
          ++Names;
        }
    };
    struct MutableVisitor : ASTVisitor<MutableVisitor>
    {
        std::size_t Nodes = 0;
        void visitASTNodeBase(ASTNodeBase *)
        {
          ++Nodes;
        }
    };
    struct CompleteExprVisitor : StrictExprVisitor<CompleteExprVisitor, ASTKind>
    {
#define INK_TEST_Expr(Name)         \
  ASTKind visit##Name(const Name *) \
  {                                 \
    return ASTKind::Name;           \
  }
#define INK_TEST_Root(Name)
#define INK_TEST_Stmt(Name)
#define INK_TEST_Decl(Name)
#define INK_TEST_SimpleItem(Name)
#define INK_TEST_BindingPattern(Name)
#define INK_TEST_MatchPattern(Name)
#define AST_NODE(Name, Base, Category) INK_TEST_##Category(Name)
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
#undef INK_TEST_Expr
#undef INK_TEST_Root
#undef INK_TEST_Stmt
#undef INK_TEST_Decl
#undef INK_TEST_SimpleItem
#undef INK_TEST_BindingPattern
#undef INK_TEST_MatchPattern
    };
    static_assert(std::is_same_v<decltype(std::declval<const BinaryExpr &>().left()), const Expr *>);
    static_assert(std::is_same_v<decltype(std::declval<const ModuleAST &>().statements()[0]), const Stmt *>);
    static_assert(std::is_same_v<decltype(std::declval<const Parameter &>().type()), const TypeSyntax *>);
    static_assert(std::is_same_v<decltype(dyn_cast<Expr>(static_cast<const ASTNodeBase *>(nullptr))), const Expr *>);
    static_assert(!std::has_virtual_destructor_v<ASTNodeBase>);
    static_assert(!std::is_destructible_v<ASTNodeBase>);
  } // namespace

  // Rollback invokes exact object and nontrivial array destructors in reverse allocation order.
  TEST(ASTContextTest, DestructionAndNestedCheckpoints)
  {
    std::vector<int> Log;
    {
      ASTContext Context;
      Context.make<Resource>(Log, 1);
      const auto First = Context.checkpoint();
      Context.make<Resource>(Log, 2);
      const auto Second = Context.checkpoint();
      {
        const std::vector<Resource> Values{Resource(Log, 3), Resource(Log, 4)};
        Context.copyArray(Values);
        Log.clear();
        Context.rollback(Second);
        EXPECT_EQ(Log, (std::vector<int>{4, 3}));
      }
      Log.clear();
      Context.rollback(First);
      EXPECT_EQ(Log, (std::vector<int>{2}));
      EXPECT_EQ(Context.allocatedBytes(), First.Bytes);
      Context.make<Resource>(Log, 5);
    }
    EXPECT_EQ(Log, (std::vector<int>{2, 5, 1}));
  }

  // Arena growth and checkpoint reuse preserve existing addresses and honor over-aligned allocations.
  TEST(ASTContextTest, StableAddressesAlignmentAndArrayCopies)
  {
    ASTContext Context;
    auto *First = Context.make<AlignedResource>();
    EXPECT_EQ(reinterpret_cast<std::uintptr_t>(First) % 256, 0U);
    const auto Saved = Context.checkpoint();
    for (int Index = 0; Index < 2000; ++Index)
    {
      Context.make<AlignedResource>();
    }
    EXPECT_EQ(First->Value, 42U);
    Context.rollback(Saved);
    EXPECT_EQ(First->Value, 42U);
    std::vector<std::string> Source{"owned", "array"};
    const auto Copy = Context.copyArray(Source);
    Source.clear();
    EXPECT_EQ(Copy[0], "owned");
    EXPECT_EQ(Copy[1], "array");
    EXPECT_TRUE(Context.copyArray<int>(std::span<const int>{}).empty());
  }

  // A visitor dispatches only its current node and honors leaf overrides and classification fallbacks.
  TEST_F(ParserTest, VisitorDispatchAndConstCasts)
  {
    const auto Result = read("a + b;");
    ASSERT_TRUE(Result.succeeded());
    CountingVisitor Visitor;
    Visitor.visit(Result.Unit->root());
    EXPECT_EQ(Visitor.Nodes, 1U);
    EXPECT_EQ(Visitor.Names, 0U);
    Visitor.visit(cast<BinaryExpr>(expression(Result))->left());
    EXPECT_EQ(Visitor.Names, 1U);
    EXPECT_FALSE(isa<Expr>(nullptr));
    EXPECT_EQ(dyn_cast<Expr>(static_cast<ASTNodeBase *>(nullptr)), nullptr);
    EXPECT_EQ(dyn_cast<Stmt>(expression(Result)), nullptr);
    EXPECT_EQ(CompleteExprVisitor{}.visit(expression(Result)), ASTKind::BinaryExpr);
    MutableVisitor Mutable;
    Mutable.visit(Result.Unit->root());
    EXPECT_EQ(Mutable.Nodes, 1U);
  }

  // Walker enter/leave events follow source order, skipping descendants and stopping immediately when requested.
  TEST_F(ParserTest, WalkerControlAndSourceOrder)
  {
    const auto Result = read("f(a, b);");
    ASSERT_TRUE(Result.succeeded());
    std::vector<std::string> Names;
    std::vector<const ASTNodeBase *> Stack;
    EXPECT_TRUE(ASTWalker{}.walk(Result.Unit->root(), [&](const ASTNodeBase *Node)
                                 {
                                   Stack.push_back(Node);
                                   if (auto *Name = dyn_cast<NameExpr>(Node))
                                   {
                                     Names.emplace_back(Name->name().Text);
                                   }
                                   return WalkAction::Continue;
                                 },
                                 [&](const ASTNodeBase *Node)
                                 {
                                   ASSERT_EQ(Stack.back(), Node);
                                   Stack.pop_back();
                                 }));
    EXPECT_TRUE(Stack.empty());
    EXPECT_EQ(Names, (std::vector<std::string>{"f", "a", "b"}));
    std::size_t Count = 0;
    ASTWalker{}.walk(expression(Result), [&](const ASTNodeBase *)
                     {
                       ++Count;
                       return WalkAction::SkipChildren;
                     });
    EXPECT_EQ(Count, 1U);
    EXPECT_FALSE(ASTWalker{}.walk(Result.Unit->root(), [&](const ASTNodeBase *)
                                  {
                                    return WalkAction::Stop;
                                  }));
  }

  // Record-owned types, defaults, attributes, guards and for-header expressions all participate in traversal.
  TEST_F(ParserTest, RecordChildrenCoverage)
  {
    const auto Result = read("[tag(attr)] func f[G: generic = fallback](p: parameter = initial): result { for (var x = begin; condition; step) { match(subject) { P::[argument](v) if (guard) => value }; } }");
    ASSERT_TRUE(Result.succeeded());
    std::vector<std::string> Names;
    ASTWalker{}.walk(Result.Unit->root(), [&](const ASTNodeBase *Node)
                     {
                       if (auto *Name = dyn_cast<NameExpr>(Node))
                       {
                         Names.emplace_back(Name->name().Text);
                       }
                       return WalkAction::Continue;
                     });
    EXPECT_EQ(Names, (std::vector<std::string>{"attr", "generic", "fallback", "parameter", "initial", "result", "begin", "condition", "step", "subject", "argument", "guard", "value"}));
  }

  // Parsed units retain token spellings after the originating compilation context is destroyed.
  TEST(ASTLifetimeTest, SourceSnapshotOwnership)
  {
    ParseResult Result;
    {
      core::CompilationContext Compilation;
      core::FrontendContext Frontend(Compilation);
      Result = parse(Frontend, tokenizer::tokenize(Frontend, "retained;"));
    }
    ASSERT_TRUE(Result.succeeded());
    const auto *Name = cast<NameExpr>(cast<ExprItem>(cast<SimpleStmt>(Result.Unit->root()->statements()[0])->items()[0])->expression());
    EXPECT_EQ(Name->name().Text, "retained");
    EXPECT_EQ(Result.Unit->input().spelling(Name->name().Id), "retained");
  }

  // Cursor lookahead saturates at EOF, including huge offsets and failed lexical prefixes.
  TEST_F(ParserTest, CursorEOFAndRestore)
  {
    TokenBuffer Buffer(tokenizer::tokenize(Frontend, "x;"));
    TokenCursor Cursor(Buffer);
    EXPECT_EQ(Cursor.peek().Kind, TokenKind::Identifier);
    Cursor.bump();
    const auto Saved = Cursor.position();
    Cursor.bump();
    EXPECT_EQ(Cursor.peek().Kind, TokenKind::EndOfFile);
    const auto End = Cursor.position();
    Cursor.bump();
    EXPECT_EQ(Cursor.position(), End);
    EXPECT_EQ(Cursor.peek(std::numeric_limits<std::size_t>::max()).Kind, TokenKind::EndOfFile);
    Cursor.restore(Saved);
    EXPECT_EQ(Cursor.peek().Kind, TokenKind::Semicolon);
    TokenBuffer Failed(tokenizer::tokenize(Frontend, "x @"));
    TokenCursor Partial(Failed);
    Partial.bump();
    EXPECT_EQ(Partial.peek().Kind, TokenKind::EndOfFile);
    EXPECT_TRUE(Partial.peek().Span.empty());
  }

  // Nested diagnostic commits remain buffered until the outer commit and rollback restores counters and owned arguments.
  TEST_F(ParserTest, DiagnosticTransactions)
  {
    ParserDiagnosticEmitter Emitter(Frontend.diagnosticEngine(), {}, 10);
    const auto Range = SourceRange::fromByteOffsets(0, 0);
    Emitter.begin();
    Emitter.report(core::makeDiagnostic<core::DiagnosticKind::ParserExpectedToken>(Range, std::string("owned")));
    Emitter.begin();
    Emitter.report(core::makeDiagnostic<core::DiagnosticKind::ParserInvalidSyntax>(Range));
    Emitter.commit();
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
    Emitter.rollback();
    EXPECT_EQ(Emitter.errorCount(), 0U);
    Emitter.begin();
    Emitter.begin();
    Emitter.report(core::makeDiagnostic<core::DiagnosticKind::ParserExpectedToken>(Range, std::string("retained")));
    Emitter.commit();
    Emitter.commit();
    ASSERT_EQ(Diagnostics.diagnostics().size(), 1U);
    EXPECT_EQ(std::get<std::string>(Diagnostics.diagnostics()[0].Arguments[0].Value), "retained");
  }

  // Structural verification rejects missing required children and children outside their parent's range.
  TEST(ASTVerifierTest, InvalidContracts)
  {
    ASTContext Context;
    const auto Range = SourceRange::fromByteOffsets(0, 1);
    auto *Missing = Context.make<MissingExpr>(Range);
    auto *Invalid = Context.make<BinaryExpr>(Range, Missing, TokenKind::Plus, Range, nullptr);
    EXPECT_FALSE(verifyAST(Invalid, 1));
    auto *Outside = Context.make<MissingExpr>(SourceRange::fromByteOffsets(2, 2));
    auto *Parent = Context.make<ParenExpr>(Range, Outside);
    EXPECT_FALSE(verifyAST(Parent, 2));
    EXPECT_FALSE(verifyAST(nullptr, 0));
  }
} // namespace ink::parser::test
