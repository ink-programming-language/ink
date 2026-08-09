#include "ink/ast/ast.h"

namespace ink::ast
{
  const char *astKindName(AstKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_AST_KIND(Name, Category) case AstKind::Name: return #Name;
#include "ink/ast/ast_kind.def"
#undef INK_AST_KIND
    }
    return "Unknown";
  }

  const char *astNodeCategoryName(AstNodeCategory Category) noexcept
  {
    switch (Category)
    {
    case AstNodeCategory::Unknown:
      return "unknown";
    case AstNodeCategory::Declaration:
      return "decl";
    case AstNodeCategory::Expression:
      return "expr";
    case AstNodeCategory::Statement:
      return "stmt";
    case AstNodeCategory::Pattern:
      return "pattern";
    }
    return "unknown";
  }

  AstNodeCategory astKindCategory(AstKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_AST_KIND(Name, Category) case AstKind::Name: return AstNodeCategory::Category;
#include "ink/ast/ast_kind.def"
#undef INK_AST_KIND
    }
    return AstNodeCategory::Unknown;
  }

  const char *astRecoveryKindName(AstRecoveryKind Kind) noexcept
  {
    switch (Kind)
    {
    case AstRecoveryKind::MissingToken:
      return "MissingToken";
    case AstRecoveryKind::UnexpectedSyntax:
      return "UnexpectedSyntax";
    }
    return "Unknown";
  }

  const char *astExpectedKindName(AstExpectedKind Kind) noexcept
  {
    switch (Kind)
    {
    case AstExpectedKind::Unknown:
      return "Unknown";
    case AstExpectedKind::Identifier:
      return "Identifier";
    case AstExpectedKind::Keyword:
      return "Keyword";
    case AstExpectedKind::BuiltinType:
      return "BuiltinType";
    case AstExpectedKind::Literal:
      return "Literal";
    case AstExpectedKind::Symbol:
      return "Symbol";
    case AstExpectedKind::EndOfFile:
      return "EndOfFile";
    }
    return "Unknown";
  }

  const char *unsupportedFeatureName(UnsupportedFeature Feature) noexcept
  {
    switch (Feature)
    {
    case UnsupportedFeature::Unknown:
      return "Unknown";
    case UnsupportedFeature::Attribute:
      return "Attribute";
    case UnsupportedFeature::Decorator:
      return "Decorator";
    case UnsupportedFeature::DeclarationModifier:
      return "DeclarationModifier";
    case UnsupportedFeature::ConstructorInitializer:
      return "ConstructorInitializer";
    case UnsupportedFeature::CallArgument:
      return "CallArgument";
    case UnsupportedFeature::Import:
      return "Import";
    case UnsupportedFeature::Generic:
      return "Generic";
    case UnsupportedFeature::Class:
      return "Class";
    case UnsupportedFeature::Interface:
      return "Interface";
    case UnsupportedFeature::Enum:
      return "Enum";
    case UnsupportedFeature::Comptime:
      return "Comptime";
    case UnsupportedFeature::Match:
      return "Match";
    case UnsupportedFeature::For:
      return "For";
    case UnsupportedFeature::Defer:
      return "Defer";
    case UnsupportedFeature::Throw:
      return "Throw";
    case UnsupportedFeature::Try:
      return "Try";
    case UnsupportedFeature::Aggregate:
      return "Aggregate";
    case UnsupportedFeature::Array:
      return "Array";
    case UnsupportedFeature::Index:
      return "Index";
    case UnsupportedFeature::Slice:
      return "Slice";
    case UnsupportedFeature::MemberAccess:
      return "MemberAccess";
    case UnsupportedFeature::PointerMemberAccess:
      return "PointerMemberAccess";
    case UnsupportedFeature::TypeConstructor:
      return "TypeConstructor";
    case UnsupportedFeature::ComplexType:
      return "ComplexType";
    case UnsupportedFeature::Tuple:
      return "Tuple";
    }
    return "Unknown";
  }

  const char *astBindingModeName(AstBindingMode Mode) noexcept
  {
    switch (Mode)
    {
    case AstBindingMode::Unknown:
      return "Unknown";
    case AstBindingMode::Let:
      return "Let";
    case AstBindingMode::Var:
      return "Var";
    case AstBindingMode::Const:
      return "Const";
    }
    return "Unknown";
  }

  const char *astParameterFlavorName(AstParameterFlavor Flavor) noexcept
  {
    switch (Flavor)
    {
    case AstParameterFlavor::Function:
      return "Function";
    case AstParameterFlavor::Generic:
      return "Generic";
    }
    return "Unknown";
  }

  const char *astFunctionFlavorName(AstFunctionFlavor Flavor) noexcept
  {
    switch (Flavor)
    {
    case AstFunctionFlavor::Function:
      return "Function";
    case AstFunctionFlavor::Decorator:
      return "Decorator";
    case AstFunctionFlavor::Destructor:
      return "Destructor";
    }
    return "Unknown";
  }

  const char *astLiteralKindName(AstLiteralKind Kind) noexcept
  {
    switch (Kind)
    {
    case AstLiteralKind::Bool:
      return "Bool";
    case AstLiteralKind::Null:
      return "Null";
    case AstLiteralKind::Integer:
      return "Integer";
    case AstLiteralKind::Float:
      return "Float";
    case AstLiteralKind::Scalar:
      return "Scalar";
    case AstLiteralKind::String:
      return "String";
    }
    return "Unknown";
  }

  core::SourceFileId AstFile::sourceFile() const noexcept
  {
    return SourceFile;
  }

  AstDeclId AstFile::root() const noexcept
  {
    return Root;
  }

  AstArenaRange<AstDeclId> AstFile::declarations() const noexcept
  {
    return Declarations;
  }

  AstArenaRange<AstExprId> AstFile::expressions() const noexcept
  {
    return Expressions;
  }

  AstArenaRange<AstStmtId> AstFile::statements() const noexcept
  {
    return Statements;
  }

  AstArenaRange<AstPatternId> AstFile::patterns() const noexcept
  {
    return Patterns;
  }

  AstTableRange AstFile::listElements() const noexcept
  {
    return ListElementRange;
  }

  AstTableRange AstFile::recoveries() const noexcept
  {
    return RecoveryRange;
  }

  std::size_t AstFile::sourceSize() const noexcept
  {
    return SourceSize;
  }

  std::size_t AstFile::cstNodeCount() const noexcept
  {
    return CstChildCounts.size();
  }

  std::size_t AstFile::cstChildCount(std::size_t Node) const
  {
    return CstChildCounts.at(Node);
  }
} // namespace ink::ast
