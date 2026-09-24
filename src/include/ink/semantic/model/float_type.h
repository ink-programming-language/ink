#ifndef INK_SEMANTIC_MODEL_FLOAT_TYPE_H
#define INK_SEMANTIC_MODEL_FLOAT_TYPE_H

#include "ink/semantic/model/builtin_type.h"

namespace ink::semantic
{
  // IEEE binary16, binary32 or binary64; never a host C++ floating-point type.
  class FloatType final : public BuiltinType
  {
    public:
      std::uint32_t bitWidth() const noexcept
      {
        return BitWidth;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return Type::classof(ValueObject) && static_cast<const Type *>(ValueObject)->typeKind() == TypeKind::Float;
      }

    private:
      FloatType(SemanticContext &Context, const Type &MetaType, std::uint32_t BitWidth) noexcept
          : BuiltinType(Context, TypeKind::Float, &MetaType),
            BitWidth(BitWidth)
      {
      }

      std::uint32_t BitWidth;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
