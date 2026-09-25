#ifndef INK_SEMANTIC_MODEL_CONSTANT_STRING_CONSTANT_H
#define INK_SEMANTIC_MODEL_CONSTANT_STRING_CONSTANT_H

#include "ink/semantic/model/constant/constant.h"
#include "ink/semantic/model/type/slice_type.h"

#include <string>
#include <string_view>

namespace ink::semantic
{
  class StringConstant final : public Constant
  {
    public:
      // Owned decoded UTF-8 bytes, including embedded NULs; no terminator is added to the value.
      std::string_view value() const noexcept
      {
        return Payload;
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
} // namespace ink::semantic

#endif
