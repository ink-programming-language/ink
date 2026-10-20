#include "parser_test_support.h"
#include <initializer_list>
#include <iterator>
#include <string_view>

namespace ink::frontend::parser::test
{
  namespace
  {
    struct PrefixExpectation
    {
        TokenKind Operator;
        bool ConstQualified;
    };

    void expectPrefixSequence(const Expr *Value, std::initializer_list<PrefixExpectation> Prefixes, std::string_view Name = "u8")
    {
      for (const auto Prefix : Prefixes)
      {
        const auto *Unary = dyn_cast<UnaryExpr>(Value);
        ASSERT_NE(Unary, nullptr);
        EXPECT_EQ(Unary->op(), Prefix.Operator);
        EXPECT_EQ(Unary->isConstQualified(), Prefix.ConstQualified);
        const auto Qualifier = Unary->qualifier();
        EXPECT_EQ(Qualifier.has_value(), Prefix.ConstQualified);
        if (Prefix.ConstQualified)
        {
          ASSERT_TRUE(Qualifier.has_value());
          EXPECT_EQ(Qualifier->Kind, TypeQualifierKind::Const);
          EXPECT_TRUE(Qualifier->Range.isValid());
          EXPECT_FALSE(Qualifier->Range.empty());
        }
        Value = Unary->operand();
      }
      const auto *Leaf = dyn_cast<NameExpr>(Value);
      ASSERT_NE(Leaf, nullptr);
      EXPECT_EQ(Leaf->name().Text, Name);
    }

    void expectKeptConstPointerSuffix(const ParseResult &Result)
    {
      ASSERT_EQ(Result.Status, ParseStatus::Completed);
      ASSERT_GE(Result.Unit->root()->statements().size(), 2U);
      const auto *Statement = dyn_cast<DeclStmt>(Result.Unit->root()->statements().back());
      ASSERT_NE(Statement, nullptr);
      const auto *Variable = dyn_cast<VarDecl>(Statement->declaration());
      ASSERT_NE(Variable, nullptr);
      const auto *Binding = dyn_cast<NameBindingPattern>(Variable->binding());
      ASSERT_NE(Binding, nullptr);
      EXPECT_EQ(Binding->name().Text, "kept");
      ASSERT_NE(Variable->initializer(), nullptr);
      EXPECT_TRUE(isa<LiteralExpr>(Variable->initializer()));
    }
  } // namespace

  // Binding constness and pointee constness remain independent in all four declaration forms.
  TEST_F(ParserTest, ConstPointerDeclarationCombinations)
  {
    const auto Result = read("var Mutable: *u8 = Source; const Fixed: *u8 = Source; var ReadOnly: *const u8 = Source; const FixedReadOnly: *const u8 = Source;");
    ASSERT_TRUE(Result.succeeded());
    ASSERT_EQ(Result.Unit->root()->statements().size(), 4U);
    for (std::size_t Index = 0; Index < 4; ++Index)
    {
      const auto *Variable = cast<VarDecl>(declaration(Result, Index));
      EXPECT_EQ(Variable->constant(), Index % 2 != 0);
      ASSERT_NE(Variable->type(), nullptr);
      if (Index < 2)
      {
        expectPrefixSequence(Variable->type()->expression(), {{TokenKind::Star, false}});
      }
      else
      {
        expectPrefixSequence(Variable->type()->expression(), {{TokenKind::Star, true}});
      }
      ASSERT_TRUE(isa<NameExpr>(Variable->initializer()));
      EXPECT_EQ(cast<NameExpr>(Variable->initializer())->name().Text, "Source");
    }
    EXPECT_TRUE(Diagnostics.diagnostics().empty());
  }

