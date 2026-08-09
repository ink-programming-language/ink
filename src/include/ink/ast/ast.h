#ifndef INK_AST_AST_H
#define INK_AST_AST_H

#include "ink/core/source_file_id.h"
#include "ink/core/source_range.h"
#include "ink/core/string_interner.h"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <variant>
#include <vector>

namespace ink::ast
{
  enum class AstNodeCategory : std::uint8_t
  {
    Unknown,
    Declaration,
    Expression,
    Statement,
    Pattern,
  };

  enum class AstKind : std::uint8_t
  {
#define INK_AST_KIND(Name, Category) Name,
#include "ink/ast/ast_kind.def"
#undef INK_AST_KIND
  };

  const char *astKindName(AstKind Kind) noexcept;
  const char *astNodeCategoryName(AstNodeCategory Category) noexcept;
  AstNodeCategory astKindCategory(AstKind Kind) noexcept;

  template <typename Tag>
  class AstId
  {
  public:
    using ValueType = std::uint32_t;

    static constexpr ValueType InvalidValue = std::numeric_limits<ValueType>::max();

    constexpr AstId() noexcept = default;

    static constexpr AstId fromValue(ValueType Value) noexcept
    {
      return AstId(Value);
    }

    constexpr ValueType value() const noexcept
    {
      return Value;
    }

    constexpr bool isValid() const noexcept
    {
      return Value != InvalidValue;
    }

    explicit constexpr operator bool() const noexcept
    {
      return isValid();
    }

  private:
    explicit constexpr AstId(ValueType Value) noexcept : Value(Value)
    {
    }

    ValueType Value = InvalidValue;
  };

  template <typename Tag>
  constexpr bool operator==(AstId<Tag> Left, AstId<Tag> Right) noexcept
  {
    return Left.value() == Right.value();
  }

  template <typename Tag>
  constexpr bool operator!=(AstId<Tag> Left, AstId<Tag> Right) noexcept
  {
    return !(Left == Right);
  }

  template <typename Tag>
  constexpr bool operator<(AstId<Tag> Left, AstId<Tag> Right) noexcept
  {
    return Left.value() < Right.value();
  }

  using AstDeclId = AstId<struct AstDeclIdTag>;
  using AstExprId = AstId<struct AstExprIdTag>;
  using AstStmtId = AstId<struct AstStmtIdTag>;
  using AstPatternId = AstId<struct AstPatternIdTag>;

  struct AstNodeRef
  {
    AstNodeCategory Category = AstNodeCategory::Unknown;
    std::uint32_t Index = std::numeric_limits<std::uint32_t>::max();

    static constexpr AstNodeRef declaration(AstDeclId Id) noexcept
    {
      return {AstNodeCategory::Declaration, Id.value()};
    }

    static constexpr AstNodeRef expression(AstExprId Id) noexcept
    {
      return {AstNodeCategory::Expression, Id.value()};
    }

    static constexpr AstNodeRef statement(AstStmtId Id) noexcept
    {
      return {AstNodeCategory::Statement, Id.value()};
    }

    static constexpr AstNodeRef pattern(AstPatternId Id) noexcept
    {
      return {AstNodeCategory::Pattern, Id.value()};
    }

    constexpr bool isValid() const noexcept
    {
      return Category != AstNodeCategory::Unknown && Index != std::numeric_limits<std::uint32_t>::max();
    }
  };

  constexpr bool operator==(AstNodeRef Left, AstNodeRef Right) noexcept
  {
    return Left.Category == Right.Category && Left.Index == Right.Index;
  }

  constexpr bool operator!=(AstNodeRef Left, AstNodeRef Right) noexcept
  {
    return !(Left == Right);
  }

  class CstOrigin
  {
  public:
    using ValueType = std::uint32_t;

    static constexpr ValueType InvalidValue = std::numeric_limits<ValueType>::max();

    constexpr CstOrigin() noexcept = default;

    static constexpr CstOrigin node(ValueType Value) noexcept
    {
      return CstOrigin(Value, InvalidValue);
    }

    static constexpr CstOrigin element(ValueType Node, ValueType Element) noexcept
    {
      return CstOrigin(Node, Element);
    }

    constexpr ValueType node() const noexcept
    {
      return Node;
    }

    constexpr bool hasElement() const noexcept
    {
      return Element != InvalidValue;
    }

    constexpr ValueType element() const noexcept
    {
      return Element;
    }

    constexpr bool isValid() const noexcept
    {
      return Node != InvalidValue;
    }

  private:
    explicit constexpr CstOrigin(ValueType Node, ValueType Element) noexcept : Node(Node), Element(Element)
    {
    }

    ValueType Node = InvalidValue;
    ValueType Element = InvalidValue;
  };

  constexpr bool operator==(CstOrigin Left, CstOrigin Right) noexcept
  {
    return Left.node() == Right.node() && Left.element() == Right.element();
  }

