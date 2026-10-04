#ifndef INK_SEMANTIC_OPERATOR_METHOD_H
#define INK_SEMANTIC_OPERATOR_METHOD_H

#include "ink/tokenizer/token.h"

#include <string_view>

namespace ink::semantic
{
  enum class OperatorMethodKind
  {
    Unary,
    Binary,
    Reflected,
    InPlace,
  };

  struct OperatorMethodMapping
  {
      tokenizer::TokenKind Operator;
      OperatorMethodKind Kind;
      std::string_view Name;
  };

  // Unary and binary methods receive snapshots; reflected and in-place dispatch remain reserved.
  inline constexpr OperatorMethodMapping OperatorMethodMappings[] = {
    {tokenizer::TokenKind::Plus, OperatorMethodKind::Binary, "__add__"},
    {tokenizer::TokenKind::Plus, OperatorMethodKind::Reflected, "__radd__"},
    {tokenizer::TokenKind::Minus, OperatorMethodKind::Binary, "__sub__"},
    {tokenizer::TokenKind::Minus, OperatorMethodKind::Reflected, "__rsub__"},
    {tokenizer::TokenKind::Star, OperatorMethodKind::Binary, "__mul__"},
    {tokenizer::TokenKind::Star, OperatorMethodKind::Reflected, "__rmul__"},
    {tokenizer::TokenKind::Slash, OperatorMethodKind::Binary, "__truediv__"},
    {tokenizer::TokenKind::Slash, OperatorMethodKind::Reflected, "__rtruediv__"},
    {tokenizer::TokenKind::Percent, OperatorMethodKind::Binary, "__mod__"},
    {tokenizer::TokenKind::Percent, OperatorMethodKind::Reflected, "__rmod__"},
    {tokenizer::TokenKind::Amp, OperatorMethodKind::Binary, "__and__"},
    {tokenizer::TokenKind::Amp, OperatorMethodKind::Reflected, "__rand__"},
    {tokenizer::TokenKind::Pipe, OperatorMethodKind::Binary, "__or__"},
    {tokenizer::TokenKind::Pipe, OperatorMethodKind::Reflected, "__ror__"},
    {tokenizer::TokenKind::Caret, OperatorMethodKind::Binary, "__xor__"},
    {tokenizer::TokenKind::Caret, OperatorMethodKind::Reflected, "__rxor__"},
    {tokenizer::TokenKind::ShiftLeft, OperatorMethodKind::Binary, "__lshift__"},
    {tokenizer::TokenKind::ShiftLeft, OperatorMethodKind::Reflected, "__rlshift__"},
    {tokenizer::TokenKind::ShiftRight, OperatorMethodKind::Binary, "__rshift__"},
    {tokenizer::TokenKind::ShiftRight, OperatorMethodKind::Reflected, "__rrshift__"},
    {tokenizer::TokenKind::EqualEqual, OperatorMethodKind::Binary, "__eq__"},
    {tokenizer::TokenKind::BangEqual, OperatorMethodKind::Binary, "__ne__"},
    {tokenizer::TokenKind::Less, OperatorMethodKind::Binary, "__lt__"},
    {tokenizer::TokenKind::LessEqual, OperatorMethodKind::Binary, "__le__"},
    {tokenizer::TokenKind::Greater, OperatorMethodKind::Binary, "__gt__"},
    {tokenizer::TokenKind::GreaterEqual, OperatorMethodKind::Binary, "__ge__"},
    {tokenizer::TokenKind::Plus, OperatorMethodKind::Unary, "__pos__"},
    {tokenizer::TokenKind::Minus, OperatorMethodKind::Unary, "__neg__"},
    {tokenizer::TokenKind::Tilde, OperatorMethodKind::Unary, "__invert__"},
    {tokenizer::TokenKind::PlusAssign, OperatorMethodKind::InPlace, "__iadd__"},
    {tokenizer::TokenKind::MinusAssign, OperatorMethodKind::InPlace, "__isub__"},
    {tokenizer::TokenKind::StarAssign, OperatorMethodKind::InPlace, "__imul__"},
    {tokenizer::TokenKind::SlashAssign, OperatorMethodKind::InPlace, "__itruediv__"},
    {tokenizer::TokenKind::PercentAssign, OperatorMethodKind::InPlace, "__imod__"},
    {tokenizer::TokenKind::AmpAssign, OperatorMethodKind::InPlace, "__iand__"},
    {tokenizer::TokenKind::PipeAssign, OperatorMethodKind::InPlace, "__ior__"},
    {tokenizer::TokenKind::CaretAssign, OperatorMethodKind::InPlace, "__ixor__"},
    {tokenizer::TokenKind::ShiftLeftAssign, OperatorMethodKind::InPlace, "__ilshift__"},
    {tokenizer::TokenKind::ShiftRightAssign, OperatorMethodKind::InPlace, "__irshift__"},
  };

  constexpr std::string_view operatorMethodName(tokenizer::TokenKind Operator, OperatorMethodKind Kind) noexcept
  {
    for (const auto &Mapping : OperatorMethodMappings)
    {
      if (Mapping.Operator == Operator && Mapping.Kind == Kind)
      {
        return Mapping.Name;
      }
    }
    return {};
  }

  constexpr bool operatorMethodEnabled(tokenizer::TokenKind Operator, OperatorMethodKind Kind) noexcept
  {
    return (Kind == OperatorMethodKind::Unary || Kind == OperatorMethodKind::Binary) && !operatorMethodName(Operator, Kind).empty();
  }

  enum class SpecialMethodKind
  {
    GetItem,
    SetItem,
    Call,
    ToBool,
    Initialize,
    Destroy,
  };

  struct SpecialMethodMapping
  {
      SpecialMethodKind Kind;
      std::string_view Name;
      bool Enabled;
  };

  // Names are reserved protocol hooks; ordinary direct calls to methods with these names remain ordinary calls.
  inline constexpr SpecialMethodMapping SpecialMethodMappings[] = {
    {SpecialMethodKind::GetItem, "__getitem__", false},
    {SpecialMethodKind::SetItem, "__setitem__", false},
    {SpecialMethodKind::Call, "__call__", false},
    {SpecialMethodKind::ToBool, "__bool__", false},
    {SpecialMethodKind::Initialize, "__init__", false},
    {SpecialMethodKind::Destroy, "__del__", false},
  };

  constexpr std::string_view specialMethodName(SpecialMethodKind Kind) noexcept
  {
    for (const auto &Mapping : SpecialMethodMappings)
    {
      if (Mapping.Kind == Kind)
      {
        return Mapping.Name;
      }
    }
    return {};
  }

  constexpr bool specialMethodEnabled(SpecialMethodKind Kind) noexcept
  {
    for (const auto &Mapping : SpecialMethodMappings)
    {
      if (Mapping.Kind == Kind)
      {
        return Mapping.Enabled;
      }
    }
    return false;
  }
} // namespace ink::semantic

#endif
