#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/core/context.h"
#include "ink/semantic/model/type/array_type.h"
#include "ink/semantic/model/type/builtin_type.h"
#include "ink/semantic/model/constant/constant_pool.h"
#include "ink/semantic/model/decl/class_decl.h"
#include "ink/semantic/model/decl/function_decl.h"
#include "ink/semantic/model/decl/module_decl.h"
#include "ink/semantic/model/function/basic_block.h"
#include "ink/semantic/model/function/function.h"
#include "ink/semantic/model/function/function_type.h"
#include "ink/semantic/model/instruction/alloca_instruction.h"
#include "ink/semantic/model/instruction/add_instruction.h"
#include "ink/semantic/model/instruction/call_instruction.h"
#include "ink/semantic/model/instruction/load_instruction.h"
#include "ink/semantic/model/instruction/store_instruction.h"
#include "ink/semantic/model/instruction/return_instruction.h"
#include "ink/semantic/model/module/module.h"
#include "ink/semantic/model/type/class_type.h"
#include "ink/semantic/model/type/enum_type.h"
#include "ink/semantic/model/type/float_type.h"
#include "ink/semantic/model/type/integer_type.h"
#include "ink/semantic/model/type/interface_type.h"
#include "ink/semantic/model/name/name_pool.h"
#include "ink/semantic/model/type/pointer_type.h"
#include "ink/semantic/model/type/reference_type.h"
#include "ink/semantic/model/type/slice_type.h"
#include "ink/semantic/model/type/user_defined_type.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ink::semantic
{
  // Model storage for a semantic session. CompilationContext and borrowed AST
  // units must outlive this context. No pool supports removal or index recycling.
  class SemanticContext final
  {
    public:
      explicit SemanticContext(core::CompilationContext &Compilation);
      ~SemanticContext();
      SemanticContext(const SemanticContext &) = delete;
      SemanticContext &operator=(const SemanticContext &) = delete;
      SemanticContext(SemanticContext &&) = delete;
      SemanticContext &operator=(SemanticContext &&) = delete;

      core::CompilationContext &compilationContext() noexcept
      {
        return Compilation;
      }

      const core::CompilationContext &compilationContext() const noexcept
      {
        return Compilation;
      }

      NamePool &namePool() noexcept
      {
        return Names;
      }

      const NamePool &namePool() const noexcept
      {
        return Names;
      }

      ConstantPool &constantPool() noexcept;
      const ConstantPool &constantPool() const noexcept;

      const BuiltinType &getMetaType() const noexcept;
      const BuiltinType &getVoidType() const noexcept;
      const BuiltinType &getBoolType() const noexcept;
      // Internal type for basic block identities.
      const BuiltinType &getLabelType() const noexcept;
      // Internal type for module values.
      const BuiltinType &getModuleType() const noexcept;
      // BitWidth == 0 is invalid. Signedness participates in canonical identity.
      const IntegerType *getIntegerType(std::uint32_t BitWidth, bool Signed);
      // IEEE binary16/32/64. Other widths return null.
      const FloatType *getFloatType(std::uint32_t BitWidth);
      // Structural factories require local component types. Source-language
      // legality and target layout limits are checked by later analysis.
      const ArrayType *getArrayType(const Type &ElementType, std::uint64_t ElementCount);
      const SliceType *getSliceType(const Type &ElementType, AccessKind Access);
      const PointerType *getPointerType(const Type &PointeeType, AccessKind Access);
      const ReferenceType *getReferenceType(const Type &ReferentType, AccessKind Access);
      // Fixed-arity signatures are canonical by return type and ordered parameters.
      // All component types must be local; null parameters are rejected.
      const FunctionType *getFunctionType(const Type &ReturnType, std::span<const Type *const> ParameterTypes = {});
      // Resolved nominal identities are allocated directly, without a semantic Decl.
      // Callers reuse the returned object for the same type or instantiated type.
      const ClassType *createClassType(Name TypeName);
      const EnumType *createEnumType(Name TypeName);
      const InterfaceType *createInterfaceType(Name TypeName);
      // Forward to constantPool(); both entry points return the same objects.
      // Requires this context's type, valid bits and exactly matching payload width.
      // Conversion/overflow checking belongs to semantic analysis, not interning.
      const IntegerConstant *getIntegerConstant(const IntegerType &ValueType, const IntegerBits &Payload);
      const BoolConstant &getBoolConstant(bool Payload) const noexcept;
      // Requires a local read-only u8 slice and already validated, decoded UTF-8 bytes.
      const StringConstant *getStringConstant(const SliceType &ValueType, std::string_view Payload);
      // Requires valid bits in the local type's exact IEEE binary16/32/64 format.
      const FloatConstant *getFloatConstant(const FloatType &ValueType, const FloatBits &Payload);
      // Closed functions carry a local signature and initially have no body.
      // Creates parameters with this function as their outer. Empty kinds default to Positional;
      // otherwise exactly one valid kind per signature slot is required. Kinds are binding metadata,
      // not part of FunctionType identity, and do not enable named-argument binding or variadic expansion.
      // Empty names leave all slots unnamed; otherwise supply one local Name (or unnamed Name{}) per slot.
      // Calling convention and language linkage are independent function metadata, not FunctionType identity.
      // Defaults to the target C calling convention and Ink language linkage; undefined enum values return null.
      Function *createFunction(Name FunctionName, const FunctionType &Signature, std::span<const ParameterKind> ParameterKinds = {}, std::span<const Name> ParameterNames = {}, CallingConvention Convention = CallingConvention::C, LanguageLinkage Linkage = LanguageLinkage::Ink);
      // Creates a local function's empty entry block and sets its parent. Foreign functions or existing bodies return null.
      BasicBlock *createFunctionBody(Function &FunctionValue);
      // Each block has an independent identity, the local label type and an initially empty value list.
      BasicBlock *createBasicBlock();
      // Appends a new block to a local function; the first block becomes its entry. Foreign functions return null.
      BasicBlock *createBasicBlock(Function &FunctionValue);
      // Both objects must belong here. Rejects existing parents, cycles, types and constants without changing either object.
      // Returns must be inserted into a function's block and match that function's return type.
      bool appendValue(BasicBlock &Block, Value &Child);
      // Detaches a local direct child without destroying it; a non-child or foreign object returns false.
      bool removeValue(BasicBlock &Block, Value &Child) noexcept;
      // Requires a local name; each module receives a distinct empty entry block.
      Module *createModule(Name ModuleName);
      // A function-typed value is required. Arguments are
      // non-null local values with exactly matching types after analysis/conversion.
      // Each call has its own identity; these factories do not execute the function.
      CallInstruction *createCallInstruction(const Value &Callee, std::span<const Value *const> Arguments = {});
      // Allocates one uninitialized object and returns a read-write pointer; arrays use ArrayType.
      // Supports bool, integer, float, pointer, reference, slice and arrays of those types.
      // Rejects foreign, non-storage and unresolved nominal types; size/alignment await target layout.
      AllocaInstruction *createAllocaInstruction(const Type &AllocatedType);
      // Requires a local pointer to a supported storage type; accepts read-only and read-write access.
      LoadInstruction *createLoadInstruction(const Value &Address);
      // Requires local operands, read-write pointer access and exact pointee/value type identity.
      // Returns a void-typed operation. These factories create detached nodes without executing them.
      StoreInstruction *createStoreInstruction(const Value &Address, const Value &StoredValue);
      // Same local integer type for both operands; result wraps to that width.
      AddInstruction *createAddInstruction(const Value &Left, const Value &Right);
      // Creates a detached return; a supplied operand must be local and non-void.
      // Function identity and return-signature validation are determined when appendValue() attaches it.
      ReturnInstruction *createReturnInstruction(const Value *ReturnedValue = nullptr);
      // Declaration names must belong to namePool(); child lists are populated by the caller.
      // The borrowed AST and its ParsedUnit must outlive this context.
      ModuleDecl *createModuleDecl(Name DeclName, const parser::ModuleAST &AST);
      // Function/class declarations require generic AST definitions.
      FunctionDecl *createFunctionDecl(Name DeclName, const parser::FunctionDecl &AST);
      ClassDecl *createClassDecl(Name DeclName, const parser::ClassDecl &AST);

    private:
      struct ArrayTypeKey
      {
          const Type *ElementType;
          std::uint64_t ElementCount;

          bool operator==(const ArrayTypeKey &) const noexcept = default;
      };

      struct ArrayTypeKeyHash
      {
          std::size_t operator()(const ArrayTypeKey &Key) const noexcept;
      };

      struct AccessTypeKey
      {
          const Type *TargetType;
          AccessKind Access;

          bool operator==(const AccessTypeKey &) const noexcept = default;
      };

      struct AccessTypeKeyHash
      {
          std::size_t operator()(const AccessTypeKey &Key) const noexcept;
      };

      core::CompilationContext &Compilation;
      NamePool Names;
      std::unique_ptr<BuiltinType> MetaType;
      std::unique_ptr<BuiltinType> VoidType;
      std::unique_ptr<BuiltinType> BoolType;
      std::unique_ptr<BuiltinType> LabelType;
      std::unique_ptr<BuiltinType> ModuleType;
      // Model destructors never traverse borrowed AST/type/value edges.
      std::vector<std::unique_ptr<Decl>> Declarations;
      std::unordered_map<std::uint64_t, std::unique_ptr<IntegerType>> IntegerTypes;
      std::unordered_map<std::uint32_t, std::unique_ptr<FloatType>> FloatTypes;
      std::vector<std::unique_ptr<UserDefinedType>> UserDefinedTypes;
      std::unordered_map<ArrayTypeKey, std::unique_ptr<ArrayType>, ArrayTypeKeyHash> ArrayTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<SliceType>, AccessTypeKeyHash> SliceTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<PointerType>, AccessTypeKeyHash> PointerTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<ReferenceType>, AccessTypeKeyHash> ReferenceTypes;
      std::unordered_multimap<std::size_t, std::unique_ptr<FunctionType>> FunctionTypes;
      // Constants are destroyed before the types they reference.
      std::unique_ptr<ConstantPool> Constants;
      std::vector<std::unique_ptr<Function>> Functions;
      std::vector<std::unique_ptr<BasicBlock>> BasicBlocks;
      std::vector<std::unique_ptr<Module>> Modules;
      std::vector<std::unique_ptr<CallInstruction>> Calls;
      std::vector<std::unique_ptr<AllocaInstruction>> Allocations;
      std::vector<std::unique_ptr<LoadInstruction>> Loads;
      std::vector<std::unique_ptr<StoreInstruction>> Stores;
      std::vector<std::unique_ptr<FunctionParameter>> Parameters;
      std::vector<std::unique_ptr<AddInstruction>> Additions;
      std::vector<std::unique_ptr<ReturnInstruction>> Returns;
  };
} // namespace ink::semantic

#endif
