#include "ink/semantic/model/context.h"
#include "ink/parser/parser.h"

#include <gtest/gtest.h>

#include <type_traits>

namespace ink::semantic::test
{
  static_assert(std::is_base_of_v<Value, Type>);
  static_assert(std::is_base_of_v<Value, Constant>);
  static_assert(std::is_base_of_v<Value, ExprValue>);
  static_assert(!std::is_base_of_v<Value, Decl>);
  static_assert(!std::is_copy_constructible_v<VarDecl>);

  // Type values use the unique metatype, whose own value type closes the cycle.
  TEST(SemanticModelTest, TypesAndConstantsHaveDistinctKindsAndValueTypes)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Type &Meta = Context.getMetaType();
    EXPECT_EQ(&Meta.type(), &Meta);
    EXPECT_EQ(&Context.getVoidType().type(), &Meta);
    EXPECT_EQ(&Context.getBoolType().type(), &Meta);
    EXPECT_EQ(Meta.typeKind(), TypeKind::Meta);
    EXPECT_EQ(Context.getVoidType().typeKind(), TypeKind::Void);
    EXPECT_EQ(Context.getBoolType().typeKind(), TypeKind::Bool);
    EXPECT_TRUE(Type::classof(&Meta));
    EXPECT_FALSE(Constant::classof(&Meta));
    const Value &Boolean = Context.getBoolConstant(true);
    EXPECT_EQ(&Boolean.type(), &Context.getBoolType());
    EXPECT_TRUE(Constant::classof(&Boolean));
    EXPECT_TRUE(BoolConstant::classof(&Boolean));
    EXPECT_FALSE(Type::classof(&Boolean));
    EXPECT_FALSE(IntegerConstant::classof(&Boolean));
    EXPECT_EQ(&Context.compilationContext(), &Compilation);
  }

  // Integer types are canonical by width and signedness, with zero width rejected.
  TEST(SemanticModelTest, IntegerTypesAreCanonicalAndContextLocal)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Signed32 = Context.getIntegerType(32, true);
    ASSERT_NE(Signed32, nullptr);
    EXPECT_EQ(Signed32, Context.getIntegerType(32, true));
    EXPECT_NE(Signed32, Context.getIntegerType(32, false));
    EXPECT_NE(Signed32, Context.getIntegerType(64, true));
    EXPECT_NE(Signed32, Other.getIntegerType(32, true));
    EXPECT_EQ(Signed32->bitWidth(), 32U);
    EXPECT_TRUE(Signed32->isSigned());
    EXPECT_EQ(&Signed32->type(), &Context.getMetaType());
    EXPECT_TRUE(IntegerType::classof(Signed32));
    EXPECT_FALSE(IntegerType::classof(&Context.getBoolType()));
    EXPECT_EQ(Context.getIntegerType(0, true), nullptr);
  }

  // Constants reuse exact typed bit patterns without merging signed and unsigned values.
  TEST(SemanticModelTest, IntegerConstantsPreserveWideValuesAndSignedness)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const IntegerType *Signed = Context.getIntegerType(128, true);
    const IntegerType *Unsigned = Context.getIntegerType(128, false);
    llvm::APInt Bits = llvm::APInt::getAllOnes(128);
    const IntegerConstant *Negative = Context.getIntegerConstant(*Signed, Bits);
    const IntegerConstant *Positive = Context.getIntegerConstant(*Unsigned, Bits);
    ASSERT_NE(Negative, nullptr);
    ASSERT_NE(Positive, nullptr);
    EXPECT_EQ(Negative, Context.getIntegerConstant(*Signed, llvm::APInt::getAllOnes(128)));
    EXPECT_NE(Negative, Positive);
    EXPECT_EQ(Negative->value().getBitWidth(), 128U);
    EXPECT_TRUE(Negative->value().isAllOnes());
    EXPECT_EQ(&Negative->type(), Signed);
    EXPECT_EQ(&Positive->type(), Unsigned);
    Bits.clearAllBits();
    EXPECT_TRUE(Negative->value().isAllOnes());
    EXPECT_NE(Negative, Context.getIntegerConstant(*Signed, Bits));
    EXPECT_TRUE(IntegerConstant::classof(Negative));
  }

  // Invalid constant requests report failure instead of truncating bits or borrowing foreign types.
  TEST(SemanticModelTest, IntegerConstantsRejectMismatchedWidthsAndForeignTypes)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const IntegerType *Signed32 = Context.getIntegerType(32, true);
    EXPECT_EQ(Context.getIntegerConstant(*Signed32, llvm::APInt(64, 1)), nullptr);
    EXPECT_EQ(Context.getIntegerConstant(*Other.getIntegerType(32, true), llvm::APInt(32, 1)), nullptr);
    EXPECT_NE(Context.getIntegerConstant(*Signed32, llvm::APInt(32, 1)), nullptr);
  }

  // Repeated bool requests share immutable values and keep false and true distinct.
  TEST(SemanticModelTest, BoolConstantsAreCanonical)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const BoolConstant &False = Context.getBoolConstant(false);
    const BoolConstant &True = Context.getBoolConstant(true);
    EXPECT_EQ(&False, &Context.getBoolConstant(false));
    EXPECT_EQ(&True, &Context.getBoolConstant(true));
    EXPECT_NE(&False, &True);
    EXPECT_FALSE(False.value());
    EXPECT_TRUE(True.value());
    EXPECT_EQ(&False.type(), &Context.getBoolType());
  }

  // Same-spelling variables retain distinct declaration identities and one shared name.
  TEST(SemanticModelTest, VariablesHaveIndependentIdentityAndSingleAssignmentTypes)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    SemanticContext Other(Compilation);
    const Name X = Context.namePool().intern("X");
    VarDecl *First = Context.createVarDecl(X, BindingMutability::Mutable);
    VarDecl *Second = Context.createVarDecl(X, BindingMutability::Immutable);
    ASSERT_NE(First, nullptr);
    ASSERT_NE(Second, nullptr);
    EXPECT_NE(First, Second);
    EXPECT_EQ(First->name(), Second->name());
    EXPECT_EQ(Context.namePool().size(), 1U);
    EXPECT_EQ(First->type(), nullptr);
    EXPECT_EQ(First->initializer(), nullptr);
    EXPECT_EQ(First->mutability(), BindingMutability::Mutable);
    EXPECT_EQ(Second->mutability(), BindingMutability::Immutable);
    EXPECT_TRUE(First->isMutable());
    EXPECT_FALSE(Second->isMutable());
    EXPECT_TRUE(VarDecl::classof(First));
    EXPECT_FALSE(First->setType(*Other.getIntegerType(32, true)));
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    EXPECT_TRUE(First->setType(*Int32));
    EXPECT_TRUE(First->setType(*Int32));
    EXPECT_FALSE(First->setType(*Context.getIntegerType(64, true)));
    EXPECT_EQ(First->type(), Int32);
    EXPECT_EQ(Context.createVarDecl(Name{}, BindingMutability::Mutable), nullptr);
    EXPECT_EQ(Context.createVarDecl(X, static_cast<BindingMutability>(255)), nullptr);
    EXPECT_EQ(&First->context(), &Context);
  }

  // Container growth preserves declaration, type and constant addresses used as identities.
  TEST(SemanticModelTest, ModelIdentitiesSurviveStorageGrowth)
  {
    core::CompilationContext Compilation;
    SemanticContext Context(Compilation);
    const Name X = Context.namePool().intern("X");
    VarDecl *Declaration = Context.createVarDecl(X, BindingMutability::Mutable);
    const IntegerType *Int32 = Context.getIntegerType(32, true);
    const IntegerConstant *Zero = Context.getIntegerConstant(*Int32, llvm::APInt(32, 0));
    ASSERT_NE(Declaration, nullptr);
    ASSERT_NE(Zero, nullptr);
    ASSERT_TRUE(Declaration->setType(*Int32));
    for (std::uint32_t Index = 1; Index <= 1024; ++Index)
    {
      ASSERT_NE(Context.getIntegerType(Index, false), nullptr);
      ASSERT_NE(Context.getIntegerConstant(*Int32, llvm::APInt(32, Index)), nullptr);
      ASSERT_NE(Context.createVarDecl(X, BindingMutability::Mutable), nullptr);
    }
    EXPECT_EQ(Declaration->name(), X);
    EXPECT_EQ(Declaration->type(), Int32);
    EXPECT_EQ(Int32, Context.getIntegerType(32, true));
    EXPECT_EQ(Zero, Context.getIntegerConstant(*Int32, llvm::APInt(32, 0)));
    EXPECT_TRUE(Zero->value().isZero());
  }

  // Parsed var/const declarations retain source and typed expression values with explicit binding mutability.
  TEST(SemanticModelTest, VariableModelCanReferenceAnExistingParsedDeclaration)
  {
    core::CompilationContext Compilation;
    core::FrontendContext Frontend(Compilation);
    struct BindingCase
    {
        const char *Source;
        BindingMutability ExpectedMutability;
    };
    constexpr BindingCase Cases[] = {
        {"var X: int32 = 0;", BindingMutability::Mutable},
        {"const X: int32 = 0;", BindingMutability::Immutable},
    };
    for (const BindingCase &Case : Cases)
    {
      SCOPED_TRACE(Case.Source);
      auto Parsed = parser::parse(Frontend, tokenizer::tokenize(Frontend, Case.Source));
      ASSERT_TRUE(Parsed.succeeded());
      const parser::ModuleAST *Module = Parsed.Unit->root();
      ASSERT_EQ(Module->statements().size(), 1U);
      ASSERT_TRUE(parser::DeclStmt::classof(Module->statements()[0]));
      const auto *Statement = static_cast<const parser::DeclStmt *>(Module->statements()[0]);
      ASSERT_TRUE(parser::VarDecl::classof(Statement->declaration()));
      const auto *Syntax = static_cast<const parser::VarDecl *>(Statement->declaration());
      ASSERT_TRUE(parser::NameBindingPattern::classof(Syntax->binding()));
      const auto *Binding = static_cast<const parser::NameBindingPattern *>(Syntax->binding());
      SemanticContext Context(Compilation);
      const Name X = Context.namePool().intern(Binding->name().Text);
      const DeclSource Source{Parsed.Unit->input().lexedFile().sourceId(), Syntax->getSourceRange(), Syntax};
      const BindingMutability Mutability = Syntax->constant() ? BindingMutability::Immutable : BindingMutability::Mutable;
      const IntegerType *Int32 = Context.getIntegerType(32, true);
      ASSERT_NE(Syntax->initializer(), nullptr);
      const ExprValue *Initializer = Context.createExprValue(*Int32, *Syntax->initializer(), Source.Source);
      ASSERT_NE(Initializer, nullptr);
      VarDecl *Declaration = Context.createVarDecl(X, Mutability, Source, Initializer);
      ASSERT_NE(Declaration, nullptr);
      EXPECT_EQ(Context.namePool().text(Declaration->name()), "X");
      EXPECT_EQ(Declaration->source().Source, Source.Source);
      EXPECT_EQ(Declaration->source().Range, Source.Range);
      EXPECT_EQ(Declaration->source().Syntax, Syntax);
      EXPECT_EQ(Declaration->initializer(), Initializer);
      EXPECT_EQ(&Initializer->expression(), Syntax->initializer());
      EXPECT_EQ(Initializer->sourceId(), Source.Source);
      EXPECT_EQ(&Initializer->context(), &Context);
      EXPECT_EQ(Declaration->mutability(), Case.ExpectedMutability);
      EXPECT_EQ(Declaration->isMutable(), Case.ExpectedMutability == BindingMutability::Mutable);
      EXPECT_TRUE(Declaration->setType(*Int32));
    }
  }
} // namespace ink::semantic::test