  constexpr bool operator!=(CstOrigin Left, CstOrigin Right) noexcept
  {
    return !(Left == Right);
  }

  struct AstNodeList
  {
    std::size_t First = 0;
    std::size_t Count = 0;

    constexpr bool empty() const noexcept
    {
      return Count == 0;
    }
  };

  struct AstRecoveryRange
  {
    std::size_t First = 0;
    std::size_t Count = 0;

    constexpr bool empty() const noexcept
    {
      return Count == 0;
    }
  };

  enum class AstRecoveryKind : std::uint8_t
  {
    MissingToken,
    UnexpectedSyntax,
  };

  enum class AstExpectedKind : std::uint8_t
  {
    Unknown,
    Identifier,
    Keyword,
    BuiltinType,
    Literal,
    Symbol,
    EndOfFile,
  };

  const char *astRecoveryKindName(AstRecoveryKind Kind) noexcept;
  const char *astExpectedKindName(AstExpectedKind Kind) noexcept;

  struct AstRecovery
  {
    AstRecoveryKind Kind = AstRecoveryKind::UnexpectedSyntax;
    core::SourceRange Range;
    CstOrigin Origin;
    AstExpectedKind ExpectedKind = AstExpectedKind::Unknown;
    core::InternedStringId Spelling;
  };

  struct AstNodeHeader
  {
    core::SourceRange Range;
    std::optional<CstOrigin> Origin;
    AstRecoveryRange Recoveries;
    AstNodeList Supplemental;
  };

  enum class UnsupportedFeature : std::uint8_t
  {
    Unknown,
    Attribute,
    Decorator,
    DeclarationModifier,
    ConstructorInitializer,
    CallArgument,
    Import,
    Generic,
    Class,
    Interface,
    Enum,
    Comptime,
    Match,
    For,
    Defer,
    Throw,
    Try,
    Aggregate,
    Array,
    Index,
    Slice,
    MemberAccess,
    PointerMemberAccess,
    TypeConstructor,
    ComplexType,
    Tuple,
  };

  const char *unsupportedFeatureName(UnsupportedFeature Feature) noexcept;

  enum class AstBindingMode : std::uint8_t
  {
    Unknown,
    Let,
    Var,
    Const,
  };

  enum class AstParameterFlavor : std::uint8_t
  {
    Function,
    Generic,
  };

  enum class AstFunctionFlavor : std::uint8_t
  {
    Function,
    Decorator,
    Destructor,
  };

  enum class AstLiteralKind : std::uint8_t
  {
    Bool,
    Null,
    Integer,
    Float,
    Scalar,
    String,
  };

  const char *astBindingModeName(AstBindingMode Mode) noexcept;
  const char *astParameterFlavorName(AstParameterFlavor Flavor) noexcept;
  const char *astFunctionFlavorName(AstFunctionFlavor Flavor) noexcept;
  const char *astLiteralKindName(AstLiteralKind Kind) noexcept;

  struct SourceFilePayload
  {
    AstNodeList Items;
  };

  struct ErrorPayload
  {
    AstNodeList Recovered;
  };

  struct ImportPayload
  {
    core::InternedStringId Path;
    std::optional<core::InternedStringId> Alias;
  };

  struct BindingPayload
  {
    AstBindingMode Mode = AstBindingMode::Unknown;
    bool TopLevel = false;
    AstPatternId Pattern;
    std::optional<AstExprId> Type;
    std::optional<AstExprId> Initializer;
  };

  struct ParameterPayload
  {
    AstParameterFlavor Flavor = AstParameterFlavor::Function;
    core::InternedStringId Name;
    AstExprId Type;
    std::optional<AstExprId> DefaultValue;
    bool IsPack = false;
  };

  struct FunctionPayload
  {
    AstFunctionFlavor Flavor = AstFunctionFlavor::Function;
    core::InternedStringId Name;
    AstNodeList Parameters;
    std::optional<AstExprId> ResultType;
    std::optional<AstStmtId> Body;
  };

  struct UnsupportedPayload
  {
    UnsupportedFeature Feature = UnsupportedFeature::Unknown;
    AstNodeList Children;
  };

  using DeclarationPayload = std::variant<SourceFilePayload, ErrorPayload, ImportPayload, BindingPayload, ParameterPayload, FunctionPayload, UnsupportedPayload>;

  struct Declaration
  {
    AstNodeHeader Header;
    AstKind Kind = AstKind::Unknown;
    DeclarationPayload Payload = ErrorPayload{};
  };

  struct LiteralPayload
  {
    AstLiteralKind Kind = AstLiteralKind::Integer;
    core::InternedStringId Spelling;
  };

  struct NamePayload
  {
    core::InternedStringId Name;
  };

  struct ThisPayload
  {
  };

  struct GroupPayload
  {
    AstExprId Value;
  };

  struct UnaryPayload
  {
    core::InternedStringId Operator;
    AstExprId Operand;
  };

  struct BinaryPayload
  {
    AstExprId Left;
    core::InternedStringId Operator;
    AstExprId Right;
  };

