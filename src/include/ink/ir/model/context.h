#ifndef INK_IR_CONTEXT_H
#define INK_IR_CONTEXT_H

#include "ink/core/context.h"
#include "ink/ir/model/constant_pool.h"
#include "ink/ir/model/struct_type.h"
#include "ink/ir/model/type.h"

#include <array>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <vector>

namespace ink::ir
{
  class IRContext
  {
    public:
      explicit IRContext(core::CompilationContext &Compilation);
      ~IRContext();
      IRContext(const IRContext &) = delete;
      IRContext &operator=(const IRContext &) = delete;
      IRContext(IRContext &&) = delete;
      IRContext &operator=(IRContext &&) = delete;

      core::CompilationContext &compilationContext() noexcept
      {
        return Compilation;
      }

      const core::CompilationContext &compilationContext() const noexcept
      {
        return Compilation;
      }

      core::DiagnosticEngine &diagnosticEngine() noexcept
      {
        return Compilation.diagnosticEngine();
      }

      const core::DiagnosticEngine &diagnosticEngine() const noexcept
      {
        return Compilation.diagnosticEngine();
      }

      const Type &getType(TypeKind Kind) const noexcept;
      const StructType &createStructType(Name Name, std::initializer_list<const Type *> FieldTypes);
      const StructType &createStructType(Name Name, std::vector<const Type *> FieldTypes);
      const StructType &createStructType(Name Name, std::vector<StructField> Fields, StructLayoutConstraints LayoutConstraints = {});

      ConstantPool &constantPool() noexcept
      {
        return Constants;
      }

      const ConstantPool &constantPool() const noexcept
      {
        return Constants;
      }

    private:
      core::CompilationContext &Compilation;
      std::array<std::unique_ptr<Type>, static_cast<std::size_t>(TypeKind::Struct)> PrimitiveTypes;
      ConstantPool Constants;
      std::vector<std::unique_ptr<StructType>> StructTypes;
  };
} // namespace ink::ir

#endif
