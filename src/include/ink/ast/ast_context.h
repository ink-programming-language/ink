#ifndef INK_AST_AST_CONTEXT_H
#define INK_AST_AST_CONTEXT_H

#include "ink/ast/ast.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace ink::ast
{
  struct AstNodeView
  {
    AstNodeRef Ref;
    AstKind Kind = AstKind::Unknown;
    const AstNodeHeader *Header = nullptr;
  };

  class AstNodeListView
  {
  public:
    using ConstIterator = const AstNodeRef *;

    constexpr AstNodeListView() noexcept = default;
    constexpr AstNodeListView(const AstNodeRef *Data, std::size_t Size) noexcept : Data(Data), Size(Size)
    {
    }

    constexpr const AstNodeRef *data() const noexcept
    {
      return Data;
    }

    constexpr std::size_t size() const noexcept
    {
      return Size;
    }

    constexpr bool empty() const noexcept
    {
      return Size == 0;
    }

    const AstNodeRef &operator[](std::size_t Index) const;

    constexpr ConstIterator begin() const noexcept
    {
      return Data;
    }

    constexpr ConstIterator end() const noexcept
    {
      return Size == 0 ? Data : Data + Size;
    }

  private:
    const AstNodeRef *Data = nullptr;
    std::size_t Size = 0;
  };

  class AstRecoveryView
  {
  public:
    using ConstIterator = const AstRecovery *;

    constexpr AstRecoveryView() noexcept = default;
    constexpr AstRecoveryView(const AstRecovery *Data, std::size_t Size) noexcept : Data(Data), Size(Size)
    {
    }

    constexpr const AstRecovery *data() const noexcept
    {
      return Data;
    }

    constexpr std::size_t size() const noexcept
    {
      return Size;
    }

    constexpr bool empty() const noexcept
    {
      return Size == 0;
    }

    const AstRecovery &operator[](std::size_t Index) const;

    constexpr ConstIterator begin() const noexcept
    {
      return Data;
    }

    constexpr ConstIterator end() const noexcept
    {
      return Size == 0 ? Data : Data + Size;
    }

  private:
    const AstRecovery *Data = nullptr;
    std::size_t Size = 0;
  };

  class AstContext
  {
  public:
    AstContext() = default;
    AstContext(const AstContext &) = delete;
    AstContext &operator=(const AstContext &) = delete;
    AstContext(AstContext &&) = delete;
    AstContext &operator=(AstContext &&) = delete;

    AstNodeList addList(std::vector<AstNodeRef> Nodes);
    AstRecoveryRange addRecoveries(std::vector<AstRecovery> Recoveries);
    AstDeclId addDeclaration(Declaration Node);
    AstExprId addExpression(Expression Node);
    AstStmtId addStatement(Statement Node);
    AstPatternId addPattern(Pattern Node);
    AstFile createFile(core::SourceFileId SourceFile, AstDeclId Root, std::size_t SourceSize, const core::StringInterner &Strings, std::vector<std::uint32_t> CstChildCounts);

    const Declaration &declaration(AstDeclId Id) const;
    const Expression &expression(AstExprId Id) const;
    const Statement &statement(AstStmtId Id) const;
    const Pattern &pattern(AstPatternId Id) const;
    AstNodeView node(AstNodeRef Ref) const;
    AstNodeListView list(AstNodeList Nodes) const;
    AstRecoveryView recoveries(AstRecoveryRange Range) const;

    bool contains(AstDeclId Id) const noexcept;
    bool contains(AstExprId Id) const noexcept;
    bool contains(AstStmtId Id) const noexcept;
    bool contains(AstPatternId Id) const noexcept;
    bool contains(AstNodeRef Ref) const noexcept;
    bool contains(AstNodeList Nodes) const noexcept;
    bool contains(AstRecoveryRange Range) const noexcept;
    bool contains(const AstFile &File) const noexcept;

    const std::vector<Declaration> &declarations() const noexcept;
    const std::vector<Expression> &expressions() const noexcept;
    const std::vector<Statement> &statements() const noexcept;
    const std::vector<Pattern> &patterns() const noexcept;
    const std::vector<AstNodeRef> &listElements() const noexcept;
    const std::vector<AstRecovery> &allRecoveries() const noexcept;

  private:
    std::vector<Declaration> Declarations;
    std::vector<Expression> Expressions;
    std::vector<Statement> Statements;
    std::vector<Pattern> Patterns;
    std::vector<AstNodeRef> ListElements;
    std::vector<AstRecovery> Recoveries;
    std::vector<AstFile> Files;
    std::size_t RegisteredDeclarationCount = 0;
    std::size_t RegisteredExpressionCount = 0;
    std::size_t RegisteredStatementCount = 0;
    std::size_t RegisteredPatternCount = 0;
    std::size_t RegisteredListElementCount = 0;
    std::size_t RegisteredRecoveryCount = 0;
  };
} // namespace ink::ast

#endif
