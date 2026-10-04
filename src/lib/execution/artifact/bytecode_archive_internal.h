#ifndef INK_EXECUTION_ARTIFACT_BYTECODE_ARCHIVE_INTERNAL_H
#define INK_EXECUTION_ARTIFACT_BYTECODE_ARCHIVE_INTERNAL_H

#include "ink/execution/artifact/bytecode_archive.h"

#include <array>
#include <cstdint>
#include <limits>
#include <string_view>
#include <utility>

namespace ink::execution::archive
{
  inline constexpr std::string_view Magic("INKBC\0\r\n", 8);
  inline constexpr std::uint32_t FormatVersion = 7;
  inline constexpr std::uint32_t InstructionVersion = 3;

  // File tags are independent of enum declaration order. Existing tags must never change.
  inline constexpr std::array OpcodeTags = {
      std::pair{BytecodeOpcode::Alloca, 1U},
      std::pair{BytecodeOpcode::AllocaLocal, 2U},
      std::pair{BytecodeOpcode::Load, 3U},
      std::pair{BytecodeOpcode::LoadI8, 4U},
      std::pair{BytecodeOpcode::LoadI16, 5U},
      std::pair{BytecodeOpcode::LoadI32, 6U},
      std::pair{BytecodeOpcode::LoadI64, 7U},
      std::pair{BytecodeOpcode::LoadLocal, 8U},
      std::pair{BytecodeOpcode::Store, 9U},
      std::pair{BytecodeOpcode::StoreI8, 10U},
      std::pair{BytecodeOpcode::StoreI16, 11U},
      std::pair{BytecodeOpcode::StoreI32, 12U},
      std::pair{BytecodeOpcode::StoreI64, 13U},
      std::pair{BytecodeOpcode::StoreLocal, 14U},
      std::pair{BytecodeOpcode::CString, 15U},
      std::pair{BytecodeOpcode::AddI8, 16U},
      std::pair{BytecodeOpcode::AddI16, 17U},
      std::pair{BytecodeOpcode::AddI32, 18U},
      std::pair{BytecodeOpcode::AddI64, 19U},
      std::pair{BytecodeOpcode::AddWide, 20U},
      std::pair{BytecodeOpcode::LogicalNot, 21U},
      std::pair{BytecodeOpcode::LogicalAnd, 22U},
      std::pair{BytecodeOpcode::LogicalOr, 23U},
      std::pair{BytecodeOpcode::CompareSigned8, 24U},
      std::pair{BytecodeOpcode::CompareSigned16, 25U},
      std::pair{BytecodeOpcode::CompareSigned32, 26U},
      std::pair{BytecodeOpcode::CompareSigned64, 27U},
      std::pair{BytecodeOpcode::CompareUnsigned8, 28U},
      std::pair{BytecodeOpcode::CompareUnsigned16, 29U},
      std::pair{BytecodeOpcode::CompareUnsigned32, 30U},
      std::pair{BytecodeOpcode::CompareUnsigned64, 31U},
      std::pair{BytecodeOpcode::CompareWide, 32U},
      std::pair{BytecodeOpcode::CompareBool, 33U},
      std::pair{BytecodeOpcode::CallDirect, 34U},
      std::pair{BytecodeOpcode::CallIndirect, 35U},
      std::pair{BytecodeOpcode::Jump, 36U},
      std::pair{BytecodeOpcode::JumpIf, 37U},
      std::pair{BytecodeOpcode::Return, 38U},
      std::pair{BytecodeOpcode::ReturnVoid, 39U},
      std::pair{BytecodeOpcode::Function, 40U},
      std::pair{BytecodeOpcode::Failure, 41U},
      std::pair{BytecodeOpcode::Array, 42U},
      std::pair{BytecodeOpcode::ArrayRepeat, 43U},
      std::pair{BytecodeOpcode::ArrayElementPointer, 44U},
      std::pair{BytecodeOpcode::ArrayExtract, 45U},
      std::pair{BytecodeOpcode::Class, 46U},
      std::pair{BytecodeOpcode::FieldExtract, 47U},
      std::pair{BytecodeOpcode::FieldPointer, 48U},
  };
  static_assert(OpcodeTags.size() == static_cast<std::size_t>(BytecodeOpcode::Count), "Add an explicit archive tag for every opcode");

  inline constexpr std::array StatusTags = {
      std::pair{ExecutionStatus::Success, 0U},
      std::pair{ExecutionStatus::InvalidFrame, 1U},
      std::pair{ExecutionStatus::InvalidBinding, 2U},
      std::pair{ExecutionStatus::DuplicateBinding, 3U},
      std::pair{ExecutionStatus::UnknownBinding, 4U},
      std::pair{ExecutionStatus::InvalidPlace, 5U},
      std::pair{ExecutionStatus::ExpiredPlace, 6U},
      std::pair{ExecutionStatus::Uninitialized, 7U},
      std::pair{ExecutionStatus::RuntimeValue, 8U},
      std::pair{ExecutionStatus::ReadOnly, 9U},
      std::pair{ExecutionStatus::TypeMismatch, 10U},
      std::pair{ExecutionStatus::ForeignContext, 11U},
      std::pair{ExecutionStatus::InvalidArguments, 12U},
      std::pair{ExecutionStatus::MissingBody, 13U},
      std::pair{ExecutionStatus::SymbolNotFound, 14U},
      std::pair{ExecutionStatus::UnsupportedExternalSignature, 15U},
      std::pair{ExecutionStatus::HostAbiMismatch, 16U},
      std::pair{ExecutionStatus::UnsupportedOperation, 17U},
      std::pair{ExecutionStatus::DivisionByZero, 18U},
      std::pair{ExecutionStatus::InvalidShift, 19U},
      std::pair{ExecutionStatus::Overflow, 20U},
      std::pair{ExecutionStatus::BudgetExceeded, 21U},
      std::pair{ExecutionStatus::Cancelled, 22U},
      std::pair{ExecutionStatus::IndexOutOfBounds, 23U},
  };

