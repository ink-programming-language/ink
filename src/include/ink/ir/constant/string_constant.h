#ifndef INK_IR_CONSTANT_STRING_CONSTANT_H
#define INK_IR_CONSTANT_STRING_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/string_type.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace ink::ir
{
  class ConstantContext;

  // Owns the decoded UTF-8 bytes; the borrowed type must outlive this constant.
  // Empty strings and embedded NUL are valid. No escapes, normalization or terminator are added.
  class StringConstant final : public Constant
  {
    public:
      // The returned view remains valid for this constant's lifetime.
      std::string_view value() const noexcept;
      std::size_t byteLength() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class ConstantContext;

      StringConstant(const StringType *StringTypeValue, std::string Value) noexcept;

      std::string StoredValue;
  };
} // namespace ink::ir

#endif
