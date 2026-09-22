#include "parser_test_support.h"
namespace ink::parser::test
{
  namespace
  {
    struct DispatchCounter : ASTVisitor<DispatchCounter>
    {
        std::size_t Count = 0;
        void visitASTNodeBase(ASTNodeBase *)
        {
          ++Count;
        }
    };
    struct ConstDispatchCounter : ConstASTVisitor<ConstDispatchCounter>
    {
        std::size_t Count = 0;
        void visitASTNodeBase(const ASTNodeBase *)
        {
          ++Count;
        }
    };
  } // namespace
  // Every registered concrete kind dispatches through both visitors and reports its own category.
  TEST(ASTVisitorTest, EveryRegisteredKind)
  {
    ASTContext Context;
    const auto Range = SourceRange::fromByteOffsets(0, 0);
    std::vector<ASTNodeBase *> Nodes;
    Nodes.push_back(Context.make<ModuleAST>(Range, ASTArray<Stmt *>{}));
    Nodes.push_back(Context.make<TypeSyntax>(Range, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<MissingExpr>(Range));
    Nodes.push_back(Context.make<ErrorExpr>(Range));
    Nodes.push_back(Context.make<MissingStmt>(Range));
    Nodes.push_back(Context.make<ErrorStmt>(Range));
    Nodes.push_back(Context.make<MissingDecl>(Range, ASTArray<Attribute>{}));
    Nodes.push_back(Context.make<ErrorDecl>(Range, ASTArray<Attribute>{}));
    Nodes.push_back(Context.make<MissingBindingPattern>(Range));
    Nodes.push_back(Context.make<ErrorBindingPattern>(Range));
    Nodes.push_back(Context.make<MissingMatchPattern>(Range));
    Nodes.push_back(Context.make<ErrorMatchPattern>(Range));
    Nodes.push_back(Context.make<NameExpr>(Range, NameToken{}));
    Nodes.push_back(Context.make<LiteralExpr>(Range, TokenId{}, TokenKind{}));
    Nodes.push_back(Context.make<UnaryExpr>(Range, TokenKind{}, Range, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<ComptimeExpr>(Range, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<BinaryExpr>(Range, static_cast<Expr *>(nullptr), TokenKind{}, Range, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<ConditionalExpr>(Range, static_cast<Expr *>(nullptr), static_cast<Expr *>(nullptr), static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<ParenExpr>(Range, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<TupleExpr>(Range, ASTArray<Expr *>{}));
    Nodes.push_back(Context.make<ArrayExpr>(Range, ASTArray<Expr *>{}));
    Nodes.push_back(Context.make<ArrayRepeatExpr>(Range, static_cast<Expr *>(nullptr), static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<CallExpr>(Range, static_cast<Expr *>(nullptr), ASTArray<Argument>{}, bool{}));
    Nodes.push_back(Context.make<IndexExpr>(Range, static_cast<Expr *>(nullptr), static_cast<Expr *>(nullptr), bool{}));
    Nodes.push_back(Context.make<MemberExpr>(Range, static_cast<Expr *>(nullptr), TokenKind{}, NameToken{}));
    Nodes.push_back(Context.make<GenericApplyExpr>(Range, static_cast<Expr *>(nullptr), ASTArray<Argument>{}));
    Nodes.push_back(Context.make<PostfixUpdateExpr>(Range, static_cast<Expr *>(nullptr), TokenKind{}, Range));
    Nodes.push_back(Context.make<FunctionTypeExpr>(Range, ASTArray<FunctionTypeParameter>{}, static_cast<TypeSyntax *>(nullptr)));
    Nodes.push_back(Context.make<LambdaExpr>(Range, ASTArray<Parameter>{}, ASTArray<Parameter>{}, static_cast<TypeSyntax *>(nullptr), static_cast<BlockStmt *>(nullptr)));
    Nodes.push_back(Context.make<MatchExpr>(Range, static_cast<Expr *>(nullptr), ASTArray<MatchArm>{}));
    Nodes.push_back(Context.make<BlockExpr>(Range, static_cast<BlockStmt *>(nullptr)));
    Nodes.push_back(Context.make<ExprItem>(Range, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<AssignmentItem>(Range, static_cast<Expr *>(nullptr), TokenKind{}, Range, static_cast<SimpleItem *>(nullptr)));
    Nodes.push_back(Context.make<SimpleStmt>(Range, ASTArray<SimpleItem *>{}));
    Nodes.push_back(Context.make<BlockStmt>(Range, ASTArray<Stmt *>{}, bool{}));
    Nodes.push_back(Context.make<DeclStmt>(Range, static_cast<Decl *>(nullptr)));
    Nodes.push_back(Context.make<IfStmt>(Range, static_cast<Expr *>(nullptr), static_cast<Stmt *>(nullptr), static_cast<Stmt *>(nullptr)));
    Nodes.push_back(Context.make<WhileStmt>(Range, static_cast<Expr *>(nullptr), static_cast<Stmt *>(nullptr)));
    Nodes.push_back(Context.make<ClassicForStmt>(Range, static_cast<Stmt *>(nullptr), static_cast<Expr *>(nullptr), ASTArray<SimpleItem *>{}, static_cast<Stmt *>(nullptr)));
    Nodes.push_back(Context.make<ForInStmt>(Range, static_cast<BindingPattern *>(nullptr), static_cast<Expr *>(nullptr), static_cast<Stmt *>(nullptr)));
    Nodes.push_back(Context.make<SwitchStmt>(Range, static_cast<Expr *>(nullptr), ASTArray<SwitchClause>{}));
    Nodes.push_back(Context.make<ReturnStmt>(Range, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<BreakStmt>(Range));
    Nodes.push_back(Context.make<ContinueStmt>(Range));
    Nodes.push_back(Context.make<YieldStmt>(Range, bool{}, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<DeferStmt>(Range, static_cast<Stmt *>(nullptr)));
    Nodes.push_back(Context.make<ComptimeStmt>(Range, Range, static_cast<Stmt *>(nullptr)));
    Nodes.push_back(Context.make<DirectImportStmt>(Range, ASTArray<ImportEntry>{}));
    Nodes.push_back(Context.make<FromImportStmt>(Range, std::size_t{}, ASTArray<TokenId>{}, ASTArray<NameToken>{}, ASTArray<ImportEntry>{}));
    Nodes.push_back(Context.make<VarDecl>(Range, ASTArray<Attribute>{}, bool{}, static_cast<BindingPattern *>(nullptr), static_cast<TypeSyntax *>(nullptr), static_cast<Expr *>(nullptr), VarDeclForm{}));
    Nodes.push_back(Context.make<FieldDecl>(Range, ASTArray<Attribute>{}, NameToken{}, FieldTailKind{}, static_cast<TypeSyntax *>(nullptr), ASTArray<Parameter>{}, static_cast<Expr *>(nullptr)));
    Nodes.push_back(Context.make<FunctionDecl>(Range, ASTArray<Attribute>{}, NameToken{}, ASTArray<Parameter>{}, ASTArray<Parameter>{}, static_cast<TypeSyntax *>(nullptr), FunctionBodyKind{}, static_cast<BlockStmt *>(nullptr)));
    Nodes.push_back(Context.make<ClassDecl>(Range, ASTArray<Attribute>{}, NameToken{}, ASTArray<Parameter>{}, ASTArray<BaseSpec>{}, AggregateForm{}, static_cast<BlockStmt *>(nullptr), Range));
    Nodes.push_back(Context.make<EnumDecl>(Range, ASTArray<Attribute>{}, NameToken{}, ASTArray<Parameter>{}, ASTArray<BaseSpec>{}, AggregateForm{}, static_cast<BlockStmt *>(nullptr), Range));
    Nodes.push_back(Context.make<InterfaceDecl>(Range, ASTArray<Attribute>{}, NameToken{}, ASTArray<Parameter>{}, ASTArray<BaseSpec>{}, AggregateForm{}, static_cast<BlockStmt *>(nullptr), Range));
    Nodes.push_back(Context.make<WildcardBindingPattern>(Range));
    Nodes.push_back(Context.make<NameBindingPattern>(Range, NameToken{}));
    Nodes.push_back(Context.make<TupleBindingPattern>(Range, ASTArray<BindingPattern *>{}));
    Nodes.push_back(Context.make<ArrayBindingPattern>(Range, ASTArray<BindingPattern *>{}, std::optional<RestBinding>{}));
    Nodes.push_back(Context.make<WildcardMatchPattern>(Range));
    Nodes.push_back(Context.make<NameMatchPattern>(Range, ASTArray<PathSegment>{}, bool{}, ASTArray<MatchPattern *>{}));
    Nodes.push_back(Context.make<TupleMatchPattern>(Range, ASTArray<MatchPattern *>{}));
    Nodes.push_back(Context.make<ArrayMatchPattern>(Range, ASTArray<MatchPattern *>{}, std::optional<RestBinding>{}));
    Nodes.push_back(Context.make<LiteralMatchPattern>(Range, static_cast<LiteralExpr *>(nullptr), bool{}));
    Nodes.push_back(Context.make<GroupedMatchPattern>(Range, static_cast<MatchPattern *>(nullptr)));
    Nodes.push_back(Context.make<OrMatchPattern>(Range, ASTArray<MatchPattern *>{}));
    constexpr std::size_t RegisteredCount = 0
#define AST_NODE(Name, Base, Category, Id) +1
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
        ;
    ASSERT_EQ(Nodes.size(), RegisteredCount);
    DispatchCounter Mutable;
    ConstDispatchCounter Constant;
    for (auto *Node : Nodes)
    {
      Mutable.visit(Node);
      Constant.visit(Node);
      EXPECT_NE(std::string_view(astKindName(Node->getKind())), "InvalidASTKind");
      EXPECT_EQ(isa<Expr>(Node), categoryOf(Node->getKind()) == ASTCategory::Expr);
      EXPECT_EQ(isa<Stmt>(Node), categoryOf(Node->getKind()) == ASTCategory::Stmt);
      EXPECT_EQ(isa<Decl>(Node), categoryOf(Node->getKind()) == ASTCategory::Decl);
      EXPECT_EQ(isa<BindingPattern>(Node), categoryOf(Node->getKind()) == ASTCategory::BindingPattern);
      EXPECT_EQ(isa<MatchPattern>(Node), categoryOf(Node->getKind()) == ASTCategory::MatchPattern);
    }
    EXPECT_EQ(Mutable.Count, RegisteredCount);
    EXPECT_EQ(Constant.Count, RegisteredCount);
  }
} // namespace ink::parser::test