  inline constexpr std::array PredicateTags = {
      std::pair{core::ComparisonPredicate::Equal, 0U},
      std::pair{core::ComparisonPredicate::NotEqual, 1U},
      std::pair{core::ComparisonPredicate::Less, 2U},
      std::pair{core::ComparisonPredicate::LessEqual, 3U},
      std::pair{core::ComparisonPredicate::Greater, 4U},
      std::pair{core::ComparisonPredicate::GreaterEqual, 5U},
  };

  inline constexpr std::array RuntimeKindTags = {
      std::pair{RuntimeKind::Invalid, 0U},
      std::pair{RuntimeKind::Void, 1U},
      std::pair{RuntimeKind::Boolean, 2U},
      std::pair{RuntimeKind::Integer, 3U},
      std::pair{RuntimeKind::Float, 4U},
      std::pair{RuntimeKind::String, 5U},
      std::pair{RuntimeKind::Pointer, 6U},
      std::pair{RuntimeKind::Function, 7U},
      std::pair{RuntimeKind::Array, 8U},
      std::pair{RuntimeKind::Class, 9U},
  };

  inline constexpr std::array ArtifactKindTags = {
      std::pair{BytecodeArtifactKind::Object, 1U},
      std::pair{BytecodeArtifactKind::Executable, 2U},
  };

  inline constexpr std::array SymbolKindTags = {
      std::pair{BytecodeSymbolKind::Definition, 1U},
      std::pair{BytecodeSymbolKind::Import, 2U},
      std::pair{BytecodeSymbolKind::Native, 3U},
  };

  inline constexpr std::array VisibilityTags = {
      std::pair{BytecodeVisibility::Private, 1U},
      std::pair{BytecodeVisibility::Module, 2U},
      std::pair{BytecodeVisibility::Public, 3U},
  };

  inline constexpr std::array GenericArgumentTags = {
      std::pair{core::GenericArgumentKind::Type, 1U},
      std::pair{core::GenericArgumentKind::Value, 2U},
  };

  template <typename Enum, std::size_t Size>
  bool encodeTag(Enum Value, const std::array<std::pair<Enum, std::uint32_t>, Size> &Table, std::uint32_t &Tag) noexcept
  {
    for (const auto &[Candidate, Wire] : Table)
    {
      if (Candidate == Value)
      {
        Tag = Wire;
        return true;
      }
    }
    return false;
  }

  template <typename Enum, std::size_t Size>
  bool decodeTag(std::uint32_t Tag, const std::array<std::pair<Enum, std::uint32_t>, Size> &Table, Enum &Value) noexcept
  {
    for (const auto &[Candidate, Wire] : Table)
    {
      if (Wire == Tag)
      {
        Value = Candidate;
        return true;
      }
    }
    return false;
  }

  template <typename Enum, std::size_t Size>
  bool encodeOperand(std::uint32_t Value, const std::array<std::pair<Enum, std::uint32_t>, Size> &Table, std::uint32_t &Tag) noexcept
  {
    for (const auto &[Candidate, Wire] : Table)
    {
      if (static_cast<std::uint32_t>(Candidate) == Value)
      {
        Tag = Wire;
        return true;
      }
    }
    return false;
  }

  template <typename Enum, std::size_t Size>
  bool decodeOperand(std::uint32_t Tag, const std::array<std::pair<Enum, std::uint32_t>, Size> &Table, std::uint32_t &Value) noexcept
  {
    Enum Decoded;
    if (!decodeTag(Tag, Table, Decoded))
    {
      return false;
    }
    Value = static_cast<std::uint32_t>(Decoded);
    return true;
  }

  class Budget
  {
    public:
      explicit Budget(BytecodeLimits Limits)
          : Limits(Limits)
      {
      }

    protected:
      bool fail(BytecodeStatus Failure, std::string_view Reason)
      {
        if (Status == BytecodeStatus::Success)
        {
          Status = Failure;
          Message = Reason;
        }
        return false;
      }

      bool allocate(std::size_t Count, std::size_t ElementSize = 1)
      {
        if (Status != BytecodeStatus::Success)
        {
          return false;
        }
        if (ElementSize != 0 && Count > (Limits.MaxAllocationBytes - Allocated) / ElementSize)
        {
          return fail(BytecodeStatus::LimitExceeded, "Bytecode allocation budget exceeded");
        }
        Allocated += Count * ElementSize;
        return true;
      }

      bool records(std::size_t Count)
      {
        if (Status != BytecodeStatus::Success)
        {
          return false;
        }
        if (Count > Limits.MaxRecords - Records)
        {
          return fail(BytecodeStatus::LimitExceeded, "Bytecode record budget exceeded");
        }
        Records += Count;
        return true;
      }

      bool stringBytes(std::size_t Count)
      {
        if (Status != BytecodeStatus::Success)
        {
          return false;
        }
        if (Count > Limits.MaxStringBytes - StringBytes)
        {
          return fail(BytecodeStatus::LimitExceeded, "Bytecode string budget exceeded");
        }
        StringBytes += Count;
        return true;
      }

      BytecodeLimits Limits;
      BytecodeStatus Status = BytecodeStatus::Success;
      std::string Message;
      std::size_t Allocated = 0;
      std::size_t Records = 0;
      std::size_t StringBytes = 0;
  };
} // namespace ink::execution::archive

#endif
