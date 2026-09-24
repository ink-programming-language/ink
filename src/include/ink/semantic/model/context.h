#ifndef INK_SEMANTIC_MODEL_CONTEXT_H
#define INK_SEMANTIC_MODEL_CONTEXT_H

#include "ink/core/context.h"
#include "ink/semantic/model/type/array_type.h"
#include "ink/semantic/model/type/builtin_type.h"
#include "ink/semantic/model/constant_pool.h"
#include "ink/semantic/model/decl/class_decl.h"
#include "ink/semantic/model/decl/function_decl.h"
#include "ink/semantic/model/decl/module_decl.h"
#include "ink/semantic/model/expr_value.h"
#include "ink/semantic/model/function/basic_block.h"
#include "ink/semantic/model/function/function.h"
#include "ink/semantic/model/function/function_type.h"
#include "ink/semantic/model/instruction/call_instruction.h"
#include "ink/semantic/model/module/module.h"
#include "ink/semantic/model/type/class_type.h"
#include "ink/semantic/model/type/enum_type.h"
#include "ink/semantic/model/type/float_type.h"
#include "ink/semantic/model/type/integer_type.h"
#include "ink/semantic/model/type/interface_type.h"
#include "ink/semantic/model/name_pool.h"
#include "ink/semantic/model/type/pointer_type.h"
#include "ink/semantic/model/type/reference_type.h"
#include "ink/semantic/model/type/slice_type.h"
#include "ink/semantic/model/type/user_defined_type.h"
#include "ink/semantic/model/variable.h"

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
      const StringConst *getStringConst(const SliceType &ValueType, std::string_view Payload);
      // Requires valid bits in the local type's exact IEEE binary16/32/64 format.
      const FloatConst *getFloatConst(const FloatType &ValueType, const FloatBits &Payload);
      // The caller has checked Expression and supplies its local result type.
      // This records its origin without evaluation or AST-based interning.
      ExprValue *createExprValue(const Type &ValueType, const parser::Expr &Expression, core::SourceId Source = {});
      // Closed functions carry a local signature and initially have no body.
      Function *createFunction(Name FunctionName, const FunctionType &Signature);
      // Creates a local function's empty entry block and sets its parent. Foreign functions or existing bodies return null.
      BasicBlock *createFunctionBody(Function &FunctionValue);
      // Each block has an independent identity, the local label type and an initially empty value list.
      BasicBlock *createBasicBlock();
      // Appends a new block to a local function; the first block becomes its entry. Foreign functions return null.
      BasicBlock *createBasicBlock(Function &FunctionValue);
      // Both objects must belong here. Rejects existing parents, cycles, types and constants without changing either object.
      bool appendValue(BasicBlock &Block, Value &Child);
      // Detaches a local direct child without destroying it; a non-child or foreign object returns false.
      bool removeValue(BasicBlock &Block, Value &Child) noexcept;
      // Requires a local name; each module receives a distinct empty entry block.
      Module *createModule(Name ModuleName);
      // A function-typed value is required. Arguments are
      // non-null local values with exactly matching types after analysis/conversion.
      // Each call has its own identity; these factories do not execute the function.
      CallInstruction *createCallInstruction(const Value &Callee, std::span<const Value *const> Arguments = {});
      // Name must originate in namePool(); an initializer must belong here too.
      // Invalid indices, mutability or foreign initializers fail.
      Variable *createVariable(Name VariableName, BindingMutability Mutability, const Value *Initializer = nullptr);
      // Declaration names must belong to namePool(); child lists are populated by the caller.
      // The borrowed AST and its ParsedUnit must outlive this context.
      ModuleDecl *createModuleDecl(Name DeclName, const parser::ModuleAST &AST);
      // Function/class declarations require generic AST definitions.
      FunctionDecl *createFunctionDecl(Name DeclName, const parser::FunctionDecl &AST);
      ClassDecl *createClassDecl(Name DeclName, const parser::ClassDecl &AST);

    private:
      class Impl;
      core::CompilationContext &Compilation;
      NamePool Names;
      std::unique_ptr<Impl> Storage;
  };
} // namespace ink::semantic

#endif
