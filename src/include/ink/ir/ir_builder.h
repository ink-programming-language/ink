#ifndef INK_IR_IR_BUILDER_H
#define INK_IR_IR_BUILDER_H

#include "ink/ir/context.h"
#include "ink/ir/decl/class_decl.h"
#include "ink/ir/decl/function_decl.h"
#include "ink/ir/decl/module_decl.h"
#include "ink/ir/function/function.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/add_instruction.h"
#include "ink/ir/instruction/array_instruction.h"
#include "ink/ir/instruction/array_element_pointer_instruction.h"
#include "ink/ir/instruction/array_extract_instruction.h"
#include "ink/ir/instruction/class_instruction.h"
#include "ink/ir/instruction/field_extract_instruction.h"
#include "ink/ir/instruction/field_pointer_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/c_string_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/compare_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/logical_and_instruction.h"
#include "ink/ir/instruction/logical_not_instruction.h"
#include "ink/ir/instruction/logical_or_instruction.h"
#include "ink/ir/instruction/store_instruction.h"
#include "ink/ir/instruction/return_instruction.h"

#include <memory>
#include <span>
#include <type_traits>
#include <utility>

namespace ink::ir
{
  // Creates IR objects and edits ownership using Context storage and explicit parent relationships.
  // Context and selected blocks must outlive this builder and saved points; clear points before deleting their nodes.
  // Builders sharing a context have independent insertion points; concurrent model mutation is not supported.
  class IRBuilder final
  {
    public:
      // Before == nullptr means the current end of Block. Both null means no insertion point.
      // Node identities survive vector growth; removing or reparenting Before invalidates the saved point.
      struct InsertPoint
      {
          BasicBlock *Block = nullptr;
          Value *Before = nullptr;
      };

      class InsertPointGuard final
      {
        public:
          explicit InsertPointGuard(IRBuilder &Builder) noexcept;
          ~InsertPointGuard();
          InsertPointGuard(const InsertPointGuard &) = delete;
          InsertPointGuard &operator=(const InsertPointGuard &) = delete;
          InsertPointGuard(InsertPointGuard &&) = delete;
          InsertPointGuard &operator=(InsertPointGuard &&) = delete;

        private:
          IRBuilder &Builder;
          InsertPoint SavedPoint;
      };

      explicit IRBuilder(IRContext &Context) noexcept;
      IRBuilder(const IRBuilder &) = delete;
      IRBuilder &operator=(const IRBuilder &) = delete;
      IRBuilder(IRBuilder &&) = delete;
      IRBuilder &operator=(IRBuilder &&) = delete;

      IRContext &context() const noexcept
      {
        return Context;
      }

      BasicBlock *insertBlock() const noexcept
      {
        return Point.Block;
      }

      // Selects the block's end or a direct child to insert before. Repeated creations preserve their call order.
      // Invalid set/restore operations return false and clear the point, preventing insertion at an old location.
      bool setInsertPoint(BasicBlock &Block, Value *Before = nullptr) noexcept;
      bool setInsertPoint(Value &Before) noexcept;
      void clearInsertPoint() noexcept;
      InsertPoint saveInsertPoint() const noexcept;
      bool restoreInsertPoint(InsertPoint SavedPoint) noexcept;

      // Explicit ownership edits use their arguments and do not change the current insertion point.
      // Transfers ownership only on success; pass std::move(Owner). Failure leaves Owner and the block unchanged.
      // Both objects must belong to this builder's Context. Rejects existing parents, cycles, types and constants.
      // Terminators must end a function's block; branches stay in that function and returns match its return type.
      template <typename ValueType>
      bool appendValue(BasicBlock &Block, std::unique_ptr<ValueType> &&Child)
      {
        return insertValue(Block, std::move(Child));
      }

      // Inserts before a direct child, or appends when Before is null, with the same checks as appendValue().
      // An invalid anchor leaves both objects unchanged.
      template <typename ValueType>
      bool insertValue(BasicBlock &Block, std::unique_ptr<ValueType> &&Child, Value *Before = nullptr)
      {
        static_assert(std::is_base_of_v<Value, ValueType> && !std::is_const_v<ValueType>);
        if (!Child || !canInsertValue(Block, *Child, Before))
        {
          return false;
        }
        insertOwnedValue(Block, std::move(Child), Before);
        return true;
      }

