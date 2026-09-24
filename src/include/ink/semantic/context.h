#ifndef INK_SEMANTIC_CONTEXT_H
#define INK_SEMANTIC_CONTEXT_H

#include "ink/core/context.h"
#include "ink/semantic/constant.h"
#include "ink/semantic/decl.h"
#include "ink/semantic/expr_value.h"
#include "ink/semantic/name_pool.h"

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

      const BuiltinType &getMetaType() const noexcept;
      const BuiltinType &getVoidType() const noexcept;
      const BuiltinType &getBoolType() const noexcept;
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
      // One nominal type per local, non-instantiated type declaration.
      const UserDefinedType *getUserDefinedType(const TypeDecl &Declaration);
      // Requires this context's type and exactly matching payload width.
      // Conversion/overflow checking belongs to semantic analysis, not interning.
      const IntegerConstant *getIntegerConstant(const IntegerType &ValueType, const llvm::APInt &Payload);
      const BoolConstant &getBoolConstant(bool Payload) const noexcept;
      // The caller has checked Expression and supplies its local result type.
      // This records its origin without evaluation or AST-based interning.
      const ExprValue *createExprValue(const Type &ValueType, const parser::Expr &Expression, core::SourceId Source = {});
      // Name must originate in namePool(); an initializer must belong here too.
      // Invalid indices, mutability or foreign initializers fail.
      VarDecl *createVarDecl(Name DeclName, BindingMutability Mutability, DeclSource Source = {}, const Value *Initializer = nullptr);
      // Only Class, Enum and Interface are valid declaration kinds here.
      TypeDecl *createTypeDecl(Name DeclName, DeclKind Kind, DeclSource Source = {});

    private:
      class Impl;
      core::CompilationContext &Compilation;
      NamePool Names;
      std::unique_ptr<Impl> Storage;
  };
} // namespace ink::semantic

#endif
