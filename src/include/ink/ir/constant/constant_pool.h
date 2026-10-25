#ifndef INK_IR_CONSTANT_CONSTANT_POOL_H
#define INK_IR_CONSTANT_CONSTANT_POOL_H

#include "ink/ir/constant/array_constant.h"
#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/float_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/constant/null_pointer_constant.h"
#include "ink/ir/constant/string_constant.h"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ir
{
  class IRContext;

  namespace detail
  {
    class IRLoadTransaction;
  } // namespace detail

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

      template <typename ConstantT> requires std::same_as<ConstantT, BoolConstant>
      const ConstantT *get(bool Value) const noexcept
      {
        return Value ? &True : &False;
      }

      // Typed lookups return nullptr for null/foreign-context types or invalid
      // content. Raw bits must fit the width; numeric integers match signedness.
      // Raw IntegerBits must have exactly the type's width; no implicit resizing is performed.
      template <typename ConstantT> requires std::same_as<ConstantT, IntegerConstant>
      const ConstantT *get(const IntegerType *IntegerTypeValue, const core::IntegerBits &Bits)
      {
        return internIntegerFromBits(IntegerTypeValue, Bits);
      }

      template <typename ConstantT, std::integral IntegerT> requires (std::same_as<ConstantT, IntegerConstant> && !std::same_as<std::remove_cv_t<IntegerT>, bool> && sizeof(IntegerT) <= sizeof(std::uint64_t))
      const ConstantT *get(const IntegerType *IntegerTypeValue, IntegerT Value)
      {
        if constexpr (std::is_signed_v<IntegerT>)
        {
          return internSignedInteger(IntegerTypeValue, static_cast<std::int64_t>(Value));
        }
        else
        {
          return internUnsignedInteger(IntegerTypeValue, static_cast<std::uint64_t>(Value));
        }
      }

      // Exact bits distinguish signed zeros and NaN payloads; the bit width must match the float type.
      template <typename ConstantT> requires std::same_as<ConstantT, FloatConstant>
      const ConstantT *get(const FloatType *FloatTypeValue, const core::IntegerBits &Bits)
      {
        return internFloatFromBits(FloatTypeValue, Bits);
      }

      // Numeric overloads use this context's binary32 type for float and
      // binary64 type for double, preserving bits without precision conversion.
      template <typename ConstantT, typename FloatT> requires (std::same_as<ConstantT, FloatConstant> && (std::same_as<FloatT, float> || std::same_as<FloatT, double>))
      const ConstantT *get(FloatT Value)
      {
        return internFloat(Value);
      }

      // Owns the element list and borrows only constants owned by this context.
      // Count must match, and each element must be able to initialize the array's element type.
      template <typename ConstantT> requires std::same_as<ConstantT, ArrayConstant>
      const ConstantT *get(const ArrayType *ConstantType, std::vector<const Constant *> Elements = {})
      {
        return internArray(ConstantType, std::move(Elements));
      }

      // Returns the one null pointer literal shared by all pointer types in this context.
      template <typename ConstantT> requires std::same_as<ConstantT, NullPointerConstant>
      const ConstantT *get() const noexcept
      {
        return &NullPointer;
      }

      // Owns decoded UTF-8 bytes, including empty text and embedded NUL. Invalid
      // UTF-8 or lengths exceeding the target limit return nullptr. No escapes,
      // normalization or terminator are added; exact byte contents are the key.
      // Uses this context's ArrayType(u8, byteLength), so size is the byte count and alignment is 1.
      template <typename ConstantT> requires std::same_as<ConstantT, StringConstant>
      const ConstantT *get(std::string Value)
      {
        return internString(std::move(Value));
      }

    private:
      friend class IRContext;
      friend class detail::IRLoadTransaction;

      explicit ConstantPool(IRContext &Owner);
      ~ConstantPool() noexcept;

      const IntegerConstant *internIntegerFromBits(const IntegerType *IntegerTypeValue, const core::IntegerBits &Bits);
      const IntegerConstant *internSignedInteger(const IntegerType *IntegerTypeValue, std::int64_t Value);
      const IntegerConstant *internUnsignedInteger(const IntegerType *IntegerTypeValue, std::uint64_t Value);
      const FloatConstant *internFloatFromBits(const FloatType *FloatTypeValue, const core::IntegerBits &Bits);
      const FloatConstant *internFloatFromBits(const FloatType *FloatTypeValue, std::uint64_t Bits);
      const FloatConstant *internFloat(float Value);
      const FloatConstant *internFloat(double Value);
      const ArrayConstant *internArray(const ArrayType *ConstantType, std::vector<const Constant *> Elements);
      const StringConstant *internString(std::string Value);

      struct IntegerBitsHash
      {
          std::size_t operator()(const core::IntegerBits &Bits) const noexcept;
      };

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
      std::unordered_map<const IntegerType *, std::unordered_map<core::IntegerBits, const IntegerConstant *, IntegerBitsHash>> IntegerConstants;
      std::unordered_map<const FloatType *, std::unordered_map<std::uint64_t, const FloatConstant *>> FloatConstants;
      std::unordered_map<ArrayKey, const ArrayConstant *, ArrayKeyHash> ArrayConstants;
      // Keys borrow immutable stored string bytes. Destroy this cache before Storage.
      std::unordered_map<std::string_view, const StringConstant *> StringConstants;
  };
} // namespace ink::ir

#endif