  struct CallPayload
  {
    AstExprId Callee;
    AstNodeList Arguments;
  };

  struct IfExpressionPayload
  {
    AstExprId Condition;
    AstExprId ThenValue;
    AstExprId ElseValue;
  };

  struct FunctionTypePayload
  {
    AstNodeList Parameters;
    std::optional<AstExprId> Result;
  };

  using ExpressionPayload = std::variant<ErrorPayload, LiteralPayload, NamePayload, ThisPayload, GroupPayload, UnaryPayload, BinaryPayload, CallPayload, IfExpressionPayload, FunctionTypePayload, UnsupportedPayload>;

  struct Expression
  {
    AstNodeHeader Header;
    AstKind Kind = AstKind::Unknown;
    ExpressionPayload Payload = ErrorPayload{};
  };

  struct BlockPayload
  {
    AstNodeList Items;
  };

  struct AssignmentPayload
  {
    AstExprId Left;
    core::InternedStringId Operator;
    AstExprId Right;
  };

  struct ExpressionStatementPayload
  {
    AstExprId Value;
  };

  struct IfStatementPayload
  {
    AstNodeRef Condition;
    AstStmtId ThenBlock;
    std::optional<AstStmtId> ElseBlock;
  };

  struct WhileStatementPayload
  {
    AstNodeRef Condition;
    AstStmtId Body;
  };

  struct ReturnPayload
  {
    std::optional<AstExprId> Value;
  };

  struct ControlStatementPayload
  {
  };

  using StatementPayload = std::variant<ErrorPayload, BlockPayload, AssignmentPayload, ExpressionStatementPayload, IfStatementPayload, WhileStatementPayload, ReturnPayload, ControlStatementPayload, UnsupportedPayload>;

  struct Statement
  {
    AstNodeHeader Header;
    AstKind Kind = AstKind::Unknown;
    StatementPayload Payload = ErrorPayload{};
  };

  struct BindingPatternPayload
  {
    core::InternedStringId Name;
  };

  struct WildcardPatternPayload
  {
  };

  struct TuplePatternPayload
  {
    AstNodeList Elements;
  };

  struct VariantPatternPayload
  {
    core::InternedStringId Name;
    AstNodeList Elements;
  };

  using PatternPayload = std::variant<ErrorPayload, BindingPatternPayload, WildcardPatternPayload, TuplePatternPayload, VariantPatternPayload, UnsupportedPayload>;

  struct Pattern
  {
    AstNodeHeader Header;
    AstKind Kind = AstKind::Unknown;
    PatternPayload Payload = ErrorPayload{};
  };

  template <typename Id>
  struct AstArenaRange
  {
    Id Begin;
    Id End;

    constexpr bool contains(Id Value) const noexcept
    {
      return Begin.isValid() && End.isValid() && Value.isValid() && Begin.value() <= Value.value() && Value.value() < End.value();
    }

    constexpr std::size_t size() const noexcept
    {
      return Begin.isValid() && End.isValid() && Begin.value() <= End.value() ? static_cast<std::size_t>(End.value() - Begin.value()) : 0;
    }
  };

  struct AstTableRange
  {
    std::size_t Begin = 0;
    std::size_t End = 0;

    constexpr bool contains(std::size_t First, std::size_t Count) const noexcept
    {
      return Begin <= End && Begin <= First && First <= End && Count <= End - First;
    }

    constexpr std::size_t size() const noexcept
    {
      return Begin <= End ? End - Begin : 0;
    }
  };

  class AstContext;
  class AstPrinter;

  class AstFile
  {
  public:
    core::SourceFileId sourceFile() const noexcept;
    AstDeclId root() const noexcept;
    AstArenaRange<AstDeclId> declarations() const noexcept;
    AstArenaRange<AstExprId> expressions() const noexcept;
    AstArenaRange<AstStmtId> statements() const noexcept;
    AstArenaRange<AstPatternId> patterns() const noexcept;
    AstTableRange listElements() const noexcept;
    AstTableRange recoveries() const noexcept;
    std::size_t sourceSize() const noexcept;
    std::size_t cstNodeCount() const noexcept;
    std::size_t cstChildCount(std::size_t Node) const;

  private:
    core::SourceFileId SourceFile;
    AstDeclId Root;
    AstArenaRange<AstDeclId> Declarations;
    AstArenaRange<AstExprId> Expressions;
    AstArenaRange<AstStmtId> Statements;
    AstArenaRange<AstPatternId> Patterns;
    AstTableRange ListElementRange;
    AstTableRange RecoveryRange;
    std::size_t SourceSize = 0;
    std::vector<std::uint32_t> CstChildCounts;
    const AstContext *Owner = nullptr;
    const core::StringInterner *Strings = nullptr;
    std::size_t Ordinal = 0;

    friend class AstContext;
    friend class AstPrinter;
    friend class AstVerifier;
  };
} // namespace ink::ast

#endif
