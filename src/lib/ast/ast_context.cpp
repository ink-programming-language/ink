#include "ink/ast/ast_context.h"

#include <iterator>
#include <stdexcept>
#include <utility>

namespace ink::ast
{
  namespace
  {
    template <typename Id>
    Id idFromSize(std::size_t Value)
    {
      if (Value >= Id::InvalidValue)
      {
        throw std::length_error("AST arena exceeds the typed ID range");
      }
      return Id::fromValue(static_cast<typename Id::ValueType>(Value));
    }

    template <typename Id, typename Node>
    Id appendNode(Node Value, std::vector<Node> &Arena)
    {
      if (Arena.size() >= Id::InvalidValue - 1U)
      {
        throw std::length_error("AST arena exceeds the typed ID range");
      }
      const Id Result = Id::fromValue(static_cast<typename Id::ValueType>(Arena.size()));
      Arena.push_back(std::move(Value));
      return Result;
    }

    template <typename Id>
    bool equalRange(AstArenaRange<Id> Left, AstArenaRange<Id> Right) noexcept
    {
      return Left.Begin == Right.Begin && Left.End == Right.End;
    }

    bool equalRange(AstTableRange Left, AstTableRange Right) noexcept
    {
      return Left.Begin == Right.Begin && Left.End == Right.End;
    }
  } // namespace

  const AstNodeRef &AstNodeListView::operator[](std::size_t Index) const
  {
    if (Index >= Size)
    {
      throw std::out_of_range("AST node-list index is out of range");
    }
    return Data[Index];
  }

  const AstRecovery &AstRecoveryView::operator[](std::size_t Index) const
  {
    if (Index >= Size)
    {
      throw std::out_of_range("AST recovery index is out of range");
    }
    return Data[Index];
  }

  AstNodeList AstContext::addList(std::vector<AstNodeRef> Nodes)
  {
    const AstNodeList Result{ListElements.size(), Nodes.size()};
    ListElements.insert(ListElements.end(), std::make_move_iterator(Nodes.begin()), std::make_move_iterator(Nodes.end()));
    return Result;
  }

  AstRecoveryRange AstContext::addRecoveries(std::vector<AstRecovery> Values)
  {
    const AstRecoveryRange Result{Recoveries.size(), Values.size()};
    Recoveries.insert(Recoveries.end(), std::make_move_iterator(Values.begin()), std::make_move_iterator(Values.end()));
    return Result;
  }

  AstDeclId AstContext::addDeclaration(Declaration Node)
  {
    return appendNode<AstDeclId>(std::move(Node), Declarations);
  }

  AstExprId AstContext::addExpression(Expression Node)
  {
    return appendNode<AstExprId>(std::move(Node), Expressions);
  }

  AstStmtId AstContext::addStatement(Statement Node)
  {
    return appendNode<AstStmtId>(std::move(Node), Statements);
  }

  AstPatternId AstContext::addPattern(Pattern Node)
  {
    return appendNode<AstPatternId>(std::move(Node), Patterns);
  }

  AstFile AstContext::createFile(core::SourceFileId SourceFile, AstDeclId Root, std::size_t SourceSize, const core::StringInterner &Strings, std::vector<std::uint32_t> CstChildCounts)
  {
    if (!SourceFile.isValid())
    {
      throw std::invalid_argument("AST file requires a valid SourceFileId");
    }
    if (CstChildCounts.size() > CstOrigin::InvalidValue)
    {
      throw std::length_error("CST node table exceeds the origin ID range");
    }

    AstFile Result;
    Result.SourceFile = SourceFile;
    Result.Root = Root;
    Result.Declarations = {idFromSize<AstDeclId>(RegisteredDeclarationCount), idFromSize<AstDeclId>(Declarations.size())};
    Result.Expressions = {idFromSize<AstExprId>(RegisteredExpressionCount), idFromSize<AstExprId>(Expressions.size())};
    Result.Statements = {idFromSize<AstStmtId>(RegisteredStatementCount), idFromSize<AstStmtId>(Statements.size())};
    Result.Patterns = {idFromSize<AstPatternId>(RegisteredPatternCount), idFromSize<AstPatternId>(Patterns.size())};
    Result.ListElementRange = {RegisteredListElementCount, ListElements.size()};
    Result.RecoveryRange = {RegisteredRecoveryCount, Recoveries.size()};
    if (!Result.Declarations.contains(Root))
    {
      throw std::invalid_argument("AST file root must belong to its newly appended declaration range");
    }

    Result.SourceSize = SourceSize;
    Result.CstChildCounts = std::move(CstChildCounts);
    Result.Owner = this;
    Result.Strings = &Strings;
    Result.Ordinal = Files.size();
    Files.push_back(Result);

    RegisteredDeclarationCount = Declarations.size();
    RegisteredExpressionCount = Expressions.size();
    RegisteredStatementCount = Statements.size();
    RegisteredPatternCount = Patterns.size();
    RegisteredListElementCount = ListElements.size();
    RegisteredRecoveryCount = Recoveries.size();
    return Result;
  }

  const Declaration &AstContext::declaration(AstDeclId Id) const
  {
    return Declarations.at(Id.value());
  }

  const Expression &AstContext::expression(AstExprId Id) const
  {
    return Expressions.at(Id.value());
  }

