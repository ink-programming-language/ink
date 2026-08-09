#include "ink/ast/ast.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <type_traits>

namespace ink::ast
{
  namespace
  {
    template <typename Enum>
    struct NameExpectation
    {
      Enum Value;
      const char *Name;
    };

    struct KindExpectation
    {
      AstKind Kind;
      AstNodeCategory Category;
      const char *Name;
    };

    static_assert(!std::is_convertible<AstDeclId, AstDeclId::ValueType>::value, "AST IDs must not implicitly convert to their storage type");
    static_assert(!std::is_convertible<AstDeclId::ValueType, AstDeclId>::value, "AST IDs must not implicitly construct from their storage type");
    static_assert(!std::is_same<AstDeclId, AstExprId>::value, "AST arena IDs must remain strongly typed");

    // Verifies every AST kind has the stable public name and node category declared by the AST schema.
    TEST(AstKindContractTest, NamesAndClassifiesEveryKind)
    {
      constexpr KindExpectation Expectations[] = {
#define INK_AST_KIND(Name, Category) {AstKind::Name, AstNodeCategory::Category, #Name},
#include "ink/ast/ast_kind.def"
#undef INK_AST_KIND
      };

      for (const KindExpectation &Expectation : Expectations)
      {
        EXPECT_STREQ(astKindName(Expectation.Kind), Expectation.Name);
        EXPECT_EQ(astKindCategory(Expectation.Kind), Expectation.Category);
      }

      const auto OutOfDomain = static_cast<AstKind>(255);
      EXPECT_STREQ(astKindName(OutOfDomain), "Unknown");
      EXPECT_EQ(astKindCategory(OutOfDomain), AstNodeCategory::Unknown);
    }

    // Verifies node, recovery, and expected-token enum names cover every public value and reject out-of-domain values deterministically.
    TEST(AstEnumContractTest, NamesStructuralAndRecoveryEnums)
    {
      constexpr NameExpectation<AstNodeCategory> CategoryExpectations[] = {
          {AstNodeCategory::Unknown, "unknown"},
          {AstNodeCategory::Declaration, "decl"},
          {AstNodeCategory::Expression, "expr"},
          {AstNodeCategory::Statement, "stmt"},
          {AstNodeCategory::Pattern, "pattern"},
      };
      constexpr NameExpectation<AstRecoveryKind> RecoveryExpectations[] = {
          {AstRecoveryKind::MissingToken, "MissingToken"},
          {AstRecoveryKind::UnexpectedSyntax, "UnexpectedSyntax"},
      };
      constexpr NameExpectation<AstExpectedKind> ExpectedKindExpectations[] = {
          {AstExpectedKind::Unknown, "Unknown"},
          {AstExpectedKind::Identifier, "Identifier"},
          {AstExpectedKind::Keyword, "Keyword"},
          {AstExpectedKind::BuiltinType, "BuiltinType"},
          {AstExpectedKind::Literal, "Literal"},
          {AstExpectedKind::Symbol, "Symbol"},
          {AstExpectedKind::EndOfFile, "EndOfFile"},
      };

      for (const auto &Expectation : CategoryExpectations)
      {
        EXPECT_STREQ(astNodeCategoryName(Expectation.Value), Expectation.Name);
      }
      for (const auto &Expectation : RecoveryExpectations)
      {
        EXPECT_STREQ(astRecoveryKindName(Expectation.Value), Expectation.Name);
      }
      for (const auto &Expectation : ExpectedKindExpectations)
      {
        EXPECT_STREQ(astExpectedKindName(Expectation.Value), Expectation.Name);
      }

      EXPECT_STREQ(astNodeCategoryName(static_cast<AstNodeCategory>(255)), "unknown");
      EXPECT_STREQ(astRecoveryKindName(static_cast<AstRecoveryKind>(255)), "Unknown");
      EXPECT_STREQ(astExpectedKindName(static_cast<AstExpectedKind>(255)), "Unknown");
    }

