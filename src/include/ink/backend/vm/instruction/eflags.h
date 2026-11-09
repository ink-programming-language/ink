#pragma once

#include "ink/backend/vm/instruction/condition_code.h"

#include <cstdint>

namespace ink::vm
{
  using EFlags = std::uint32_t;

  enum class EFlag : EFlags
  {
    CF = EFlags{1} << 0,
    ZF = EFlags{1} << 6,
    SF = EFlags{1} << 7,
    OF = EFlags{1} << 11,
  };

  constexpr EFlags eflagMask(EFlag Flag) noexcept
  {
    return static_cast<EFlags>(Flag);
  }

  inline constexpr EFlags ArithmeticEFlagsMask = eflagMask(EFlag::CF) | eflagMask(EFlag::ZF) | eflagMask(EFlag::SF) | eflagMask(EFlag::OF);

  // Describes implicit flag dependencies without evaluating arithmetic results or branch conditions.
  constexpr EFlags conditionEFlagsMask(ConditionCode Condition) noexcept
  {
    switch (Condition)
    {
    case ConditionCode::None:
    case ConditionCode::Always:
      return 0;
    case ConditionCode::Equal:
    case ConditionCode::NotEqual:
      return eflagMask(EFlag::ZF);
    case ConditionCode::Below:
    case ConditionCode::AboveOrEqual:
      return eflagMask(EFlag::CF);
    case ConditionCode::BelowOrEqual:
    case ConditionCode::Above:
      return eflagMask(EFlag::CF) | eflagMask(EFlag::ZF);
    case ConditionCode::Less:
    case ConditionCode::GreaterOrEqual:
      return eflagMask(EFlag::SF) | eflagMask(EFlag::OF);
    case ConditionCode::LessOrEqual:
    case ConditionCode::Greater:
      return eflagMask(EFlag::ZF) | eflagMask(EFlag::SF) | eflagMask(EFlag::OF);
    case ConditionCode::Overflow:
    case ConditionCode::NotOverflow:
      return eflagMask(EFlag::OF);
    case ConditionCode::Sign:
    case ConditionCode::NotSign:
      return eflagMask(EFlag::SF);
    }
    return 0;
  }
} // namespace ink::vm