  // The native WriteFile declaration qualifies only its input buffer and retains the writable output parameter.
  TEST_F(ParserTest, ConstPointerNativeWriteFile)
  {
    const auto Result = read("import \"C\" func WriteFile(File: *void, Buffer: *const u8, BytesToWrite: u32, BytesWritten: *u32, Overlapped: *void): i32;");
    ASSERT_TRUE(Result.succeeded());
    const auto *Function = cast<FunctionDecl>(declaration(Result));
    EXPECT_EQ(Function->name().Text, "WriteFile");
    EXPECT_EQ(Function->nativeSymbolKind(), NativeSymbolKind::Import);
    EXPECT_EQ(Function->bodyKind(), FunctionBodyKind::DeclarationOnly);
    ASSERT_EQ(Function->parameters().size(), 5U);
    EXPECT_EQ(Function->parameters()[1].name().Text, "Buffer");
    expectPrefixSequence(Function->parameters()[0].type()->expression(), {{TokenKind::Star, false}}, "void");
    expectPrefixSequence(Function->parameters()[1].type()->expression(), {{TokenKind::Star, true}});
    expectPrefixSequence(Function->parameters()[2].type()->expression(), {}, "u32");
    EXPECT_EQ(Function->parameters()[3].name().Text, "BytesWritten");
    expectPrefixSequence(Function->parameters()[3].type()->expression(), {{TokenKind::Star, false}}, "u32");
    expectPrefixSequence(Function->parameters()[4].type()->expression(), {{TokenKind::Star, false}}, "void");
    expectPrefixSequence(Function->returnType()->expression(), {}, "i32");
  }

  // Each const qualifier attaches to its own pointer layer without changing adjacent pointer layers.
  TEST_F(ParserTest, ConstPointerNestedLayers)
  {
    const auto Result = read("var Plain: **u8; var FixedSlot: *const *u8; var ReadOnlyValue: **const u8; var Both: *const *const u8;");
    ASSERT_TRUE(Result.succeeded());
    expectPrefixSequence(cast<VarDecl>(declaration(Result, 0))->type()->expression(), {{TokenKind::Star, false}, {TokenKind::Star, false}});
    expectPrefixSequence(cast<VarDecl>(declaration(Result, 1))->type()->expression(), {{TokenKind::Star, true}, {TokenKind::Star, false}});
    expectPrefixSequence(cast<VarDecl>(declaration(Result, 2))->type()->expression(), {{TokenKind::Star, false}, {TokenKind::Star, true}});
    expectPrefixSequence(cast<VarDecl>(declaration(Result, 3))->type()->expression(), {{TokenKind::Star, true}, {TokenKind::Star, true}});
  }

  // References accept the same qualifier and compose with independently qualified pointer layers.
  TEST_F(ParserTest, ConstReferenceAndPointerComposition)
  {
    const auto Result = read("var Ref: &const u8; var PointerToRef: *const &u8; var RefToPointer: &const *const u8; &const u8;");
    ASSERT_TRUE(Result.succeeded());
    expectPrefixSequence(cast<VarDecl>(declaration(Result, 0))->type()->expression(), {{TokenKind::Amp, true}});
    expectPrefixSequence(cast<VarDecl>(declaration(Result, 1))->type()->expression(), {{TokenKind::Star, true}, {TokenKind::Amp, false}});
    expectPrefixSequence(cast<VarDecl>(declaration(Result, 2))->type()->expression(), {{TokenKind::Amp, true}, {TokenKind::Star, true}});
    expectPrefixSequence(expression(Result, 3), {{TokenKind::Amp, true}});
  }

  // Qualified pointer expressions can define type values and appear in generic arguments without a type annotation.
  TEST_F(ParserTest, ConstPointerTypeValuesAndGenericArguments)
  {
    const auto Result = read("const PType = *const u8; Factory::[*const u8, &const u8]; *const u8;");
    ASSERT_TRUE(Result.succeeded());
    const auto *TypeValue = cast<VarDecl>(declaration(Result));
    EXPECT_TRUE(TypeValue->constant());
    EXPECT_EQ(TypeValue->type(), nullptr);
    expectPrefixSequence(TypeValue->initializer(), {{TokenKind::Star, true}});
    const auto *Generic = cast<GenericApplyExpr>(expression(Result, 1));
    ASSERT_EQ(Generic->arguments().size(), 2U);
    expectPrefixSequence(Generic->arguments()[0].value(), {{TokenKind::Star, true}});
    expectPrefixSequence(Generic->arguments()[1].value(), {{TokenKind::Amp, true}});
    expectPrefixSequence(expression(Result, 2), {{TokenKind::Star, true}});
  }

