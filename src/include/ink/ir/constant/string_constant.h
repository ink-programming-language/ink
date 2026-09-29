#ifndef INK_IR_CONSTANT_STRING_CONSTANT_H
#define INK_IR_CONSTANT_STRING_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/slice_type.h"

#include <string>
#include <string_view>

namespace ink::ir
{
  class StringConstant final : public Constant
  {
    public:
      // Owned decoded UTF-8 bytes, including embedded NULs; the storage terminator is not part of the value.
      std::string_view value() const noexcept
      {
        return Payload;
      }

      // Complete immutable storage for lowering: payload followed by one additional NUL, including for empty values.
      std::string_view nullTerminatedValue() const noexcept
      {
        return {Payload.c_str(), Payload.size() + 1};
      }

      // Zero-copy C string conversion; nullptr explicitly rejects embedded NULs that would truncate the value.
      const char *tryGetCString() const noexcept
      {
        return Payload.find('\0') == std::string::npos ? Payload.c_str() : nullptr;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::StringConstant;
      }

    private:
      StringConstant(const SliceType &ValueType, std::string_view Payload)
          : Constant(ValueKind::StringConstant, ValueType),
            Payload(Payload)
      {
      }

      std::string Payload;

      friend class ConstantPool;
  };
} // namespace ink::ir

#endif
