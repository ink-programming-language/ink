#pragma once

#include "ink/core/source_range.h"
#include "ink/frontend/tokenizer/token.h"
#include <cstddef>
#include <string_view>

namespace ink::frontend::parser
{
  using core::SourceRange;
  using tokenizer::TokenKind;
  using TokenId = std::size_t;
  inline constexpr TokenId InvalidTokenId = static_cast<TokenId>(-1);
  struct NameToken
  {
      TokenId Id = InvalidTokenId;
      std::string_view Text;
      SourceRange Range;
  };
  struct RestBinding
  {
      NameToken Name;
      bool Wildcard;
      SourceRange EllipsisRange;
  };
  enum class ArgumentKind
  {
    Named,
    Positional,
    SpreadPositional
  };
  enum class AttributeSuffix
  {
    None,
    Arguments,
    Value
  };
  enum class VarDeclForm
  {
    Uninitialized,
    Initialized
  };
  enum class FieldTailKind
  {
    None,
    Typed,
    InitializerOnly,
    Payload
  };
  enum class FunctionBodyKind
  {
    DeclarationOnly,
    Definition
  };
  enum class AggregateForm
  {
    Forward,
    Definition
  };
  enum class DeclarationVisibility
  {
    Default,
    Public,
    Private,
  };
} // namespace ink::frontend::parser
