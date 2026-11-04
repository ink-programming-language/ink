#pragma once

#include "ink/ir/type/array_type.h"
#include "ink/ir/value/value.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace ink::ir
{
  // Owns decoded bytes, including embedded NULs, without adding a terminator to the IR array.
  class StringConstant final : public Value
  {
    public:
      std::size_t hash() const noexcept;
      static std::size_t hash(std::string_view Value) noexcept;
      bool operator==(const StringConstant &Other) const noexcept;
      bool operator==(std::string_view Other) const noexcept;

      StringConstant(const ArrayType &ValueType, std::string_view Bytes)
          : Value(ValueType),
            Bytes(Bytes)
      {
      }

      ~StringConstant() = default;

      std::string_view value() const noexcept
      {
        return Bytes;
      }

    private:
      std::string Bytes;
  };
} // namespace ink::ir
