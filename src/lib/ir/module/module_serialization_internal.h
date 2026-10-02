#ifndef INK_IR_MODULE_SERIALIZATION_INTERNAL_H
#define INK_IR_MODULE_SERIALIZATION_INTERNAL_H

#include "ink/ir/module/module_serialization.h"
#include "ink/ir/instruction/compare_instruction.h"

#include <limits>
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace ink::ir::archive
{
  inline constexpr std::size_t Unlimited = std::numeric_limits<std::size_t>::max();

  enum class Tag : unsigned
  {
#define IR_ARCHIVE_TAG(Name, Id, Text, Min, Max, HasText) Name = Id,
#include "module_serialization_tags.def"
#undef IR_ARCHIVE_TAG
  };

  struct TagInfo
  {
      Tag Kind;
      std::string_view Text;
      std::size_t MinFields;
      std::size_t MaxFields;
      bool HasText;
  };

  inline constexpr TagInfo Tags[] = {
#define IR_ARCHIVE_TAG(Name, Id, Text, Min, Max, HasText) {Tag::Name, Text, Min, Max, HasText},
#include "module_serialization_tags.def"
#undef IR_ARCHIVE_TAG
  };

  inline bool isInstruction(Tag Kind)
  {
    return (Kind >= Tag::Call && Kind <= Tag::Return) || (Kind >= Tag::Branch && Kind <= Tag::Compare);
  }

  struct ComparisonPredicateInfo
  {
      ComparisonPredicate Predicate;
      std::uint64_t Wire;
      std::string_view Text;
  };

  // Wire values are explicit so changes to the public enum cannot change existing archives.
  inline constexpr ComparisonPredicateInfo ComparisonPredicates[] = {
      {ComparisonPredicate::Equal, 0, "eq"},
      {ComparisonPredicate::NotEqual, 1, "ne"},
      {ComparisonPredicate::Less, 2, "lt"},
      {ComparisonPredicate::LessEqual, 3, "le"},
      {ComparisonPredicate::Greater, 4, "gt"},
      {ComparisonPredicate::GreaterEqual, 5, "ge"},
  };

  inline const ComparisonPredicateInfo *comparisonPredicateInfo(ComparisonPredicate Predicate)
  {
    for (const auto &Info : ComparisonPredicates)
    {
      if (Info.Predicate == Predicate)
      {
        return &Info;
      }
    }
    return nullptr;
  }

  inline const ComparisonPredicateInfo *comparisonPredicateInfo(std::uint64_t Wire)
  {
    for (const auto &Info : ComparisonPredicates)
    {
      if (Info.Wire == Wire)
      {
        return &Info;
      }
    }
    return nullptr;
  }

  inline const ComparisonPredicateInfo *comparisonPredicateInfo(std::string_view Text)
  {
    for (const auto &Info : ComparisonPredicates)
    {
      if (Info.Text == Text)
      {
        return &Info;
      }
    }
    return nullptr;
  }

  inline const TagInfo *tagInfo(unsigned Kind)
  {
    for (const auto &Info : Tags)
    {
      if (static_cast<unsigned>(Info.Kind) == Kind)
      {
        return &Info;
      }
    }
    return nullptr;
  }

  struct Record
  {
      Tag Kind = Tag::Meta;
      std::uint64_t Type = 0;
      std::uint64_t Parent = 0;
      std::vector<std::uint64_t> Fields;
      std::string Text;
  };

  class State
  {
    public:
      explicit State(ModuleArchiveLimits Limits)
          : Limits(Limits)
      {
      }

      bool good() const noexcept
      {
        return Status == ModuleArchiveStatus::Success;
      }

      parser::ASTArchiveLimits astLimits() const
      {
        auto Result = Limits.AST;
        Result.MaxAllocationBytes = std::min(Result.MaxAllocationBytes, Limits.MaxAllocationBytes - Allocated);
        Result.MaxArchiveBytes = std::min(Result.MaxArchiveBytes, Limits.MaxStringBytes);
        return Result;
      }

      bool fail(std::string_view Reason, ModuleArchiveStatus Failure = ModuleArchiveStatus::InvalidArchive)
      {
        if (good())
        {
          Status = Failure;
          Message = Reason;
        }
        return false;
      }

      bool charge(std::size_t Count, std::size_t Size)
      {
        if (!good())
        {
          return false;
        }
        if (Size && Count > (Limits.MaxAllocationBytes - Allocated) / Size)
        {
          return fail("Module archive allocation limit exceeded", ModuleArchiveStatus::LimitExceeded);
        }
        Allocated += Count * Size;
        return true;
      }

      bool addRecord()
      {
        if (Records.size() >= Limits.MaxObjects)
        {
          return fail("Module archive object limit exceeded", ModuleArchiveStatus::LimitExceeded);
        }
        // Includes records, graph bookkeeping, ownership, pool objects and container overhead.
        if (!charge(1, 1024))
        {
          return false;
        }
        Records.emplace_back();
        return true;
      }

      bool fields(std::size_t Count)
      {
        if (Count > Limits.MaxFields)
        {
          return fail("Module archive field limit exceeded", ModuleArchiveStatus::LimitExceeded);
        }
        return charge(Count, 64);
      }

      bool string(std::size_t Size)
      {
        if (Size > Limits.MaxStringBytes)
        {
          return fail("Module archive string limit exceeded", ModuleArchiveStatus::LimitExceeded);
        }
        return charge(Size, 8);
      }

      ModuleArchiveLimits Limits;
      ModuleArchiveStatus Status = ModuleArchiveStatus::Success;
      std::string Message;
      std::vector<Record> Records;
      std::vector<std::pair<std::uint64_t, std::uint64_t>> OperandTypes;
      std::unordered_map<std::uint64_t, std::uint64_t> TypeAliases;
      bool TextFormat = false;

    private:
      std::size_t Allocated = 0;
  };

  bool collect(const Module &ModuleValue, State &Data, std::span<const parser::ParseResult *const> Sources);
  Module *restore(IRContext &Context, State &Data);
  bool readText(std::string_view Text, State &Data);
  bool readBinary(std::string_view Bytes, State &Data);
  std::string writeText(State &Data);
  std::string writeBinary(State &Data);
} // namespace ink::ir::archive

#endif
