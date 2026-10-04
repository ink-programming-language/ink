#include "ink/ir/analysis/type_layout.h"
#include "ink/core/object_layout.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/class_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/integer_type.h"

#include <algorithm>
#include <unordered_map>

namespace ink::ir
{
  namespace
  {
    bool alignSize(std::uint64_t &Size, std::uint64_t Alignment, std::uint64_t Limit)
    {
      const std::uint64_t Padding = (Alignment - Size % Alignment) % Alignment;
      if (Size > Limit || Padding > Limit - Size)
      {
        return false;
      }
      Size += Padding;
      return true;
    }

    std::optional<TypeLayout> layout(const Type &ValueType, const core::TargetContext &Target, std::vector<const Type *> &Active, std::unordered_map<const Type *, TypeLayout> &Cache)
    {
      if (const auto Found = Cache.find(&ValueType); Found != Cache.end())
      {
        return Found->second;
      }
      if (Active.size() >= 256 || std::find(Active.begin(), Active.end(), &ValueType) != Active.end())
      {
        return std::nullopt;
      }
      const std::uint64_t PointerBytes = Target.pointerByteWidth();
      const std::uint64_t Limit = Target.maximumPointerSizeValue();
      TypeLayout Result;
      switch (ValueType.typeKind())
      {
      case TypeKind::Bool:
        Result.Size = 1;
        break;
      case TypeKind::Integer:
        Result.Size = (static_cast<std::uint64_t>(static_cast<const IntegerType &>(ValueType).bitWidth()) + 7) / 8;
        Result.Alignment = Target.integerAlignment(static_cast<const IntegerType &>(ValueType).bitWidth());
        break;
      case TypeKind::Float:
        Result.Size = static_cast<const FloatType &>(ValueType).bitWidth() / 8;
        Result.Alignment = Target.floatAlignment(static_cast<const FloatType &>(ValueType).bitWidth());
        break;
      case TypeKind::Pointer:
      case TypeKind::Reference:
      case TypeKind::Function:
        Result.Size = PointerBytes;
        Result.Alignment = PointerBytes;
        break;
      case TypeKind::Slice:
        Result.Size = PointerBytes * 2;
        Result.Alignment = PointerBytes;
        break;
      case TypeKind::Array:
      {
        const auto &Array = static_cast<const ArrayType &>(ValueType);
        Active.push_back(&ValueType);
        const auto Element = layout(Array.elementType(), Target, Active, Cache);
        Active.pop_back();
        if (!Element || (Element->Stride && Array.elementCount() > Limit / Element->Stride))
        {
          return std::nullopt;
        }
        Result.Size = Element->Stride * Array.elementCount();
        Result.Alignment = Element->Alignment;
        Result.Stride = Result.Size;
        Cache.emplace(&ValueType, Result);
        return Result;
      }
      case TypeKind::Class:
      {
        const auto &Class = static_cast<const ClassType &>(ValueType);
        if (!Class.isComplete())
        {
          return std::nullopt;
        }
        Active.push_back(&ValueType);
        core::ObjectLayoutBuilder Object(Limit);
        for (const ClassField &Field : Class.fields())
        {
          const auto FieldLayout = layout(*Field.FieldType, Target, Active, Cache);
          const auto Offset = FieldLayout ? Object.append(FieldLayout->Size, FieldLayout->Alignment) : std::nullopt;
          if (!Offset)
          {
            Active.pop_back();
            return std::nullopt;
          }
          Result.FieldOffsets.push_back(*Offset);
        }
        Active.pop_back();
        const auto Size = Object.size();
        if (!Size)
        {
          return std::nullopt;
        }
        Result.Size = *Size;
        Result.Alignment = Object.alignment();
        Result.Stride = Result.Size;
        Cache.emplace(&ValueType, Result);
        return Result;
      }
      default:
        return std::nullopt;
      }
      if (!alignSize(Result.Size, Result.Alignment, Limit))
      {
        return std::nullopt;
      }
      Result.Stride = Result.Size;
      Cache.emplace(&ValueType, Result);
      return Result;
    }
  } // namespace

  std::optional<TypeLayout> computeTypeLayout(const Type &ValueType, const core::TargetContext &Target)
  {
    std::vector<const Type *> Active;
    std::unordered_map<const Type *, TypeLayout> Cache;
    return layout(ValueType, Target, Active, Cache);
  }
} // namespace ink::ir