      // Detaches a direct child and returns ownership; foreign objects and non-children return null.
      [[nodiscard]] std::unique_ptr<Value> removeValue(BasicBlock &Block, Value &Child) noexcept;
      // Deletes a direct child and its subtree. Callers must first discard or repair borrowed references and builder points.
      bool eraseValue(BasicBlock &Block, Value &Child) noexcept;

      // Root ownership can be detached for nesting, or restored. Failed insertion preserves the supplied owner.
      [[nodiscard]] std::unique_ptr<Module> removeModule(Module &ModuleValue) noexcept;
      template <typename ValueType>
      bool appendModule(std::unique_ptr<ValueType> &&ModuleValue)
      {
        static_assert(std::is_base_of_v<Value, ValueType> && !std::is_const_v<ValueType>);
        if (!ModuleValue || !Module::classof(ModuleValue.get()) || &ModuleValue->context() != &Context || ModuleValue->outer())
        {
          return false;
        }
        std::unique_ptr<Value> Owner = std::move(ModuleValue);
        Context.Modules.push_back(std::unique_ptr<Module>(static_cast<Module *>(Owner.release())));
        Context.notifyChanged();
        return true;
      }

      // Deleting a root invalidates borrowed references to its subtree, including resolver bindings and builder points.
      bool eraseModule(Module &ModuleValue) noexcept;

      // Types, declarations, functions, blocks and modules do not use or change the insertion point.
      // Resolved nominal identities are allocated directly in Context.typePool(), without a Decl node.
      // Callers reuse the returned object for the same type or instantiated type.
      const ClassType *createClassType(Name TypeName);
      // Completes a nominal definition once; pointer recursion is legal, infinite value layouts are not.
      bool defineClassType(const ClassType &ValueType, std::span<const ClassField> Fields, std::string_view Identity);
      bool registerClassType(Module &ModuleValue, const ClassType &Class);
      bool setClassMethod(const ClassType &Class, Function &Method);
      bool setFieldInitializer(const ClassType &Class, std::size_t Field, Function &Initializer);
      const EnumType *createEnumType(Name TypeName);
      const InterfaceType *createInterfaceType(Name TypeName);

      // Returns a detached owner. Closed functions carry a local signature and initially have no body.
      // Creates parameters with this function as their outer. Empty kinds default to Positional;
      // otherwise exactly one valid kind per signature slot is required. Kinds are binding metadata,
      // not part of FunctionType identity, and do not enable named-argument binding or variadic expansion.
      // Empty names leave all slots unnamed; otherwise supply one local Name (or unnamed Name{}) per slot.
      // Calling convention, language linkage and native binding are function metadata, not FunctionType identity.
      // Defaults to a local function with the target C convention and Ink linkage; invalid enum values return null.
      // Native imports and exports require C convention and linkage. Local C functions may have bodies.
      [[nodiscard]] std::unique_ptr<Function> createFunction(Name FunctionName, const FunctionType &Signature, std::span<const ParameterKind> ParameterKinds = {}, std::span<const Name> ParameterNames = {}, CallingConvention Convention = CallingConvention::C, LanguageLinkage Linkage = LanguageLinkage::Ink, FunctionBinding Binding = FunctionBinding::Local);
      // Creates an empty entry block. Foreign contexts, native imports or existing bodies return null.
      BasicBlock *createFunctionBody(Function &FunctionValue);
      bool setFunctionVisibility(Function &FunctionValue, VisibilityKind Visibility) noexcept;
      // Native imports cannot have bodies; native imports and exports require C convention and linkage.
      bool setFunctionBinding(Function &FunctionValue, FunctionBinding Binding) noexcept;
      // Returns a detached owner with the local label type and an initially empty value list.
      [[nodiscard]] std::unique_ptr<BasicBlock> createBasicBlock();
      // Appends a block; the first becomes the entry. Foreign contexts and native imports return null.
      BasicBlock *createBasicBlock(Function &FunctionValue);

      // Creates a context-owned root module with its own empty entry block. Requires a local name.
      Module *createModule(Name ModuleName);