  // Named function declarations and function type values retain qualifiers on parameters and return types.
  TEST_F(ParserTest, ConstPointerFunctionSignatures)
  {
    const auto Result = read("func inspect(Data: &const u8): *const u8; var Callback: func(*const u8, Context: &const u8): *const void;");
    ASSERT_TRUE(Result.succeeded());
    const auto *Function = cast<FunctionDecl>(declaration(Result));
    ASSERT_EQ(Function->parameters().size(), 1U);
    expectPrefixSequence(Function->parameters()[0].type()->expression(), {{TokenKind::Amp, true}});
    expectPrefixSequence(Function->returnType()->expression(), {{TokenKind::Star, true}});
    const auto *Callback = cast<FunctionTypeExpr>(cast<VarDecl>(declaration(Result, 1))->type()->expression());
    ASSERT_EQ(Callback->parameters().size(), 2U);
    EXPECT_FALSE(Callback->parameters()[0].name().has_value());
    ASSERT_TRUE(Callback->parameters()[1].name().has_value());
    EXPECT_EQ(Callback->parameters()[1].name()->Text, "Context");
    expectPrefixSequence(Callback->parameters()[0].type()->expression(), {{TokenKind::Star, true}});
    expectPrefixSequence(Callback->parameters()[1].type()->expression(), {{TokenKind::Amp, true}});
    expectPrefixSequence(Callback->returnType()->expression(), {{TokenKind::Star, true}}, "void");
  }

  // Array element qualifiers stay distinct from array qualifiers, while postfix expressions respect explicit pointer grouping.
  TEST_F(ParserTest, ConstPointerArrayAndGroupingPrecedence)
  {
    const auto Result = read("var PointerToArray: *const [u8; 4]; var ArrayOfPointers: [*const u8; 4]; var PointerToGroup: *const (u8[4]); var QualifiedGeneric: *const Buffer::[u8]; var IndexedPointer: (*const u8)[4];");
    ASSERT_TRUE(Result.succeeded());
    const auto *Pointer = cast<UnaryExpr>(cast<VarDecl>(declaration(Result, 0))->type()->expression());
    EXPECT_TRUE(Pointer->isConstQualified());
    const auto *Array = cast<ArrayRepeatExpr>(Pointer->operand());
    expectPrefixSequence(Array->value(), {});
    EXPECT_TRUE(isa<LiteralExpr>(Array->count()));
    const auto *OuterArray = cast<ArrayRepeatExpr>(cast<VarDecl>(declaration(Result, 1))->type()->expression());
    expectPrefixSequence(OuterArray->value(), {{TokenKind::Star, true}});
    const auto *GroupedPointer = cast<UnaryExpr>(cast<VarDecl>(declaration(Result, 2))->type()->expression());
    EXPECT_TRUE(GroupedPointer->isConstQualified());
    EXPECT_TRUE(isa<IndexExpr>(cast<ParenExpr>(GroupedPointer->operand())->expression()));
    const auto *GenericPointer = cast<UnaryExpr>(cast<VarDecl>(declaration(Result, 3))->type()->expression());
    EXPECT_TRUE(GenericPointer->isConstQualified());
    const auto *Generic = cast<GenericApplyExpr>(GenericPointer->operand());
    EXPECT_EQ(cast<NameExpr>(Generic->object())->name().Text, "Buffer");
    ASSERT_EQ(Generic->arguments().size(), 1U);
    expectPrefixSequence(Generic->arguments()[0].value(), {});
    const auto *IndexedPointer = cast<IndexExpr>(cast<VarDecl>(declaration(Result, 4))->type()->expression());
    const auto *Group = cast<ParenExpr>(IndexedPointer->object());
    expectPrefixSequence(Group->expression(), {{TokenKind::Star, true}});
  }

