#ifndef INK_PARSER_CST_H
#define INK_PARSER_CST_H

#include "ink/tokenizer/token.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace ink::parser
{
  using CstNodeId = std::size_t;

  enum class CstKind
  {
#define INK_CST_KIND(Name) Name,
#include "ink/parser/cst_kind.def"
#undef INK_CST_KIND
  };

  const char *cstKindName(CstKind Kind) noexcept;

  enum class CstNodeFlags : std::uint8_t
  {
    None = 0,
    HasError = 1U << 0U,
    HasMissing = 1U << 1U,
  };

  constexpr CstNodeFlags operator|(CstNodeFlags Left, CstNodeFlags Right) noexcept
  {
    return static_cast<CstNodeFlags>(static_cast<std::uint8_t>(Left) | static_cast<std::uint8_t>(Right));
  }

  constexpr CstNodeFlags &operator|=(CstNodeFlags &Left, CstNodeFlags Right) noexcept
  {
    Left = Left | Right;
    return Left;
  }

  constexpr bool hasFlag(CstNodeFlags Value, CstNodeFlags Flag) noexcept
  {
    return (static_cast<std::uint8_t>(Value) & static_cast<std::uint8_t>(Flag)) != 0;
  }

  struct CstNode
  {
      CstKind Kind = CstKind::Unknown;
      std::size_t FirstChild = 0;
      std::size_t ChildCount = 0;
      std::size_t TokenCount = 0;
      std::size_t TextLength = 0;
      CstNodeFlags Flags = CstNodeFlags::None;
  };

  inline bool operator==(const CstNode &Left, const CstNode &Right) noexcept
  {
    return Left.Kind == Right.Kind && Left.FirstChild == Right.FirstChild && Left.ChildCount == Right.ChildCount && Left.TokenCount == Right.TokenCount && Left.TextLength == Right.TextLength && Left.Flags == Right.Flags;
  }

  inline bool operator!=(const CstNode &Left, const CstNode &Right) noexcept
  {
    return !(Left == Right);
  }

  struct CstNodeRef
  {
      CstNodeId Id = 0;
  };

  inline bool operator==(CstNodeRef Left, CstNodeRef Right) noexcept
  {
    return Left.Id == Right.Id;
  }

  inline bool operator!=(CstNodeRef Left, CstNodeRef Right) noexcept
  {
    return !(Left == Right);
  }

  struct CstTokenRef
  {
      // Token offset from the beginning of the node that directly owns this element.
      std::size_t TokenOffset = 0;
  };

  inline bool operator==(CstTokenRef Left, CstTokenRef Right) noexcept
  {
    return Left.TokenOffset == Right.TokenOffset;
  }

  inline bool operator!=(CstTokenRef Left, CstTokenRef Right) noexcept
  {
    return !(Left == Right);
  }

  struct MissingToken
  {
      tokenizer::TokenKind ExpectedKind = tokenizer::TokenKind::InvalidCharacter;
      std::string ExpectedSpelling;
      std::size_t AnchorByteOffset = 0;
  };

  inline bool operator==(const MissingToken &Left, const MissingToken &Right)
  {
    return Left.ExpectedKind == Right.ExpectedKind && Left.ExpectedSpelling == Right.ExpectedSpelling && Left.AnchorByteOffset == Right.AnchorByteOffset;
  }

  inline bool operator!=(const MissingToken &Left, const MissingToken &Right)
  {
    return !(Left == Right);
  }

  using CstElement = std::variant<CstNodeRef, CstTokenRef, MissingToken>;

  class CstTree
  {
    public:
      const std::vector<CstNode> &nodes() const noexcept
      {
        return Nodes;
      }

      const std::vector<CstElement> &children() const noexcept
      {
        return Children;
      }

      CstNodeId root() const noexcept
      {
        return Root;
      }

      const CstNode &node(CstNodeId Id) const noexcept;

    private:
      std::vector<CstNode> Nodes;
      std::vector<CstElement> Children;
      CstNodeId Root = 0;

      friend class ParserImpl;
      friend class CstBuilder;
  };
} // namespace ink::parser

#endif
