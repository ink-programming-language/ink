#ifndef INK_SEMANTIC_MODEL_FUNCTION_H
#define INK_SEMANTIC_MODEL_FUNCTION_H

#include "ink/semantic/model/function/basic_block.h"
#include "ink/semantic/model/function/function_type.h"
#include "ink/semantic/model/function/function_parameter.h"
#include "ink/semantic/model/name/name.h"

#include <vector>

namespace ink::semantic
{
  // A closed callable identity. Generic definitions must be instantiated before creating this value.
  class Function final : public Value
  {
    public:
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

      bool hasBody() const noexcept
      {
        return !Blocks.empty();
      }

      std::span<const FunctionParameter *const> parameters() const noexcept
      {
        return Parameters;
      }

      // The first block is the entry; declarations have no blocks and return null.
      BasicBlock *entryBlock() noexcept
      {
        return Blocks.empty() ? nullptr : Blocks.front();
      }

      const BasicBlock *entryBlock() const noexcept
      {
        return Blocks.empty() ? nullptr : Blocks.front();
      }

      // Structural children in insertion order, not execution order. Context manages membership.
      const std::vector<BasicBlock *> &blocks() const noexcept
      {
        return Blocks;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Function;
      }

    private:
      Function(Name FunctionName, const FunctionType &Signature, CallingConvention Convention, LanguageLinkage Linkage) noexcept
          : Value(Signature.context(), ValueKind::Function, Signature),
            FunctionName(FunctionName),
            Convention(Convention),
            Linkage(Linkage)
      {
      }

      Name FunctionName;
      CallingConvention Convention;
      LanguageLinkage Linkage;
      std::vector<BasicBlock *> Blocks;
      std::vector<const FunctionParameter *> Parameters;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