  // Pointer nodes retain separate operator and qualifier token ranges without adding qualifier children to AST traversal.
  TEST_F(ParserTest, ConstPointerSourceRangesTraversalAndDump)
  {
    const std::string Source = "* const /*inner*/ *const u8;";
    const auto Result = read(Source);
    ASSERT_TRUE(Result.succeeded());
    const std::size_t OuterConst = Source.find("const");
    const std::size_t InnerStar = Source.find('*', Source.find("*/") + 2);
    const std::size_t InnerConst = Source.find("const", OuterConst + 1);
    const std::size_t OperatorStarts[] = {0, InnerStar};
    const std::size_t QualifierStarts[] = {OuterConst, InnerConst};
    const Expr *Value = expression(Result);
    for (std::size_t Index = 0; Index < 2; ++Index)
    {
      const auto *Unary = dyn_cast<UnaryExpr>(Value);
      ASSERT_NE(Unary, nullptr);
      EXPECT_EQ(Unary->op(), TokenKind::Star);
      EXPECT_EQ(Unary->operatorRange(), SourceRange::fromByteOffsets(OperatorStarts[Index], OperatorStarts[Index] + 1));
      EXPECT_TRUE(Unary->isConstQualified());
      ASSERT_TRUE(Unary->qualifier().has_value());
      EXPECT_EQ(Unary->qualifier()->Kind, TypeQualifierKind::Const);
      EXPECT_EQ(Unary->qualifier()->Range, SourceRange::fromByteOffsets(QualifierStarts[Index], QualifierStarts[Index] + 5));
      EXPECT_EQ(Unary->getSourceRange(), SourceRange::fromByteOffsets(OperatorStarts[Index], Source.size() - 1));
      Value = Unary->operand();
    }
    expectPrefixSequence(Value, {});
    std::vector<ASTKind> Kinds;
    std::vector<TokenKind> WalkedOperators;
    const auto Visit = [&](const ASTNodeBase *Node)
    {
      Kinds.push_back(Node->getKind());
      if (const auto *Unary = dyn_cast<UnaryExpr>(Node))
      {
        WalkedOperators.push_back(Unary->op());
      }
      return WalkAction::Continue;
    };
    EXPECT_TRUE(ASTWalker{}.walk(expression(Result), Visit));
    EXPECT_EQ(Kinds, (std::vector<ASTKind>{ASTKind::UnaryExpr, ASTKind::UnaryExpr, ASTKind::NameExpr}));
    EXPECT_EQ(WalkedOperators, (std::vector<TokenKind>{TokenKind::Star, TokenKind::Star}));
    const std::string Dump = dumpAST(*Result.Unit);
    const std::string QualifierText = "Qualifier={ Kind=Const Range=";
    const auto FirstQualifier = Dump.find(QualifierText);
    ASSERT_NE(FirstQualifier, std::string::npos);
    const auto SecondQualifier = Dump.find(QualifierText, FirstQualifier + 1);
    ASSERT_NE(SecondQualifier, std::string::npos);
    EXPECT_EQ(Dump.find(QualifierText, SecondQualifier + 1), std::string::npos);
    EXPECT_NE(Dump.find("OperatorRange=[0, 1)"), std::string::npos);
    EXPECT_NE(Dump.find("Qualifier={ Kind=Const Range=[2, 7) }"), std::string::npos);
    EXPECT_EQ(Dump.find(std::string("Op=") + tokenizer::tokenKindName(TokenKind::KwConst)), std::string::npos);
    const auto Unqualified = read("*u8;");
    ASSERT_TRUE(Unqualified.succeeded());
    EXPECT_NE(dumpAST(*Unqualified.Unit).find("Qualifier=none"), std::string::npos);
  }

