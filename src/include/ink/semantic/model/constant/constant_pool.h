#ifndef INK_SEMANTIC_MODEL_CONSTANT_CONSTANT_POOL_H
#define INK_SEMANTIC_MODEL_CONSTANT_CONSTANT_POOL_H

#include "ink/semantic/model/constant/bool_constant.h"
#include "ink/semantic/model/constant/float_constant.h"
#include "ink/semantic/model/constant/integer_constant.h"
#include "ink/semantic/model/constant/string_constant.h"

#include <cstddef>
#include <memory>
#include <unordered_map>

namespace ink::semantic
{
  class SemanticContext;

  // Each SemanticContext owns one pool of immutable, context-local constants.
  // Canonical objects keep their addresses until the context is destroyed.
  class ConstantPool final
  {
    public:
      ~ConstantPool();
      ConstantPool(const ConstantPool &) = delete;
      ConstantPool &operator=(const ConstantPool &) = delete;
      ConstantPool(ConstantPool &&) = delete;
      ConstantPool &operator=(ConstantPool &&) = delete;

      const SemanticContext &context() const noexcept
      {
        return Context;
      }

      // Foreign types, malformed bits or mismatched widths return null without inserting.
      // Interning preserves exact typed bits; conversions belong to analysis.
      const IntegerConstant *getIntegerConstant(const IntegerType &ValueType, const IntegerBits &Payload);
      const BoolConstant &getBoolConstant(bool Payload) const noexcept;
      // Requires a local read-only u8 slice. The caller supplies validated, decoded UTF-8;
      // interning copies and compares every byte without escape decoding or normalization.
      const StringConstant *getStringConstant(const SliceType &ValueType, std::string_view Payload);
      // Requires valid bits in the local type's exact IEEE format; never converts or rounds.
      const FloatConstant *getFloatConstant(const FloatType &ValueType, const FloatBits &Payload);

      // Both bool constants are present from construction and included in size().
      std::size_t size() const noexcept;
      bool owns(const Constant &ConstantValue) const noexcept;

    private:
      // The context creates its types before constructing its only constant pool.
      explicit ConstantPool(const SemanticContext &Context);

      const SemanticContext &Context;
      std::unordered_multimap<std::size_t, std::unique_ptr<IntegerConstant>> IntegerConstants;
      std::unordered_multimap<std::size_t, std::unique_ptr<StringConstant>> StringConstants;
      std::unordered_multimap<std::size_t, std::unique_ptr<FloatConstant>> FloatConstants;
      std::unique_ptr<BoolConstant> FalseValue;
      std::unique_ptr<BoolConstant> TrueValue;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