      // Attaches the local module's unique declaration root, using its name; existing roots return null.
      // The AST and its ParsedUnit must outlive the owning module.
      ModuleDecl *createModuleDecl(Module &Owner, const parser::ModuleAST &AST);
      // Appends an owned child to a local declaration; names must belong to Context.namePool().
      // Function/class declarations require generic AST definitions. Failure leaves the tree unchanged.
      FunctionDecl *createFunctionDecl(Decl &Parent, Name DeclName, const parser::FunctionDecl &AST);
      ClassDecl *createClassDecl(Decl &Parent, Name DeclName, const parser::ClassDecl &AST);

      // Detached instruction factories ignore the insertion point and return owners for explicit transfer.
      // A function-typed value is required. Arguments are
      // non-null local values with exactly matching types after analysis/conversion.
      // Each call has its own identity; these factories do not execute the function.
      [[nodiscard]] std::unique_ptr<CallInstruction> createDetachedCallInstruction(const Value &Callee, std::span<const Value *const> Arguments = {});
      // Copies a local NUL-free string into fresh writable u8 storage on each execution.
      // Must be attached to a function block; its storage lasts for that function activation.
      [[nodiscard]] std::unique_ptr<CStringInstruction> createDetachedCStringInstruction(const StringConstant &Source);
      // Allocates one uninitialized object and returns a read-write pointer; arrays use ArrayType.
      // Supports bool, integer, float, pointer, reference, slice and arrays of those types.
      // Rejects foreign, non-storage and unresolved nominal types; size/alignment await target layout.
      [[nodiscard]] std::unique_ptr<AllocaInstruction> createDetachedAllocaInstruction(const Type &AllocatedType);
      // Requires a local pointer to a supported storage type; accepts read-only and read-write access.
      [[nodiscard]] std::unique_ptr<LoadInstruction> createDetachedLoadInstruction(const Value &Address);
      // Requires local operands, read-write pointer access and exact pointee/value type identity.
      // Returns a void-typed operation. These factories create detached nodes without executing them.
      [[nodiscard]] std::unique_ptr<StoreInstruction> createDetachedStoreInstruction(const Value &Address, const Value &StoredValue);
      // Same local integer type for both operands; result wraps to that width.
      [[nodiscard]] std::unique_ptr<AddInstruction> createDetachedAddInstruction(const Value &Left, const Value &Right);
      // Requires exact local element types; repetition requires exactly one element operand.
      [[nodiscard]] std::unique_ptr<ArrayInstruction> createDetachedArrayInstruction(const ArrayType &ValueType, std::span<const Value *const> Elements, bool Repeated = false);
      // Indices are local integers; execution checks negative and out-of-range indices.
      [[nodiscard]] std::unique_ptr<ArrayElementPointerInstruction> createDetachedArrayElementPointerInstruction(const Value &Address, const Value &Index);
      [[nodiscard]] std::unique_ptr<ArrayExtractInstruction> createDetachedArrayExtractInstruction(const Value &Array, const Value &Index);
      [[nodiscard]] std::unique_ptr<ClassInstruction> createDetachedClassInstruction(const ClassType &ValueType, std::span<const Value *const> Fields);
      [[nodiscard]] std::unique_ptr<FieldExtractInstruction> createDetachedFieldExtractInstruction(const Value &Object, std::size_t FieldIndex);
      [[nodiscard]] std::unique_ptr<FieldPointerInstruction> createDetachedFieldPointerInstruction(const Value &Address, std::size_t FieldIndex);
      // Logical operations accept only local bool operands; And and Or are eager IR operations.
      [[nodiscard]] std::unique_ptr<LogicalNotInstruction> createDetachedLogicalNotInstruction(const Value &Operand);
      [[nodiscard]] std::unique_ptr<LogicalAndInstruction> createDetachedLogicalAndInstruction(const Value &Left, const Value &Right);
      [[nodiscard]] std::unique_ptr<LogicalOrInstruction> createDetachedLogicalOrInstruction(const Value &Left, const Value &Right);
      // Same local integer type for all predicates; bool operands support only Equal and NotEqual.
      [[nodiscard]] std::unique_ptr<CompareInstruction> createDetachedCompareInstruction(ComparisonPredicate Predicate, const Value &Left, const Value &Right);
      // Creates a detached return; a supplied operand must be local and non-void.
      // Function identity and return-signature validation are determined when appendValue() attaches it.
      [[nodiscard]] std::unique_ptr<ReturnInstruction> createDetachedReturnInstruction(const Value *ReturnedValue = nullptr);
      // Targets must be local function blocks or detached blocks; attachment requires the source and targets in one function.
      [[nodiscard]] std::unique_ptr<BranchInstruction> createDetachedBranchInstruction(const BasicBlock &Target);
      // Requires a local bool condition. Attached targets must belong to the same function.
      [[nodiscard]] std::unique_ptr<ConditionalBranchInstruction> createDetachedConditionalBranchInstruction(const Value &Condition, const BasicBlock &TrueTarget, const BasicBlock &FalseTarget);