  // A missing qualified operand reports an expression error while preserving the following declaration or parameter.
  TEST_F(ParserTest, ConstPointerMissingOperandRecovery)
  {
    for (const char *Source : {"var Broken: *const; var kept = 42;", "const Broken = &const; var kept = 42;", "func Broken(Bad: *const, Kept: u8): void; var kept = 42;", "func Broken(Bad: &const, Kept: u8): void; var kept = 42;"})
    {
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      ASSERT_FALSE(Result.succeeded());
      ASSERT_FALSE(Diagnostics.diagnostics().empty());
      EXPECT_EQ(Diagnostics.diagnostics().front().Kind, core::DiagnosticKind::ParserExpectedExpression);
      expectKeptConstPointerSuffix(Result);
      const Expr *Value;
      if (const auto *Function = dyn_cast<FunctionDecl>(declaration(Result)))
      {
        ASSERT_EQ(Function->parameters().size(), 2U);
        EXPECT_EQ(Function->parameters()[1].name().Text, "Kept");
        expectPrefixSequence(Function->parameters()[1].type()->expression(), {});
        Value = Function->parameters()[0].type()->expression();
      }
      else
      {
        const auto *Variable = cast<VarDecl>(declaration(Result));
        Value = Variable->type() ? Variable->type()->expression() : Variable->initializer();
      }
      const auto *Prefix = dyn_cast<UnaryExpr>(Value);
      ASSERT_NE(Prefix, nullptr);
      EXPECT_TRUE(Prefix->isConstQualified());
      ASSERT_TRUE(Prefix->qualifier().has_value());
      EXPECT_EQ(Prefix->qualifier()->Kind, TypeQualifierKind::Const);
      ASSERT_TRUE(isa<MissingExpr>(Prefix->operand()));
      EXPECT_TRUE(Prefix->operand()->getSourceRange().empty());
    }
  }

