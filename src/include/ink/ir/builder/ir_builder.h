#ifndef INK_IR_BUILDER_IR_BUILDER_H
#define INK_IR_BUILDER_IR_BUILDER_H

#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/address_of_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/dereference_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
#include "ink/ir/instruction/icmp_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/neg_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/context.h"
#include "ink/ir/function/function.h"

#include <utility>

namespace ink::ir
{
  // Appends resolved instructions to a function-owned block; borrows an IRContext and owns no IR.
  // The context and selected block must outlive their use through this builder. Access requires external synchronization.
  // Creation requires an insertion block with no terminator; invalid insertion state is asserted with check().
  // Creation returns a block-owned instruction, or nullptr for invalid types or operands without changing the block or insertion point.
  // Checks local types and operand ownership, but not dominance or whole-function control flow.
  // Ordinary operands may be local constants, current-function parameters/instructions or functions; storage requires explicit load/address operations.
  // Unit parameters and unit-producing calls are values; terminators and stores cannot be operands.
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
      // A terminated block can be selected, but instruction creation on it violates the asserted insertion precondition.
      [[nodiscard]] bool setInsertPoint(BasicBlock &Block) noexcept;
      void clearInsertPoint() noexcept;

      // Callees must have this exact signature, either directly or through a pointer to it. Arguments are already in parameter order.
      // Variadic calls and variadic parameter operands are rejected until parameter-pack lowering is available.
      [[nodiscard]] CallInstruction *createCall(const Function &Callee, std::vector<Argument> Arguments = {});
      [[nodiscard]] CallInstruction *createCall(const FunctionType &Signature, const Value &Callee, std::vector<Argument> Arguments = {});

      // Requires identical integer or pointer operand types. A local null literal may also compare equal/not-equal to a typed pointer.
      [[nodiscard]] ICmpInstruction *createICmp(ICmpPredicate Predicate, const Value &Left, const Value &Right);
      // Requires an integer or float and a valid insertion point; constant operands also produce a NegInstruction.
      // Unsupported operands return nullptr without appending; invalid insertion state is asserted with check().
      [[nodiscard]] NegInstruction *createNeg(const Value &Operand);
      // Allocates one non-void object with a runtime layout; zero-sized aggregates are supported. The result is storage of AllocatedType.
      [[nodiscard]] AllocaInstruction *createAlloca(const Type &AllocatedType, bool Mutable = true, bool Initialized = false);
      // Refers to the object of an evaluated, non-opaque pointer without allocating or reading memory.
      // The result is storage of the pointee type; pointer-valued storage must be explicitly loaded first.
      // Target lifetime, initialization and access permissions remain the caller's responsibility.
      [[nodiscard]] DereferenceInstruction *createDereference(const Value &Pointer);
      // Requires storage; the resulting pointer is an ordinary value and preserves the stored object's type as its pointee.
      [[nodiscard]] AddressOfInstruction *createAddressOf(const Value &Storage);
      // Memory operands are storage of the exact object type, or usable non-opaque pointers with that pointee type.
      // Null literals can initialize pointer slots, but cannot be used as untyped memory addresses.
      [[nodiscard]] LoadInstruction *createLoad(const Type &LoadedType, const Value &Pointer);
      // Ordinary stores reject immutable storage and do not change its declaration-initialization marker.
      [[nodiscard]] StoreInstruction *createStore(const Value &StoredValue, const Value &Pointer);
      // Initializes a current-function local once, including immutable locals, and records declaration initialization.
      // This marker does not prove initialization on any control-flow path.
      [[nodiscard]] StoreInstruction *createInitialize(const Value &Initializer, AllocaInstruction &Storage);
      // Infers the result pointer type from the source type and index path. Empty indices preserve the source type.
      // The first index steps over source objects; later indices traverse arrays or nominal fields.
      // Field indices must be nonnegative integer constants in range. Array indices need not be constant or in range.
      // InBounds records a caller guarantee; it does not prove runtime address bounds. Opaque pointers cannot be indexed.
      // Selecting a unit array element or field may produce an opaque pointer, but cannot traverse that unit further.
      [[nodiscard]] GetElementPtrInstruction *createGetElementPtr(const Type &SourceElementType, const Value &Pointer, std::vector<const Value *> Indices, bool InBounds = false);

      [[nodiscard]] BranchInstruction *createBranch(BasicBlock &Target);
      [[nodiscard]] ConditionalBranchInstruction *createConditionalBranch(const Value &Condition, BasicBlock &TrueTarget, BasicBlock &FalseTarget);

      // The no-argument form requires a void function; the value form requires a compatible non-void return type.
      [[nodiscard]] ReturnInstruction *createReturn();
      [[nodiscard]] ReturnInstruction *createReturn(const Value &ReturnedValue);

    private:
      bool canAppend() const noexcept;
      bool validOperand(const Value &Operand) const noexcept;
      bool validStorage(const Value &Storage) const noexcept;
      bool canUseAs(const Value &Operand, const Type &ExpectedType) const noexcept;
      const Type *memoryPointeeType(const Value &Pointer) const noexcept;
      bool validTarget(const BasicBlock &Target) const noexcept;

      template <typename InstructionT, typename... ArgumentTypes>
      InstructionT *append(ArgumentTypes &&...Arguments);

      IRContext &Context;
      BasicBlock *InsertBlock = nullptr;
  };
} // namespace ink::ir

#endif
