#ifndef INK_IR_VALUE_HANDLE_H
#define INK_IR_VALUE_HANDLE_H

#include "ink/ir/model/constant.h"
#include "ink/ir/model/operand.h"

#include <memory>
#include <type_traits>
#include <utility>

namespace ink::ir
{
  // Instruction operands own non-constant operand objects and borrow immutable constants from an IRContext constant pool.
  class ValueHandle final
  {
    public:
      ValueHandle() noexcept = default;

      ValueHandle(const Constant &ConstantValue) noexcept
          : ValuePointer(&ConstantValue)
      {
      }

      template <typename OperandType, std::enable_if_t<std::is_base_of_v<Operand, OperandType>, int> = 0>
      ValueHandle(std::unique_ptr<OperandType> OperandValue) noexcept
          : OwnedOperand(std::move(OperandValue)),
            ValuePointer(OwnedOperand.get())
      {
      }

      ValueHandle(const ValueHandle &) = delete;
      ValueHandle &operator=(const ValueHandle &) = delete;

      ValueHandle(ValueHandle &&Other) noexcept
          : OwnedOperand(std::move(Other.OwnedOperand)),
            ValuePointer(OwnedOperand ? OwnedOperand.get() : Other.ValuePointer)
      {
        Other.ValuePointer = nullptr;
      }

      ValueHandle &operator=(ValueHandle &&Other) noexcept
      {
        if (this != &Other)
        {
          OwnedOperand = std::move(Other.OwnedOperand);
          ValuePointer = OwnedOperand ? OwnedOperand.get() : Other.ValuePointer;
          Other.ValuePointer = nullptr;
        }
        return *this;
      }

      ValueHandle &operator=(const Constant &ConstantValue) noexcept
      {
        OwnedOperand.reset();
        ValuePointer = &ConstantValue;
        return *this;
      }

      template <typename OperandType, std::enable_if_t<std::is_base_of_v<Operand, OperandType>, int> = 0>
      ValueHandle &operator=(std::unique_ptr<OperandType> OperandValue) noexcept
      {
        OwnedOperand = std::move(OperandValue);
        ValuePointer = OwnedOperand.get();
        return *this;
      }

      explicit operator bool() const noexcept
      {
        return ValuePointer != nullptr;
      }

      const Value *get() const noexcept
      {
        return ValuePointer;
      }

      const Value &operator*() const noexcept
      {
        return *ValuePointer;
      }

      const Value *operator->() const noexcept
      {
        return ValuePointer;
      }

    private:
      std::unique_ptr<Operand> OwnedOperand;
      const Value *ValuePointer = nullptr;
  };
} // namespace ink::ir

#endif