  const Statement &AstContext::statement(AstStmtId Id) const
  {
    return Statements.at(Id.value());
  }

  const Pattern &AstContext::pattern(AstPatternId Id) const
  {
    return Patterns.at(Id.value());
  }

  AstNodeView AstContext::node(AstNodeRef Ref) const
  {
    switch (Ref.Category)
    {
    case AstNodeCategory::Declaration:
    {
      const Declaration &Node = declaration(AstDeclId::fromValue(Ref.Index));
      return {Ref, Node.Kind, &Node.Header};
    }
    case AstNodeCategory::Expression:
    {
      const Expression &Node = expression(AstExprId::fromValue(Ref.Index));
      return {Ref, Node.Kind, &Node.Header};
    }
    case AstNodeCategory::Statement:
    {
      const Statement &Node = statement(AstStmtId::fromValue(Ref.Index));
      return {Ref, Node.Kind, &Node.Header};
    }
    case AstNodeCategory::Pattern:
    {
      const Pattern &Node = pattern(AstPatternId::fromValue(Ref.Index));
      return {Ref, Node.Kind, &Node.Header};
    }
    case AstNodeCategory::Unknown:
      break;
    }
    throw std::out_of_range("AST node reference has no valid category");
  }

  AstNodeListView AstContext::list(AstNodeList Nodes) const
  {
    if (!contains(Nodes))
    {
      throw std::out_of_range("AST node-list range is out of bounds");
    }
    return Nodes.Count == 0 ? AstNodeListView() : AstNodeListView(ListElements.data() + Nodes.First, Nodes.Count);
  }

  AstRecoveryView AstContext::recoveries(AstRecoveryRange Range) const
  {
    if (!contains(Range))
    {
      throw std::out_of_range("AST recovery range is out of bounds");
    }
    return Range.Count == 0 ? AstRecoveryView() : AstRecoveryView(Recoveries.data() + Range.First, Range.Count);
  }

  bool AstContext::contains(AstDeclId Id) const noexcept
  {
    return Id.isValid() && Id.value() < Declarations.size();
  }

  bool AstContext::contains(AstExprId Id) const noexcept
  {
    return Id.isValid() && Id.value() < Expressions.size();
  }

  bool AstContext::contains(AstStmtId Id) const noexcept
  {
    return Id.isValid() && Id.value() < Statements.size();
  }

  bool AstContext::contains(AstPatternId Id) const noexcept
  {
    return Id.isValid() && Id.value() < Patterns.size();
  }

  bool AstContext::contains(AstNodeRef Ref) const noexcept
  {
    switch (Ref.Category)
    {
    case AstNodeCategory::Declaration:
      return contains(AstDeclId::fromValue(Ref.Index));
    case AstNodeCategory::Expression:
      return contains(AstExprId::fromValue(Ref.Index));
    case AstNodeCategory::Statement:
      return contains(AstStmtId::fromValue(Ref.Index));
    case AstNodeCategory::Pattern:
      return contains(AstPatternId::fromValue(Ref.Index));
    case AstNodeCategory::Unknown:
      return false;
    }
    return false;
  }

  bool AstContext::contains(AstNodeList Nodes) const noexcept
  {
    return Nodes.First <= ListElements.size() && Nodes.Count <= ListElements.size() - Nodes.First;
  }

  bool AstContext::contains(AstRecoveryRange Range) const noexcept
  {
    return Range.First <= Recoveries.size() && Range.Count <= Recoveries.size() - Range.First;
  }

  bool AstContext::contains(const AstFile &File) const noexcept
  {
    if (File.Owner != this || File.Ordinal >= Files.size())
    {
      return false;
    }
    const AstFile &Registered = Files[File.Ordinal];
    if (File.SourceFile != Registered.SourceFile || File.Root != Registered.Root || !equalRange(File.Declarations, Registered.Declarations) || !equalRange(File.Expressions, Registered.Expressions) || !equalRange(File.Statements, Registered.Statements) || !equalRange(File.Patterns, Registered.Patterns) || !equalRange(File.ListElementRange, Registered.ListElementRange) || !equalRange(File.RecoveryRange, Registered.RecoveryRange) || File.SourceSize != Registered.SourceSize || File.Owner != Registered.Owner || File.Strings != Registered.Strings || File.Ordinal != Registered.Ordinal || File.CstChildCounts.size() != Registered.CstChildCounts.size())
    {
      return false;
    }
    for (std::size_t Index = 0; Index < File.CstChildCounts.size(); ++Index)
    {
      if (File.CstChildCounts[Index] != Registered.CstChildCounts[Index])
      {
        return false;
      }
    }
    return true;
  }

  const std::vector<Declaration> &AstContext::declarations() const noexcept
  {
    return Declarations;
  }

  const std::vector<Expression> &AstContext::expressions() const noexcept
  {
    return Expressions;
  }

  const std::vector<Statement> &AstContext::statements() const noexcept
  {
    return Statements;
  }

  const std::vector<Pattern> &AstContext::patterns() const noexcept
  {
    return Patterns;
  }

  const std::vector<AstNodeRef> &AstContext::listElements() const noexcept
  {
    return ListElements;
  }

  const std::vector<AstRecovery> &AstContext::allRecoveries() const noexcept
  {
    return Recoveries;
  }
} // namespace ink::ast