    // Verifies all lowering payload enum values have stable diagnostic names, including deterministic fallbacks for invalid values.
    TEST(AstEnumContractTest, NamesLoweringPayloadEnums)
    {
      constexpr NameExpectation<UnsupportedFeature> FeatureExpectations[] = {
          {UnsupportedFeature::Unknown, "Unknown"},
          {UnsupportedFeature::Attribute, "Attribute"},
          {UnsupportedFeature::Decorator, "Decorator"},
          {UnsupportedFeature::DeclarationModifier, "DeclarationModifier"},
          {UnsupportedFeature::ConstructorInitializer, "ConstructorInitializer"},
          {UnsupportedFeature::CallArgument, "CallArgument"},
          {UnsupportedFeature::Import, "Import"},
          {UnsupportedFeature::Generic, "Generic"},
          {UnsupportedFeature::Class, "Class"},
          {UnsupportedFeature::Interface, "Interface"},
          {UnsupportedFeature::Enum, "Enum"},
          {UnsupportedFeature::Comptime, "Comptime"},
          {UnsupportedFeature::Match, "Match"},
          {UnsupportedFeature::For, "For"},
          {UnsupportedFeature::Defer, "Defer"},
          {UnsupportedFeature::Throw, "Throw"},
          {UnsupportedFeature::Try, "Try"},
          {UnsupportedFeature::Aggregate, "Aggregate"},
          {UnsupportedFeature::Array, "Array"},
          {UnsupportedFeature::Index, "Index"},
          {UnsupportedFeature::Slice, "Slice"},
          {UnsupportedFeature::MemberAccess, "MemberAccess"},
          {UnsupportedFeature::PointerMemberAccess, "PointerMemberAccess"},
          {UnsupportedFeature::TypeConstructor, "TypeConstructor"},
          {UnsupportedFeature::ComplexType, "ComplexType"},
          {UnsupportedFeature::Tuple, "Tuple"},
      };
      constexpr NameExpectation<AstBindingMode> BindingExpectations[] = {
          {AstBindingMode::Unknown, "Unknown"},
          {AstBindingMode::Let, "Let"},
          {AstBindingMode::Var, "Var"},
          {AstBindingMode::Const, "Const"},
      };
      constexpr NameExpectation<AstParameterFlavor> ParameterExpectations[] = {
          {AstParameterFlavor::Function, "Function"},
          {AstParameterFlavor::Generic, "Generic"},
      };
      constexpr NameExpectation<AstFunctionFlavor> FunctionExpectations[] = {
          {AstFunctionFlavor::Function, "Function"},
          {AstFunctionFlavor::Decorator, "Decorator"},
          {AstFunctionFlavor::Destructor, "Destructor"},
      };
      constexpr NameExpectation<AstLiteralKind> LiteralExpectations[] = {
          {AstLiteralKind::Bool, "Bool"},
          {AstLiteralKind::Null, "Null"},
          {AstLiteralKind::Integer, "Integer"},
          {AstLiteralKind::Float, "Float"},
          {AstLiteralKind::Scalar, "Scalar"},
          {AstLiteralKind::String, "String"},
      };

      for (const auto &Expectation : FeatureExpectations)
      {
        EXPECT_STREQ(unsupportedFeatureName(Expectation.Value), Expectation.Name);
      }
      for (const auto &Expectation : BindingExpectations)
      {
        EXPECT_STREQ(astBindingModeName(Expectation.Value), Expectation.Name);
      }
      for (const auto &Expectation : ParameterExpectations)
      {
        EXPECT_STREQ(astParameterFlavorName(Expectation.Value), Expectation.Name);
      }
      for (const auto &Expectation : FunctionExpectations)
      {
        EXPECT_STREQ(astFunctionFlavorName(Expectation.Value), Expectation.Name);
      }
      for (const auto &Expectation : LiteralExpectations)
      {
        EXPECT_STREQ(astLiteralKindName(Expectation.Value), Expectation.Name);
      }

      EXPECT_STREQ(unsupportedFeatureName(static_cast<UnsupportedFeature>(255)), "Unknown");
      EXPECT_STREQ(astBindingModeName(static_cast<AstBindingMode>(255)), "Unknown");
      EXPECT_STREQ(astParameterFlavorName(static_cast<AstParameterFlavor>(255)), "Unknown");
      EXPECT_STREQ(astFunctionFlavorName(static_cast<AstFunctionFlavor>(255)), "Unknown");
      EXPECT_STREQ(astLiteralKindName(static_cast<AstLiteralKind>(255)), "Unknown");
    }

