#ifndef INK_IR_FUNCTION_H
#define INK_IR_FUNCTION_H

#include "ink/ir/function/basic_block.h"
#include "ink/ir/function/function_type.h"
#include "ink/ir/function/function_parameter.h"
#include "ink/ir/name/name.h"

#include <vector>
#include <limits>

namespace ink::ir
{
  class ClassType;

  // A closed callable identity. Generic definitions must be instantiated before creating this value.
  class Function final : public Value
  {
    public:
      ~Function() override;

      Name name() const noexcept
      {
        return FunctionName;
      }

      const FunctionType &functionType() const noexcept
      {
        return static_cast<const FunctionType &>(type());
      }

      CallingConvention callingConvention() const noexcept
      {
        return Convention;
      }

      LanguageLinkage languageLinkage() const noexcept
      {
        return Linkage;
      }

      FunctionBinding binding() const noexcept
      {
        return Binding;
      }

      bool isNativeImport() const noexcept
      {
        return Binding == FunctionBinding::Import;
      }

      bool isNativeExport() const noexcept
      {
        return Binding == FunctionBinding::Export;
      }

      VisibilityKind visibility() const noexcept
      {
        return Visibility;
      }

      const ClassType *classOwner() const noexcept
      {
        return ClassOwner;
      }

      std::size_t initializerField() const noexcept
      {
        return InitializerField;
      }

      bool hasBody() const noexcept
      {
        return !Blocks.empty();
      }

      const std::vector<std::unique_ptr<FunctionParameter>> &parameters() const noexcept
      {
        return Parameters;
      }

      // The first block is the entry; declarations have no blocks and return null.
      BasicBlock *entryBlock() noexcept
      {
        return Blocks.empty() ? nullptr : Blocks.front().get();
      }

      const BasicBlock *entryBlock() const noexcept
      {
        return Blocks.empty() ? nullptr : Blocks.front().get();
      }

      // Owned blocks in insertion order, not execution order. IRBuilder creates and attaches blocks.
      const std::vector<std::unique_ptr<BasicBlock>> &blocks() const noexcept
      {
        return Blocks;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Function;
      }

    private:
      Function(Name FunctionName, const FunctionType &Signature, CallingConvention Convention, LanguageLinkage Linkage, FunctionBinding Binding) noexcept
          : Value(Signature.context(), ValueKind::Function, Signature),
            FunctionName(FunctionName),
            Convention(Convention),
            Linkage(Linkage),
            Binding(Binding)
      {
      }

      const ClassType *ClassOwner = nullptr;
      std::size_t InitializerField = std::numeric_limits<std::size_t>::max();
      Name FunctionName;
      CallingConvention Convention;
      LanguageLinkage Linkage;
      FunctionBinding Binding;
      VisibilityKind Visibility = VisibilityKind::Public;
      // Blocks are destroyed before the parameters referenced by their instructions.
      std::vector<std::unique_ptr<FunctionParameter>> Parameters;
      std::vector<std::unique_ptr<BasicBlock>> Blocks;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
