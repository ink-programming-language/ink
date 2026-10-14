#ifndef INK_IR_CONSTANT_CONSTANT_POOL_H
#define INK_IR_CONSTANT_CONSTANT_POOL_H

#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/null_pointer_constant.h"
#include "ink/ir/constant/string_constant.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::ir
{
  class IRContext;

  // Owns and reuses immutable constants as an inseparable part of one IRContext.
  // Access through IRContext::constantPool(); constants and pooled types share the context lifetime.
  // External IR users must die first. Operations require external synchronization.
  class ConstantPool final
  {
    public:
      ConstantPool(const ConstantPool &) = delete;
      ConstantPool &operator=(const ConstantPool &) = delete;
      ConstantPool(ConstantPool &&) = delete;
      ConstantPool &operator=(ConstantPool &&) = delete;

      const IRContext &irContext() const noexcept;
      bool owns(const Constant *ConstantValue) const noexcept;

      const BoolConstant *getBoolConstant(bool Value) const noexcept;

      // Typed lookups return nullptr for null/foreign-context types or invalid
      // content. Raw bits must fit the width; numeric integers match signedness.
      const IntegerConstant *getIntegerConstantFromBits(const IntegerType *IntegerTypeValue, std::uint64_t Bits);
      const IntegerConstant *getSignedIntegerConstant(const IntegerType *IntegerTypeValue, std::int64_t Value);
      const IntegerConstant *getUnsignedIntegerConstant(const IntegerType *IntegerTypeValue, std::uint64_t Value);

      // Exact bits distinguish signed zeros and NaN payloads.
      const FloatConstant *getFloatConstantFromBits(const FloatType *FloatTypeValue, std::uint64_t Bits);

      // Numeric overloads use this context's binary32 type for float and
      // binary64 type for double, preserving bits without precision conversion.
      const FloatConstant *getFloatConstant(float Value);
      const FloatConstant *getFloatConstant(double Value);

      // Owns the element list and borrows only constants owned by this context.
      // Count must match, and each element must be able to initialize the array's element type.
      const ArrayConstant *getArrayConstant(const ArrayType *ConstantType, std::vector<const Constant *> Elements = {});
      // Returns the one null pointer literal shared by all pointer types in this context.
      const NullPointerConstant *getNullPointerConstant() const noexcept;

      // Owns decoded UTF-8 bytes, including empty text and embedded NUL. Invalid
      // UTF-8 or lengths exceeding the target limit return nullptr. No escapes,
      // normalization or terminator are added; exact byte contents are the key.
      // Uses this context's ArrayType(u8, byteLength), so size is the byte count and alignment is 1.
      const StringConstant *getStringConstant(std::string Value);

    private:
      friend class IRContext;

      explicit ConstantPool(IRContext &Owner);
      ~ConstantPool() noexcept;

      struct ArrayKey
      {
          const ArrayType *ConstantType;
          std::vector<const Constant *> Elements;

          bool operator==(const ArrayKey &Other) const noexcept;
      };

      struct ArrayKeyHash
      {
          std::size_t operator()(const ArrayKey &Key) const noexcept;
      };

      template <typename ConstantT>
      const ConstantT *own(std::unique_ptr<ConstantT> ConstantValue);

      IRContext &Owner;
      BoolConstant False;
      BoolConstant True;
      NullPointerConstant NullPointer;
      std::vector<std::unique_ptr<Constant>> Storage;
      std::unordered_set<const Constant *> OwnedConstants;
      std::unordered_map<const IntegerType *, std::unordered_map<std::uint64_t, const IntegerConstant *>> IntegerConstants;
      std::unordered_map<const FloatType *, std::unordered_map<std::uint64_t, const FloatConstant *>> FloatConstants;
      std::unordered_map<ArrayKey, const ArrayConstant *, ArrayKeyHash> ArrayConstants;
      // Keys borrow immutable stored string bytes. Destroy this cache before Storage.
      std::unordered_map<std::string_view, const StringConstant *> StringConstants;
  };
} // namespace ink::ir

#endif
