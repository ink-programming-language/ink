#ifndef INK_IR_CONSTANT_CONSTANT_POOL_H
#define INK_IR_CONSTANT_CONSTANT_POOL_H

#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/class_constant.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/string_constant.h"

#include <cstddef>
#include <memory>
#include <unordered_map>

namespace ink::ir
{
  class IRContext;

  // Each IRContext owns one pool of immutable, context-local constants.
  // Canonical objects keep their addresses until the context is destroyed.
  class ConstantPool final
  {
    public:
      ~ConstantPool();
      ConstantPool(const ConstantPool &) = delete;
      ConstantPool &operator=(const ConstantPool &) = delete;
      ConstantPool(ConstantPool &&) = delete;
      ConstantPool &operator=(ConstantPool &&) = delete;

      const IRContext &context() const noexcept
      {
        return Context;
      }

      // Foreign types, malformed bits or mismatched widths return null without inserting.
      // Interning preserves exact typed bits; conversions belong to analysis.
      const IntegerConstant *getIntegerConstant(const IntegerType &ValueType, const IntegerBits &Payload);
      const BoolConstant &getBoolConstant(bool Payload) const noexcept;
      // Requires a local read-only u8 slice. The caller supplies validated, decoded UTF-8;
      // interning copies and compares every byte without escape decoding or normalization.
      // Storage has an additional NUL; it is excluded from the value's length and interning identity.
      const StringConstant *getStringConstant(const SliceType &ValueType, std::string_view Payload);
      // Requires valid bits in the local type's exact IEEE format; never converts or rounds.
      const FloatConstant *getFloatConstant(const FloatType &ValueType, const FloatBits &Payload);

      // Requires the exact number of canonical local constants with the array's element type.
      // Empty and nested arrays are canonicalized by their type and ordered element identities.
      const ArrayConstant *getArrayConstant(const ArrayType &ValueType, std::span<const Constant *const> Elements);
      const ClassConstant *getClassConstant(const ClassType &ValueType, std::span<const Constant *const> Fields);

      // Both bool constants are present from construction and included in size().
      std::size_t size() const noexcept;
      bool owns(const Constant &ConstantValue) const noexcept;

    private:
      // The context creates its types before constructing its only constant pool.
      explicit ConstantPool(const IRContext &Context);

      const IRContext &Context;
      std::unordered_multimap<std::size_t, std::unique_ptr<IntegerConstant>> IntegerConstants;
      std::unordered_multimap<std::size_t, std::unique_ptr<StringConstant>> StringConstants;
      std::unordered_multimap<std::size_t, std::unique_ptr<FloatConstant>> FloatConstants;
      std::unordered_multimap<std::size_t, std::unique_ptr<ArrayConstant>> ArrayConstants;
      std::unordered_multimap<std::size_t, std::unique_ptr<ClassConstant>> ClassConstants;
      std::unique_ptr<BoolConstant> FalseValue;
      std::unique_ptr<BoolConstant> TrueValue;

      friend class IRContext;
  };
} // namespace ink::ir

#endif
