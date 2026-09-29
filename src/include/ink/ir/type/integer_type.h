#ifndef INK_IR_INTEGER_TYPE_H
#define INK_IR_INTEGER_TYPE_H

#include "ink/ir/type/builtin_type.h"

namespace ink::ir
{
  class IntegerType final : public BuiltinType
  {
    public:
      std::uint32_t bitWidth() const noexcept
      {
        return BitWidth;
      }

      bool isSigned() const noexcept
      {
        return Signed;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::IntegerType;
      }

    private:
      IntegerType(IRContext &Context, const Type &MetaType, std::uint32_t BitWidth, bool Signed) noexcept
          : BuiltinType(Context, ValueKind::IntegerType, TypeKind::Integer, &MetaType),
            BitWidth(BitWidth),
            Signed(Signed)
      {
      }

      std::uint32_t BitWidth;
      bool Signed;

      friend class TypePool;
  };
} // namespace ink::ir

#endif