    // Verifies typed AST IDs, heterogeneous node references, and CST origins preserve their validity and identity boundaries.
    TEST(AstIdentityContractTest, PreservesTypedReferencesAndOrigins)
    {
      constexpr AstDeclId First = AstDeclId::fromValue(1);
      constexpr AstDeclId Equal = AstDeclId::fromValue(1);
      constexpr AstDeclId Later = AstDeclId::fromValue(2);
      constexpr AstNodeRef Declaration = AstNodeRef::declaration(First);
      constexpr AstNodeRef Expression = AstNodeRef::expression(AstExprId::fromValue(1));
      constexpr CstOrigin Node = CstOrigin::node(7);
      constexpr CstOrigin Element = CstOrigin::element(7, 3);

      EXPECT_FALSE(AstDeclId{}.isValid());
      EXPECT_EQ(First, Equal);
      EXPECT_NE(First, Later);
      EXPECT_LT(First, Later);
      EXPECT_TRUE(Declaration.isValid());
      EXPECT_NE(Declaration, Expression);
      EXPECT_FALSE(AstNodeRef{}.isValid());
      EXPECT_TRUE(Node.isValid());
      EXPECT_FALSE(Node.hasElement());
      EXPECT_TRUE(Element.hasElement());
      EXPECT_EQ(Element.node(), 7U);
      EXPECT_EQ(Element.element(), 3U);
      EXPECT_NE(Node, Element);
    }

    // Verifies AST half-open arena and table ranges include exact boundaries and return zero size for invalid or reversed ranges.
    TEST(AstRangeContractTest, HandlesBoundariesAndInvalidRanges)
    {
      constexpr AstArenaRange<AstExprId> Arena{AstExprId::fromValue(3), AstExprId::fromValue(7)};
      constexpr AstArenaRange<AstExprId> ReversedArena{AstExprId::fromValue(7), AstExprId::fromValue(3)};
      constexpr AstArenaRange<AstExprId> InvalidArena{};
      constexpr AstTableRange Table{3, 7};
      constexpr AstTableRange ReversedTable{7, 3};

      EXPECT_EQ(Arena.size(), 4U);
      EXPECT_TRUE(Arena.contains(AstExprId::fromValue(3)));
      EXPECT_TRUE(Arena.contains(AstExprId::fromValue(6)));
      EXPECT_FALSE(Arena.contains(AstExprId::fromValue(7)));
      EXPECT_FALSE(Arena.contains({}));
      EXPECT_EQ(ReversedArena.size(), 0U);
      EXPECT_EQ(InvalidArena.size(), 0U);
      EXPECT_EQ(Table.size(), 4U);
      EXPECT_TRUE(Table.contains(3, 4));
      EXPECT_TRUE(Table.contains(7, 0));
      EXPECT_FALSE(Table.contains(2, 1));
      EXPECT_FALSE(Table.contains(6, 2));
      EXPECT_EQ(ReversedTable.size(), 0U);
      EXPECT_FALSE(ReversedTable.contains(7, 0));
      EXPECT_TRUE((AstNodeList{9, 0}).empty());
      EXPECT_FALSE((AstNodeList{9, 1}).empty());
      EXPECT_TRUE((AstRecoveryRange{9, 0}).empty());
      EXPECT_FALSE((AstRecoveryRange{9, 1}).empty());
    }
  } // namespace
} // namespace ink::ast
