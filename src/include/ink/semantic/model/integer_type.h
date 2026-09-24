#ifndef INK_SEMANTIC_MODEL_INTEGER_TYPE_H
#define INK_SEMANTIC_MODEL_INTEGER_TYPE_H

#include "ink/semantic/model/builtin_type.h"

namespace ink::semantic
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
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Integer;
      }

    private:
      IntegerType(SemanticContext &Context, const Type &MetaType, std::uint32_t BitWidth, bool Signed) noexcept
          : BuiltinType(Context, TypeKind::Integer, &MetaType),
            BitWidth(BitWidth),
            Signed(Signed)
      {
      }

      std::uint32_t BitWidth;
      bool Signed;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
