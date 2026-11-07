#pragma once

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/array_type.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace ink::ir
{
  // Owns decoded bytes, including embedded NULs, without adding a terminator to the IR array.
  class StringConstant final : public Constant
  {
    public:
      std::size_t hash() const noexcept;
      static std::size_t hash(std::string_view Value) noexcept;
      bool operator==(const StringConstant &Other) const noexcept;
      bool operator==(std::string_view Other) const noexcept;

      StringConstant(const ArrayType &ValueType, std::string_view Bytes)
          : Constant(ValueType, ValueKind::StringConstant),
            Bytes(Bytes)
      {
      }

      StringConstant(const ArrayType &&, std::string_view) = delete;

      ~StringConstant() = default;

      std::string_view value() const noexcept
      {
        return Bytes;
      }

    private:
      std::string Bytes;
  };
} // namespace ink::ir
