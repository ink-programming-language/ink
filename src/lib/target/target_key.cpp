#include "ink/target/target_key.h"

#include <string>

namespace ink::target
{
  namespace
  {
    void appendStringField(std::string &Result, const char *Name, const std::string &Value)
    {
      Result += Name;
      Result += '=';
      Result += std::to_string(Value.size());
      Result += ':';
      Result += Value;
      Result += ';';
    }
  } // namespace

  bool TargetKey::isValid() const noexcept
  {
    return !Triple.empty() && (PointerBitWidth == 32 || PointerBitWidth == 64) && (Endianness == TargetEndianness::Little || Endianness == TargetEndianness::Big);
  }

  std::string TargetKey::canonicalString() const
  {
    std::string Result;
    appendStringField(Result, "triple", Triple);
    appendStringField(Result, "cpu", Cpu);
    appendStringField(Result, "features", Features);
    Result += "pointer=" + std::to_string(PointerBitWidth) + ";endianness=" + targetEndiannessName(Endianness) + ";";
    return Result;
  }

  bool operator==(const TargetKey &Left, const TargetKey &Right) noexcept
  {
    return Left.Triple == Right.Triple && Left.Cpu == Right.Cpu && Left.Features == Right.Features && Left.PointerBitWidth == Right.PointerBitWidth && Left.Endianness == Right.Endianness;
  }

  bool operator!=(const TargetKey &Left, const TargetKey &Right) noexcept
  {
    return !(Left == Right);
  }

  const char *targetEndiannessName(TargetEndianness Endianness) noexcept
  {
    switch (Endianness)
    {
    case TargetEndianness::Unknown:
      return "unknown";
    case TargetEndianness::Little:
      return "little";
    case TargetEndianness::Big:
      return "big";
    }
    return "unknown";
  }
} // namespace ink::target
