#ifndef INK_IR_BUILDER_H
#define INK_IR_BUILDER_H

#include "ink/ir/instruction/arithmetic.h"
#include "ink/ir/instruction/control_flow.h"
#include "ink/ir/instruction/instruction.h"
#include "ink/ir/instruction/memory.h"
#include "ink/ir/model/basic_block.h"
#include "ink/ir/model/global_variable.h"
#include "ink/ir/model/module.h"
#include "ink/ir/model/struct_type.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::ir
{
  // Builds Module structure independently of any source syntax and provides reusable insertion and SSA allocation services for lowering pipelines.
  class IRBuilder final
  {
    public:
      explicit IRBuilder(IRContext &Context) noexcept;
      IRBuilder(const IRBuilder &) = delete;
      IRBuilder &operator=(const IRBuilder &) = delete;
      IRBuilder(IRBuilder &&) noexcept = default;
      IRBuilder &operator=(IRBuilder &&) noexcept = default;

      IRContext &context() noexcept;
      const IRContext &context() const noexcept;
      Module &module() noexcept;
      const Module &module() const noexcept;

      void setModuleName(std::optional<Name> NameValue);
      const StructType &createStructType(Name NameValue, std::vector<StructField> Fields, StructLayoutConstraints LayoutConstraints = {});
      ByteConstantId addByteConstant(Name NameValue, std::string Data);
      GlobalId addGlobal(GlobalVariable GlobalValue);
      FunctionId addFunction(Function FunctionValue);
      std::optional<BlockId> addBlock(FunctionId Function, BasicBlock Block);
      FunctionId createFunction(Name NameValue, const Type &ResultType, std::vector<Parameter> Parameters = {}, std::vector<Attribute> Attributes = {});
      std::optional<BlockId> createBlock(FunctionId Function, Name NameValue);

      Function *function(FunctionId Function) noexcept;
      const Function *function(FunctionId Function) const noexcept;
      BasicBlock *block(FunctionId Function, BlockId Block) noexcept;
      const BasicBlock *block(FunctionId Function, BlockId Block) const noexcept;

      bool setInitializer(std::optional<FunctionId> Function) noexcept;
      bool setFinalizer(std::optional<FunctionId> Function) noexcept;
      bool setInsertionPoint(FunctionId Function, BlockId Block) noexcept;
      void clearInsertionPoint() noexcept;
      std::optional<FunctionId> insertionFunction() const noexcept;
      std::optional<BlockId> insertionBlock() const noexcept;

      std::optional<ValueId> allocateValueId(FunctionId Function) noexcept;
      std::optional<ValueId> allocateValueId() noexcept;

      CallInstruction *createCall(FunctionId Callee, std::vector<ValueHandle> Arguments = {});
      ImportInstruction *createImport(Name Module);
      AllocaInstruction *createAlloca(const Type &ResultType, ValueHandle Size);
      GetElementPointerInstruction *createGetElementPointer(const Type &ElementType, ValueHandle Pointer, ValueHandle Index, std::vector<ValueHandle> FieldIndices = {});
      LoadInstruction *createLoad(const Type &ResultType, ValueHandle Pointer);
      StoreInstruction *createStore(ValueHandle StoredValue, ValueHandle Pointer);
      LifetimeEndInstruction *createLifetimeEnd(ValueHandle Slice);
      SliceDataInstruction *createSliceData(ValueHandle Slice);
      SliceLengthInstruction *createSliceLength(ValueHandle Slice);
      PhiInstruction *createPhi(const Type &ResultType, std::vector<PhiIncoming> IncomingValues);
      AddInstruction *createAdd(ValueHandle Left, ValueHandle Right);
      CompareInstruction *createCompare(ComparePredicate Predicate, ValueHandle Left, ValueHandle Right);
      InsertValueInstruction *createInsertValue(ValueHandle Aggregate, ValueHandle Element, std::size_t FieldIndex);
      ExtractValueInstruction *createExtractValue(ValueHandle Aggregate, std::size_t FieldIndex);
      BranchInstruction *createBranch(BlockId Target);
      ConditionalBranchInstruction *createConditionalBranch(ValueHandle Condition, BlockId TrueTarget, BlockId FalseTarget);
      ReturnInstruction *createReturn();
      ReturnInstruction *createReturn(ValueHandle ReturnValue);

      ValueHandle createValueOperand(const Type &ValueType, ValueId Value) const;
      ValueHandle createGlobalAddress(const Type &ValueType, ByteConstantId Constant, std::size_t ByteOffset) const;
      ValueHandle createGlobalVariableAddress(const Type &ValueType, GlobalId Global) const;

      template <typename IntegerType, std::enable_if_t<std::is_integral_v<IntegerType>, int> = 0>
      const IntegerConstant &getIntegerConstant(const Type &ValueType, IntegerType Integer)
      {
        return context().constantPool().getIntegerConstant(ValueType, Integer);
      }

      const FloatConstant &getFloatConstant(const Type &ValueType, FloatFormat Format, std::uint64_t BitPattern);
      const StringConstant &getStringConstant(const Type &ValueType, std::string Data);
      const NullConstant &getNullConstant(const Type &ValueType);
      const ZeroInitializer &getZeroInitializer(const Type &ValueType);
      const AggregateConstant &getAggregateConstant(const Type &ValueType, const std::vector<std::reference_wrapper<const Constant>> &Elements);

      Module takeModule();
      void reset();

    private:
      bool hasValidInsertionPoint() const noexcept;
      Instruction *insertInstruction(std::unique_ptr<Instruction> InstructionValue);

      template <typename InstructionType>
      InstructionType *insertInstruction(std::unique_ptr<InstructionType> InstructionValue)
      {
        InstructionType *Result = InstructionValue.get();
        return insertInstruction(std::unique_ptr<Instruction>(std::move(InstructionValue))) == nullptr ? nullptr : Result;
      }

      template <typename InstructionType>
      InstructionType *insertValueInstruction(std::unique_ptr<InstructionType> InstructionValue)
      {
        if (!hasValidInsertionPoint())
        {
          return nullptr;
        }
        const std::optional<ValueId> Result = allocateValueId();
        if (!Result.has_value())
        {
          return nullptr;
        }
        InstructionValue->Result = *Result;
        return insertInstruction(std::move(InstructionValue));
      }

      void updateNextValueId(FunctionId Function, const Instruction &InstructionValue) noexcept;
      std::size_t findNextValueId(const Function &FunctionValue) const noexcept;

      IRContext *Context;
      Module ModuleValue;
      std::vector<std::size_t> NextValueIds;
      std::optional<FunctionId> InsertionFunction;
      std::optional<BlockId> InsertionBlock;
  };
} // namespace ink::ir

#endif
