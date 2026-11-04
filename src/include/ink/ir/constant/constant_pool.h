#pragma once

#include "ink/core/support/hash.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/string_constant.h"
#include "ink/ir/type/type_pool.h"

#include <cstdint>
#include <functional>
#include <string_view>
#include <unordered_set>

namespace ink::ir
{
  // Owns immutable constants. Returned pointers, references, and views survive insertions.
  // Types must outlive this pool. Type arguments are canonicalized into that TypePool.
  // Inputs are parsed numeric values or decoded string bytes, not source spellings.
  // Concurrent operations require external synchronization.
  class ConstantPool final
  {
    public:
      explicit ConstantPool(TypePool &Types) noexcept;
      ConstantPool(TypePool &&) = delete;
      ~ConstantPool() = default;
      ConstantPool(const ConstantPool &) = delete;
      ConstantPool &operator=(const ConstantPool &) = delete;
      ConstantPool(ConstantPool &&) = delete;
      ConstantPool &operator=(ConstantPool &&) = delete;

      // Supports 8/16/32/64-bit types; nonzero bits above the target width are rejected.
      [[nodiscard]] const IntegerConstant *getInteger(const IntegerType &ValueType, std::uint64_t Bits);
      // Numeric entry points reject values outside the target's signed or unsigned range.
      [[nodiscard]] const IntegerConstant *getSignedInteger(const IntegerType &ValueType, std::int64_t Value);
      [[nodiscard]] const IntegerConstant *getUnsignedInteger(const IntegerType &ValueType, std::uint64_t Value);

      const FloatConstant &getFloat(float Value);
      const FloatConstant &getFloat(double Value);
      // Rejects unsupported widths and nonzero bits above bit 31 for a float32 type.
      [[nodiscard]] const FloatConstant *getFloatBits(const FloatType &ValueType, std::uint64_t Bits);

      // Strings have type array<unsigned integer8, byte count>, without a trailing NUL.
      const StringConstant &getString(std::string_view Bytes);

    private:
      TypePool &Types;
      std::unordered_set<IntegerConstant, core::Hash<IntegerConstant>, std::equal_to<>> Integers;
      std::unordered_set<FloatConstant, core::Hash<FloatConstant>, std::equal_to<>> Floats;
      std::unordered_set<StringConstant, core::Hash<StringConstant>, std::equal_to<>> Strings;
  };
} // namespace ink::ir
