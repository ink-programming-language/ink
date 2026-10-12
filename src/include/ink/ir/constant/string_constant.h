#ifndef INK_IR_CONSTANT_STRING_CONSTANT_H
#define INK_IR_CONSTANT_STRING_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/structure_type/array_type.h"

#include <cstddef>
#include <string>
#include <string_view>

namespace ink::ir
{
  class ConstantPool;

  // Owns decoded UTF-8 bytes as a compact ArrayType(u8, byteLength()) constant.
  // The borrowed array type must outlive this constant; this is not a pointer-and-length view.
  // Empty strings and embedded NUL are valid. No escapes, normalization or terminator are added.
  class StringConstant final : public Constant
  {
    public:
      // The returned view remains valid for this constant's lifetime.
      std::string_view value() const noexcept;
      std::size_t byteLength() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class ConstantPool;

      StringConstant(const ArrayType *ArrayTypeValue, std::string Value) noexcept;

      std::string StoredValue;
  };
} // namespace ink::ir

#endif