      // Inserting instruction factories require a valid point. Failure returns null without insertion.
      // Instructions cannot follow a terminator. Alloca uses the selected point, with no implicit move to the entry block.
      CallInstruction *createCallInstruction(const Value &Callee, std::span<const Value *const> Arguments = {});
      CStringInstruction *createCStringInstruction(const StringConstant &Source);
      AllocaInstruction *createAllocaInstruction(const Type &AllocatedType);
      LoadInstruction *createLoadInstruction(const Value &Address);
      StoreInstruction *createStoreInstruction(const Value &Address, const Value &StoredValue);
      AddInstruction *createAddInstruction(const Value &Left, const Value &Right);
      ArrayInstruction *createArrayInstruction(const ArrayType &ValueType, std::span<const Value *const> Elements, bool Repeated = false);
      ArrayElementPointerInstruction *createArrayElementPointerInstruction(const Value &Address, const Value &Index);
      ArrayExtractInstruction *createArrayExtractInstruction(const Value &Array, const Value &Index);
      ClassInstruction *createClassInstruction(const ClassType &ValueType, std::span<const Value *const> Fields);
      FieldExtractInstruction *createFieldExtractInstruction(const Value &Object, std::size_t FieldIndex);
      FieldPointerInstruction *createFieldPointerInstruction(const Value &Address, std::size_t FieldIndex);
      LogicalNotInstruction *createLogicalNotInstruction(const Value &Operand);
      LogicalAndInstruction *createLogicalAndInstruction(const Value &Left, const Value &Right);
      LogicalOrInstruction *createLogicalOrInstruction(const Value &Left, const Value &Right);
      CompareInstruction *createCompareInstruction(ComparisonPredicate Predicate, const Value &Left, const Value &Right);
      // Returns require a function block, a matching return value, and an unterminated block's end.
      ReturnInstruction *createReturnInstruction(const Value *ReturnedValue = nullptr);
      BranchInstruction *createBranchInstruction(const BasicBlock &Target);
      ConditionalBranchInstruction *createConditionalBranchInstruction(const Value &Condition, const BasicBlock &TrueTarget, const BasicBlock &FalseTarget);

    private:
      bool isValidInsertPoint(const BasicBlock &Block, const Value *Before) const noexcept;
      bool canInsertAt(const BasicBlock &Block, const Value *Before, bool IsTerminator) const noexcept;
      bool canInsertReturn(const BasicBlock &Block, const Value *ReturnedValue, const Value *Before) const noexcept;
      bool isValidBranchTarget(const BasicBlock &Target) const noexcept;
      bool canInsertBranch(const BasicBlock &Block, const BasicBlock &Target, const Value *Before) const noexcept;
      bool canInsertValue(const BasicBlock &Block, const Value &Child, const Value *Before) const noexcept;
      void insertOwnedValue(BasicBlock &Block, std::unique_ptr<Value> Child, Value *Before);

      bool canInsert() const noexcept;

      template <typename InstructionType>
      InstructionType *insert(std::unique_ptr<InstructionType> Instruction)
      {
        InstructionType *Pointer = Instruction.get();
        return Instruction && insertValue(*Point.Block, std::move(Instruction), Point.Before) ? Pointer : nullptr;
      }

      IRContext &Context;
      InsertPoint Point;
  };
} // namespace ink::ir

#endif
