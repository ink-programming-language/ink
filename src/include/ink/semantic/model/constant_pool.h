#ifndef INK_SEMANTIC_MODEL_CONSTANT_POOL_H
#define INK_SEMANTIC_MODEL_CONSTANT_POOL_H

#include "ink/semantic/model/constant.h"

#include <cstddef>
#include <memory>

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
      const StringConst *getStringConst(const SliceType &ValueType, std::string_view Payload);
      // Requires valid bits in the local type's exact IEEE format; never converts or rounds.
      const FloatConst *getFloatConst(const FloatType &ValueType, const FloatBits &Payload);

      // Both bool constants are present from construction and included in size().
      std::size_t size() const noexcept;
      bool owns(const Constant &ConstantValue) const noexcept;

    private:
      // The context creates its types before constructing its only constant pool.
      explicit ConstantPool(const SemanticContext &Context);

      class Impl;
      const SemanticContext &Context;
      std::unique_ptr<Impl> Storage;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
