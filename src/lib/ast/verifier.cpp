#include "ink/ast/verifier.h"

#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ast
{
  namespace
  {
    void addError(AstVerificationResult &Result, AstVerificationErrorKind Kind, AstNodeRef Node, std::size_t ElementIndex, std::string Message)
    {
      Result.Errors.push_back({Kind, Node, ElementIndex, std::move(Message)});
    }

    bool validRange(core::SourceRange Range, std::size_t SourceSize) noexcept
    {
      return Range.Start <= Range.End && Range.End <= SourceSize;
    }

    bool containsRange(core::SourceRange Parent, core::SourceRange Child) noexcept
    {
      return Parent.Start <= Child.Start && Child.End <= Parent.End;
    }

    template <typename Id, typename Node>
    bool validArenaRange(AstArenaRange<Id> Range, const std::vector<Node> &Arena) noexcept
    {
      return Range.Begin.isValid() && Range.End.isValid() && Range.Begin.value() <= Range.End.value() && Range.End.value() <= Arena.size();
    }

    bool validTableRange(AstTableRange Range, std::size_t TableSize) noexcept
    {
      return Range.Begin <= Range.End && Range.End <= TableSize;
    }

    bool fileContains(const AstFile &File, AstNodeRef Ref) noexcept
    {
      switch (Ref.Category)
      {
      case AstNodeCategory::Declaration:
        return File.declarations().contains(AstDeclId::fromValue(Ref.Index));
      case AstNodeCategory::Expression:
        return File.expressions().contains(AstExprId::fromValue(Ref.Index));
      case AstNodeCategory::Statement:
        return File.statements().contains(AstStmtId::fromValue(Ref.Index));
      case AstNodeCategory::Pattern:
        return File.patterns().contains(AstPatternId::fromValue(Ref.Index));
      case AstNodeCategory::Unknown:
        return false;
      }
      return false;
    }

    bool validAstKind(AstKind Kind) noexcept
    {
      if (Kind == AstKind::Unknown)
      {
        return false;
      }
      switch (Kind)
      {
#define INK_AST_KIND(Name, Category) case AstKind::Name: return true;
#include "ink/ast/ast_kind.def"
#undef INK_AST_KIND
      }
      return false;
    }

    bool validRecoveryKind(AstRecoveryKind Kind) noexcept
    {
      return Kind == AstRecoveryKind::MissingToken || Kind == AstRecoveryKind::UnexpectedSyntax;
    }

    bool validExpectedKind(AstExpectedKind Kind) noexcept
    {
      switch (Kind)
      {
      case AstExpectedKind::Unknown:
      case AstExpectedKind::Identifier:
      case AstExpectedKind::Keyword:
      case AstExpectedKind::BuiltinType:
      case AstExpectedKind::Literal:
      case AstExpectedKind::Symbol:
      case AstExpectedKind::EndOfFile:
        return true;
      }
      return false;
    }

    bool validUnsupportedFeature(UnsupportedFeature Feature) noexcept
    {
      switch (Feature)
      {
      case UnsupportedFeature::Unknown:
        return false;
      case UnsupportedFeature::Attribute:
      case UnsupportedFeature::Decorator:
      case UnsupportedFeature::DeclarationModifier:
      case UnsupportedFeature::ConstructorInitializer:
      case UnsupportedFeature::CallArgument:
      case UnsupportedFeature::Import:
      case UnsupportedFeature::Generic:
      case UnsupportedFeature::Class:
      case UnsupportedFeature::Interface:
      case UnsupportedFeature::Enum:
      case UnsupportedFeature::Comptime:
      case UnsupportedFeature::Match:
      case UnsupportedFeature::For:
      case UnsupportedFeature::Defer:
      case UnsupportedFeature::Throw:
      case UnsupportedFeature::Try:
      case UnsupportedFeature::Aggregate:
      case UnsupportedFeature::Array:
      case UnsupportedFeature::Index:
      case UnsupportedFeature::Slice:
      case UnsupportedFeature::MemberAccess:
      case UnsupportedFeature::PointerMemberAccess:
      case UnsupportedFeature::TypeConstructor:
      case UnsupportedFeature::ComplexType:
      case UnsupportedFeature::Tuple:
        return true;
      }
      return false;
    }

    bool validBindingMode(AstBindingMode Mode) noexcept
    {
      return Mode == AstBindingMode::Let || Mode == AstBindingMode::Var || Mode == AstBindingMode::Const;
    }

    bool validParameterFlavor(AstParameterFlavor Flavor) noexcept
    {
      return Flavor == AstParameterFlavor::Function || Flavor == AstParameterFlavor::Generic;
    }

    bool validFunctionFlavor(AstFunctionFlavor Flavor) noexcept
    {
      return Flavor == AstFunctionFlavor::Function || Flavor == AstFunctionFlavor::Decorator || Flavor == AstFunctionFlavor::Destructor;
    }

    bool validLiteralKind(AstLiteralKind Kind) noexcept
    {
      switch (Kind)
      {
      case AstLiteralKind::Bool:
      case AstLiteralKind::Null:
      case AstLiteralKind::Integer:
      case AstLiteralKind::Float:
      case AstLiteralKind::Scalar:
      case AstLiteralKind::String:
        return true;
      }
      return false;
    }

    enum class ChildRole : std::uint8_t
    {
      Any,
      TopLevelDeclaration,
      Expression,
      Statement,
      Pattern,
      DeclarationOrStatement,
      ExpressionOrPattern,
      ParameterDeclaration,
      TypeExpression,
      Supplemental,
    };

    bool isTypeUnsupportedFeature(UnsupportedFeature Feature) noexcept
    {
      switch (Feature)
      {
      case UnsupportedFeature::Generic:
      case UnsupportedFeature::Class:
      case UnsupportedFeature::Array:
      case UnsupportedFeature::TypeConstructor:
      case UnsupportedFeature::ComplexType:
      case UnsupportedFeature::Tuple:
        return true;
      default:
        return false;
      }
    }

    bool isTypeExpression(const AstContext &Context, AstNodeRef Ref)
    {
      if (Ref.Category != AstNodeCategory::Expression)
      {
        return false;
      }
      const Expression &Node = Context.expression(AstExprId::fromValue(Ref.Index));
      switch (Node.Kind)
      {
      case AstKind::ErrorExpression:
      case AstKind::TypeNameExpression:
      case AstKind::BuiltinTypeExpression:
      case AstKind::TypeGroupExpression:
      case AstKind::FunctionTypeExpression:
        return true;
      case AstKind::UnsupportedExpression:
      {
        const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload);
        return Payload && isTypeUnsupportedFeature(Payload->Feature);
      }
      default:
        return false;
      }
    }

    bool isSupplementalKind(AstKind Kind) noexcept
    {
      switch (Kind)
      {
      case AstKind::ErrorDeclaration:
      case AstKind::UnsupportedDeclaration:
      case AstKind::ErrorExpression:
      case AstKind::UnsupportedExpression:
      case AstKind::ErrorStatement:
      case AstKind::UnsupportedStatement:
      case AstKind::ErrorPattern:
      case AstKind::UnsupportedPattern:
        return true;
      default:
        return false;
      }
    }

    bool matchesRole(const AstContext &Context, AstNodeRef Ref, ChildRole Role)
    {
      switch (Role)
      {
      case ChildRole::Any:
        return true;
      case ChildRole::TopLevelDeclaration:
        return Ref.Category == AstNodeCategory::Declaration && Context.node(Ref).Kind != AstKind::SourceFile && Context.node(Ref).Kind != AstKind::ParameterDeclaration;
      case ChildRole::Expression:
        return Ref.Category == AstNodeCategory::Expression;
      case ChildRole::Statement:
        return Ref.Category == AstNodeCategory::Statement;
      case ChildRole::Pattern:
        return Ref.Category == AstNodeCategory::Pattern;
      case ChildRole::DeclarationOrStatement:
        return Ref.Category == AstNodeCategory::Statement || (Ref.Category == AstNodeCategory::Declaration && Context.node(Ref).Kind != AstKind::SourceFile && Context.node(Ref).Kind != AstKind::ParameterDeclaration);
      case ChildRole::ExpressionOrPattern:
        return Ref.Category == AstNodeCategory::Expression || Ref.Category == AstNodeCategory::Pattern;
      case ChildRole::ParameterDeclaration:
        return Ref.Category == AstNodeCategory::Declaration && (Context.node(Ref).Kind == AstKind::ParameterDeclaration || Context.node(Ref).Kind == AstKind::ErrorDeclaration);
      case ChildRole::TypeExpression:
        return isTypeExpression(Context, Ref);
      case ChildRole::Supplemental:
        return isSupplementalKind(Context.node(Ref).Kind);
      }
      return false;
    }

    struct ChildEdge
    {
      AstNodeRef Ref;
      std::size_t ElementIndex = 0;
    };

    struct VisitState
    {
      std::uint32_t DeclarationBegin = 0;
      std::uint32_t ExpressionBegin = 0;
      std::uint32_t StatementBegin = 0;
      std::uint32_t PatternBegin = 0;
      std::vector<std::uint8_t> Declarations;
      std::vector<std::uint8_t> Expressions;
      std::vector<std::uint8_t> Statements;
      std::vector<std::uint8_t> Patterns;

      std::uint8_t &state(AstNodeRef Ref)
      {
        switch (Ref.Category)
        {
        case AstNodeCategory::Declaration:
          return Declarations[Ref.Index - DeclarationBegin];
        case AstNodeCategory::Expression:
          return Expressions[Ref.Index - ExpressionBegin];
        case AstNodeCategory::Statement:
          return Statements[Ref.Index - StatementBegin];
        case AstNodeCategory::Pattern:
          return Patterns[Ref.Index - PatternBegin];
        case AstNodeCategory::Unknown:
          break;
        }
        throw std::logic_error("cannot access visit state for an uncategorized AST node");
      }
    };

    struct Frame
    {
      AstNodeRef Ref;
      std::vector<ChildEdge> Children;
      std::size_t NextChild = 0;
      bool Entered = false;
    };

    void appendUnreachableErrors(AstVerificationResult &Result, const AstFile &File, const VisitState &States)
    {
      for (std::uint32_t Index = File.declarations().Begin.value(); Index < File.declarations().End.value(); ++Index)
      {
        if (States.Declarations[Index - States.DeclarationBegin] == 0)
        {
          addError(Result, AstVerificationErrorKind::UnreachableNode, AstNodeRef::declaration(AstDeclId::fromValue(Index)), 0, "AST declaration is not reachable from the file root");
        }
      }
      for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
      {
        if (States.Expressions[Index - States.ExpressionBegin] == 0)
        {
          addError(Result, AstVerificationErrorKind::UnreachableNode, AstNodeRef::expression(AstExprId::fromValue(Index)), 0, "AST expression is not reachable from the file root");
        }
      }
      for (std::uint32_t Index = File.statements().Begin.value(); Index < File.statements().End.value(); ++Index)
      {
        if (States.Statements[Index - States.StatementBegin] == 0)
        {
          addError(Result, AstVerificationErrorKind::UnreachableNode, AstNodeRef::statement(AstStmtId::fromValue(Index)), 0, "AST statement is not reachable from the file root");
        }
      }
      for (std::uint32_t Index = File.patterns().Begin.value(); Index < File.patterns().End.value(); ++Index)
      {
        if (States.Patterns[Index - States.PatternBegin] == 0)
        {
          addError(Result, AstVerificationErrorKind::UnreachableNode, AstNodeRef::pattern(AstPatternId::fromValue(Index)), 0, "AST pattern is not reachable from the file root");
        }
      }
    }
  } // namespace

  const char *astVerificationErrorKindName(AstVerificationErrorKind Kind) noexcept
  {
    switch (Kind)
    {
    case AstVerificationErrorKind::InvalidSourceFile:
      return "InvalidSourceFile";
    case AstVerificationErrorKind::InvalidFileOwner:
      return "InvalidFileOwner";
    case AstVerificationErrorKind::InvalidArenaRange:
      return "InvalidArenaRange";
    case AstVerificationErrorKind::InvalidRoot:
      return "InvalidRoot";
    case AstVerificationErrorKind::InvalidKind:
      return "InvalidKind";
    case AstVerificationErrorKind::InvalidPayload:
      return "InvalidPayload";
    case AstVerificationErrorKind::InvalidEnum:
      return "InvalidEnum";
    case AstVerificationErrorKind::InvalidNodeReference:
      return "InvalidNodeReference";
    case AstVerificationErrorKind::ForeignNodeReference:
      return "ForeignNodeReference";
    case AstVerificationErrorKind::InvalidList:
      return "InvalidList";
    case AstVerificationErrorKind::ForeignListReference:
      return "ForeignListReference";
    case AstVerificationErrorKind::InvalidRecovery:
      return "InvalidRecovery";
    case AstVerificationErrorKind::ForeignRecoveryReference:
      return "ForeignRecoveryReference";
    case AstVerificationErrorKind::KindCategoryMismatch:
      return "KindCategoryMismatch";
    case AstVerificationErrorKind::InvalidSourceRange:
      return "InvalidSourceRange";
    case AstVerificationErrorKind::ChildRangeOutsideParent:
      return "ChildRangeOutsideParent";
    case AstVerificationErrorKind::InvalidOrigin:
      return "InvalidOrigin";
    case AstVerificationErrorKind::DuplicateOrigin:
      return "DuplicateOrigin";
    case AstVerificationErrorKind::DuplicateNode:
      return "DuplicateNode";
    case AstVerificationErrorKind::UnreachableNode:
      return "UnreachableNode";
    case AstVerificationErrorKind::InvalidString:
      return "InvalidString";
    }
    return "Unknown";
  }

  AstVerificationResult AstVerifier::verify(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings) const
  {
    AstVerificationResult Result;
    if (!Context.contains(File))
    {
      addError(Result, AstVerificationErrorKind::InvalidFileOwner, {}, 0, "AST file is not registered with this context");
      return Result;
    }
    if (File.Strings != &Strings)
    {
      addError(Result, AstVerificationErrorKind::InvalidString, {}, 0, "AST file is being verified with a different string interner");
    }
    if (!File.sourceFile().isValid())
    {
      addError(Result, AstVerificationErrorKind::InvalidSourceFile, {}, 0, "AST file has no valid source file identity");
    }

    const bool ValidDeclarationRange = validArenaRange(File.declarations(), Context.declarations());
    const bool ValidExpressionRange = validArenaRange(File.expressions(), Context.expressions());
    const bool ValidStatementRange = validArenaRange(File.statements(), Context.statements());
    const bool ValidPatternRange = validArenaRange(File.patterns(), Context.patterns());
    const bool ValidListRange = validTableRange(File.listElements(), Context.listElements().size());
    const bool ValidRecoveryRange = validTableRange(File.recoveries(), Context.allRecoveries().size());
    if (!ValidDeclarationRange || !ValidExpressionRange || !ValidStatementRange || !ValidPatternRange || !ValidListRange || !ValidRecoveryRange)
    {
      addError(Result, AstVerificationErrorKind::InvalidArenaRange, {}, 0, "AST file owns an invalid arena or table range");
      return Result;
    }
    if (!Context.contains(File.root()) || !File.declarations().contains(File.root()))
    {
      addError(Result, AstVerificationErrorKind::InvalidRoot, {}, 0, "AST file root is outside its declaration arena range");
      return Result;
    }

    const AstNodeRef Root = AstNodeRef::declaration(File.root());
    VisitState States{File.declarations().Begin.value(), File.expressions().Begin.value(), File.statements().Begin.value(), File.patterns().Begin.value(), std::vector<std::uint8_t>(File.declarations().size(), 0), std::vector<std::uint8_t>(File.expressions().size(), 0), std::vector<std::uint8_t>(File.statements().size(), 0), std::vector<std::uint8_t>(File.patterns().size(), 0)};
    std::vector<std::uint8_t> ListElementVisits(File.listElements().size(), 0);
    std::vector<std::uint8_t> RecoveryVisits(File.recoveries().size(), 0);
    std::unordered_set<std::uint64_t> RecoveryOrigins;
    RecoveryOrigins.reserve(File.recoveries().size());
    std::vector<Frame> Work;
    States.state(Root) = 1;
    Work.push_back({Root, {}, 0, false});

    while (!Work.empty())
    {
      Frame &Current = Work.back();
      const AstNodeView View = Context.node(Current.Ref);
      const AstNodeHeader &Header = *View.Header;
      if (!Current.Entered)
      {
        Current.Entered = true;
        std::size_t ElementIndex = 0;
        if (!validAstKind(View.Kind))
        {
          addError(Result, AstVerificationErrorKind::InvalidKind, Current.Ref, 0, "AST node has an unknown or invalid kind");
        }
        else if (astKindCategory(View.Kind) != Current.Ref.Category)
        {
          addError(Result, AstVerificationErrorKind::KindCategoryMismatch, Current.Ref, 0, "AST kind is stored in the wrong typed arena");
        }
        if (Current.Ref != Root && View.Kind == AstKind::SourceFile)
        {
          addError(Result, AstVerificationErrorKind::InvalidRoot, Current.Ref, 0, "SourceFile may only appear as the AST file root");
        }
        if (!validRange(Header.Range, File.sourceSize()))
        {
          addError(Result, AstVerificationErrorKind::InvalidSourceRange, Current.Ref, 0, "AST node source range is outside its file");
        }
        if (Header.Origin)
        {
          if (!Header.Origin->isValid() || Header.Origin->node() >= File.cstNodeCount() || (Header.Origin->hasElement() && Header.Origin->element() >= File.cstChildCount(Header.Origin->node())))
          {
            addError(Result, AstVerificationErrorKind::InvalidOrigin, Current.Ref, 0, "AST node CST origin is outside the parsed file");
          }
        }

        const auto addRequiredString = [&](core::InternedStringId Id, const char *Description)
        {
          if (!Strings.contains(Id))
          {
            addError(Result, AstVerificationErrorKind::InvalidString, Current.Ref, ElementIndex, std::string(Description) + " references an unknown interned string");
          }
          else if (Strings.string(Id).empty())
          {
            addError(Result, AstVerificationErrorKind::InvalidString, Current.Ref, ElementIndex, std::string(Description) + " must not be empty");
          }
        };
        const auto addChild = [&](AstNodeRef Child, ChildRole Role)
        {
          const std::size_t ChildElementIndex = ElementIndex++;
          if (!Child.isValid() || !Context.contains(Child))
          {
            addError(Result, AstVerificationErrorKind::InvalidNodeReference, Current.Ref, ChildElementIndex, "AST payload references an invalid child node");
            return;
          }
          if (!fileContains(File, Child))
          {
            addError(Result, AstVerificationErrorKind::ForeignNodeReference, Current.Ref, ChildElementIndex, "AST payload references a child owned by another file");
            return;
          }
          if (!matchesRole(Context, Child, Role))
          {
            addError(Result, AstVerificationErrorKind::InvalidNodeReference, Current.Ref, ChildElementIndex, "AST payload child has the wrong semantic category or kind");
          }
          if (Child.Category == AstNodeCategory::Declaration && Context.node(Child).Kind == AstKind::BindingDeclaration)
          {
            const BindingPayload *Binding = std::get_if<BindingPayload>(&Context.declaration(AstDeclId::fromValue(Child.Index)).Payload);
            if (Binding && ((Role == ChildRole::TopLevelDeclaration && !Binding->TopLevel) || (Role == ChildRole::DeclarationOrStatement && Binding->TopLevel)))
            {
              addError(Result, AstVerificationErrorKind::InvalidPayload, Current.Ref, ChildElementIndex, "binding declaration top-level flag disagrees with its structural parent");
            }
          }
          Current.Children.push_back({Child, ChildElementIndex});
        };
        const auto addList = [&](AstNodeList List, ChildRole Role)
        {
          if (!Context.contains(List))
          {
            addError(Result, AstVerificationErrorKind::InvalidList, Current.Ref, ElementIndex++, "AST payload list is outside the context table");
            return;
          }
          if (!File.listElements().contains(List.First, List.Count))
          {
            addError(Result, AstVerificationErrorKind::ForeignListReference, Current.Ref, ElementIndex++, "AST payload list is outside the file's table range");
            return;
          }
          for (std::size_t Index = 0; Index < List.Count; ++Index)
          {
            const std::size_t TableIndex = List.First + Index;
            const std::size_t VisitIndex = TableIndex - File.listElements().Begin;
            if (ListElementVisits[VisitIndex] != 0)
            {
              addError(Result, AstVerificationErrorKind::InvalidList, Current.Ref, ElementIndex, "AST list table entry is owned by more than one payload field");
            }
            ListElementVisits[VisitIndex] = 1;
            addChild(Context.listElements()[TableIndex], Role);
          }
        };
        const auto addOptionalExpression = [&](const std::optional<AstExprId> &Id)
        {
          if (Id)
          {
            addChild(AstNodeRef::expression(*Id), ChildRole::Expression);
          }
        };
        const auto addOptionalTypeExpression = [&](const std::optional<AstExprId> &Id)
        {
          if (Id)
          {
            addChild(AstNodeRef::expression(*Id), ChildRole::TypeExpression);
          }
        };
        const auto addOptionalStatement = [&](const std::optional<AstStmtId> &Id)
        {
          if (Id)
          {
            addChild(AstNodeRef::statement(*Id), ChildRole::Statement);
          }
        };
        const auto addUnsupported = [&](const UnsupportedPayload &Payload)
        {
          if (!validUnsupportedFeature(Payload.Feature))
          {
            addError(Result, AstVerificationErrorKind::InvalidEnum, Current.Ref, ElementIndex, "unsupported AST payload has an unknown or invalid feature");
          }
          addList(Payload.Children, ChildRole::Any);
        };
        const auto addErrorPayload = [&](const ErrorPayload &Payload)
        {
          addList(Payload.Recovered, ChildRole::Any);
        };
        const auto invalidPayload = [&]()
        {
          addError(Result, AstVerificationErrorKind::InvalidPayload, Current.Ref, ElementIndex, "AST kind does not match its tagged payload");
        };

        switch (Current.Ref.Category)
        {
        case AstNodeCategory::Declaration:
        {
          const Declaration &Node = Context.declaration(AstDeclId::fromValue(Current.Ref.Index));
          switch (Node.Kind)
          {
          case AstKind::SourceFile:
            if (const SourceFilePayload *Payload = std::get_if<SourceFilePayload>(&Node.Payload))
            {
              addList(Payload->Items, ChildRole::TopLevelDeclaration);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::ErrorDeclaration:
            if (const ErrorPayload *Payload = std::get_if<ErrorPayload>(&Node.Payload))
            {
              addErrorPayload(*Payload);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::ImportDeclaration:
            if (const ImportPayload *Payload = std::get_if<ImportPayload>(&Node.Payload))
            {
              addRequiredString(Payload->Path, "import path");
              if (Payload->Alias)
              {
                addRequiredString(*Payload->Alias, "import alias");
              }
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::BindingDeclaration:
            if (const BindingPayload *Payload = std::get_if<BindingPayload>(&Node.Payload))
            {
              if (!validBindingMode(Payload->Mode))
              {
                addError(Result, AstVerificationErrorKind::InvalidEnum, Current.Ref, ElementIndex, "binding declaration has an unknown or invalid mode");
              }
              addChild(AstNodeRef::pattern(Payload->Pattern), ChildRole::Pattern);
              addOptionalTypeExpression(Payload->Type);
              addOptionalExpression(Payload->Initializer);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::ParameterDeclaration:
            if (const ParameterPayload *Payload = std::get_if<ParameterPayload>(&Node.Payload))
            {
              if (!validParameterFlavor(Payload->Flavor))
              {
                addError(Result, AstVerificationErrorKind::InvalidEnum, Current.Ref, ElementIndex, "parameter declaration has an invalid flavor");
              }
              addRequiredString(Payload->Name, "parameter name");
              addChild(AstNodeRef::expression(Payload->Type), ChildRole::TypeExpression);
              addOptionalExpression(Payload->DefaultValue);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::FunctionDeclaration:
            if (const FunctionPayload *Payload = std::get_if<FunctionPayload>(&Node.Payload))
            {
              if (!validFunctionFlavor(Payload->Flavor))
              {
                addError(Result, AstVerificationErrorKind::InvalidEnum, Current.Ref, ElementIndex, "function declaration has an invalid flavor");
              }
              addRequiredString(Payload->Name, "function name");
              addList(Payload->Parameters, ChildRole::ParameterDeclaration);
              addOptionalTypeExpression(Payload->ResultType);
              addOptionalStatement(Payload->Body);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::UnsupportedDeclaration:
            if (const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload))
            {
              addUnsupported(*Payload);
            }
            else
            {
              invalidPayload();
            }
            break;
          default:
            if (astKindCategory(Node.Kind) == AstNodeCategory::Declaration)
            {
              invalidPayload();
            }
            break;
          }
          break;
        }
        case AstNodeCategory::Expression:
        {
          const Expression &Node = Context.expression(AstExprId::fromValue(Current.Ref.Index));
          switch (Node.Kind)
          {
          case AstKind::ErrorExpression:
            if (const ErrorPayload *Payload = std::get_if<ErrorPayload>(&Node.Payload))
            {
              addErrorPayload(*Payload);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::LiteralExpression:
            if (const LiteralPayload *Payload = std::get_if<LiteralPayload>(&Node.Payload))
            {
              if (!validLiteralKind(Payload->Kind))
              {
                addError(Result, AstVerificationErrorKind::InvalidEnum, Current.Ref, ElementIndex, "literal expression has an invalid literal kind");
              }
              addRequiredString(Payload->Spelling, "literal spelling");
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::NameExpression:
          case AstKind::TypeNameExpression:
          case AstKind::BuiltinTypeExpression:
            if (const NamePayload *Payload = std::get_if<NamePayload>(&Node.Payload))
            {
              addRequiredString(Payload->Name, "name expression");
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::ThisExpression:
            if (!std::holds_alternative<ThisPayload>(Node.Payload))
            {
              invalidPayload();
            }
            break;
          case AstKind::GroupExpression:
            if (const GroupPayload *Payload = std::get_if<GroupPayload>(&Node.Payload))
            {
              addChild(AstNodeRef::expression(Payload->Value), ChildRole::Expression);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::TypeGroupExpression:
            if (const GroupPayload *Payload = std::get_if<GroupPayload>(&Node.Payload))
            {
              addChild(AstNodeRef::expression(Payload->Value), ChildRole::TypeExpression);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::UnaryExpression:
          case AstKind::ComptimeExpression:
            if (const UnaryPayload *Payload = std::get_if<UnaryPayload>(&Node.Payload))
            {
              addRequiredString(Payload->Operator, "unary operator");
              addChild(AstNodeRef::expression(Payload->Operand), ChildRole::Expression);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::BinaryExpression:
            if (const BinaryPayload *Payload = std::get_if<BinaryPayload>(&Node.Payload))
            {
              addChild(AstNodeRef::expression(Payload->Left), ChildRole::Expression);
              addRequiredString(Payload->Operator, "binary operator");
              addChild(AstNodeRef::expression(Payload->Right), ChildRole::Expression);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::CallExpression:
            if (const CallPayload *Payload = std::get_if<CallPayload>(&Node.Payload))
            {
              addChild(AstNodeRef::expression(Payload->Callee), ChildRole::Expression);
              addList(Payload->Arguments, ChildRole::Expression);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::IfExpression:
            if (const IfExpressionPayload *Payload = std::get_if<IfExpressionPayload>(&Node.Payload))
            {
              addChild(AstNodeRef::expression(Payload->Condition), ChildRole::Expression);
              addChild(AstNodeRef::expression(Payload->ThenValue), ChildRole::Expression);
              addChild(AstNodeRef::expression(Payload->ElseValue), ChildRole::Expression);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::FunctionTypeExpression:
            if (const FunctionTypePayload *Payload = std::get_if<FunctionTypePayload>(&Node.Payload))
            {
              addList(Payload->Parameters, ChildRole::TypeExpression);
              addOptionalTypeExpression(Payload->Result);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::UnsupportedExpression:
            if (const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload))
            {
              addUnsupported(*Payload);
            }
            else
            {
              invalidPayload();
            }
            break;
          default:
            if (astKindCategory(Node.Kind) == AstNodeCategory::Expression)
            {
              invalidPayload();
            }
            break;
          }
          break;
        }
        case AstNodeCategory::Statement:
        {
          const Statement &Node = Context.statement(AstStmtId::fromValue(Current.Ref.Index));
          switch (Node.Kind)
          {
          case AstKind::ErrorStatement:
            if (const ErrorPayload *Payload = std::get_if<ErrorPayload>(&Node.Payload))
            {
              addErrorPayload(*Payload);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::BlockStatement:
            if (const BlockPayload *Payload = std::get_if<BlockPayload>(&Node.Payload))
            {
              addList(Payload->Items, ChildRole::DeclarationOrStatement);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::AssignmentStatement:
            if (const AssignmentPayload *Payload = std::get_if<AssignmentPayload>(&Node.Payload))
            {
              addChild(AstNodeRef::expression(Payload->Left), ChildRole::Expression);
              addRequiredString(Payload->Operator, "assignment operator");
              addChild(AstNodeRef::expression(Payload->Right), ChildRole::Expression);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::ExpressionStatement:
            if (const ExpressionStatementPayload *Payload = std::get_if<ExpressionStatementPayload>(&Node.Payload))
            {
              addChild(AstNodeRef::expression(Payload->Value), ChildRole::Expression);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::IfStatement:
            if (const IfStatementPayload *Payload = std::get_if<IfStatementPayload>(&Node.Payload))
            {
              addChild(Payload->Condition, ChildRole::ExpressionOrPattern);
              addChild(AstNodeRef::statement(Payload->ThenBlock), ChildRole::Statement);
              addOptionalStatement(Payload->ElseBlock);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::WhileStatement:
            if (const WhileStatementPayload *Payload = std::get_if<WhileStatementPayload>(&Node.Payload))
            {
              addChild(Payload->Condition, ChildRole::ExpressionOrPattern);
              addChild(AstNodeRef::statement(Payload->Body), ChildRole::Statement);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::ReturnStatement:
            if (const ReturnPayload *Payload = std::get_if<ReturnPayload>(&Node.Payload))
            {
              addOptionalExpression(Payload->Value);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::BreakStatement:
          case AstKind::ContinueStatement:
            if (!std::holds_alternative<ControlStatementPayload>(Node.Payload))
            {
              invalidPayload();
            }
            break;
          case AstKind::UnsupportedStatement:
            if (const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload))
            {
              addUnsupported(*Payload);
            }
            else
            {
              invalidPayload();
            }
            break;
          default:
            if (astKindCategory(Node.Kind) == AstNodeCategory::Statement)
            {
              invalidPayload();
            }
            break;
          }
          break;
        }
        case AstNodeCategory::Pattern:
        {
          const Pattern &Node = Context.pattern(AstPatternId::fromValue(Current.Ref.Index));
          switch (Node.Kind)
          {
          case AstKind::ErrorPattern:
            if (const ErrorPayload *Payload = std::get_if<ErrorPayload>(&Node.Payload))
            {
              addErrorPayload(*Payload);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::BindingPattern:
            if (const BindingPatternPayload *Payload = std::get_if<BindingPatternPayload>(&Node.Payload))
            {
              addRequiredString(Payload->Name, "binding pattern name");
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::WildcardPattern:
            if (!std::holds_alternative<WildcardPatternPayload>(Node.Payload))
            {
              invalidPayload();
            }
            break;
          case AstKind::TuplePattern:
            if (const TuplePatternPayload *Payload = std::get_if<TuplePatternPayload>(&Node.Payload))
            {
              addList(Payload->Elements, ChildRole::Pattern);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::VariantPattern:
            if (const VariantPatternPayload *Payload = std::get_if<VariantPatternPayload>(&Node.Payload))
            {
              addRequiredString(Payload->Name, "variant pattern name");
              addList(Payload->Elements, ChildRole::Pattern);
            }
            else
            {
              invalidPayload();
            }
            break;
          case AstKind::UnsupportedPattern:
            if (const UnsupportedPayload *Payload = std::get_if<UnsupportedPayload>(&Node.Payload))
            {
              addUnsupported(*Payload);
            }
            else
            {
              invalidPayload();
            }
            break;
          default:
            if (astKindCategory(Node.Kind) == AstNodeCategory::Pattern)
            {
              invalidPayload();
            }
            break;
          }
          break;
        }
        case AstNodeCategory::Unknown:
          break;
        }

        if (!Context.contains(Header.Recoveries))
        {
          addError(Result, AstVerificationErrorKind::InvalidRecovery, Current.Ref, ElementIndex, "AST recovery range is outside the context table");
        }
        else if (!File.recoveries().contains(Header.Recoveries.First, Header.Recoveries.Count))
        {
          addError(Result, AstVerificationErrorKind::ForeignRecoveryReference, Current.Ref, ElementIndex, "AST recovery range is outside the file's table range");
        }
        else
        {
          for (std::size_t Index = 0; Index < Header.Recoveries.Count; ++Index)
          {
            const std::size_t TableIndex = Header.Recoveries.First + Index;
            const std::size_t VisitIndex = TableIndex - File.recoveries().Begin;
            if (RecoveryVisits[VisitIndex] != 0)
            {
              addError(Result, AstVerificationErrorKind::InvalidRecovery, Current.Ref, ElementIndex, "AST recovery table entry is owned by more than one node");
            }
            RecoveryVisits[VisitIndex] = 1;
            const AstRecovery &Recovery = Context.allRecoveries()[TableIndex];
            if (!validRecoveryKind(Recovery.Kind))
            {
              addError(Result, AstVerificationErrorKind::InvalidEnum, Current.Ref, ElementIndex, "AST recovery has an invalid kind");
            }
            if (!validExpectedKind(Recovery.ExpectedKind) || (Recovery.Kind == AstRecoveryKind::MissingToken && Recovery.ExpectedKind == AstExpectedKind::Unknown))
            {
              addError(Result, AstVerificationErrorKind::InvalidRecovery, Current.Ref, ElementIndex, "missing-token recovery has no valid expected token kind");
            }
            if (!validRange(Recovery.Range, File.sourceSize()))
            {
              addError(Result, AstVerificationErrorKind::InvalidSourceRange, Current.Ref, ElementIndex, "AST recovery source range is outside its file");
            }
            else if (validRange(Header.Range, File.sourceSize()) && !containsRange(Header.Range, Recovery.Range))
            {
              addError(Result, AstVerificationErrorKind::ChildRangeOutsideParent, Current.Ref, ElementIndex, "AST recovery source range is outside its owner node");
            }
            if (Recovery.Kind == AstRecoveryKind::MissingToken && !Recovery.Range.empty())
            {
              addError(Result, AstVerificationErrorKind::InvalidRecovery, Current.Ref, ElementIndex, "missing-token recovery must have a zero-width source range");
            }
            if (!Strings.contains(Recovery.Spelling))
            {
              addError(Result, AstVerificationErrorKind::InvalidString, Current.Ref, ElementIndex, "AST recovery references an unknown interned spelling");
            }
            else if (Strings.string(Recovery.Spelling).empty())
            {
              addError(Result, AstVerificationErrorKind::InvalidString, Current.Ref, ElementIndex, "AST recovery spelling must not be empty");
            }
            if (!Recovery.Origin.isValid() || Recovery.Origin.node() >= File.cstNodeCount() || (Recovery.Origin.hasElement() && Recovery.Origin.element() >= File.cstChildCount(Recovery.Origin.node())))
            {
              addError(Result, AstVerificationErrorKind::InvalidOrigin, Current.Ref, ElementIndex, "AST recovery CST origin is outside the parsed file");
            }
            else if (Recovery.Kind == AstRecoveryKind::MissingToken && !Recovery.Origin.hasElement())
            {
              addError(Result, AstVerificationErrorKind::InvalidOrigin, Current.Ref, ElementIndex, "missing-token recovery must identify its CST element");
            }
            else if (Recovery.Origin.hasElement())
            {
              const std::uint64_t Key = (static_cast<std::uint64_t>(Recovery.Origin.node()) << 32U) | Recovery.Origin.element();
              if (!RecoveryOrigins.insert(Key).second)
              {
                addError(Result, AstVerificationErrorKind::DuplicateOrigin, Current.Ref, ElementIndex, "CST recovery element is referenced more than once");
              }
            }
          }
        }

        addList(Header.Supplemental, ChildRole::Supplemental);
      }

      if (Current.NextChild != Current.Children.size())
      {
        const ChildEdge Edge = Current.Children[Current.NextChild++];
        const AstNodeHeader &ChildHeader = *Context.node(Edge.Ref).Header;
        if (validRange(Header.Range, File.sourceSize()) && validRange(ChildHeader.Range, File.sourceSize()) && !containsRange(Header.Range, ChildHeader.Range))
        {
          addError(Result, AstVerificationErrorKind::ChildRangeOutsideParent, Current.Ref, Edge.ElementIndex, "AST child source range is outside its parent node");
        }
        std::uint8_t &ChildState = States.state(Edge.Ref);
        if (ChildState != 0)
        {
          addError(Result, AstVerificationErrorKind::DuplicateNode, Current.Ref, Edge.ElementIndex, ChildState == 1 ? "AST contains a node-reference cycle" : "AST node is reachable more than once");
          continue;
        }
        ChildState = 1;
        Work.push_back({Edge.Ref, {}, 0, false});
        continue;
      }

      States.state(Current.Ref) = 2;
      Work.pop_back();
    }

    appendUnreachableErrors(Result, File, States);
    for (std::size_t Index = File.listElements().Begin; Index < File.listElements().End; ++Index)
    {
      if (ListElementVisits[Index - File.listElements().Begin] == 0)
      {
        addError(Result, AstVerificationErrorKind::InvalidList, {}, Index - File.listElements().Begin, "AST file contains an unowned list table entry");
      }
    }
    for (std::size_t Index = File.recoveries().Begin; Index < File.recoveries().End; ++Index)
    {
      if (RecoveryVisits[Index - File.recoveries().Begin] == 0)
      {
        addError(Result, AstVerificationErrorKind::InvalidRecovery, {}, Index - File.recoveries().Begin, "AST file contains an unowned recovery table entry");
      }
    }

    const Declaration &RootNode = Context.declaration(File.root());
    if (RootNode.Kind != AstKind::SourceFile || !std::holds_alternative<SourceFilePayload>(RootNode.Payload) || RootNode.Header.Range != core::SourceRange{0, File.sourceSize()})
    {
      addError(Result, AstVerificationErrorKind::InvalidRoot, Root, 0, "AST root must be a SourceFile payload covering the complete source buffer");
    }
    return Result;
  }

  AstVerificationResult verifyAst(const AstContext &Context, const AstFile &File, const core::StringInterner &Strings)
  {
    return AstVerifier().verify(Context, File, Strings);
  }
} // namespace ink::ast
