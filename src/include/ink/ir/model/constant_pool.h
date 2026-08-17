#ifndef INK_IR_CONSTANT_POOL_H
#define INK_IR_CONSTANT_POOL_H

#include "ink/ir/model/constant.h"

#include <cstddef>
#include <functional>
#include <initializer_list>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir
{
  // Owns immutable constants and returns one stable canonical object for each exact constant representation.
  class ConstantPool final
  {
    public:
      ConstantPool();
      ~ConstantPool();
      ConstantPool(const ConstantPool &) = delete;
      ConstantPool &operator=(const ConstantPool &) = delete;
      ConstantPool(ConstantPool &&) = delete;
      ConstantPool &operator=(ConstantPool &&) = delete;

      template <typename IntegerType, std::enable_if_t<std::is_integral_v<IntegerType>, int> = 0>
      const IntegerConstant &getIntegerConstant(const Type &ValueType, IntegerType Integer)
      {
        auto Candidate = std::unique_ptr<IntegerConstant>(new IntegerConstant(ValueType, Integer));
        return static_cast<const IntegerConstant &>(intern(std::move(Candidate)));
      }

      const FloatConstant &getFloatConstant(const Type &ValueType, FloatFormat Format, std::uint64_t BitPattern);
      const StringConstant &getStringConstant(const Type &ValueType, std::string Data);
      const NullConstant &getNullConstant(const Type &ValueType);
      const ZeroInitializer &getZeroInitializer(const Type &ValueType);
      const AggregateConstant &getAggregateConstant(const Type &ValueType, const std::vector<std::reference_wrapper<const Constant>> &Elements);
      const AggregateConstant &getAggregateConstant(const Type &ValueType, std::initializer_list<std::reference_wrapper<const Constant>> Elements);

      std::size_t size() const noexcept;
      bool owns(const Constant &ConstantValue) const noexcept;

    private:
      const Constant &intern(std::unique_ptr<Constant> Candidate);
      const Constant &internCopy(const Constant &Source);

      class Impl;
      std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::ir

#endif