  // Repeating const at one pointer or reference layer is rejected without losing a later valid declaration.
  TEST_F(ParserTest, ConstPointerDuplicateQualifierRecovery)
  {
    for (const char *Source : {"var Broken: *const const u8; var kept = 42;", "var Broken: &const const u8; var kept = 42;", "const Broken = *const const u8; var kept = 42;", "const Broken = &const const u8; var kept = 42;"})
    {
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Diagnostics.diagnostics().empty());
      expectKeptConstPointerSuffix(Result);
    }
  }

  // Const is not a standalone type prefix and cannot qualify unary operators other than pointer or reference construction.
  TEST_F(ParserTest, ConstPointerRejectsMisplacedQualifiers)
  {
    const char *Sources[] = {
        "var Broken: const u8; var kept = 42;",
        "var Broken: const *u8; var kept = 42;",
        "const Broken = const u8; var kept = 42;",
        "const Broken = const *u8; var kept = 42;",
        "var Broken: +const u8; var kept = 42;",
        "var Broken: -const u8; var kept = 42;",
        "var Broken: !const u8; var kept = 42;",
        "var Broken: ~const u8; var kept = 42;",
        "var Broken: ++const u8; var kept = 42;",
        "var Broken: --const u8; var kept = 42;",
    };
    for (const char *Source : Sources)
    {
      SCOPED_TRACE(Source);
      const auto Result = read(Source);
      EXPECT_FALSE(Result.succeeded());
      EXPECT_FALSE(Diagnostics.diagnostics().empty());
      expectKeptConstPointerSuffix(Result);
    }
  }

  // Structural verification rejects the old standalone const unary node even when its operand and ranges are valid.
  TEST(ASTVerifierTest, ConstQualifierRejectsStandaloneUnaryNode)
  {
    ASTContext Context;
    auto *Operand = Context.make<MissingExpr>(SourceRange::fromByteOffsets(6, 8));
    auto *StandaloneConst = Context.make<UnaryExpr>(SourceRange::fromByteOffsets(0, 8), TokenKind::KwConst, SourceRange::fromByteOffsets(0, 5), Operand);
    EXPECT_FALSE(verifyAST(StandaloneConst, 8));
  }

  // Only pointer and reference unary nodes may carry a const qualifier, including when all ranges are well formed.
  TEST(ASTVerifierTest, ConstQualifierRejectsOtherUnaryOperators)
  {
    ASTContext Context;
    auto *Operand = Context.make<MissingExpr>(SourceRange::fromByteOffsets(8, 10));
    for (const auto Operator : {TokenKind::Plus, TokenKind::Minus, TokenKind::Bang, TokenKind::Tilde, TokenKind::PlusPlus, TokenKind::MinusMinus})
    {
      SCOPED_TRACE(tokenizer::tokenKindName(Operator));
      const auto OperatorRange = SourceRange::fromByteOffsets(0, Operator == TokenKind::PlusPlus || Operator == TokenKind::MinusMinus ? 2 : 1);
      auto *Qualified = Context.make<UnaryExpr>(SourceRange::fromByteOffsets(0, 10), Operator, OperatorRange, Operand, TypeQualifier{TypeQualifierKind::Const, SourceRange::fromByteOffsets(2, 7)});
      EXPECT_FALSE(verifyAST(Qualified, 10));
    }
  }

  // Qualifier ranges must be nonempty, valid, within their node, and between the operator token and operand.
  TEST(ASTVerifierTest, ConstQualifierRejectsInvalidSourceRanges)
  {
    ASTContext Context;
    const auto NodeRange = SourceRange::fromByteOffsets(4, 14);
    const auto OperatorRange = SourceRange::fromByteOffsets(4, 5);
    auto *Operand = Context.make<MissingExpr>(SourceRange::fromByteOffsets(12, 14));
    const SourceRange InvalidRanges[] = {
        SourceRange{},
        SourceRange::fromByteOffsets(6, 6),
        SourceRange::fromByteOffsets(11, 6),
        SourceRange::fromByteOffsets(2, 4),
        SourceRange::fromByteOffsets(14, 16),
        SourceRange::fromByteOffsets(4, 9),
        SourceRange::fromByteOffsets(10, 13),
        SourceRange::fromByteOffsets(13, 14),
    };
    for (const auto Operator : {TokenKind::Star, TokenKind::Amp})
    {
      SCOPED_TRACE(tokenizer::tokenKindName(Operator));
      for (std::size_t Index = 0; Index < std::size(InvalidRanges); ++Index)
      {
        SCOPED_TRACE(Index);
        auto *Qualified = Context.make<UnaryExpr>(NodeRange, Operator, OperatorRange, Operand, TypeQualifier{TypeQualifierKind::Const, InvalidRanges[Index]});
        EXPECT_FALSE(verifyAST(Qualified, 16));
      }
    }
  }

  // Unknown qualifier kinds are rejected and do not make a pointer or reference appear const qualified.
  TEST(ASTVerifierTest, TypeQualifierRejectsUnknownKind)
  {
    ASTContext Context;
    auto *Operand = Context.make<MissingExpr>(SourceRange::fromByteOffsets(8, 10));
    for (const auto Operator : {TokenKind::Star, TokenKind::Amp})
    {
      SCOPED_TRACE(tokenizer::tokenKindName(Operator));
      auto *Qualified = Context.make<UnaryExpr>(SourceRange::fromByteOffsets(0, 10), Operator, SourceRange::fromByteOffsets(0, 1), Operand, TypeQualifier{static_cast<TypeQualifierKind>(255), SourceRange::fromByteOffsets(2, 7)});
      ASSERT_TRUE(Qualified->qualifier().has_value());
      EXPECT_FALSE(Qualified->isConstQualified());
      EXPECT_FALSE(verifyAST(Qualified, 10));
    }
  }
} // namespace ink::frontend::parser::test
