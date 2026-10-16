#ifndef INK_IR_BUILDER_IR_BUILDER_H
#define INK_IR_BUILDER_IR_BUILDER_H

#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
#include "ink/ir/instruction/icmp_instruction.h"
#include "ink/ir/instruction/neg_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

#include <utility>

namespace ink::ir
{
  // Appends resolved instructions to a function-owned block; borrows an IRContext and owns no IR.
  // The context and selected block must outlive their use through this builder. Access requires external synchronization.
  // Creation returns a block-owned instruction, or nullptr without changing the block or insertion point.
  // Checks local types, operand ownership and terminators, but not dominance or whole-function control flow.
  // Operands may be local constants, current-function parameters/instructions or functions; globals require explicit storage operations.
  // Unit parameters and unit-producing calls are values; terminators cannot be operands.
  class IRBuilder final
  {
    public:
      explicit IRBuilder(IRContext &Context) noexcept;

      auto &irContext(this auto &&Self) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.Context);
      }

      auto &typePool(this auto &&Self) noexcept
      {
        return Self.irContext().typePool();
      }

      auto &constantPool(this auto &&Self) noexcept
      {
        return Self.irContext().constantPool();
      }

      BasicBlock *insertBlock() const noexcept;
      Function *insertFunction() const noexcept;
      // Rejects detached blocks and foreign-context function signatures, preserving the previous insertion point.
      // A terminated block can be selected, but no further instructions can be appended to it.
      [[nodiscard]] bool setInsertPoint(BasicBlock &Block) noexcept;
      void clearInsertPoint() noexcept;

      // Callees must have this exact signature, either directly or through a pointer to it. Arguments are already in parameter order.
      // Variadic calls and variadic parameter operands are rejected until parameter-pack lowering is available.
      [[nodiscard]] CallInstruction *createCall(const Function &Callee, std::vector<Argument> Arguments = {});
      [[nodiscard]] CallInstruction *createCall(const FunctionType &Signature, const Value &Callee, std::vector<Argument> Arguments = {});

      // Requires identical integer or pointer operand types. A local null literal may also compare equal/not-equal to a typed pointer.
      [[nodiscard]] ICmpInstruction *createICmp(ICmpPredicate Predicate, const Value &Left, const Value &Right);
      // Requires an integer or float and a valid insertion point; constant operands also produce a NegInstruction.
      // Unsupported operands or invalid insertion state return nullptr without appending.
      [[nodiscard]] NegInstruction *createNeg(const Value &Operand);
      // Empty indices preserve the source type. The first index steps over source objects; later indices traverse arrays or nominal fields.
      // Field indices must be nonnegative integer constants in range. Array indices need not be constant or in range.
      // InBounds records a caller guarantee; it does not prove runtime address bounds. Opaque pointers cannot be indexed.
      // Selecting a unit array element or field may produce an opaque pointer, but cannot traverse that unit further.
      [[nodiscard]] GetElementPtrInstruction *createGetElementPtr(const PointerType &ResultType, const Type &SourceElementType, const Value &Pointer, std::vector<const Value *> Indices, bool InBounds = false);

      [[nodiscard]] BranchInstruction *createBranch(BasicBlock &Target);
      [[nodiscard]] ConditionalBranchInstruction *createConditionalBranch(const Value &Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget);

      // The no-argument form requires a void function; the value form requires a compatible non-void return type.
      [[nodiscard]] ReturnInstruction *createReturn();
      [[nodiscard]] ReturnInstruction *createReturn(const Value &ReturnedValue);

    private:
      bool canAppend() const noexcept;
      bool validOperand(const Value &Operand) const noexcept;
      bool canUseAs(const Value &Operand, const Type &ExpectedType) const noexcept;
      bool validTarget(const BasicBlock &Target) const noexcept;

      template <typename InstructionT, typename... ArgumentTypes>
      InstructionT *append(ArgumentTypes &&...Arguments);

      IRContext &Context;
      BasicBlock *InsertBlock = nullptr;
  };
} // namespace ink::ir

#endif
