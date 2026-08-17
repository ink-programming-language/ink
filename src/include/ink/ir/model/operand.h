#ifndef INK_IR_OPERAND_H
#define INK_IR_OPERAND_H

#include "ink/ir/model/id.h"
#include "ink/ir/model/value.h"

#include <cstddef>

namespace ink::ir
{
  class Operand : public Value
  {
    protected:
      Operand(ValueKind Kind, const Type &ValueType) noexcept
          : Value(Kind, ValueType)
      {
      }
  };

  class ValueOperand final : public Operand
  {
    public:
      ValueOperand(const Type &ValueType, ValueId Id) noexcept
          : Operand(ValueKind::ValueOperand, ValueType),
            Id(Id)
      {
      }

      ValueId id() const noexcept
      {
        return Id;
      }

    private:
      ValueId Id;
  };

  class GlobalAddressOperand final : public Operand
  {
    public:
      GlobalAddressOperand(const Type &ValueType, ByteConstantId Constant, std::size_t ByteOffset) noexcept
          : Operand(ValueKind::GlobalAddressOperand, ValueType),
            Constant(Constant),
            ByteOffset(ByteOffset)
      {
      }

      GlobalAddressOperand(const Type &ValueType, GlobalId Constant, std::size_t ByteOffset) noexcept
          : GlobalAddressOperand(ValueType, ByteConstantId{Constant.value()}, ByteOffset)
      {
      }

      ByteConstantId byteConstant() const noexcept
      {
        return Constant;
      }

      ByteConstantId global() const noexcept
      {
        return byteConstant();
      }

      std::size_t byteOffset() const noexcept
      {
        return ByteOffset;
      }

      void resolveByteConstant(ByteConstantId ResolvedConstant) noexcept
      {
        Constant = ResolvedConstant;
      }

      void resolveGlobal(GlobalId ResolvedConstant) noexcept
      {
        resolveByteConstant(ByteConstantId{ResolvedConstant.value()});
      }

    private:
      ByteConstantId Constant;
      std::size_t ByteOffset;
  };

  class GlobalVariableAddressOperand final : public Operand
  {
    public:
      GlobalVariableAddressOperand(const Type &ValueType, GlobalId Global) noexcept
          : Operand(ValueKind::GlobalVariableAddressOperand, ValueType),
            Global(Global)
      {
      }

      GlobalId global() const noexcept
      {
        return Global;
      }

      void resolveGlobal(GlobalId ResolvedGlobal) noexcept
      {
        Global = ResolvedGlobal;
      }

    private:
      GlobalId Global;
  };
} // namespace ink::ir

#endif
